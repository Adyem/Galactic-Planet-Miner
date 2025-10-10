#include "combat.hpp"

bool CombatManager::is_assault_active(int planet_id) const
{
    const Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    return entry != ft_nullptr && entry->value.active;
}

void CombatManager::get_active_planets(ft_vector<int> &out) const
{
    out.clear();
    size_t count = this->_encounters.size();
    if (count == 0)
        return ;
    const Pair<int, ft_combat_encounter> *entries = this->_encounters.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        if (entries[i].value.active)
            out.push_back(entries[i].key);
    }
}

double CombatManager::get_raider_shield(int planet_id) const
{
    const Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active || !entry->value.raider_fleet)
        return 0.0;
    return static_cast<double>(entry->value.raider_fleet->get_total_ship_shield());
}

double CombatManager::get_raider_hull(int planet_id) const
{
    const Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active || !entry->value.raider_fleet)
        return 0.0;
    return static_cast<double>(entry->value.raider_fleet->get_total_ship_hp());
}

double CombatManager::get_elapsed(int planet_id) const
{
    const Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return 0.0;
    return entry->value.elapsed;
}

bool CombatManager::get_raider_positions(int planet_id, ft_vector<ft_ship_spatial_state> &out) const
{
    out.clear();
    const Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return false;
    const ft_combat_encounter &encounter = entry->value;
    size_t count = encounter.raider_tracks.size();
    if (count == 0)
        return true;
    const Pair<int, ft_ship_tracker> *entries = encounter.raider_tracks.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
        out.push_back(entries[i].value.spatial);
    return true;
}

bool CombatManager::get_defender_positions(int planet_id, ft_vector<ft_ship_spatial_state> &out) const
{
    out.clear();
    const Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return false;
    const ft_combat_encounter &encounter = entry->value;
    size_t count = encounter.defender_tracks.size();
    if (count == 0)
        return true;
    const Pair<int, ft_ship_tracker> *entries = encounter.defender_tracks.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
        out.push_back(entries[i].value.spatial);
    return true;
}
