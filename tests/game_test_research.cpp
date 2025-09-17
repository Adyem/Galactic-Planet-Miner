#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "buildings.hpp"
#include "fleets.hpp"
#include "game_test_scenarios.hpp"

int verify_crafting_and_energy_research()
{
    Game baseline(ft_string("127.0.0.1:8080"), ft_string("/"));
    baseline.set_ore(PLANET_TERRA, ORE_IRON, 200);
    baseline.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    baseline.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(baseline.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    FT_ASSERT(baseline.place_building(PLANET_TERRA, BUILDING_SMELTER, 0, 2) != 0);
    baseline.tick(0.0);
    baseline.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 0);
    baseline.tick(8.0);
    int baseline_bars = baseline.get_ore(PLANET_TERRA, ITEM_IRON_BAR);
    FT_ASSERT(baseline_bars >= 2);
    double baseline_energy = baseline.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(baseline_energy > 1.9 && baseline_energy < 2.1);

    Game advanced(ft_string("127.0.0.1:8080"), ft_string("/"));
    int item_ids[] = {
        ITEM_IRON_BAR,
        ITEM_COPPER_BAR,
        ITEM_MITHRIL_BAR,
        ITEM_ENGINE_PART,
        ITEM_TITANIUM_BAR,
        ITEM_ADVANCED_ENGINE_PART,
        ITEM_FUSION_REACTOR,
        ITEM_ACCUMULATOR
    };
    for (size_t idx = 0; idx < sizeof(item_ids) / sizeof(item_ids[0]); ++idx)
        advanced.ensure_planet_item_slot(PLANET_TERRA, item_ids[idx]);
    int ore_ids[] = {
        ORE_IRON,
        ORE_COPPER,
        ORE_COAL,
        ORE_MITHRIL,
        ORE_TIN,
        ORE_SILVER,
        ORE_GOLD,
        ORE_TITANIUM,
        ORE_OBSIDIAN,
        ORE_CRYSTAL,
        ORE_TRITIUM
    };
    for (size_t idx = 0; idx < sizeof(ore_ids) / sizeof(ore_ids[0]); ++idx)
        advanced.set_ore(PLANET_TERRA, ore_ids[idx], 600);
    advanced.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    advanced.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 200);
    advanced.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 200);
    advanced.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 200);
    advanced.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 200);
    advanced.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 200);
    advanced.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 12);
    advanced.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 60);

    FT_ASSERT(advanced.start_research(RESEARCH_UNLOCK_MARS));
    advanced.tick(30.0);
    FT_ASSERT(advanced.start_research(RESEARCH_SOLAR_PANELS));
    advanced.tick(25.0);
    FT_ASSERT(advanced.start_research(RESEARCH_CRAFTING_MASTERY));
    advanced.tick(35.0);
    FT_ASSERT(advanced.start_research(RESEARCH_FASTER_CRAFTING));
    advanced.tick(40.0);
    FT_ASSERT(advanced.start_research(RESEARCH_EMERGENCY_ENERGY_CONSERVATION));
    advanced.tick(30.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, advanced.get_research_status(RESEARCH_FASTER_CRAFTING));
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, advanced.get_research_status(RESEARCH_EMERGENCY_ENERGY_CONSERVATION));

    FT_ASSERT(advanced.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    FT_ASSERT(advanced.place_building(PLANET_TERRA, BUILDING_SMELTER, 0, 2) != 0);
    advanced.tick(0.0);
    advanced.set_ore(PLANET_TERRA, ORE_IRON, 200);
    advanced.set_ore(PLANET_TERRA, ORE_COAL, 200);
    advanced.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 0);
    advanced.tick(8.0);
    int accelerated_bars = advanced.get_ore(PLANET_TERRA, ITEM_IRON_BAR);
    double accelerated_energy = advanced.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(accelerated_bars >= baseline_bars + 2);
    FT_ASSERT(accelerated_energy + 0.01 < baseline_energy);
    FT_ASSERT(accelerated_energy < 1.5);
    return 1;
}

int verify_auxiliary_and_escape_protocol()
{
    Game fleet_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    int item_ids[] = {
        ITEM_IRON_BAR,
        ITEM_COPPER_BAR,
        ITEM_MITHRIL_BAR,
        ITEM_ENGINE_PART,
        ITEM_TITANIUM_BAR,
        ITEM_ADVANCED_ENGINE_PART,
        ITEM_FUSION_REACTOR,
        ITEM_ACCUMULATOR
    };
    for (size_t idx = 0; idx < sizeof(item_ids) / sizeof(item_ids[0]); ++idx)
        fleet_game.ensure_planet_item_slot(PLANET_TERRA, item_ids[idx]);
    int ore_ids[] = {
        ORE_IRON,
        ORE_COPPER,
        ORE_MITHRIL,
        ORE_COAL,
        ORE_TIN,
        ORE_SILVER,
        ORE_GOLD,
        ORE_TITANIUM,
        ORE_OBSIDIAN,
        ORE_CRYSTAL,
        ORE_TRITIUM
    };
    for (size_t idx = 0; idx < sizeof(ore_ids) / sizeof(ore_ids[0]); ++idx)
        fleet_game.set_ore(PLANET_TERRA, ore_ids[idx], 1000);
    fleet_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 600);
    fleet_game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 600);
    fleet_game.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 600);
    fleet_game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 400);
    fleet_game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 600);
    fleet_game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 600);
    fleet_game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 30);
    fleet_game.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 120);

    fleet_game.create_fleet(1);
    FT_ASSERT_EQ(0, fleet_game.create_ship(1, SHIP_CAPITAL));

    fleet_game.create_fleet(99);
    int pre_escape = fleet_game.create_ship(99, SHIP_SHIELD);
    fleet_game.set_ship_hp(99, pre_escape, 20);
    FT_ASSERT_EQ(0, fleet_game.sub_ship_hp(99, pre_escape, 25));

    FT_ASSERT(fleet_game.start_research(RESEARCH_UNLOCK_MARS));
    fleet_game.tick(30.0);
    FT_ASSERT(fleet_game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    fleet_game.tick(40.0);
    FT_ASSERT(fleet_game.start_research(RESEARCH_UNLOCK_VULCAN));
    fleet_game.tick(55.0);

    FT_ASSERT(fleet_game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    fleet_game.tick(30.0);
    FT_ASSERT(fleet_game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    fleet_game.tick(40.0);
    FT_ASSERT(fleet_game.start_research(RESEARCH_SHIELD_TECHNOLOGY));
    fleet_game.tick(35.0);
    FT_ASSERT(fleet_game.start_research(RESEARCH_REPAIR_DRONE_TECHNOLOGY));
    fleet_game.tick(45.0);

    FT_ASSERT(fleet_game.start_research(RESEARCH_ARMAMENT_ENHANCEMENT_I));
    fleet_game.tick(35.0);
    FT_ASSERT(fleet_game.start_research(RESEARCH_ARMAMENT_ENHANCEMENT_II));
    fleet_game.tick(45.0);

    FT_ASSERT(fleet_game.start_research(RESEARCH_CAPITAL_SHIP_INITIATIVE));
    fleet_game.tick(55.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, fleet_game.get_research_status(RESEARCH_CAPITAL_SHIP_INITIATIVE));

    int first_capital = fleet_game.create_ship(1, SHIP_CAPITAL);
    FT_ASSERT(first_capital != 0);
    FT_ASSERT_EQ(0, fleet_game.create_ship(1, SHIP_CAPITAL));
    FT_ASSERT_EQ(0, fleet_game.create_ship(1, SHIP_FRIGATE_SUPPORT));

    FT_ASSERT(fleet_game.start_research(RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT));
    fleet_game.tick(60.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, fleet_game.get_research_status(RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT));

    fleet_game.create_fleet(2);
    int support_frigate = fleet_game.create_ship(2, SHIP_FRIGATE_SUPPORT);
    FT_ASSERT(support_frigate != 0);
    int second_capital = fleet_game.create_ship(2, SHIP_CAPITAL);
    FT_ASSERT(second_capital != 0);
    FT_ASSERT_EQ(0, fleet_game.create_ship(2, SHIP_CAPITAL));

    FT_ASSERT(fleet_game.start_research(RESEARCH_ESCAPE_POD_LIFELINE));
    fleet_game.tick(45.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, fleet_game.get_research_status(RESEARCH_ESCAPE_POD_LIFELINE));

    fleet_game.create_fleet(3);
    int rescue_ship = fleet_game.create_ship(3, SHIP_SHIELD);
    fleet_game.set_ship_hp(3, rescue_ship, 18);
    int first_result = fleet_game.sub_ship_hp(3, rescue_ship, 30);
    FT_ASSERT_EQ(10, first_result);
    int second_result = fleet_game.sub_ship_hp(3, rescue_ship, 15);
    FT_ASSERT_EQ(0, second_result);
    int third_result = fleet_game.sub_ship_hp(3, rescue_ship, 5);
    FT_ASSERT_EQ(0, third_result);
    return 1;
}
