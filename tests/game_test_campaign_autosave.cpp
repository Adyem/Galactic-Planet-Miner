    int capital = victory_game.create_ship(90, SHIP_CAPITAL);
    FT_ASSERT(capital != 0);
    victory_game.set_ship_hp(90, capital, 160);
    victory_game.set_ship_shield(90, capital, 70);
    victory_game.set_ship_armor(90, capital, 40);

    victory_game.create_fleet(91);
    int escort = victory_game.create_ship(91, SHIP_SHIELD);
    FT_ASSERT(escort != 0);
    victory_game.set_ship_hp(91, escort, 90);
    victory_game.set_ship_shield(91, escort, 40);
    victory_game.tick(0.0);

    size_t journal_before = victory_game.get_journal_entries().size();
    size_t lore_before = victory_game.get_lore_log().size();

    ft_string expected_journal_fragments[] = {
        ft_string("Raider Broadcast Intercept"),
        ft_string("Defense Debrief"),
        ft_string("Liberation Signal")
    };
    ft_string expected_lore_fragments[] = {
        ft_string("Sunflare charges"),
        ft_string("Repair drones sealed"),
        ft_string("Zara's sacrifice")
    };

    for (int assault = 0; assault < 3; ++assault)
    {
        FT_ASSERT(victory_game.start_raider_assault(PLANET_TERRA, 1.0));
        FT_ASSERT(victory_game.assign_fleet_to_assault(PLANET_TERRA, 90));
        FT_ASSERT(victory_game.assign_fleet_to_assault(PLANET_TERRA, 91));
        double elapsed = 0.0;
        while (victory_game.is_assault_active(PLANET_TERRA) && elapsed < 240.0)
        {
            victory_game.tick(2.0);
            elapsed += 2.0;
        }
        FT_ASSERT(!victory_game.is_assault_active(PLANET_TERRA));

        const ft_vector<ft_string> &journal_entries = victory_game.get_journal_entries();
        FT_ASSERT_EQ(journal_entries.size(), journal_before + static_cast<size_t>(assault + 1));
        const ft_string &new_entry = journal_entries[journal_before + assault];
        FT_ASSERT(ft_strstr(new_entry.c_str(), expected_journal_fragments[assault].c_str()) != ft_nullptr);

        bool lore_found = false;
        const ft_vector<ft_string> &lore_entries = victory_game.get_lore_log();
        for (size_t i = lore_before; i < lore_entries.size(); ++i)
        {
            const char *line = lore_entries[i].c_str();
            if (ft_strstr(line, expected_lore_fragments[assault].c_str()) != ft_nullptr &&
                ft_strstr(line, "Defense logged at Terra.") != ft_nullptr)
            {
                lore_found = true;
                break;
            }
        }
        FT_ASSERT(lore_found);
    }

    size_t journal_after_three = victory_game.get_journal_entries().size();
    size_t lore_after_three = victory_game.get_lore_log().size();

    FT_ASSERT(victory_game.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(victory_game.assign_fleet_to_assault(PLANET_TERRA, 90));
    FT_ASSERT(victory_game.assign_fleet_to_assault(PLANET_TERRA, 91));
    double elapsed = 0.0;
    while (victory_game.is_assault_active(PLANET_TERRA) && elapsed < 240.0)
    {
        victory_game.tick(2.0);
        elapsed += 2.0;
    }
    FT_ASSERT(!victory_game.is_assault_active(PLANET_TERRA));

    const ft_vector<ft_string> &journal_entries = victory_game.get_journal_entries();
    FT_ASSERT_EQ(journal_entries.size(), journal_after_three);

    int broadcast_count = 0;
    int debrief_count = 0;
    int liberation_count = 0;
    for (size_t i = journal_before; i < journal_entries.size(); ++i)
    {
        const ft_string &entry = journal_entries[i];
        if (ft_strstr(entry.c_str(), expected_journal_fragments[0].c_str()) != ft_nullptr)
            broadcast_count += 1;
        if (ft_strstr(entry.c_str(), expected_journal_fragments[1].c_str()) != ft_nullptr)
            debrief_count += 1;
        if (ft_strstr(entry.c_str(), expected_journal_fragments[2].c_str()) != ft_nullptr)
            liberation_count += 1;
    }
    FT_ASSERT_EQ(broadcast_count, 1);
    FT_ASSERT_EQ(debrief_count, 1);
    FT_ASSERT_EQ(liberation_count, 1);

    const ft_vector<ft_string> &lore_entries = victory_game.get_lore_log();
    int sunflare_count = 0;
    int repair_count = 0;
    int zara_count = 0;
    for (size_t i = lore_before; i < lore_entries.size(); ++i)
    {
        const char *line = lore_entries[i].c_str();
        if (ft_strstr(line, expected_lore_fragments[0].c_str()) != ft_nullptr)
            sunflare_count += 1;
        if (ft_strstr(line, expected_lore_fragments[1].c_str()) != ft_nullptr)
            repair_count += 1;
        if (ft_strstr(line, expected_lore_fragments[2].c_str()) != ft_nullptr)
            zara_count += 1;
    }
    FT_ASSERT_EQ(sunflare_count, 1);
    FT_ASSERT_EQ(repair_count, 1);
    FT_ASSERT_EQ(zara_count, 1);
    FT_ASSERT(lore_entries.size() >= lore_after_three);
    return 1;
}

int verify_supply_route_escalation()
{
    Game escalation_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    escalation_game.ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);
    escalation_game.set_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 0);
    escalation_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 400);

    double threat_level = 0.0;
    int convoy_attempts = 0;
    while (threat_level < 5.0 && convoy_attempts < 12)
    {
        int moved = escalation_game.transfer_ore(PLANET_TERRA, PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 40);
        FT_ASSERT(moved > 0);
        double waited = 0.0;
        while (escalation_game.get_active_convoy_count() > 0 && waited < 480.0)
        {
            escalation_game.tick(6.0);
            waited += 6.0;
        }
        FT_ASSERT(waited < 480.0);
        threat_level = escalation_game.get_supply_route_threat_level(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
        convoy_attempts += 1;
    }
    FT_ASSERT(threat_level >= 5.0);

    size_t lore_before = escalation_game.get_lore_log().size();
    bool assault_started = false;
    double accumulated = 0.0;
    while (!assault_started && accumulated < 240.0)
    {
        escalation_game.tick(6.0);
        accumulated += 6.0;
        if (escalation_game.is_assault_active(PLANET_TERRA) ||
            escalation_game.is_assault_active(PLANET_NOCTARIS_PRIME))
        {
            assault_started = true;
        }
    }
    FT_ASSERT(assault_started);
    bool terra_under_attack = escalation_game.is_assault_active(PLANET_TERRA);
    bool noctaris_under_attack = escalation_game.is_assault_active(PLANET_NOCTARIS_PRIME);
    FT_ASSERT(terra_under_attack || noctaris_under_attack);

    const ft_vector<ft_string> &log = escalation_game.get_lore_log();
    bool escalation_logged = false;
    for (size_t idx = lore_before; idx < log.size(); ++idx)
    {
        const char *entry = log[idx].c_str();
        if (ft_strstr(entry, "convoy route") != ft_nullptr && ft_strstr(entry, "assault") != ft_nullptr)
        {
            escalation_logged = true;
            break;
        }
    }
    FT_ASSERT(escalation_logged);

    if (terra_under_attack)
    {
        escalation_game.tick(12.0);
        FT_ASSERT(escalation_game.is_assault_active(PLANET_TERRA));
    }
    return 1;
}

int verify_late_campaign_raider_focus()
{
    Game raid_focus_game(ft_string("127.0.0.1:8080"), ft_string("/"));

    raid_focus_game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
    raid_focus_game.ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);
    raid_focus_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    raid_focus_game.set_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 0);

    int moved = raid_focus_game.transfer_ore(PLANET_TERRA, PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 40);
    FT_ASSERT(moved >= 40);

    size_t convoy_count = raid_focus_game._active_convoys.size();
    FT_ASSERT(convoy_count > 0);
    Pair<int, Game::ft_supply_convoy> *entries = raid_focus_game._active_convoys.end();
    entries -= convoy_count;
    Game::ft_supply_convoy &convoy = entries[0].value;

    convoy.origin_escort = 0;
    convoy.destination_escort = 0;
    convoy.escort_rating = 0;

    double early_risk = raid_focus_game.calculate_convoy_raid_risk(convoy, false, false);
    FT_ASSERT(early_risk > 0.0);

    raid_focus_game._convoys_delivered_total = 48;
    raid_focus_game._order_branch_assault_victories = 4;
    raid_focus_game._rebellion_branch_assault_victories = 3;

    double late_risk = raid_focus_game.calculate_convoy_raid_risk(convoy, false, false);
    FT_ASSERT(late_risk > early_risk * 1.5);

    convoy.origin_escort = 6;
    convoy.destination_escort = 6;
    double fortified_risk = raid_focus_game.calculate_convoy_raid_risk(convoy, false, false);

    FT_ASSERT(fortified_risk < late_risk);
    FT_ASSERT(fortified_risk <= early_risk * 1.1);

    return 1;
}

int verify_escort_veterancy_progression()
{
    Game veterancy_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(fast_forward_to_supply_quests(veterancy_game));
    while (veterancy_game.get_active_convoy_count() > 0)
    {
        veterancy_game.tick(2.0);
    }
    const int escort_fleet_id = 9301;
    veterancy_game.create_fleet(escort_fleet_id);
    int shield_uid = veterancy_game.create_ship(escort_fleet_id, SHIP_SHIELD);
    int radar_uid = veterancy_game.create_ship(escort_fleet_id, SHIP_RADAR);
    FT_ASSERT(shield_uid != 0);
    FT_ASSERT(radar_uid != 0);
    veterancy_game.set_fleet_location_planet(escort_fleet_id, PLANET_TERRA);

    veterancy_game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
    veterancy_game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    veterancy_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    veterancy_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);

    int assigned = veterancy_game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS);
    if (assigned != 0)
        FT_ASSERT(veterancy_game.clear_convoy_escort(PLANET_TERRA, PLANET_MARS));

    double travel_times[3] = {0.0, 0.0, 0.0};
    int bonuses[3] = {0, 0, 0};
    for (int run = 0; run < 3; ++run)
    {
