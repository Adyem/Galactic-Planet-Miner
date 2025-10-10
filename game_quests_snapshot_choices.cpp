    int critical_decision_status = this->_quests.get_status(QUEST_CRITICAL_DECISION);
    int critical_decision_choice = this->_quests.get_choice(QUEST_CRITICAL_DECISION);
    if (critical_decision_status != QUEST_STATUS_LOCKED || critical_decision_choice != QUEST_CHOICE_NONE)
    {
        ft_story_choice_snapshot decision_snapshot;
        decision_snapshot.quest_id = QUEST_CRITICAL_DECISION;
        decision_snapshot.title = ft_string("Critical Decision: Blackthorne's Fate");
        decision_snapshot.awaiting_selection = (critical_decision_status == QUEST_STATUS_AWAITING_CHOICE);
        decision_snapshot.has_been_made = (critical_decision_choice != QUEST_CHOICE_NONE
            && critical_decision_status == QUEST_STATUS_COMPLETED);

        ft_story_choice_option_snapshot execute_option;
        execute_option.choice_id = QUEST_CHOICE_EXECUTE_BLACKTHORNE;
        execute_option.title = ft_string("Execute Blackthorne");
        execute_option.summary = ft_string("Decisive justice steadies loyalists, yet the execution risks martyring him and rallying the rebellion.");
        execute_option.is_available = (critical_decision_status != QUEST_STATUS_LOCKED);
        execute_option.is_selected = (critical_decision_choice == QUEST_CHOICE_EXECUTE_BLACKTHORNE);
        decision_snapshot.options.push_back(execute_option);

        ft_story_choice_option_snapshot spare_option;
        spare_option.choice_id = QUEST_CHOICE_SPARE_BLACKTHORNE;
        spare_option.title = ft_string("Spare Blackthorne");
        spare_option.summary = ft_string("Sparing him uncovers Dominion corruption but emboldens raiders who frame mercy as weakness.");
        spare_option.is_available = (critical_decision_status != QUEST_STATUS_LOCKED);
        spare_option.is_selected = (critical_decision_choice == QUEST_CHOICE_SPARE_BLACKTHORNE);
        decision_snapshot.options.push_back(spare_option);

        out.critical_choices.push_back(ft_move(decision_snapshot));
    }

    int verdict_status = this->_quests.get_status(QUEST_ORDER_FINAL_VERDICT);
    int verdict_choice = this->_quests.get_choice(QUEST_ORDER_FINAL_VERDICT);
    if (verdict_status != QUEST_STATUS_LOCKED || verdict_choice != QUEST_CHOICE_NONE)
    {
        ft_story_choice_snapshot verdict_snapshot;
        verdict_snapshot.quest_id = QUEST_ORDER_FINAL_VERDICT;
        verdict_snapshot.title = ft_string("Final Mandate for Rebel Prisoners");
        verdict_snapshot.awaiting_selection = (verdict_status == QUEST_STATUS_AWAITING_CHOICE);
        verdict_snapshot.has_been_made = (verdict_choice != QUEST_CHOICE_NONE
            && verdict_status == QUEST_STATUS_COMPLETED);

        ft_story_choice_option_snapshot purge_option;
        purge_option.choice_id = QUEST_CHOICE_ORDER_EXECUTE_REBELS;
        purge_option.title = ft_string("Purge the Rebels");
        purge_option.summary = ft_string("Executing the captives cements Dominion control yet spreads fear through every colony broadcast.");
        purge_option.is_available = (verdict_status != QUEST_STATUS_LOCKED);
        purge_option.is_selected = (verdict_choice == QUEST_CHOICE_ORDER_EXECUTE_REBELS);
        verdict_snapshot.options.push_back(purge_option);

        ft_story_choice_option_snapshot trial_option;
        trial_option.choice_id = QUEST_CHOICE_ORDER_TRIAL_REBELS;
        trial_option.title = ft_string("Stage Public Trials");
        trial_option.summary = ft_string("Public tribunals promise reform even as loyalists fear clemency will fracture the regime's authority.");
        trial_option.is_available = (verdict_status != QUEST_STATUS_LOCKED);
        trial_option.is_selected = (verdict_choice == QUEST_CHOICE_ORDER_TRIAL_REBELS);
        verdict_snapshot.options.push_back(trial_option);

        out.critical_choices.push_back(ft_move(verdict_snapshot));
    }
