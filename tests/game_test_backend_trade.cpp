int verify_trade_relay_convoy_modifiers()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    auto stock_resource = [&](int planet_id, int resource_id, int amount) {
        game.ensure_planet_item_slot(planet_id, resource_id);
        game.set_ore(planet_id, resource_id, amount);
    };

    stock_resource(PLANET_TERRA, ORE_IRON, 400);
    stock_resource(PLANET_TERRA, ORE_COPPER, 400);
    stock_resource(PLANET_TERRA, ORE_COAL, 400);
    stock_resource(PLANET_TERRA, ORE_MITHRIL, 200);
    stock_resource(PLANET_TERRA, ORE_GOLD, 120);
    stock_resource(PLANET_TERRA, ORE_TITANIUM, 160);
    stock_resource(PLANET_TERRA, ORE_TIN, 160);
    stock_resource(PLANET_TERRA, ORE_SILVER, 160);
    stock_resource(PLANET_TERRA, ITEM_IRON_BAR, 240);
    stock_resource(PLANET_TERRA, ITEM_COPPER_BAR, 240);
    stock_resource(PLANET_TERRA, ITEM_ENGINE_PART, 180);
    stock_resource(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 140);
    stock_resource(PLANET_TERRA, ITEM_TITANIUM_BAR, 140);
    stock_resource(PLANET_TERRA, ITEM_FUSION_REACTOR, 12);
    stock_resource(PLANET_TERRA, ITEM_ACCUMULATOR, 40);

    auto complete_research = [&](int research_id) -> int {
        FT_ASSERT(game.start_research(research_id));
        game.tick(200.0);
        FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(research_id));
        return 1;
    };

    complete_research(RESEARCH_UNLOCK_MARS);
    complete_research(RESEARCH_UNLOCK_ZALTHOR);
    complete_research(RESEARCH_UNLOCK_VULCAN);
    complete_research(RESEARCH_UNLOCK_NOCTARIS);
    complete_research(RESEARCH_SOLAR_PANELS);
    complete_research(RESEARCH_CRAFTING_MASTERY);
    complete_research(RESEARCH_INTERSTELLAR_TRADE);

    stock_resource(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 0);
    stock_resource(PLANET_NOCTARIS_PRIME, ITEM_COPPER_BAR, 60);
    stock_resource(PLANET_NOCTARIS_PRIME, ITEM_ADVANCED_ENGINE_PART, 30);
    stock_resource(PLANET_NOCTARIS_PRIME, ITEM_ACCUMULATOR, 12);

    int noctaris_start = game.get_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);
    stock_resource(PLANET_TERRA, ITEM_IRON_BAR, 220);
    int dispatched = game.transfer_ore(PLANET_TERRA, PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 100);
    FT_ASSERT_EQ(100, dispatched);
    double baseline_elapsed = 0.0;
    while (game.get_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR) == noctaris_start && baseline_elapsed < 600.0)
    {
        game.tick(1.0);
        baseline_elapsed += 1.0;
    }
    FT_ASSERT(baseline_elapsed < 600.0);
    int noctaris_after_baseline = game.get_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);
    int baseline_delivered = noctaris_after_baseline - noctaris_start;
    FT_ASSERT(baseline_delivered > 0);
    FT_ASSERT(baseline_delivered < 100);
    FT_ASSERT_EQ(0, game.get_active_convoy_count());

    stock_resource(PLANET_TERRA, ITEM_COPPER_BAR, 120);
    stock_resource(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 120);
    stock_resource(PLANET_TERRA, ITEM_ACCUMULATOR, 40);

    int terra_relay = game.place_building(PLANET_TERRA, BUILDING_TRADE_RELAY, 2, 0);
    FT_ASSERT(terra_relay != 0);
    int noctaris_relay = game.place_building(PLANET_NOCTARIS_PRIME, BUILDING_TRADE_RELAY, 1, 0);
    FT_ASSERT(noctaris_relay != 0);

    int noctaris_relay_start = game.get_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);
    stock_resource(PLANET_TERRA, ITEM_IRON_BAR, 220);
    int relay_dispatched = game.transfer_ore(PLANET_TERRA, PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 100);
    FT_ASSERT_EQ(100, relay_dispatched);
    double relay_elapsed = 0.0;
    while (game.get_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR) == noctaris_relay_start && relay_elapsed < 600.0)
    {
        game.tick(1.0);
        relay_elapsed += 1.0;
    }
    FT_ASSERT(relay_elapsed < 600.0);
    int noctaris_final = game.get_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);
    int relay_delivered = noctaris_final - noctaris_relay_start;
    FT_ASSERT_EQ(100, relay_delivered);
    FT_ASSERT(relay_elapsed + 0.001 < baseline_elapsed);
    FT_ASSERT_EQ(0, game.get_active_convoy_count());

    return 1;
}

int verify_convoy_escort_travel_speed()
{
    Game baseline(ft_string("127.0.0.1:8080"), ft_string("/"));
    Game escorted(ft_string("127.0.0.1:8080"), ft_string("/"));

    auto prepare = [&](Game &game) -> int {
        FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
        game.tick(200.0);
        FT_ASSERT(game.is_planet_unlocked(PLANET_MARS));
        game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
        game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
        game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
        game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
        return 1;
    };

    auto dispatch_and_measure = [&](Game &game, bool with_escort) -> double {
        game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
        game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
        if (with_escort)
        {
            const int fleet_id = 7801;
            game.create_fleet(fleet_id);
            FT_ASSERT(game.create_ship(fleet_id, SHIP_SHIELD) != 0);
            FT_ASSERT(game.create_ship(fleet_id, SHIP_SHIELD) != 0);
            FT_ASSERT(game.create_ship(fleet_id, SHIP_SHIELD) != 0);
            FT_ASSERT(game.assign_convoy_escort(PLANET_TERRA, PLANET_MARS, fleet_id));
        }
        else
        {
            int assigned = game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS);
            if (assigned != 0)
                FT_ASSERT(game.clear_convoy_escort(PLANET_TERRA, PLANET_MARS));
        }
        int destination_start = game.get_ore(PLANET_MARS, ITEM_IRON_BAR);
        int dispatched = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 100);
        FT_ASSERT_EQ(100, dispatched);
        if (with_escort)
            FT_ASSERT_EQ(0, game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS));
        double elapsed = 0.0;
        while (game.get_ore(PLANET_MARS, ITEM_IRON_BAR) == destination_start && elapsed < 600.0)
        {
            game.tick(0.5);
            elapsed += 0.5;
        }
        FT_ASSERT(elapsed < 600.0);
        int delivered = game.get_ore(PLANET_MARS, ITEM_IRON_BAR) - destination_start;
        FT_ASSERT_EQ(100, delivered);
        double cleanup = elapsed;
        while (game.get_active_convoy_count() > 0 && cleanup < 600.0)
        {
            game.tick(0.5);
            cleanup += 0.5;
        }
        FT_ASSERT_EQ(0, game.get_active_convoy_count());
        return elapsed;
    };

    prepare(baseline);
    double baseline_time = dispatch_and_measure(baseline, false);

    prepare(escorted);
    double escorted_time = dispatch_and_measure(escorted, true);

    FT_ASSERT(escorted_time + 0.5 < baseline_time);

    return 1;
}

