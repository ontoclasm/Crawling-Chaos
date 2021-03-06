/**
 * @file
 * @brief Functions that may be missing from some systems
**/

#include "AppHdr.h"

#include "defines.h"
#include "itemname.h" // is_vowel()
#include "libutil.h"
#include "externs.h"
#include "files.h"
#include "macro.h"
#include "message.h"
#include "unicode.h"
#include "viewgeom.h"

#include <sstream>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#ifdef TARGET_OS_WINDOWS
    #undef ARRAYSZ
    #include <windows.h>
    #undef max

    #ifdef WINMM_PLAY_SOUNDS
        #include <mmsystem.h>
    #endif
#endif

#ifdef UNIX
    #include <signal.h>
#endif

#ifdef DGL_ENABLE_CORE_DUMP
    #include <sys/time.h>
    #include <sys/resource.h>
#endif

unsigned int isqrt(unsigned int a)
{
    unsigned int rem = 0, root = 0;
    for (int i = 0; i < 16; i++)
    {
        root <<= 1;
        rem = (rem << 2) + (a >> 30);
        a <<= 2;
        if (++root <= rem)
            rem -= root++;
        else
            root--;
    }
    return (root >> 1);
}

int isqrt_ceil(int x)
{
    if (x <= 0)
        return 0;
    return isqrt(x - 1) + 1;
}

description_level_type description_type_by_name(const char *desc)
{
    if (!desc)
        return DESC_PLAIN;

    if (!strcmp("The", desc) || !strcmp("the", desc))
        return DESC_THE;
    else if (!strcmp("A", desc) || !strcmp("a", desc))
        return DESC_A;
    else if (!strcmp("Your", desc) || !strcmp("your", desc))
        return DESC_YOUR;
    else if (!strcmp("its", desc))
        return DESC_ITS;
    else if (!strcmp("worn", desc))
        return DESC_INVENTORY_EQUIP;
    else if (!strcmp("inv", desc))
        return DESC_INVENTORY;
    else if (!strcmp("none", desc))
        return DESC_NONE;
    else if (!strcmp("base", desc))
        return DESC_BASENAME;
    else if (!strcmp("qual", desc))
        return DESC_QUALNAME;

    return DESC_PLAIN;
}

static std::string _number_to_string(unsigned number, bool in_words)
{
    return (in_words? number_in_words(number) : make_stringf("%u", number));
}

std::string apply_description(description_level_type desc,
                              const std::string &name,
                              int quantity, bool in_words)
{
    switch (desc)
    {
    case DESC_THE:
        return ("the " + name);
    case DESC_A:
        return (quantity > 1 ? _number_to_string(quantity, in_words) + name
                             : article_a(name, true));
    case DESC_YOUR:
        return ("your " + name);
    case DESC_PLAIN:
    default:
        return (name);
    }
}

// Should return true if the filename contains nothing that
// the shell can do damage with.
bool shell_safe(const char *file)
{
    int match = strcspn(file, "\\`$*?|><&\n!;");
    return (match < 0 || !file[match]);
}

void play_sound(const char *file)
{
#if defined(WINMM_PLAY_SOUNDS)
    // Check whether file exists, is readable, etc.?
    if (file && *file)
        sndPlaySoundW(OUTW(file), SND_ASYNC | SND_NODEFAULT);

#elif defined(SOUND_PLAY_COMMAND)
    char command[255];
    command[0] = 0;
    if (file && *file && (strlen(file) + strlen(SOUND_PLAY_COMMAND) < 255)
        && shell_safe(file))
    {
        snprintf(command, sizeof command, SOUND_PLAY_COMMAND, file);
        system(OUTS(command));
    }
#endif
}

std::string strip_filename_unsafe_chars(const std::string &s)
{
    return replace_all_of(s, " .&`\"\'|;{}()[]<>*%$#@!~?", "");
}

std::string vmake_stringf(const char* s, va_list args)
{
    char buf1[8000];
    va_list orig_args;
    va_copy(orig_args, args);
    size_t len = vsnprintf(buf1, sizeof buf1, s, orig_args);
    va_end(orig_args);
    if (len < sizeof buf1)
        return (buf1);

    char *buf2 = (char*)malloc(len + 1);
    va_copy(orig_args, args);
    vsnprintf(buf2, len + 1, s, orig_args);
    va_end(orig_args);
    std::string ret(buf2);
    free(buf2);

    return (ret);
}

std::string make_stringf(const char *s, ...)
{
    va_list args;
    va_start(args, s);
    std::string ret = vmake_stringf(s, args);
    va_end(args);
    return ret;
}

bool key_is_escape(int key)
{
    switch (key)
    {
    CASE_ESCAPE
        return (true);
    default:
        return (false);
    }
}

std::string &uppercase(std::string &s)
{
    for (unsigned i = 0, sz = s.size(); i < sz; ++i)
        s[i] = toupper(s[i]);

    return (s);
}

std::string &lowercase(std::string &s)
{
    s = lowercase_string(s);
    return (s);
}

std::string lowercase_string(std::string s)
{
    std::string res;
    ucs_t c;
    char buf[4];
    for (const char *tp = s.c_str(); int len = utf8towc(&c, tp); tp += len)
        res.append(buf, wctoutf8(buf, towlower(c)));
    return (res);
}

// Warning: this (and uppercase_first()) relies on no libc (glibc, BSD libc,
// MSVC crt) supporting letters that expand or contract, like German ß (-> SS)
// upon capitalization / lowercasing.  This is mostly a fault of the API --
// there's no way to return two characters in one code point.
// Also, all characters must have the same length in bytes before and after
// lowercasing, all platforms currently have this property.
//
// A non-hacky version would be slower for no gain other than sane code; at
// least unless you use some more powerful API.
std::string lowercase_first(std::string s)
{
    ucs_t c;
    if (!s.empty())
    {
        utf8towc(&c, &s[0]);
        wctoutf8(&s[0], towlower(c));
    }
    return (s);
}

std::string uppercase_first(std::string s)
{
    // Incorrect due to those pesky Dutch having "ij" as a single letter (wtf?).
    // Too bad, there's no standard function to handle that character, and I
    // don't care enough.
    ucs_t c;
    if (!s.empty())
    {
        utf8towc(&c, &s[0]);
        wctoutf8(&s[0], towupper(c));
    }
    return (s);
}

int ends_with(const std::string &s, const char *suffixes[])
{
    if (!suffixes)
        return (0);

    for (int i = 0; suffixes[i]; ++i)
        if (ends_with(s, suffixes[i]))
            return (1 + i);

    return (0);
}

bool strip_suffix(std::string &s, const std::string &suffix)
{
    if (ends_with(s, suffix))
    {
        s.erase(s.length() - suffix.length(), suffix.length());
        trim_string(s);
        return (true);
    }
    return false;
}

// Returns true if s contains tag 'tag', and strips out tag from s.
bool strip_tag(std::string &s, const std::string &tag, bool skip_padding)
{
    if (s == tag)
    {
        s.clear();
        return (true);
    }

    std::string::size_type pos;

    if (skip_padding)
    {
        if ((pos = s.find(tag)) != std::string::npos)
        {
            s.erase(pos, tag.length());
            trim_string(s);
            return (true);
        }
        return (false);
    }

    if ((pos = s.find(" " + tag + " ")) != std::string::npos)
    {
        // Leave one space intact.
        s.erase(pos, tag.length() + 1);
        trim_string(s);
        return (true);
    }

    if ((pos = s.find(tag + " ")) == 0
        || ((pos = s.find(" " + tag)) != std::string::npos
            && pos + tag.length() + 1 == s.length()))
    {
        s.erase(pos, tag.length() + 1);
        trim_string(s);
        return (true);
    }

    return (false);
}

std::vector<std::string> strip_multiple_tag_prefix (std::string &s, const std::string &tagprefix)
{
    std::vector<std::string> results;

    while (true)
    {
        std::string this_result = strip_tag_prefix(s, tagprefix);
        if (this_result.empty())
            break;

        results.push_back(this_result);
    }

    return results;
}

std::string strip_tag_prefix(std::string &s, const std::string &tagprefix)
{
    std::string::size_type pos = s.find(tagprefix);

    while (pos && pos != std::string::npos && !isspace(s[pos - 1]))
        pos = s.find(tagprefix, pos + 1);

    if (pos == std::string::npos)
        return ("");

    std::string::size_type ns = s.find(" ", pos);
    if (ns == std::string::npos)
        ns = s.length();

    const std::string argument =
        s.substr(pos + tagprefix.length(), ns - pos - tagprefix.length());

    s.erase(pos, ns - pos + 1);
    trim_string(s);

    return (argument);
}

int strip_number_tag(std::string &s, const std::string &tagprefix)
{
    const std::string num = strip_tag_prefix(s, tagprefix);
    int x;
    if (num.empty() || !parse_int(num.c_str(), x))
        return TAG_UNFOUND;
    return x;
}

bool parse_int(const char *s, int &i)
{
    if (!s || !*s)
        return false;
    char *err;
    long x = strtol(s, &err, 10);
    if (*err || x < INT_MIN || x > INT_MAX)
        return false;
    i = x;
    return true;
}

// Naively prefix A/an to a noun.
std::string article_a(const std::string &name, bool lowercase)
{
    if (!name.length())
        return name;

    const char *a  = lowercase? "a "  : "A ";
    const char *an = lowercase? "an " : "An ";
    switch (name[0])
    {
        case 'a': case 'e': case 'i': case 'o': case 'u':
        case 'A': case 'E': case 'I': case 'O': case 'U':
            // XXX: Hack
            if (starts_with(name, "one-"))
                return a + name;
            return an + name;
        default:
            return a + name;
    }
}

const char *standard_plural_qualifiers[] =
{
    " of ", " labeled ", NULL
};

// Pluralises a monster or item name.  This'll need to be updated for
// correctness whenever new monsters/items are added.
std::string pluralise(const std::string &name,
                      const char *qualifiers[],
                      const char *no_qualifier[])
{
    std::string::size_type pos;

    if (qualifiers)
    {
        for (int i = 0; qualifiers[i]; ++i)
            if ((pos = name.find(qualifiers[i])) != std::string::npos
                && !ends_with(name, no_qualifier))
            {
                return pluralise(name.substr(0, pos)) + name.substr(pos);
            }
    }

    if (!name.empty() && name[name.length() - 1] == ')'
        && (pos = name.rfind(" (")) != std::string::npos)
    {
        return (pluralise(name.substr(0, pos)) + name.substr(pos));
    }

    if (ends_with(name, "us"))
    {
        // Fungus, ufetubus, for instance.
        return name.substr(0, name.length() - 2) + "i";
    }
    else if (ends_with(name, "larva") || ends_with(name, "amoeba")
          || ends_with(name, "antenna"))
    {
        // Giant amoebae sounds a little weird, to tell the truth.
        return name + "e";
    }
    else if (ends_with(name, "ex"))
    {
        // Vortex; vortexes is legal, but the classic plural is cooler.
        return name.substr(0, name.length() - 2) + "ices";
    }
    else if (ends_with(name, "mosquito") || ends_with(name, "ss"))
        return name + "es";
    else if (ends_with(name, "cyclops"))
        return name.substr(0, name.length() - 1) + "es";
    else if (name == "catoblepas")
        return "catoblepae";
    else if (ends_with(name, "s"))
        return name;
    else if (ends_with(name, "y"))
    {
        if (name == "y")
            return ("ys");
        // day -> days, boy -> boys, etc
        else if (is_vowel(name[name.length() - 2]))
            return name + "s";
        // jelly -> jellies
        else
            return name.substr(0, name.length() - 1) + "ies";
    }
    else if (ends_with(name, "fe"))
    {
        // knife -> knives
        return name.substr(0, name.length() - 2) + "ves";
    }
    else if (ends_with(name, "staff"))
    {
        // staff -> staves
        return name.substr(0, name.length() - 2) + "ves";
    }
    else if (ends_with(name, "f") && !ends_with(name, "ff"))
    {
        // elf -> elves, but not hippogriff -> hippogrives.
        // TODO: if someone defines a "goblin chief", this should be revisited.
        return name.substr(0, name.length() - 1) + "ves";
    }
    else if (ends_with(name, "mage"))
    {
        // mage -> magi
        return name.substr(0, name.length() - 1) + "i";
    }
    else if (ends_with(name, "sheep") || ends_with(name, "fish")
             || ends_with(name, "folk") || ends_with(name, "spawn")
             || ends_with(name, "tengu") || ends_with(name, "shedu")
             || ends_with(name, "swine") || ends_with(name, "efreet")
             // "shedu" is male, "lammasu" is female of the same creature
             || ends_with(name, "lammasu") || ends_with(name, "lamassu"))
    {
        return name;
    }
    else if (ends_with(name, "ch") || ends_with(name, "sh")
             || ends_with(name, "x"))
    {
        // To handle cockroaches, sphinxes, and bushes.
        return name + "es";
    }
    else if (ends_with(name, "simulacrum") || ends_with(name, "eidolon"))
    {
        // simulacrum -> simulacra (correct Latin pluralisation)
        // also eidolon -> eidola (correct Greek pluralisation)
        return name.substr(0, name.length() - 2) + "a";
    }
    else if (name == "foot")
        return "feet";
    else if (name == "ophan" || name == "cherub" || name == "seraph")
    {
        // Unlike "angel" which is fully assimilated, and "cherub" and "seraph"
        // which may be pluralised both ways, "ophan" always uses Hebrew
        // pluralisation.
        return name + "im";
    }

    return name + "s";
}

std::string apostrophise(const std::string &name)
{
    if (name.empty())
        return (name);

    if (name == "you" || name == "You")
        return (name + "r");

    if (name == "it" || name == "It")
        return (name + "s");

    const char lastc = name[name.length() - 1];
    return (name + (lastc == 's' ? "'" : "'s"));
}

std::string apostrophise_fixup(const std::string &msg)
{
    if (msg.empty())
        return (msg);

    // XXX: This is rather hackish.
    return (replace_all(msg, "s's", "s'"));
}

static std::string pow_in_words(int pow)
{
    switch (pow)
    {
    case 0:
        return "";
    case 3:
        return " thousand";
    case 6:
        return " million";
    case 9:
        return " billion";
    case 12:
    default:
        return " trillion";
    }
}

static std::string tens_in_words(unsigned num)
{
    static const char *numbers[] = {
        "", "one", "two", "three", "four", "five", "six", "seven",
        "eight", "nine", "ten", "eleven", "twelve", "thirteen", "fourteen",
        "fifteen", "sixteen", "seventeen", "eighteen", "nineteen"
    };
    static const char *tens[] = {
        "", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy",
        "eighty", "ninety"
    };

    if (num < 20)
        return numbers[num];

    int ten = num / 10, digit = num % 10;
    return std::string(tens[ten])
             + (digit ? std::string("-") + numbers[digit] : "");
}

static std::string join_strings(const std::string &a, const std::string &b)
{
    if (!a.empty() && !b.empty())
        return (a + " " + b);

    return (a.empty() ? b : a);
}

static std::string hundreds_in_words(unsigned num)
{
    unsigned dreds = num / 100, tens = num % 100;
    std::string sdreds = dreds? tens_in_words(dreds) + " hundred" : "";
    std::string stens  = tens? tens_in_words(tens) : "";
    return join_strings(sdreds, stens);
}

std::string number_in_words(unsigned num, int pow)
{
    if (pow == 12)
        return number_in_words(num, 0) + pow_in_words(pow);

    unsigned thousands = num % 1000, rest = num / 1000;
    if (!rest && !thousands)
        return ("zero");

    return join_strings((rest? number_in_words(rest, pow + 3) : ""),
                        (thousands? hundreds_in_words(thousands)
                                    + pow_in_words(pow)
                                  : ""));
}

std::string replace_all(std::string s,
                        const std::string &find,
                        const std::string &repl)
{
    ASSERT(!find.empty());
    std::string::size_type start = 0;
    std::string::size_type found;

    while ((found = s.find(find, start)) != std::string::npos)
    {
        s.replace(found, find.length(), repl);
        start = found + repl.length();
    }

    return (s);
}

// Replaces all occurrences of any of the characters in tofind with the
// replacement string.
std::string replace_all_of(std::string s,
                           const std::string &tofind,
                           const std::string &replacement)
{
    ASSERT(!tofind.empty());
    std::string::size_type start = 0;
    std::string::size_type found;

    while ((found = s.find_first_of(tofind, start)) != std::string::npos)
    {
        s.replace(found, 1, replacement);
        start = found + replacement.length();
    }

    return (s);
}

int count_occurrences(const std::string &text, const std::string &s)
{
    ASSERT(!s.empty());
    int nfound = 0;
    std::string::size_type pos = 0;

    while ((pos = text.find(s, pos)) != std::string::npos)
    {
        ++nfound;
        pos += s.length();
    }

    return (nfound);
}

std::string trimmed_string(std::string s)
{
    trim_string(s);
    return (s);
}

// also used with macros
std::string &trim_string(std::string &str)
{
    str.erase(0, str.find_first_not_of(" \t\n\r"));
    str.erase(str.find_last_not_of(" \t\n\r") + 1);

    return (str);
}

std::string &trim_string_right(std::string &str)
{
    str.erase(str.find_last_not_of(" \t\n\r") + 1);
    return (str);
}

static void add_segment(std::vector<std::string> &segs,
                         std::string s,
                         bool trim,
                         bool accept_empty)
{
    if (trim && !s.empty())
        trim_string(s);

    if (accept_empty || !s.empty())
        segs.push_back(s);
}

std::vector<std::string> split_string(const std::string &sep,
                                       std::string s,
                                       bool trim_segments,
                                       bool accept_empty_segments,
                                       int nsplits)
{
    std::vector<std::string> segments;
    int separator_length = sep.length();

    std::string::size_type pos;
    while (nsplits && (pos = s.find(sep)) != std::string::npos)
    {
        add_segment(segments, s.substr(0, pos),
                    trim_segments, accept_empty_segments);

        s.erase(0, pos + separator_length);

        if (nsplits > 0)
            --nsplits;
    }

    if (!s.empty())
        add_segment(segments, s, trim_segments, accept_empty_segments);

    return segments;
}

static const std::string _get_indent(const std::string &s)
{
    size_t prefix = 0;
    if (starts_with(s, "\"")    // ASCII quotes
        || starts_with(s, "“")  // English quotes
        || starts_with(s, "„")  // Polish/German/... quotes
        || starts_with(s, "«")  // French quotes
        || starts_with(s, "»")  // Danish/... quotes
        || starts_with(s, "•")) // bulleted lists
    {
        prefix = 1;
    }
    else if (starts_with(s, "「"))  // Chinese/Japanese quotes
        prefix = 2;

    size_t nspaces = s.find_first_not_of(' ', prefix);
    if (nspaces == std::string::npos)
        nspaces = 0;
    if (!(prefix += nspaces))
        return "";
    return std::string(prefix, ' ');
}

// The provided string is consumed!
std::string wordwrap_line(std::string &s, int width, bool tags, bool indent)
{
    const char *cp0 = s.c_str();
    const char *cp = cp0, *space = 0;
    ucs_t c;

    while (int clen = utf8towc(&c, cp))
    {
        int cw = wcwidth(c);
        if (c == ' ')
            space = cp;
        else if (c == '\n')
        {
            space = cp;
            break;
        }
        if (c == '<' && tags)
        {
            ASSERT(cw == 1);
            if (cp[1] == '<') // "<<" escape
            {
                // Note: this must be after a possible wrap, otherwise we could
                // split the escape between lines.
                cp++;
            }
            else
            {
                cw = 0;
                // Skip the whole tag.
                while (*cp != '>')
                {
                    if (!*cp)
                    {
                        // Everything so far fitted, report error.
                        std::string ret = s + ">";
                        s = "<lightred>ERROR: string above had unterminated tag</lightred>";
                        return ret;
                    }
                    cp++;
                }
            }
        }

        if (cw > width)
            break;

        if (cw >= 0)
            width -= cw;
        cp += clen;
    }

    if (!c)
    {
        // everything fits
        std::string ret = s;
        s.clear();
        return ret;
    }

    if (space)
        cp = space;
    const std::string ret = s.substr(0, cp - cp0);

    const std::string indentation = (indent && c != '\n') ? _get_indent(s) : "";

    // eat all trailing spaces and up to one newline
    while (*cp == ' ')
        cp++;
    if (*cp == '\n')
        cp++;
    s.erase(0, cp - cp0);

    // if we had to break a line, reinsert the indendation
    if (indent && c != '\n')
        s = indentation + s;

    return ret;
}

/**
 * Compare two strings, sorting integer numeric parts according to their value.
 *
 * "foo123bar" > "foo99bar"
 * "0.10" > "0.9" (version sort)
 *
 * @param a String one.
 * @param b String two.
 * @param limit If passed, comparison ends after X numeric parts.
 * @return As in strcmp().
**/
int numcmp(const char *a, const char *b, int limit)
{
    int res;

not_numeric:
    while (*a && *a == *b && !isadigit(*a))
    {
        a++;
        b++;
    }
    if (!a && !b)
        return 0;
    if (!isadigit(*a) || !isadigit(*b))
        return (*a < *b) ? -1 : (*a > *b) ? 1 : 0;
    while (*a == '0')
        a++;
    while (*b == '0')
        b++;
    res = 0;
    while (isadigit(*a))
    {
        if (!isadigit(*b))
            return 1;
        if (*a != *b && !res)
            res = (*a < *b) ? -1 : 1;
        a++;
        b++;
    }
    if (isadigit(*b))
        return -1;
    if (res)
        return res;

    if (--limit)
        goto not_numeric;
    return 0;
}

// make STL sort happy
bool numcmpstr(const std::string a, const std::string b)
{
    return numcmp(a.c_str(), b.c_str()) == -1;
}

bool version_is_stable(const char *v)
{
    // vulnerable to changes in the versioning scheme
    for (;; v++)
    {
        if (*v == '.' || isadigit(*v))
            continue;
        if (*v == '-')
            return isadigit(v[1]);
        return true;
    }
}

#ifndef USE_TILE_LOCAL
static coord_def _cgettopleft(GotoRegion region)
{
    switch (region)
    {
    case GOTO_MLIST:
        return crawl_view.mlistp;
    case GOTO_STAT:
        return crawl_view.hudp;
    case GOTO_MSG:
        return crawl_view.msgp;
    case GOTO_DNGN:
        return crawl_view.viewp;
    case GOTO_CRT:
    default:
        return crawl_view.termp;
    }
}

coord_def cgetpos(GotoRegion region)
{
    const coord_def where = coord_def(wherex(), wherey());
    return (where - _cgettopleft(region) + coord_def(1, 1));
}

static GotoRegion _current_region = GOTO_CRT;

void cgotoxy(int x, int y, GotoRegion region)
{
    _current_region = region;
    const coord_def tl = _cgettopleft(region);
    const coord_def sz = cgetsize(region);

#ifdef ASSERTS
    if (x < 1 || y < 1 || x > sz.x || y > sz.y)
    {
        save_game(false); // should be safe
        die("screen write out of bounds: (%d,%d) into (%d,%d)", x, y,
            sz.x, sz.y);
    }
#endif

    gotoxy_sys(tl.x + x - 1, tl.y + y - 1);

#ifdef USE_TILE_WEB
    tiles.cgotoxy(x, y, region);
#endif
}

GotoRegion get_cursor_region()
{
    return (_current_region);
}
#endif // USE_TILE_LOCAL

coord_def cgetsize(GotoRegion region)
{
    switch (region)
    {
    case GOTO_MLIST:
        return crawl_view.mlistsz;
    case GOTO_STAT:
        return crawl_view.hudsz;
    case GOTO_MSG:
        return crawl_view.msgsz;
    case GOTO_DNGN:
        return crawl_view.viewsz;
    case GOTO_CRT:
    default:
        return crawl_view.termsz;
    }
}

void cscroll(int n, GotoRegion region)
{
    // only implemented for the message window right now
    if (region == GOTO_MSG)
        scroll_message_window(n);
}


mouse_mode mouse_control::ms_current_mode = MOUSE_MODE_NORMAL;

size_t strlcpy(char *dst, const char *src, size_t n)
{
    if (!n)
        return strlen(src);

    const char *s = src;

    while (--n > 0)
        if (!(*dst++ = *s++))
            break;

    if (!n)
    {
        *dst++ = 0;
        while (*s++)
            ;
    }

    return s - src - 1;
}

std::string unwrap_desc(std::string desc)
{
    // Don't append a newline to an empty description.
    if (desc == "")
        return "";

    trim_string_right(desc);

    // An empty line separates paragraphs.
    desc = replace_all(desc, "\n\n", "\\n\\n");
    // Indented lines are pre-formatted.
    desc = replace_all(desc, "\n ", "\\n ");

    // Newlines are still whitespace.
    desc = replace_all(desc, "\n", " ");
    // Can force a newline with a literal "\n".
    desc = replace_all(desc, "\\n", "\n");

    return desc + "\n";
}

#ifdef TARGET_OS_WINDOWS
// FIXME: This function should detect if aero is running, but the DwmIsCompositionEnabled
// function isn't included in msys, so I don't know how to do that. Instead, I just check
// if we are running vista or higher. -rla
static bool _is_aero()
{
    OSVERSIONINFOEX osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (GetVersionEx((OSVERSIONINFO *) &osvi))
        return (osvi.dwMajorVersion >= 6);
    else
        return false;
}

taskbar_pos get_taskbar_pos()
{
    RECT rect;
    HWND taskbar = FindWindow("Shell_traywnd", NULL);
    if (taskbar && GetWindowRect(taskbar, &rect))
    {
        if (rect.right - rect.left > rect.bottom - rect.top)
        {
            if (rect.top > 0)
                return TASKBAR_BOTTOM;
            else
                return TASKBAR_TOP;
        }
        else
        {
            if (rect.left > 0)
                return TASKBAR_RIGHT;
            else
                return TASKBAR_LEFT;
        }
    }
    return TASKBAR_NO;
}

int get_taskbar_size()
{
    RECT rect;
    int size;
    taskbar_pos tpos = get_taskbar_pos();
    HWND taskbar = FindWindow("Shell_traywnd", NULL);

    if (taskbar && GetWindowRect(taskbar, &rect))
    {
        if (tpos & TASKBAR_H)
                size = rect.bottom - rect.top;
        else if (tpos & TASKBAR_V)
                size = rect.right - rect.left;
        else
            return 0;

        if (_is_aero())
            size += 3; // Taskbar behave strangely when aero is active.

        return size;
    }
    return 0;
}

static BOOL WINAPI console_handler(DWORD sig)
{
    switch (sig)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        return true; // block the signal
    default:
        return false;
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        if (crawl_state.seen_hups++)
            return true;

        sighup_save_and_exit();
        return true;
    }
}

void init_signals()
{
    // If there's no console, this will return an error, which we ignore.
    // For GUI programs there's no controlling terminal, but there's no hurt in
    // blindly trying -- this way, we support Cygwin.
    SetConsoleCtrlHandler(console_handler, true);
}

#else

/* [ds] This SIGHUP handling is primitive and far from safe, but it
 * should be better than nothing. Feel free to get rigorous on this.
 */
static void handle_hangup(int)
{
    if (crawl_state.seen_hups++)
        return;

#ifdef USE_TILE_LOCAL
    // XXX: Will a tiles build ever need to handle the HUP signal?
    sighup_save_and_exit();
#elif defined(USE_CURSES)
    // When using Curses, closing stdin will cause any Curses call blocking
    // on key-presses to immediately return, including any call that was
    // still blocking in the main thread when the HUP signal was caught.
    // This should guarantee that the main thread will un-stall and
    // will eventually return to _input() in main.cc, which will then
    // call sighup_save_and_exit().
    //
    // The point to all this is that if a user is playing a game on a
    // remote server and disconnects at a --more-- prompt, that when
    // the player reconnects the code behind the more() call will execute
    // before the disconnected game is saved, thus (for example) preventing
    // the hack of avoiding excomunication consesquences because of the
    // more() after "You have lost your religion!"
    fclose(stdin);
#else
     #error "Must use either Curses or tiles on Unix"
#endif
}

void init_signals()
{
#ifdef DGAMELAUNCH
    // Force timezone to UTC.
    setenv("TZ", "", 1);
    tzset();
#endif

#ifdef USE_UNIX_SIGNALS
#ifdef SIGQUIT
    signal(SIGQUIT, SIG_IGN);
#endif

#ifdef SIGINT
    signal(SIGINT, SIG_IGN);
#endif

    signal(SIGHUP, handle_hangup);
#endif

#ifdef DGL_ENABLE_CORE_DUMP
    rlimit lim;
    if (!getrlimit(RLIMIT_CORE, &lim))
    {
        lim.rlim_cur = RLIM_INFINITY;
        setrlimit(RLIMIT_CORE, &lim);
    }
#endif
}

#endif
