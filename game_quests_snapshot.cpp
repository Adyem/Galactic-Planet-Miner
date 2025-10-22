#include "game.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/Template/move.hpp"
#include "game_quests_internal.hpp"

void Game::get_quest_log_snapshot(ft_quest_log_snapshot &out) const
{
#include "game_quests_snapshot_initialize.inc"
#include "game_quests_snapshot_entries.inc"
#include "game_quests_snapshot_acts.inc"
#include "game_quests_snapshot_choices.inc"
#include "game_quests_snapshot_epilogue.inc"
#include "game_quests_snapshot_recent.inc"
}
