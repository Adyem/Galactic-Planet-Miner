#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "game_test_scenarios.hpp"
#include "achievements.hpp"
#include "game.hpp"

int verify_achievement_catalog()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    ft_vector<int> ids;
    game.get_achievement_ids(ids);
    FT_ASSERT_EQ(20, static_cast<int>(ids.size()));

    bool found_second_home = false;
    bool found_pioneer = false;
    bool found_pilot = false;
    bool found_guardian = false;
    bool found_skirmish = false;
    bool found_defense = false;
    bool found_investigate = false;
    bool found_supply = false;
    bool found_streak = false;
    bool found_escort = false;
    bool found_battle = false;
    bool found_decision = false;
    bool found_order = false;
    bool found_rebellion = false;
    bool found_order_suppress = false;
    bool found_order_dominion = false;
    bool found_order_verdict = false;
    bool found_rebellion_network = false;
    bool found_rebellion_liberation = false;
    bool found_rebellion_final = false;

    for (size_t i = 0; i < ids.size(); ++i)
    {
        ft_achievement_info info;
        FT_ASSERT(game.get_achievement_info(ids[i], info));
        if (info.id == ACHIEVEMENT_SECOND_HOME)
        {
            found_second_home = true;
            FT_ASSERT_EQ(ACHIEVEMENT_STATUS_LOCKED, info.status);
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_RESEARCH_PIONEER)
        {
            found_pioneer = true;
            FT_ASSERT_EQ(3, info.target);
        }
        else if (info.id == ACHIEVEMENT_LOGISTICS_PILOT)
        {
            found_pilot = true;
            FT_ASSERT_EQ(3, info.target);
        }
        else if (info.id == ACHIEVEMENT_CONVOY_STREAK_GUARDIAN)
        {
            found_guardian = true;
            FT_ASSERT_EQ(5, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_INITIAL_SKIRMISHES)
        {
            found_skirmish = true;
            FT_ASSERT_EQ(1, info.target);
            FT_ASSERT_EQ(ACHIEVEMENT_STATUS_LOCKED, info.status);
        }
        else if (info.id == ACHIEVEMENT_QUEST_DEFENSE_OF_TERRA)
        {
            found_defense = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_INVESTIGATE_RAIDERS)
        {
            found_investigate = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_SECURE_SUPPLY_LINES)
        {
            found_supply = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_STEADY_SUPPLY_STREAK)
        {
            found_streak = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_HIGH_VALUE_ESCORT)
        {
            found_escort = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_CLIMACTIC_BATTLE)
        {
            found_battle = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_CRITICAL_DECISION)
        {
            found_decision = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_ORDER_UPRISING)
        {
            found_order = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_REBELLION_FLEET)
        {
            found_rebellion = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_ORDER_SUPPRESS_RAIDS)
        {
            found_order_suppress = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_ORDER_DOMINION)
        {
            found_order_dominion = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_ORDER_FINAL_VERDICT)
        {
            found_order_verdict = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_REBELLION_NETWORK)
        {
            found_rebellion_network = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_REBELLION_LIBERATION)
        {
            found_rebellion_liberation = true;
            FT_ASSERT_EQ(1, info.target);
        }
        else if (info.id == ACHIEVEMENT_QUEST_REBELLION_FINAL_PUSH)
        {
            found_rebellion_final = true;
            FT_ASSERT_EQ(1, info.target);
        }
    }

    FT_ASSERT(found_second_home);
    FT_ASSERT(found_pioneer);
    FT_ASSERT(found_pilot);
    FT_ASSERT(found_guardian);
    FT_ASSERT(found_skirmish);
    FT_ASSERT(found_defense);
    FT_ASSERT(found_investigate);
    FT_ASSERT(found_supply);
    FT_ASSERT(found_streak);
    FT_ASSERT(found_escort);
    FT_ASSERT(found_battle);
    FT_ASSERT(found_decision);
    FT_ASSERT(found_order);
    FT_ASSERT(found_rebellion);
    FT_ASSERT(found_order_suppress);
    FT_ASSERT(found_order_dominion);
    FT_ASSERT(found_order_verdict);
    FT_ASSERT(found_rebellion_network);
    FT_ASSERT(found_rebellion_liberation);
    FT_ASSERT(found_rebellion_final);
    return 1;
}

static void stock_resource(Game &game, int planet_id, int resource_id, int amount)
{
    game.ensure_planet_item_slot(planet_id, resource_id);
    game.set_ore(planet_id, resource_id, amount);
}

static int complete_story_path(Game &game, bool execute_branch)
{
    if (execute_branch)
    {
        FT_ASSERT(advance_to_order_final_verdict(game));
    }
    else
    {
        FT_ASSERT(advance_to_rebellion_final_push(game));
    }

    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_INITIAL_SKIRMISHES));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_DEFENSE_OF_TERRA));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_SECOND_HOME));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_INVESTIGATE_RAIDERS));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_RESEARCH_PIONEER));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_LOGISTICS_PILOT));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_CONVOY_STREAK_GUARDIAN));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_SECURE_SUPPLY_LINES));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_STEADY_SUPPLY_STREAK));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_HIGH_VALUE_ESCORT));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_CLIMACTIC_BATTLE));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_CRITICAL_DECISION));

    if (execute_branch)
    {
        FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_ORDER_UPRISING));
        FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_ORDER_SUPPRESS_RAIDS));
        FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_ORDER_DOMINION));
        FT_ASSERT(game.resolve_quest_choice(QUEST_ORDER_FINAL_VERDICT, QUEST_CHOICE_ORDER_EXECUTE_REBELS));
        game.tick(0.0);
        FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_ORDER_FINAL_VERDICT));
        FT_ASSERT_EQ(ACHIEVEMENT_STATUS_LOCKED, game.get_achievement_status(ACHIEVEMENT_QUEST_REBELLION_FINAL_PUSH));
    }
    else
    {
        FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_REBELLION_FLEET));
        FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_REBELLION_NETWORK));
        FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_REBELLION_LIBERATION));
        stock_resource(game, PLANET_NOCTARIS_PRIME, ORE_NANOMATERIAL, 3);
        game.tick(0.0);
        FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_REBELLION_FINAL_PUSH));
        FT_ASSERT_EQ(ACHIEVEMENT_STATUS_LOCKED, game.get_achievement_status(ACHIEVEMENT_QUEST_ORDER_FINAL_VERDICT));
    }
    return 1;
}

int verify_achievement_progression()
{
    Game rebellion_path(ft_string("127.0.0.1:8080"), ft_string("/"));
    if (!complete_story_path(rebellion_path, false))
        return 0;
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, rebellion_path.get_achievement_status(ACHIEVEMENT_QUEST_REBELLION_FINAL_PUSH));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, rebellion_path.get_achievement_status(ACHIEVEMENT_QUEST_REBELLION_NETWORK));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, rebellion_path.get_achievement_status(ACHIEVEMENT_QUEST_REBELLION_LIBERATION));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_LOCKED, rebellion_path.get_achievement_status(ACHIEVEMENT_QUEST_ORDER_SUPPRESS_RAIDS));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_LOCKED, rebellion_path.get_achievement_status(ACHIEVEMENT_QUEST_ORDER_DOMINION));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_LOCKED, rebellion_path.get_achievement_status(ACHIEVEMENT_QUEST_ORDER_FINAL_VERDICT));

    Game order_path(ft_string("127.0.0.1:8080"), ft_string("/"));
    if (!complete_story_path(order_path, true))
        return 0;
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, order_path.get_achievement_status(ACHIEVEMENT_QUEST_ORDER_FINAL_VERDICT));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, order_path.get_achievement_status(ACHIEVEMENT_QUEST_ORDER_SUPPRESS_RAIDS));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, order_path.get_achievement_status(ACHIEVEMENT_QUEST_ORDER_DOMINION));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_LOCKED, order_path.get_achievement_status(ACHIEVEMENT_QUEST_REBELLION_FINAL_PUSH));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_LOCKED, order_path.get_achievement_status(ACHIEVEMENT_QUEST_REBELLION_NETWORK));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_LOCKED, order_path.get_achievement_status(ACHIEVEMENT_QUEST_REBELLION_LIBERATION));

    return 1;
}

int verify_quest_achievement_failures()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    stock_resource(game, PLANET_TERRA, ORE_IRON, 32);
    stock_resource(game, PLANET_TERRA, ORE_COPPER, 32);
    game.tick(0.0);
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, game.get_achievement_status(ACHIEVEMENT_QUEST_INITIAL_SKIRMISHES));

    double elapsed = 0.0;
    while (elapsed < 200.0)
    {
        game.tick(5.0);
        elapsed += 5.0;
    }
    int defense_status = game.get_achievement_status(ACHIEVEMENT_QUEST_DEFENSE_OF_TERRA);
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_LOCKED, defense_status);
    int quest_state = game.get_quest_status(QUEST_DEFENSE_OF_TERRA);
    FT_ASSERT(quest_state == QUEST_STATUS_ACTIVE || quest_state == QUEST_STATUS_AVAILABLE);
    return 1;
}
