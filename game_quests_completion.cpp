void Game::handle_quest_completion(int quest_id)
{
    ft_string entry;
    const ft_quest_definition *definition = this->_quests.get_definition(quest_id);
    bool quick_completion = false;
    double quick_ratio = 0.0;
    if (definition != ft_nullptr)
    {
        ft_quest_completion_info completion_info;
        if (this->_quests.consume_completion_info(quest_id, completion_info)
            && completion_info.timed && completion_info.time_limit > 0.0)
        {
            if (completion_info.time_remaining > completion_info.time_limit)
                completion_info.time_remaining = completion_info.time_limit;
            if (completion_info.time_remaining < 0.0)
                completion_info.time_remaining = 0.0;
            quick_ratio = completion_info.time_remaining / completion_info.time_limit;
            if (quick_ratio > 1.0)
                quick_ratio = 1.0;
            if (quick_ratio < 0.0)
                quick_ratio = 0.0;
            if (quick_ratio >= 0.55)
                quick_completion = true;
        }
    }
    if (quest_id == QUEST_INITIAL_SKIRMISHES)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ITEM_ENGINE_PART);
        this->add_ore(PLANET_TERRA, ITEM_ENGINE_PART, 2);
        entry = ft_string("Old Miner Joe cheers as Terra's convoys return with salvaged engine parts.");
        this->unlock_journal_entry(JOURNAL_ENTRY_INITIAL_RAIDER_SKIRMISHES,
            ft_string("Journal – Initial Raider Skirmishes: Old Miner Joe and Farmer Daisy rally Terra's escorts to shield coreshipments as raids ignite across the belt."));
    }
    else if (quest_id == QUEST_DEFENSE_OF_TERRA)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_COAL);
        this->add_ore(PLANET_TERRA, ORE_COAL, 6);
        entry = ft_string("Professor Lumen catalogs the victory over Terra: new coal shipments fuel the forges.");
        this->unlock_journal_entry(JOURNAL_ENTRY_DEFENSE_OF_TERRA,
            ft_string("Journal – Defense of Terra: Professor Lumen warns of strange cosmic surges even as Terra's defensive wings drive the raiders from the core."));
    }
    else if (quest_id == QUEST_INVESTIGATE_RAIDERS)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_MITHRIL);
        this->add_ore(PLANET_TERRA, ORE_MITHRIL, 4);
        entry = ft_string("Farmer Daisy archives decoded mithril caches that hint at raider supply routes.");
        this->unlock_journal_entry(JOURNAL_ENTRY_INVESTIGATE_RAIDERS,
            ft_string("Journal – Investigate Raider Motives: Intelligence sweeps expose how injustice on the fringe fuels Blackthorne's insurgency and fractures the colonies."));
    }
    else if (quest_id == QUEST_SECURE_SUPPLY_LINES)
    {
        entry = ft_string("Quartermaster Nia reports convoy routes are fortified and raid losses remain contained.");
        this->unlock_journal_entry(JOURNAL_ENTRY_SECURE_SUPPLY_LINES,
            ft_string("Journal – Secure Supply Lines: Logistics crews weave Trade Relays and escorts together so Terra, Mars, and Zalthor can breathe between assaults."));
    }
    else if (quest_id == QUEST_STEADY_SUPPLY_STREAK)
    {
        entry = ft_string("Supply masters toast a flawless convoy streak that reassures nervous settlers.");
        this->unlock_journal_entry(JOURNAL_ENTRY_STEADY_SUPPLY_STREAK,
            ft_string("Journal – Steady Supply Streak: Weeks of uninterrupted deliveries calm the frontier, proving the core still watches over the outlying farms."));
    }
    else if (quest_id == QUEST_HIGH_VALUE_ESCORT)
    {
        entry = ft_string("Escort captains celebrate steering high-value cargos through the gauntlet.");
        this->unlock_journal_entry(JOURNAL_ENTRY_HIGH_VALUE_ESCORT,
            ft_string("Journal – High-Value Escort: Shield cruisers, repair drones, and Sunflare sloops coordinate to keep priceless freight intact under relentless fire."));
    }
    else if (quest_id == QUEST_CLIMACTIC_BATTLE)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_TITANIUM);
        this->add_ore(PLANET_TERRA, ORE_TITANIUM, 3);
        entry = ft_string("Old Scout Finn records the climactic stand: captured titanium plating is repurposed.");
    }
    else if (quest_id == QUEST_ORDER_UPRISING)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ITEM_ENGINE_PART);
        this->add_ore(PLANET_TERRA, ITEM_ENGINE_PART, 3);
        entry = ft_string("Farmer Daisy distributes engine parts to keep loyalist transports operational.");
        this->unlock_journal_entry(JOURNAL_ENTRY_ORDER_UPRISING,
            ft_string("Journal – Order's Last Stand: Marshal Rhea rallies loyalist militias to stamp out uprisings sparked by Blackthorne's execution."));
    }
    else if (quest_id == QUEST_REBELLION_FLEET)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_OBSIDIAN);
        this->add_ore(PLANET_TERRA, ORE_OBSIDIAN, 2);
        entry = ft_string("Professor Lumen preserves obsidian shards from allied rebels as evidence of hope.");
        this->unlock_journal_entry(JOURNAL_ENTRY_REBELLION_FLEET,
            ft_string("Journal – Rebellion Rising: Sparing Blackthorne awakens secret allies who pledge obsidian hulls and shelter to the growing resistance."));
    }
    else if (quest_id == QUEST_ORDER_SUPPRESS_RAIDS)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_MARS);
        if (route != ft_nullptr)
        {
            double reduction = route->threat_level;
            if (reduction > 0.0)
                this->modify_route_threat(*route, -reduction, false);
            route->quiet_timer += 180.0;
            if (route->quiet_timer > 480.0)
                route->quiet_timer = 480.0;
        }
        this->_order_branch_assault_victories = 0;
        this->_order_branch_pending_assault = 0;
        entry = ft_string("Marshal Rhea files triumphant reports as Order sentries hush the raider signal web.");
        this->trigger_branch_assault(PLANET_MARS, 1.15, true);
        this->unlock_journal_entry(JOURNAL_ENTRY_ORDER_SUPPRESS_RAIDS,
            ft_string("Journal – Suppress the Raider Cells: Loyalist patrols choke off raid traffic while Proximity Radars knita surveillance lattice over Mars."));
    }
    else if (quest_id == QUEST_ORDER_DOMINION)
    {
        this->_order_branch_pending_assault = 0;
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_TITANIUM);
        this->add_ore(PLANET_TERRA, ORE_TITANIUM, 4);
        entry = ft_string("Marshal Rhea proclaims Dominion law after Mars bows to loyalist control.");
        this->unlock_journal_entry(JOURNAL_ENTRY_ORDER_DOMINION,
            ft_string("Journal – Order Dominion: Dominion edicts lock down the colonies, trading personal freedoms for the promise of security under the core."));
    }
    else if (quest_id == QUEST_REBELLION_NETWORK)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
        if (route != ft_nullptr)
        {
            if (route->threat_level > 1.5)
                this->modify_route_threat(*route, -1.5, false);
            route->quiet_timer += 120.0;
            if (route->quiet_timer > 420.0)
                route->quiet_timer = 420.0;
        }
        this->_rebellion_branch_assault_victories = 0;
        this->_rebellion_branch_pending_assault = 0;
        entry = ft_string("Captain Blackthorne celebrates the shadow network relays lighting up across the belt.");
        this->trigger_branch_assault(PLANET_ZALTHOR, 1.05, false);
        this->unlock_journal_entry(JOURNAL_ENTRY_REBELLION_NETWORK,
            ft_string("Journal – Shadow Network: Rebel couriers light hidden relays across the fringe, trading intel and hope while convoys dodge Order patrols."));
    }
    else if (quest_id == QUEST_REBELLION_LIBERATION)
    {
        this->_rebellion_branch_pending_assault = 0;
        this->ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ORE_OBSIDIAN);
        this->add_ore(PLANET_NOCTARIS_PRIME, ORE_OBSIDIAN, 5);
        entry = ft_string("Farmer Daisy chronicles rebel banners rising over Zalthor's liberated shipyards.");
        this->unlock_journal_entry(JOURNAL_ENTRY_REBELLION_LIBERATION,
            ft_string("Journal – Liberation of the Frontier: Allied cells secure Zalthor's shipyards, promising a new order forged by those who once lived in the shadows."));
    }
    else if (quest_id == QUEST_REBELLION_FINAL_PUSH)
    {
        this->ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ORE_NANOMATERIAL);
        this->add_ore(PLANET_NOCTARIS_PRIME, ORE_NANOMATERIAL, 2);
        entry = ft_string("Professor Lumen oversees nanomaterial forges primed for the freedom armada's final strike.");
        this->unlock_journal_entry(JOURNAL_ENTRY_REBELLION_FINAL_PUSH,
            ft_string("Journal – Battle for Freedom: Rebel flotillas stockpile nanothread hull patches and liberation pledges before challenging the Dominion's last wall."));
    }
    else if (quest_id == QUEST_SIDE_CONVOY_RESCUE)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_IRON);
        this->add_ore(PLANET_TERRA, ORE_IRON, 6);
        entry = ft_string("Old Miner Joe leads relief crews as convoy escorts haul stranded settlers back under Terra's shield.");
        this->unlock_journal_entry(JOURNAL_ENTRY_SIDE_CONVOY_RESCUE,
            ft_string("Journal – Convoy Rescue Effort: Emergency escorts reroute ore and med-pods to isolated habitats while Terra's beacons promise the belt it hasn't been forgotten."));
    }
    else if (quest_id == QUEST_SIDE_OUTPOST_REPAIR)
    {
        this->ensure_planet_item_slot(PLANET_MARS, ITEM_ENGINE_PART);
        this->add_ore(PLANET_MARS, ITEM_ENGINE_PART, 2);
        entry = ft_string("Professor Lumen certifies that rebuilt redline outposts are online and requesting fresh supply lanes.");
        this->unlock_journal_entry(JOURNAL_ENTRY_SIDE_OUTPOST_REPAIR,
            ft_string("Journal – Outpost Repair Run: Convoy crews deliver engine spares and hull plating to scarred border forts, weaving Mars' defense lattice back into the network."));
    }
    else if (quest_id == QUEST_SIDE_ORDER_PROPAGANDA)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_MARS);
        if (route != ft_nullptr)
        {
            double reduction = 0.75;
            if (route->threat_level < reduction)
                reduction = route->threat_level;
            if (reduction > 0.0)
                this->modify_route_threat(*route, -reduction, false);
            route->quiet_timer += 120.0;
            if (route->quiet_timer > 480.0)
                route->quiet_timer = 480.0;
        }
        entry = ft_string("Marshal Rhea's propaganda blitz steadies loyalist convoys while raider broadcasters fall silent.");
        this->unlock_journal_entry(JOURNAL_ENTRY_SIDE_ORDER_PROPAGANDA,
            ft_string("Journal – Order Broadcast Blitz: Dominion signals flood the relays, promising security as convoys tighten formations and the belt watches under wary eyes."));
    }
    else if (quest_id == QUEST_SIDE_REBELLION_BROADCAST)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
        if (route != ft_nullptr)
        {
            if (route->threat_level > 0.5)
                this->modify_route_threat(*route, -0.5, false);
            route->quiet_timer += 150.0;
            if (route->quiet_timer > 450.0)
                route->quiet_timer = 450.0;
        }
        this->ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ORE_CRYSTAL);
        this->add_ore(PLANET_NOCTARIS_PRIME, ORE_CRYSTAL, 2);
        entry = ft_string("Captain Blackthorne hails encoded rebel broadcasts linking hidden relays without tipping Dominion scouts.");
        this->unlock_journal_entry(JOURNAL_ENTRY_SIDE_REBELLION_BROADCAST,
            ft_string("Journal – Rebel Signal Uprising: Secret transmissions ignite uprisings from mine shafts to orbital farms, each message promising convoys will be the rebellion's lifeline."));
    }
    if (entry.size() > 0)
        this->append_lore_entry(entry);
    if (quick_completion && definition != ft_nullptr)
        this->handle_quick_quest_completion(*definition, quick_ratio);
    this->record_quest_achievement(quest_id);
    this->update_dynamic_quest_pressure();
    ft_string tag("quest_completed_");
    tag.append(ft_to_string(quest_id));
    if (!this->save_campaign_checkpoint(tag))
        return ;
}
