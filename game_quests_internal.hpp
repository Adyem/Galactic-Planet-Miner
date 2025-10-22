#ifndef GAME_QUESTS_INTERNAL_HPP
#define GAME_QUESTS_INTERNAL_HPP

#include "game.hpp"

double game_quests_internal_get_objective_current_amount(
    const ft_quest_objective &objective,
    const ft_quest_context &context);

bool game_quests_internal_is_objective_met_for_snapshot(
    const ft_quest_objective &objective,
    const ft_quest_context &context);

void game_quests_internal_populate_branch_snapshot(
    const QuestManager &manager,
    Game::ft_story_branch_snapshot &out, int branch_id,
    const char *name, const char *summary,
    const int *quest_ids, size_t quest_count,
    int required_choice, int active_choice);

const int *game_quests_internal_get_act_one_quest_ids();
size_t game_quests_internal_get_act_one_quest_count();

const int *game_quests_internal_get_act_two_quest_ids();
size_t game_quests_internal_get_act_two_quest_count();

const int *game_quests_internal_get_order_branch_quest_ids();
size_t game_quests_internal_get_order_branch_quest_count();

const int *game_quests_internal_get_rebellion_branch_quest_ids();
size_t game_quests_internal_get_rebellion_branch_quest_count();

#endif
