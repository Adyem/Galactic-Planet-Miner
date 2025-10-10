int QuestManager::get_active_quest_id() const
{
    size_t count = this->_progress.size();
    const Pair<int, ft_quest_progress> *entries = this->_progress.end();
    entries -= count;
    int side_candidate = 0;
    for (size_t i = 0; i < count; ++i)
    {
        if (entries[i].value.status != QUEST_STATUS_AWAITING_CHOICE)
            continue;
        const ft_quest_definition *definition = this->get_definition(entries[i].key);
        if (definition == ft_nullptr)
            continue;
        if (!definition->is_side_quest)
            return entries[i].key;
        if (side_candidate == 0)
            side_candidate = entries[i].key;
    }
    for (size_t i = 0; i < count; ++i)
    {
        if (entries[i].value.status != QUEST_STATUS_ACTIVE)
            continue;
        const ft_quest_definition *definition = this->get_definition(entries[i].key);
        if (definition == ft_nullptr)
            continue;
        if (!definition->is_side_quest)
            return entries[i].key;
        if (side_candidate == 0)
            side_candidate = entries[i].key;
    }
    return side_candidate;
}

int QuestManager::get_status(int quest_id) const
{
    const Pair<int, ft_quest_progress> *entry = this->_progress.find(quest_id);
    if (entry == ft_nullptr)
        return QUEST_STATUS_LOCKED;
    return entry->value.status;
}

double QuestManager::get_time_remaining(int quest_id) const
{
    const Pair<int, ft_quest_progress> *entry = this->_progress.find(quest_id);
    if (entry == ft_nullptr)
        return 0.0;
    return entry->value.time_remaining;
}

const ft_quest_definition *QuestManager::get_definition(int quest_id) const
{
    const Pair<int, ft_sharedptr<ft_quest_definition> > *entry = this->_definitions.find(quest_id);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return entry->value.get();
}

bool QuestManager::make_choice(int quest_id, int choice_id)
{
    Pair<int, ft_quest_progress> *progress_entry = this->_progress.find(quest_id);
    if (progress_entry == ft_nullptr)
        return false;
    if (progress_entry->value.status != QUEST_STATUS_AWAITING_CHOICE)
        return false;
    const ft_quest_definition *definition = this->get_definition(quest_id);
    if (definition == ft_nullptr)
        return false;
    bool valid = false;
    for (size_t i = 0; i < definition->choices.size(); ++i)
    {
        if (definition->choices[i].choice_id == choice_id)
        {
            valid = true;
            break;
        }
    }
    if (!valid)
        return false;
    Pair<int, int> *choice_entry = this->_quest_choices.find(quest_id);
    if (choice_entry == ft_nullptr)
        this->_quest_choices.insert(quest_id, choice_id);
    else
        choice_entry->value = choice_id;
    progress_entry->value.status = QUEST_STATUS_COMPLETED;
    progress_entry->value.time_remaining = 0.0;
    this->update_availability();
    this->activate_next();
    return true;
}

int QuestManager::get_choice(int quest_id) const
{
    const Pair<int, int> *entry = this->_quest_choices.find(quest_id);
    if (entry == ft_nullptr)
        return QUEST_CHOICE_NONE;
    return entry->value;
}

bool QuestManager::consume_completion_info(int quest_id, ft_quest_completion_info &out)
{
    Pair<int, ft_quest_completion_info> *entry = this->_recent_completion_info.find(quest_id);
    if (entry == ft_nullptr)
        return false;
    out = entry->value;
    this->_recent_completion_info.remove(quest_id);
    return true;
}

void QuestManager::snapshot_definitions(ft_vector<Pair<int, ft_sharedptr<ft_quest_definition> > > &out) const
{
    ft_map_snapshot(this->_definitions, out);
}
