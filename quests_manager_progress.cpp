bool QuestManager::are_objectives_met(const ft_quest_definition &definition,
    const ft_quest_context &context) const
{
    if (definition.objectives.size() == 0)
        return true;
    for (size_t i = 0; i < definition.objectives.size(); ++i)
    {
        const ft_quest_objective &objective = definition.objectives[i];
        if (objective.type == QUEST_OBJECTIVE_RESOURCE_TOTAL)
        {
            const Pair<int, int> *entry = context.resource_totals.find(objective.target_id);
            if (entry == ft_nullptr || entry->value < objective.amount)
                return false;
        }
        else if (objective.type == QUEST_OBJECTIVE_RESEARCH_COMPLETED)
        {
            const Pair<int, int> *entry = context.research_status.find(objective.target_id);
            if (entry == ft_nullptr || entry->value < 1)
                return false;
        }
        else if (objective.type == QUEST_OBJECTIVE_FLEET_COUNT)
        {
            if (context.total_ship_count < objective.amount)
                return false;
        }
        else if (objective.type == QUEST_OBJECTIVE_TOTAL_SHIP_HP)
        {
            if (context.total_ship_hp < objective.amount)
                return false;
        }
        else if (objective.type == QUEST_OBJECTIVE_CONVOYS_DELIVERED)
        {
            if (context.successful_deliveries < objective.amount)
                return false;
        }
        else if (objective.type == QUEST_OBJECTIVE_CONVOY_STREAK)
        {
            if (context.delivery_streak < objective.amount)
                return false;
        }
        else if (objective.type == QUEST_OBJECTIVE_CONVOY_RAID_LOSSES_AT_MOST)
        {
            if (context.convoy_raid_losses > objective.amount)
                return false;
        }
        else if (objective.type == QUEST_OBJECTIVE_MAX_CONVOY_THREAT_AT_MOST)
        {
            double threshold = static_cast<double>(objective.amount) / 100.0;
            if (context.maximum_convoy_threat > threshold)
                return false;
        }
        else if (objective.type == QUEST_OBJECTIVE_BUILDING_COUNT)
        {
            const Pair<int, int> *entry = context.building_counts.find(objective.target_id);
            if (entry == ft_nullptr || entry->value < objective.amount)
                return false;
        }
        else if (objective.type == QUEST_OBJECTIVE_ASSAULT_VICTORIES)
        {
            const Pair<int, int> *entry = context.assault_victories.find(objective.target_id);
            if (entry == ft_nullptr || entry->value < objective.amount)
                return false;
        }
    }
    return true;
}

void QuestManager::update(double seconds, const ft_quest_context &context,
                          ft_vector<int> &completed, ft_vector<int> &failed,
                          ft_vector<int> &awaiting_choice)
{
    size_t count = this->_progress.size();
    Pair<int, ft_quest_progress> *entries = this->_progress.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        ft_quest_progress &progress = entries[i].value;
        if (progress.status != QUEST_STATUS_ACTIVE)
            continue;
        const ft_quest_definition *definition = this->get_definition(entries[i].key);
        if (definition == ft_nullptr)
            continue;
        if (progress.time_remaining > 0.0)
        {
            if (progress.time_remaining > seconds)
                progress.time_remaining -= seconds;
            else
            {
                progress.time_remaining = 0.0;
                progress.status = QUEST_STATUS_FAILED;
                failed.push_back(entries[i].key);
                this->_recent_completion_info.remove(entries[i].key);
                if (definition->time_limit > 0.0)
                {
                    progress.status = QUEST_STATUS_AVAILABLE;
                    progress.time_remaining = definition->time_limit * this->_time_scale;
                }
                continue;
            }
        }
        if (this->are_objectives_met(*definition, context))
        {
            if (definition->requires_choice)
            {
                progress.status = QUEST_STATUS_AWAITING_CHOICE;
                awaiting_choice.push_back(entries[i].key);
                this->_recent_completion_info.remove(entries[i].key);
            }
            else
            {
                if (definition->time_limit > 0.0)
                {
                    ft_quest_completion_info info;
                    info.timed = true;
                    info.time_limit = definition->time_limit * this->_time_scale;
                    info.time_remaining = progress.time_remaining;
                    Pair<int, ft_quest_completion_info> *info_entry = this->_recent_completion_info.find(entries[i].key);
                    if (info_entry == ft_nullptr)
                        this->_recent_completion_info.insert(entries[i].key, info);
                    else
                        info_entry->value = info;
                }
                else
                    this->_recent_completion_info.remove(entries[i].key);
                progress.status = QUEST_STATUS_COMPLETED;
                progress.time_remaining = 0.0;
                completed.push_back(entries[i].key);
            }
        }
    }
    if (completed.size() > 0 || failed.size() > 0)
        this->update_availability();
    this->activate_next();
}

void QuestManager::set_time_scale(double scale)
{
    if (scale <= 0.0)
        scale = 1.0;
    double delta = scale - this->_time_scale;
    if (delta < 0.0)
        delta = -delta;
    if (delta < 0.000001)
        return ;
    double ratio = scale / this->_time_scale;
    this->_time_scale = scale;
    size_t count = this->_progress.size();
    Pair<int, ft_quest_progress> *entries = this->_progress.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        ft_quest_progress &progress = entries[i].value;
        if (progress.time_remaining <= 0.0)
            continue;
        const ft_quest_definition *definition = this->get_definition(entries[i].key);
        if (definition == ft_nullptr || definition->time_limit <= 0.0)
            continue;
        if (progress.status == QUEST_STATUS_ACTIVE
            || progress.status == QUEST_STATUS_AVAILABLE)
        {
            progress.time_remaining *= ratio;
            double max_time = definition->time_limit * this->_time_scale;
            if (progress.time_remaining > max_time)
                progress.time_remaining = max_time;
        }
    }
}
