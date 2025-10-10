#include "combat.hpp"
#include "libft_math_bridge.hpp"

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
    const double seconds_epsilon = 1e-6;
    if (math_fabs(seconds) <= seconds_epsilon && !auto_candidate)
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
