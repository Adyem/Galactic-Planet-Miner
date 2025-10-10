#include "research.hpp"

namespace
{
#include "research_definitions.cpp"
}

ResearchManager::ResearchManager()
    : _duration_scale(1.0)
{
    ft_vector<ft_sharedptr<ft_research_definition> > definitions;
    collect_research_definitions(definitions);
    for (size_t i = 0; i < definitions.size(); ++i)
        this->register_research(definitions[i]);
    this->update_availability();
}

void ResearchManager::register_research(const ft_sharedptr<ft_research_definition> &definition)
{
    this->_definitions.insert(definition->id, definition);
    ft_research_progress progress;
    if (definition->prerequisites.size() == 0)
        progress.status = RESEARCH_STATUS_AVAILABLE;
    else
        progress.status = RESEARCH_STATUS_LOCKED;
    progress.remaining_time = 0.0;
    this->_progress.insert(definition->id, progress);
}

void ResearchManager::update_availability()
{
    size_t count = this->_progress.size();
    Pair<int, ft_research_progress> *entries = this->_progress.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        ft_research_progress &progress = entries[i].value;
        if (progress.status != RESEARCH_STATUS_LOCKED)
            continue;
        const ft_research_definition *definition = this->get_definition(entries[i].key);
        if (definition == ft_nullptr)
            continue;
        bool ready = true;
        for (size_t j = 0; j < definition->prerequisites.size(); ++j)
        {
            int prereq = definition->prerequisites[j];
            const Pair<int, ft_research_progress> *entry = this->_progress.find(prereq);
            if (entry == ft_nullptr || entry->value.status != RESEARCH_STATUS_COMPLETED)
            {
                ready = false;
                break;
            }
        }
        if (ready)
            progress.status = RESEARCH_STATUS_AVAILABLE;
    }
}

void ResearchManager::tick(double seconds, ft_vector<int> &completed)
{
    size_t count = this->_progress.size();
    Pair<int, ft_research_progress> *entries = this->_progress.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        ft_research_progress &progress = entries[i].value;
        if (progress.status != RESEARCH_STATUS_IN_PROGRESS)
            continue;
        if (progress.remaining_time > seconds)
            progress.remaining_time -= seconds;
        else
        {
            progress.remaining_time = 0.0;
            progress.status = RESEARCH_STATUS_COMPLETED;
            completed.push_back(entries[i].key);
        }
    }
    if (completed.size() > 0)
        this->update_availability();
}

void ResearchManager::set_duration_scale(double scale)
{
    if (scale <= 0.0)
        scale = 1.0;
    double delta = scale - this->_duration_scale;
    if (delta < 0.0)
        delta = -delta;
    if (delta < 0.000001)
        return ;
    double ratio = scale / this->_duration_scale;
    this->_duration_scale = scale;
    size_t count = this->_progress.size();
    Pair<int, ft_research_progress> *entries = this->_progress.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        ft_research_progress &progress = entries[i].value;
        if (progress.status == RESEARCH_STATUS_IN_PROGRESS && progress.remaining_time > 0.0)
            progress.remaining_time *= ratio;
    }
}

bool ResearchManager::can_start(int research_id) const
{
    const Pair<int, ft_research_progress> *entry = this->_progress.find(research_id);
    if (entry == ft_nullptr)
        return false;
    return entry->value.status == RESEARCH_STATUS_AVAILABLE;
}

bool ResearchManager::start(int research_id)
{
    Pair<int, ft_research_progress> *entry = this->_progress.find(research_id);
    if (entry == ft_nullptr)
        return false;
    if (entry->value.status != RESEARCH_STATUS_AVAILABLE)
        return false;
    const ft_research_definition *definition = this->get_definition(research_id);
    if (definition == ft_nullptr)
        return false;
    entry->value.status = RESEARCH_STATUS_IN_PROGRESS;
    double scaled = definition->duration * this->_duration_scale;
    if (scaled <= 0.0)
        entry->value.remaining_time = 0.0;
    else
        entry->value.remaining_time = scaled;
    if (entry->value.remaining_time <= 0.0)
    {
        entry->value.remaining_time = 0.0;
        entry->value.status = RESEARCH_STATUS_COMPLETED;
        this->update_availability();
    }
    return true;
}

bool ResearchManager::is_completed(int research_id) const
{
    const Pair<int, ft_research_progress> *entry = this->_progress.find(research_id);
    if (entry == ft_nullptr)
        return false;
    return entry->value.status == RESEARCH_STATUS_COMPLETED;
}

int ResearchManager::get_status(int research_id) const
{
    const Pair<int, ft_research_progress> *entry = this->_progress.find(research_id);
    if (entry == ft_nullptr)
        return RESEARCH_STATUS_LOCKED;
    return entry->value.status;
}

double ResearchManager::get_remaining_time(int research_id) const
{
    const Pair<int, ft_research_progress> *entry = this->_progress.find(research_id);
    if (entry == ft_nullptr)
        return 0.0;
    return entry->value.remaining_time;
}

const ft_research_definition *ResearchManager::get_definition(int research_id) const
{
    const Pair<int, ft_sharedptr<ft_research_definition> > *entry = this->_definitions.find(research_id);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return entry->value.get();
}

void ResearchManager::mark_completed(int research_id)
{
    Pair<int, ft_research_progress> *entry = this->_progress.find(research_id);
    if (entry == ft_nullptr)
        return ;
    entry->value.status = RESEARCH_STATUS_COMPLETED;
    entry->value.remaining_time = 0.0;
    this->update_availability();
}

void ResearchManager::get_progress_state(ft_map<int, ft_research_progress> &out) const
{
    out.clear();
    size_t count = this->_progress.size();
    if (count == 0)
        return ;
    const Pair<int, ft_research_progress> *entries = this->_progress.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
        out.insert(entries[i].key, entries[i].value);
}

bool ResearchManager::set_progress_state(const ft_map<int, ft_research_progress> &state)
{
    bool applied = false;
    size_t count = state.size();
    if (count == 0)
    {
        this->update_availability();
        return false;
    }
    const Pair<int, ft_research_progress> *entries = state.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        Pair<int, ft_research_progress> *existing = this->_progress.find(entries[i].key);
        if (existing == ft_nullptr)
            continue;
        int status = entries[i].value.status;
        if (status < RESEARCH_STATUS_LOCKED)
            status = RESEARCH_STATUS_LOCKED;
        else if (status > RESEARCH_STATUS_COMPLETED)
            status = RESEARCH_STATUS_COMPLETED;
        existing->value.status = status;
        double remaining = entries[i].value.remaining_time;
        if (remaining < 0.0)
            remaining = 0.0;
        existing->value.remaining_time = remaining;
        if (existing->value.status == RESEARCH_STATUS_COMPLETED
            || existing->value.status == RESEARCH_STATUS_AVAILABLE
            || existing->value.status == RESEARCH_STATUS_LOCKED)
        {
            existing->value.remaining_time = 0.0;
        }
        else if (existing->value.status == RESEARCH_STATUS_IN_PROGRESS
            && existing->value.remaining_time <= 0.0)
        {
            existing->value.remaining_time = 0.0;
            existing->value.status = RESEARCH_STATUS_COMPLETED;
        }
        applied = true;
    }
    this->update_availability();
    return applied;
}
