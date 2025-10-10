static const Game::ft_story_act_snapshot *find_story_act(
    const Game::ft_quest_log_snapshot &snapshot, int act_id) noexcept
{
    for (size_t index = 0; index < snapshot.story_acts.size(); ++index)
    {
        if (snapshot.story_acts[index].act_id == act_id)
            return &snapshot.story_acts[index];
    }
    return ft_nullptr;
}

static const Game::ft_story_branch_snapshot *find_story_branch(
    const Game::ft_story_act_snapshot &act, int branch_id) noexcept
{
    for (size_t index = 0; index < act.branches.size(); ++index)
    {
        if (act.branches[index].branch_id == branch_id)
            return &act.branches[index];
    }
    return ft_nullptr;
}

static const Game::ft_story_choice_snapshot *find_story_choice(
    const ft_vector<Game::ft_story_choice_snapshot> &choices, int quest_id) noexcept
{
    for (size_t index = 0; index < choices.size(); ++index)
    {
        if (choices[index].quest_id == quest_id)
            return &choices[index];
    }
    return ft_nullptr;
}

static const Game::ft_quest_log_entry *find_main_quest(
    const ft_vector<Game::ft_quest_log_entry> &entries, int quest_id) noexcept
{
    for (size_t index = 0; index < entries.size(); ++index)
    {
        if (entries[index].quest_id == quest_id)
            return &entries[index];
    }
    return ft_nullptr;
}

static bool contains_entry_with_text(
    const ft_vector<ft_string> &entries, const char *text_a, const char *text_b = ft_nullptr)
{
    for (size_t index = 0; index < entries.size(); ++index)
    {
        const char *paragraph = entries[index].c_str();
        if (ft_strstr(paragraph, text_a) != ft_nullptr)
        {
            if (text_b == ft_nullptr)
                return true;
            if (ft_strstr(paragraph, text_b) != ft_nullptr)
                return true;
        }
    }
    return false;
}

static bool array_contains_choice_id(const ft_vector<int> &ids, int target) noexcept
{
    for (size_t index = 0; index < ids.size(); ++index)
    {
        if (ids[index] == target)
            return true;
    }
    return false;
}

static int verify_blackthorne_choice(const Game::ft_story_choice_snapshot &choice)
{
    FT_ASSERT(choice.has_been_made);
    FT_ASSERT(!choice.awaiting_selection);
    FT_ASSERT(choice.options.size() == 2);

    bool saw_execute_summary = false;
    bool saw_spare_summary = false;
    bool execute_selected = false;
    bool spare_selected = false;

    for (size_t index = 0; index < choice.options.size(); ++index)
    {
        const Game::ft_story_choice_option_snapshot &option = choice.options[index];
        FT_ASSERT(option.is_available);
        if (option.choice_id == QUEST_CHOICE_EXECUTE_BLACKTHORNE)
        {
            execute_selected = option.is_selected;
            if (ft_strstr(option.summary.c_str(), "martyring") != ft_nullptr
                || ft_strstr(option.summary.c_str(), "rallying the rebellion") != ft_nullptr)
                saw_execute_summary = true;
        }
        else if (option.choice_id == QUEST_CHOICE_SPARE_BLACKTHORNE)
        {
            spare_selected = option.is_selected;
            if (ft_strstr(option.summary.c_str(), "corruption") != ft_nullptr
                && ft_strstr(option.summary.c_str(), "weakness") != ft_nullptr)
                saw_spare_summary = true;
        }
    }

    FT_ASSERT(execute_selected);
    FT_ASSERT(!spare_selected);
    FT_ASSERT(saw_execute_summary);
    FT_ASSERT(saw_spare_summary);
    return (1);
}

static int verify_verdict_choice(const Game::ft_story_choice_snapshot &choice)
{
    FT_ASSERT(choice.awaiting_selection);
    FT_ASSERT(!choice.has_been_made);
    FT_ASSERT(choice.options.size() == 2);

    bool saw_fear_summary = false;
    bool saw_reform_summary = false;

    for (size_t index = 0; index < choice.options.size(); ++index)
    {
        const Game::ft_story_choice_option_snapshot &option = choice.options[index];
        FT_ASSERT(option.is_available);
        FT_ASSERT(!option.is_selected);
        if (option.choice_id == QUEST_CHOICE_ORDER_EXECUTE_REBELS)
        {
            if (ft_strstr(option.summary.c_str(), "fear") != ft_nullptr)
                saw_fear_summary = true;
        }
        else if (option.choice_id == QUEST_CHOICE_ORDER_TRIAL_REBELS)
        {
            if (ft_strstr(option.summary.c_str(), "reform") != ft_nullptr)
                saw_reform_summary = true;
        }
    }

    FT_ASSERT(saw_fear_summary);
    FT_ASSERT(saw_reform_summary);
    return (1);
}

static int verify_branch_progress(const Game::ft_story_act_snapshot &act_three)
{
    const Game::ft_story_branch_snapshot *order_branch
        = find_story_branch(act_three, Game::STORY_BRANCH_ORDER_DOMINION);
    const Game::ft_story_branch_snapshot *rebellion_branch
        = find_story_branch(act_three, Game::STORY_BRANCH_REBELLION_LIBERATION);

    FT_ASSERT(order_branch != ft_nullptr);
    FT_ASSERT(rebellion_branch != ft_nullptr);

    FT_ASSERT(order_branch->is_active);
    FT_ASSERT(order_branch->is_available);
    FT_ASSERT(order_branch->completed_quests >= 3);
    FT_ASSERT(order_branch->completed_quests < order_branch->total_quests);
    FT_ASSERT(ft_strstr(order_branch->summary.c_str(), "Dominion") != ft_nullptr);

    FT_ASSERT(!rebellion_branch->is_available);
    FT_ASSERT(!rebellion_branch->is_active);
    return (1);
}

static int verify_main_quest_progress(const Game::ft_quest_log_snapshot &snapshot)
{
    const Game::ft_quest_log_entry *verdict_entry
        = find_main_quest(snapshot.main_quests, QUEST_ORDER_FINAL_VERDICT);
    const Game::ft_quest_log_entry *dominion_entry
        = find_main_quest(snapshot.main_quests, QUEST_ORDER_DOMINION);
    const Game::ft_quest_log_entry *rebellion_entry
        = find_main_quest(snapshot.main_quests, QUEST_REBELLION_FLEET);

    FT_ASSERT(verdict_entry != ft_nullptr);
    FT_ASSERT_EQ(QUEST_STATUS_AWAITING_CHOICE, verdict_entry->status);
    FT_ASSERT(verdict_entry->requires_choice);
    FT_ASSERT(verdict_entry->awaiting_choice);
    FT_ASSERT(verdict_entry->choices.size() == 2);

    bool has_execution_option = false;
    bool has_trial_option = false;
    for (size_t index = 0; index < verdict_entry->choices.size(); ++index)
    {
        const Game::ft_quest_choice_snapshot &choice = verdict_entry->choices[index];
        FT_ASSERT(choice.is_available);
        if (ft_strstr(choice.description.c_str(), "Purge the captured rebels") != ft_nullptr)
            has_execution_option = true;
        if (ft_strstr(choice.description.c_str(), "promise reform") != ft_nullptr)
            has_trial_option = true;
    }
    FT_ASSERT(has_execution_option);
    FT_ASSERT(has_trial_option);
    FT_ASSERT_EQ(QUEST_CHOICE_NONE, verdict_entry->selected_choice);

    FT_ASSERT(array_contains_choice_id(snapshot.awaiting_choice_ids, QUEST_ORDER_FINAL_VERDICT));

    FT_ASSERT(dominion_entry != ft_nullptr);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, dominion_entry->status);
    FT_ASSERT(dominion_entry->objectives_completed);

    FT_ASSERT(rebellion_entry != ft_nullptr);
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, rebellion_entry->status);
    FT_ASSERT(!rebellion_entry->branch_requirement_met);
    return (1);
}

static int verify_recent_logs(const Game::ft_quest_log_snapshot &snapshot)
{
    FT_ASSERT(snapshot.recent_journal_entries.size() >= 1);
    FT_ASSERT(snapshot.recent_journal_entries.size() <= 5);
    FT_ASSERT(contains_entry_with_text(snapshot.recent_journal_entries, "Final Order Mandate"));

    for (size_t index = 0; index < snapshot.main_quests.size(); ++index)
    {
        const Game::ft_quest_log_entry &entry = snapshot.main_quests[index];
        if (entry.time_limit > 0.0)
        {
            FT_ASSERT(entry.time_remaining_ratio >= 0.0);
            FT_ASSERT(entry.time_remaining_ratio <= 1.0);
        }
    }
    for (size_t index = 0; index < snapshot.side_quests.size(); ++index)
    {
        const Game::ft_quest_log_entry &entry = snapshot.side_quests[index];
        if (entry.time_limit > 0.0)
        {
            FT_ASSERT(entry.time_remaining_ratio >= 0.0);
            FT_ASSERT(entry.time_remaining_ratio <= 1.0);
        }
    }

    FT_ASSERT(snapshot.recent_lore_entries.size() <= 5);
    if (!snapshot.recent_lore_entries.empty())
    {
        const ft_string &latest_entry
            = snapshot.recent_lore_entries[snapshot.recent_lore_entries.size() - 1];
        FT_ASSERT(latest_entry.size() > 0);
    }
    return (1);
}

static int verify_epilogue_overview(const Game::ft_quest_log_snapshot &snapshot)
{
    FT_ASSERT(snapshot.epilogue.is_available);
    FT_ASSERT(ft_strstr(snapshot.epilogue.title.c_str(), "Dominion") != ft_nullptr);
    FT_ASSERT(snapshot.epilogue.paragraphs.size() >= 1);
    FT_ASSERT(contains_entry_with_text(snapshot.epilogue.paragraphs, "verdict", "rebels"));
    return (1);
}
