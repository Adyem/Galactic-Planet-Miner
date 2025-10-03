#include "achievements.hpp"
#include "libft/Template/move.hpp"

AchievementManager::AchievementManager()
{
    ft_sharedptr<ft_achievement_definition> second_home(new ft_achievement_definition());
    second_home->id = ACHIEVEMENT_SECOND_HOME;
    second_home->name = ft_string("Second Home");
    second_home->description = ft_string("Unlock an additional colony world.");
    second_home->event_id = ACHIEVEMENT_EVENT_PLANET_UNLOCKED;
    second_home->target_value = 1;
    second_home->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(second_home);

    ft_sharedptr<ft_achievement_definition> pioneer(new ft_achievement_definition());
    pioneer->id = ACHIEVEMENT_RESEARCH_PIONEER;
    pioneer->name = ft_string("Research Pioneer");
    pioneer->description = ft_string("Complete three major research projects.");
    pioneer->event_id = ACHIEVEMENT_EVENT_RESEARCH_COMPLETED;
    pioneer->target_value = 3;
    pioneer->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(pioneer);

    ft_sharedptr<ft_achievement_definition> pilot(new ft_achievement_definition());
    pilot->id = ACHIEVEMENT_LOGISTICS_PILOT;
    pilot->name = ft_string("Logistics Pilot");
    pilot->description = ft_string("Deliver three convoys successfully.");
    pilot->event_id = ACHIEVEMENT_EVENT_CONVOY_DELIVERED;
    pilot->target_value = 3;
    pilot->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(pilot);

    ft_sharedptr<ft_achievement_definition> guardian(new ft_achievement_definition());
    guardian->id = ACHIEVEMENT_CONVOY_STREAK_GUARDIAN;
    guardian->name = ft_string("Streak Guardian");
    guardian->description = ft_string("Maintain a five convoy success streak.");
    guardian->event_id = ACHIEVEMENT_EVENT_CONVOY_STREAK_BEST;
    guardian->target_value = 5;
    guardian->progress_mode = ACHIEVEMENT_PROGRESS_BEST_VALUE;
    this->register_achievement(guardian);

    ft_sharedptr<ft_achievement_definition> skirmish(new ft_achievement_definition());
    skirmish->id = ACHIEVEMENT_QUEST_INITIAL_SKIRMISHES;
    skirmish->name = ft_string("Perimeter Guardian");
    skirmish->description = ft_string("Complete the Initial Raider Skirmishes quest.");
    skirmish->event_id = ACHIEVEMENT_EVENT_QUEST_INITIAL_SKIRMISHES;
    skirmish->target_value = 1;
    skirmish->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(skirmish);

    ft_sharedptr<ft_achievement_definition> defense(new ft_achievement_definition());
    defense->id = ACHIEVEMENT_QUEST_DEFENSE_OF_TERRA;
    defense->name = ft_string("Terra's Shield");
    defense->description = ft_string("Complete the Defense of Terra quest.");
    defense->event_id = ACHIEVEMENT_EVENT_QUEST_DEFENSE_OF_TERRA;
    defense->target_value = 1;
    defense->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(defense);

    ft_sharedptr<ft_achievement_definition> investigate(new ft_achievement_definition());
    investigate->id = ACHIEVEMENT_QUEST_INVESTIGATE_RAIDERS;
    investigate->name = ft_string("Cipher Breaker");
    investigate->description = ft_string("Complete the Investigate Raider Motives quest.");
    investigate->event_id = ACHIEVEMENT_EVENT_QUEST_INVESTIGATE_RAIDERS;
    investigate->target_value = 1;
    investigate->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(investigate);

    ft_sharedptr<ft_achievement_definition> supply(new ft_achievement_definition());
    supply->id = ACHIEVEMENT_QUEST_SECURE_SUPPLY_LINES;
    supply->name = ft_string("Supply Line Sentinel");
    supply->description = ft_string("Complete the Secure Supply Lines quest.");
    supply->event_id = ACHIEVEMENT_EVENT_QUEST_SECURE_SUPPLY_LINES;
    supply->target_value = 1;
    supply->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(supply);

    ft_sharedptr<ft_achievement_definition> streak(new ft_achievement_definition());
    streak->id = ACHIEVEMENT_QUEST_STEADY_SUPPLY_STREAK;
    streak->name = ft_string("Streak Specialist");
    streak->description = ft_string("Complete the Steady Supply Streak quest.");
    streak->event_id = ACHIEVEMENT_EVENT_QUEST_STEADY_SUPPLY_STREAK;
    streak->target_value = 1;
    streak->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(streak);

    ft_sharedptr<ft_achievement_definition> escort(new ft_achievement_definition());
    escort->id = ACHIEVEMENT_QUEST_HIGH_VALUE_ESCORT;
    escort->name = ft_string("Escort Commander");
    escort->description = ft_string("Complete the High-Value Escort quest.");
    escort->event_id = ACHIEVEMENT_EVENT_QUEST_HIGH_VALUE_ESCORT;
    escort->target_value = 1;
    escort->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(escort);

    ft_sharedptr<ft_achievement_definition> battle(new ft_achievement_definition());
    battle->id = ACHIEVEMENT_QUEST_CLIMACTIC_BATTLE;
    battle->name = ft_string("Climactic Victor");
    battle->description = ft_string("Complete the Climactic Battle quest.");
    battle->event_id = ACHIEVEMENT_EVENT_QUEST_CLIMACTIC_BATTLE;
    battle->target_value = 1;
    battle->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(battle);

    ft_sharedptr<ft_achievement_definition> decision(new ft_achievement_definition());
    decision->id = ACHIEVEMENT_QUEST_CRITICAL_DECISION;
    decision->name = ft_string("Decisive Arbiter");
    decision->description = ft_string("Resolve The Critical Decision quest.");
    decision->event_id = ACHIEVEMENT_EVENT_QUEST_CRITICAL_DECISION;
    decision->target_value = 1;
    decision->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(decision);

    ft_sharedptr<ft_achievement_definition> order(new ft_achievement_definition());
    order->id = ACHIEVEMENT_QUEST_ORDER_UPRISING;
    order->name = ft_string("Order's Hammer");
    order->description = ft_string("Complete the Order's Last Stand quest.");
    order->event_id = ACHIEVEMENT_EVENT_QUEST_ORDER_UPRISING;
    order->target_value = 1;
    order->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(order);

    ft_sharedptr<ft_achievement_definition> rebellion(new ft_achievement_definition());
    rebellion->id = ACHIEVEMENT_QUEST_REBELLION_FLEET;
    rebellion->name = ft_string("Rebellion's Hope");
    rebellion->description = ft_string("Complete the Rebellion Rising quest.");
    rebellion->event_id = ACHIEVEMENT_EVENT_QUEST_REBELLION_FLEET;
    rebellion->target_value = 1;
    rebellion->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(rebellion);

    ft_sharedptr<ft_achievement_definition> suppress(new ft_achievement_definition());
    suppress->id = ACHIEVEMENT_QUEST_ORDER_SUPPRESS_RAIDS;
    suppress->name = ft_string("Sensor Web Architect");
    suppress->description = ft_string("Complete the Suppress the Raider Cells quest.");
    suppress->event_id = ACHIEVEMENT_EVENT_QUEST_ORDER_SUPPRESS_RAIDS;
    suppress->target_value = 1;
    suppress->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(suppress);

    ft_sharedptr<ft_achievement_definition> dominion(new ft_achievement_definition());
    dominion->id = ACHIEVEMENT_QUEST_ORDER_DOMINION;
    dominion->name = ft_string("Dominion Enforcer");
    dominion->description = ft_string("Complete the Order Dominion quest.");
    dominion->event_id = ACHIEVEMENT_EVENT_QUEST_ORDER_DOMINION;
    dominion->target_value = 1;
    dominion->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(dominion);

    ft_sharedptr<ft_achievement_definition> tribunal(new ft_achievement_definition());
    tribunal->id = ACHIEVEMENT_QUEST_ORDER_FINAL_VERDICT;
    tribunal->name = ft_string("Order Tribunal Arbiter");
    tribunal->description = ft_string("Resolve the Final Order Mandate quest.");
    tribunal->event_id = ACHIEVEMENT_EVENT_QUEST_ORDER_FINAL_VERDICT;
    tribunal->target_value = 1;
    tribunal->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(tribunal);

    ft_sharedptr<ft_achievement_definition> network(new ft_achievement_definition());
    network->id = ACHIEVEMENT_QUEST_REBELLION_NETWORK;
    network->name = ft_string("Shadow Network Weaver");
    network->description = ft_string("Complete the Shadow Network quest.");
    network->event_id = ACHIEVEMENT_EVENT_QUEST_REBELLION_NETWORK;
    network->target_value = 1;
    network->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(network);

    ft_sharedptr<ft_achievement_definition> liberation(new ft_achievement_definition());
    liberation->id = ACHIEVEMENT_QUEST_REBELLION_LIBERATION;
    liberation->name = ft_string("Liberation Vanguard");
    liberation->description = ft_string("Complete the Liberation of the Frontier quest.");
    liberation->event_id = ACHIEVEMENT_EVENT_QUEST_REBELLION_LIBERATION;
    liberation->target_value = 1;
    liberation->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(liberation);

    ft_sharedptr<ft_achievement_definition> final_push(new ft_achievement_definition());
    final_push->id = ACHIEVEMENT_QUEST_REBELLION_FINAL_PUSH;
    final_push->name = ft_string("Freedom's Edge");
    final_push->description = ft_string("Complete the Battle for Freedom quest.");
    final_push->event_id = ACHIEVEMENT_EVENT_QUEST_REBELLION_FINAL_PUSH;
    final_push->target_value = 1;
    final_push->progress_mode = ACHIEVEMENT_PROGRESS_INCREMENTAL;
    this->register_achievement(final_push);
}

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
