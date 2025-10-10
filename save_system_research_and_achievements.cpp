        fleets.insert(fleet_id, fleet);
        current = current->next;
    }
    json_free_groups(groups);
    return true;
}

ft_string SaveSystem::serialize_research(const ResearchManager &research) const noexcept
{
    json_document document;
    json_group *settings = save_system_create_group(document, "research_settings");
    if (!settings)
        return save_system_abort_serialization(document);
    long scaled = this->scale_double_to_long(research.get_duration_scale());
    ft_string value = ft_to_string(scaled);
    if (!save_system_add_item(document, settings, "duration_scale", value.c_str()))
        return save_system_abort_serialization(document);
    ft_map<int, ft_research_progress> progress;
    research.get_progress_state(progress);
    size_t count = progress.size();
    if (count > 0)
    {
        const Pair<int, ft_research_progress> *entries = progress.end();
        entries -= count;
        for (size_t i = 0; i < count; ++i)
        {
            ft_string group_name = "research_";
            group_name.append(ft_to_string(entries[i].key));
            json_group *group = save_system_create_group(document, group_name.c_str());
            if (!group)
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "id", entries[i].key))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "status", entries[i].value.status))
                return save_system_abort_serialization(document);
            long remaining_scaled = this->scale_double_to_long(entries[i].value.remaining_time);
            ft_string remaining_value = ft_to_string(remaining_scaled);
            if (!save_system_add_item(document, group, "remaining_time", remaining_value.c_str()))
                return save_system_abort_serialization(document);
        }
    }
    char *serialized = document.write_to_string();
    if (!serialized)
        return ft_string();
    ft_string result(serialized);
    cma_free(serialized);
    return result;
}

bool SaveSystem::deserialize_research(const char *content, ResearchManager &research) const noexcept
{
    if (!content)
        return false;
    json_group *groups = json_read_from_string(content);
    if (!groups)
        return false;
    ft_map<int, ft_research_progress> snapshot;
    double duration_scale = research.get_duration_scale();
    json_group *current = groups;
    while (current)
    {
        json_item *id_item = json_find_item(current, "id");
        if (id_item)
        {
            int research_id = ft_atoi(id_item->value);
            ft_research_progress progress;
            progress.status = RESEARCH_STATUS_LOCKED;
            progress.remaining_time = 0.0;
            json_item *status_item = json_find_item(current, "status");
            if (status_item)
                progress.status = ft_atoi(status_item->value);
            json_item *remaining_item = json_find_item(current, "remaining_time");
            if (remaining_item)
                progress.remaining_time = this->unscale_long_to_double(ft_atol(remaining_item->value));
            snapshot.insert(research_id, progress);
        }
        else
        {
            json_item *scale_item = json_find_item(current, "duration_scale");
            if (scale_item)
                duration_scale = this->unscale_long_to_double(ft_atol(scale_item->value));
        }
        current = current->next;
    }
    research.set_duration_scale(duration_scale);
    research.set_progress_state(snapshot);
    json_free_groups(groups);
    return true;
}

ft_string SaveSystem::serialize_achievements(const AchievementManager &achievements) const noexcept
{
    json_document document;
    ft_map<int, ft_achievement_progress> progress;
    achievements.get_progress_state(progress);
    size_t count = progress.size();
    if (count > 0)
    {
        const Pair<int, ft_achievement_progress> *entries = progress.end();
        entries -= count;
        for (size_t i = 0; i < count; ++i)
        {
            ft_string group_name = "achievement_";
            group_name.append(ft_to_string(entries[i].key));
            json_group *group = save_system_create_group(document, group_name.c_str());
            if (!group)
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "id", entries[i].key))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "value", entries[i].value.value))
                return save_system_abort_serialization(document);
            int completed_flag = entries[i].value.completed ? 1 : 0;
            if (!save_system_add_item(document, group, "completed", completed_flag))
                return save_system_abort_serialization(document);
        }
    }
    char *serialized = document.write_to_string();
    if (!serialized)
        return ft_string();
    ft_string result(serialized);
    cma_free(serialized);
    return result;
}

bool SaveSystem::deserialize_achievements(const char *content, AchievementManager &achievements) const noexcept
{
    if (!content)
        return false;
    json_group *groups = json_read_from_string(content);
    if (!groups)
        return false;
    ft_map<int, ft_achievement_progress> snapshot;
    json_group *current = groups;
    while (current)
    {
        json_item *id_item = json_find_item(current, "id");
        if (!id_item)
        {
            current = current->next;
            continue;
        }
        int achievement_id = ft_atoi(id_item->value);
        ft_achievement_progress progress;
        progress.value = 0;
        progress.completed = false;
        json_item *value_item = json_find_item(current, "value");
        if (value_item)
            progress.value = ft_atoi(value_item->value);
        json_item *completed_item = json_find_item(current, "completed");
        if (completed_item)
            progress.completed = ft_atoi(completed_item->value) != 0;
        snapshot.insert(achievement_id, progress);
        current = current->next;
    }
    achievements.set_progress_state(snapshot);
    json_free_groups(groups);
    return true;
}

ft_string SaveSystem::encode_building_grid(const ft_vector<int> &grid) const noexcept
{
    ft_string encoded;
    size_t size = grid.size();
    if (size == 0)
        return encoded;
    int current = grid[0];
    size_t run = 1;
    for (size_t i = 1; i < size; ++i)
    {
        int value = grid[i];
        if (value == current)
        {
            ++run;
            continue;
        }
        save_system_append_grid_run(encoded, current, run);
        current = value;
        run = 1;
    }
    save_system_append_grid_run(encoded, current, run);
    return encoded;
}

bool SaveSystem::decode_building_grid(const char *encoded, size_t expected_cells,
    ft_vector<int> &grid) const noexcept
{
    grid.clear();
    if (expected_cells == 0)
        return true;
    if (!encoded || *encoded == '\0')
    {
        grid.resize(expected_cells, 0);
        return true;
    }
    grid.reserve(expected_cells);
    const char *cursor = encoded;
    size_t produced = 0;
    while (*cursor != '\0')
    {
        while (*cursor == ' ')
            ++cursor;
        if (*cursor == '\0')
            break;
        const char *token_begin = cursor;
        while (*cursor != '\0' && *cursor != ' ')
            ++cursor;
        const char *token_end = cursor;
        long value_long = 0;
        size_t run = 0;
        if (!save_system_parse_grid_run(token_begin, token_end, value_long, run))
            return false;
        if (run > static_cast<size_t>(BUILDING_GRID_MAX_CELLS))
            return false;
        if (produced > expected_cells)
            return false;
        size_t remaining = expected_cells - produced;
        if (run > remaining)
            return false;
        for (size_t i = 0; i < run; ++i)
        {
            grid.push_back(static_cast<int>(value_long));
        }
        produced += run;
    }
    if (produced > expected_cells)
        return false;
    if (produced < expected_cells)
    {
        size_t deficit = expected_cells - produced;
        for (size_t i = 0; i < deficit; ++i)
            grid.push_back(0);
    }
    return grid.size() == expected_cells;
}

ft_string SaveSystem::encode_building_instances(const ft_map<int, ft_building_instance> &instances)
    const noexcept
{
    ft_string encoded;
    size_t count = instances.size();
    if (count == 0)
        return encoded;
    const Pair<int, ft_building_instance> *entries = instances.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_building_instance &instance = entries[i].value;
        if (!encoded.empty())
            encoded.append(";");
        encoded.append(ft_to_string(entries[i].key));
