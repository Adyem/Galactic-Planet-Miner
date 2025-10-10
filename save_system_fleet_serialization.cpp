                int ore_id = ft_atoi(item->key + 9);
                Pair<int, int> entry;
                entry.key = ore_id;
                int amount = 0;
                if (ore_id > 0 && save_system_parse_stack_amount(item->value, amount))
                {
                    entry.value = amount;
                    resource_amounts.push_back(entry);
                }
            }
            else if (item->key && ft_strncmp(item->key, "rate_", 5) == 0)
            {
                int ore_id = ft_atoi(item->key + 5);
                Pair<int, long> entry;
                entry.key = ore_id;
                entry.value = ft_atol(item->value);
                resource_rates.push_back(entry);
            }
            else if (item->key && ft_strncmp(item->key, "carryover_", 10) == 0)
            {
                int ore_id = ft_atoi(item->key + 10);
                Pair<int, long> entry;
                entry.key = ore_id;
                entry.value = ft_atol(item->value);
                resource_carryover.push_back(entry);
            }
            else if (item->key && ft_strncmp(item->key, "item_", 5) == 0)
            {
                int item_id = ft_atoi(item->key + 5);
                Pair<int, int> entry;
                entry.key = item_id;
                int amount = 0;
                if (item_id > 0 && save_system_parse_stack_amount(item->value, amount))
                {
                    entry.value = amount;
                    inventory_items.push_back(entry);
                }
            }
            item = item->next;
        }
        for (size_t i = 0; i < resource_rates.size(); ++i)
        {
            int ore_id = resource_rates[i].key;
            double rate = this->unscale_long_to_double(resource_rates[i].value);
            if (!save_system_is_finite(rate))
                rate = 0.0;
            if (ore_id > 0)
                planet->register_resource(ore_id, rate);
        }
        for (size_t i = 0; i < resource_amounts.size(); ++i)
        {
            int ore_id = resource_amounts[i].key;
            if (ore_id <= 0)
                continue;
            planet->register_resource(ore_id, planet->get_rate(ore_id));
            int sanitized = planet->clamp_resource_amount(ore_id, resource_amounts[i].value);
            planet->set_resource(ore_id, sanitized);
        }
        for (size_t i = 0; i < resource_carryover.size(); ++i)
        {
            int ore_id = resource_carryover[i].key;
            double carry_value = this->unscale_long_to_double(resource_carryover[i].value);
            if (!save_system_is_finite(carry_value))
                carry_value = 0.0;
            if (ore_id > 0)
                planet->set_carryover(ore_id, carry_value);
        }
        for (size_t i = 0; i < inventory_items.size(); ++i)
        {
            int item_id = inventory_items[i].key;
            if (item_id <= 0)
                continue;
            planet->ensure_item_slot(item_id);
            int sanitized = planet->clamp_resource_amount(item_id, inventory_items[i].value);
            planet->set_resource(item_id, sanitized);
        }
        planets.insert(planet_id, planet);
        current = current->next;
    }
    json_free_groups(groups);
    return true;
}

ft_string SaveSystem::serialize_fleets(const ft_map<int, ft_sharedptr<ft_fleet> > &fleets) const noexcept
{
    json_document document;
    size_t count = fleets.size();
    if (count > 0)
    {
        const Pair<int, ft_sharedptr<ft_fleet> > *entries = fleets.end();
        entries -= count;
        for (size_t i = 0; i < count; ++i)
        {
            ft_sharedptr<ft_fleet> fleet = entries[i].value;
            if (!fleet)
                continue;
            ft_string group_name = "fleet_";
            ft_string id_string = ft_to_string(entries[i].key);
            group_name.append(id_string);
            json_group *group = save_system_create_group(document, group_name.c_str());
            if (!group)
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "id", entries[i].key))
                return save_system_abort_serialization(document);
            ft_location location = fleet->get_location();
            if (!save_system_add_item(document, group, "location_type", location.type))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "location_from", location.from))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "location_to", location.to))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "location_misc", location.misc))
                return save_system_abort_serialization(document);
            long travel_scaled = this->scale_double_to_long(fleet->get_travel_time());
            ft_string travel_value = ft_to_string(travel_scaled);
            if (!save_system_add_item(document, group, "travel_time", travel_value.c_str()))
                return save_system_abort_serialization(document);
            long veterancy_scaled = this->scale_double_to_long(fleet->get_escort_veterancy());
            ft_string veterancy_value = ft_to_string(veterancy_scaled);
            if (!save_system_add_item(document, group, "escort_veterancy", veterancy_value.c_str()))
                return save_system_abort_serialization(document);
            int ship_total = fleet->get_ship_count();
            if (!save_system_add_item(document, group, "ship_count", ship_total))
                return save_system_abort_serialization(document);
            ft_vector<int> ship_ids;
            fleet->get_ship_ids(ship_ids);
            for (size_t j = 0; j < ship_ids.size(); ++j)
            {
                int ship_id = ship_ids[j];
                const ft_ship *ship = fleet->get_ship(ship_id);
                if (!ship)
                    continue;
                ft_string index_string = ft_to_string(static_cast<long>(j));
                ft_string base_key = "ship_";
                base_key.append(index_string);
                ft_string key = base_key;
                key.append("_id");
                if (!save_system_add_item(document, group, key.c_str(), ship->id))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_type");
                if (!save_system_add_item(document, group, key.c_str(), ship->type))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_armor");
                if (!save_system_add_item(document, group, key.c_str(), ship->armor))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_hp");
                if (!save_system_add_item(document, group, key.c_str(), ship->hp))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_shield");
                if (!save_system_add_item(document, group, key.c_str(), ship->shield))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_max_hp");
                if (!save_system_add_item(document, group, key.c_str(), ship->max_hp))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_max_shield");
                if (!save_system_add_item(document, group, key.c_str(), ship->max_shield))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_max_speed");
                long max_speed_scaled = this->scale_double_to_long(ship->max_speed);
                ft_string max_speed_value = ft_to_string(max_speed_scaled);
                if (!save_system_add_item(document, group, key.c_str(), max_speed_value.c_str()))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_acceleration");
                long acceleration_scaled = this->scale_double_to_long(ship->acceleration);
                ft_string acceleration_value = ft_to_string(acceleration_scaled);
                if (!save_system_add_item(document, group, key.c_str(), acceleration_value.c_str()))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_deceleration");
                long deceleration_scaled = this->scale_double_to_long(ship->deceleration);
                ft_string deceleration_value = ft_to_string(deceleration_scaled);
                if (!save_system_add_item(document, group, key.c_str(), deceleration_value.c_str()))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_turn_speed");
                long turn_speed_scaled = this->scale_double_to_long(ship->turn_speed);
                ft_string turn_speed_value = ft_to_string(turn_speed_scaled);
                if (!save_system_add_item(document, group, key.c_str(), turn_speed_value.c_str()))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_behavior");
                if (!save_system_add_item(document, group, key.c_str(), ship->combat_behavior))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_outnumbered");
                if (!save_system_add_item(document, group, key.c_str(), ship->outnumbered_behavior))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_unescorted");
                if (!save_system_add_item(document, group, key.c_str(), ship->unescorted_behavior))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_low_hp");
                if (!save_system_add_item(document, group, key.c_str(), ship->low_hp_behavior))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_role");
                if (!save_system_add_item(document, group, key.c_str(), ship->role))
                    return save_system_abort_serialization(document);
            }
        }
    }
    char *serialized = document.write_to_string();
    if (!serialized)
        return ft_string();
    ft_string result(serialized);
    cma_free(serialized);
    return result;
}

bool SaveSystem::deserialize_fleets(const char *content,
    ft_map<int, ft_sharedptr<ft_fleet> > &fleets) const noexcept
{
    if (!content)
        return false;
    json_group *groups = json_read_from_string(content);
    if (!groups)
        return false;
    fleets.clear();
    ft_set<int> seen_ship_ids;
    long highest_ship_id = static_cast<long>(SAVE_SHIP_ID_MIN) - 1;
    json_group *current = groups;
    while (current)
    {
        json_item *id_item = json_find_item(current, "id");
        if (!id_item)
        {
            current = current->next;
            continue;
        }
        int fleet_id = ft_atoi(id_item->value);
        ft_sharedptr<ft_fleet> fleet = this->create_fleet_instance(fleet_id);
        if (!fleet)
        {
            current = current->next;
            continue;
        }
        json_item *type_item = json_find_item(current, "location_type");
        int location_type = type_item ? ft_atoi(type_item->value) : LOCATION_PLANET;
        json_item *from_item = json_find_item(current, "location_from");
        int location_from = from_item ? ft_atoi(from_item->value) : PLANET_TERRA;
        json_item *to_item = json_find_item(current, "location_to");
