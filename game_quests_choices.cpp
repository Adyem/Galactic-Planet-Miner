void Game::handle_quest_choice_prompt(int quest_id)
{
    if (quest_id == QUEST_CRITICAL_DECISION)
    {
        ft_string entry("Navigator Zara's sacrifice forces a reckoning over Captain Blackthorne's fate.");
        this->append_lore_entry(entry);
        this->unlock_journal_entry(JOURNAL_ENTRY_CLIMACTIC_BATTLE,
            ft_string("Journal – Climactic Battle: Navigator Zara gives everything to shield the fleet, leaving command to decide whether Blackthorne lives or dies."));
    }
    else if (quest_id == QUEST_ORDER_FINAL_VERDICT)
    {
        ft_string entry("Tribunal screens flicker as the Order convenes to determine how rebels will be judged.");
        this->append_lore_entry(entry);
        this->unlock_journal_entry(JOURNAL_ENTRY_ORDER_FINAL_MANDATE,
            ft_string("Journal – Final Order Mandate: Marshal Rhea demands a verdict on the captured rebels, forcing loyalists to choose between fear and reform."));
    }
}

void Game::handle_quest_choice_resolution(int quest_id, int choice_id)
{
    ft_string entry;
    if (quest_id == QUEST_CRITICAL_DECISION)
    {
        if (choice_id == QUEST_CHOICE_EXECUTE_BLACKTHORNE)
        {
            this->ensure_planet_item_slot(PLANET_TERRA, ORE_COAL);
            this->add_ore(PLANET_TERRA, ORE_COAL, 5);
            entry = ft_string("Captain Blackthorne's execution steels Terra's loyalists; coal stockpiles surge for the war effort.");
            this->unlock_journal_entry(JOURNAL_ENTRY_DECISION_EXECUTE_BLACKTHORNE,
                ft_string("Journal – The Critical Decision: Executing Blackthorne satisfies hardliners but risks martyring him for every colonist who ever felt abandoned."));
        }
        else if (choice_id == QUEST_CHOICE_SPARE_BLACKTHORNE)
        {
            this->ensure_planet_item_slot(PLANET_TERRA, ORE_CRYSTAL);
            this->add_ore(PLANET_TERRA, ORE_CRYSTAL, 3);
            entry = ft_string("Sparing Blackthorne yields encoded crystal data that Professor Lumen studies for hidden conspiracies.");
            this->unlock_journal_entry(JOURNAL_ENTRY_DECISION_SPARE_BLACKTHORNE,
                ft_string("Journal – The Critical Decision: Sparing Blackthorne opens the door to expose corruption, even as loyalists whisper that the commander has gone soft."));
        }
    }
    else if (quest_id == QUEST_ORDER_FINAL_VERDICT)
    {
        if (choice_id == QUEST_CHOICE_ORDER_EXECUTE_REBELS)
        {
            ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_MARS);
            if (route != ft_nullptr)
            {
                double reduction = route->threat_level;
                if (reduction > 0.0)
                    this->modify_route_threat(*route, -reduction, false);
                route->quiet_timer += 240.0;
                if (route->quiet_timer > 720.0)
                    route->quiet_timer = 720.0;
            }
            this->ensure_planet_item_slot(PLANET_TERRA, ORE_TITANIUM);
            this->add_ore(PLANET_TERRA, ORE_TITANIUM, 2);
            entry = ft_string("Executions broadcast across the belt; Order patrols lock down lanes as titanium armor is rushed to the garrisons.");
            this->unlock_journal_entry(JOURNAL_ENTRY_ORDER_VERDICT_EXECUTION,
                ft_string("Journal – Ironclad Verdict: The Order purges the rebels, tightening convoys with fear even as whispers of martyrdom ripple through the colonies."));
        }
        else if (choice_id == QUEST_CHOICE_ORDER_TRIAL_REBELS)
        {
            ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
            if (route != ft_nullptr)
                this->modify_route_threat(*route, 0.35, true);
            this->ensure_planet_item_slot(PLANET_TERRA, ORE_CRYSTAL);
            this->add_ore(PLANET_TERRA, ORE_CRYSTAL, 2);
            entry = ft_string("Tribunals promise reform; crystal data archives flow to the public while sympathizers rally cautiously.");
            this->unlock_journal_entry(JOURNAL_ENTRY_ORDER_VERDICT_REFORM,
                ft_string("Journal – Trials for Tomorrow: Public hearings spare the rebels, trading short-term unrest for a fragile hope that the Order can change."));
        }
    }
    if (entry.size() > 0)
        this->append_lore_entry(entry);
    this->record_quest_achievement(quest_id);
}

bool Game::resolve_quest_choice(int quest_id, int choice_id)
{
    if (!this->_quests.make_choice(quest_id, choice_id))
        return false;
    ft_quest_context context;
    this->build_quest_context(context);
    ft_vector<int> quest_completed;
    ft_vector<int> quest_failed;
    ft_vector<int> quest_choices;
    this->_quests.update(0.0, context, quest_completed, quest_failed, quest_choices);
    for (size_t i = 0; i < quest_completed.size(); ++i)
        this->handle_quest_completion(quest_completed[i]);
    for (size_t i = 0; i < quest_failed.size(); ++i)
        this->handle_quest_failure(quest_failed[i]);
    for (size_t i = 0; i < quest_choices.size(); ++i)
        this->handle_quest_choice_prompt(quest_choices[i]);
    this->handle_quest_choice_resolution(quest_id, choice_id);
    return true;
}
