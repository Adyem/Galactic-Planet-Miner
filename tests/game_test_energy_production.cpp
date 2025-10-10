#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "fleets.hpp"
#include "buildings.hpp"
#include "quests.hpp"
#include "game_test_scenarios.hpp"

int verify_building_tick_state_persistence()
{
    Game tick_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    tick_game.set_ore(PLANET_TERRA, ORE_IRON, 120);
    tick_game.set_ore(PLANET_TERRA, ORE_COPPER, 120);
    tick_game.set_ore(PLANET_TERRA, ORE_COAL, 120);

    FT_ASSERT(tick_game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 0, 1) != 0);
    FT_ASSERT(tick_game.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 0) != 0);

    tick_game.tick(0.0);
    double baseline_energy = tick_game.get_planet_energy_consumption(PLANET_TERRA);
    int baseline_logistic = tick_game.get_planet_logistic_usage(PLANET_TERRA);
    int baseline_bars = tick_game.get_ore(PLANET_TERRA, ITEM_IRON_BAR);

    tick_game.tick(1.0);
    double active_energy = tick_game.get_planet_energy_consumption(PLANET_TERRA);
    int active_logistic = tick_game.get_planet_logistic_usage(PLANET_TERRA);
    FT_ASSERT(active_energy > baseline_energy + 0.5);
    FT_ASSERT(active_logistic > baseline_logistic);

    for (int step = 0; step < 5; ++step)
        tick_game.tick(1.0);

    int produced_bars = tick_game.get_ore(PLANET_TERRA, ITEM_IRON_BAR);
    FT_ASSERT(produced_bars > baseline_bars);
    return 1;
}

int verify_crafting_resume_requires_full_cycle()
{
    Game production_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    production_game.set_ore(PLANET_TERRA, ORE_IRON, 200);
    production_game.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    production_game.set_ore(PLANET_TERRA, ORE_COAL, 200);
    production_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 10);
    production_game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 10);
    production_game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 0);

    FT_ASSERT(production_game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    FT_ASSERT(production_game.place_building(PLANET_TERRA, BUILDING_CRAFTING_BAY, 0, 2) != 0);
    production_game.tick(1.0);

    int initial_parts = production_game.get_ore(PLANET_TERRA, ITEM_ENGINE_PART);
    FT_ASSERT_EQ(0, initial_parts);

    FT_ASSERT(production_game.place_building(PLANET_TERRA, BUILDING_CONVEYOR, 3, 2) != 0);

    production_game.tick(0.0);
    FT_ASSERT_EQ(initial_parts, production_game.get_ore(PLANET_TERRA, ITEM_ENGINE_PART));

    production_game.tick(7.5);
    FT_ASSERT_EQ(initial_parts, production_game.get_ore(PLANET_TERRA, ITEM_ENGINE_PART));

    production_game.tick(1.0);
    int after_resume = production_game.get_ore(PLANET_TERRA, ITEM_ENGINE_PART);
    FT_ASSERT(after_resume >= initial_parts + 1);
    return 1;
}
