#include "combat.hpp"

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
            double prolonged_adjustment = encounter.elapsed * 0.08;
            if (prolonged_adjustment > 10.0)
                prolonged_adjustment = 10.0;
            spike_threshold -= prolonged_adjustment;
            if (encounter.energy_pressure > 1.2)
                spike_threshold -= (encounter.energy_pressure - 1.2) * 4.0;
            if (spike_threshold < 10.0)
                spike_threshold = 10.0;
            if (encounter.spike_timer >= spike_threshold)
            {
                encounter.spike_time_remaining = 4.0 + encounter.narrative_pressure * 4.0;
                if (encounter.energy_pressure > 0.0)
                    encounter.spike_time_remaining += encounter.energy_pressure * 2.0;
                if (prolonged_adjustment > 0.0)
                    encounter.spike_time_remaining += prolonged_adjustment * 0.1;
                encounter.spike_timer = 0.0;
                spike_active = true;
            }
        }
        this->update_formations(encounter, seconds, spike_active);
        double player_damage = this->calculate_player_power(encounter) * seconds * this->_player_weapon_multiplier;
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
        double enemy_power = this->calculate_raider_power(encounter);
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
