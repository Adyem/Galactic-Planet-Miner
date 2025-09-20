#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "fleets.hpp"
#include "buildings.hpp"
#include "quests.hpp"
#include "game_test_scenarios.hpp"

int compare_energy_pressure_scenarios()
{
    Game energy_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    energy_game.set_ore(PLANET_TERRA, ORE_IRON, 200);
    energy_game.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    energy_game.set_ore(PLANET_TERRA, ORE_COAL, 50);
    FT_ASSERT(energy_game.place_building(PLANET_TERRA, BUILDING_SMELTER, 1, 0) != 0);
    energy_game.tick(0.0);
    double deficit_pressure = energy_game.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(deficit_pressure > 1.9 && deficit_pressure < 2.1);
    energy_game.create_fleet(1);
    int deficit_capital = energy_game.create_ship(1, SHIP_CAPITAL);
    energy_game.set_ship_hp(1, deficit_capital, 220);
    energy_game.set_ship_shield(1, deficit_capital, 100);
    energy_game.create_fleet(2);
    int deficit_guard = energy_game.create_ship(2, SHIP_SHIELD);
    energy_game.set_ship_hp(2, deficit_guard, 140);
    energy_game.set_ship_shield(2, deficit_guard, 80);
    FT_ASSERT(energy_game.start_raider_assault(PLANET_TERRA, 1.0));
    double energy_raider_shield = energy_game.get_assault_raider_shield(PLANET_TERRA);
    double energy_raider_hull = energy_game.get_assault_raider_hull(PLANET_TERRA);
    FT_ASSERT(energy_game.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(energy_game.assign_fleet_to_assault(PLANET_TERRA, 2));
    energy_game.tick(3.0);
    int deficit_hp = energy_game.get_ship_hp(1, deficit_capital);
    FT_ASSERT(energy_game.is_assault_active(PLANET_TERRA));

    Game balanced_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    balanced_game.set_ore(PLANET_TERRA, ORE_IRON, 200);
    balanced_game.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    balanced_game.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(balanced_game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    FT_ASSERT(balanced_game.place_building(PLANET_TERRA, BUILDING_SMELTER, 1, 0) != 0);
    balanced_game.tick(0.0);
    double balanced_pressure = balanced_game.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(balanced_pressure < 0.1);
    balanced_game.create_fleet(1);
    int balanced_capital = balanced_game.create_ship(1, SHIP_CAPITAL);
    balanced_game.set_ship_hp(1, balanced_capital, 220);
    balanced_game.set_ship_shield(1, balanced_capital, 100);
    balanced_game.create_fleet(2);
    int balanced_guard = balanced_game.create_ship(2, SHIP_SHIELD);
    balanced_game.set_ship_hp(2, balanced_guard, 140);
    balanced_game.set_ship_shield(2, balanced_guard, 80);
    FT_ASSERT(balanced_game.start_raider_assault(PLANET_TERRA, 1.0));
    double balanced_raider_shield = balanced_game.get_assault_raider_shield(PLANET_TERRA);
    double balanced_raider_hull = balanced_game.get_assault_raider_hull(PLANET_TERRA);
    FT_ASSERT(balanced_game.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(balanced_game.assign_fleet_to_assault(PLANET_TERRA, 2));
    balanced_game.tick(3.0);
    int balanced_hp = balanced_game.get_ship_hp(1, balanced_capital);
    FT_ASSERT(balanced_game.is_assault_active(PLANET_TERRA));
    FT_ASSERT(energy_raider_shield > balanced_raider_shield);
    FT_ASSERT(energy_raider_hull > balanced_raider_hull);
    FT_ASSERT(balanced_hp > deficit_hp);

    ft_vector<ft_ship_spatial_state> energy_raider_positions;
    ft_vector<ft_ship_spatial_state> balanced_raider_positions;
    FT_ASSERT(energy_game.get_assault_raider_positions(PLANET_TERRA, energy_raider_positions));
    FT_ASSERT(balanced_game.get_assault_raider_positions(PLANET_TERRA, balanced_raider_positions));
    FT_ASSERT(energy_raider_positions.size() >= balanced_raider_positions.size());
    int energy_radar_count = 0;
    int energy_shield_count = 0;
    int balanced_radar_count = 0;
    int balanced_shield_count = 0;
    for (size_t idx = 0; idx < energy_raider_positions.size(); ++idx)
    {
        if (energy_raider_positions[idx].ship_type == SHIP_RADAR)
            energy_radar_count += 1;
        else if (energy_raider_positions[idx].ship_type == SHIP_SHIELD)
            energy_shield_count += 1;
    }
    for (size_t idx = 0; idx < balanced_raider_positions.size(); ++idx)
    {
        if (balanced_raider_positions[idx].ship_type == SHIP_RADAR)
            balanced_radar_count += 1;
        else if (balanced_raider_positions[idx].ship_type == SHIP_SHIELD)
            balanced_shield_count += 1;
    }
    FT_ASSERT(energy_radar_count > balanced_radar_count);
    FT_ASSERT(energy_shield_count >= balanced_shield_count);

    Game surge_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    surge_game.set_ore(PLANET_TERRA, ORE_IRON, 320);
    surge_game.set_ore(PLANET_TERRA, ORE_COPPER, 320);
    surge_game.set_ore(PLANET_TERRA, ORE_COAL, 320);
    FT_ASSERT(surge_game.place_building(PLANET_TERRA, BUILDING_TRANSFER_NODE, 0, 3) != 0);
    FT_ASSERT(surge_game.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 0) != 0);
    FT_ASSERT(surge_game.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 2) != 0);
    FT_ASSERT(surge_game.place_building(PLANET_TERRA, BUILDING_PROCESSOR, 0, 1) != 0);
    FT_ASSERT(surge_game.place_building(PLANET_TERRA, BUILDING_SMELTER, 1, 2) != 0);
    surge_game.tick(0.0);
    double surge_pressure = surge_game.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(surge_pressure > 7.4);
    surge_game.create_fleet(1);
    int surge_capital = surge_game.create_ship(1, SHIP_CAPITAL);
    surge_game.set_ship_hp(1, surge_capital, 220);
    surge_game.set_ship_shield(1, surge_capital, 100);
    FT_ASSERT(surge_game.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(surge_game.assign_fleet_to_assault(PLANET_TERRA, 1));
    surge_game.tick(0.5);
    ft_vector<ft_ship_spatial_state> surge_raider_positions;
    FT_ASSERT(surge_game.get_assault_raider_positions(PLANET_TERRA, surge_raider_positions));
    int surge_radar_count = 0;
    int surge_shield_count = 0;
    for (size_t idx = 0; idx < surge_raider_positions.size(); ++idx)
    {
        if (surge_raider_positions[idx].ship_type == SHIP_RADAR)
            surge_radar_count += 1;
        else if (surge_raider_positions[idx].ship_type == SHIP_SHIELD)
            surge_shield_count += 1;
    }
    FT_ASSERT(surge_raider_positions.size() >= energy_raider_positions.size());
    FT_ASSERT(surge_radar_count >= energy_radar_count + 1);
    FT_ASSERT(surge_shield_count >= energy_shield_count + 1);
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

int compare_storyline_assaults()
{
    Game early_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    early_game.set_ore(PLANET_TERRA, ORE_IRON, 200);
    early_game.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    early_game.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(early_game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    early_game.tick(0.0);
    early_game.create_fleet(1);
    int early_capital = early_game.create_ship(1, SHIP_CAPITAL);
    early_game.set_ship_hp(1, early_capital, 220);
    early_game.set_ship_shield(1, early_capital, 100);
    early_game.create_fleet(2);
    int early_guard = early_game.create_ship(2, SHIP_SHIELD);
    early_game.set_ship_hp(2, early_guard, 140);
    early_game.set_ship_shield(2, early_guard, 80);
    FT_ASSERT(early_game.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(early_game.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(early_game.assign_fleet_to_assault(PLANET_TERRA, 2));
    early_game.tick(3.0);
    int early_hp = early_game.get_ship_hp(1, early_capital);
    FT_ASSERT(early_game.is_assault_active(PLANET_TERRA));

    Game narrative_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    narrative_game.set_ore(PLANET_TERRA, ORE_IRON, 20);
    narrative_game.set_ore(PLANET_TERRA, ORE_COPPER, 20);
    narrative_game.tick(0.0);
    narrative_game.create_fleet(1);
    int narrative_setup_one = narrative_game.create_ship(1, SHIP_SHIELD);
    narrative_game.set_ship_hp(1, narrative_setup_one, 80);
    narrative_game.create_fleet(2);
    int narrative_setup_two = narrative_game.create_ship(2, SHIP_SHIELD);
    narrative_game.set_ship_hp(2, narrative_setup_two, 60);
    narrative_game.tick(0.0);
    narrative_game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    narrative_game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    narrative_game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    FT_ASSERT(narrative_game.start_research(RESEARCH_UNLOCK_MARS));
    narrative_game.tick(30.0);
    narrative_game.set_ore(PLANET_TERRA, ORE_MITHRIL, 8);
    narrative_game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    FT_ASSERT(narrative_game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    narrative_game.tick(40.0);
    narrative_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, narrative_game.get_active_quest());
    narrative_game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    narrative_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
    narrative_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    for (int convoy = 0; convoy < 8; ++convoy)
    {
        int moved = narrative_game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 20);
        FT_ASSERT(moved >= 20);
        double waited = 0.0;
        while (narrative_game.get_active_convoy_count() > 0 && waited < 240.0)
        {
            narrative_game.tick(4.0);
            waited += 4.0;
        }
        FT_ASSERT(waited < 240.0);
    }
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, narrative_game.get_quest_status(QUEST_HIGH_VALUE_ESCORT));
    FT_ASSERT_EQ(QUEST_CLIMACTIC_BATTLE, narrative_game.get_active_quest());
    narrative_game.remove_ship(1, narrative_setup_one);
    narrative_game.remove_ship(2, narrative_setup_two);
    narrative_game.set_ore(PLANET_TERRA, ORE_IRON, 200);
    narrative_game.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    narrative_game.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(narrative_game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    narrative_game.tick(0.0);
    int narrative_capital = narrative_game.create_ship(1, SHIP_CAPITAL);
    narrative_game.set_ship_hp(1, narrative_capital, 220);
    narrative_game.set_ship_shield(1, narrative_capital, 100);
    int narrative_guard = narrative_game.create_ship(2, SHIP_SHIELD);
    narrative_game.set_ship_hp(2, narrative_guard, 140);
    narrative_game.set_ship_shield(2, narrative_guard, 80);
    FT_ASSERT(narrative_game.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(narrative_game.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(narrative_game.assign_fleet_to_assault(PLANET_TERRA, 2));
    narrative_game.tick(3.0);
    int narrative_hp = narrative_game.get_ship_hp(1, narrative_capital);
    FT_ASSERT(narrative_game.is_assault_active(PLANET_TERRA));
    FT_ASSERT(narrative_hp < early_hp);

    ft_vector<ft_ship_spatial_state> early_raider_positions;
    ft_vector<ft_ship_spatial_state> narrative_raider_positions;
    FT_ASSERT(early_game.get_assault_raider_positions(PLANET_TERRA, early_raider_positions));
    FT_ASSERT(narrative_game.get_assault_raider_positions(PLANET_TERRA, narrative_raider_positions));
    int early_capital_count = 0;
    int narrative_capital_count = 0;
    for (size_t idx = 0; idx < early_raider_positions.size(); ++idx)
    {
        if (early_raider_positions[idx].ship_type == SHIP_CAPITAL)
            early_capital_count += 1;
    }
    for (size_t idx = 0; idx < narrative_raider_positions.size(); ++idx)
    {
        if (narrative_raider_positions[idx].ship_type == SHIP_CAPITAL)
            narrative_capital_count += 1;
    }
    FT_ASSERT_EQ(0, early_capital_count);
    FT_ASSERT(narrative_capital_count >= 1);
    FT_ASSERT(narrative_raider_positions.size() > early_raider_positions.size());
    return 1;
}
