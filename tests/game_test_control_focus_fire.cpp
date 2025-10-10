#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft_math_bridge.hpp"
#include "fleets.hpp"
#include "buildings.hpp"
#include "research.hpp"
#include "game_test_scenarios.hpp"

int evaluate_focus_fire_cooldowns()
{
    Game focus_baseline(ft_string("127.0.0.1:8080"), ft_string("/"));
    focus_baseline.set_ore(PLANET_TERRA, ORE_IRON, 200);
    focus_baseline.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    focus_baseline.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(focus_baseline.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    focus_baseline.tick(0.0);
    focus_baseline.create_fleet(1);
    int focus_baseline_capital = focus_baseline.create_ship(1, SHIP_CAPITAL);
    focus_baseline.set_ship_hp(1, focus_baseline_capital, 220);
    focus_baseline.set_ship_shield(1, focus_baseline_capital, 100);
    focus_baseline.create_fleet(2);
    int focus_baseline_guard = focus_baseline.create_ship(2, SHIP_SHIELD);
    focus_baseline.set_ship_hp(2, focus_baseline_guard, 140);
    focus_baseline.set_ship_shield(2, focus_baseline_guard, 80);
    FT_ASSERT(focus_baseline.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    double baseline_focus_shield_start = focus_baseline.get_assault_raider_shield(PLANET_TERRA);
    double baseline_focus_hull_start = focus_baseline.get_assault_raider_hull(PLANET_TERRA);
    FT_ASSERT(focus_baseline.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(focus_baseline.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(focus_baseline.set_assault_support(PLANET_TERRA, false, false, false));
    focus_baseline.tick(1.0);
    double baseline_focus_shield_end = focus_baseline.get_assault_raider_shield(PLANET_TERRA);
    double baseline_focus_hull_end = focus_baseline.get_assault_raider_hull(PLANET_TERRA);
    double baseline_focus_shield_delta = baseline_focus_shield_start - baseline_focus_shield_end;
    double baseline_focus_hull_delta = baseline_focus_hull_start - baseline_focus_hull_end;
    FT_ASSERT(baseline_focus_shield_delta > 0.0);

    Game focus_burst(ft_string("127.0.0.1:8080"), ft_string("/"));
    focus_burst.set_ore(PLANET_TERRA, ORE_IRON, 200);
    focus_burst.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    focus_burst.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(focus_burst.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    focus_burst.tick(0.0);
    focus_burst.create_fleet(1);
    int focus_burst_capital = focus_burst.create_ship(1, SHIP_CAPITAL);
    focus_burst.set_ship_hp(1, focus_burst_capital, 220);
    focus_burst.set_ship_shield(1, focus_burst_capital, 100);
    focus_burst.create_fleet(2);
    int focus_burst_guard = focus_burst.create_ship(2, SHIP_SHIELD);
    focus_burst.set_ship_hp(2, focus_burst_guard, 140);
    focus_burst.set_ship_shield(2, focus_burst_guard, 80);
    FT_ASSERT(focus_burst.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    double burst_focus_shield_start = focus_burst.get_assault_raider_shield(PLANET_TERRA);
    double burst_focus_hull_start = focus_burst.get_assault_raider_hull(PLANET_TERRA);
    FT_ASSERT(math_fabs(burst_focus_shield_start - baseline_focus_shield_start) < 0.01);
    FT_ASSERT(math_fabs(burst_focus_hull_start - baseline_focus_hull_start) < 0.01);
    FT_ASSERT(focus_burst.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(focus_burst.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(focus_burst.set_assault_support(PLANET_TERRA, false, false, false));
    FT_ASSERT(focus_burst.trigger_assault_focus_fire(PLANET_TERRA));
    focus_burst.tick(1.0);
    double burst_focus_shield_end = focus_burst.get_assault_raider_shield(PLANET_TERRA);
    double burst_focus_hull_end = focus_burst.get_assault_raider_hull(PLANET_TERRA);
    double burst_focus_shield_delta = burst_focus_shield_start - burst_focus_shield_end;
    double burst_focus_hull_delta = burst_focus_hull_start - burst_focus_hull_end;
    FT_ASSERT(burst_focus_shield_delta > baseline_focus_shield_delta * 1.15);
    FT_ASSERT(burst_focus_hull_delta >= baseline_focus_hull_delta);

    Game focus_cooldown_balanced(ft_string("127.0.0.1:8080"), ft_string("/"));
    focus_cooldown_balanced.set_ore(PLANET_TERRA, ORE_IRON, 240);
    focus_cooldown_balanced.set_ore(PLANET_TERRA, ORE_COPPER, 240);
    focus_cooldown_balanced.set_ore(PLANET_TERRA, ORE_COAL, 240);
    FT_ASSERT(focus_cooldown_balanced.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    focus_cooldown_balanced.tick(0.0);
    double focus_balanced_pressure = focus_cooldown_balanced.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(focus_balanced_pressure < 0.1);
    focus_cooldown_balanced.create_fleet(1);
    int focus_balanced_guard = focus_cooldown_balanced.create_ship(1, SHIP_SHIELD);
    focus_cooldown_balanced.set_ship_hp(1, focus_balanced_guard, 20);
    focus_cooldown_balanced.set_ship_shield(1, focus_balanced_guard, 380);
    FT_ASSERT(focus_cooldown_balanced.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(focus_cooldown_balanced.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(focus_cooldown_balanced.trigger_assault_focus_fire(PLANET_TERRA));
    focus_cooldown_balanced.tick(4.0);
    FT_ASSERT(focus_cooldown_balanced.is_assault_active(PLANET_TERRA));
    FT_ASSERT(!focus_cooldown_balanced.trigger_assault_focus_fire(PLANET_TERRA));

    Game focus_cooldown_stressed(ft_string("127.0.0.1:8080"), ft_string("/"));
    focus_cooldown_stressed.set_ore(PLANET_TERRA, ORE_IRON, 280);
    focus_cooldown_stressed.set_ore(PLANET_TERRA, ORE_COPPER, 280);
    focus_cooldown_stressed.set_ore(PLANET_TERRA, ORE_COAL, 280);
    FT_ASSERT(focus_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_TRANSFER_NODE, 0, 3) != 0);
    FT_ASSERT(focus_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 0) != 0);
    FT_ASSERT(focus_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 2) != 0);
    FT_ASSERT(focus_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_PROCESSOR, 0, 1) != 0);
    focus_cooldown_stressed.tick(0.0);
    double focus_stressed_pressure = focus_cooldown_stressed.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(focus_stressed_pressure > 6.4);
    focus_cooldown_stressed.create_fleet(1);
    int focus_stressed_guard = focus_cooldown_stressed.create_ship(1, SHIP_SHIELD);
    focus_cooldown_stressed.set_ship_hp(1, focus_stressed_guard, 20);
    focus_cooldown_stressed.set_ship_shield(1, focus_stressed_guard, 380);
    FT_ASSERT(focus_cooldown_stressed.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(focus_cooldown_stressed.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(focus_cooldown_stressed.trigger_assault_focus_fire(PLANET_TERRA));
    focus_cooldown_stressed.tick(4.0);
    FT_ASSERT(focus_cooldown_stressed.is_assault_active(PLANET_TERRA));
    FT_ASSERT(!focus_cooldown_stressed.trigger_assault_focus_fire(PLANET_TERRA));

    focus_cooldown_balanced.tick(5.8);
    focus_cooldown_stressed.tick(5.8);
    FT_ASSERT(focus_cooldown_balanced.is_assault_active(PLANET_TERRA));
    FT_ASSERT(focus_cooldown_stressed.is_assault_active(PLANET_TERRA));
    FT_ASSERT(focus_cooldown_stressed.trigger_assault_focus_fire(PLANET_TERRA));
    FT_ASSERT(!focus_cooldown_balanced.trigger_assault_focus_fire(PLANET_TERRA));
    focus_cooldown_balanced.tick(2.5);
    FT_ASSERT(focus_cooldown_balanced.is_assault_active(PLANET_TERRA));
    FT_ASSERT(focus_cooldown_balanced.trigger_assault_focus_fire(PLANET_TERRA));
    return 1;
}
