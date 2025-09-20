#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../src/libft_math_bridge.hpp"
#include "fleets.hpp"

#define private public
#define protected public
#include "combat.hpp"
#undef private
#undef protected

#include "game_test_scenarios.hpp"

int verify_ship_range_defaults()
{
    ft_fleet fleet(101);

    int shield_id = fleet.create_ship(SHIP_SHIELD);
    const ft_ship *shield = fleet.get_ship(shield_id);
    FT_ASSERT(shield != ft_nullptr);
    FT_ASSERT(math_fabs(shield->optimal_range - 205.0) < 1e-6);
    FT_ASSERT(math_fabs(shield->max_range - 265.0) < 1e-6);
    FT_ASSERT(math_fabs(shield->base_damage - 5.5) < 1e-6);

    int radar_id = fleet.create_ship(SHIP_RADAR);
    const ft_ship *radar = fleet.get_ship(radar_id);
    FT_ASSERT(radar != ft_nullptr);
    FT_ASSERT(math_fabs(radar->optimal_range - 235.0) < 1e-6);
    FT_ASSERT(math_fabs(radar->max_range - 320.0) < 1e-6);
    FT_ASSERT(math_fabs(radar->base_damage - 4.5) < 1e-6);

    int corvette_id = fleet.create_ship(SHIP_CORVETTE);
    const ft_ship *corvette = fleet.get_ship(corvette_id);
    FT_ASSERT(corvette != ft_nullptr);
    FT_ASSERT(math_fabs(corvette->optimal_range - 190.0) < 1e-6);
    FT_ASSERT(math_fabs(corvette->max_range - 235.0) < 1e-6);
    FT_ASSERT(math_fabs(corvette->base_damage - 8.0) < 1e-6);

    return 1;
}

static void prepare_tracker(CombatManager::ft_ship_tracker &tracker,
    double base_damage, double optimal, double maximum, double hp_ratio,
    double spatial_z)
{
    tracker.base_damage = base_damage;
    tracker.optimal_range = optimal;
    tracker.max_range = maximum;
    tracker.hp_ratio = hp_ratio;
    tracker.spatial.z = spatial_z;
}

int verify_range_aware_combat_power()
{
    CombatManager manager;
    CombatManager::ft_combat_encounter encounter;
    encounter.raider_frontline = 150.0;
    encounter.defender_line = -60.0;

    CombatManager::ft_ship_tracker tracker;

    prepare_tracker(tracker, 12.0, 100.0, 140.0, 1.0,
        encounter.raider_frontline - 80.0);
    encounter.defender_tracks.clear();
    encounter.defender_tracks.insert(1, tracker);
    double in_range = manager.calculate_player_power(encounter);
    FT_ASSERT(math_fabs(in_range - 12.0) < 1e-6);

    prepare_tracker(tracker, 12.0, 100.0, 140.0, 1.0,
        encounter.raider_frontline - 120.0);
    encounter.defender_tracks.clear();
    encounter.defender_tracks.insert(2, tracker);
    double falloff = manager.calculate_player_power(encounter);
    FT_ASSERT(math_fabs(falloff - 6.0) < 1e-6);

    prepare_tracker(tracker, 12.0, 100.0, 140.0, 1.0,
        encounter.raider_frontline - 150.0);
    encounter.defender_tracks.clear();
    encounter.defender_tracks.insert(3, tracker);
    double out_of_range = manager.calculate_player_power(encounter);
    FT_ASSERT(math_fabs(out_of_range - 0.0) < 1e-6);

    prepare_tracker(tracker, 20.0, 80.0, 120.0, 0.05,
        encounter.raider_frontline - 60.0);
    encounter.defender_tracks.clear();
    encounter.defender_tracks.insert(4, tracker);
    double low_hp = manager.calculate_player_power(encounter);
    FT_ASSERT(low_hp > 1.9);
    FT_ASSERT(low_hp < 2.1);

    CombatManager::ft_ship_tracker raider_tracker;
    prepare_tracker(raider_tracker, 9.0, 70.0, 110.0, 1.0,
        encounter.defender_line + 60.0);
    encounter.raider_tracks.clear();
    encounter.raider_tracks.insert(7, raider_tracker);
    double raider_in_range = manager.calculate_raider_power(encounter);
    FT_ASSERT(math_fabs(raider_in_range - 9.0) < 1e-6);

    prepare_tracker(raider_tracker, 9.0, 70.0, 110.0, 1.0,
        encounter.defender_line + 90.0);
    encounter.raider_tracks.clear();
    encounter.raider_tracks.insert(8, raider_tracker);
    double raider_falloff = manager.calculate_raider_power(encounter);
    FT_ASSERT(math_fabs(raider_falloff - 4.5) < 1e-6);

    prepare_tracker(raider_tracker, 9.0, 70.0, 110.0, 1.0,
        encounter.defender_line + 130.0);
    encounter.raider_tracks.clear();
    encounter.raider_tracks.insert(9, raider_tracker);
    double raider_out = manager.calculate_raider_power(encounter);
    FT_ASSERT(math_fabs(raider_out - 0.0) < 1e-6);

    prepare_tracker(raider_tracker, 16.0, 90.0, 130.0, 0.02,
        encounter.defender_line + 70.0);
    encounter.raider_tracks.clear();
    encounter.raider_tracks.insert(10, raider_tracker);
    double raider_low_hp = manager.calculate_raider_power(encounter);
    FT_ASSERT(raider_low_hp > 1.5);
    FT_ASSERT(raider_low_hp < 1.7);

    return 1;
}
