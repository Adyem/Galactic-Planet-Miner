#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "buildings.hpp"
#include "fleets.hpp"
#include "game_test_scenarios.hpp"
#include "research.hpp"

namespace
{
    int unlock_shield_support(Game &game)
    {
        game.ensure_planet_item_slot(PLANET_TERRA, ITEM_COPPER_BAR);
        game.ensure_planet_item_slot(PLANET_TERRA, ITEM_MITHRIL_BAR);
        game.ensure_planet_item_slot(PLANET_TERRA, ITEM_ACCUMULATOR);
        game.set_ore(PLANET_TERRA, ORE_IRON, 400);
        game.set_ore(PLANET_TERRA, ORE_COPPER, 400);
        game.set_ore(PLANET_TERRA, ORE_COAL, 400);
        game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 160);
        game.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 160);
        game.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 60);
        FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
        game.tick(60.0);
        FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_MARS));
        FT_ASSERT(game.start_research(RESEARCH_UNLOCK_ZALTHOR));
        game.tick(60.0);
        FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_ZALTHOR));
        FT_ASSERT(game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
        game.tick(60.0);
        FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_I));
        FT_ASSERT(game.start_research(RESEARCH_SHIELD_TECHNOLOGY));
        game.tick(60.0);
        FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_SHIELD_TECHNOLOGY));
        return 1;
    }
}

int compare_generator_support()
{
    Game generator_off(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(unlock_shield_support(generator_off));
    generator_off.set_ore(PLANET_TERRA, ORE_IRON, 320);
    generator_off.set_ore(PLANET_TERRA, ORE_COPPER, 320);
    generator_off.set_ore(PLANET_TERRA, ORE_COAL, 320);
    FT_ASSERT(generator_off.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    generator_off.tick(0.0);
    generator_off.create_fleet(1);
    int generator_off_capital = generator_off.create_ship(1, SHIP_CAPITAL);
    generator_off.set_ship_hp(1, generator_off_capital, 240);
    generator_off.set_ship_shield(1, generator_off_capital, 160);
    generator_off.sub_ship_shield(1, generator_off_capital, 120);
    generator_off.create_fleet(2);
    int generator_off_guard = generator_off.create_ship(2, SHIP_SHIELD);
    generator_off.set_ship_hp(2, generator_off_guard, 140);
    generator_off.set_ship_shield(2, generator_off_guard, 80);
    FT_ASSERT(generator_off.start_raider_assault(PLANET_TERRA, 0.8, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(generator_off.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(generator_off.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(generator_off.set_assault_support(PLANET_TERRA, false, false, false));
    FT_ASSERT(generator_off.set_assault_aggression(PLANET_TERRA, 0.2));
    int generator_off_start = generator_off.get_ship_shield(1, generator_off_capital);
    generator_off.tick(0.5);
    int generator_off_delta = generator_off.get_ship_shield(1, generator_off_capital) - generator_off_start;

    Game generator_stable(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(unlock_shield_support(generator_stable));
    generator_stable.set_ore(PLANET_TERRA, ORE_IRON, 320);
    generator_stable.set_ore(PLANET_TERRA, ORE_COPPER, 320);
    generator_stable.set_ore(PLANET_TERRA, ORE_COAL, 320);
    FT_ASSERT(generator_stable.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    generator_stable.tick(0.0);
    double stable_pressure = generator_stable.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(stable_pressure < 0.1);
    generator_stable.create_fleet(1);
    int generator_stable_capital = generator_stable.create_ship(1, SHIP_CAPITAL);
    generator_stable.set_ship_hp(1, generator_stable_capital, 240);
    generator_stable.set_ship_shield(1, generator_stable_capital, 160);
    generator_stable.sub_ship_shield(1, generator_stable_capital, 120);
    generator_stable.create_fleet(2);
    int generator_stable_guard = generator_stable.create_ship(2, SHIP_SHIELD);
    generator_stable.set_ship_hp(2, generator_stable_guard, 140);
    generator_stable.set_ship_shield(2, generator_stable_guard, 80);
    FT_ASSERT(generator_stable.start_raider_assault(PLANET_TERRA, 0.8, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(generator_stable.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(generator_stable.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(generator_stable.set_assault_support(PLANET_TERRA, false, false, true));
    FT_ASSERT(generator_stable.set_assault_aggression(PLANET_TERRA, 0.2));
    int generator_stable_start = generator_stable.get_ship_shield(1, generator_stable_capital);
    generator_stable.tick(0.5);
    int generator_stable_delta = generator_stable.get_ship_shield(1, generator_stable_capital) - generator_stable_start;

    Game generator_stressed(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(unlock_shield_support(generator_stressed));
    generator_stressed.set_ore(PLANET_TERRA, ORE_IRON, 320);
    generator_stressed.set_ore(PLANET_TERRA, ORE_COPPER, 320);
    generator_stressed.set_ore(PLANET_TERRA, ORE_COAL, 320);
    FT_ASSERT(generator_stressed.place_building(PLANET_TERRA, BUILDING_TRANSFER_NODE, 0, 3) != 0);
    FT_ASSERT(generator_stressed.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 0) != 0);
    FT_ASSERT(generator_stressed.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 2) != 0);
    FT_ASSERT(generator_stressed.place_building(PLANET_TERRA, BUILDING_PROCESSOR, 0, 1) != 0);
    FT_ASSERT(generator_stressed.place_building(PLANET_TERRA, BUILDING_SMELTER, 1, 2) != 0);
    generator_stressed.tick(0.0);
    double stressed_pressure = generator_stressed.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(stressed_pressure > 7.0);
    generator_stressed.create_fleet(1);
    int generator_stressed_capital = generator_stressed.create_ship(1, SHIP_CAPITAL);
    generator_stressed.set_ship_hp(1, generator_stressed_capital, 240);
    generator_stressed.set_ship_shield(1, generator_stressed_capital, 160);
    generator_stressed.sub_ship_shield(1, generator_stressed_capital, 120);
    generator_stressed.create_fleet(2);
    int generator_stressed_guard = generator_stressed.create_ship(2, SHIP_SHIELD);
    generator_stressed.set_ship_hp(2, generator_stressed_guard, 140);
    generator_stressed.set_ship_shield(2, generator_stressed_guard, 80);
    FT_ASSERT(generator_stressed.start_raider_assault(PLANET_TERRA, 0.8, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(generator_stressed.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(generator_stressed.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(generator_stressed.set_assault_support(PLANET_TERRA, false, false, true));
    FT_ASSERT(generator_stressed.set_assault_aggression(PLANET_TERRA, 0.2));
    int generator_stressed_start = generator_stressed.get_ship_shield(1, generator_stressed_capital);
    generator_stressed.tick(0.5);
    int generator_stressed_delta = generator_stressed.get_ship_shield(1, generator_stressed_capital) - generator_stressed_start;

    FT_ASSERT(generator_off_delta <= 0);
    FT_ASSERT(generator_stable_delta > 0);
    FT_ASSERT(generator_stable_delta > generator_off_delta + 6);
    FT_ASSERT(generator_stable_delta > generator_stressed_delta + 6);
    FT_ASSERT(stressed_pressure > stable_pressure + 6.0);
    return 1;
}

int verify_auto_shield_generator_projection()
{
    Game auto_support(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(unlock_shield_support(auto_support));
    auto_support.ensure_planet_item_slot(PLANET_TERRA, ITEM_TITANIUM_BAR);
    auto_support.ensure_planet_item_slot(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART);
    auto_support.set_ore(PLANET_TERRA, ORE_IRON, 420);
    auto_support.set_ore(PLANET_TERRA, ORE_COPPER, 420);
    auto_support.set_ore(PLANET_TERRA, ORE_COAL, 420);
    auto_support.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 40);
    auto_support.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 40);
    auto_support.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 24);
    FT_ASSERT(auto_support.start_research(RESEARCH_SOLAR_PANELS));
    auto_support.tick(60.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, auto_support.get_research_status(RESEARCH_SOLAR_PANELS));
    FT_ASSERT(auto_support.place_building(PLANET_TERRA, BUILDING_SHIELD_GENERATOR, 1, 0) != 0);
    FT_ASSERT(auto_support.place_building(PLANET_TERRA, BUILDING_SOLAR_ARRAY, 0, 1) != 0);
    FT_ASSERT(auto_support.place_building(PLANET_TERRA, BUILDING_SOLAR_ARRAY, 0, 2) != 0);
    auto_support.tick(0.0);
    double stable_generation = auto_support.get_planet_energy_generation(PLANET_TERRA);
    double stable_consumption = auto_support.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(stable_generation + 0.0001 >= stable_consumption);
    auto_support.create_fleet(1);
    int auto_capital = auto_support.create_ship(1, SHIP_CAPITAL);
    auto_support.set_ship_hp(1, auto_capital, 240);
    auto_support.set_ship_shield(1, auto_capital, 160);
    auto_support.sub_ship_shield(1, auto_capital, 120);
    FT_ASSERT(auto_support.start_raider_assault(PLANET_TERRA, 0.4));
    FT_ASSERT(auto_support.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(auto_support.set_assault_aggression(PLANET_TERRA, 0.2));
    int auto_start = auto_support.get_ship_shield(1, auto_capital);
    auto_support.tick(0.4);
    int auto_delta = auto_support.get_ship_shield(1, auto_capital) - auto_start;

    Game starved_support(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(unlock_shield_support(starved_support));
    starved_support.ensure_planet_item_slot(PLANET_TERRA, ITEM_TITANIUM_BAR);
    starved_support.ensure_planet_item_slot(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART);
    starved_support.set_ore(PLANET_TERRA, ORE_IRON, 420);
    starved_support.set_ore(PLANET_TERRA, ORE_COPPER, 420);
    starved_support.set_ore(PLANET_TERRA, ORE_COAL, 420);
    starved_support.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 40);
    starved_support.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 40);
    starved_support.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 24);
    FT_ASSERT(starved_support.place_building(PLANET_TERRA, BUILDING_SHIELD_GENERATOR, 1, 0) != 0);
    starved_support.tick(0.0);
    double starved_generation = starved_support.get_planet_energy_generation(PLANET_TERRA);
    FT_ASSERT(starved_generation < 1.0);
    starved_support.create_fleet(1);
    int starved_capital = starved_support.create_ship(1, SHIP_CAPITAL);
    starved_support.set_ship_hp(1, starved_capital, 240);
    starved_support.set_ship_shield(1, starved_capital, 160);
    starved_support.sub_ship_shield(1, starved_capital, 120);
    FT_ASSERT(starved_support.start_raider_assault(PLANET_TERRA, 0.4));
    FT_ASSERT(starved_support.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(starved_support.set_assault_aggression(PLANET_TERRA, 0.2));
    int starved_start = starved_support.get_ship_shield(1, starved_capital);
    starved_support.tick(0.4);
    int starved_delta = starved_support.get_ship_shield(1, starved_capital) - starved_start;

    FT_ASSERT(auto_delta > 0);
    FT_ASSERT(auto_delta > starved_delta + 3);
    return 1;
}

int verify_sunflare_docking_focus()
{
    Game dock_test(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(unlock_shield_support(dock_test));
    dock_test.create_fleet(1);
    int dock_capital = dock_test.create_ship(1, SHIP_CAPITAL);
    dock_test.set_ship_hp(1, dock_capital, 240);
    dock_test.set_ship_shield(1, dock_capital, 160);
    dock_test.sub_ship_shield(1, dock_capital, 120);
    dock_test.create_fleet(2);
    int dock_sloop = dock_test.create_ship(2, SHIP_SUNFLARE_SLOOP);
    dock_test.set_ship_hp(2, dock_sloop, 80);
    dock_test.set_ship_shield(2, dock_sloop, 50);
    FT_ASSERT(dock_test.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(dock_test.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(dock_test.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(dock_test.set_assault_support(PLANET_TERRA, true, false, false));
    dock_test.tick(2.0);
    int undirected_shield = dock_test.get_ship_shield(1, dock_capital);

    Game focus_test(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(unlock_shield_support(focus_test));
    focus_test.create_fleet(1);
    int focus_capital = focus_test.create_ship(1, SHIP_CAPITAL);
    focus_test.set_ship_hp(1, focus_capital, 240);
    focus_test.set_ship_shield(1, focus_capital, 160);
    focus_test.sub_ship_shield(1, focus_capital, 120);
    focus_test.create_fleet(2);
    int focus_sloop = focus_test.create_ship(2, SHIP_SUNFLARE_SLOOP);
    focus_test.set_ship_hp(2, focus_sloop, 80);
    focus_test.set_ship_shield(2, focus_sloop, 50);
    FT_ASSERT(focus_test.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(focus_test.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(focus_test.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(focus_test.set_assault_support(PLANET_TERRA, true, false, false));
    FT_ASSERT(focus_test.set_assault_sunflare_target(PLANET_TERRA, 1, focus_capital));
    focus_test.tick(2.0);
    int directed_shield = focus_test.get_ship_shield(1, focus_capital);
    FT_ASSERT(directed_shield > undirected_shield + 6);
    FT_ASSERT(focus_test.set_assault_sunflare_target(PLANET_TERRA, 0, 0));
    FT_ASSERT(focus_test.set_assault_support(PLANET_TERRA, false, false, false));
    FT_ASSERT(!focus_test.set_assault_sunflare_target(PLANET_TERRA, 1, focus_capital));
    return 1;
}
