#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "fleets.hpp"
#include "buildings.hpp"
#include "quests.hpp"
#include "game_test_scenarios.hpp"

int verify_emergency_energy_conservation()
{
    Game conservation(ft_string("127.0.0.1:8080"), ft_string("/"));

    conservation.set_ore(PLANET_TERRA, ORE_IRON, 600);
    conservation.set_ore(PLANET_TERRA, ORE_COPPER, 400);
    conservation.set_ore(PLANET_TERRA, ORE_COAL, 260);
    conservation.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 10);

    FT_ASSERT(conservation.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 0, 1) != 0);
    FT_ASSERT(conservation.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 0) != 0);

    FT_ASSERT(conservation.start_research(RESEARCH_UNLOCK_MARS));
    conservation.tick(35.0);
    FT_ASSERT(conservation.start_research(RESEARCH_SOLAR_PANELS));
    conservation.tick(35.0);
    FT_ASSERT(conservation.start_research(RESEARCH_EMERGENCY_ENERGY_CONSERVATION));
    conservation.tick(40.0);

    conservation.set_ore(PLANET_TERRA, ORE_IRON, 300);

    conservation.tick(0.0);
    double idle_consumption = conservation.get_planet_energy_consumption(PLANET_TERRA);
    conservation.tick(1.0);
    double active_consumption = conservation.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(active_consumption > idle_consumption + 0.4);

    int baseline_bars = conservation.get_ore(PLANET_TERRA, ITEM_IRON_BAR);

    FT_ASSERT(conservation.start_raider_assault(PLANET_TERRA, 1.0));

    conservation.create_fleet(50);
    conservation.set_fleet_location_planet(50, PLANET_TERRA);
    int shield_ship = conservation.create_ship(50, SHIP_SHIELD);
    conservation.set_ship_hp(50, shield_ship, 180);
    conservation.set_ship_shield(50, shield_ship, 140);

    conservation.create_fleet(51);
    conservation.set_fleet_location_planet(51, PLANET_TERRA);
    int escort_ship = conservation.create_ship(51, SHIP_CORVETTE);
    conservation.set_ship_hp(51, escort_ship, 160);
    conservation.set_ship_shield(51, escort_ship, 100);

    FT_ASSERT(conservation.assign_fleet_to_assault(PLANET_TERRA, 50));
    FT_ASSERT(conservation.assign_fleet_to_assault(PLANET_TERRA, 51));

    conservation.tick(0.0);
    FT_ASSERT(conservation.is_planet_energy_conservation_active(PLANET_TERRA));

    conservation.tick(3.0);
    double conserved_consumption = conservation.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(conserved_consumption <= idle_consumption + 0.0001);

    int paused_bars = conservation.get_ore(PLANET_TERRA, ITEM_IRON_BAR);
    FT_ASSERT_EQ(baseline_bars, paused_bars);

    for (int step = 0; step < 120 && conservation.is_assault_active(PLANET_TERRA); ++step)
        conservation.tick(1.0);

    FT_ASSERT(!conservation.is_assault_active(PLANET_TERRA));
    conservation.tick(0.0);
    FT_ASSERT(!conservation.is_planet_energy_conservation_active(PLANET_TERRA));

    conservation.tick(6.0);
    double resumed_consumption = conservation.get_planet_energy_consumption(PLANET_TERRA);
    int resumed_bars = conservation.get_ore(PLANET_TERRA, ITEM_IRON_BAR);
    FT_ASSERT(resumed_consumption > idle_consumption + 0.4);
    FT_ASSERT(resumed_bars > paused_bars);

    return 1;
}
