QuestManager::QuestManager()
    : _time_scale(1.0),
      _side_quest_cycle(),
      _side_cycle_index(0)
{
    ft_vector<ft_sharedptr<ft_quest_definition> > definitions;
    collect_quest_definitions(definitions);
    for (size_t i = 0; i < definitions.size(); ++i)
        this->register_quest(definitions[i]);
    this->update_availability();
    this->activate_next();
}

void QuestManager::register_quest(const ft_sharedptr<ft_quest_definition> &definition)
{
    this->_definitions.insert(definition->id, definition);
    ft_quest_progress progress;
    if (definition->prerequisites.size() == 0 && definition->required_choice_quest == 0)
        progress.status = QUEST_STATUS_AVAILABLE;
    else
        progress.status = QUEST_STATUS_LOCKED;
    progress.time_remaining = 0.0;
    this->_progress.insert(definition->id, progress);
    if (definition->is_side_quest)
        this->_side_quest_cycle.push_back(definition->id);
}

void QuestManager::update_availability()
{
    size_t count = this->_progress.size();
    Pair<int, ft_quest_progress> *entries = this->_progress.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        ft_quest_progress &progress = entries[i].value;
        if (progress.status != QUEST_STATUS_LOCKED)
            continue;
        const ft_quest_definition *definition = this->get_definition(entries[i].key);
        if (definition == ft_nullptr)
            continue;
        bool ready = true;
        for (size_t j = 0; j < definition->prerequisites.size(); ++j)
        {
            int prereq = definition->prerequisites[j];
            const Pair<int, ft_quest_progress> *pr_entry = this->_progress.find(prereq);
            if (pr_entry == ft_nullptr || pr_entry->value.status != QUEST_STATUS_COMPLETED)
            {
                ready = false;
                break;
            }
        }
        if (!ready)
            continue;
        if (definition->required_choice_quest != 0)
        {
            const Pair<int, int> *choice_entry = this->_quest_choices.find(definition->required_choice_quest);
            if (choice_entry == ft_nullptr || choice_entry->value != definition->required_choice_value)
                continue;
        }
        progress.status = QUEST_STATUS_AVAILABLE;
    }
}
