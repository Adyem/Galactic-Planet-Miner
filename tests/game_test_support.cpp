#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "fleets.hpp"
#include "buildings.hpp"
#include "game_test_scenarios.hpp"

int compare_generator_support()
{
    Game generator_off(ft_string("127.0.0.1:8080"), ft_string("/"));
    generator_off.set_ore(PLANET_TERRA, ORE_IRON, 200);
    generator_off.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    generator_off.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(generator_off.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    generator_off.tick(0.0);
    generator_off.create_fleet(1);
    int generator_off_capital = generator_off.create_ship(1, SHIP_CAPITAL);
    generator_off.set_ship_hp(1, generator_off_capital, 220);
    generator_off.set_ship_shield(1, generator_off_capital, 100);
    generator_off.create_fleet(2);
    int generator_off_guard = generator_off.create_ship(2, SHIP_SHIELD);
    generator_off.set_ship_hp(2, generator_off_guard, 140);
    generator_off.set_ship_shield(2, generator_off_guard, 80);
    FT_ASSERT(generator_off.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(generator_off.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(generator_off.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(generator_off.set_assault_support(PLANET_TERRA, false, false, false));
    generator_off.tick(3.0);
    int generator_off_hp = generator_off.get_ship_hp(1, generator_off_capital);
    int generator_off_shield = generator_off.get_ship_shield(1, generator_off_capital);

    Game generator_on(ft_string("127.0.0.1:8080"), ft_string("/"));
    generator_on.set_ore(PLANET_TERRA, ORE_IRON, 200);
    generator_on.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    generator_on.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(generator_on.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    generator_on.tick(0.0);
    generator_on.create_fleet(1);
    int generator_on_capital = generator_on.create_ship(1, SHIP_CAPITAL);
    generator_on.set_ship_hp(1, generator_on_capital, 220);
    generator_on.set_ship_shield(1, generator_on_capital, 100);
    generator_on.create_fleet(2);
    int generator_on_guard = generator_on.create_ship(2, SHIP_SHIELD);
    generator_on.set_ship_hp(2, generator_on_guard, 140);
    generator_on.set_ship_shield(2, generator_on_guard, 80);
    FT_ASSERT(generator_on.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(generator_on.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(generator_on.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(generator_on.set_assault_support(PLANET_TERRA, false, false, true));
    generator_on.tick(3.0);
    int generator_on_hp = generator_on.get_ship_hp(1, generator_on_capital);
    int generator_on_shield = generator_on.get_ship_shield(1, generator_on_capital);
    FT_ASSERT(generator_on_hp >= generator_off_hp);
    FT_ASSERT(generator_on_shield > generator_off_shield);
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
