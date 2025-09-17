#include "combat.hpp"
#include <algorithm>
#include <cmath>

namespace
{
    const double FT_TWO_PI = 6.28318530717958647692;
    const double FT_DEG_TO_RAD = FT_TWO_PI / 360.0;
}

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

int CombatManager::add_raider_ship(ft_fleet &fleet, int ship_type, int base_hp,
    int base_shield, int armor, double scale) const
{
    double normalized = scale;
    if (normalized < 0.5)
        normalized = 0.5;
    if (normalized > 3.0)
        normalized = 3.0;
    int ship_uid = fleet.create_ship(ship_type);
    double hp_value = static_cast<double>(base_hp) * normalized;
    int scaled_hp = static_cast<int>(hp_value);
    if (static_cast<double>(scaled_hp) < hp_value)
        scaled_hp += 1;
    if (scaled_hp < 1)
        scaled_hp = 1;
    double shield_value = static_cast<double>(base_shield) * normalized;
    int scaled_shield = static_cast<int>(shield_value);
    if (static_cast<double>(scaled_shield) < shield_value)
        scaled_shield += 1;
    if (scaled_shield < 0)
        scaled_shield = 0;
    fleet.set_ship_hp(ship_uid, scaled_hp);
    fleet.set_ship_shield(ship_uid, scaled_shield);
    fleet.set_ship_armor(ship_uid, armor);
    return ship_uid;
}

void CombatManager::build_raider_fleet(ft_combat_encounter &encounter,
    double difficulty, double energy_pressure, double narrative_pressure)
{
    double scale = difficulty;
    if (scale < 0.5)
        scale = 0.5;
    if (scale > 2.5)
        scale = 2.5;
    double energy_scale = 1.0 + energy_pressure * 0.2;
    double narrative_scale = 1.0 + narrative_pressure * 0.12;
    scale *= energy_scale;
    scale *= narrative_scale;
    encounter.raider_fleet = ft_sharedptr<ft_fleet>(new ft_fleet(-(encounter.planet_id + 1000)));
    ft_fleet &fleet = *encounter.raider_fleet;
    fleet.set_location_planet(encounter.planet_id);
    this->add_raider_ship(fleet, SHIP_RADAR, 60, 20, 12, scale);
    this->add_raider_ship(fleet, SHIP_RADAR, 60, 20, 12, scale);
    this->add_raider_ship(fleet, SHIP_SHIELD, 50, 30, 8, scale);
    this->add_raider_ship(fleet, SHIP_SALVAGE, 50, 10, 18, scale);

    double energy_remaining = energy_pressure;
    while (energy_remaining >= 0.5)
    {
        this->add_raider_ship(fleet, SHIP_RADAR, 58, 18, 10, scale);
        energy_remaining -= 0.5;
    }
    if (energy_remaining >= 0.2)
        this->add_raider_ship(fleet, SHIP_RADAR, 58, 18, 10, scale);
    if (energy_pressure >= 1.0)
        this->add_raider_ship(fleet, SHIP_SHIELD, 60, 36, 12, scale);

    int heavy = 0;
    if (narrative_pressure >= 0.3)
        heavy = 1;
    if (narrative_pressure >= 0.9)
        heavy = 2;
    for (int i = 0; i < heavy; ++i)
        this->add_raider_ship(fleet, SHIP_CAPITAL, 140, 60, 30, scale);

    double attack_multiplier = 1.25;
    if (difficulty > 0.0)
        attack_multiplier *= difficulty;
    attack_multiplier *= (1.0 + energy_pressure * 0.25);
    attack_multiplier *= (1.0 + narrative_pressure * 0.3);
    if (attack_multiplier < 0.75)
        attack_multiplier = 0.75;
    if (attack_multiplier > 4.0)
        attack_multiplier = 4.0;
    encounter.attack_multiplier = attack_multiplier;

    double defense_multiplier = 1.0;
    if (difficulty > 1.0)
        defense_multiplier += (difficulty - 1.0) * 0.2;
    else if (difficulty < 1.0)
        defense_multiplier -= (1.0 - difficulty) * 0.15;
    if (defense_multiplier < 0.7)
        defense_multiplier = 0.7;
    defense_multiplier += energy_pressure * 0.1;
    defense_multiplier += narrative_pressure * 0.05;
    if (defense_multiplier < 0.7)
        defense_multiplier = 0.7;
    if (defense_multiplier > 2.5)
        defense_multiplier = 2.5;
    encounter.defense_multiplier = defense_multiplier;
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

void CombatManager::apply_raider_support(ft_combat_encounter &encounter, double seconds,
    bool spike_active)
{
    if (!encounter.raider_fleet || seconds <= 0.0)
        return ;
    double base_factor = encounter.energy_pressure * 0.15;
    if (spike_active)
        base_factor += 0.35 + encounter.narrative_pressure * 0.15;
    else
        base_factor += encounter.narrative_pressure * 0.08;
    if (base_factor <= 0.0)
        return ;
    double shield_rate = (4.0 + encounter.energy_pressure * 3.0) * base_factor;
    double hull_rate = (2.0 + encounter.narrative_pressure * 2.0) * base_factor;
    if (shield_rate <= 0.0 && hull_rate <= 0.0)
        return ;
    encounter.pending_shield_support += shield_rate * seconds;
    encounter.pending_hull_support += hull_rate * seconds;
    int shield_amount = static_cast<int>(encounter.pending_shield_support);
    int hull_amount = static_cast<int>(encounter.pending_hull_support);
    if (shield_amount <= 0 && hull_amount <= 0)
        return ;
    encounter.pending_shield_support -= static_cast<double>(shield_amount);
    encounter.pending_hull_support -= static_cast<double>(hull_amount);
    if (shield_amount < 0)
        shield_amount = 0;
    if (hull_amount < 0)
        hull_amount = 0;
    if (shield_amount == 0 && hull_amount == 0)
        return ;
    encounter.raider_fleet->apply_support(shield_amount, hull_amount);
}

void CombatManager::sync_raider_tracks(ft_combat_encounter &encounter)
{
    ft_vector<int> ship_ids;
    if (!encounter.raider_fleet)
    {
        encounter.raider_operational_ships = 0;
        encounter.raider_line_ships = 0;
        encounter.raider_support_ships = 0;
        while (encounter.raider_tracks.size() > 0)
        {
            Pair<int, ft_ship_tracker> *entry = encounter.raider_tracks.end();
            entry -= 1;
            encounter.raider_tracks.remove(entry->key);
        }
        return ;
    }
    encounter.raider_fleet->get_ship_ids(ship_ids);
    ft_vector<int> active_ids;
    encounter.raider_operational_ships = 0;
    encounter.raider_line_ships = 0;
    encounter.raider_support_ships = 0;
    for (size_t i = 0; i < ship_ids.size(); ++i)
    {
        int ship_uid = ship_ids[i];
        int hp = encounter.raider_fleet->get_ship_hp(ship_uid);
        int shield = encounter.raider_fleet->get_ship_shield(ship_uid);
        if (hp <= 0 && shield <= 0)
        {
            encounter.raider_tracks.remove(ship_uid);
            continue;
        }
        active_ids.push_back(ship_uid);
        Pair<int, ft_ship_tracker> *entry = encounter.raider_tracks.find(ship_uid);
        const ft_ship *ship_data = encounter.raider_fleet->get_ship(ship_uid);
        if (ship_data == ft_nullptr)
        {
            encounter.raider_tracks.remove(ship_uid);
            continue;
        }
        encounter.raider_operational_ships += 1;
        if (ship_data->role == SHIP_ROLE_LINE)
            encounter.raider_line_ships += 1;
        else
            encounter.raider_support_ships += 1;
        if (entry == ft_nullptr)
        {
            ft_ship_tracker tracker;
            this->initialize_tracker(tracker, ship_uid, *ship_data, true, encounter);
            encounter.raider_tracks.insert(ship_uid, tracker);
            entry = encounter.raider_tracks.find(ship_uid);
            if (entry == ft_nullptr)
                continue;
        }
        else if (entry->value.spatial.ship_type != ship_data->type
            || entry->value.role != ship_data->role)
        {
            this->initialize_tracker(entry->value, ship_uid, *ship_data, true, encounter);
        }
        ft_ship_tracker &tracker = entry->value;
        tracker.spatial.ship_type = ship_data->type;
        tracker.max_hp = ship_data->max_hp;
        tracker.max_shield = ship_data->max_shield;
        tracker.max_speed = ship_data->max_speed;
        if (tracker.max_speed < 4.0)
            tracker.max_speed = 4.0;
        tracker.acceleration = ship_data->acceleration;
        if (tracker.acceleration < 0.5)
            tracker.acceleration = 0.5;
        tracker.turn_speed = ship_data->turn_speed;
        if (tracker.turn_speed < 10.0)
            tracker.turn_speed = 10.0;
        tracker.normal_behavior = ship_data->combat_behavior;
        tracker.outnumbered_behavior = ship_data->outnumbered_behavior;
        tracker.unescorted_behavior = ship_data->unescorted_behavior;
        tracker.low_hp_behavior = ship_data->low_hp_behavior;
        tracker.role = ship_data->role;
        tracker.requires_escort = (ship_data->role != SHIP_ROLE_LINE);
        double normalized_hp = static_cast<double>(ship_data->max_hp);
        if (normalized_hp <= 0.0)
            normalized_hp = (hp > 0) ? static_cast<double>(hp) : 1.0;
        tracker.hp_ratio = static_cast<double>(hp) / normalized_hp;
        if (tracker.hp_ratio < 0.0)
            tracker.hp_ratio = 0.0;
        if (tracker.hp_ratio > 1.0)
            tracker.hp_ratio = 1.0;
        double normalized_shield = static_cast<double>(ship_data->max_shield);
        if (normalized_shield <= 0.0)
            tracker.shield_ratio = (shield > 0) ? 1.0 : 0.0;
        else
        {
            tracker.shield_ratio = static_cast<double>(shield) / normalized_shield;
            if (tracker.shield_ratio < 0.0)
                tracker.shield_ratio = 0.0;
            if (tracker.shield_ratio > 1.0)
                tracker.shield_ratio = 1.0;
        }
        double max_runtime_speed = tracker.max_speed * 1.5;
        if (tracker.current_speed > max_runtime_speed)
            tracker.current_speed = max_runtime_speed;
    }
    ft_vector<int> to_remove;
    size_t stored = encounter.raider_tracks.size();
    if (stored == 0)
        return ;
    Pair<int, ft_ship_tracker> *entries = encounter.raider_tracks.end();
    entries -= stored;
    for (size_t i = 0; i < stored; ++i)
    {
        bool found = false;
        for (size_t j = 0; j < active_ids.size(); ++j)
        {
            if (entries[i].key == active_ids[j])
            {
                found = true;
                break;
            }
        }
        if (!found)
            to_remove.push_back(entries[i].key);
    }
    for (size_t i = 0; i < to_remove.size(); ++i)
        encounter.raider_tracks.remove(to_remove[i]);
}

void CombatManager::sync_defender_tracks(ft_combat_encounter &encounter,
    const ft_vector<ft_sharedptr<ft_fleet> > &defenders)
{
    ft_vector<int> active_ids;
    encounter.defender_operational_ships = 0;
    encounter.defender_line_ships = 0;
    encounter.defender_support_ships = 0;
    for (size_t i = 0; i < defenders.size(); ++i)
    {
        ft_sharedptr<ft_fleet> fleet = defenders[i];
        if (!fleet)
            continue;
        ft_vector<int> ship_ids;
        fleet->get_ship_ids(ship_ids);
        for (size_t j = 0; j < ship_ids.size(); ++j)
        {
            int ship_uid = ship_ids[j];
            int hp = fleet->get_ship_hp(ship_uid);
            int shield = fleet->get_ship_shield(ship_uid);
            if (hp <= 0 && shield <= 0)
            {
                encounter.defender_tracks.remove(ship_uid);
                continue;
            }
            active_ids.push_back(ship_uid);
            Pair<int, ft_ship_tracker> *entry = encounter.defender_tracks.find(ship_uid);
            const ft_ship *ship_data = fleet->get_ship(ship_uid);
            if (ship_data == ft_nullptr)
            {
                encounter.defender_tracks.remove(ship_uid);
                continue;
            }
            encounter.defender_operational_ships += 1;
            if (ship_data->role == SHIP_ROLE_LINE)
                encounter.defender_line_ships += 1;
            else
                encounter.defender_support_ships += 1;
            if (entry == ft_nullptr)
            {
                ft_ship_tracker tracker;
                this->initialize_tracker(tracker, ship_uid, *ship_data, false, encounter);
                encounter.defender_tracks.insert(ship_uid, tracker);
                entry = encounter.defender_tracks.find(ship_uid);
                if (entry == ft_nullptr)
                    continue;
            }
            else if (entry->value.spatial.ship_type != ship_data->type
                || entry->value.role != ship_data->role)
            {
                this->initialize_tracker(entry->value, ship_uid, *ship_data, false, encounter);
            }
            ft_ship_tracker &tracker = entry->value;
            tracker.spatial.ship_type = ship_data->type;
            tracker.max_hp = ship_data->max_hp;
            tracker.max_shield = ship_data->max_shield;
            tracker.max_speed = ship_data->max_speed;
            if (tracker.max_speed < 4.0)
                tracker.max_speed = 4.0;
            tracker.acceleration = ship_data->acceleration;
            if (tracker.acceleration < 0.5)
                tracker.acceleration = 0.5;
            tracker.turn_speed = ship_data->turn_speed;
            if (tracker.turn_speed < 10.0)
                tracker.turn_speed = 10.0;
            tracker.normal_behavior = ship_data->combat_behavior;
            tracker.outnumbered_behavior = ship_data->outnumbered_behavior;
            tracker.unescorted_behavior = ship_data->unescorted_behavior;
            tracker.low_hp_behavior = ship_data->low_hp_behavior;
            tracker.role = ship_data->role;
            tracker.requires_escort = (ship_data->role != SHIP_ROLE_LINE);
            double normalized_hp = static_cast<double>(ship_data->max_hp);
            if (normalized_hp <= 0.0)
                normalized_hp = (hp > 0) ? static_cast<double>(hp) : 1.0;
            tracker.hp_ratio = static_cast<double>(hp) / normalized_hp;
            if (tracker.hp_ratio < 0.0)
                tracker.hp_ratio = 0.0;
            if (tracker.hp_ratio > 1.0)
                tracker.hp_ratio = 1.0;
            double normalized_shield = static_cast<double>(ship_data->max_shield);
            if (normalized_shield <= 0.0)
                tracker.shield_ratio = (shield > 0) ? 1.0 : 0.0;
            else
            {
                tracker.shield_ratio = static_cast<double>(shield) / normalized_shield;
                if (tracker.shield_ratio < 0.0)
                    tracker.shield_ratio = 0.0;
                if (tracker.shield_ratio > 1.0)
                    tracker.shield_ratio = 1.0;
            }
            double max_runtime_speed = tracker.max_speed * 1.5;
            if (tracker.current_speed > max_runtime_speed)
                tracker.current_speed = max_runtime_speed;
        }
    }
    ft_vector<int> to_remove;
    size_t stored = encounter.defender_tracks.size();
    if (stored == 0)
        return ;
    Pair<int, ft_ship_tracker> *entries = encounter.defender_tracks.end();
    entries -= stored;
    for (size_t i = 0; i < stored; ++i)
    {
        bool found = false;
        for (size_t j = 0; j < active_ids.size(); ++j)
        {
            if (entries[i].key == active_ids[j])
            {
                found = true;
                break;
            }
        }
        if (!found)
            to_remove.push_back(entries[i].key);
    }
    for (size_t i = 0; i < to_remove.size(); ++i)
        encounter.defender_tracks.remove(to_remove[i]);
}

void CombatManager::initialize_tracker(ft_ship_tracker &tracker, int ship_uid,
    const ft_ship &ship, bool raider_side,
    const ft_combat_encounter &encounter)
{
    tracker.spatial.ship_uid = ship_uid;
    tracker.spatial.ship_type = ship.type;
    tracker.base_preferred_radius = raider_side ? 30.0 : 26.0;
    tracker.base_advance_bias = raider_side ? 4.0 : 6.0;
    tracker.preferred_radius = tracker.base_preferred_radius;
    tracker.advance_bias = tracker.base_advance_bias;
    tracker.base_flank = false;
    tracker.flank = false;
    int lane_seed = ship_uid % 9;
    tracker.lane_offset = static_cast<double>(lane_seed) * 9.0 - 36.0;
    int layer_seed = (ship_uid / 9) % 5;
    tracker.vertical_layer = static_cast<double>(layer_seed) * 2.5 - 5.0;
    tracker.drift_origin = static_cast<double>((ship_uid % 11)) * 0.37;
    tracker.drift_speed = raider_side ? 0.85 : 0.65;
    tracker.max_speed = ship.max_speed;
    if (tracker.max_speed < 4.0)
        tracker.max_speed = 4.0;
    tracker.acceleration = ship.acceleration;
    if (tracker.acceleration < 0.5)
        tracker.acceleration = 0.5;
    tracker.turn_speed = ship.turn_speed;
    if (tracker.turn_speed < 10.0)
        tracker.turn_speed = 10.0;
    tracker.current_speed = tracker.max_speed * 0.25;
    tracker.requires_escort = (ship.role != SHIP_ROLE_LINE);
    tracker.role = ship.role;
    tracker.max_hp = ship.max_hp;
    tracker.max_shield = ship.max_shield;
    tracker.normal_behavior = ship.combat_behavior;
    tracker.outnumbered_behavior = ship.outnumbered_behavior;
    tracker.unescorted_behavior = ship.unescorted_behavior;
    tracker.low_hp_behavior = ship.low_hp_behavior;
    tracker.hp_ratio = 1.0;
    tracker.shield_ratio = 1.0;
    tracker.heading_x = 0.0;
    tracker.heading_y = 0.0;
    tracker.heading_z = 0.0;
    if (ship.role == SHIP_ROLE_TRANSPORT)
        tracker.base_advance_bias -= 4.0;
    if (ship.type == SHIP_CAPITAL)
    {
        tracker.base_preferred_radius = raider_side ? 18.0 : 12.0;
        tracker.base_advance_bias = raider_side ? -6.0 : 18.0;
        tracker.drift_speed += 0.25;
        tracker.base_flank = true;
        tracker.lane_offset *= raider_side ? 1.2 : 0.9;
        tracker.vertical_layer *= 0.5;
    }
    else if (ship.type == SHIP_RADAR)
    {
        tracker.base_preferred_radius = raider_side ? 26.0 : 32.0;
        tracker.base_advance_bias = 2.0;
        tracker.drift_speed += 0.18;
        tracker.base_flank = true;
    }
    else if (ship.type == SHIP_SHIELD)
    {
        tracker.base_preferred_radius = raider_side ? 32.0 : 22.0;
        tracker.base_advance_bias = 8.0;
        tracker.drift_speed += 0.1;
        tracker.vertical_layer += raider_side ? 2.0 : 1.5;
    }
    else if (ship.type == SHIP_SALVAGE)
    {
        tracker.base_preferred_radius = raider_side ? 46.0 : 48.0;
        tracker.base_advance_bias = raider_side ? 12.0 : -8.0;
        tracker.drift_speed = 0.42;
        tracker.vertical_layer += raider_side ? 6.0 : 4.0;
    }
    tracker.preferred_radius = tracker.base_preferred_radius;
    if (tracker.preferred_radius < 4.0)
        tracker.preferred_radius = 4.0;
    tracker.advance_bias = tracker.base_advance_bias;
    tracker.flank = tracker.base_flank;
    double init_x;
    double init_y;
    double init_z;
    this->compute_target(encounter, tracker, raider_side, false, init_x, init_y, init_z);
    tracker.spatial.x = init_x;
    tracker.spatial.y = init_y;
    tracker.spatial.z = init_z;
}

void CombatManager::update_tracks(ft_map<int, ft_ship_tracker> &tracks,
    ft_combat_encounter &encounter, double seconds, bool raider_side,
    bool spike_active)
{
    size_t count = tracks.size();
    if (count == 0 || seconds <= 0.0)
        return ;
    Pair<int, ft_ship_tracker> *entries = tracks.end();
    entries -= count;
    double allied = raider_side ? static_cast<double>(encounter.raider_operational_ships)
        : static_cast<double>(encounter.defender_operational_ships);
    double enemy = raider_side ? static_cast<double>(encounter.defender_operational_ships)
        : static_cast<double>(encounter.raider_operational_ships);
    int escorts = raider_side ? encounter.raider_line_ships
        : encounter.defender_line_ships;
    double aggression = raider_side ? encounter.raider_aggression : 1.0;
    if (aggression < 0.2)
        aggression = 0.2;
    if (aggression > 2.5)
        aggression = 2.5;
    for (size_t i = 0; i < count; ++i)
    {
        ft_ship_tracker &tracker = entries[i].value;
        double radius = tracker.base_preferred_radius;
        double advance = tracker.base_advance_bias;
        bool flank = tracker.base_flank;
        double desired_speed = tracker.max_speed;
        bool outnumbered = false;
        double outnumbered_threshold = 1.2 + (aggression - 1.0) * 0.4;
        if (outnumbered_threshold < 1.05)
            outnumbered_threshold = 1.05;
        if (outnumbered_threshold > 1.9)
            outnumbered_threshold = 1.9;
        if (allied > 0.0 && enemy > allied * outnumbered_threshold)
            outnumbered = true;
        bool unescorted = tracker.requires_escort && escorts <= 0;
        if (raider_side && aggression > 1.2 && tracker.role != SHIP_ROLE_TRANSPORT)
            unescorted = tracker.requires_escort && escorts <= -1;
        double effective_ratio = tracker.hp_ratio + tracker.shield_ratio * 0.3;
        if (effective_ratio > 1.0)
            effective_ratio = 1.0;
        if (effective_ratio < 0.0)
            effective_ratio = 0.0;
        double low_hp_threshold = 0.45 - (aggression - 1.0) * 0.18;
        if (low_hp_threshold < 0.18)
            low_hp_threshold = 0.18;
        if (low_hp_threshold > 0.7)
            low_hp_threshold = 0.7;
        bool low_hp = effective_ratio < low_hp_threshold;
        int behavior = tracker.normal_behavior;
        if (low_hp && tracker.low_hp_behavior != 0)
            behavior = tracker.low_hp_behavior;
        else if (unescorted && tracker.unescorted_behavior != 0)
            behavior = tracker.unescorted_behavior;
        else if (outnumbered && tracker.outnumbered_behavior != 0)
            behavior = tracker.outnumbered_behavior;
        if (raider_side)
        {
            if (aggression > 1.2)
            {
                if (behavior == SHIP_BEHAVIOR_RETREAT
                    || behavior == SHIP_BEHAVIOR_WITHDRAW_SUPPORT)
                {
                    if (aggression >= 1.5 && tracker.role == SHIP_ROLE_LINE)
                        behavior = SHIP_BEHAVIOR_CHARGE;
                    else
                        behavior = SHIP_BEHAVIOR_LINE_HOLD;
                }
            }
            else if (aggression < 0.85)
            {
                if (behavior == SHIP_BEHAVIOR_CHARGE)
                    behavior = SHIP_BEHAVIOR_LINE_HOLD;
                else if (behavior == SHIP_BEHAVIOR_LINE_HOLD)
                    behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            }
        }
        this->apply_behavior_adjustments(tracker, behavior, raider_side,
            spike_active, aggression, radius, advance, flank, desired_speed);
        if (spike_active && raider_side)
            desired_speed += 2.0;
        if (!raider_side && encounter.control_mode == ASSAULT_CONTROL_ACTIVE
            && encounter.manual_focus_remaining > 0.0)
            desired_speed += 4.0;
        double max_speed_limit = tracker.max_speed * 1.35;
        if (desired_speed > max_speed_limit)
            desired_speed = max_speed_limit;
        if (desired_speed < 0.0)
            desired_speed = 0.0;
        tracker.preferred_radius = radius;
        tracker.advance_bias = advance;
        tracker.flank = flank;
        double target_x;
        double target_y;
        double target_z;
        this->compute_target(encounter, tracker, raider_side, spike_active,
            target_x, target_y, target_z);
        double dx = target_x - tracker.spatial.x;
        double dy = target_y - tracker.spatial.y;
        double dz = target_z - tracker.spatial.z;
        double distance_sq = dx * dx + dy * dy + dz * dz;
        if (distance_sq < 0.0001)
        {
            tracker.spatial.x = target_x;
            tracker.spatial.y = target_y;
            tracker.spatial.z = target_z;
            continue;
        }
        double distance = std::sqrt(distance_sq);
        double speed_step = tracker.acceleration * seconds;
        if (speed_step < 0.0)
            speed_step = 0.0;
        if (tracker.current_speed < desired_speed)
        {
            tracker.current_speed += speed_step;
            if (tracker.current_speed > desired_speed)
                tracker.current_speed = desired_speed;
        }
        else
        {
            tracker.current_speed -= speed_step;
            if (tracker.current_speed < desired_speed)
                tracker.current_speed = desired_speed;
        }
        if (tracker.current_speed < 0.0)
            tracker.current_speed = 0.0;
        double allowed = tracker.current_speed * seconds;
        if (allowed <= 0.0)
            continue;
        double desired_x = dx / distance;
        double desired_y = dy / distance;
        double desired_z = dz / distance;
        double heading_length = std::sqrt(tracker.heading_x * tracker.heading_x
            + tracker.heading_y * tracker.heading_y
            + tracker.heading_z * tracker.heading_z);
        double max_turn_rad = tracker.turn_speed * seconds * FT_DEG_TO_RAD;
        if (heading_length > 0.0 && max_turn_rad > 0.0)
        {
            double dot = tracker.heading_x * desired_x
                + tracker.heading_y * desired_y
                + tracker.heading_z * desired_z;
            if (dot > 1.0)
                dot = 1.0;
            else if (dot < -1.0)
                dot = -1.0;
            double angle = std::acos(dot);
            if (angle > max_turn_rad)
            {
                double ratio_limit = max_turn_rad / angle;
                if (ratio_limit < 0.2)
                    ratio_limit = 0.2;
                allowed *= ratio_limit;
            }
        }
        if (allowed <= 0.0)
            continue;
        tracker.heading_x = desired_x;
        tracker.heading_y = desired_y;
        tracker.heading_z = desired_z;
        double ratio = allowed / distance;
        if (ratio >= 1.0)
        {
            tracker.spatial.x = target_x;
            tracker.spatial.y = target_y;
            tracker.spatial.z = target_z;
        }
        else
        {
            tracker.spatial.x += dx * ratio;
            tracker.spatial.y += dy * ratio;
            tracker.spatial.z += dz * ratio;
        }
    }
}

void CombatManager::update_formations(ft_combat_encounter &encounter, double seconds,
    bool spike_active)
{
    if (seconds <= 0.0)
        return ;
    encounter.formation_time += seconds;
    if (encounter.formation_time > 4096.0)
        encounter.formation_time = std::fmod(encounter.formation_time, FT_TWO_PI);
    double push = 18.0 + encounter.attack_multiplier * 4.0;
    push += encounter.energy_pressure * 6.0;
    push += encounter.narrative_pressure * 5.0;
    double aggression = encounter.raider_aggression;
    if (aggression < 0.2)
        aggression = 0.2;
    if (aggression > 2.5)
        aggression = 2.5;
    double aggression_scale = 0.75 + aggression * 0.25;
    push *= aggression_scale;
    if (spike_active)
        push += 10.0;
    if (encounter.control_mode == ASSAULT_CONTROL_ACTIVE)
    {
        push *= 0.92;
        if (encounter.tactical_pause_remaining > 0.0)
            push *= 0.5;
    }
    double defender_boost = 0.0;
    if (encounter.control_mode == ASSAULT_CONTROL_ACTIVE && encounter.manual_focus_remaining > 0.0)
        defender_boost = 3.5;
    encounter.raider_frontline -= push * seconds * 0.1;
    encounter.raider_frontline += defender_boost * seconds;
    if (encounter.raider_frontline < 16.0)
        encounter.raider_frontline = 16.0;
    if (encounter.raider_frontline > 160.0)
        encounter.raider_frontline = 160.0;
    double target_line = -encounter.raider_frontline * 0.55 - 10.0;
    if (encounter.control_mode == ASSAULT_CONTROL_ACTIVE)
    {
        if (encounter.manual_focus_remaining > 0.0)
            target_line += 4.0;
        if (encounter.tactical_pause_remaining > 0.0)
            target_line += 2.0;
    }
    if (target_line > -18.0)
        target_line = -18.0;
    if (target_line < -110.0)
        target_line = -110.0;
    double delta = target_line - encounter.defender_line;
    double adjust = seconds * 18.0;
    if (delta > adjust)
        delta = adjust;
    else if (delta < -adjust)
        delta = -adjust;
    encounter.defender_line += delta;
    this->update_tracks(encounter.raider_tracks, encounter, seconds, true, spike_active);
    this->update_tracks(encounter.defender_tracks, encounter, seconds, false, spike_active);
}

void CombatManager::compute_target(const ft_combat_encounter &encounter,
    const ft_ship_tracker &tracker, bool raider_side, bool spike_active,
    double &out_x, double &out_y, double &out_z) const
{
    double phase = encounter.formation_time * tracker.drift_speed + tracker.drift_origin;
    double sway = std::sin(phase);
    double lift = std::cos(phase * 0.6);
    double flank_scale = tracker.flank ? 13.0 : 6.5;
    out_x = tracker.lane_offset + sway * flank_scale;
    out_y = tracker.vertical_layer + lift * (tracker.flank ? 3.5 : 1.8);
    if (raider_side)
    {
        double baseline = encounter.raider_frontline - tracker.preferred_radius + tracker.advance_bias;
        if (spike_active)
            baseline -= 5.0;
        if (baseline < 4.0)
            baseline = 4.0;
        if (baseline > 200.0)
            baseline = 200.0;
        out_z = baseline;
    }
    else
    {
        double baseline = encounter.defender_line - tracker.preferred_radius + tracker.advance_bias;
        if (encounter.control_mode == ASSAULT_CONTROL_ACTIVE && encounter.manual_focus_remaining > 0.0)
            baseline += 3.0;
        if (encounter.control_mode == ASSAULT_CONTROL_ACTIVE && encounter.tactical_pause_remaining > 0.0)
            baseline += 2.5;
        if (baseline > -2.0)
            baseline = -2.0;
        if (baseline < -200.0)
            baseline = -200.0;
        out_z = baseline;
    }
}

void CombatManager::apply_behavior_adjustments(const ft_ship_tracker &tracker, int behavior,
    bool raider_side, bool spike_active, double aggression, double &radius,
    double &advance, bool &flank, double &desired_speed) const
{
    (void)tracker;
    if (behavior <= 0)
        return ;
    if (aggression < 0.2)
        aggression = 0.2;
    if (aggression > 2.5)
        aggression = 2.5;
    switch (behavior)
    {
    case SHIP_BEHAVIOR_LINE_HOLD:
        radius += raider_side ? -2.0 : -1.0;
        desired_speed *= 0.85;
        break;
    case SHIP_BEHAVIOR_FLANK_SWEEP:
        flank = true;
        radius += 8.0;
        advance += raider_side ? 6.0 : 4.0;
        desired_speed *= 1.05;
        break;
    case SHIP_BEHAVIOR_SCREEN_SUPPORT:
        flank = false;
        radius += raider_side ? 10.0 : 8.0;
        advance -= 6.0;
        desired_speed *= 0.75;
        break;
    case SHIP_BEHAVIOR_CHARGE:
        radius *= 0.85;
        advance += raider_side ? 14.0 : 12.0;
        desired_speed *= 1.08;
        break;
    case SHIP_BEHAVIOR_RETREAT:
        radius += 12.0;
        advance -= raider_side ? 8.0 : 6.0;
        desired_speed *= 0.92;
        break;
    case SHIP_BEHAVIOR_WITHDRAW_SUPPORT:
        flank = false;
        radius += 20.0;
        advance -= raider_side ? 14.0 : 12.0;
        desired_speed *= 0.88;
        break;
    case SHIP_BEHAVIOR_LAST_STAND:
        radius *= 0.70;
        advance += raider_side ? 18.0 : 14.0;
        desired_speed *= 1.12;
        break;
    default:
        break;
    }
    if (spike_active && raider_side && behavior != SHIP_BEHAVIOR_WITHDRAW_SUPPORT)
        desired_speed *= 1.02;
    if (raider_side)
    {
        double diff = aggression - 1.0;
        if (diff > 0.0)
        {
            advance += diff * 10.0;
            radius -= diff * 6.0;
            desired_speed *= (1.0 + diff * 0.18);
        }
        else if (diff < 0.0)
        {
            double caution = -diff;
            advance -= caution * 8.0;
            radius += caution * 12.0;
            desired_speed *= (1.0 - caution * 0.22);
        }
    }
    else if (aggression < 1.0)
    {
        double caution = 1.0 - aggression;
        radius += caution * 4.0;
    }
    if (desired_speed < 0.0)
        desired_speed = 0.0;
    if (radius < 4.0)
        radius = 4.0;
    if (radius > 280.0)
        radius = 280.0;
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
        if (encounter.control_mode == ASSAULT_CONTROL_ACTIVE)
        {
            if (encounter.manual_focus_cooldown > 0.0)
            {
                encounter.manual_focus_cooldown -= seconds;
                if (encounter.manual_focus_cooldown < 0.0)
                    encounter.manual_focus_cooldown = 0.0;
            }
            if (encounter.tactical_pause_cooldown > 0.0)
            {
                encounter.tactical_pause_cooldown -= seconds;
                if (encounter.tactical_pause_cooldown < 0.0)
                    encounter.tactical_pause_cooldown = 0.0;
            }
        }
        else
        {
            encounter.manual_focus_remaining = 0.0;
            encounter.manual_focus_cooldown = 0.0;
            encounter.tactical_pause_remaining = 0.0;
            encounter.tactical_pause_cooldown = 0.0;
        }
        encounter.elapsed += seconds;
        ft_vector<ft_sharedptr<ft_fleet> > defenders;
        this->gather_defenders(encounter, fleets, planet_fleets, defenders);
        this->sync_raider_tracks(encounter);
        if (defenders.size() == 0)
        {
            failed.push_back(encounter.planet_id);
            to_remove.push_back(entries[i].key);
            continue;
        }
        this->sync_defender_tracks(encounter, defenders);
        bool spike_active = false;
        if (encounter.spike_time_remaining > 0.0)
            spike_active = true;
        double time_leftover = seconds;
        if (time_leftover < 0.0)
            time_leftover = 0.0;
        if (encounter.spike_time_remaining > 0.0 && time_leftover > 0.0)
        {
            if (encounter.spike_time_remaining > time_leftover)
                encounter.spike_time_remaining -= time_leftover;
            else
            {
                time_leftover -= encounter.spike_time_remaining;
                encounter.spike_time_remaining = 0.0;
            }
        }
        if (time_leftover > 0.0)
        {
            encounter.spike_timer += time_leftover;
            double spike_threshold = 30.0 - encounter.narrative_pressure * 8.0;
            if (spike_threshold < 12.0)
                spike_threshold = 12.0;
            if (encounter.spike_timer >= spike_threshold)
            {
                encounter.spike_time_remaining = 4.0 + encounter.narrative_pressure * 4.0;
                if (encounter.energy_pressure > 0.0)
                    encounter.spike_time_remaining += encounter.energy_pressure * 2.0;
                encounter.spike_timer = 0.0;
                spike_active = true;
            }
        }
        this->update_formations(encounter, seconds, spike_active);
        double player_damage = this->calculate_player_power(defenders) * seconds * this->_player_weapon_multiplier;
        if (encounter.control_mode == ASSAULT_CONTROL_ACTIVE)
        {
            double control_bonus = 0.1;
            if (encounter.energy_pressure > 0.0)
                control_bonus += encounter.energy_pressure * 0.05;
            if (encounter.narrative_pressure > 0.0)
                control_bonus += encounter.narrative_pressure * 0.05;
            player_damage *= (1.0 + control_bonus);
            if (encounter.manual_focus_remaining > 0.0)
                player_damage *= 1.35;
        }
        if (player_damage > 0.0 && encounter.raider_fleet)
        {
            double effective_player_damage = player_damage;
            if (encounter.defense_multiplier > 0.0)
                effective_player_damage /= encounter.defense_multiplier;
            if (effective_player_damage < 0.0)
                effective_player_damage = 0.0;
            encounter.raider_fleet->absorb_damage(effective_player_damage, 1.0, 1.0);
        }
        if (!encounter.raider_fleet || !encounter.raider_fleet->has_operational_ships())
        {
            completed.push_back(encounter.planet_id);
            to_remove.push_back(entries[i].key);
            continue;
        }
        double damage_scale = encounter.attack_multiplier;
        if (damage_scale < 0.5)
            damage_scale = 0.5;
        if (encounter.control_mode == ASSAULT_CONTROL_ACTIVE)
            damage_scale *= 0.9;
        if (spike_active)
        {
            double spike_bonus = 0.25;
            if (encounter.narrative_pressure > 0.0)
                spike_bonus += encounter.narrative_pressure * 0.35;
            if (encounter.energy_pressure > 0.0)
                spike_bonus += encounter.energy_pressure * 0.2;
            damage_scale += spike_bonus;
        }
        double enemy_power = 0.0;
        if (encounter.raider_fleet)
            enemy_power = encounter.raider_fleet->get_attack_power();
        double intensity = 1.0 + encounter.elapsed / 45.0;
        double raider_damage = enemy_power * intensity * damage_scale * seconds;
        if (encounter.control_mode == ASSAULT_CONTROL_ACTIVE)
        {
            double mitigation = 0.0;
            if (encounter.narrative_pressure > 0.0)
                mitigation += encounter.narrative_pressure * 0.08;
            if (encounter.energy_pressure > 0.0)
                mitigation += encounter.energy_pressure * 0.05;
            if (mitigation > 0.35)
                mitigation = 0.35;
            if (mitigation > 0.0)
                raider_damage *= (1.0 - mitigation);
            if (encounter.tactical_pause_remaining > 0.0)
                raider_damage *= 0.2;
        }
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
        this->apply_raider_support(encounter, seconds, spike_active);
        if (encounter.control_mode == ASSAULT_CONTROL_ACTIVE)
        {
            if (encounter.manual_focus_remaining > 0.0)
            {
                encounter.manual_focus_remaining -= seconds;
                if (encounter.manual_focus_remaining < 0.0)
                    encounter.manual_focus_remaining = 0.0;
            }
            if (encounter.tactical_pause_remaining > 0.0)
            {
                encounter.tactical_pause_remaining -= seconds;
                if (encounter.tactical_pause_remaining < 0.0)
                    encounter.tactical_pause_remaining = 0.0;
            }
        }
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
