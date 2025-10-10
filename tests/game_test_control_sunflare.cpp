#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft_math_bridge.hpp"
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

int verify_sunflare_targeting_rules()
{
    Game rules(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(unlock_shield_support(rules));
    FT_ASSERT(!rules.set_assault_sunflare_target(PLANET_TERRA, 1, 1));
    rules.create_fleet(1);
    int capital = rules.create_ship(1, SHIP_CAPITAL);
    rules.set_ship_hp(1, capital, 240);
    rules.set_ship_shield(1, capital, 160);
    rules.sub_ship_shield(1, capital, 120);
    rules.create_fleet(2);
    int sloop = rules.create_ship(2, SHIP_SUNFLARE_SLOOP);
    rules.set_ship_hp(2, sloop, 80);
    rules.set_ship_shield(2, sloop, 50);
    FT_ASSERT(rules.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(rules.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(rules.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(!rules.set_assault_sunflare_target(PLANET_TERRA, 1, capital));
    FT_ASSERT(rules.set_assault_control_mode(PLANET_TERRA, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(!rules.set_assault_sunflare_target(PLANET_TERRA, 1, capital));
    FT_ASSERT(rules.set_assault_support(PLANET_TERRA, true, false, false));
    FT_ASSERT(!rules.set_assault_sunflare_target(PLANET_TERRA, 99, capital));
    FT_ASSERT(!rules.set_assault_sunflare_target(PLANET_TERRA, 1, 999999));
    FT_ASSERT(rules.set_assault_sunflare_target(PLANET_TERRA, 1, capital));
    FT_ASSERT(rules.set_assault_sunflare_target(PLANET_TERRA, 0, 0));
    FT_ASSERT(rules.set_assault_support(PLANET_TERRA, false, false, false));
    FT_ASSERT(!rules.set_assault_sunflare_target(PLANET_TERRA, 1, capital));
    return 1;
}
