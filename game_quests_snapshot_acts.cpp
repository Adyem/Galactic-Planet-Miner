    Game::ft_story_act_snapshot act_one;
    act_one.act_id = STORY_ACT_RISING_THREAT;
    act_one.name = ft_string("Act I: The Rising Threat");
    act_one.theme = ft_string("Raiders strike the colonies while Terra rallies escorts and convoys to shield the core worlds.");
    act_one.total_main_quests = static_cast<int>(sizeof(kActOneQuestIds) / sizeof(kActOneQuestIds[0]));
    act_one.is_completed = (act_one.total_main_quests > 0);
    for (size_t i = 0; i < sizeof(kActOneQuestIds) / sizeof(kActOneQuestIds[0]); ++i)
    {
        int status = this->_quests.get_status(kActOneQuestIds[i]);
        if (status == QUEST_STATUS_COMPLETED)
            act_one.completed_main_quests += 1;
        else
            act_one.is_completed = false;
    }

    Game::ft_story_act_snapshot act_two;
    act_two.act_id = STORY_ACT_UNRAVELING_TRUTH;
    act_two.name = ft_string("Act II: The Unraveling Truth");
    act_two.theme = ft_string("Investigations expose the raiders' motives and force a pivotal judgment on Blackthorne's fate.");
    act_two.total_main_quests = static_cast<int>(sizeof(kActTwoQuestIds) / sizeof(kActTwoQuestIds[0]));
    act_two.is_completed = (act_two.total_main_quests > 0);
    for (size_t i = 0; i < sizeof(kActTwoQuestIds) / sizeof(kActTwoQuestIds[0]); ++i)
    {
        int status = this->_quests.get_status(kActTwoQuestIds[i]);
        if (status == QUEST_STATUS_COMPLETED)
            act_two.completed_main_quests += 1;
        else
            act_two.is_completed = false;
    }
    act_two.awaiting_branch_choice = (this->_quests.get_status(QUEST_CRITICAL_DECISION) == QUEST_STATUS_AWAITING_CHOICE);

    int active_choice = this->_quests.get_choice(QUEST_CRITICAL_DECISION);

    Game::ft_story_act_snapshot act_three;
    act_three.act_id = STORY_ACT_CROSSROADS;
    act_three.name = ft_string("Act III: Crossroads of Order and Rebellion");
    act_three.theme = ft_string("With the system in turmoil, commanders either enforce Dominion rule or join the uprising to reshape the frontier.");

    Game::ft_story_branch_snapshot order_branch;
    populate_branch_snapshot(this->_quests, order_branch,
        STORY_BRANCH_ORDER_DOMINION,
        "Branch A: Dominion Crackdown",
        "Crush uprisings sparked by Blackthorne's execution and cement Order control.",
        kOrderBranchQuestIds, sizeof(kOrderBranchQuestIds) / sizeof(kOrderBranchQuestIds[0]),
        QUEST_CHOICE_EXECUTE_BLACKTHORNE, active_choice);

    Game::ft_story_branch_snapshot rebellion_branch;
    populate_branch_snapshot(this->_quests, rebellion_branch,
        STORY_BRANCH_REBELLION_LIBERATION,
        "Branch B: Liberation Offensive",
        "Rally hidden allies after sparing Blackthorne and liberate the frontier worlds.",
        kRebellionBranchQuestIds, sizeof(kRebellionBranchQuestIds) / sizeof(kRebellionBranchQuestIds[0]),
        QUEST_CHOICE_SPARE_BLACKTHORNE, active_choice);

    bool branch_selected = false;
    if (order_branch.is_active)
    {
        act_three.total_main_quests = order_branch.total_quests;
        act_three.completed_main_quests = order_branch.completed_quests;
        branch_selected = true;
    }
    else if (rebellion_branch.is_active)
    {
        act_three.total_main_quests = rebellion_branch.total_quests;
        act_three.completed_main_quests = rebellion_branch.completed_quests;
        branch_selected = true;
    }
    else if (order_branch.is_available)
    {
        act_three.total_main_quests = order_branch.total_quests;
        act_three.completed_main_quests = order_branch.completed_quests;
    }
    else if (rebellion_branch.is_available)
    {
        act_three.total_main_quests = rebellion_branch.total_quests;
        act_three.completed_main_quests = rebellion_branch.completed_quests;
    }
    else
    {
        act_three.total_main_quests = static_cast<int>(sizeof(kOrderBranchQuestIds) / sizeof(kOrderBranchQuestIds[0]));
        act_three.completed_main_quests = 0;
    }
    if (act_three.total_main_quests > 0 && act_three.completed_main_quests >= act_three.total_main_quests)
        act_three.is_completed = true;

    bool act_one_completed = act_one.is_completed;
    bool act_two_completed = act_two.is_completed;

    out.current_act_id = STORY_ACT_RISING_THREAT;
    if (act_one_completed)
        out.current_act_id = STORY_ACT_UNRAVELING_TRUTH;
    if (act_one_completed && act_two_completed)
        out.current_act_id = STORY_ACT_CROSSROADS;

    act_one.is_active = (out.current_act_id == STORY_ACT_RISING_THREAT);
    act_two.is_active = (out.current_act_id == STORY_ACT_UNRAVELING_TRUTH);
    act_three.is_active = (out.current_act_id == STORY_ACT_CROSSROADS);
    if (!branch_selected)
        act_three.is_completed = false;

    act_three.branches.push_back(ft_move(order_branch));
    act_three.branches.push_back(ft_move(rebellion_branch));

    out.story_acts.push_back(ft_move(act_one));
    out.story_acts.push_back(ft_move(act_two));
    out.story_acts.push_back(ft_move(act_three));
