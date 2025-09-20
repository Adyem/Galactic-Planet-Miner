#include "combat.hpp"

#include "libft_math_bridge.hpp"

void CombatManager::update_formations(ft_combat_encounter &encounter, double seconds,
    bool spike_active)
{
    static const double FT_TWO_PI = 6.28318530717958647692;

    if (seconds <= 0.0)
        return ;
    encounter.formation_time += seconds;
    if (encounter.formation_time > 4096.0)
        encounter.formation_time = math_fmod(encounter.formation_time, FT_TWO_PI);
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
    double sway = ft_sin(phase);
    double lift = math_cos(phase * 0.6);
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
