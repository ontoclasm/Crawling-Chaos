#ifndef AREAS_H
#define AREAS_H

enum area_centre_type
{
    AREA_NONE,
    AREA_SANCTUARY,
    AREA_SILENCE,
    AREA_HALO,
    AREA_LIQUID,
    AREA_ORB,
    AREA_ANTIHALO,
};

void invalidate_agrid(bool recheck_new = false);

class actor;
void areas_actor_moved(const actor* act, const coord_def& oldpos);

void create_sanctuary(const coord_def& center, int time);
bool remove_sanctuary(bool did_attack = false);
void decrease_sanctuary_radius();

coord_def find_centre_for (const coord_def& f, area_centre_type at = AREA_NONE);

bool silenced(const coord_def& p);

// Does the given point lie within a halo?
bool haloed(const coord_def& p);

// or is the ground there liquified?
bool liquefied(const coord_def& p, bool check_actual = true);

// Is it enlightened by the orb?
bool orb_haloed(const coord_def& p);

// ...or endarkened by an antihalo?
bool antihaloed(const coord_def& p);

#endif
