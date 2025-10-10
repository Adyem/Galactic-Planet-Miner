#include "combat.hpp"

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
        this->add_raider_ship(*encounter.raider_fleet, SHIP_RAIDER_CORVETTE, 72, 24, 14, difficulty <= 0.0 ? 1.0 : difficulty);
    this->sync_raider_tracks(encounter);
    encounter.spike_timer = 0.0;
    encounter.spike_time_remaining = 0.0;
    if (narrative_pressure > 0.0)
    {
        double initial_duration;
        if (narrative_pressure >= 0.3)
            initial_duration = 3.5 + narrative_pressure * 3.5;
        else
            initial_duration = 2.5 + narrative_pressure * 2.5;
        if (encounter.energy_pressure > 0.0)
            initial_duration += encounter.energy_pressure * (narrative_pressure >= 0.3 ? 0.75 : 0.5);
        encounter.spike_time_remaining = initial_duration;
        double warmup_window = (narrative_pressure >= 0.3) ? 18.0 : 22.0;
        warmup_window -= narrative_pressure * 6.0;
        warmup_window -= encounter.energy_pressure * (narrative_pressure >= 0.3 ? 3.0 : 2.0);
        if (warmup_window < 6.0)
            warmup_window = 6.0;
        encounter.spike_timer = warmup_window * 0.5;
    }
    encounter.pending_shield_support = 0.0;
    encounter.pending_hull_support = 0.0;
    encounter.sunflare_target_fleet_id = 0;
    encounter.sunflare_target_ship_uid = 0;
    encounter.sunflare_focus_pool = 0.0;
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
    encounter.auto_repair_drones_active = false;
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
