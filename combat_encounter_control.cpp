#include "combat.hpp"

bool CombatManager::set_support(int planet_id, bool sunflare_docked,
    bool repair_drones_active, bool shield_generator_online)
{
    Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return false;
    if (entry->value.control_mode != ASSAULT_CONTROL_ACTIVE)
        return false;
    entry->value.modifiers.sunflare_docked = sunflare_docked;
    entry->value.modifiers.repair_drones_active = repair_drones_active;
    entry->value.modifiers.shield_generator_online = shield_generator_online;
    entry->value.auto_repair_drones_active = false;
    if (!sunflare_docked)
    {
        entry->value.sunflare_target_fleet_id = 0;
        entry->value.sunflare_target_ship_uid = 0;
        entry->value.sunflare_focus_pool = 0.0;
    }
    return true;
}

bool CombatManager::set_sunflare_dock_target(int planet_id, int fleet_id, int ship_uid)
{
    Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return false;
    ft_combat_encounter &encounter = entry->value;
    if (encounter.control_mode != ASSAULT_CONTROL_ACTIVE)
        return false;
    if (!encounter.modifiers.sunflare_docked)
    {
        encounter.sunflare_target_fleet_id = 0;
        encounter.sunflare_target_ship_uid = 0;
        encounter.sunflare_focus_pool = 0.0;
        return false;
    }
    if (fleet_id == 0 || ship_uid == 0)
    {
        encounter.sunflare_target_fleet_id = 0;
        encounter.sunflare_target_ship_uid = 0;
        encounter.sunflare_focus_pool = 0.0;
        return true;
    }
    bool found = false;
    if (encounter.fleet_ids)
    {
        const ft_vector<int> &ids = *encounter.fleet_ids;
        size_t count = ids.size();
        for (size_t i = 0; i < count; ++i)
        {
            if (ids[i] == fleet_id)
            {
                found = true;
                break;
            }
        }
    }
    if (!found)
    {
        if (fleet_id == -encounter.planet_id)
            found = true;
    }
    if (!found)
        return false;
    encounter.sunflare_target_fleet_id = fleet_id;
    encounter.sunflare_target_ship_uid = ship_uid;
    encounter.sunflare_focus_pool = 0.0;
    return true;
}

bool CombatManager::set_control_mode(int planet_id, int control_mode)
{
    Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return false;
    if (control_mode != ASSAULT_CONTROL_ACTIVE)
        control_mode = ASSAULT_CONTROL_AUTO;
    if (entry->value.control_mode == control_mode)
        return true;
    entry->value.control_mode = control_mode;
    if (control_mode == ASSAULT_CONTROL_AUTO)
    {
        entry->value.modifiers.sunflare_docked = false;
        entry->value.modifiers.repair_drones_active = false;
        entry->value.modifiers.shield_generator_online = false;
        entry->value.manual_focus_remaining = 0.0;
        entry->value.manual_focus_cooldown = 0.0;
        entry->value.tactical_pause_remaining = 0.0;
        entry->value.tactical_pause_cooldown = 0.0;
        entry->value.auto_repair_drones_active = false;
    }
    else
    {
        entry->value.auto_generator_stability = 0.0;
        entry->value.auto_repair_drones_active = false;
    }
    return true;
}

bool CombatManager::set_raider_aggression(int planet_id, double aggression)
{
    Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return false;
    if (aggression < 0.2)
        aggression = 0.2;
    if (aggression > 2.5)
        aggression = 2.5;
    entry->value.raider_aggression = aggression;
    return true;
}

bool CombatManager::trigger_focus_fire(int planet_id)
{
    Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return false;
    ft_combat_encounter &encounter = entry->value;
    if (encounter.control_mode != ASSAULT_CONTROL_ACTIVE)
        return false;
    if (encounter.manual_focus_remaining > 0.0)
        return false;
    if (encounter.manual_focus_cooldown > 0.0)
        return false;
    double duration = 4.0 + encounter.narrative_pressure * 1.5;
    if (duration > 6.0)
        duration = 6.0;
    encounter.manual_focus_remaining = duration;
    double cooldown = 12.0 - encounter.narrative_pressure * 4.0;
    cooldown -= encounter.energy_pressure * 1.5;
    if (cooldown < 6.0)
        cooldown = 6.0;
    encounter.manual_focus_cooldown = cooldown;
    return true;
}

bool CombatManager::request_tactical_pause(int planet_id)
{
    Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return false;
    ft_combat_encounter &encounter = entry->value;
    if (encounter.control_mode != ASSAULT_CONTROL_ACTIVE)
        return false;
    if (encounter.tactical_pause_remaining > 0.0)
        return false;
    if (encounter.tactical_pause_cooldown > 0.0)
        return false;
    double duration = 1.5 + encounter.energy_pressure * 0.5;
    duration += encounter.narrative_pressure * 0.5;
    if (duration > 3.0)
        duration = 3.0;
    encounter.tactical_pause_remaining = duration;
    double cooldown = 20.0 - encounter.narrative_pressure * 5.0;
    cooldown -= encounter.energy_pressure * 2.0;
    if (cooldown < 8.0)
        cooldown = 8.0;
    encounter.tactical_pause_cooldown = cooldown;
    return true;
}

void CombatManager::set_auto_shield_generator(int planet_id, double stability)
{
    Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return ;
    if (entry->value.control_mode != ASSAULT_CONTROL_AUTO)
    {
        entry->value.auto_generator_stability = 0.0;
        return ;
    }
    if (stability < 0.0)
        stability = 0.0;
    if (stability > 1.0)
        stability = 1.0;
    entry->value.auto_generator_stability = stability;
}
