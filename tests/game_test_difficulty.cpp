#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "quests.hpp"
#include "research.hpp"
#include "game_test_scenarios.hpp"

int verify_difficulty_scaling()
{
    Game hard_game(ft_string("127.0.0.1:8080"), ft_string("/"), GAME_DIFFICULTY_HARD);
    FT_ASSERT_EQ(GAME_DIFFICULTY_HARD, hard_game.get_difficulty());
    hard_game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    hard_game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    hard_game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    hard_game.tick(0.0);
    double hard_defense_timer = hard_game.get_quest_time_remaining(QUEST_DEFENSE_OF_TERRA);
    FT_ASSERT(hard_defense_timer > 134.9 && hard_defense_timer < 135.1);
    hard_game.set_ore(PLANET_TERRA, ORE_IRON, 0);
    hard_game.produce(10.0);
    int hard_iron_yield = hard_game.get_ore(PLANET_TERRA, ORE_IRON);
    FT_ASSERT(hard_iron_yield <= 4);
    hard_game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    hard_game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    hard_game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    FT_ASSERT(hard_game.start_research(RESEARCH_UNLOCK_MARS));
    double hard_research_time = hard_game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(hard_research_time > 35.9 && hard_research_time < 36.1);
    FT_ASSERT(hard_game.start_raider_assault(PLANET_TERRA, 1.0));
    double hard_shield = hard_game.get_assault_raider_shield(PLANET_TERRA);
    FT_ASSERT(hard_shield > 100.5 && hard_shield < 101.5);
    return 1;
}
