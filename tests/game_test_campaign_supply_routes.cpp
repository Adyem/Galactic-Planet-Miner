    FT_ASSERT(game.assign_fleet_to_assault(PLANET_ZALTHOR, 20));
    FT_ASSERT(game.assign_fleet_to_assault(PLANET_ZALTHOR, 21));
    double liberation_elapsed = 0.0;
    while (game.is_assault_active(PLANET_ZALTHOR) && liberation_elapsed < 240.0)
    {
        game.tick(2.0);
        liberation_elapsed += 2.0;
    }
    FT_ASSERT(!game.is_assault_active(PLANET_ZALTHOR));

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_REBELLION_LIBERATION));
    FT_ASSERT_EQ(QUEST_REBELLION_FINAL_PUSH, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_REBELLION_FINAL_PUSH));

    game.ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ORE_NANOMATERIAL);
    game.set_ore(PLANET_NOCTARIS_PRIME, ORE_NANOMATERIAL, 0);
    return 1;
}

int validate_initial_campaign_flow(Game &game)
{
    FT_ASSERT_EQ(GAME_DIFFICULTY_STANDARD, game.get_difficulty());
    FT_ASSERT(game.is_planet_unlocked(PLANET_TERRA));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_MARS));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_ZALTHOR));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_VULCAN));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_NOCTARIS_PRIME));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_LUNA));

    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_UNLOCK_MARS));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_UNLOCK_VULCAN));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_UNLOCK_NOCTARIS));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_SOLAR_PANELS));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_CRAFTING_MASTERY));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_UNLOCK_LUNA));
    FT_ASSERT(!game.can_start_research(RESEARCH_UNLOCK_MARS));
    FT_ASSERT(!game.can_start_research(RESEARCH_UNLOCK_ZALTHOR));

    FT_ASSERT_EQ(QUEST_INITIAL_SKIRMISHES, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_INITIAL_SKIRMISHES));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_DEFENSE_OF_TERRA));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_INVESTIGATE_RAIDERS));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_CLIMACTIC_BATTLE));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_CRITICAL_DECISION));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_ORDER_SUPPRESS_RAIDS));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_ORDER_DOMINION));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_REBELLION_NETWORK));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_REBELLION_LIBERATION));
    FT_ASSERT_EQ(QUEST_CHOICE_NONE, game.get_quest_choice(QUEST_CRITICAL_DECISION));
    game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 0);

    int ore = game.add_ore(PLANET_TERRA, ORE_COPPER, 5);
    FT_ASSERT_EQ(5, ore);
    ore = game.sub_ore(PLANET_TERRA, ORE_COPPER, 2);
    FT_ASSERT_EQ(3, ore);
    game.set_ore(PLANET_TERRA, ORE_IRON, 7);
    FT_ASSERT_EQ(7, game.get_ore(PLANET_TERRA, ORE_IRON));

    double rate = game.get_rate(PLANET_TERRA, ORE_IRON);
    FT_ASSERT(rate > 0.49 && rate < 0.51);
    const ft_vector<Pair<int, double> > &terra_res = game.get_planet_resources(PLANET_TERRA);
    FT_ASSERT_EQ(3, static_cast<int>(terra_res.size()));
    FT_ASSERT(game.get_rate(PLANET_MARS, ORE_MITHRIL) < 0.0001);

    game.produce(10.0);
    FT_ASSERT_EQ(12, game.get_ore(PLANET_TERRA, ORE_IRON));
    FT_ASSERT(!game.start_research(RESEARCH_UNLOCK_MARS));

    game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_INITIAL_SKIRMISHES));
    FT_ASSERT_EQ(QUEST_DEFENSE_OF_TERRA, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_DEFENSE_OF_TERRA));
    int parts_after_skirmish = game.get_ore(PLANET_TERRA, ITEM_ENGINE_PART);
    FT_ASSERT(parts_after_skirmish >= 2);
    double defense_timer = game.get_quest_time_remaining(QUEST_DEFENSE_OF_TERRA);
    FT_ASSERT(defense_timer > 179.9 && defense_timer < 180.1);
    size_t lore_before_failure = game.get_lore_log().size();
    game.tick(200.0);
    int parts_after_failure = game.get_ore(PLANET_TERRA, ITEM_ENGINE_PART);
    FT_ASSERT(parts_after_failure < parts_after_skirmish);
    size_t lore_after_failure = game.get_lore_log().size();
    FT_ASSERT(lore_after_failure > lore_before_failure);
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_DEFENSE_OF_TERRA));
    defense_timer = game.get_quest_time_remaining(QUEST_DEFENSE_OF_TERRA);
    FT_ASSERT(defense_timer > 179.9 && defense_timer < 180.1);
    game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    FT_ASSERT(game.can_start_research(RESEARCH_UNLOCK_MARS));
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    FT_ASSERT_EQ(RESEARCH_STATUS_IN_PROGRESS, game.get_research_status(RESEARCH_UNLOCK_MARS));
    double remaining = game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(remaining > 29.9 && remaining < 30.1);
    game.tick(10.0);
    double after = game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(after > 19.9 && after < 20.1);
    game.tick(25.0);
    FT_ASSERT(game.is_planet_unlocked(PLANET_MARS));
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_MARS));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_SOLAR_PANELS));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_CRAFTING_MASTERY));
    double mithril_rate = game.get_rate(PLANET_MARS, ORE_MITHRIL);
    FT_ASSERT(mithril_rate > 0.049 && mithril_rate < 0.051);

    game.set_ore(PLANET_MARS, ORE_IRON, 0);
    game.set_ore(PLANET_MARS, ORE_COPPER, 0);
    game.set_ore(PLANET_MARS, ORE_MITHRIL, 0);
    game.set_ore(PLANET_MARS, ORE_COAL, 0);
    game.produce(40.0);
    FT_ASSERT(game.get_ore(PLANET_MARS, ORE_MITHRIL) >= 2);

    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_UNLOCK_LUNA));
    game.set_ore(PLANET_TERRA, ORE_IRON, 70);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 50);
    game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 12);
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_LUNA));
    game.tick(40.0);
    FT_ASSERT(game.is_planet_unlocked(PLANET_LUNA));
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_LUNA));
    double luna_titanium = game.get_rate(PLANET_LUNA, ORE_TITANIUM);
    FT_ASSERT(luna_titanium > 0.039 && luna_titanium < 0.041);

    game.set_ore(PLANET_TERRA, ORE_COAL, 3);
    game.set_ore(PLANET_MARS, ORE_COAL, 4);
    game.set_ore(PLANET_MARS, ORE_MITHRIL, 9);
    int coal_before = game.get_ore(PLANET_TERRA, ORE_COAL) + game.get_ore(PLANET_MARS, ORE_COAL);
    int mithril_before = game.get_ore(PLANET_TERRA, ORE_MITHRIL) + game.get_ore(PLANET_MARS, ORE_MITHRIL);
    FT_ASSERT(game.can_start_research(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT_EQ(RESEARCH_STATUS_IN_PROGRESS, game.get_research_status(RESEARCH_UNLOCK_ZALTHOR));
    int coal_after = game.get_ore(PLANET_TERRA, ORE_COAL) + game.get_ore(PLANET_MARS, ORE_COAL);
    int mithril_after = game.get_ore(PLANET_TERRA, ORE_MITHRIL) + game.get_ore(PLANET_MARS, ORE_MITHRIL);
    FT_ASSERT_EQ(coal_before - 6, coal_after);
    FT_ASSERT_EQ(mithril_before - 8, mithril_after);
    game.tick(20.0);
    double z_remaining = game.get_research_time_remaining(RESEARCH_UNLOCK_ZALTHOR);
    FT_ASSERT(z_remaining > 19.9 && z_remaining < 20.1);
    game.tick(25.0);
    FT_ASSERT(game.is_planet_unlocked(PLANET_ZALTHOR));
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_UNLOCK_VULCAN));
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_INVESTIGATE_RAIDERS));
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_SECURE_SUPPLY_LINES));
    FT_ASSERT_EQ(0, game.get_total_convoys_delivered());
    FT_ASSERT_EQ(0, game.get_convoy_delivery_streak());
    FT_ASSERT_EQ(0, game.get_convoy_raid_losses());
    game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    for (int shipment = 0; shipment < 8; ++shipment)
    {
        int moved = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 20);
        FT_ASSERT(moved >= 20);
        double waited = 0.0;
        while (game.get_active_convoy_count() > 0 && waited < 240.0)
        {
            game.tick(4.0);
            waited += 4.0;
        }
        FT_ASSERT(waited < 240.0);
    }
    FT_ASSERT(game.get_total_convoys_delivered() >= 8);
    FT_ASSERT(game.get_convoy_delivery_streak() >= 8);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_SECURE_SUPPLY_LINES));
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_STEADY_SUPPLY_STREAK));
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_HIGH_VALUE_ESCORT));
    FT_ASSERT_EQ(QUEST_CLIMACTIC_BATTLE, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_CLIMACTIC_BATTLE));

    game.set_ore(PLANET_ZALTHOR, ORE_GOLD, 7);
    game.set_ore(PLANET_MARS, ORE_MITHRIL, 12);
    int gold_before = game.get_ore(PLANET_ZALTHOR, ORE_GOLD);
    int mithril_before_v = game.get_ore(PLANET_MARS, ORE_MITHRIL);
    FT_ASSERT(game.can_start_research(RESEARCH_UNLOCK_VULCAN));
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_VULCAN));
    FT_ASSERT_EQ(RESEARCH_STATUS_IN_PROGRESS, game.get_research_status(RESEARCH_UNLOCK_VULCAN));
    int gold_after = game.get_ore(PLANET_ZALTHOR, ORE_GOLD);
    int mithril_after_v = game.get_ore(PLANET_MARS, ORE_MITHRIL);
    FT_ASSERT_EQ(gold_before - 6, gold_after);
    FT_ASSERT_EQ(mithril_before_v - 8, mithril_after_v);
    game.tick(30.0);
    double v_remaining = game.get_research_time_remaining(RESEARCH_UNLOCK_VULCAN);
    FT_ASSERT(v_remaining > 24.9 && v_remaining < 25.1);
    game.tick(30.0);
    FT_ASSERT(game.is_planet_unlocked(PLANET_VULCAN));
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_VULCAN));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_UNLOCK_NOCTARIS));
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_CLIMACTIC_BATTLE));
    FT_ASSERT_EQ(QUEST_CRITICAL_DECISION, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_AWAITING_CHOICE, game.get_quest_status(QUEST_CRITICAL_DECISION));
    FT_ASSERT(game.resolve_quest_choice(QUEST_CRITICAL_DECISION, QUEST_CHOICE_SPARE_BLACKTHORNE));
    FT_ASSERT_EQ(QUEST_CHOICE_SPARE_BLACKTHORNE, game.get_quest_choice(QUEST_CRITICAL_DECISION));
    int crystal_reward = game.get_ore(PLANET_TERRA, ORE_CRYSTAL);
    FT_ASSERT(crystal_reward >= 3);
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_ORDER_UPRISING));
    FT_ASSERT_EQ(QUEST_REBELLION_FLEET, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_REBELLION_FLEET));

    game.set_ore(PLANET_VULCAN, ORE_TIN, 9);
    game.set_ore(PLANET_VULCAN, ORE_SILVER, 8);
    game.set_ore(PLANET_VULCAN, ORE_TITANIUM, 7);
    int tin_before = game.get_ore(PLANET_VULCAN, ORE_TIN);
    int silver_before = game.get_ore(PLANET_VULCAN, ORE_SILVER);
    int titanium_before = game.get_ore(PLANET_VULCAN, ORE_TITANIUM);
    FT_ASSERT(game.can_start_research(RESEARCH_UNLOCK_NOCTARIS));
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_NOCTARIS));
    FT_ASSERT_EQ(RESEARCH_STATUS_IN_PROGRESS, game.get_research_status(RESEARCH_UNLOCK_NOCTARIS));
    int tin_after = game.get_ore(PLANET_VULCAN, ORE_TIN);
    int silver_after = game.get_ore(PLANET_VULCAN, ORE_SILVER);
    int titanium_after = game.get_ore(PLANET_VULCAN, ORE_TITANIUM);
    FT_ASSERT_EQ(tin_before - 6, tin_after);
    FT_ASSERT_EQ(silver_before - 6, silver_after);
    FT_ASSERT_EQ(titanium_before - 5, titanium_after);
    game.tick(30.0);
    double n_remaining = game.get_research_time_remaining(RESEARCH_UNLOCK_NOCTARIS);
    FT_ASSERT(n_remaining > 29.9 && n_remaining < 30.1);
    game.tick(35.0);
    FT_ASSERT(game.is_planet_unlocked(PLANET_NOCTARIS_PRIME));
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_NOCTARIS));
    game.set_ore(PLANET_NOCTARIS_PRIME, ORE_OBSIDIAN, 6);
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_CHOICE_SPARE_BLACKTHORNE, game.get_quest_choice(QUEST_CRITICAL_DECISION));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_ORDER_UPRISING));
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_REBELLION_FLEET));
    FT_ASSERT_EQ(QUEST_REBELLION_NETWORK, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_REBELLION_NETWORK));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_ORDER_SUPPRESS_RAIDS));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_ORDER_DOMINION));

    game.create_fleet(98);
    FT_ASSERT_EQ(0, game.create_ship(98, SHIP_CAPITAL));
    game.remove_fleet(98);

    game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 12);
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 12);
    game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 12);
    game.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 6);
    game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 6);
