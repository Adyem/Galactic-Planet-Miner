void QuestManager::activate_next()
{
    size_t count = this->_progress.size();
    if (count == 0)
        return ;
    Pair<int, ft_quest_progress> *entries = this->_progress.end();
    entries -= count;
    bool main_active = false;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_quest_definition *definition = this->get_definition(entries[i].key);
        if (definition == ft_nullptr || definition->is_side_quest)
            continue;
        if (entries[i].value.status == QUEST_STATUS_ACTIVE
            || entries[i].value.status == QUEST_STATUS_AWAITING_CHOICE)
        {
            main_active = true;
            break;
        }
    }
    if (!main_active)
    {
        int next_id = 0;
        for (size_t i = 0; i < count; ++i)
        {
            const ft_quest_definition *definition = this->get_definition(entries[i].key);
            if (definition == ft_nullptr || definition->is_side_quest)
                continue;
            if (entries[i].value.status == QUEST_STATUS_AVAILABLE)
            {
                if (next_id == 0 || entries[i].key < next_id)
                    next_id = entries[i].key;
            }
        }
        if (next_id != 0)
        {
            Pair<int, ft_quest_progress> *progress_entry = this->_progress.find(next_id);
            if (progress_entry != ft_nullptr)
            {
                progress_entry->value.status = QUEST_STATUS_ACTIVE;
                const ft_quest_definition *definition = this->get_definition(next_id);
                if (definition != ft_nullptr && definition->time_limit > 0.0)
                    progress_entry->value.time_remaining = definition->time_limit * this->_time_scale;
                else
                    progress_entry->value.time_remaining = 0.0;
            }
        }
    }
    this->activate_side_quests();
}

void QuestManager::activate_side_quests()
{
    size_t count = this->_progress.size();
    if (count == 0)
        return ;
    Pair<int, ft_quest_progress> *entries = this->_progress.end();
    entries -= count;
    const size_t max_active = 1;
    size_t active_count = 0;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_quest_definition *definition = this->get_definition(entries[i].key);
        if (definition == ft_nullptr || !definition->is_side_quest)
            continue;
        if (entries[i].value.status == QUEST_STATUS_ACTIVE)
            active_count += 1;
    }
    if (active_count >= max_active || this->_side_quest_cycle.size() == 0)
        return ;
    size_t cycle_count = this->_side_quest_cycle.size();
    size_t attempts = cycle_count;
    while (attempts > 0 && active_count < max_active)
    {
        int quest_id = this->_side_quest_cycle[this->_side_cycle_index];
        this->_side_cycle_index += 1;
        if (this->_side_cycle_index >= cycle_count)
            this->_side_cycle_index = 0;
        Pair<int, ft_quest_progress> *progress_entry = this->_progress.find(quest_id);
        if (progress_entry == ft_nullptr)
        {
            attempts -= 1;
            continue;
        }
        const ft_quest_definition *definition = this->get_definition(quest_id);
        if (definition == ft_nullptr || !definition->is_side_quest)
        {
            attempts -= 1;
            continue;
        }
        if (progress_entry->value.status != QUEST_STATUS_AVAILABLE)
        {
            attempts -= 1;
            continue;
        }
        progress_entry->value.status = QUEST_STATUS_ACTIVE;
        if (definition->time_limit > 0.0)
            progress_entry->value.time_remaining = definition->time_limit * this->_time_scale;
        else
            progress_entry->value.time_remaining = 0.0;
        active_count += 1;
        attempts -= 1;
    }
}
