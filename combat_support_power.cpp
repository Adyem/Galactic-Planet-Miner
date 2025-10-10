#include "combat.hpp"
#include "libft_math_bridge.hpp"

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
