        encoded.append(",");
        encoded.append(ft_to_string(instance.definition_id));
        encoded.append(",");
        encoded.append(ft_to_string(instance.x));
        encoded.append(",");
        encoded.append(ft_to_string(instance.y));
        encoded.append(",");
        long progress_scaled = this->scale_double_to_long(instance.progress);
        encoded.append(ft_to_string(progress_scaled));
        encoded.append(",");
        encoded.append(instance.active ? "1" : "0");
    }
    return encoded;
}

bool SaveSystem::decode_building_instances(const char *encoded,
    ft_map<int, ft_building_instance> &instances) const noexcept
{
    instances.clear();
    if (!encoded || *encoded == '\0')
        return true;
    const char *cursor = encoded;
    while (*cursor != '\0')
    {
        while (*cursor == ';')
            ++cursor;
        if (*cursor == '\0')
            break;
        const char *segment_end = cursor;
        while (*segment_end != '\0' && *segment_end != ';')
            ++segment_end;
        const char *field_cursor = cursor;
        long key_value = 0;
        long definition_value = 0;
        long x_value = 0;
        long y_value = 0;
        long progress_value = 0;
        long active_value = 0;
        if (!save_system_parse_csv_long(field_cursor, segment_end, key_value))
            return false;
        if (!save_system_parse_csv_long(field_cursor, segment_end, definition_value))
            return false;
        if (!save_system_parse_csv_long(field_cursor, segment_end, x_value))
            return false;
        if (!save_system_parse_csv_long(field_cursor, segment_end, y_value))
            return false;
        if (!save_system_parse_csv_long(field_cursor, segment_end, progress_value))
            return false;
        if (!save_system_parse_csv_long(field_cursor, segment_end, active_value))
            return false;
        if (field_cursor != segment_end)
            return false;
        ft_building_instance instance;
        instance.uid = static_cast<int>(key_value);
        instance.definition_id = static_cast<int>(definition_value);
        instance.x = static_cast<int>(x_value);
        instance.y = static_cast<int>(y_value);
        instance.progress = this->unscale_long_to_double(progress_value);
        instance.active = active_value != 0;
        int insert_key = instance.uid;
        instances.insert(insert_key, instance);
        cursor = segment_end;
        if (*cursor == ';')
            ++cursor;
    }
    return true;
}

ft_string SaveSystem::serialize_buildings(const BuildingManager &buildings) const noexcept
{
    json_document document;
    json_group *manager_group = save_system_create_group(document, "buildings_manager");
    if (!manager_group)
        return save_system_abort_serialization(document);
    if (!save_system_add_item(document, manager_group, "type", "manager"))
        return save_system_abort_serialization(document);

    long crafting_energy_scaled = this->scale_double_to_long(buildings._crafting_energy_multiplier);
    ft_string crafting_energy_value = ft_to_string(crafting_energy_scaled);
    if (!save_system_add_item(document, manager_group, "crafting_energy_multiplier",
        crafting_energy_value.c_str()))
    {
        return save_system_abort_serialization(document);
    }
    long crafting_speed_scaled = this->scale_double_to_long(buildings._crafting_speed_multiplier);
    ft_string crafting_speed_value = ft_to_string(crafting_speed_scaled);
    if (!save_system_add_item(document, manager_group, "crafting_speed_multiplier",
        crafting_speed_value.c_str()))
    {
        return save_system_abort_serialization(document);
    }
    long global_energy_scaled = this->scale_double_to_long(buildings._global_energy_multiplier);
    ft_string global_energy_value = ft_to_string(global_energy_scaled);
    if (!save_system_add_item(document, manager_group, "global_energy_multiplier",
        global_energy_value.c_str()))
    {
        return save_system_abort_serialization(document);
    }

    size_t unlock_count = buildings._building_unlocks.size();
    if (unlock_count > 0)
    {
        const Pair<int, bool> *unlock_entries = buildings._building_unlocks.end();
        unlock_entries -= unlock_count;
        for (size_t i = 0; i < unlock_count; ++i)
        {
            ft_string key("unlock_");
            key.append(ft_to_string(unlock_entries[i].key));
            int unlocked = unlock_entries[i].value ? 1 : 0;
            if (!save_system_add_item(document, manager_group, key.c_str(), unlocked))
                return save_system_abort_serialization(document);
        }
    }

    size_t planet_count = buildings._planets.size();
    if (planet_count > 0)
    {
        const Pair<int, ft_planet_build_state> *entries = buildings._planets.end();
        entries -= planet_count;
        for (size_t i = 0; i < planet_count; ++i)
        {
            const ft_planet_build_state &state = entries[i].value;
            ft_string group_name = "building_planet_";
            group_name.append(ft_to_string(entries[i].key));
            json_group *group = save_system_create_group(document, group_name.c_str());
            if (!group)
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "type", "planet"))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "id", entries[i].key))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "width", state.width))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "height", state.height))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "base_logistic", state.base_logistic))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "research_logistic_bonus",
                state.research_logistic_bonus))
            {
                return save_system_abort_serialization(document);
            }
            if (!save_system_add_item(document, group, "used_plots", state.used_plots))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "logistic_capacity", state.logistic_capacity))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "logistic_usage", state.logistic_usage))
                return save_system_abort_serialization(document);
            long base_energy_scaled = this->scale_double_to_long(state.base_energy_generation);
            ft_string base_energy_value = ft_to_string(base_energy_scaled);
            if (!save_system_add_item(document, group, "base_energy_generation",
                base_energy_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            long energy_generation_scaled = this->scale_double_to_long(state.energy_generation);
            ft_string energy_generation_value = ft_to_string(energy_generation_scaled);
            if (!save_system_add_item(document, group, "energy_generation",
                energy_generation_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            long energy_consumption_scaled = this->scale_double_to_long(state.energy_consumption);
            ft_string energy_consumption_value = ft_to_string(energy_consumption_scaled);
            if (!save_system_add_item(document, group, "energy_consumption",
                energy_consumption_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            long support_energy_scaled = this->scale_double_to_long(state.support_energy);
            ft_string support_energy_value = ft_to_string(support_energy_scaled);
            if (!save_system_add_item(document, group, "support_energy", support_energy_value.c_str()))
                return save_system_abort_serialization(document);
            long mine_multiplier_scaled = this->scale_double_to_long(state.mine_multiplier);
            ft_string mine_multiplier_value = ft_to_string(mine_multiplier_scaled);
            if (!save_system_add_item(document, group, "mine_multiplier",
                mine_multiplier_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            long convoy_speed_scaled = this->scale_double_to_long(state.convoy_speed_bonus);
            ft_string convoy_speed_value = ft_to_string(convoy_speed_scaled);
            if (!save_system_add_item(document, group, "convoy_speed_bonus",
                convoy_speed_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            long convoy_risk_scaled = this->scale_double_to_long(state.convoy_raid_risk_modifier);
            ft_string convoy_risk_value = ft_to_string(convoy_risk_scaled);
            if (!save_system_add_item(document, group, "convoy_raid_risk_modifier",
                convoy_risk_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            long deficit_scaled = this->scale_double_to_long(state.energy_deficit_pressure);
            ft_string deficit_value = ft_to_string(deficit_scaled);
            if (!save_system_add_item(document, group, "energy_deficit_pressure",
                deficit_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            if (!save_system_add_item(document, group, "next_instance_id", state.next_instance_id))
                return save_system_abort_serialization(document);

            ft_string grid_payload = this->encode_building_grid(state.grid);
            if (grid_payload.size() > 0)
            {
                if (!save_system_add_item(document, group, "grid", grid_payload.c_str()))
                    return save_system_abort_serialization(document);
            }

            ft_string instance_payload = this->encode_building_instances(state.instances);
            if (instance_payload.size() > 0)
            {
                if (!save_system_add_item(document, group, "instances", instance_payload.c_str()))
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

bool SaveSystem::deserialize_buildings(const char *content, BuildingManager &buildings) const noexcept
{
    if (!content)
        return false;
    json_group *groups = json_read_from_string(content);
    if (!groups)
        return false;

    ft_map<int, ft_planet_build_state> parsed_planets;
    ft_map<int, bool> parsed_unlocks;
    double parsed_crafting_energy = 1.0;
    double parsed_crafting_speed = 1.0;
    double parsed_global_energy = 1.0;

    json_group *current = groups;
    while (current)
    {
        json_item *type_item = json_find_item(current, "type");
        const char *type_value = ft_nullptr;
        if (type_item)
            type_value = type_item->value;
        if (type_value && ft_strcmp(type_value, "manager") == 0)
