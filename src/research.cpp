#include "research.hpp"

ResearchManager::ResearchManager()
{
    ft_sharedptr<ft_research_definition> mars(new ft_research_definition());
    mars->id = RESEARCH_UNLOCK_MARS;
    mars->name = ft_string("Unlock Mars");
    mars->duration = 30.0;
    mars->prerequisites.clear();
    mars->costs.clear();
    mars->unlock_planets.clear();
    Pair<int, int> cost;
    cost.key = ORE_IRON;
    cost.value = 18;
    mars->costs.push_back(cost);
    cost.key = ORE_COPPER;
    cost.value = 12;
    mars->costs.push_back(cost);
    cost.key = ORE_COAL;
    cost.value = 6;
    mars->costs.push_back(cost);
    mars->unlock_planets.push_back(PLANET_MARS);
    this->register_research(mars);

    ft_sharedptr<ft_research_definition> zalthor(new ft_research_definition());
    zalthor->id = RESEARCH_UNLOCK_ZALTHOR;
    zalthor->name = ft_string("Unlock Zalthor");
    zalthor->duration = 40.0;
    zalthor->prerequisites.clear();
    zalthor->prerequisites.push_back(RESEARCH_UNLOCK_MARS);
    zalthor->costs.clear();
    cost.key = ORE_MITHRIL;
    cost.value = 8;
    zalthor->costs.push_back(cost);
    cost.key = ORE_COAL;
    cost.value = 6;
    zalthor->costs.push_back(cost);
    zalthor->unlock_planets.clear();
    zalthor->unlock_planets.push_back(PLANET_ZALTHOR);
    this->register_research(zalthor);

    ft_sharedptr<ft_research_definition> vulcan(new ft_research_definition());
    vulcan->id = RESEARCH_UNLOCK_VULCAN;
    vulcan->name = ft_string("Unlock Vulcan");
    vulcan->duration = 55.0;
    vulcan->prerequisites.clear();
    vulcan->prerequisites.push_back(RESEARCH_UNLOCK_ZALTHOR);
    vulcan->costs.clear();
    cost.key = ORE_GOLD;
    cost.value = 6;
    vulcan->costs.push_back(cost);
    cost.key = ORE_MITHRIL;
    cost.value = 8;
    vulcan->costs.push_back(cost);
    vulcan->unlock_planets.clear();
    vulcan->unlock_planets.push_back(PLANET_VULCAN);
    this->register_research(vulcan);

    ft_sharedptr<ft_research_definition> noctaris(new ft_research_definition());
    noctaris->id = RESEARCH_UNLOCK_NOCTARIS;
    noctaris->name = ft_string("Unlock Noctaris Prime");
    noctaris->duration = 60.0;
    noctaris->prerequisites.clear();
    noctaris->prerequisites.push_back(RESEARCH_UNLOCK_VULCAN);
    noctaris->costs.clear();
    cost.key = ORE_TITANIUM;
    cost.value = 5;
    noctaris->costs.push_back(cost);
    cost.key = ORE_SILVER;
    cost.value = 6;
    noctaris->costs.push_back(cost);
    cost.key = ORE_TIN;
    cost.value = 6;
    noctaris->costs.push_back(cost);
    noctaris->unlock_planets.clear();
    noctaris->unlock_planets.push_back(PLANET_NOCTARIS_PRIME);
    this->register_research(noctaris);

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
    entry->value.remaining_time = definition->duration;
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
