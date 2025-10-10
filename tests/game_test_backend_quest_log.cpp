static void verify_initial_story_state(const Game::ft_quest_log_snapshot &snapshot)
{
    FT_ASSERT_EQ(Game::STORY_ACT_RISING_THREAT, snapshot.current_act_id);
    FT_ASSERT(snapshot.story_acts.size() == 3);
    FT_ASSERT(!snapshot.epilogue.is_available);
    FT_ASSERT(snapshot.critical_choices.size() == 0);

    const Game::ft_story_act_snapshot *act_one
        = find_story_act(snapshot, Game::STORY_ACT_RISING_THREAT);
    const Game::ft_story_act_snapshot *act_two
        = find_story_act(snapshot, Game::STORY_ACT_UNRAVELING_TRUTH);
    const Game::ft_story_act_snapshot *act_three
        = find_story_act(snapshot, Game::STORY_ACT_CROSSROADS);

    FT_ASSERT(act_one != ft_nullptr);
    FT_ASSERT(act_two != ft_nullptr);
    FT_ASSERT(act_three != ft_nullptr);

    FT_ASSERT(act_one->is_active);
    FT_ASSERT(!act_one->is_completed);
    FT_ASSERT_EQ(0, act_one->completed_main_quests);
    FT_ASSERT_EQ(6, act_one->total_main_quests);
    FT_ASSERT(ft_strstr(act_one->theme.c_str(), "Raiders") != ft_nullptr);

    FT_ASSERT(!act_two->is_active);
    FT_ASSERT_EQ(0, act_two->completed_main_quests);
    FT_ASSERT_EQ(2, act_two->total_main_quests);
    FT_ASSERT(!act_two->awaiting_branch_choice);

    FT_ASSERT(!act_three->is_active);
    FT_ASSERT(act_three->branches.size() == 2);

    FT_ASSERT_EQ(QUEST_INITIAL_SKIRMISHES, snapshot.active_main_quest_id);

    const Game::ft_quest_log_entry *initial_entry
        = find_main_quest(snapshot.main_quests, QUEST_INITIAL_SKIRMISHES);
    FT_ASSERT(initial_entry != ft_nullptr);
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, initial_entry->status);
    FT_ASSERT(initial_entry->objectives.size() >= 1);

    bool found_iron_requirement = false;
    for (size_t index = 0; index < initial_entry->objectives.size(); ++index)
    {
        const Game::ft_quest_objective_snapshot &objective = initial_entry->objectives[index];
        if (objective.type == QUEST_OBJECTIVE_RESOURCE_TOTAL && objective.target_id == ORE_IRON)
        {
            found_iron_requirement = true;
            FT_ASSERT(objective.current_amount >= 0.0);
            break;
        }
    }
    FT_ASSERT(found_iron_requirement);
}

static void verify_post_verdict_snapshot(const Game::ft_quest_log_snapshot &snapshot)
{
    const Game::ft_story_act_snapshot *act_one
        = find_story_act(snapshot, Game::STORY_ACT_RISING_THREAT);
    const Game::ft_story_act_snapshot *act_two
        = find_story_act(snapshot, Game::STORY_ACT_UNRAVELING_TRUTH);
    const Game::ft_story_act_snapshot *act_three
        = find_story_act(snapshot, Game::STORY_ACT_CROSSROADS);

    FT_ASSERT_EQ(Game::STORY_ACT_CROSSROADS, snapshot.current_act_id);
    FT_ASSERT(act_one != ft_nullptr);
    FT_ASSERT(act_two != ft_nullptr);
    FT_ASSERT(act_three != ft_nullptr);

    FT_ASSERT(act_one->is_completed);
    FT_ASSERT(act_two->is_completed);
    FT_ASSERT(!act_two->awaiting_branch_choice);
    FT_ASSERT(act_three->is_active);
    FT_ASSERT(!act_three->is_completed);
    FT_ASSERT_EQ(4, act_three->total_main_quests);

    FT_ASSERT(snapshot.critical_choices.size() >= 2);
    const Game::ft_story_choice_snapshot *blackthorne_choice
        = find_story_choice(snapshot.critical_choices, QUEST_CRITICAL_DECISION);
    const Game::ft_story_choice_snapshot *verdict_choice
        = find_story_choice(snapshot.critical_choices, QUEST_ORDER_FINAL_VERDICT);

    FT_ASSERT(blackthorne_choice != ft_nullptr);
    FT_ASSERT(verdict_choice != ft_nullptr);

    verify_blackthorne_choice(*blackthorne_choice);
    verify_verdict_choice(*verdict_choice);
    verify_branch_progress(*act_three);
    verify_epilogue_overview(snapshot);
    verify_main_quest_progress(snapshot);
    verify_recent_logs(snapshot);
}

static void verify_trial_resolution_snapshot(const Game::ft_quest_log_snapshot &snapshot)
{
    verify_epilogue_overview(snapshot);
    FT_ASSERT(contains_entry_with_text(snapshot.epilogue.paragraphs, "reform", "trials"));

    const Game::ft_story_choice_snapshot *final_verdict_record
        = find_story_choice(snapshot.critical_choices, QUEST_ORDER_FINAL_VERDICT);

    FT_ASSERT(final_verdict_record != ft_nullptr);
    FT_ASSERT(final_verdict_record->has_been_made);
    FT_ASSERT(!final_verdict_record->awaiting_selection);

    bool trial_selected = false;
    for (size_t index = 0; index < final_verdict_record->options.size(); ++index)
    {
        const Game::ft_story_choice_option_snapshot &option = final_verdict_record->options[index];
        if (option.choice_id == QUEST_CHOICE_ORDER_TRIAL_REBELS)
            trial_selected = option.is_selected;
    }
    FT_ASSERT(trial_selected);
}

int verify_quest_log_snapshot()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    Game::ft_quest_log_snapshot snapshot;
    game.get_quest_log_snapshot(snapshot);

    verify_initial_story_state(snapshot);

    FT_ASSERT(advance_to_order_final_verdict(game));

    game.get_quest_log_snapshot(snapshot);
    verify_post_verdict_snapshot(snapshot);

    FT_ASSERT(game.resolve_quest_choice(QUEST_ORDER_FINAL_VERDICT, QUEST_CHOICE_ORDER_TRIAL_REBELS));
    game.tick(0.0);

    game.get_quest_log_snapshot(snapshot);
    verify_trial_resolution_snapshot(snapshot);

    return 1;
}
