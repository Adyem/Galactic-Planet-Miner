#include "combat.hpp"

#include <cmath>

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
