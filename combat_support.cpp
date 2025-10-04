#include "combat.hpp"

namespace
{
    double compute_range_factor(double distance, double optimal, double maximum)
    {
        if (maximum <= 0.0)
            return 1.0;
        if (distance <= 0.0)
            return 1.0;
        if (distance <= optimal)
            return 1.0;
        if (distance >= maximum)
            return 0.0;
        double window = maximum - optimal;
        if (window <= 0.0)
            return (distance <= maximum) ? 1.0 : 0.0;
        double falloff = 1.0 - (distance - optimal) / window;
        if (falloff < 0.0)
            falloff = 0.0;
        if (falloff > 1.0)
            falloff = 1.0;
        return falloff;
    }
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

double CombatManager::compute_tracker_contribution(const ft_ship_tracker &tracker,
    double distance) const
{
    if (tracker.base_damage <= 0.0)
        return 0.0;
    if (tracker.hp_ratio <= 0.0)
        return 0.0;
    if (distance < 0.0)
        distance = 0.0;
    double range_factor = compute_range_factor(distance, tracker.optimal_range, tracker.max_range);
    if (range_factor <= 0.0)
        return 0.0;
    double efficiency = tracker.hp_ratio;
    if (efficiency < 0.1)
        efficiency = 0.1;
    if (efficiency > 1.0)
        efficiency = 1.0;
    return tracker.base_damage * efficiency * range_factor;
}

double CombatManager::calculate_side_power(const ft_map<int, ft_ship_tracker> &tracks,
    double opposing_frontline, bool raider_side) const
{
    size_t count = tracks.size();
    if (count == 0)
        return 0.0;
    const Pair<int, ft_ship_tracker> *entries = tracks.end();
    entries -= count;
    double total = 0.0;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_ship_tracker &tracker = entries[i].value;
        double distance;
        if (raider_side)
            distance = tracker.spatial.z - opposing_frontline;
        else
            distance = opposing_frontline - tracker.spatial.z;
        total += this->compute_tracker_contribution(tracker, distance);
    }
    return total;
}

double CombatManager::calculate_player_power(const ft_combat_encounter &encounter) const
{
    return this->calculate_side_power(encounter.defender_tracks, encounter.raider_frontline, false);
}

double CombatManager::calculate_raider_power(const ft_combat_encounter &encounter) const
{
    return this->calculate_side_power(encounter.raider_tracks, encounter.defender_line, true);
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
    this->add_raider_ship(fleet, SHIP_RAIDER_CORVETTE, 88, 28, 16, scale);
    this->add_raider_ship(fleet, SHIP_RAIDER_CORVETTE, 88, 28, 16, scale);
    this->add_raider_ship(fleet, SHIP_RAIDER_DESTROYER, 132, 46, 22, scale);
    this->add_raider_ship(fleet, SHIP_SHIELD, 52, 32, 10, scale);
    this->add_raider_ship(fleet, SHIP_SALVAGE, 50, 10, 18, scale);

    double energy_remaining = energy_pressure;
    while (energy_remaining >= 0.8)
    {
        this->add_raider_ship(fleet, SHIP_RAIDER_DESTROYER, 126, 42, 20, scale);
        energy_remaining -= 0.8;
    }
    if (energy_remaining >= 0.4)
        this->add_raider_ship(fleet, SHIP_RAIDER_CORVETTE, 84, 26, 16, scale);
    if (energy_pressure >= 1.0)
        this->add_raider_ship(fleet, SHIP_RAIDER_DESTROYER, 140, 52, 24, scale);

    int heavy = 0;
    if (narrative_pressure >= 0.3)
        heavy = 1;
    if (narrative_pressure >= 0.9)
        heavy = 2;
    for (int i = 0; i < heavy; ++i)
        this->add_raider_ship(fleet, SHIP_RAIDER_BATTLESHIP, 220, 90, 40, scale);

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

void CombatManager::apply_support(ft_combat_encounter &encounter,
    ft_vector<ft_sharedptr<ft_fleet> > &defenders,
    double seconds)
{
    bool auto_candidate = (encounter.control_mode == ASSAULT_CONTROL_AUTO);
    if (!auto_candidate && !encounter.modifiers.sunflare_docked
        && !encounter.modifiers.repair_drones_active)
    {
        encounter.auto_repair_drones_active = false;
        return ;
    }
    if (seconds < 0.0)
        seconds = 0.0;
    if (seconds == 0.0 && !auto_candidate)
        return ;
    int sunflare_count = 0;
    int drone_count = 0;
    int total_missing_hp = 0;
    int total_max_hp = 0;
    int severely_wounded = 0;
    ft_vector<int> ship_ids;
    size_t defender_count = defenders.size();
    for (size_t i = 0; i < defender_count; ++i)
    {
        ft_sharedptr<ft_fleet> &fleet = defenders[i];
        if (!fleet)
            continue;
        fleet->get_ship_ids(ship_ids);
        for (size_t j = 0; j < ship_ids.size(); ++j)
        {
            int ship_uid = ship_ids[j];
            const ft_ship *ship = fleet->get_ship(ship_uid);
            if (ship == ft_nullptr)
                continue;
            if (ship->hp <= 0 && ship->shield <= 0)
                continue;
            switch (ship->type)
            {
            case SHIP_SUNFLARE_SLOOP:
                sunflare_count += 1;
                break;
            case SHIP_FRIGATE_CARRIER:
            case SHIP_FRIGATE_PROTECTOR:
                sunflare_count += 1;
                drone_count += 1;
                break;
            case SHIP_FRIGATE_ECLIPSE:
                drone_count += 1;
                break;
            case SHIP_CAPITAL_NOVA:
            case SHIP_CAPITAL_PROTECTOR:
                sunflare_count += 2;
                drone_count += 1;
                break;
            case SHIP_CAPITAL_ECLIPSE:
                drone_count += 2;
                break;
            default:
                break;
            }
            if (ship->type == SHIP_REPAIR_DRONE)
                drone_count += 1;
            if (ship->hp > 0 && ship->max_hp > 0)
            {
                total_max_hp += ship->max_hp;
                int missing = ship->max_hp - ship->hp;
                if (missing > 0)
                {
                    total_missing_hp += missing;
                    double severe_threshold = static_cast<double>(ship->max_hp) * 0.75;
                    if (static_cast<double>(ship->hp) < severe_threshold)
                        severely_wounded += 1;
                }
            }
        }
    }
    double shield_rate = 0.0;
    double targeted_rate = 0.0;
    double repair_rate = 0.0;
    bool auto_repair = false;
    if (auto_candidate && drone_count > 0)
    {
        if (total_missing_hp > 0)
        {
            if (severely_wounded > 0)
                auto_repair = true;
            else if (total_missing_hp >= 12)
                auto_repair = true;
            else if (total_max_hp > 0)
            {
                double missing_ratio = static_cast<double>(total_missing_hp) / static_cast<double>(total_max_hp);
                if (missing_ratio >= 0.03)
                    auto_repair = true;
            }
            if (!auto_repair && encounter.elapsed > 20.0)
                auto_repair = true;
        }
    }
    if (!auto_candidate)
        auto_repair = false;
    encounter.auto_repair_drones_active = auto_repair;
    if (encounter.modifiers.sunflare_docked && sunflare_count > 0)
    {
        double sunflare_rate = 8.0 * static_cast<double>(sunflare_count);
        shield_rate += sunflare_rate;
        if (encounter.sunflare_target_ship_uid != 0)
            targeted_rate = 12.0 * static_cast<double>(sunflare_count);
    }
    double generator_stability = 0.0;
    if (encounter.modifiers.shield_generator_online)
        generator_stability = 1.0;
    if (encounter.auto_generator_stability > generator_stability)
        generator_stability = encounter.auto_generator_stability;
    if (generator_stability > 0.0)
    {
        double stability = generator_stability;
        if (encounter.energy_pressure > 0.0)
        {
            double normalized = encounter.energy_pressure / 1.5;
            if (normalized > 1.0)
                normalized = 1.0;
            double energy_factor = 1.0 - normalized;
            if (energy_factor < 0.0)
                energy_factor = 0.0;
            stability *= energy_factor;
        }
        if (stability > 0.0)
        {
            double throughput = 12.0 * stability;
            double amplification = stability * stability * 6.0;
            shield_rate += throughput + amplification;
        }
    }
    bool repair_enabled = encounter.modifiers.repair_drones_active;
    if (!repair_enabled && auto_repair)
        repair_enabled = true;
    if (repair_enabled && drone_count > 0)
        repair_rate = 10.0 * static_cast<double>(drone_count);
    if (targeted_rate > 0.0)
        encounter.sunflare_focus_pool += targeted_rate * seconds;
    if (shield_rate <= 0.0 && repair_rate <= 0.0 && encounter.sunflare_focus_pool <= 0.0)
        return ;
    int shield_bonus = static_cast<int>(shield_rate * seconds + 0.5);
    int repair_bonus = static_cast<int>(repair_rate * seconds + 0.5);
    int targeted_bonus = 0;
    if (encounter.sunflare_focus_pool > 0.0)
    {
        targeted_bonus = static_cast<int>(encounter.sunflare_focus_pool);
        if (targeted_bonus > 0)
            encounter.sunflare_focus_pool -= static_cast<double>(targeted_bonus);
        if (encounter.sunflare_focus_pool < 0.0)
            encounter.sunflare_focus_pool = 0.0;
    }
    size_t count = defenders.size();
    for (size_t i = 0; i < count; ++i)
    {
        if (!defenders[i])
            continue;
        defenders[i]->apply_support(shield_bonus, repair_bonus);
    }
    if (targeted_bonus > 0 && encounter.sunflare_target_fleet_id != 0)
    {
        int applied = 0;
        for (size_t i = 0; i < count; ++i)
        {
            if (!defenders[i])
                continue;
            if (defenders[i]->get_id() != encounter.sunflare_target_fleet_id)
                continue;
            applied = defenders[i]->apply_targeted_shield(encounter.sunflare_target_ship_uid, targeted_bonus);
            break;
        }
        if (applied < targeted_bonus)
        {
            int remainder = targeted_bonus - applied;
            encounter.sunflare_focus_pool += static_cast<double>(remainder);
        }
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
