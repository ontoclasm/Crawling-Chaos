/*
 *  File: version.h
 *  Summary: Contains version information
 *  Written by: ??
 *
 *  Modified for Crawl Reference by $Author$ on $Date$
 *
 *  Change History (most recent first):
 *
 *     <2>     10/12/99    BCR     Added BUILD_DATE #define
 *     <1>     -/--/--     ---     Created
 */

/* Crawl versioning:
 * Crawl uses three numbers to determine the version:
 * Version, which changes when the dev team makes enormous overhauls
 *    to the game (which may cause savefiles from previous versions to
 *    temporarily stop working, for example)
 * Release, which changes when siginficant new features have been
 *    added to the game.
 * Mod, which changes with every publicly released version that
 *    contains nothing more than bug fixes, cosmetic changes,
 *    internal cleanup, etc.
 *
 * Further, any source or binary uploaded anywhere that is _not_ of
 * release quality should be labelled as such:
 * alpha for potentially unstable dev versions, or
 * beta for feature-complete and mostly balanced versions
 *
 * several alphas or betas in a row should be labelled incrementally;
 * alpha1 -> alpha2 -> alpha3 -> beta1 -> beta2 -> ...
 */


#ifndef VERSION_H
#define VERSION_H

#define CRAWL "Dungeon Crawl Stone Soup"

#define VER_NUM  "0.3.2"
#define VER_QUAL ""

// last updated 07august2001 {mv}
/* ***********************************************************************
 * called from: chardump - command - newgame
 * *********************************************************************** */
#define VERSION  VER_NUM VER_QUAL " (crawl-ref)"

// last updated 20feb2001 {GDL}
/* ***********************************************************************
 * called from: command
 * *********************************************************************** */
#define BUILD_DATE __DATE__

#ifdef BUILD_REVISION
#define VERSION_DETAIL "r" BUILD_REVISION "/" BUILD_DATE
#else
#define VERSION_DETAIL BUILD_DATE
#endif

#define SAVE_MAJOR_VERSION 3

#endif
