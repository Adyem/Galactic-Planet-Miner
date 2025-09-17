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

bool CombatManager::start_raider_assault(int planet_id, double difficulty)
{
    if (difficulty <= 0.0)
        difficulty = 1.0;
    Pair<int, ft_combat_encounter> *existing = this->_encounters.find(planet_id);
    if (existing != ft_nullptr && existing->value.active)
        return false;
    if (existing != ft_nullptr)
        this->_encounters.remove(planet_id);
    ft_combat_encounter encounter;
    encounter.planet_id = planet_id;
    encounter.raider_shield = 80.0 * difficulty;
    encounter.raider_hull = 220.0 * difficulty;
    encounter.base_damage = 18.0 * difficulty;
    encounter.elapsed = 0.0;
    encounter.active = true;
    this->_encounters.insert(planet_id, encounter);
    return true;
}

bool CombatManager::add_fleet(int planet_id, int fleet_id)
{
    Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return false;
    ft_sharedptr<ft_vector<int> > &fleets_ptr = entry->value.fleet_ids;
    if (!fleets_ptr)
        fleets_ptr = ft_sharedptr<ft_vector<int> >(new ft_vector<int>());
    ft_vector<int> &fleets = *fleets_ptr;
    for (size_t i = 0; i < fleets.size(); ++i)
    {
        if (fleets[i] == fleet_id)
            return true;
    }
    fleets.push_back(fleet_id);
    return true;
}

bool CombatManager::set_support(int planet_id, bool sunflare_docked,
    bool repair_drones_active, bool shield_generator_online)
{
    Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return false;
    entry->value.modifiers.sunflare_docked = sunflare_docked;
    entry->value.modifiers.repair_drones_active = repair_drones_active;
    entry->value.modifiers.shield_generator_online = shield_generator_online;
    return true;
}

bool CombatManager::is_assault_active(int planet_id) const
{
    const Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    return entry != ft_nullptr && entry->value.active;
}

double CombatManager::get_raider_shield(int planet_id) const
{
    const Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return 0.0;
    return entry->value.raider_shield;
}

double CombatManager::get_raider_hull(int planet_id) const
{
    const Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return 0.0;
    return entry->value.raider_hull;
}

double CombatManager::get_elapsed(int planet_id) const
{
    const Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    if (entry == ft_nullptr || !entry->value.active)
        return 0.0;
    return entry->value.elapsed;
}

void CombatManager::gather_defenders(const ft_combat_encounter &encounter,
    ft_map<int, ft_sharedptr<ft_fleet> > &fleets,
    ft_map<int, ft_sharedptr<ft_fleet> > &planet_fleets,
    ft_vector<ft_sharedptr<ft_fleet> > &out) const
{
    if (encounter.fleet_ids)
    {
        const ft_vector<int> &ids = *encounter.fleet_ids;
        size_t count = ids.size();
        for (size_t i = 0; i < count; ++i)
        {
            int fleet_id = ids[i];
            Pair<int, ft_sharedptr<ft_fleet> > *entry = fleets.find(fleet_id);
            if (entry != ft_nullptr && entry->value)
                out.push_back(entry->value);
        }
    }
    Pair<int, ft_sharedptr<ft_fleet> > *garrison = planet_fleets.find(encounter.planet_id);
    if (garrison != ft_nullptr && garrison->value)
        out.push_back(garrison->value);
}

double CombatManager::calculate_player_power(const ft_vector<ft_sharedptr<ft_fleet> > &defenders) const
{
    double power = 0.0;
    size_t count = defenders.size();
    for (size_t i = 0; i < count; ++i)
    {
        if (!defenders[i])
            continue;
        power += defenders[i]->get_attack_power();
    }
    return power;
}

void CombatManager::apply_support(const ft_combat_encounter &encounter,
    ft_vector<ft_sharedptr<ft_fleet> > &defenders,
    double seconds)
{
    if ((!encounter.modifiers.sunflare_docked && !encounter.modifiers.repair_drones_active)
        || seconds <= 0.0)
        return ;
    int shield_bonus = 0;
    int repair_bonus = 0;
    if (encounter.modifiers.sunflare_docked)
        shield_bonus = static_cast<int>(seconds * 8.0 + 0.5);
    if (encounter.modifiers.repair_drones_active)
        repair_bonus = static_cast<int>(seconds * 10.0 + 0.5);
    size_t count = defenders.size();
    for (size_t i = 0; i < count; ++i)
    {
        if (!defenders[i])
            continue;
        defenders[i]->apply_support(shield_bonus, repair_bonus);
    }
}

void CombatManager::tick(double seconds, ft_map<int, ft_sharedptr<ft_fleet> > &fleets,
    ft_map<int, ft_sharedptr<ft_fleet> > &planet_fleets,
    ft_vector<int> &completed, ft_vector<int> &failed)
{
    if (seconds < 0.0)
        seconds = 0.0;
    size_t count = this->_encounters.size();
    if (count == 0)
        return ;
    Pair<int, ft_combat_encounter> *entries = this->_encounters.end();
    entries -= count;
    ft_vector<int> to_remove;
    for (size_t i = 0; i < count; ++i)
    {
        ft_combat_encounter &encounter = entries[i].value;
        if (!encounter.active)
            continue;
        encounter.elapsed += seconds;
        ft_vector<ft_sharedptr<ft_fleet> > defenders;
        this->gather_defenders(encounter, fleets, planet_fleets, defenders);
        if (defenders.size() == 0)
        {
            failed.push_back(encounter.planet_id);
            to_remove.push_back(entries[i].key);
            continue;
        }
        double player_damage = this->calculate_player_power(defenders) * seconds * this->_player_weapon_multiplier;
        if (player_damage > 0.0)
        {
            if (player_damage >= encounter.raider_shield)
            {
                player_damage -= encounter.raider_shield;
                encounter.raider_shield = 0.0;
            }
            else
            {
                encounter.raider_shield -= player_damage;
                player_damage = 0.0;
            }
            if (player_damage > 0.0)
            {
                if (player_damage >= encounter.raider_hull)
                    encounter.raider_hull = 0.0;
                else
                    encounter.raider_hull -= player_damage;
            }
        }
        if (encounter.raider_hull <= 0.0)
        {
            completed.push_back(encounter.planet_id);
            to_remove.push_back(entries[i].key);
            continue;
        }
        double intensity = 1.0 + encounter.elapsed / 45.0;
        double raider_damage = encounter.base_damage * intensity * seconds;
        if (encounter.modifiers.shield_generator_online)
            raider_damage *= 0.8;
        double leftover = raider_damage;
        bool defenders_operational = false;
        size_t defender_count = defenders.size();
        for (size_t j = 0; j < defender_count && leftover > 0.0; ++j)
        {
            ft_sharedptr<ft_fleet> &fleet = defenders[j];
            if (!fleet)
                continue;
            if (fleet->has_operational_ships())
                defenders_operational = true;
            leftover = fleet->absorb_damage(leftover, this->_player_shield_multiplier, this->_player_hull_multiplier);
        }
        if (!defenders_operational || leftover > 0.0)
        {
            failed.push_back(encounter.planet_id);
            to_remove.push_back(entries[i].key);
            continue;
        }
        this->apply_support(encounter, defenders, seconds);
        bool any_active = false;
        for (size_t j = 0; j < defender_count; ++j)
        {
            if (defenders[j] && defenders[j]->has_operational_ships())
            {
                any_active = true;
                break;
            }
        }
        if (!any_active)
        {
            failed.push_back(encounter.planet_id);
            to_remove.push_back(entries[i].key);
        }
    }
    for (size_t i = 0; i < to_remove.size(); ++i)
        this->_encounters.remove(to_remove[i]);
}
