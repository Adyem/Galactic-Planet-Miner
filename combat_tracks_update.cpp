void CombatManager::update_tracks(ft_map<int, ft_ship_tracker> &tracks,
    ft_combat_encounter &encounter, double seconds, bool raider_side,
    bool spike_active)
{
    static const double FT_TWO_PI = 6.28318530717958647692;
    static const double FT_DEG_TO_RAD = FT_TWO_PI / 360.0;

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
        double distance = math_sqrt(distance_sq);
        double accel_step = tracker.acceleration * seconds;
        if (accel_step < 0.0)
            accel_step = 0.0;
        double decel_step = tracker.deceleration * seconds;
        if (decel_step < 0.0)
            decel_step = 0.0;
        if (tracker.current_speed < desired_speed)
        {
            tracker.current_speed += accel_step;
            if (tracker.current_speed > desired_speed)
                tracker.current_speed = desired_speed;
        }
        else
        {
            tracker.current_speed -= decel_step;
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
        double heading_length = math_sqrt(tracker.heading_x * tracker.heading_x
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
            double angle = math_acos(dot);
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
