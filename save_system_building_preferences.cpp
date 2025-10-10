        {
            json_item *item = current->items;
            while (item)
            {
                if (!item->key)
                {
                    item = item->next;
                    continue;
                }
                if (ft_strcmp(item->key, "crafting_energy_multiplier") == 0)
                {
                    parsed_crafting_energy = this->unscale_long_to_double(ft_atol(item->value));
                    if (parsed_crafting_energy <= 0.0)
                        parsed_crafting_energy = 1.0;
                }
                else if (ft_strcmp(item->key, "crafting_speed_multiplier") == 0)
                {
                    parsed_crafting_speed = this->unscale_long_to_double(ft_atol(item->value));
                    if (parsed_crafting_speed <= 0.0)
                        parsed_crafting_speed = 1.0;
                }
                else if (ft_strcmp(item->key, "global_energy_multiplier") == 0)
                {
                    parsed_global_energy = this->unscale_long_to_double(ft_atol(item->value));
                    if (parsed_global_energy <= 0.0)
                        parsed_global_energy = 1.0;
                }
                else if (ft_strncmp(item->key, "unlock_", 7) == 0)
                {
                    int building_id = ft_atoi(item->key + 7);
                    bool unlocked = ft_atoi(item->value) != 0;
                    Pair<int, bool> *entry = parsed_unlocks.find(building_id);
                    if (entry == ft_nullptr)
                        parsed_unlocks.insert(building_id, unlocked);
                    else
                        entry->value = unlocked;
                }
                item = item->next;
            }
        }
        else
        {
            json_item *id_item = json_find_item(current, "id");
            if (!id_item)
            {
                current = current->next;
                continue;
            }
            int planet_id = ft_atoi(id_item->value);
            ft_planet_build_state state;
            state.planet_id = planet_id;
            ft_vector<Pair<int, int> > grid_entries;
            ft_map<int, ft_building_instance> instance_lookup;
            const char *grid_payload_ptr = ft_nullptr;
            const char *instance_payload_ptr = ft_nullptr;
            json_item *item = current->items;
            while (item)
            {
                if (!item->key)
                {
                    item = item->next;
                    continue;
                }
                if (ft_strcmp(item->key, "type") == 0)
                {
                    item = item->next;
                    continue;
                }
                if (ft_strcmp(item->key, "width") == 0)
                    state.width = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "height") == 0)
                    state.height = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "base_logistic") == 0)
                    state.base_logistic = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "research_logistic_bonus") == 0)
                    state.research_logistic_bonus = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "used_plots") == 0)
                    state.used_plots = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "logistic_capacity") == 0)
                    state.logistic_capacity = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "logistic_usage") == 0)
                    state.logistic_usage = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "base_energy_generation") == 0)
                    state.base_energy_generation = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "energy_generation") == 0)
                    state.energy_generation = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "energy_consumption") == 0)
                    state.energy_consumption = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "support_energy") == 0)
                    state.support_energy = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "mine_multiplier") == 0)
                    state.mine_multiplier = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "convoy_speed_bonus") == 0)
                    state.convoy_speed_bonus = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "convoy_raid_risk_modifier") == 0)
                    state.convoy_raid_risk_modifier = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "energy_deficit_pressure") == 0)
                    state.energy_deficit_pressure = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "next_instance_id") == 0)
                {
                    state.next_instance_id = ft_atoi(item->value);
                    if (state.next_instance_id >= FT_BUILDING_INSTANCE_ID_MAX)
                        state.next_instance_id = FT_BUILDING_INSTANCE_ID_MAX;
                }
                else if (ft_strcmp(item->key, "grid") == 0)
                    grid_payload_ptr = item->value;
                else if (ft_strcmp(item->key, "instances") == 0)
                    instance_payload_ptr = item->value;
                else if (ft_strncmp(item->key, "cell_", 5) == 0)
                {
                    Pair<int, int> entry;
                    entry.key = ft_atoi(item->key + 5);
                    entry.value = ft_atoi(item->value);
                    grid_entries.push_back(entry);
                }
                else if (ft_strncmp(item->key, "instance_", 9) == 0)
                {
                    int uid = ft_atoi(item->key + 9);
                    const char *suffix = ft_strchr(item->key + 9, '_');
                    if (!suffix)
                    {
                        item = item->next;
                        continue;
                    }
                    suffix += 1;
                    Pair<int, ft_building_instance> *entry = instance_lookup.find(uid);
                    if (entry == ft_nullptr)
                    {
                        ft_building_instance instance;
                        instance.uid = uid;
                        instance_lookup.insert(uid, instance);
                        entry = instance_lookup.find(uid);
                    }
                    if (entry == ft_nullptr)
                    {
                        item = item->next;
                        continue;
                    }
                    ft_building_instance &instance = entry->value;
                    if (ft_strcmp(suffix, "uid") == 0)
                        instance.uid = ft_atoi(item->value);
                    else if (ft_strcmp(suffix, "definition") == 0)
                        instance.definition_id = ft_atoi(item->value);
                    else if (ft_strcmp(suffix, "x") == 0)
                        instance.x = ft_atoi(item->value);
                    else if (ft_strcmp(suffix, "y") == 0)
                        instance.y = ft_atoi(item->value);
                    else if (ft_strcmp(suffix, "progress") == 0)
                        instance.progress = this->unscale_long_to_double(ft_atol(item->value));
                    else if (ft_strcmp(suffix, "active") == 0)
                        instance.active = ft_atoi(item->value) != 0;
                }
                item = item->next;
            }

            if (state.width < 0)
                state.width = 0;
            if (state.height < 0)
                state.height = 0;

            size_t sanitized_width = static_cast<size_t>(state.width);
            size_t sanitized_height = static_cast<size_t>(state.height);
            size_t max_cells_limit = static_cast<size_t>(BUILDING_GRID_MAX_CELLS);
            if (sanitized_width > max_cells_limit || sanitized_height > max_cells_limit)
            {
                json_free_groups(groups);
                return false;
            }
            if (sanitized_width > 0 && sanitized_height > max_cells_limit / sanitized_width)
            {
                json_free_groups(groups);
                return false;
            }
            if (state.base_logistic < 0)
                state.base_logistic = 0;
            if (state.research_logistic_bonus < 0)
                state.research_logistic_bonus = 0;
            if (state.logistic_capacity < 0)
                state.logistic_capacity = 0;
            if (state.logistic_usage < 0)
                state.logistic_usage = 0;
            if (state.logistic_usage > state.logistic_capacity)
                state.logistic_usage = state.logistic_capacity;
            if (state.next_instance_id <= 0)
                state.next_instance_id = 1;
            if (state.mine_multiplier < 1.0)
                state.mine_multiplier = 1.0;
            if (state.convoy_speed_bonus < 0.0)
                state.convoy_speed_bonus = 0.0;
            if (state.convoy_raid_risk_modifier < 0.0)
                state.convoy_raid_risk_modifier = 0.0;
            if (state.support_energy < 0.0)
                state.support_energy = 0.0;
            if (state.energy_deficit_pressure < 0.0)
                state.energy_deficit_pressure = 0.0;

            long total_cells_long = static_cast<long>(state.width) * static_cast<long>(state.height);
            if (total_cells_long < 0)
                total_cells_long = 0;
            size_t total_cells = static_cast<size_t>(total_cells_long);
            state.grid.clear();
            if (total_cells > 0)
            {
                bool decoded_grid = false;
                if (grid_payload_ptr && grid_payload_ptr[0] != '\0')
                {
                    if (!this->decode_building_grid(grid_payload_ptr, total_cells, state.grid))
                    {
                        json_free_groups(groups);
                        return false;
                    }
                    decoded_grid = true;
                }
                if (!decoded_grid)
                {
                    state.grid.resize(total_cells, 0);
                    for (size_t j = 0; j < grid_entries.size(); ++j)
                    {
                        int index = grid_entries[j].key;
                        if (index < 0)
                            continue;
                        size_t cell_index = static_cast<size_t>(index);
                        if (cell_index >= total_cells)
                            continue;
                        state.grid[cell_index] = grid_entries[j].value;
                    }
                }
            }

            int max_plots = state.width * state.height;
            if (max_plots < 0)
                max_plots = 0;
            if (state.used_plots < 0)
                state.used_plots = 0;
            if (state.used_plots > max_plots)
                state.used_plots = max_plots;

            state.instances.clear();
            bool decoded_instances = false;
            if (instance_payload_ptr)
            {
                if (!this->decode_building_instances(instance_payload_ptr, state.instances))
                {
                    json_free_groups(groups);
                    return false;
                }
                decoded_instances = true;
            }
            if (!decoded_instances)
            {
