    Game::ft_story_epilogue_snapshot &epilogue = out.epilogue;
    epilogue.is_available = false;
    epilogue.title = ft_string();
    epilogue.paragraphs.clear();
    int critical_choice = this->_quests.get_choice(QUEST_CRITICAL_DECISION);
    bool spared_blackthorne = (critical_choice == QUEST_CHOICE_SPARE_BLACKTHORNE);
    bool executed_blackthorne = (critical_choice == QUEST_CHOICE_EXECUTE_BLACKTHORNE);
    int order_final_status = this->_quests.get_status(QUEST_ORDER_FINAL_VERDICT);
    int rebellion_final_status = this->_quests.get_status(QUEST_REBELLION_FINAL_PUSH);
    int order_dominion_status = this->_quests.get_status(QUEST_ORDER_DOMINION);
    int order_uprising_status = this->_quests.get_status(QUEST_ORDER_UPRISING);
    int rebellion_liberation_status = this->_quests.get_status(QUEST_REBELLION_LIBERATION);
    int rebellion_fleet_status = this->_quests.get_status(QUEST_REBELLION_FLEET);

    if (order_final_status == QUEST_STATUS_COMPLETED)
    {
        epilogue.is_available = true;
        epilogue.title = ft_string("Epilogue: Dominion's Shadow");
        ft_string paragraph("Dominion fleets sweep the trade lanes, restoring stability while fear quietly settles over every habitat.");
        epilogue.paragraphs.push_back(paragraph);
        int final_verdict_choice = this->_quests.get_choice(QUEST_ORDER_FINAL_VERDICT);
        if (final_verdict_choice == QUEST_CHOICE_ORDER_EXECUTE_REBELS)
        {
            ft_string martyr("Captain Blackthorne's execution cements his legacy as a martyr; underground cells whisper his name even as public purges broadcast the price of defiance.");
            epilogue.paragraphs.push_back(martyr);
        }
        else if (final_verdict_choice == QUEST_CHOICE_ORDER_TRIAL_REBELS)
        {
            ft_string reform("Public trials promise reform, yet colonists debate whether contrition can erase the rot that killed Blackthorne and scarred the frontier.");
            epilogue.paragraphs.push_back(reform);
        }
        else
        {
            ft_string warning("Even with Dominion banners raised, the uncertain fate of the rebels reminds citizens that calculated fear still anchors the regime.");
            epilogue.paragraphs.push_back(warning);
        }
    }
    else if (rebellion_final_status == QUEST_STATUS_COMPLETED)
    {
        epilogue.is_available = true;
        epilogue.title = ft_string("Epilogue: Reborn Frontier");
        ft_string struggle("Victory topples the corrupt regime, but liberated worlds inherit shattered shipyards and hungry colonies struggling to rebuild.");
        epilogue.paragraphs.push_back(struggle);
        if (spared_blackthorne)
        {
            ft_string redemption("Blackthorne now works beside the commander, striving to mend the belt even as survivors question whether his redemption can balance the cost of revolt.");
            epilogue.paragraphs.push_back(redemption);
        }
        else
        {
            ft_string legacy("Blackthorne's martyrdom becomes a rallying cry for justice as settlers vow never to repeat the Dominion's oppression.");
            epilogue.paragraphs.push_back(legacy);
        }
    }
    else if (executed_blackthorne &&
        (order_dominion_status == QUEST_STATUS_COMPLETED || order_dominion_status == QUEST_STATUS_ACTIVE ||
         order_uprising_status == QUEST_STATUS_COMPLETED))
    {
        epilogue.is_available = true;
        epilogue.title = ft_string("Epilogue Forecast: Dominion Ascendant");
        ft_string buildup("With Blackthorne dead, Dominion loyalists prepare to decide how ruthlessly they will cement control over the colonies.");
        epilogue.paragraphs.push_back(buildup);
        if (order_final_status == QUEST_STATUS_AWAITING_CHOICE)
        {
            ft_string verdict("The pending verdict over captured rebels will determine whether fear or staged reform defines the coming era.");
            epilogue.paragraphs.push_back(verdict);
        }
    }
    else if (spared_blackthorne &&
        (rebellion_liberation_status == QUEST_STATUS_COMPLETED || rebellion_liberation_status == QUEST_STATUS_ACTIVE ||
         rebellion_fleet_status == QUEST_STATUS_COMPLETED))
    {
        epilogue.is_available = true;
        epilogue.title = ft_string("Epilogue Forecast: Resistance Musters");
        ft_string rally("Sparing Blackthorne ignites a swelling resistance as allied cells assemble fleets for the coming Battle for Freedom.");
        epilogue.paragraphs.push_back(rally);
        if (rebellion_final_status == QUEST_STATUS_ACTIVE || rebellion_final_status == QUEST_STATUS_AVAILABLE)
        {
            ft_string cost("Every convoy of aid highlights how liberation will demand sacrifice long after the decisive strike.");
            epilogue.paragraphs.push_back(cost);
        }
    }
