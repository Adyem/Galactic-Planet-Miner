struct QuestObjectiveEntry
{
    int type;
    int target_id;
    int amount;
};

struct QuestChoiceEntry
{
    int         choice_id;
    const char  *description;
};

struct QuestTemplate
{
    int                         id;
    const char                  *name;
    const char                  *description;
    double                      time_limit;
    bool                        side_quest;
    bool                        requires_choice;
    int                         required_choice_quest;
    int                         required_choice_value;
    const int                   *prerequisites;
    size_t                      prerequisite_count;
    const QuestObjectiveEntry   *objectives;
    size_t                      objective_count;
    const QuestChoiceEntry      *choices;
    size_t                      choice_count;
};

static void copy_ids(const int *data, size_t count, ft_vector<int> &out)
{
    out.clear();
    for (size_t i = 0; i < count; ++i)
        out.push_back(data[i]);
}

static void copy_objectives(const QuestObjectiveEntry *data, size_t count, ft_vector<ft_quest_objective> &out)
{
    out.clear();
    for (size_t i = 0; i < count; ++i)
    {
        ft_quest_objective objective;
        objective.type = data[i].type;
        objective.target_id = data[i].target_id;
        objective.amount = data[i].amount;
        out.push_back(objective);
    }
}

static void copy_choices(const QuestChoiceEntry *data, size_t count, ft_vector<ft_quest_choice_definition> &out)
{
    out.clear();
    for (size_t i = 0; i < count; ++i)
    {
        ft_quest_choice_definition choice;
        choice.choice_id = data[i].choice_id;
        choice.description = ft_string(data[i].description);
        out.push_back(choice);
    }
}

static const QuestObjectiveEntry kSkirmishObjectives[] = {
    {QUEST_OBJECTIVE_RESOURCE_TOTAL, ORE_IRON, 10},
    {QUEST_OBJECTIVE_RESOURCE_TOTAL, ORE_COPPER, 10}
};

static const int kDefensePrereqs[] = {QUEST_INITIAL_SKIRMISHES};
static const QuestObjectiveEntry kDefenseObjectives[] = {
    {QUEST_OBJECTIVE_FLEET_COUNT, 0, 2},
    {QUEST_OBJECTIVE_TOTAL_SHIP_HP, 0, 120}
};

static const int kInvestigatePrereqs[] = {QUEST_DEFENSE_OF_TERRA};
static const QuestObjectiveEntry kInvestigateObjectives[] = {
    {QUEST_OBJECTIVE_RESEARCH_COMPLETED, RESEARCH_UNLOCK_MARS, 1},
    {QUEST_OBJECTIVE_RESEARCH_COMPLETED, RESEARCH_UNLOCK_ZALTHOR, 1}
};

static const int kSecurePrereqs[] = {QUEST_INVESTIGATE_RAIDERS};
static const QuestObjectiveEntry kSecureObjectives[] = {
    {QUEST_OBJECTIVE_CONVOYS_DELIVERED, 0, 3},
    {QUEST_OBJECTIVE_CONVOY_RAID_LOSSES_AT_MOST, 0, 1}
};

static const int kStreakPrereqs[] = {QUEST_SECURE_SUPPLY_LINES};
static const QuestObjectiveEntry kStreakObjectives[] = {
    {QUEST_OBJECTIVE_CONVOY_STREAK, 0, 3}
};

static const int kEscortPrereqs[] = {QUEST_STEADY_SUPPLY_STREAK};
static const QuestObjectiveEntry kEscortObjectives[] = {
    {QUEST_OBJECTIVE_CONVOYS_DELIVERED, 0, 8},
    {QUEST_OBJECTIVE_CONVOY_RAID_LOSSES_AT_MOST, 0, 1}
};

static const int kBattlePrereqs[] = {QUEST_INVESTIGATE_RAIDERS, QUEST_HIGH_VALUE_ESCORT};
static const QuestObjectiveEntry kBattleObjectives[] = {
    {QUEST_OBJECTIVE_RESEARCH_COMPLETED, RESEARCH_UNLOCK_VULCAN, 1},
    {QUEST_OBJECTIVE_TOTAL_SHIP_HP, 0, 180}
};

static const int kDecisionPrereqs[] = {QUEST_CLIMACTIC_BATTLE};
static const QuestChoiceEntry kDecisionChoices[] = {
    {QUEST_CHOICE_EXECUTE_BLACKTHORNE, "Execute Blackthorne to preserve order."},
    {QUEST_CHOICE_SPARE_BLACKTHORNE, "Spare Blackthorne and investigate corruption."}
};

static const int kOrderPrereqs[] = {QUEST_CRITICAL_DECISION};
static const QuestObjectiveEntry kOrderObjectives[] = {
    {QUEST_OBJECTIVE_RESOURCE_TOTAL, ORE_COAL, 20}
};

static const int kRebellionPrereqs[] = {QUEST_CRITICAL_DECISION};
static const QuestObjectiveEntry kRebellionObjectives[] = {
    {QUEST_OBJECTIVE_RESEARCH_COMPLETED, RESEARCH_UNLOCK_NOCTARIS, 1},
    {QUEST_OBJECTIVE_RESOURCE_TOTAL, ORE_OBSIDIAN, 4}
};

static const int kOrderSuppressPrereqs[] = {QUEST_ORDER_UPRISING};
static const QuestObjectiveEntry kOrderSuppressObjectives[] = {
    {QUEST_OBJECTIVE_MAX_CONVOY_THREAT_AT_MOST, 0, 250},
    {QUEST_OBJECTIVE_BUILDING_COUNT, BUILDING_PROXIMITY_RADAR, 1}
};

static const int kOrderDominionPrereqs[] = {QUEST_ORDER_SUPPRESS_RAIDS};
static const QuestObjectiveEntry kOrderDominionObjectives[] = {
    {QUEST_OBJECTIVE_ASSAULT_VICTORIES, PLANET_MARS, 1}
};

static const int kRebellionNetworkPrereqs[] = {QUEST_REBELLION_FLEET};
static const QuestObjectiveEntry kRebellionNetworkObjectives[] = {
    {QUEST_OBJECTIVE_MAX_CONVOY_THREAT_AT_MOST, 0, 300},
    {QUEST_OBJECTIVE_BUILDING_COUNT, BUILDING_TRADE_RELAY, 1}
};

static const int kRebellionLiberationPrereqs[] = {QUEST_REBELLION_NETWORK};
static const QuestObjectiveEntry kRebellionLiberationObjectives[] = {
    {QUEST_OBJECTIVE_ASSAULT_VICTORIES, PLANET_ZALTHOR, 1}
};

static const int kOrderVerdictPrereqs[] = {QUEST_ORDER_DOMINION};
static const QuestChoiceEntry kOrderVerdictChoices[] = {
    {QUEST_CHOICE_ORDER_EXECUTE_REBELS, "Purge the captured rebels to cement the Order."},
    {QUEST_CHOICE_ORDER_TRIAL_REBELS, "Stage a public trial and promise reform."}
};

static const int kRebellionFinalPushPrereqs[] = {QUEST_REBELLION_LIBERATION};
static const QuestObjectiveEntry kRebellionFinalPushObjectives[] = {
    {QUEST_OBJECTIVE_TOTAL_SHIP_HP, 0, 220},
    {QUEST_OBJECTIVE_RESOURCE_TOTAL, ORE_NANOMATERIAL, 3}
};

static const int kSideRescuePrereqs[] = {QUEST_DEFENSE_OF_TERRA};
static const QuestObjectiveEntry kSideRescueObjectives[] = {
    {QUEST_OBJECTIVE_CONVOYS_DELIVERED, 0, 4},
    {QUEST_OBJECTIVE_CONVOY_RAID_LOSSES_AT_MOST, 0, 0}
};

static const int kSideOutpostPrereqs[] = {QUEST_INVESTIGATE_RAIDERS};
static const QuestObjectiveEntry kSideOutpostObjectives[] = {
    {QUEST_OBJECTIVE_BUILDING_COUNT, BUILDING_PROXIMITY_RADAR, 2},
    {QUEST_OBJECTIVE_TOTAL_SHIP_HP, 0, 150}
};

static const int kSideOrderPropPrereqs[] = {QUEST_ORDER_SUPPRESS_RAIDS};
static const QuestObjectiveEntry kSideOrderPropObjectives[] = {
    {QUEST_OBJECTIVE_MAX_CONVOY_THREAT_AT_MOST, 0, 200},
    {QUEST_OBJECTIVE_CONVOYS_DELIVERED, 0, 2}
};

static const int kSideRebellionBroadcastPrereqs[] = {QUEST_REBELLION_NETWORK};
static const QuestObjectiveEntry kSideRebellionBroadcastObjectives[] = {
    {QUEST_OBJECTIVE_BUILDING_COUNT, BUILDING_TRADE_RELAY, 2},
    {QUEST_OBJECTIVE_CONVOY_STREAK, 0, 2}
};

static const QuestTemplate kQuestTemplates[] = {
    {QUEST_INITIAL_SKIRMISHES, "Initial Raider Skirmishes", "Protect supply convoys and fortify Terra's perimeter.", 0.0, false, false, 0, 0, ft_nullptr, 0, kSkirmishObjectives, sizeof(kSkirmishObjectives) / sizeof(kSkirmishObjectives[0]), ft_nullptr, 0},
    {QUEST_DEFENSE_OF_TERRA, "Defense of Terra", "Assemble a defensive wing to repel raider assaults.", 180.0, false, false, 0, 0, kDefensePrereqs, sizeof(kDefensePrereqs) / sizeof(kDefensePrereqs[0]), kDefenseObjectives, sizeof(kDefenseObjectives) / sizeof(kDefenseObjectives[0]), ft_nullptr, 0},
    {QUEST_INVESTIGATE_RAIDERS, "Investigate Raider Motives", "Complete research to uncover the raiders' plans.", 240.0, false, false, 0, 0, kInvestigatePrereqs, sizeof(kInvestigatePrereqs) / sizeof(kInvestigatePrereqs[0]), kInvestigateObjectives, sizeof(kInvestigateObjectives) / sizeof(kInvestigateObjectives[0]), ft_nullptr, 0},
    {QUEST_SECURE_SUPPLY_LINES, "Secure Supply Lines", "Deliver convoys while keeping raid losses contained.", 0.0, false, false, 0, 0, kSecurePrereqs, sizeof(kSecurePrereqs) / sizeof(kSecurePrereqs[0]), kSecureObjectives, sizeof(kSecureObjectives) / sizeof(kSecureObjectives[0]), ft_nullptr, 0},
    {QUEST_STEADY_SUPPLY_STREAK, "Steady Supply Streak", "Maintain an uninterrupted chain of convoy deliveries.", 0.0, false, false, 0, 0, kStreakPrereqs, sizeof(kStreakPrereqs) / sizeof(kStreakPrereqs[0]), kStreakObjectives, sizeof(kStreakObjectives) / sizeof(kStreakObjectives[0]), ft_nullptr, 0},
    {QUEST_HIGH_VALUE_ESCORT, "High-Value Escort", "Escort critical shipments through heightened raids.", 0.0, false, false, 0, 0, kEscortPrereqs, sizeof(kEscortPrereqs) / sizeof(kEscortPrereqs[0]), kEscortObjectives, sizeof(kEscortObjectives) / sizeof(kEscortObjectives[0]), ft_nullptr, 0},
    {QUEST_CLIMACTIC_BATTLE, "Climactic Battle", "Prepare the fleets and technology for the climactic assault.", 300.0, false, false, 0, 0, kBattlePrereqs, sizeof(kBattlePrereqs) / sizeof(kBattlePrereqs[0]), kBattleObjectives, sizeof(kBattleObjectives) / sizeof(kBattleObjectives[0]), ft_nullptr, 0},
    {QUEST_CRITICAL_DECISION, "The Critical Decision", "Decide Blackthorne's fate and set the course for the system.", 0.0, false, true, 0, 0, kDecisionPrereqs, sizeof(kDecisionPrereqs) / sizeof(kDecisionPrereqs[0]), ft_nullptr, 0, kDecisionChoices, sizeof(kDecisionChoices) / sizeof(kDecisionChoices[0])},
    {QUEST_ORDER_UPRISING, "Order's Last Stand", "Crush the uprising that rises after Blackthorne's execution.", 360.0, false, false, QUEST_CRITICAL_DECISION, QUEST_CHOICE_EXECUTE_BLACKTHORNE, kOrderPrereqs, sizeof(kOrderPrereqs) / sizeof(kOrderPrereqs[0]), kOrderObjectives, sizeof(kOrderObjectives) / sizeof(kOrderObjectives[0]), ft_nullptr, 0},
    {QUEST_REBELLION_FLEET, "Rebellion Rising", "Rally new allies after sparing Blackthorne.", 360.0, false, false, QUEST_CRITICAL_DECISION, QUEST_CHOICE_SPARE_BLACKTHORNE, kRebellionPrereqs, sizeof(kRebellionPrereqs) / sizeof(kRebellionPrereqs[0]), kRebellionObjectives, sizeof(kRebellionObjectives) / sizeof(kRebellionObjectives[0]), ft_nullptr, 0},
    {QUEST_ORDER_SUPPRESS_RAIDS, "Suppress the Raider Cells", "Deploy Order loyalists to quiet convoy lanes and establish sensor grids.", 0.0, false, false, QUEST_CRITICAL_DECISION, QUEST_CHOICE_EXECUTE_BLACKTHORNE, kOrderSuppressPrereqs, sizeof(kOrderSuppressPrereqs) / sizeof(kOrderSuppressPrereqs[0]), kOrderSuppressObjectives, sizeof(kOrderSuppressObjectives) / sizeof(kOrderSuppressObjectives[0]), ft_nullptr, 0},
    {QUEST_ORDER_DOMINION, "Order Dominion", "Rally the Order fleets to crush a decisive assault and cement control.", 0.0, false, false, QUEST_CRITICAL_DECISION, QUEST_CHOICE_EXECUTE_BLACKTHORNE, kOrderDominionPrereqs, sizeof(kOrderDominionPrereqs) / sizeof(kOrderDominionPrereqs[0]), kOrderDominionObjectives, sizeof(kOrderDominionObjectives) / sizeof(kOrderDominionObjectives[0]), ft_nullptr, 0},
    {QUEST_REBELLION_NETWORK, "Shadow Network", "Spin up rebel listening posts and keep the raids suppressed long enough to regroup.", 0.0, false, false, QUEST_CRITICAL_DECISION, QUEST_CHOICE_SPARE_BLACKTHORNE, kRebellionNetworkPrereqs, sizeof(kRebellionNetworkPrereqs) / sizeof(kRebellionNetworkPrereqs[0]), kRebellionNetworkObjectives, sizeof(kRebellionNetworkObjectives) / sizeof(kRebellionNetworkObjectives[0]), ft_nullptr, 0},
    {QUEST_REBELLION_LIBERATION, "Liberation of the Frontier", "Lead allied cells through a flagship assault to secure the frontier worlds.", 0.0, false, false, QUEST_CRITICAL_DECISION, QUEST_CHOICE_SPARE_BLACKTHORNE, kRebellionLiberationPrereqs, sizeof(kRebellionLiberationPrereqs) / sizeof(kRebellionLiberationPrereqs[0]), kRebellionLiberationObjectives, sizeof(kRebellionLiberationObjectives) / sizeof(kRebellionLiberationObjectives[0]), ft_nullptr, 0},
    {QUEST_ORDER_FINAL_VERDICT, "Final Order Mandate", "Decide how the Order handles the captured rebels.", 0.0, false, true, QUEST_CRITICAL_DECISION, QUEST_CHOICE_EXECUTE_BLACKTHORNE, kOrderVerdictPrereqs, sizeof(kOrderVerdictPrereqs) / sizeof(kOrderVerdictPrereqs[0]), ft_nullptr, 0, kOrderVerdictChoices, sizeof(kOrderVerdictChoices) / sizeof(kOrderVerdictChoices[0])},
    {QUEST_REBELLION_FINAL_PUSH, "Battle for Freedom", "Gather strength and resources to shatter the Dominion's last fleet.", 0.0, false, false, QUEST_CRITICAL_DECISION, QUEST_CHOICE_SPARE_BLACKTHORNE, kRebellionFinalPushPrereqs, sizeof(kRebellionFinalPushPrereqs) / sizeof(kRebellionFinalPushPrereqs[0]), kRebellionFinalPushObjectives, sizeof(kRebellionFinalPushObjectives) / sizeof(kRebellionFinalPushObjectives[0]), ft_nullptr, 0},
    {QUEST_SIDE_CONVOY_RESCUE, "Convoy Rescue Effort", "Rush aid to stranded settlers while keeping raids suppressed.", 210.0, true, false, 0, 0, kSideRescuePrereqs, sizeof(kSideRescuePrereqs) / sizeof(kSideRescuePrereqs[0]), kSideRescueObjectives, sizeof(kSideRescueObjectives) / sizeof(kSideRescueObjectives[0]), ft_nullptr, 0},
    {QUEST_SIDE_OUTPOST_REPAIR, "Outpost Repair Run", "Deliver supplies and ships to reinforce remote garrisons.", 240.0, true, false, 0, 0, kSideOutpostPrereqs, sizeof(kSideOutpostPrereqs) / sizeof(kSideOutpostPrereqs[0]), kSideOutpostObjectives, sizeof(kSideOutpostObjectives) / sizeof(kSideOutpostObjectives[0]), ft_nullptr, 0},
    {QUEST_SIDE_ORDER_PROPAGANDA, "Order Broadcast Blitz", "Project loyalist strength across the belt through tight security convoys.", 240.0, true, false, 0, 0, kSideOrderPropPrereqs, sizeof(kSideOrderPropPrereqs) / sizeof(kSideOrderPropPrereqs[0]), kSideOrderPropObjectives, sizeof(kSideOrderPropObjectives) / sizeof(kSideOrderPropObjectives[0]), ft_nullptr, 0},
    {QUEST_SIDE_REBELLION_BROADCAST, "Rebel Signal Uprising", "Coordinate underground relays to rally sympathizers without losing supply momentum.", 240.0, true, false, 0, 0, kSideRebellionBroadcastPrereqs, sizeof(kSideRebellionBroadcastPrereqs) / sizeof(kSideRebellionBroadcastPrereqs[0]), kSideRebellionBroadcastObjectives, sizeof(kSideRebellionBroadcastObjectives) / sizeof(kSideRebellionBroadcastObjectives[0]), ft_nullptr, 0}
};

static void collect_quest_definitions(ft_vector<ft_sharedptr<ft_quest_definition> > &out)
{
    out.clear();
    size_t template_count = sizeof(kQuestTemplates) / sizeof(kQuestTemplates[0]);
    for (size_t i = 0; i < template_count; ++i)
    {
        const QuestTemplate &entry = kQuestTemplates[i];
        ft_sharedptr<ft_quest_definition> definition(new ft_quest_definition());
        definition->id = entry.id;
        definition->name = ft_string(entry.name);
        definition->description = ft_string(entry.description);
        definition->time_limit = entry.time_limit;
        definition->is_side_quest = entry.side_quest;
        definition->requires_choice = entry.requires_choice;
        definition->required_choice_quest = entry.required_choice_quest;
        definition->required_choice_value = entry.required_choice_value;
        copy_ids(entry.prerequisites, entry.prerequisite_count, definition->prerequisites);
        copy_objectives(entry.objectives, entry.objective_count, definition->objectives);
        copy_choices(entry.choices, entry.choice_count, definition->choices);
        out.push_back(definition);
    }
}
