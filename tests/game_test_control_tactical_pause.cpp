#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft_math_bridge.hpp"
#include "fleets.hpp"
#include "buildings.hpp"
#include "research.hpp"
#include "game_test_scenarios.hpp"

int validate_tactical_pause_behaviors()
{
    Game pause_baseline(ft_string("127.0.0.1:8080"), ft_string("/"));
    pause_baseline.set_ore(PLANET_TERRA, ORE_IRON, 200);
    pause_baseline.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    pause_baseline.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(pause_baseline.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    pause_baseline.tick(0.0);
    pause_baseline.create_fleet(1);
    int pause_baseline_capital = pause_baseline.create_ship(1, SHIP_CAPITAL);
    pause_baseline.set_ship_hp(1, pause_baseline_capital, 220);
    pause_baseline.set_ship_shield(1, pause_baseline_capital, 100);
    pause_baseline.create_fleet(2);
    int pause_baseline_guard = pause_baseline.create_ship(2, SHIP_SHIELD);
    pause_baseline.set_ship_hp(2, pause_baseline_guard, 140);
    pause_baseline.set_ship_shield(2, pause_baseline_guard, 80);
    FT_ASSERT(pause_baseline.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(pause_baseline.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(pause_baseline.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(pause_baseline.set_assault_support(PLANET_TERRA, false, false, false));
    pause_baseline.tick(2.0);
    int pause_baseline_hp = pause_baseline.get_ship_hp(1, pause_baseline_capital);
    int pause_baseline_shield = pause_baseline.get_ship_shield(1, pause_baseline_capital);

    Game pause_active(ft_string("127.0.0.1:8080"), ft_string("/"));
    pause_active.set_ore(PLANET_TERRA, ORE_IRON, 200);
    pause_active.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    pause_active.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(pause_active.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    pause_active.tick(0.0);
    pause_active.create_fleet(1);
    int pause_active_capital = pause_active.create_ship(1, SHIP_CAPITAL);
    pause_active.set_ship_hp(1, pause_active_capital, 220);
    pause_active.set_ship_shield(1, pause_active_capital, 100);
    pause_active.create_fleet(2);
    int pause_active_guard = pause_active.create_ship(2, SHIP_SHIELD);
    pause_active.set_ship_hp(2, pause_active_guard, 140);
    pause_active.set_ship_shield(2, pause_active_guard, 80);
    FT_ASSERT(pause_active.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(pause_active.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(pause_active.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(pause_active.set_assault_support(PLANET_TERRA, false, false, false));
    FT_ASSERT(pause_active.request_assault_tactical_pause(PLANET_TERRA));
    pause_active.tick(2.0);
    int pause_active_hp = pause_active.get_ship_hp(1, pause_active_capital);
    int pause_active_shield = pause_active.get_ship_shield(1, pause_active_capital);
    FT_ASSERT(pause_active_hp >= pause_baseline_hp);
    FT_ASSERT(pause_active_shield > pause_baseline_shield);

    Game pause_cooldown_balanced(ft_string("127.0.0.1:8080"), ft_string("/"));
    pause_cooldown_balanced.set_ore(PLANET_TERRA, ORE_IRON, 240);
    pause_cooldown_balanced.set_ore(PLANET_TERRA, ORE_COPPER, 240);
    pause_cooldown_balanced.set_ore(PLANET_TERRA, ORE_COAL, 240);
    FT_ASSERT(pause_cooldown_balanced.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    pause_cooldown_balanced.tick(0.0);
    double pause_balanced_pressure = pause_cooldown_balanced.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(pause_balanced_pressure < 0.1);
    pause_cooldown_balanced.create_fleet(1);
    int pause_balanced_guard = pause_cooldown_balanced.create_ship(1, SHIP_SHIELD);
    pause_cooldown_balanced.set_ship_hp(1, pause_balanced_guard, 20);
    pause_cooldown_balanced.set_ship_shield(1, pause_balanced_guard, 420);
    FT_ASSERT(pause_cooldown_balanced.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(pause_cooldown_balanced.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(pause_cooldown_balanced.request_assault_tactical_pause(PLANET_TERRA));
    pause_cooldown_balanced.tick(2.0);
    FT_ASSERT(pause_cooldown_balanced.is_assault_active(PLANET_TERRA));
    FT_ASSERT(!pause_cooldown_balanced.request_assault_tactical_pause(PLANET_TERRA));

    Game pause_cooldown_stressed(ft_string("127.0.0.1:8080"), ft_string("/"));
    pause_cooldown_stressed.set_ore(PLANET_TERRA, ORE_IRON, 280);
    pause_cooldown_stressed.set_ore(PLANET_TERRA, ORE_COPPER, 280);
    pause_cooldown_stressed.set_ore(PLANET_TERRA, ORE_COAL, 280);
    FT_ASSERT(pause_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_TRANSFER_NODE, 0, 3) != 0);
    FT_ASSERT(pause_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 0) != 0);
    FT_ASSERT(pause_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 2) != 0);
    FT_ASSERT(pause_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_PROCESSOR, 0, 1) != 0);
    pause_cooldown_stressed.tick(0.0);
    double pause_stressed_pressure = pause_cooldown_stressed.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(pause_stressed_pressure > 6.4);
    pause_cooldown_stressed.create_fleet(1);
    int pause_stressed_guard = pause_cooldown_stressed.create_ship(1, SHIP_SHIELD);
    pause_cooldown_stressed.set_ship_hp(1, pause_stressed_guard, 20);
    pause_cooldown_stressed.set_ship_shield(1, pause_stressed_guard, 420);
    FT_ASSERT(pause_cooldown_stressed.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(pause_cooldown_stressed.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(pause_cooldown_stressed.request_assault_tactical_pause(PLANET_TERRA));
    pause_cooldown_stressed.tick(2.0);
    FT_ASSERT(pause_cooldown_stressed.is_assault_active(PLANET_TERRA));
    FT_ASSERT(!pause_cooldown_stressed.request_assault_tactical_pause(PLANET_TERRA));

    pause_cooldown_balanced.tick(15.8);
    pause_cooldown_stressed.tick(15.8);
    FT_ASSERT(pause_cooldown_balanced.is_assault_active(PLANET_TERRA));
    FT_ASSERT(pause_cooldown_stressed.is_assault_active(PLANET_TERRA));
    FT_ASSERT(pause_cooldown_stressed.request_assault_tactical_pause(PLANET_TERRA));
    FT_ASSERT(!pause_cooldown_balanced.request_assault_tactical_pause(PLANET_TERRA));
    pause_cooldown_balanced.tick(3.0);
    FT_ASSERT(pause_cooldown_balanced.is_assault_active(PLANET_TERRA));
    FT_ASSERT(pause_cooldown_balanced.request_assault_tactical_pause(PLANET_TERRA));
    return 1;
}
