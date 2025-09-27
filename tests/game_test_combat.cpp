#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../src/libft_math_bridge.hpp"
#include "fleets.hpp"

#define private public
#define protected public
#include "combat.hpp"
#include "game.hpp"
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

int verify_raider_ship_maneuver_profiles()
{
    ft_fleet fleet(3021);

    int corvette_id = fleet.create_ship(SHIP_RAIDER_CORVETTE);
    int destroyer_id = fleet.create_ship(SHIP_RAIDER_DESTROYER);
    int battleship_id = fleet.create_ship(SHIP_RAIDER_BATTLESHIP);

    const ft_ship *corvette = fleet.get_ship(corvette_id);
    const ft_ship *destroyer = fleet.get_ship(destroyer_id);
    const ft_ship *battleship = fleet.get_ship(battleship_id);

    FT_ASSERT(corvette != ft_nullptr);
    FT_ASSERT(destroyer != ft_nullptr);
    FT_ASSERT(battleship != ft_nullptr);

    FT_ASSERT(corvette->deceleration > destroyer->deceleration);
    FT_ASSERT(destroyer->deceleration > battleship->deceleration);

    FT_ASSERT(corvette->turn_speed > destroyer->turn_speed);
    FT_ASSERT(destroyer->turn_speed > battleship->turn_speed);

    FT_ASSERT(corvette->max_speed > destroyer->max_speed);
    FT_ASSERT(destroyer->max_speed >= battleship->max_speed);

    return 1;
}

int verify_combat_tracker_deceleration()
{
    CombatManager manager;
    CombatManager::ft_combat_encounter encounter;
    encounter.defender_operational_ships = 1;
    encounter.raider_operational_ships = 1;
    encounter.defender_line_ships = 1;
    encounter.raider_aggression = 1.0;
    encounter.defender_line = -60.0;

    CombatManager::ft_ship_tracker tracker;
    tracker.base_preferred_radius = 30.0;
    tracker.base_advance_bias = 0.0;
    tracker.base_flank = false;
    tracker.preferred_radius = 30.0;
    tracker.advance_bias = 0.0;
    tracker.flank = false;
    tracker.max_speed = 10.0;
    tracker.acceleration = 2.0;
    tracker.deceleration = 3.0;
    tracker.turn_speed = 90.0;
    tracker.current_speed = 16.0;
    tracker.requires_escort = false;
    tracker.role = SHIP_ROLE_LINE;
    tracker.hp_ratio = 1.0;
    tracker.shield_ratio = 1.0;
    tracker.normal_behavior = SHIP_BEHAVIOR_LINE_HOLD;
    tracker.outnumbered_behavior = 0;
    tracker.unescorted_behavior = 0;
    tracker.low_hp_behavior = 0;
    tracker.heading_x = 0.0;
    tracker.heading_y = 0.0;
    tracker.heading_z = 1.0;
    tracker.drift_speed = 0.0;
    tracker.drift_origin = 0.0;
    tracker.lane_offset = 0.0;
    tracker.vertical_layer = 0.0;
    tracker.spatial.x = 0.0;
    tracker.spatial.y = 0.0;
    tracker.spatial.z = 0.0;

    encounter.defender_tracks.insert(1, tracker);

    manager.update_tracks(encounter.defender_tracks, encounter, 1.0, false, false);
    Pair<int, CombatManager::ft_ship_tracker> *first = encounter.defender_tracks.find(1);
    FT_ASSERT(first != ft_nullptr);
    FT_ASSERT(first->value.current_speed > 12.9);
    FT_ASSERT(first->value.current_speed < 13.1);

    manager.update_tracks(encounter.defender_tracks, encounter, 1.0, false, false);
    Pair<int, CombatManager::ft_ship_tracker> *second = encounter.defender_tracks.find(1);
    FT_ASSERT(second != ft_nullptr);
    FT_ASSERT(second->value.current_speed > 9.9);
    FT_ASSERT(second->value.current_speed < 10.1);

    manager.update_tracks(encounter.defender_tracks, encounter, 1.0, false, false);
    Pair<int, CombatManager::ft_ship_tracker> *third = encounter.defender_tracks.find(1);
    FT_ASSERT(third != ft_nullptr);
    FT_ASSERT(third->value.current_speed > 8.4);
    FT_ASSERT(third->value.current_speed < 8.6);

    return 1;
}

int verify_raider_ships_unavailable_to_players()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    int fleet_id = 6021;
    game.create_fleet(fleet_id);

    int corvette_id = game.create_ship(fleet_id, SHIP_RAIDER_CORVETTE);
    int destroyer_id = game.create_ship(fleet_id, SHIP_RAIDER_DESTROYER);
    int battleship_id = game.create_ship(fleet_id, SHIP_RAIDER_BATTLESHIP);

    FT_ASSERT(corvette_id == 0);
    FT_ASSERT(destroyer_id == 0);
    FT_ASSERT(battleship_id == 0);

    ft_sharedptr<ft_fleet> fleet = game.get_fleet(fleet_id);
    FT_ASSERT(fleet);
    FT_ASSERT(fleet->get_ship_count() == 0);

    return 1;
}
