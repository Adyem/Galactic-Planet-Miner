#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "fleets.hpp"
#include "buildings.hpp"
#include "quests.hpp"
#include "game_test_scenarios.hpp"

int verify_helios_beacon_energy_network()
{
    Game network_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    network_game.set_ore(PLANET_TERRA, ORE_IRON, 400);
    network_game.set_ore(PLANET_TERRA, ORE_COPPER, 360);
    network_game.set_ore(PLANET_TERRA, ORE_COAL, 320);
    network_game.set_ore(PLANET_TERRA, ORE_MITHRIL, 180);
    network_game.set_ore(PLANET_TERRA, ORE_CRYSTAL, 160);
    network_game.set_ore(PLANET_TERRA, ORE_TRITIUM, 90);
    network_game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 220);
    network_game.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 160);
    network_game.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 40);
    network_game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 80);
    network_game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 6);
    network_game.set_ore(PLANET_MARS, ITEM_COPPER_BAR, 60);
    network_game.set_ore(PLANET_MARS, ITEM_ADVANCED_ENGINE_PART, 24);
    network_game.set_ore(PLANET_MARS, ITEM_ACCUMULATOR, 12);

    FT_ASSERT(network_game.start_research(RESEARCH_UNLOCK_MARS));
    network_game.tick(120.0);
    FT_ASSERT(network_game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    network_game.tick(120.0);
    FT_ASSERT(network_game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    network_game.tick(120.0);
    FT_ASSERT(network_game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    network_game.tick(120.0);
    FT_ASSERT(network_game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_III));
    network_game.tick(150.0);
    FT_ASSERT(network_game.start_research(RESEARCH_SHIELD_TECHNOLOGY));
    network_game.tick(150.0);

    int generator = network_game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 0, 2);
    if (generator == 0)
        generator = network_game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0);
    FT_ASSERT(generator != 0);

    int relay = network_game.place_building(PLANET_MARS, BUILDING_TRADE_RELAY, 1, 0);
    if (relay == 0)
        relay = network_game.place_building(PLANET_MARS, BUILDING_TRADE_RELAY, 1, 1);
    FT_ASSERT(relay != 0);

    network_game.tick(0.0);
    double baseline = network_game.get_planet_energy_pressure(PLANET_MARS);
    FT_ASSERT(baseline > 3.4);

    int helios = network_game.place_building(PLANET_TERRA, BUILDING_HELIOS_BEACON, 2, 1);
    if (helios == 0)
        helios = network_game.place_building(PLANET_TERRA, BUILDING_HELIOS_BEACON, 1, 1);
    FT_ASSERT(helios != 0);

    network_game.tick(0.0);
    double boosted = network_game.get_planet_energy_pressure(PLANET_MARS);
    FT_ASSERT(boosted < baseline - 1.2);

    FT_ASSERT(network_game.remove_building(PLANET_TERRA, generator));
    network_game.tick(0.0);
    double collapsed = network_game.get_planet_energy_pressure(PLANET_MARS);
    FT_ASSERT(collapsed > boosted + 1.0);
    FT_ASSERT(collapsed < baseline - 0.2);
    return 1;
}
