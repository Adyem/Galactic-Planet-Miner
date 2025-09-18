#include "research.hpp"

ResearchManager::ResearchManager()
    : _duration_scale(1.0)
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

    ft_sharedptr<ft_research_definition> planning_terra(new ft_research_definition());
    planning_terra->id = RESEARCH_URBAN_PLANNING_TERRA;
    planning_terra->name = ft_string("Urban Planning I");
    planning_terra->duration = 20.0;
    planning_terra->prerequisites.clear();
    planning_terra->costs.clear();
    cost.key = ITEM_IRON_BAR;
    cost.value = 15;
    planning_terra->costs.push_back(cost);
    cost.key = ITEM_ENGINE_PART;
    cost.value = 5;
    planning_terra->costs.push_back(cost);
    planning_terra->unlock_planets.clear();
    this->register_research(planning_terra);

    ft_sharedptr<ft_research_definition> planning_mars(new ft_research_definition());
    planning_mars->id = RESEARCH_URBAN_PLANNING_MARS;
    planning_mars->name = ft_string("Urban Planning II");
    planning_mars->duration = 25.0;
    planning_mars->prerequisites.clear();
    planning_mars->prerequisites.push_back(RESEARCH_UNLOCK_MARS);
    planning_mars->costs.clear();
    cost.key = ITEM_IRON_BAR;
    cost.value = 30;
    planning_mars->costs.push_back(cost);
    cost.key = ITEM_ENGINE_PART;
    cost.value = 10;
    planning_mars->costs.push_back(cost);
    planning_mars->unlock_planets.clear();
    this->register_research(planning_mars);

    ft_sharedptr<ft_research_definition> planning_zalthor(new ft_research_definition());
    planning_zalthor->id = RESEARCH_URBAN_PLANNING_ZALTHOR;
    planning_zalthor->name = ft_string("Urban Planning III");
    planning_zalthor->duration = 30.0;
    planning_zalthor->prerequisites.clear();
    planning_zalthor->prerequisites.push_back(RESEARCH_UNLOCK_ZALTHOR);
    planning_zalthor->costs.clear();
    cost.key = ITEM_IRON_BAR;
    cost.value = 45;
    planning_zalthor->costs.push_back(cost);
    cost.key = ITEM_ENGINE_PART;
    cost.value = 15;
    planning_zalthor->costs.push_back(cost);
    planning_zalthor->unlock_planets.clear();
    this->register_research(planning_zalthor);

    ft_sharedptr<ft_research_definition> solar(new ft_research_definition());
    solar->id = RESEARCH_SOLAR_PANELS;
    solar->name = ft_string("Solar Panel Engineering");
    solar->duration = 25.0;
    solar->prerequisites.clear();
    solar->prerequisites.push_back(RESEARCH_UNLOCK_MARS);
    solar->costs.clear();
    cost.key = ORE_IRON;
    cost.value = 20;
    solar->costs.push_back(cost);
    cost.key = ORE_COPPER;
    cost.value = 30;
    solar->costs.push_back(cost);
    cost.key = ITEM_ACCUMULATOR;
    cost.value = 2;
    solar->costs.push_back(cost);
    solar->unlock_planets.clear();
    this->register_research(solar);

    ft_sharedptr<ft_research_definition> mastery(new ft_research_definition());
    mastery->id = RESEARCH_CRAFTING_MASTERY;
    mastery->name = ft_string("Crafting Mastery");
    mastery->duration = 35.0;
    mastery->prerequisites.clear();
    mastery->prerequisites.push_back(RESEARCH_SOLAR_PANELS);
    mastery->costs.clear();
    cost.key = ITEM_ADVANCED_ENGINE_PART;
    cost.value = 5;
    mastery->costs.push_back(cost);
    cost.key = ITEM_TITANIUM_BAR;
    cost.value = 10;
    mastery->costs.push_back(cost);
    cost.key = ITEM_FUSION_REACTOR;
    cost.value = 1;
    mastery->costs.push_back(cost);
    mastery->unlock_planets.clear();
    this->register_research(mastery);

    ft_sharedptr<ft_research_definition> trade(new ft_research_definition());
    trade->id = RESEARCH_INTERSTELLAR_TRADE;
    trade->name = ft_string("Interstellar Trade Networks");
    trade->duration = 32.0;
    trade->prerequisites.clear();
    trade->prerequisites.push_back(RESEARCH_CRAFTING_MASTERY);
    trade->costs.clear();
    cost.key = ITEM_ADVANCED_ENGINE_PART;
    cost.value = 6;
    trade->costs.push_back(cost);
    cost.key = ITEM_ACCUMULATOR;
    cost.value = 3;
    trade->costs.push_back(cost);
    cost.key = ITEM_TITANIUM_BAR;
    cost.value = 6;
    trade->costs.push_back(cost);
    trade->unlock_planets.clear();
    this->register_research(trade);

    ft_sharedptr<ft_research_definition> structural_i(new ft_research_definition());
    structural_i->id = RESEARCH_STRUCTURAL_REINFORCEMENT_I;
    structural_i->name = ft_string("Structural Reinforcement I");
    structural_i->duration = 25.0;
    structural_i->prerequisites.clear();
    structural_i->prerequisites.push_back(RESEARCH_UNLOCK_MARS);
    structural_i->costs.clear();
    cost.key = ITEM_IRON_BAR;
    cost.value = 10;
    structural_i->costs.push_back(cost);
    cost.key = ORE_COAL;
    cost.value = 10;
    structural_i->costs.push_back(cost);
    structural_i->unlock_planets.clear();
    this->register_research(structural_i);

    ft_sharedptr<ft_research_definition> structural_ii(new ft_research_definition());
    structural_ii->id = RESEARCH_STRUCTURAL_REINFORCEMENT_II;
    structural_ii->name = ft_string("Structural Reinforcement II");
    structural_ii->duration = 35.0;
    structural_ii->prerequisites.clear();
    structural_ii->prerequisites.push_back(RESEARCH_STRUCTURAL_REINFORCEMENT_I);
    structural_ii->costs.clear();
    cost.key = ITEM_IRON_BAR;
    cost.value = 20;
    structural_ii->costs.push_back(cost);
    cost.key = ORE_COAL;
    cost.value = 20;
    structural_ii->costs.push_back(cost);
    structural_ii->unlock_planets.clear();
    this->register_research(structural_ii);

    ft_sharedptr<ft_research_definition> structural_iii(new ft_research_definition());
    structural_iii->id = RESEARCH_STRUCTURAL_REINFORCEMENT_III;
    structural_iii->name = ft_string("Structural Reinforcement III");
    structural_iii->duration = 45.0;
    structural_iii->prerequisites.clear();
    structural_iii->prerequisites.push_back(RESEARCH_STRUCTURAL_REINFORCEMENT_II);
    structural_iii->costs.clear();
    cost.key = ITEM_IRON_BAR;
    cost.value = 30;
    structural_iii->costs.push_back(cost);
    cost.key = ORE_COAL;
    cost.value = 30;
    structural_iii->costs.push_back(cost);
    structural_iii->unlock_planets.clear();
    this->register_research(structural_iii);

    ft_sharedptr<ft_research_definition> defensive_i(new ft_research_definition());
    defensive_i->id = RESEARCH_DEFENSIVE_FORTIFICATION_I;
    defensive_i->name = ft_string("Defensive Fortification I");
    defensive_i->duration = 30.0;
    defensive_i->prerequisites.clear();
    defensive_i->prerequisites.push_back(RESEARCH_UNLOCK_ZALTHOR);
    defensive_i->costs.clear();
    cost.key = ITEM_COPPER_BAR;
    cost.value = 10;
    defensive_i->costs.push_back(cost);
    cost.key = ITEM_MITHRIL_BAR;
    cost.value = 5;
    defensive_i->costs.push_back(cost);
    defensive_i->unlock_planets.clear();
    this->register_research(defensive_i);

    ft_sharedptr<ft_research_definition> defensive_ii(new ft_research_definition());
    defensive_ii->id = RESEARCH_DEFENSIVE_FORTIFICATION_II;
    defensive_ii->name = ft_string("Defensive Fortification II");
    defensive_ii->duration = 40.0;
    defensive_ii->prerequisites.clear();
    defensive_ii->prerequisites.push_back(RESEARCH_DEFENSIVE_FORTIFICATION_I);
    defensive_ii->costs.clear();
    cost.key = ITEM_COPPER_BAR;
    cost.value = 20;
    defensive_ii->costs.push_back(cost);
    cost.key = ITEM_MITHRIL_BAR;
    cost.value = 10;
    defensive_ii->costs.push_back(cost);
    defensive_ii->unlock_planets.clear();
    this->register_research(defensive_ii);

    ft_sharedptr<ft_research_definition> defensive_iii(new ft_research_definition());
    defensive_iii->id = RESEARCH_DEFENSIVE_FORTIFICATION_III;
    defensive_iii->name = ft_string("Defensive Fortification III");
    defensive_iii->duration = 50.0;
    defensive_iii->prerequisites.clear();
    defensive_iii->prerequisites.push_back(RESEARCH_DEFENSIVE_FORTIFICATION_II);
    defensive_iii->costs.clear();
    cost.key = ITEM_COPPER_BAR;
    cost.value = 30;
    defensive_iii->costs.push_back(cost);
    cost.key = ITEM_MITHRIL_BAR;
    cost.value = 15;
    defensive_iii->costs.push_back(cost);
    defensive_iii->unlock_planets.clear();
    this->register_research(defensive_iii);

    ft_sharedptr<ft_research_definition> armament_i(new ft_research_definition());
    armament_i->id = RESEARCH_ARMAMENT_ENHANCEMENT_I;
    armament_i->name = ft_string("Armament Enhancement I");
    armament_i->duration = 35.0;
    armament_i->prerequisites.clear();
    armament_i->prerequisites.push_back(RESEARCH_UNLOCK_VULCAN);
    armament_i->costs.clear();
    cost.key = ITEM_ADVANCED_ENGINE_PART;
    cost.value = 5;
    armament_i->costs.push_back(cost);
    cost.key = ITEM_TITANIUM_BAR;
    cost.value = 5;
    armament_i->costs.push_back(cost);
    armament_i->unlock_planets.clear();
    this->register_research(armament_i);

    ft_sharedptr<ft_research_definition> armament_ii(new ft_research_definition());
    armament_ii->id = RESEARCH_ARMAMENT_ENHANCEMENT_II;
    armament_ii->name = ft_string("Armament Enhancement II");
    armament_ii->duration = 45.0;
    armament_ii->prerequisites.clear();
    armament_ii->prerequisites.push_back(RESEARCH_ARMAMENT_ENHANCEMENT_I);
    armament_ii->costs.clear();
    cost.key = ITEM_ADVANCED_ENGINE_PART;
    cost.value = 10;
    armament_ii->costs.push_back(cost);
    cost.key = ITEM_TITANIUM_BAR;
    cost.value = 10;
    armament_ii->costs.push_back(cost);
    cost.key = ORE_TRITIUM;
    cost.value = 5;
    armament_ii->costs.push_back(cost);
    armament_ii->unlock_planets.clear();
    this->register_research(armament_ii);

    ft_sharedptr<ft_research_definition> armament_iii(new ft_research_definition());
    armament_iii->id = RESEARCH_ARMAMENT_ENHANCEMENT_III;
    armament_iii->name = ft_string("Armament Enhancement III");
    armament_iii->duration = 55.0;
    armament_iii->prerequisites.clear();
    armament_iii->prerequisites.push_back(RESEARCH_ARMAMENT_ENHANCEMENT_II);
    armament_iii->costs.clear();
    cost.key = ITEM_ADVANCED_ENGINE_PART;
    cost.value = 15;
    armament_iii->costs.push_back(cost);
    cost.key = ITEM_TITANIUM_BAR;
    cost.value = 15;
    armament_iii->costs.push_back(cost);
    cost.key = ORE_TRITIUM;
    cost.value = 10;
    armament_iii->costs.push_back(cost);
    cost.key = ITEM_FUSION_REACTOR;
    cost.value = 1;
    armament_iii->costs.push_back(cost);
    armament_iii->unlock_planets.clear();
    this->register_research(armament_iii);

    ft_sharedptr<ft_research_definition> luna(new ft_research_definition());
    luna->id = RESEARCH_UNLOCK_LUNA;
    luna->name = ft_string("Unlock Luna");
    luna->duration = 35.0;
    luna->prerequisites.clear();
    luna->prerequisites.push_back(RESEARCH_UNLOCK_MARS);
    luna->costs.clear();
    cost.key = ORE_IRON;
    cost.value = 25;
    luna->costs.push_back(cost);
    cost.key = ORE_COPPER;
    cost.value = 20;
    luna->costs.push_back(cost);
    cost.key = ITEM_ENGINE_PART;
    cost.value = 6;
    luna->costs.push_back(cost);
    luna->unlock_planets.clear();
    luna->unlock_planets.push_back(PLANET_LUNA);
    this->register_research(luna);

    ft_sharedptr<ft_research_definition> faster_crafting(new ft_research_definition());
    faster_crafting->id = RESEARCH_FASTER_CRAFTING;
    faster_crafting->name = ft_string("Faster Crafting");
    faster_crafting->duration = 40.0;
    faster_crafting->prerequisites.clear();
    faster_crafting->prerequisites.push_back(RESEARCH_CRAFTING_MASTERY);
    faster_crafting->costs.clear();
    cost.key = ITEM_ADVANCED_ENGINE_PART;
    cost.value = 8;
    faster_crafting->costs.push_back(cost);
    cost.key = ITEM_TITANIUM_BAR;
    cost.value = 12;
    faster_crafting->costs.push_back(cost);
    cost.key = ITEM_ACCUMULATOR;
    cost.value = 4;
    faster_crafting->costs.push_back(cost);
    faster_crafting->unlock_planets.clear();
    this->register_research(faster_crafting);

    ft_sharedptr<ft_research_definition> shield(new ft_research_definition());
    shield->id = RESEARCH_SHIELD_TECHNOLOGY;
    shield->name = ft_string("Shield Technology");
    shield->duration = 35.0;
    shield->prerequisites.clear();
    shield->prerequisites.push_back(RESEARCH_DEFENSIVE_FORTIFICATION_I);
    shield->costs.clear();
    cost.key = ITEM_COPPER_BAR;
    cost.value = 15;
    shield->costs.push_back(cost);
    cost.key = ITEM_MITHRIL_BAR;
    cost.value = 12;
    shield->costs.push_back(cost);
    cost.key = ITEM_ACCUMULATOR;
    cost.value = 2;
    shield->costs.push_back(cost);
    shield->unlock_planets.clear();
    this->register_research(shield);

    ft_sharedptr<ft_research_definition> emergency(new ft_research_definition());
    emergency->id = RESEARCH_EMERGENCY_ENERGY_CONSERVATION;
    emergency->name = ft_string("Emergency Energy Conservation");
    emergency->duration = 30.0;
    emergency->prerequisites.clear();
    emergency->prerequisites.push_back(RESEARCH_SOLAR_PANELS);
    emergency->costs.clear();
    cost.key = ORE_COAL;
    cost.value = 20;
    emergency->costs.push_back(cost);
    cost.key = ITEM_ACCUMULATOR;
    cost.value = 3;
    emergency->costs.push_back(cost);
    emergency->unlock_planets.clear();
    this->register_research(emergency);

    ft_sharedptr<ft_research_definition> repair(new ft_research_definition());
    repair->id = RESEARCH_REPAIR_DRONE_TECHNOLOGY;
    repair->name = ft_string("Repair Drone Technology");
    repair->duration = 45.0;
    repair->prerequisites.clear();
    repair->prerequisites.push_back(RESEARCH_SHIELD_TECHNOLOGY);
    repair->prerequisites.push_back(RESEARCH_DEFENSIVE_FORTIFICATION_II);
    repair->costs.clear();
    cost.key = ITEM_ADVANCED_ENGINE_PART;
    cost.value = 6;
    repair->costs.push_back(cost);
    cost.key = ITEM_ACCUMULATOR;
    cost.value = 4;
    repair->costs.push_back(cost);
    cost.key = ITEM_FUSION_REACTOR;
    cost.value = 1;
    repair->costs.push_back(cost);
    repair->unlock_planets.clear();
    this->register_research(repair);

    ft_sharedptr<ft_research_definition> tritium(new ft_research_definition());
    tritium->id = RESEARCH_TRITIUM_EXTRACTION;
    tritium->name = ft_string("Tritium Extraction");
    tritium->duration = 50.0;
    tritium->prerequisites.clear();
    tritium->prerequisites.push_back(RESEARCH_UNLOCK_NOCTARIS);
    tritium->costs.clear();
    cost.key = ORE_OBSIDIAN;
    cost.value = 12;
    tritium->costs.push_back(cost);
    cost.key = ORE_CRYSTAL;
    cost.value = 6;
    tritium->costs.push_back(cost);
    cost.key = ITEM_ACCUMULATOR;
    cost.value = 3;
    tritium->costs.push_back(cost);
    tritium->unlock_planets.clear();
    this->register_research(tritium);

    ft_sharedptr<ft_research_definition> capital(new ft_research_definition());
    capital->id = RESEARCH_CAPITAL_SHIP_INITIATIVE;
    capital->name = ft_string("Capital Ship Initiative");
    capital->duration = 55.0;
    capital->prerequisites.clear();
    capital->prerequisites.push_back(RESEARCH_ARMAMENT_ENHANCEMENT_II);
    capital->costs.clear();
    cost.key = ITEM_ADVANCED_ENGINE_PART;
    cost.value = 12;
    capital->costs.push_back(cost);
    cost.key = ITEM_TITANIUM_BAR;
    cost.value = 12;
    capital->costs.push_back(cost);
    cost.key = ITEM_FUSION_REACTOR;
    cost.value = 2;
    capital->costs.push_back(cost);
    capital->unlock_planets.clear();
    this->register_research(capital);

    ft_sharedptr<ft_research_definition> auxiliary(new ft_research_definition());
    auxiliary->id = RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT;
    auxiliary->name = ft_string("Auxiliary Frigate Development");
    auxiliary->duration = 60.0;
    auxiliary->prerequisites.clear();
    auxiliary->prerequisites.push_back(RESEARCH_CAPITAL_SHIP_INITIATIVE);
    auxiliary->costs.clear();
    cost.key = ITEM_ADVANCED_ENGINE_PART;
    cost.value = 15;
    auxiliary->costs.push_back(cost);
    cost.key = ITEM_TITANIUM_BAR;
    cost.value = 15;
    auxiliary->costs.push_back(cost);
    cost.key = ORE_TRITIUM;
    cost.value = 6;
    auxiliary->costs.push_back(cost);
    cost.key = ITEM_FUSION_REACTOR;
    cost.value = 2;
    auxiliary->costs.push_back(cost);
    auxiliary->unlock_planets.clear();
    this->register_research(auxiliary);

    ft_sharedptr<ft_research_definition> escape(new ft_research_definition());
    escape->id = RESEARCH_ESCAPE_POD_LIFELINE;
    escape->name = ft_string("Escape Pod Lifeline");
    escape->duration = 45.0;
    escape->prerequisites.clear();
    escape->prerequisites.push_back(RESEARCH_REPAIR_DRONE_TECHNOLOGY);
    escape->costs.clear();
    cost.key = ITEM_ACCUMULATOR;
    cost.value = 5;
    escape->costs.push_back(cost);
    cost.key = ORE_TRITIUM;
    cost.value = 4;
    escape->costs.push_back(cost);
    cost.key = ITEM_FUSION_REACTOR;
    cost.value = 1;
    escape->costs.push_back(cost);
    escape->unlock_planets.clear();
    this->register_research(escape);

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
