    order_game.set_ship_hp(11, escort, 100);
    order_game.set_ship_shield(11, escort, 50);
    order_game.set_ship_armor(11, escort, 30);
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_INVESTIGATE_RAIDERS, order_game.get_active_quest());

    order_game.set_ore(PLANET_TERRA, ORE_IRON, 120);
    order_game.set_ore(PLANET_TERRA, ORE_COPPER, 80);
    order_game.set_ore(PLANET_TERRA, ORE_COAL, 24);
    FT_ASSERT(order_game.start_research(RESEARCH_UNLOCK_MARS));
    order_game.tick(40.0);
    order_game.set_ore(PLANET_TERRA, ORE_COAL, 24);
    order_game.set_ore(PLANET_TERRA, ORE_MITHRIL, 12);
    FT_ASSERT(order_game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    order_game.tick(50.0);
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, order_game.get_active_quest());

    order_game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    order_game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
    order_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
    order_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 220);
    for (int shipment = 0; shipment < 8; ++shipment)
    {
        int moved = order_game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 20);
        FT_ASSERT(moved >= 20);
        double waited = 0.0;
        while (order_game.get_active_convoy_count() > 0 && waited < 240.0)
        {
            order_game.tick(4.0);
            waited += 4.0;
        }
        FT_ASSERT(waited < 240.0);
    }
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, order_game.get_quest_status(QUEST_HIGH_VALUE_ESCORT));
    FT_ASSERT_EQ(QUEST_CLIMACTIC_BATTLE, order_game.get_active_quest());

    order_game.set_ore(PLANET_ZALTHOR, ORE_GOLD, 7);
    order_game.set_ore(PLANET_MARS, ORE_MITHRIL, 12);
    FT_ASSERT(order_game.start_research(RESEARCH_UNLOCK_VULCAN));
    order_game.tick(65.0);
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, order_game.get_quest_status(QUEST_CLIMACTIC_BATTLE));
    FT_ASSERT_EQ(QUEST_CRITICAL_DECISION, order_game.get_active_quest());

    FT_ASSERT(order_game.resolve_quest_choice(QUEST_CRITICAL_DECISION, QUEST_CHOICE_EXECUTE_BLACKTHORNE));
    FT_ASSERT_EQ(QUEST_CHOICE_EXECUTE_BLACKTHORNE, order_game.get_quest_choice(QUEST_CRITICAL_DECISION));
    FT_ASSERT_EQ(QUEST_ORDER_UPRISING, order_game.get_active_quest());

    order_game.set_ore(PLANET_TERRA, ORE_COAL, 24);
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, order_game.get_quest_status(QUEST_ORDER_UPRISING));
    FT_ASSERT_EQ(QUEST_ORDER_SUPPRESS_RAIDS, order_game.get_active_quest());

    order_game.ensure_planet_item_slot(PLANET_TERRA, ITEM_COPPER_BAR);
    order_game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
    order_game.ensure_planet_item_slot(PLANET_TERRA, ITEM_MITHRIL_BAR);
    order_game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 30);
    order_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 60);
    order_game.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 20);
    FT_ASSERT(order_game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    order_game.tick(45.0);
    int radar = order_game.place_building(PLANET_TERRA, BUILDING_PROXIMITY_RADAR, 0, 1);
    FT_ASSERT(radar != 0);
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, order_game.get_quest_status(QUEST_ORDER_SUPPRESS_RAIDS));
    FT_ASSERT_EQ(QUEST_ORDER_DOMINION, order_game.get_active_quest());
    FT_ASSERT(order_game.is_assault_active(PLANET_MARS));

    FT_ASSERT(order_game.assign_fleet_to_assault(PLANET_MARS, 10));
    FT_ASSERT(order_game.assign_fleet_to_assault(PLANET_MARS, 11));
    double assault_timer = 0.0;
    while (order_game.is_assault_active(PLANET_MARS) && assault_timer < 200.0)
    {
        order_game.tick(2.0);
        assault_timer += 2.0;
    }
    FT_ASSERT(!order_game.is_assault_active(PLANET_MARS));
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, order_game.get_quest_status(QUEST_ORDER_DOMINION));
    FT_ASSERT_EQ(0, order_game.get_active_quest());

    return 1;
}

int verify_order_final_verdict_outcomes()
{
    Game execution_path(ft_string("127.0.0.1:8080"), ft_string("/"));
    if (!advance_to_order_final_verdict(execution_path))
        return 0;

    FT_ASSERT_EQ(QUEST_STATUS_AWAITING_CHOICE, execution_path.get_quest_status(QUEST_ORDER_FINAL_VERDICT));
    double threat_before_mars = execution_path.get_supply_route_threat_level(PLANET_TERRA, PLANET_MARS);
    size_t lore_before = execution_path.get_lore_log().size();
    int titanium_before = execution_path.get_ore(PLANET_TERRA, ORE_TITANIUM);
    FT_ASSERT(execution_path.resolve_quest_choice(QUEST_ORDER_FINAL_VERDICT, QUEST_CHOICE_ORDER_EXECUTE_REBELS));
    FT_ASSERT(execution_path.is_journal_entry_unlocked(JOURNAL_ENTRY_ORDER_VERDICT_EXECUTION));
    FT_ASSERT(execution_path.get_ore(PLANET_TERRA, ORE_TITANIUM) >= titanium_before + 2);
    double threat_after_mars = execution_path.get_supply_route_threat_level(PLANET_TERRA, PLANET_MARS);
    FT_ASSERT(threat_after_mars < 0.001);
    FT_ASSERT(threat_after_mars <= threat_before_mars + 0.001);
    const ft_vector<ft_string> &execution_lore = execution_path.get_lore_log();
    FT_ASSERT(execution_lore.size() > lore_before);
    FT_ASSERT(lore_log_contains_fragment(execution_lore, lore_before, ft_string("Executions broadcast")));

    Game reform_path(ft_string("127.0.0.1:8080"), ft_string("/"));
    if (!advance_to_order_final_verdict(reform_path))
        return 0;

    double noctaris_before = reform_path.get_supply_route_threat_level(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
    int crystal_before = reform_path.get_ore(PLANET_TERRA, ORE_CRYSTAL);
    size_t reform_lore_start = reform_path.get_lore_log().size();
    FT_ASSERT(reform_path.resolve_quest_choice(QUEST_ORDER_FINAL_VERDICT, QUEST_CHOICE_ORDER_TRIAL_REBELS));
    FT_ASSERT(reform_path.is_journal_entry_unlocked(JOURNAL_ENTRY_ORDER_VERDICT_REFORM));
    double noctaris_after = reform_path.get_supply_route_threat_level(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
    FT_ASSERT(noctaris_after > noctaris_before + 0.2);
    FT_ASSERT(reform_path.get_ore(PLANET_TERRA, ORE_CRYSTAL) >= crystal_before + 2);
    const ft_vector<ft_string> &reform_lore = reform_path.get_lore_log();
    FT_ASSERT(lore_log_contains_fragment(reform_lore, reform_lore_start, ft_string("Tribunals promise reform")));
    return 1;
}

int verify_rebellion_final_push_outcomes()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    if (!advance_to_rebellion_final_push(game))
        return 0;

    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_REBELLION_FINAL_PUSH));
    FT_ASSERT_EQ(QUEST_REBELLION_FINAL_PUSH, game.get_active_quest());

    game.create_fleet(88);
    int flagship = game.create_ship(88, SHIP_CAPITAL);
    FT_ASSERT(flagship != 0);
    game.set_ship_hp(88, flagship, 200);
    game.set_ship_shield(88, flagship, 80);
    game.set_ship_armor(88, flagship, 50);

    game.create_fleet(89);
    int escort = game.create_ship(89, SHIP_SHIELD);
    FT_ASSERT(escort != 0);
    game.set_ship_hp(89, escort, 60);
    game.set_ship_shield(89, escort, 40);
    game.set_ship_armor(89, escort, 20);

    int nanomaterial_before = game.get_ore(PLANET_NOCTARIS_PRIME, ORE_NANOMATERIAL);
    FT_ASSERT(nanomaterial_before < 3);

    stock_resource(game, PLANET_NOCTARIS_PRIME, ORE_NANOMATERIAL, 3);
    size_t lore_before = game.get_lore_log().size();

    game.tick(0.0);

    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_REBELLION_FINAL_PUSH));
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_REBELLION_FINAL_PUSH));
    const ft_vector<ft_string> &lore = game.get_lore_log();
    FT_ASSERT(lore.size() > lore_before);
    FT_ASSERT(lore_log_contains_fragment(lore, lore_before, ft_string("nanomaterial forges")));
    FT_ASSERT(game.get_ore(PLANET_NOCTARIS_PRIME, ORE_NANOMATERIAL) >= 5);
    FT_ASSERT_EQ(0, game.get_active_quest());
    return 1;
}

int verify_side_quest_rotation()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(fast_forward_to_supply_quests(game));

    game.tick(0.0);
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_SIDE_CONVOY_RESCUE));
    FT_ASSERT(game.get_quest_status(QUEST_SIDE_OUTPOST_REPAIR) != QUEST_STATUS_ACTIVE);
    FT_ASSERT(game.get_quest_status(QUEST_SIDE_ORDER_PROPAGANDA) != QUEST_STATUS_ACTIVE);
    FT_ASSERT(game.get_quest_status(QUEST_SIDE_REBELLION_BROADCAST) != QUEST_STATUS_ACTIVE);

    stock_resource(game, PLANET_TERRA, ORE_IRON, 320);
    for (int shipment = 0; shipment < 4; ++shipment)
    {
        FT_ASSERT(dispatch_convoy_to_mars(game, ORE_IRON, 20));
        stock_resource(game, PLANET_TERRA, ORE_IRON, 320);
    }
    size_t rescue_lore_start = game.get_lore_log().size();
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_SIDE_CONVOY_RESCUE));
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_SIDE_CONVOY_RESCUE));
    const ft_vector<ft_string> &rescue_lore = game.get_lore_log();
    FT_ASSERT(lore_log_contains_fragment(rescue_lore, rescue_lore_start, ft_string("relief crews")));

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_SIDE_OUTPOST_REPAIR));
    FT_ASSERT(game.get_quest_status(QUEST_SIDE_ORDER_PROPAGANDA) != QUEST_STATUS_ACTIVE);
    FT_ASSERT(game.get_quest_status(QUEST_SIDE_REBELLION_BROADCAST) != QUEST_STATUS_ACTIVE);

    int terra_radar = game.place_building(PLANET_TERRA, BUILDING_PROXIMITY_RADAR, 2, 0);
    if (terra_radar == 0)
        terra_radar = game.place_building(PLANET_TERRA, BUILDING_PROXIMITY_RADAR, 3, 0);
    int mars_radar = game.place_building(PLANET_MARS, BUILDING_PROXIMITY_RADAR, 0, 0);
    if (mars_radar == 0)
        mars_radar = game.place_building(PLANET_MARS, BUILDING_PROXIMITY_RADAR, 1, 0);
    FT_ASSERT(terra_radar != 0);
    FT_ASSERT(mars_radar != 0);

    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_SIDE_OUTPOST_REPAIR));
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_SIDE_OUTPOST_REPAIR));
    return 1;
}

int evaluate_building_and_convoy_systems(Game &game)
{
    int terra_mine_instance = game.get_building_instance(PLANET_TERRA, 0, 0);
    FT_ASSERT(terra_mine_instance != 0);
    FT_ASSERT(!game.remove_building(PLANET_TERRA, terra_mine_instance));
    FT_ASSERT_EQ(16, game.get_planet_plot_capacity(PLANET_TERRA));
    FT_ASSERT_EQ(1, game.get_planet_plot_usage(PLANET_TERRA));
    FT_ASSERT_EQ(1, game.get_building_count(PLANET_TERRA, BUILDING_MINE_CORE));
    FT_ASSERT(!game.can_place_building(PLANET_TERRA, BUILDING_SMELTER, 0, 0));

    game.set_ore(PLANET_TERRA, ORE_IRON, 200);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    game.set_ore(PLANET_TERRA, ORE_COAL, 200);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 0);
    game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 0);

    FT_ASSERT(!game.can_place_building(PLANET_TERRA, BUILDING_SOLAR_ARRAY, 0, 1));
    FT_ASSERT(game.can_place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0));
    int terra_generator = game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0);
    FT_ASSERT(terra_generator != 0);
    FT_ASSERT(game.place_building(PLANET_TERRA, BUILDING_CONVEYOR, 1, 0) != 0);
    FT_ASSERT(game.place_building(PLANET_TERRA, BUILDING_TRANSFER_NODE, 1, 1) != 0);
    FT_ASSERT(game.place_building(PLANET_TERRA, BUILDING_SMELTER, 0, 2) != 0);
    FT_ASSERT(game.place_building(PLANET_TERRA, BUILDING_PROCESSOR, 2, 2) != 0);

    FT_ASSERT_EQ(4, game.get_planet_logistic_capacity(PLANET_TERRA));
    double terra_energy_gen = game.get_planet_energy_generation(PLANET_TERRA);
    FT_ASSERT(terra_energy_gen > 5.9 && terra_energy_gen < 6.1);
    game.tick(10.0);
    FT_ASSERT(game.get_planet_logistic_usage(PLANET_TERRA) >= 2);
    double terra_energy_use = game.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(terra_energy_use > 4.49 && terra_energy_use < 4.51);
    FT_ASSERT(game.get_ore(PLANET_TERRA, ITEM_IRON_BAR) >= 4);
    FT_ASSERT(game.get_ore(PLANET_TERRA, ITEM_COPPER_BAR) >= 2);

    int conveyor_instance = game.get_building_instance(PLANET_TERRA, 1, 0);
    FT_ASSERT(conveyor_instance != 0);
    FT_ASSERT(game.remove_building(PLANET_TERRA, conveyor_instance));
    FT_ASSERT_EQ(3, game.get_planet_logistic_capacity(PLANET_TERRA));

    game.set_ore(PLANET_VULCAN, ORE_IRON, 150);
