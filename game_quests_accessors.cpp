#include "game.hpp"

int Game::get_active_quest() const
{
    return this->_quests.get_active_quest_id();
}

int Game::get_quest_status(int quest_id) const
{
    return this->_quests.get_status(quest_id);
}

double Game::get_quest_time_remaining(int quest_id) const
{
    return this->_quests.get_time_remaining(quest_id);
}

int Game::get_quest_choice(int quest_id) const
{
    return this->_quests.get_choice(quest_id);
}
