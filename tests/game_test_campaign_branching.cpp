
    Game::ft_supply_contract contract_details;
    FT_ASSERT(contract_game.get_supply_contract(contract_id, contract_details));
    FT_ASSERT(contract_details.has_minimum_stock);
    FT_ASSERT_EQ(25, contract_details.minimum_stock);
    FT_ASSERT_EQ(30, contract_details.shipment_size);
    FT_ASSERT_EQ(2, contract_details.max_active_convoys);

    contract_game.tick(30.0);
    FT_ASSERT_EQ(0, contract_game.get_active_convoy_count());

    contract_game.tick(20.0);
    FT_ASSERT(contract_game.get_active_convoy_count() >= 1);

    contract_game.tick(120.0);
    FT_ASSERT_EQ(0, contract_game.get_active_convoy_count());
    FT_ASSERT(contract_game.get_ore(PLANET_MARS, ITEM_IRON_BAR) >= 70);

    FT_ASSERT(contract_game.update_supply_contract(contract_id, 18, 30.0, -1, 3));
    FT_ASSERT(contract_game.get_supply_contract(contract_id, contract_details));
    FT_ASSERT(!contract_details.has_minimum_stock);
    FT_ASSERT_EQ(18, contract_details.shipment_size);
    FT_ASSERT_EQ(3, contract_details.max_active_convoys);

    contract_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 5);
    contract_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 12);

    contract_game.tick(30.0);
    FT_ASSERT(contract_game.get_active_convoy_count() >= 1);

    contract_game.tick(120.0);
    FT_ASSERT(contract_game.get_ore(PLANET_MARS, ITEM_IRON_BAR) >= 17);

    contract_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 90);
    contract_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);

    contract_game.tick(90.0);
    FT_ASSERT_EQ(0, contract_game.get_active_convoy_count());
    FT_ASSERT(contract_game.get_ore(PLANET_MARS, ITEM_IRON_BAR) >= 54);

    FT_ASSERT(contract_game.cancel_supply_contract(contract_id));
    contract_game.get_supply_contract_ids(contract_ids);
    bool still_present = false;
    for (size_t idx = 0; idx < contract_ids.size(); ++idx)
    {
        if (contract_ids[idx] == contract_id)
        {
            still_present = true;
            break;
        }
    }
    FT_ASSERT(!still_present);
    FT_ASSERT(!contract_game.get_supply_contract(contract_id, contract_details));

    contract_game.tick(90.0);
    FT_ASSERT_EQ(0, contract_game.get_active_convoy_count());
    FT_ASSERT(!contract_game.cancel_supply_contract(contract_id));
    return 1;
}

int verify_supply_contract_pending_stock_buffer()
{
    Game contract_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    contract_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 180);
    contract_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);

    int contract_id = contract_game.create_supply_contract(PLANET_TERRA, PLANET_MARS,
                                                           ITEM_IRON_BAR, 30, 5.0, 30, 5);
    FT_ASSERT(contract_id > 0);

    contract_game.tick(5.0);
    FT_ASSERT_EQ(1, contract_game.get_active_convoy_count());

    Game::ft_supply_contract details;
    FT_ASSERT(contract_game.get_supply_contract(contract_id, details));
    FT_ASSERT_EQ(30, details.pending_delivery);

    contract_game.tick(5.0);
    FT_ASSERT_EQ(1, contract_game.get_active_convoy_count());
    FT_ASSERT(contract_game.get_supply_contract(contract_id, details));
    FT_ASSERT_EQ(30, details.pending_delivery);

    contract_game.tick(5.0);
    FT_ASSERT_EQ(1, contract_game.get_active_convoy_count());

    contract_game.tick(60.0);
    FT_ASSERT_EQ(0, contract_game.get_active_convoy_count());
    FT_ASSERT(contract_game.get_supply_contract(contract_id, details));
    FT_ASSERT_EQ(0, details.pending_delivery);
    FT_ASSERT(contract_game.get_ore(PLANET_MARS, ITEM_IRON_BAR) >= 30);

    contract_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 5);
    contract_game.tick(5.0);
    FT_ASSERT(contract_game.get_active_convoy_count() >= 1);
    FT_ASSERT(contract_game.get_supply_contract(contract_id, details));
    FT_ASSERT(details.pending_delivery >= 30);

    return 1;
}

int verify_convoy_quest_objectives()
{
    Game success_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(fast_forward_to_supply_quests(success_game));
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, success_game.get_active_quest());

    success_game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    success_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
    success_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 400);
    FT_ASSERT(success_game.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(success_game.start_raider_assault(PLANET_MARS, 1.0));
    int risky = success_game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 40);
    FT_ASSERT(risky >= 40);
    double elapsed = 0.0;
    while (success_game.get_active_convoy_count() > 0 && elapsed < 400.0)
    {
        success_game.tick(5.0);
        elapsed += 5.0;
    }
    FT_ASSERT(elapsed < 400.0);
    while ((success_game.is_assault_active(PLANET_TERRA) || success_game.is_assault_active(PLANET_MARS)) && elapsed < 560.0)
    {
        success_game.tick(5.0);
        elapsed += 5.0;
    }
    FT_ASSERT(success_game.get_convoy_raid_losses() >= 1);
    FT_ASSERT(!success_game.is_assault_active(PLANET_TERRA));
    FT_ASSERT(!success_game.is_assault_active(PLANET_MARS));

    for (int convoy = 0; convoy < 8; ++convoy)
    {
        int moved = success_game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 20);
        FT_ASSERT(moved >= 20);
        double wait = 0.0;
        while (success_game.get_active_convoy_count() > 0 && wait < 240.0)
        {
            success_game.tick(4.0);
            wait += 4.0;
        }
        FT_ASSERT(wait < 240.0);
    }
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, success_game.get_quest_status(QUEST_SECURE_SUPPLY_LINES));
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, success_game.get_quest_status(QUEST_STEADY_SUPPLY_STREAK));
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, success_game.get_quest_status(QUEST_HIGH_VALUE_ESCORT));
    FT_ASSERT_EQ(QUEST_CLIMACTIC_BATTLE, success_game.get_active_quest());
    FT_ASSERT(success_game.get_convoy_raid_losses() >= 1);

    Game failure_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(fast_forward_to_supply_quests(failure_game));
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, failure_game.get_active_quest());
    failure_game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    failure_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
    failure_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 400);
    for (int loss = 0; loss < 2; ++loss)
    {
        FT_ASSERT(failure_game.start_raider_assault(PLANET_TERRA, 1.0));
        FT_ASSERT(failure_game.start_raider_assault(PLANET_MARS, 1.0));
        int moved = failure_game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 30);
        FT_ASSERT(moved >= 30);
        double wait = 0.0;
        while (failure_game.get_active_convoy_count() > 0 && wait < 400.0)
        {
            failure_game.tick(5.0);
            wait += 5.0;
        }
        FT_ASSERT(wait < 400.0);
        while ((failure_game.is_assault_active(PLANET_TERRA) || failure_game.is_assault_active(PLANET_MARS)) && wait < 560.0)
        {
            failure_game.tick(5.0);
            wait += 5.0;
        }
    }
    FT_ASSERT(failure_game.get_convoy_raid_losses() >= 2);
    FT_ASSERT(!failure_game.is_assault_active(PLANET_TERRA));
    FT_ASSERT(!failure_game.is_assault_active(PLANET_MARS));

    for (int convoy = 0; convoy < 3; ++convoy)
    {
        int moved = failure_game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 20);
        FT_ASSERT(moved >= 20);
        double wait = 0.0;
        while (failure_game.get_active_convoy_count() > 0 && wait < 240.0)
        {
            failure_game.tick(4.0);
            wait += 4.0;
        }
        FT_ASSERT(wait < 240.0);
    }
    FT_ASSERT(failure_game.get_total_convoys_delivered() >= 3);
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, failure_game.get_quest_status(QUEST_SECURE_SUPPLY_LINES));
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, failure_game.get_active_quest());
    return 1;
}

int verify_multiple_convoy_raids()
{
    Game convoy_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    convoy_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    convoy_game.ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);
    convoy_game.set_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 0);
    size_t lore_before = convoy_game.get_lore_log().size();
    int transferred = convoy_game.transfer_ore(PLANET_TERRA, PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 120);
    FT_ASSERT(transferred >= 120);
    FT_ASSERT_EQ(1, convoy_game.get_active_convoy_count());

    convoy_game.tick(300.0);

    double threat_after_raids = convoy_game.get_supply_route_threat_level(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
    FT_ASSERT(threat_after_raids > 0.1);

    const ft_vector<ft_string> &log = convoy_game.get_lore_log();
    size_t raid_entries = 0;
    bool destroyed_recorded = false;
    for (size_t idx = lore_before; idx < log.size(); ++idx)
    {
        const char *entry = log[idx].c_str();
        if (ft_strstr(entry, "Raiders ambushed") != ft_nullptr)
            raid_entries += 1;
        if (ft_strstr(entry, "wiped out") != ft_nullptr || ft_strstr(entry, "failed to arrive") != ft_nullptr)
            destroyed_recorded = true;
    }
    FT_ASSERT(raid_entries >= 2 || destroyed_recorded);
    int deliveries_before = convoy_game.get_total_convoys_delivered();
    for (int convoy = 0; convoy < 3; ++convoy)
    {
        int moved_again = convoy_game.transfer_ore(PLANET_TERRA, PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 20);
        FT_ASSERT(moved_again >= 20);
        double wait = 0.0;
        while (convoy_game.get_active_convoy_count() > 0 && wait < 360.0)
        {
            convoy_game.tick(4.0);
            wait += 4.0;
        }
        FT_ASSERT(wait < 360.0);
    }
    FT_ASSERT(convoy_game.get_total_convoys_delivered() > deliveries_before);
    double threat_after_deliveries = convoy_game.get_supply_route_threat_level(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
    FT_ASSERT(threat_after_deliveries < threat_after_raids);
    return 1;
}

int verify_combat_victory_journal_rewards()
{
    Game victory_game(ft_string("127.0.0.1:8080"), ft_string("/"));

    victory_game.set_ore(PLANET_TERRA, ORE_IRON, 30);
    victory_game.set_ore(PLANET_TERRA, ORE_COPPER, 20);
    victory_game.tick(0.0);

    victory_game.create_fleet(90);
