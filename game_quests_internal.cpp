namespace
{
static double get_objective_current_amount(const ft_quest_objective &objective,
                                           const ft_quest_context &context)
{
    if (objective.type == QUEST_OBJECTIVE_RESOURCE_TOTAL)
    {
        const Pair<int, int> *entry = context.resource_totals.find(objective.target_id);
        if (entry == ft_nullptr)
            return 0.0;
        return static_cast<double>(entry->value);
    }
    if (objective.type == QUEST_OBJECTIVE_RESEARCH_COMPLETED)
    {
        const Pair<int, int> *entry = context.research_status.find(objective.target_id);
        if (entry == ft_nullptr)
            return 0.0;
        return static_cast<double>(entry->value);
    }
    if (objective.type == QUEST_OBJECTIVE_FLEET_COUNT)
        return static_cast<double>(context.total_ship_count);
    if (objective.type == QUEST_OBJECTIVE_TOTAL_SHIP_HP)
        return static_cast<double>(context.total_ship_hp);
    if (objective.type == QUEST_OBJECTIVE_CONVOYS_DELIVERED)
        return static_cast<double>(context.successful_deliveries);
    if (objective.type == QUEST_OBJECTIVE_CONVOY_STREAK)
        return static_cast<double>(context.delivery_streak);
    if (objective.type == QUEST_OBJECTIVE_CONVOY_RAID_LOSSES_AT_MOST)
        return static_cast<double>(context.convoy_raid_losses);
    if (objective.type == QUEST_OBJECTIVE_MAX_CONVOY_THREAT_AT_MOST)
        return context.maximum_convoy_threat * 100.0;
    if (objective.type == QUEST_OBJECTIVE_BUILDING_COUNT)
    {
        const Pair<int, int> *entry = context.building_counts.find(objective.target_id);
        if (entry == ft_nullptr)
            return 0.0;
        return static_cast<double>(entry->value);
    }
    if (objective.type == QUEST_OBJECTIVE_ASSAULT_VICTORIES)
    {
        const Pair<int, int> *entry = context.assault_victories.find(objective.target_id);
        if (entry == ft_nullptr)
            return 0.0;
        return static_cast<double>(entry->value);
    }
    return 0.0;
}

static bool is_objective_met_for_snapshot(const ft_quest_objective &objective,
                                          const ft_quest_context &context)
{
    if (objective.type == QUEST_OBJECTIVE_RESOURCE_TOTAL)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_RESEARCH_COMPLETED)
        return get_objective_current_amount(objective, context) >= 1.0;
    if (objective.type == QUEST_OBJECTIVE_FLEET_COUNT)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_TOTAL_SHIP_HP)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_CONVOYS_DELIVERED)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_CONVOY_STREAK)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_CONVOY_RAID_LOSSES_AT_MOST)
        return get_objective_current_amount(objective, context) <= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_MAX_CONVOY_THREAT_AT_MOST)
    {
        double threshold = static_cast<double>(objective.amount) / 100.0;
        return context.maximum_convoy_threat <= threshold;
    }
    if (objective.type == QUEST_OBJECTIVE_BUILDING_COUNT)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_ASSAULT_VICTORIES)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    return false;
}

static const int kActOneQuestIds[] = {
    QUEST_INITIAL_SKIRMISHES,
    QUEST_DEFENSE_OF_TERRA,
    QUEST_INVESTIGATE_RAIDERS,
    QUEST_SECURE_SUPPLY_LINES,
    QUEST_STEADY_SUPPLY_STREAK,
    QUEST_HIGH_VALUE_ESCORT
};

static const int kActTwoQuestIds[] = {
    QUEST_CLIMACTIC_BATTLE,
    QUEST_CRITICAL_DECISION
};

static const int kOrderBranchQuestIds[] = {
    QUEST_ORDER_UPRISING,
    QUEST_ORDER_SUPPRESS_RAIDS,
    QUEST_ORDER_DOMINION,
    QUEST_ORDER_FINAL_VERDICT
};

static const int kRebellionBranchQuestIds[] = {
    QUEST_REBELLION_FLEET,
    QUEST_REBELLION_NETWORK,
    QUEST_REBELLION_LIBERATION,
    QUEST_REBELLION_FINAL_PUSH
};

static void populate_branch_snapshot(const QuestManager &manager,
    Game::ft_story_branch_snapshot &out, int branch_id,
    const char *name, const char *summary,
    const int *quest_ids, size_t quest_count,
    int required_choice, int active_choice)
{
    out.branch_id = branch_id;
    out.name = ft_string(name);
    out.summary = ft_string(summary);
    out.total_quests = static_cast<int>(quest_count);
    out.completed_quests = 0;
    bool any_unlocked = false;
    for (size_t i = 0; i < quest_count; ++i)
    {
        int quest_id = quest_ids[i];
        int status = manager.get_status(quest_id);
        if (status != QUEST_STATUS_LOCKED)
            any_unlocked = true;
        if (status == QUEST_STATUS_COMPLETED)
            out.completed_quests += 1;
    }
    bool choice_selected = (active_choice == required_choice && required_choice != QUEST_CHOICE_NONE);
    out.is_available = any_unlocked || choice_selected;
    out.is_active = choice_selected && out.is_available;
}
} // namespace
