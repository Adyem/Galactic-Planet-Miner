    out.main_quests.clear();
    out.side_quests.clear();
    out.awaiting_choice_ids.clear();
    out.recent_journal_entries.clear();
    out.recent_lore_entries.clear();
    out.critical_choices.clear();
    out.story_acts.clear();
    out.current_act_id = STORY_ACT_RISING_THREAT;
    out.active_main_quest_id = this->get_active_quest();
    out.ui_scale = this->_ui_scale;
    out.lore_panel_anchor = this->_lore_panel_anchor;

    ft_quest_context context;
    this->build_quest_context(context);

    ft_vector<Pair<int, ft_sharedptr<ft_quest_definition> > > definitions;
    this->_quests.snapshot_definitions(definitions);
    size_t definition_count = definitions.size();
