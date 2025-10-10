static int fast_forward_to_supply_quests(Game &game)
{
    game.set_ore(PLANET_TERRA, ORE_IRON, 20);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 20);
    game.tick(0.0);

    game.create_fleet(1);
    int capital = game.create_ship(1, SHIP_CAPITAL);
    FT_ASSERT(capital != 0);
    game.set_ship_hp(1, capital, 140);
    game.create_fleet(2);
    int escort = game.create_ship(2, SHIP_SHIELD);
    FT_ASSERT(escort != 0);
    game.set_ship_hp(2, escort, 80);
    game.tick(0.0);

    game.set_ore(PLANET_TERRA, ORE_IRON, 120);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 80);
    game.set_ore(PLANET_TERRA, ORE_COAL, 24);
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    game.tick(40.0);
    game.set_ore(PLANET_TERRA, ORE_COAL, 24);
    game.set_ore(PLANET_TERRA, ORE_MITHRIL, 12);
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    game.tick(50.0);
    game.tick(0.0);
    return 1;
}

static void stock_resource(Game &game, int planet_id, int resource_id, int amount)
{
    game.ensure_planet_item_slot(planet_id, resource_id);
    game.set_ore(planet_id, resource_id, amount);
}

static int dispatch_convoy_to_mars(Game &game, int resource_id, int amount)
{
    game.ensure_planet_item_slot(PLANET_TERRA, resource_id);
    int moved = game.transfer_ore(PLANET_TERRA, PLANET_MARS, resource_id, amount);
    FT_ASSERT(moved >= amount);
    double waited = 0.0;
    while (game.get_active_convoy_count() > 0 && waited < 720.0)
    {
        game.tick(2.0);
        waited += 2.0;
    }
    FT_ASSERT(waited < 720.0);
    return 1;
}

static bool lore_log_contains_fragment(const ft_vector<ft_string> &entries,
                                       size_t start_index,
                                       const ft_string &fragment)
{
    const char *needle = fragment.c_str();
    size_t count = entries.size();
    for (size_t i = start_index; i < count; ++i)
    {
        if (ft_strstr(entries[i].c_str(), needle) != ft_nullptr)
            return true;
    }
    return false;
}

int advance_to_order_final_verdict(Game &game)
{
    stock_resource(game, PLANET_TERRA, ORE_IRON, 360);
    stock_resource(game, PLANET_TERRA, ORE_COPPER, 260);
    stock_resource(game, PLANET_TERRA, ORE_COAL, 160);
    stock_resource(game, PLANET_TERRA, ORE_MITHRIL, 160);
    stock_resource(game, PLANET_TERRA, ORE_GOLD, 120);
    stock_resource(game, PLANET_TERRA, ORE_TITANIUM, 40);
    stock_resource(game, PLANET_TERRA, ITEM_ENGINE_PART, 12);

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_DEFENSE_OF_TERRA, game.get_active_quest());

    game.create_fleet(10);
    int capital = game.create_ship(10, SHIP_CAPITAL);
    FT_ASSERT(capital != 0);
    game.set_ship_hp(10, capital, 160);
    game.set_ship_shield(10, capital, 60);
    game.set_ship_armor(10, capital, 40);

    game.create_fleet(11);
    int escort = game.create_ship(11, SHIP_SHIELD);
    FT_ASSERT(escort != 0);
    game.set_ship_hp(11, escort, 100);
    game.set_ship_shield(11, escort, 50);
    game.set_ship_armor(11, escort, 30);

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_DEFENSE_OF_TERRA));

    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    game.tick(40.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_MARS));

    stock_resource(game, PLANET_TERRA, ORE_COAL, 160);
    stock_resource(game, PLANET_TERRA, ORE_MITHRIL, 140);
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    game.tick(50.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_ZALTHOR));

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, game.get_active_quest());

    stock_resource(game, PLANET_TERRA, ORE_IRON, 400);
    for (int shipment = 0; shipment < 6; ++shipment)
    {
        FT_ASSERT(dispatch_convoy_to_mars(game, ORE_IRON, 20));
        stock_resource(game, PLANET_TERRA, ORE_IRON, 400);
    }

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_CLIMACTIC_BATTLE, game.get_active_quest());

    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_VULCAN));
    game.tick(60.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_VULCAN));

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_CRITICAL_DECISION, game.get_active_quest());
    FT_ASSERT(game.resolve_quest_choice(QUEST_CRITICAL_DECISION, QUEST_CHOICE_EXECUTE_BLACKTHORNE));
    game.tick(0.0);

    stock_resource(game, PLANET_TERRA, ORE_COAL, 120);
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_ORDER_UPRISING));

    int radar = game.place_building(PLANET_TERRA, BUILDING_PROXIMITY_RADAR, 0, 0);
    if (radar == 0)
        radar = game.place_building(PLANET_TERRA, BUILDING_PROXIMITY_RADAR, 1, 0);
    FT_ASSERT(radar != 0);

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_ORDER_SUPPRESS_RAIDS));
    FT_ASSERT(game.is_assault_active(PLANET_MARS));

    FT_ASSERT(game.assign_fleet_to_assault(PLANET_MARS, 10));
    FT_ASSERT(game.assign_fleet_to_assault(PLANET_MARS, 11));
    double assault_timer = 0.0;
    while (game.is_assault_active(PLANET_MARS) && assault_timer < 240.0)
    {
        game.tick(2.0);
        assault_timer += 2.0;
    }
    FT_ASSERT(!game.is_assault_active(PLANET_MARS));

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_ORDER_DOMINION));

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_AWAITING_CHOICE, game.get_quest_status(QUEST_ORDER_FINAL_VERDICT));
    return 1;
}

int advance_to_rebellion_final_push(Game &game)
{
    stock_resource(game, PLANET_TERRA, ORE_IRON, 360);
    stock_resource(game, PLANET_TERRA, ORE_COPPER, 260);
    stock_resource(game, PLANET_TERRA, ORE_COAL, 160);
    stock_resource(game, PLANET_TERRA, ORE_MITHRIL, 160);
    stock_resource(game, PLANET_TERRA, ORE_GOLD, 120);
    stock_resource(game, PLANET_TERRA, ORE_TITANIUM, 60);
    stock_resource(game, PLANET_TERRA, ITEM_ENGINE_PART, 12);

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_DEFENSE_OF_TERRA, game.get_active_quest());

    game.create_fleet(20);
    int capital = game.create_ship(20, SHIP_CAPITAL);
    FT_ASSERT(capital != 0);
    game.set_ship_hp(20, capital, 180);
    game.set_ship_shield(20, capital, 70);
    game.set_ship_armor(20, capital, 45);

    game.create_fleet(21);
    int escort = game.create_ship(21, SHIP_SHIELD);
    FT_ASSERT(escort != 0);
    game.set_ship_hp(21, escort, 110);
    game.set_ship_shield(21, escort, 55);
    game.set_ship_armor(21, escort, 35);

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_DEFENSE_OF_TERRA));

    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    game.tick(40.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_MARS));

    stock_resource(game, PLANET_TERRA, ORE_COAL, 140);
    stock_resource(game, PLANET_TERRA, ORE_MITHRIL, 140);
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    game.tick(50.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_ZALTHOR));

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, game.get_active_quest());

    stock_resource(game, PLANET_TERRA, ITEM_IRON_BAR, 260);
    stock_resource(game, PLANET_MARS, ITEM_IRON_BAR, 0);
    for (int shipment = 0; shipment < 6; ++shipment)
    {
        FT_ASSERT(dispatch_convoy_to_mars(game, ITEM_IRON_BAR, 20));
        stock_resource(game, PLANET_TERRA, ITEM_IRON_BAR, 260);
    }

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_CLIMACTIC_BATTLE, game.get_active_quest());

    stock_resource(game, PLANET_ZALTHOR, ORE_GOLD, 12);
    stock_resource(game, PLANET_MARS, ORE_MITHRIL, 20);
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_VULCAN));
    game.tick(65.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_VULCAN));

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_CRITICAL_DECISION, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_AWAITING_CHOICE, game.get_quest_status(QUEST_CRITICAL_DECISION));
    FT_ASSERT(game.resolve_quest_choice(QUEST_CRITICAL_DECISION, QUEST_CHOICE_SPARE_BLACKTHORNE));

    stock_resource(game, PLANET_TERRA, ORE_TIN, 120);
    stock_resource(game, PLANET_TERRA, ORE_SILVER, 120);
    stock_resource(game, PLANET_TERRA, ORE_TITANIUM, 120);
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_NOCTARIS));
    game.tick(70.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_NOCTARIS));

    stock_resource(game, PLANET_TERRA, ORE_OBSIDIAN, 12);
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_REBELLION_FLEET));
    FT_ASSERT_EQ(QUEST_REBELLION_NETWORK, game.get_active_quest());

    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_COPPER_BAR);
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART);
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_ACCUMULATOR);
    game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 80);
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 40);
    game.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 30);
    int relay = game.place_building(PLANET_TERRA, BUILDING_TRADE_RELAY, 2, 0);
    if (relay == 0)
        relay = game.place_building(PLANET_TERRA, BUILDING_TRADE_RELAY, 3, 0);
    FT_ASSERT(relay != 0);

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_REBELLION_NETWORK));
    FT_ASSERT_EQ(QUEST_REBELLION_LIBERATION, game.get_active_quest());
    FT_ASSERT(game.is_assault_active(PLANET_ZALTHOR));

