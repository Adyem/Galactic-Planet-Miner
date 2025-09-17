#include "quests.hpp"

QuestManager::QuestManager()
    : _time_scale(1.0)
{
    ft_sharedptr<ft_quest_definition> skirmish(new ft_quest_definition());
    skirmish->id = QUEST_INITIAL_SKIRMISHES;
    skirmish->name = ft_string("Initial Raider Skirmishes");
    skirmish->description = ft_string("Protect supply convoys and fortify Terra's perimeter.");
    skirmish->time_limit = 0.0;
    skirmish->requires_choice = false;
    skirmish->required_choice_quest = 0;
    skirmish->required_choice_value = 0;
    ft_quest_objective objective;
    objective.type = QUEST_OBJECTIVE_RESOURCE_TOTAL;
    objective.target_id = ORE_IRON;
    objective.amount = 10;
    skirmish->objectives.push_back(objective);
    objective.target_id = ORE_COPPER;
    objective.amount = 10;
    skirmish->objectives.push_back(objective);
    this->register_quest(skirmish);

    ft_sharedptr<ft_quest_definition> defense(new ft_quest_definition());
    defense->id = QUEST_DEFENSE_OF_TERRA;
    defense->name = ft_string("Defense of Terra");
    defense->description = ft_string("Assemble a defensive wing to repel raider assaults.");
    defense->time_limit = 180.0;
    defense->requires_choice = false;
    defense->required_choice_quest = 0;
    defense->required_choice_value = 0;
    defense->prerequisites.push_back(QUEST_INITIAL_SKIRMISHES);
    objective.type = QUEST_OBJECTIVE_FLEET_COUNT;
    objective.target_id = 0;
    objective.amount = 2;
    defense->objectives.push_back(objective);
    objective.type = QUEST_OBJECTIVE_TOTAL_SHIP_HP;
    objective.target_id = 0;
    objective.amount = 120;
    defense->objectives.push_back(objective);
    this->register_quest(defense);

    ft_sharedptr<ft_quest_definition> investigate(new ft_quest_definition());
    investigate->id = QUEST_INVESTIGATE_RAIDERS;
    investigate->name = ft_string("Investigate Raider Motives");
    investigate->description = ft_string("Complete research to uncover the raiders' plans.");
    investigate->time_limit = 240.0;
    investigate->requires_choice = false;
    investigate->required_choice_quest = 0;
    investigate->required_choice_value = 0;
    investigate->prerequisites.push_back(QUEST_DEFENSE_OF_TERRA);
    objective.type = QUEST_OBJECTIVE_RESEARCH_COMPLETED;
    objective.target_id = RESEARCH_UNLOCK_MARS;
    objective.amount = 1;
    investigate->objectives.push_back(objective);
    objective.target_id = RESEARCH_UNLOCK_ZALTHOR;
    objective.amount = 1;
    investigate->objectives.push_back(objective);
    this->register_quest(investigate);

    ft_sharedptr<ft_quest_definition> battle(new ft_quest_definition());
    battle->id = QUEST_CLIMACTIC_BATTLE;
    battle->name = ft_string("Climactic Battle");
    battle->description = ft_string("Prepare the fleets and technology for the climactic assault.");
    battle->time_limit = 300.0;
    battle->requires_choice = false;
    battle->required_choice_quest = 0;
    battle->required_choice_value = 0;
    battle->prerequisites.push_back(QUEST_INVESTIGATE_RAIDERS);
    objective.type = QUEST_OBJECTIVE_RESEARCH_COMPLETED;
    objective.target_id = RESEARCH_UNLOCK_VULCAN;
    objective.amount = 1;
    battle->objectives.push_back(objective);
    objective.type = QUEST_OBJECTIVE_TOTAL_SHIP_HP;
    objective.target_id = 0;
    objective.amount = 180;
    battle->objectives.push_back(objective);
    this->register_quest(battle);

    ft_sharedptr<ft_quest_definition> decision(new ft_quest_definition());
    decision->id = QUEST_CRITICAL_DECISION;
    decision->name = ft_string("The Critical Decision");
    decision->description = ft_string("Decide Blackthorne's fate and set the course for the system.");
    decision->time_limit = 0.0;
    decision->requires_choice = true;
    decision->required_choice_quest = 0;
    decision->required_choice_value = 0;
    decision->prerequisites.push_back(QUEST_CLIMACTIC_BATTLE);
    ft_quest_choice_definition choice;
    choice.choice_id = QUEST_CHOICE_EXECUTE_BLACKTHORNE;
    choice.description = ft_string("Execute Blackthorne to preserve order.");
    decision->choices.push_back(choice);
    choice.choice_id = QUEST_CHOICE_SPARE_BLACKTHORNE;
    choice.description = ft_string("Spare Blackthorne and investigate corruption.");
    decision->choices.push_back(choice);
    this->register_quest(decision);

    ft_sharedptr<ft_quest_definition> order(new ft_quest_definition());
    order->id = QUEST_ORDER_UPRISING;
    order->name = ft_string("Order's Last Stand");
    order->description = ft_string("Crush the uprising that rises after Blackthorne's execution.");
    order->time_limit = 360.0;
    order->requires_choice = false;
    order->required_choice_quest = QUEST_CRITICAL_DECISION;
    order->required_choice_value = QUEST_CHOICE_EXECUTE_BLACKTHORNE;
    order->prerequisites.push_back(QUEST_CRITICAL_DECISION);
    objective.type = QUEST_OBJECTIVE_RESOURCE_TOTAL;
    objective.target_id = ORE_COAL;
    objective.amount = 20;
    order->objectives.push_back(objective);
    this->register_quest(order);

    ft_sharedptr<ft_quest_definition> rebellion(new ft_quest_definition());
    rebellion->id = QUEST_REBELLION_FLEET;
    rebellion->name = ft_string("Rebellion Rising");
    rebellion->description = ft_string("Rally new allies after sparing Blackthorne.");
    rebellion->time_limit = 360.0;
    rebellion->requires_choice = false;
    rebellion->required_choice_quest = QUEST_CRITICAL_DECISION;
    rebellion->required_choice_value = QUEST_CHOICE_SPARE_BLACKTHORNE;
    rebellion->prerequisites.push_back(QUEST_CRITICAL_DECISION);
    objective.type = QUEST_OBJECTIVE_RESEARCH_COMPLETED;
    objective.target_id = RESEARCH_UNLOCK_NOCTARIS;
    objective.amount = 1;
    rebellion->objectives.push_back(objective);
    objective.type = QUEST_OBJECTIVE_RESOURCE_TOTAL;
    objective.target_id = ORE_OBSIDIAN;
    objective.amount = 4;
    rebellion->objectives.push_back(objective);
    this->register_quest(rebellion);

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

void QuestManager::activate_next()
{
    size_t count = this->_progress.size();
    if (count == 0)
        return ;
    Pair<int, ft_quest_progress> *entries = this->_progress.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        if (entries[i].value.status == QUEST_STATUS_ACTIVE || entries[i].value.status == QUEST_STATUS_AWAITING_CHOICE)
            return ;
    }
    int next_id = 0;
    for (size_t i = 0; i < count; ++i)
    {
        if (entries[i].value.status == QUEST_STATUS_AVAILABLE)
        {
            if (next_id == 0 || entries[i].key < next_id)
                next_id = entries[i].key;
        }
    }
    if (next_id == 0)
        return ;
    Pair<int, ft_quest_progress> *progress_entry = this->_progress.find(next_id);
    if (progress_entry == ft_nullptr)
        return ;
    progress_entry->value.status = QUEST_STATUS_ACTIVE;
    const ft_quest_definition *definition = this->get_definition(next_id);
    if (definition != ft_nullptr && definition->time_limit > 0.0)
        progress_entry->value.time_remaining = definition->time_limit * this->_time_scale;
    else
        progress_entry->value.time_remaining = 0.0;
}

bool QuestManager::are_objectives_met(const ft_quest_definition &definition, const ft_quest_context &context) const
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
            }
            else
            {
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
        if (progress.status == QUEST_STATUS_ACTIVE || progress.status == QUEST_STATUS_AVAILABLE)
        {
            progress.time_remaining *= ratio;
            double max_time = definition->time_limit * this->_time_scale;
            if (progress.time_remaining > max_time)
                progress.time_remaining = max_time;
        }
    }
}

int QuestManager::get_active_quest_id() const
{
    size_t count = this->_progress.size();
    const Pair<int, ft_quest_progress> *entries = this->_progress.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        if (entries[i].value.status == QUEST_STATUS_AWAITING_CHOICE)
            return entries[i].key;
    }
    for (size_t i = 0; i < count; ++i)
    {
        if (entries[i].value.status == QUEST_STATUS_ACTIVE)
            return entries[i].key;
    }
    return 0;
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
