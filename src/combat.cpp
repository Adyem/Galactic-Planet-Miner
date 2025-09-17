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

bool CombatManager::start_raider_assault(int planet_id, double difficulty,
    double energy_pressure, double narrative_pressure, int control_mode)
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
    if (energy_pressure < 0.0)
        energy_pressure = 0.0;
    if (narrative_pressure < 0.0)
        narrative_pressure = 0.0;
    if (energy_pressure > 1.5)
        energy_pressure = 1.5;
    if (narrative_pressure > 1.5)
        narrative_pressure = 1.5;
    encounter.energy_pressure = energy_pressure;
    encounter.narrative_pressure = narrative_pressure;
    if (control_mode != ASSAULT_CONTROL_ACTIVE)
        control_mode = ASSAULT_CONTROL_AUTO;
    encounter.control_mode = control_mode;
    encounter.raider_frontline = 110.0 + energy_pressure * 18.0 + narrative_pressure * 12.0;
    if (encounter.raider_frontline < 60.0)
        encounter.raider_frontline = 60.0;
    if (encounter.raider_frontline > 160.0)
        encounter.raider_frontline = 160.0;
    encounter.defender_line = -encounter.raider_frontline * 0.55 - 12.0;
    if (encounter.defender_line < -140.0)
        encounter.defender_line = -140.0;
    if (encounter.defender_line > -18.0)
        encounter.defender_line = -18.0;
    encounter.formation_time = 0.0;
    this->build_raider_fleet(encounter, difficulty, energy_pressure, narrative_pressure);
    if (!encounter.raider_fleet)
        encounter.raider_fleet = ft_sharedptr<ft_fleet>(new ft_fleet(-planet_id));
    if (encounter.raider_fleet && !encounter.raider_fleet->has_operational_ships())
        this->add_raider_ship(*encounter.raider_fleet, SHIP_RADAR, 48, 14, 10, difficulty <= 0.0 ? 1.0 : difficulty);
    this->sync_raider_tracks(encounter);
    encounter.spike_timer = 0.0;
    encounter.spike_time_remaining = 0.0;
    encounter.pending_shield_support = 0.0;
    encounter.pending_hull_support = 0.0;
    encounter.manual_focus_remaining = 0.0;
    encounter.manual_focus_cooldown = 0.0;
    encounter.tactical_pause_remaining = 0.0;
    encounter.tactical_pause_cooldown = 0.0;
    double base_aggression = 1.0 + narrative_pressure * 0.25 + energy_pressure * 0.12;
    if (base_aggression < 0.6)
        base_aggression = 0.6;
    if (base_aggression > 1.8)
        base_aggression = 1.8;
    encounter.raider_aggression = base_aggression;
    encounter.elapsed = 0.0;
    encounter.active = encounter.raider_fleet && encounter.raider_fleet->has_operational_ships();
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
    if (entry->value.control_mode != ASSAULT_CONTROL_ACTIVE)
        return false;
    entry->value.modifiers.sunflare_docked = sunflare_docked;
    entry->value.modifiers.repair_drones_active = repair_drones_active;
    entry->value.modifiers.shield_generator_online = shield_generator_online;
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

bool CombatManager::is_assault_active(int planet_id) const
{
    const Pair<int, ft_combat_encounter> *entry = this->_encounters.find(planet_id);
    return entry != ft_nullptr && entry->value.active;
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
