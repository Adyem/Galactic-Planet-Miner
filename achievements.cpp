#include "achievements.hpp"
#include "libft/Template/move.hpp"

void AchievementManager::register_achievement(const ft_sharedptr<ft_achievement_definition> &definition)
{
    if (!definition)
        return ;
    this->_definitions.insert(definition->id, definition);
    ft_achievement_progress progress;
    this->_progress.insert(definition->id, progress);
    Pair<int, ft_vector<int> > *index_entry = this->_event_index.find(definition->event_id);
    if (index_entry == ft_nullptr)
    {
        ft_vector<int> ids;
        this->_event_index.insert(definition->event_id, ft_move(ids));
        index_entry = this->_event_index.find(definition->event_id);
    }
    if (index_entry != ft_nullptr)
        index_entry->value.push_back(definition->id);
}

ft_achievement_progress &AchievementManager::ensure_progress(int achievement_id)
{
    Pair<int, ft_achievement_progress> *entry = this->_progress.find(achievement_id);
    if (entry == ft_nullptr)
    {
        ft_achievement_progress progress;
        this->_progress.insert(achievement_id, progress);
        entry = this->_progress.find(achievement_id);
    }
    return entry->value;
}

const ft_achievement_progress *AchievementManager::find_progress(int achievement_id) const
{
    const Pair<int, ft_achievement_progress> *entry = this->_progress.find(achievement_id);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
}

void AchievementManager::record_event(int event_id, int value, ft_vector<int> *completed)
{
    Pair<int, ft_vector<int> > *index_entry = this->_event_index.find(event_id);
    if (index_entry == ft_nullptr)
        return ;
    if (value < 0)
        value = 0;
    ft_vector<int> &achievement_ids = index_entry->value;
    for (size_t i = 0; i < achievement_ids.size(); ++i)
    {
        int achievement_id = achievement_ids[i];
        Pair<int, ft_sharedptr<ft_achievement_definition> > *definition_entry = this->_definitions.find(achievement_id);
        if (definition_entry == ft_nullptr)
            continue;
        const ft_sharedptr<ft_achievement_definition> &definition = definition_entry->value;
        ft_achievement_progress &progress = this->ensure_progress(achievement_id);
        if (progress.completed)
            continue;
        if (definition->progress_mode == ACHIEVEMENT_PROGRESS_INCREMENTAL)
        {
            progress.value += value;
        }
        else if (definition->progress_mode == ACHIEVEMENT_PROGRESS_BEST_VALUE)
        {
            if (value > progress.value)
                progress.value = value;
        }
        if (!progress.completed && progress.value >= definition->target_value)
        {
            progress.completed = true;
            if (completed != ft_nullptr)
                completed->push_back(achievement_id);
        }
    }
}

void AchievementManager::record_event(int event_id, int value)
{
    this->record_event(event_id, value, ft_nullptr);
}

bool AchievementManager::is_completed(int achievement_id) const
{
    return this->get_status(achievement_id) == ACHIEVEMENT_STATUS_COMPLETED;
}

int AchievementManager::get_status(int achievement_id) const
{
    const ft_achievement_progress *progress = this->find_progress(achievement_id);
    if (progress == ft_nullptr)
        return ACHIEVEMENT_STATUS_LOCKED;
    if (progress->completed)
        return ACHIEVEMENT_STATUS_COMPLETED;
    if (progress->value > 0)
        return ACHIEVEMENT_STATUS_IN_PROGRESS;
    return ACHIEVEMENT_STATUS_LOCKED;
}

int AchievementManager::get_progress(int achievement_id) const
{
    const ft_achievement_progress *progress = this->find_progress(achievement_id);
    if (progress == ft_nullptr)
        return 0;
    return progress->value;
}

int AchievementManager::get_target(int achievement_id) const
{
    const Pair<int, ft_sharedptr<ft_achievement_definition> > *entry = this->_definitions.find(achievement_id);
    if (entry == ft_nullptr)
        return 0;
    const ft_sharedptr<ft_achievement_definition> &definition = entry->value;
    if (!definition)
        return 0;
    return definition->target_value;
}

bool AchievementManager::get_info(int achievement_id, ft_achievement_info &out) const
{
    const Pair<int, ft_sharedptr<ft_achievement_definition> > *entry = this->_definitions.find(achievement_id);
    if (entry == ft_nullptr)
        return false;
    const ft_sharedptr<ft_achievement_definition> &definition = entry->value;
    if (!definition)
        return false;
    out.id = definition->id;
    out.name = definition->name;
    out.description = definition->description;
    out.target = definition->target_value;
    out.progress = 0;
    out.status = ACHIEVEMENT_STATUS_LOCKED;
    const ft_achievement_progress *progress = this->find_progress(achievement_id);
    if (progress != ft_nullptr)
    {
        out.progress = progress->value;
        if (progress->completed)
            out.status = ACHIEVEMENT_STATUS_COMPLETED;
        else if (progress->value > 0)
            out.status = ACHIEVEMENT_STATUS_IN_PROGRESS;
    }
    return true;
}

void AchievementManager::get_achievement_ids(ft_vector<int> &out) const
{
    out.clear();
    size_t count = this->_definitions.size();
    if (count == 0)
        return ;
    const Pair<int, ft_sharedptr<ft_achievement_definition> > *entries = this->_definitions.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
        out.push_back(entries[i].key);
}

void AchievementManager::get_progress_state(ft_map<int, ft_achievement_progress> &out) const
{
    out.clear();
    size_t count = this->_progress.size();
    if (count == 0)
        return ;
    const Pair<int, ft_achievement_progress> *entries = this->_progress.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
        out.insert(entries[i].key, entries[i].value);
}

bool AchievementManager::set_progress_state(const ft_map<int, ft_achievement_progress> &state)
{
    bool applied = false;
    size_t count = state.size();
    if (count == 0)
        return false;
    const Pair<int, ft_achievement_progress> *entries = state.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        Pair<int, ft_achievement_progress> *existing = this->_progress.find(entries[i].key);
        if (existing == ft_nullptr)
            continue;
        int value = entries[i].value.value;
        if (value < 0)
            value = 0;
        existing->value.value = value;
        existing->value.completed = entries[i].value.completed;
        applied = true;
    }
    return applied;
}
