#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Math/math.hpp"
#include "fleets.hpp"
#include "buildings.hpp"
#include "research.hpp"
#include "game_test_scenarios.hpp"

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

int inspect_support_ship_positioning()
{
    Game support_alone(ft_string("127.0.0.1:8080"), ft_string("/"));
    support_alone.set_ore(PLANET_TERRA, ORE_IRON, 200);
    support_alone.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    support_alone.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(support_alone.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    support_alone.tick(0.0);
    support_alone.create_fleet(1);
    int lone_salvage = support_alone.create_ship(1, SHIP_SALVAGE);
    support_alone.set_ship_hp(1, lone_salvage, 90);
    support_alone.set_ship_shield(1, lone_salvage, 30);
    FT_ASSERT(support_alone.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(support_alone.assign_fleet_to_assault(PLANET_TERRA, 1));
    support_alone.tick(0.5);
    ft_vector<ft_ship_spatial_state> support_alone_positions;
    FT_ASSERT(support_alone.get_assault_defender_positions(PLANET_TERRA, support_alone_positions));
    double unescorted_salvage_z = -1000.0;
    for (size_t idx = 0; idx < support_alone_positions.size(); ++idx)
    {
        if (support_alone_positions[idx].ship_type == SHIP_SALVAGE)
        {
            unescorted_salvage_z = support_alone_positions[idx].z;
            break;
        }
    }
    FT_ASSERT(unescorted_salvage_z < 0.0);

    Game support_escorted(ft_string("127.0.0.1:8080"), ft_string("/"));
    support_escorted.set_ore(PLANET_TERRA, ORE_IRON, 200);
    support_escorted.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    support_escorted.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(support_escorted.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    support_escorted.tick(0.0);
    support_escorted.create_fleet(1);
    int escorted_salvage = support_escorted.create_ship(1, SHIP_SALVAGE);
    support_escorted.set_ship_hp(1, escorted_salvage, 90);
    support_escorted.set_ship_shield(1, escorted_salvage, 30);
    support_escorted.create_fleet(2);
    int escort_guard = support_escorted.create_ship(2, SHIP_SHIELD);
    support_escorted.set_ship_hp(2, escort_guard, 120);
    support_escorted.set_ship_shield(2, escort_guard, 70);
    FT_ASSERT(support_escorted.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(support_escorted.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(support_escorted.assign_fleet_to_assault(PLANET_TERRA, 2));
    support_escorted.tick(0.5);
    ft_vector<ft_ship_spatial_state> escorted_positions;
    FT_ASSERT(support_escorted.get_assault_defender_positions(PLANET_TERRA, escorted_positions));
    double escorted_salvage_z = -1000.0;
    for (size_t idx = 0; idx < escorted_positions.size(); ++idx)
    {
        if (escorted_positions[idx].ship_type == SHIP_SALVAGE)
        {
            escorted_salvage_z = escorted_positions[idx].z;
            break;
        }
    }
    FT_ASSERT(escorted_salvage_z > unescorted_salvage_z + 1.2);

    Game outnumbered_line(ft_string("127.0.0.1:8080"), ft_string("/"));
    outnumbered_line.set_ore(PLANET_TERRA, ORE_IRON, 200);
    outnumbered_line.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    outnumbered_line.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(outnumbered_line.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    outnumbered_line.tick(0.0);
    outnumbered_line.create_fleet(1);
    int lone_guard = outnumbered_line.create_ship(1, SHIP_SHIELD);
    outnumbered_line.set_ship_hp(1, lone_guard, 130);
    outnumbered_line.set_ship_shield(1, lone_guard, 70);
    FT_ASSERT(outnumbered_line.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(outnumbered_line.assign_fleet_to_assault(PLANET_TERRA, 1));
    outnumbered_line.tick(0.5);
    ft_vector<ft_ship_spatial_state> outnumbered_positions;
    FT_ASSERT(outnumbered_line.get_assault_defender_positions(PLANET_TERRA, outnumbered_positions));
    double outnumbered_avg = 0.0;
    size_t outnumbered_count = 0;
    for (size_t idx = 0; idx < outnumbered_positions.size(); ++idx)
    {
        if (outnumbered_positions[idx].ship_type == SHIP_SHIELD)
        {
            outnumbered_avg += outnumbered_positions[idx].z;
            outnumbered_count += 1;
        }
    }
    FT_ASSERT(outnumbered_count > 0);
    outnumbered_avg /= static_cast<double>(outnumbered_count);

    Game supported_line(ft_string("127.0.0.1:8080"), ft_string("/"));
    supported_line.set_ore(PLANET_TERRA, ORE_IRON, 200);
    supported_line.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    supported_line.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(supported_line.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    supported_line.tick(0.0);
    for (int fleet_id = 1; fleet_id <= 5; ++fleet_id)
    {
        supported_line.create_fleet(fleet_id);
        int guard_ship = supported_line.create_ship(fleet_id, SHIP_SHIELD);
        supported_line.set_ship_hp(fleet_id, guard_ship, 130);
        supported_line.set_ship_shield(fleet_id, guard_ship, 70);
    }
    FT_ASSERT(supported_line.start_raider_assault(PLANET_TERRA, 1.0));
    for (int fleet_id = 1; fleet_id <= 5; ++fleet_id)
        FT_ASSERT(supported_line.assign_fleet_to_assault(PLANET_TERRA, fleet_id));
    supported_line.tick(0.5);
    ft_vector<ft_ship_spatial_state> supported_positions;
    FT_ASSERT(supported_line.get_assault_defender_positions(PLANET_TERRA, supported_positions));
    double supported_avg = 0.0;
    size_t supported_count = 0;
    for (size_t idx = 0; idx < supported_positions.size(); ++idx)
    {
        if (supported_positions[idx].ship_type == SHIP_SHIELD)
        {
            supported_avg += supported_positions[idx].z;
            supported_count += 1;
        }
    }
    FT_ASSERT(supported_count > 0);
    supported_avg /= static_cast<double>(supported_count);
    FT_ASSERT(outnumbered_avg + 2.0 < supported_avg);

    Game healthy_capital(ft_string("127.0.0.1:8080"), ft_string("/"));
    healthy_capital.set_ore(PLANET_TERRA, ORE_IRON, 200);
    healthy_capital.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    healthy_capital.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(healthy_capital.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    healthy_capital.tick(0.0);
    healthy_capital.create_fleet(1);
    int healthy_warship = healthy_capital.create_ship(1, SHIP_CAPITAL);
    healthy_capital.set_ship_hp(1, healthy_warship, 220);
    healthy_capital.set_ship_shield(1, healthy_warship, 100);
    FT_ASSERT(healthy_capital.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(healthy_capital.assign_fleet_to_assault(PLANET_TERRA, 1));
    healthy_capital.tick(0.5);
    ft_vector<ft_ship_spatial_state> healthy_positions;
    FT_ASSERT(healthy_capital.get_assault_defender_positions(PLANET_TERRA, healthy_positions));
    double healthy_capital_z = -1000.0;
    for (size_t idx = 0; idx < healthy_positions.size(); ++idx)
    {
        if (healthy_positions[idx].ship_type == SHIP_CAPITAL)
        {
            healthy_capital_z = healthy_positions[idx].z;
            break;
        }
    }
    FT_ASSERT(healthy_capital_z < -5.0);

    Game low_hp_capital(ft_string("127.0.0.1:8080"), ft_string("/"));
    low_hp_capital.set_ore(PLANET_TERRA, ORE_IRON, 200);
    low_hp_capital.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    low_hp_capital.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(low_hp_capital.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    low_hp_capital.tick(0.0);
    low_hp_capital.create_fleet(1);
    int desperate_warship = low_hp_capital.create_ship(1, SHIP_CAPITAL);
    low_hp_capital.set_ship_hp(1, desperate_warship, 220);
    low_hp_capital.set_ship_shield(1, desperate_warship, 100);
    low_hp_capital.set_ship_hp(1, desperate_warship, 44);
    FT_ASSERT(low_hp_capital.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(low_hp_capital.assign_fleet_to_assault(PLANET_TERRA, 1));
    low_hp_capital.tick(0.5);
    ft_vector<ft_ship_spatial_state> low_hp_positions;
    FT_ASSERT(low_hp_capital.get_assault_defender_positions(PLANET_TERRA, low_hp_positions));
    double desperate_capital_z = -1000.0;
    for (size_t idx = 0; idx < low_hp_positions.size(); ++idx)
    {
        if (low_hp_positions[idx].ship_type == SHIP_CAPITAL)
        {
            desperate_capital_z = low_hp_positions[idx].z;
            break;
        }
    }
    FT_ASSERT(desperate_capital_z > healthy_capital_z + 1.5);
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

int verify_radar_static_defense_support()
{
    Game baseline(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT_EQ(0, baseline.get_planet_escort_rating(PLANET_TERRA));
    double baseline_modifier = baseline.get_planet_convoy_raid_risk_modifier(PLANET_TERRA);
    FT_ASSERT(math_fabs(baseline_modifier) < 0.000001);

    Game radar(ft_string("127.0.0.1:8080"), ft_string("/"));
    radar.ensure_planet_item_slot(PLANET_TERRA, ITEM_COPPER_BAR);
    radar.ensure_planet_item_slot(PLANET_TERRA, ITEM_MITHRIL_BAR);
    radar.set_ore(PLANET_TERRA, ORE_IRON, 120);
    radar.set_ore(PLANET_TERRA, ORE_COPPER, 120);
    radar.set_ore(PLANET_TERRA, ORE_COAL, 120);
    radar.set_ore(PLANET_TERRA, ORE_MITHRIL, 120);
    radar.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 60);
    radar.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 40);

    FT_ASSERT(radar.start_research(RESEARCH_UNLOCK_MARS));
    radar.tick(30.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, radar.get_research_status(RESEARCH_UNLOCK_MARS));
    FT_ASSERT(radar.start_research(RESEARCH_UNLOCK_ZALTHOR));
    radar.tick(40.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, radar.get_research_status(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT(radar.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    radar.tick(30.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, radar.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_I));

    int first_radar = radar.place_building(PLANET_TERRA, BUILDING_PROXIMITY_RADAR, 0, 0);
    if (first_radar == 0)
        first_radar = radar.place_building(PLANET_TERRA, BUILDING_PROXIMITY_RADAR, 1, 0);
    FT_ASSERT(first_radar != 0);
    radar.tick(0.0);

    int proximity_rating = radar.get_planet_escort_rating(PLANET_TERRA);
    double proximity_modifier = radar.get_planet_convoy_raid_risk_modifier(PLANET_TERRA);
    FT_ASSERT(proximity_rating >= 2);
    FT_ASSERT(proximity_modifier > 0.07 && proximity_modifier < 0.09);

    FT_ASSERT(radar.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    radar.tick(40.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, radar.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    int mobile_radar = radar.place_building(PLANET_TERRA, BUILDING_MOBILE_RADAR, 2, 0);
    if (mobile_radar == 0)
        mobile_radar = radar.place_building(PLANET_TERRA, BUILDING_MOBILE_RADAR, 3, 0);
    FT_ASSERT(mobile_radar != 0);
    radar.tick(0.0);

    int enhanced_rating = radar.get_planet_escort_rating(PLANET_TERRA);
    double enhanced_modifier = radar.get_planet_convoy_raid_risk_modifier(PLANET_TERRA);
    FT_ASSERT(enhanced_rating > proximity_rating);
    FT_ASSERT(enhanced_modifier > proximity_modifier);
    FT_ASSERT(enhanced_modifier < 0.21);
    return 1;
}
