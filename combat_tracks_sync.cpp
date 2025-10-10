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
        tracker.deceleration = ship_data->deceleration;
        if (tracker.deceleration < 0.3)
            tracker.deceleration = 0.3;
        tracker.turn_speed = ship_data->turn_speed;
        if (tracker.turn_speed < 10.0)
            tracker.turn_speed = 10.0;
        tracker.optimal_range = ship_data->optimal_range;
        tracker.max_range = ship_data->max_range;
        tracker.base_damage = ship_data->base_damage;
        if (tracker.base_damage <= 0.0)
            tracker.base_damage = ft_fleet::get_ship_damage_baseline(ship_data->type);
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
            tracker.deceleration = ship_data->deceleration;
            if (tracker.deceleration < 0.3)
                tracker.deceleration = 0.3;
            tracker.turn_speed = ship_data->turn_speed;
            if (tracker.turn_speed < 10.0)
                tracker.turn_speed = 10.0;
            tracker.optimal_range = ship_data->optimal_range;
            tracker.max_range = ship_data->max_range;
            tracker.base_damage = ship_data->base_damage;
            if (tracker.base_damage <= 0.0)
                tracker.base_damage = ft_fleet::get_ship_damage_baseline(ship_data->type);
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
