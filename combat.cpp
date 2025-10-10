#include "combat.hpp"

CombatManager::CombatManager()
    : _player_weapon_multiplier(1.0),
      _player_shield_multiplier(1.0),
      _player_hull_multiplier(1.0)
{
    return ;
}

void CombatManager::set_player_weapon_multiplier(double value)
{
    if (value < 1.0)
        value = 1.0;
    this->_player_weapon_multiplier = value;
}

void CombatManager::set_player_shield_multiplier(double value)
{
    if (value < 1.0)
        value = 1.0;
    this->_player_shield_multiplier = value;
}

void CombatManager::set_player_hull_multiplier(double value)
{
    if (value < 1.0)
        value = 1.0;
    this->_player_hull_multiplier = value;
}

void CombatManager::clear_branch_assault_state() noexcept
{
    // Branch assaults do not maintain additional combat-manager state yet, so this
    // routine simply provides a stable call site for higher-level game logic.
}
