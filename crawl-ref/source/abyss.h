/**
 * @file
 * @brief Misc abyss specific functions.
**/


#ifndef ABYSS_H
#define ABYSS_H

// When shifting areas in the abyss, shift the square containing player LOS
// plus a little extra so that the player won't be disoriented by taking a
// step backward after an abyss shift.
const int ABYSS_AREA_SHIFT_RADIUS = LOS_RADIUS + 2;
extern const coord_def ABYSS_CENTRE;

struct abyss_state
{
    coord_def major_coord;
    double phase;
    double depth;
};
void abyss_morph(double duration);
void push_features_to_abyss();

void generate_abyss();
void maybe_shift_abyss_around_player();
void abyss_teleport(bool new_area);
void save_abyss_uniques();
bool is_level_incorruptible();
bool lugonu_corrupt_level(int power);
void run_corruption_effects(int duration);

#endif
