int verify_convoy_escort_assignment_persistence()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    game.tick(200.0);
    FT_ASSERT(game.is_planet_unlocked(PLANET_MARS));
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
    game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);

    const int fleet_id = 9101;
    game.create_fleet(fleet_id);
    FT_ASSERT(game.create_ship(fleet_id, SHIP_SHIELD) != 0);
    FT_ASSERT(game.assign_convoy_escort(PLANET_TERRA, PLANET_MARS, fleet_id));

    int mars_start = game.get_ore(PLANET_MARS, ITEM_IRON_BAR);
    int dispatched = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 50);
    FT_ASSERT_EQ(50, dispatched);
    FT_ASSERT_EQ(0, game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS));

    double elapsed = 0.0;
    while (game.get_ore(PLANET_MARS, ITEM_IRON_BAR) == mars_start && elapsed < 600.0)
    {
        game.tick(1.0);
        elapsed += 1.0;
    }
    FT_ASSERT(elapsed < 600.0);
    double cleanup = elapsed;
    while (game.get_active_convoy_count() > 0 && cleanup < 600.0)
    {
        game.tick(1.0);
        cleanup += 1.0;
    }
    FT_ASSERT_EQ(0, game.get_active_convoy_count());

    FT_ASSERT_EQ(fleet_id, game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS));
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);

    size_t lore_start = game.get_lore_log().size();
    int mars_second_start = game.get_ore(PLANET_MARS, ITEM_IRON_BAR);
    int dispatched_second = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 50);
    FT_ASSERT_EQ(50, dispatched_second);
    FT_ASSERT_EQ(0, game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS));
    const ft_vector<ft_string> &log = game.get_lore_log();
    FT_ASSERT(log.size() > lore_start);
    ft_string escort_fragment("Escort fleet #");
    escort_fragment.append(ft_to_string(fleet_id));
    bool found_escort_entry = false;
    for (size_t i = lore_start; i < log.size(); ++i)
    {
        if (ft_strstr(log[i].c_str(), escort_fragment.c_str()) != ft_nullptr)
        {
            found_escort_entry = true;
            break;
        }
    }
    FT_ASSERT(found_escort_entry);

    double elapsed_second = 0.0;
    bool escort_moved = false;
    while (game.get_ore(PLANET_MARS, ITEM_IRON_BAR) == mars_second_start && elapsed_second < 600.0)
    {
        game.tick(0.5);
        elapsed_second += 0.5;
        if (!escort_moved && elapsed_second >= 1.0)
        {
            game.set_fleet_location_travel(fleet_id, PLANET_MARS, PLANET_VULCAN, 10.0);
            escort_moved = true;
        }
    }
    FT_ASSERT(elapsed_second < 600.0);
    double cleanup_second = elapsed_second;
    while (game.get_active_convoy_count() > 0 && cleanup_second < 600.0)
    {
        game.tick(0.5);
        cleanup_second += 0.5;
    }
    FT_ASSERT_EQ(0, game.get_active_convoy_count());
    FT_ASSERT_EQ(0, game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS));

    return 1;
}

int verify_convoy_escort_rating_excludes_active_escort()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    int baseline_rating = game.calculate_planet_escort_rating(PLANET_TERRA);

    const int fleet_id = 9405;
    game.create_fleet(fleet_id);
    FT_ASSERT(game.create_ship(fleet_id, SHIP_SHIELD) != 0);
    game.set_fleet_location_planet(fleet_id, PLANET_TERRA);

    ft_sharedptr<ft_fleet> escort = game.get_fleet(fleet_id);
    FT_ASSERT(escort);

    int fleet_rating = game.calculate_fleet_escort_rating(*escort);
    FT_ASSERT(fleet_rating > 0);

    int combined_rating = game.calculate_planet_escort_rating(PLANET_TERRA);
    FT_ASSERT_EQ(baseline_rating + fleet_rating, combined_rating);

    Game::ft_supply_route *route = game.ensure_supply_route(PLANET_TERRA, PLANET_MARS);
    FT_ASSERT(route != ft_nullptr);

    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
    game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 50);
    game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);

    int convoy_id = game.dispatch_convoy(*route, PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 20, 0, fleet_id);
    FT_ASSERT(convoy_id > 0);
    FT_ASSERT_EQ(1, game.get_active_convoy_count());

    ft_location traveling = game.get_fleet_location(fleet_id);
    FT_ASSERT_EQ(LOCATION_TRAVEL, traveling.type);
    FT_ASSERT_EQ(PLANET_TERRA, traveling.from);
    FT_ASSERT_EQ(PLANET_MARS, traveling.to);

    int rating_during_escort = game.calculate_planet_escort_rating(PLANET_TERRA);
    FT_ASSERT_EQ(baseline_rating, rating_during_escort);

    game.tick(600.0);

    FT_ASSERT_EQ(0, game.get_active_convoy_count());

    ft_location arrived = game.get_fleet_location(fleet_id);
    FT_ASSERT_EQ(LOCATION_PLANET, arrived.type);
    FT_ASSERT_EQ(PLANET_MARS, arrived.from);
    FT_ASSERT_EQ(PLANET_MARS, arrived.to);

    int rating_after = game.calculate_planet_escort_rating(PLANET_TERRA);
    FT_ASSERT_EQ(baseline_rating, rating_after);

    return 1;
}
