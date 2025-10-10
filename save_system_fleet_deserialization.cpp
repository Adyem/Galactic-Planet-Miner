        int location_to = to_item ? ft_atoi(to_item->value) : PLANET_TERRA;
        json_item *misc_item = json_find_item(current, "location_misc");
        int location_misc = misc_item ? ft_atoi(misc_item->value) : 0;
        json_item *travel_item = json_find_item(current, "travel_time");
        long travel_scaled = travel_item ? ft_atol(travel_item->value) : 0;
        json_item *veterancy_item = json_find_item(current, "escort_veterancy");
        long veterancy_scaled = veterancy_item ? ft_atol(veterancy_item->value) : 0;
        if (location_type == LOCATION_TRAVEL)
            fleet->set_location_travel(location_from, location_to,
                this->unscale_long_to_double(travel_scaled));
        else if (location_type == LOCATION_MISC)
            fleet->set_location_misc(location_misc);
        else
            fleet->set_location_planet(location_from);
        fleet->set_escort_veterancy(this->unscale_long_to_double(veterancy_scaled));
        json_item *ship_count_item = json_find_item(current, "ship_count");
        int ship_count = ship_count_item ? ft_atoi(ship_count_item->value) : 0;
        if (ship_count < 0)
            ship_count = 0;
        int highest_ship_slot = -1;
        json_item *ship_entry = current->items;
        while (ship_entry)
        {
            if (ship_entry->key && ft_strncmp(ship_entry->key, "ship_", 5) == 0)
            {
                const char *index_start = ship_entry->key + 5;
                const char *cursor = index_start;
                while (*cursor && *cursor >= '0' && *cursor <= '9')
                    ++cursor;
                if (cursor != index_start && ft_strncmp(cursor, "_id", 3) == 0)
                {
                    int slot = ft_atoi(index_start);
                    if (slot < 0)
                    {
                        json_free_groups(groups);
                        return false;
                    }
                    if (slot > highest_ship_slot)
                        highest_ship_slot = slot;
                    if (slot >= SAVE_MAX_SHIPS_PER_FLEET)
                    {
                        json_free_groups(groups);
                        return false;
                    }
                }
            }
            ship_entry = ship_entry->next;
        }
        if (ship_count > SAVE_MAX_SHIPS_PER_FLEET)
            ship_count = SAVE_MAX_SHIPS_PER_FLEET;
        if (highest_ship_slot < 0)
            ship_count = 0;
        else
        {
            int minimum_ship_count = highest_ship_slot + 1;
            if (minimum_ship_count > SAVE_MAX_SHIPS_PER_FLEET)
                minimum_ship_count = SAVE_MAX_SHIPS_PER_FLEET;
            if (ship_count < minimum_ship_count)
                ship_count = minimum_ship_count;
        }
        int missing_streak = 0;
        bool saw_ship = false;
        for (int i = 0; i < ship_count; ++i)
        {
            ft_string index_string = ft_to_string(static_cast<long>(i));
            ft_string base_key = "ship_";
            base_key.append(index_string);
            ft_string key = base_key;
            key.append("_id");
            json_item *ship_id_item = json_find_item(current, key.c_str());
            if (!ship_id_item)
            {
                ++missing_streak;
                if (saw_ship && missing_streak >= 2)
                    break;
                continue;
            }
            missing_streak = 0;
            saw_ship = true;
            ft_ship ship_snapshot;
            ship_snapshot.id = ft_atoi(ship_id_item->value);
            if (ship_snapshot.id < SAVE_SHIP_ID_MIN
                || ship_snapshot.id > SAVE_SHIP_ID_MAX)
            {
                json_free_groups(groups);
                return false;
            }
            key = base_key;
            key.append("_type");
            json_item *ship_type_item = json_find_item(current, key.c_str());
            if (ship_type_item)
                ship_snapshot.type = ft_atoi(ship_type_item->value);
            if (!save_system_is_known_ship_type(ship_snapshot.type))
                continue;
            key = base_key;
            key.append("_armor");
            json_item *armor_item = json_find_item(current, key.c_str());
            if (armor_item)
            {
                if (!save_system_parse_non_negative_int(armor_item->value, ship_snapshot.armor))
                    ship_snapshot.armor = 0;
            }
            else
                ship_snapshot.armor = 0;
            key = base_key;
            key.append("_hp");
            json_item *hp_item = json_find_item(current, key.c_str());
            key = base_key;
            key.append("_shield");
            json_item *shield_item = json_find_item(current, key.c_str());
            key = base_key;
            key.append("_max_hp");
            json_item *max_hp_item = json_find_item(current, key.c_str());
            if (!max_hp_item)
                continue;
            int max_hp_value = 0;
            if (!save_system_parse_non_negative_int(max_hp_item->value, max_hp_value))
                continue;
            ship_snapshot.max_hp = max_hp_value;
            int hp_value = ship_snapshot.max_hp;
            if (hp_item)
            {
                if (!save_system_parse_non_negative_int(hp_item->value, hp_value))
                    continue;
            }
            if (ship_snapshot.max_hp <= 0)
                hp_value = 0;
            else if (hp_value > ship_snapshot.max_hp)
                hp_value = ship_snapshot.max_hp;
            ship_snapshot.hp = hp_value;
            key = base_key;
            key.append("_max_shield");
            json_item *max_shield_item = json_find_item(current, key.c_str());
            if (!max_shield_item)
                continue;
            int max_shield_value = 0;
            if (!save_system_parse_non_negative_int(max_shield_item->value, max_shield_value))
                continue;
            ship_snapshot.max_shield = max_shield_value;
            int shield_value = ship_snapshot.max_shield;
            if (shield_item)
            {
                if (!save_system_parse_non_negative_int(shield_item->value, shield_value))
                    continue;
            }
            if (ship_snapshot.max_shield <= 0)
                shield_value = 0;
            else if (shield_value > ship_snapshot.max_shield)
                shield_value = ship_snapshot.max_shield;
            ship_snapshot.shield = shield_value;
            key = base_key;
            key.append("_max_speed");
            json_item *max_speed_item = json_find_item(current, key.c_str());
            if (max_speed_item)
            {
                ship_snapshot.max_speed = this->unscale_long_to_double(ft_atol(max_speed_item->value));
                if (!save_system_is_finite(ship_snapshot.max_speed))
                    ship_snapshot.max_speed = 0.0;
                else if (ship_snapshot.max_speed < 0.0)
                    ship_snapshot.max_speed = 0.0;
            }
            key = base_key;
            key.append("_acceleration");
            json_item *acceleration_item = json_find_item(current, key.c_str());
            if (acceleration_item)
            {
                ship_snapshot.acceleration = this->unscale_long_to_double(ft_atol(acceleration_item->value));
                if (!save_system_is_finite(ship_snapshot.acceleration))
                    ship_snapshot.acceleration = 0.0;
                else if (ship_snapshot.acceleration < 0.0)
                    ship_snapshot.acceleration = 0.0;
            }
            key = base_key;
            key.append("_deceleration");
            json_item *deceleration_item = json_find_item(current, key.c_str());
            if (deceleration_item)
            {
                ship_snapshot.deceleration = this->unscale_long_to_double(ft_atol(deceleration_item->value));
                if (!save_system_is_finite(ship_snapshot.deceleration))
                    ship_snapshot.deceleration = 0.0;
                else if (ship_snapshot.deceleration < 0.0)
                    ship_snapshot.deceleration = 0.0;
            }
            key = base_key;
            key.append("_turn_speed");
            json_item *turn_speed_item = json_find_item(current, key.c_str());
            if (turn_speed_item)
            {
                ship_snapshot.turn_speed = this->unscale_long_to_double(ft_atol(turn_speed_item->value));
                if (!save_system_is_finite(ship_snapshot.turn_speed))
                    ship_snapshot.turn_speed = 0.0;
                else if (ship_snapshot.turn_speed < 0.0)
                    ship_snapshot.turn_speed = 0.0;
            }
            key = base_key;
            key.append("_behavior");
            json_item *behavior_item = json_find_item(current, key.c_str());
            if (behavior_item)
                ship_snapshot.combat_behavior = ft_atoi(behavior_item->value);
            key = base_key;
            key.append("_outnumbered");
            json_item *outnumbered_item = json_find_item(current, key.c_str());
            if (outnumbered_item)
                ship_snapshot.outnumbered_behavior = ft_atoi(outnumbered_item->value);
            key = base_key;
            key.append("_unescorted");
            json_item *unescorted_item = json_find_item(current, key.c_str());
            if (unescorted_item)
                ship_snapshot.unescorted_behavior = ft_atoi(unescorted_item->value);
            key = base_key;
            key.append("_low_hp");
            json_item *low_hp_item = json_find_item(current, key.c_str());
            if (low_hp_item)
                ship_snapshot.low_hp_behavior = ft_atoi(low_hp_item->value);
            key = base_key;
            key.append("_role");
            json_item *role_item = json_find_item(current, key.c_str());
            if (role_item)
                ship_snapshot.role = ft_atoi(role_item->value);
            if (seen_ship_ids.find(ship_snapshot.id) != ft_nullptr)
            {
                long candidate = highest_ship_id + 1;
                if (candidate < SAVE_SHIP_ID_MIN)
                    candidate = SAVE_SHIP_ID_MIN;
                while (candidate <= SAVE_SHIP_ID_MAX)
                {
                    if (seen_ship_ids.find(static_cast<int>(candidate)) == ft_nullptr)
                        break;
                    ++candidate;
                }
                if (candidate > SAVE_SHIP_ID_MAX)
                {
                    json_free_groups(groups);
                    return false;
                }
                ship_snapshot.id = static_cast<int>(candidate);
            }
            seen_ship_ids.insert(ship_snapshot.id);
            if (ship_snapshot.id > highest_ship_id)
                highest_ship_id = ship_snapshot.id;
            if (!save_system_is_finite(ship_snapshot.max_speed) || ship_snapshot.max_speed < 0.0)
                ship_snapshot.max_speed = 0.0;
            if (!save_system_is_finite(ship_snapshot.acceleration) || ship_snapshot.acceleration < 0.0)
                ship_snapshot.acceleration = 0.0;
            if (!save_system_is_finite(ship_snapshot.deceleration) || ship_snapshot.deceleration < 0.0)
                ship_snapshot.deceleration = ship_snapshot.acceleration;
            if (!save_system_is_finite(ship_snapshot.turn_speed) || ship_snapshot.turn_speed < 0.0)
                ship_snapshot.turn_speed = 0.0;
            fleet->add_ship_snapshot(ship_snapshot);
        }
