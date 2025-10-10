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
    tracker.deceleration = ship.deceleration;
    if (tracker.deceleration < 0.3)
        tracker.deceleration = 0.3;
    tracker.turn_speed = ship.turn_speed;
    if (tracker.turn_speed < 10.0)
        tracker.turn_speed = 10.0;
    tracker.current_speed = tracker.max_speed * 0.25;
    tracker.requires_escort = (ship.role != SHIP_ROLE_LINE);
    tracker.role = ship.role;
    tracker.max_hp = ship.max_hp;
    tracker.max_shield = ship.max_shield;
    tracker.optimal_range = ship.optimal_range;
    tracker.max_range = ship.max_range;
    tracker.base_damage = ship.base_damage;
    if (tracker.base_damage <= 0.0)
        tracker.base_damage = ft_fleet::get_ship_damage_baseline(ship.type);
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
    if (is_capital_ship_type(ship.type))
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
