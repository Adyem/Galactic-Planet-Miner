int verify_locked_planet_reward_delivery()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    const int iron_reward = 75;
    const int copper_reward = 30;

    game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    int iron_total = game.add_ore(PLANET_MARS, ITEM_IRON_BAR, iron_reward);
    FT_ASSERT_EQ(iron_reward, iron_total);

    int after_sub = game.sub_ore(PLANET_MARS, ITEM_IRON_BAR, 15);
    FT_ASSERT_EQ(iron_reward - 15, after_sub);

    game.set_ore(PLANET_MARS, ITEM_COPPER_BAR, copper_reward);

    FT_ASSERT_EQ(iron_reward - 15, game.get_ore(PLANET_MARS, ITEM_IRON_BAR));
    FT_ASSERT_EQ(copper_reward, game.get_ore(PLANET_MARS, ITEM_COPPER_BAR));

    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    double elapsed = 0.0;
    while (!game.is_planet_unlocked(PLANET_MARS) && elapsed < 1200.0)
    {
        game.tick(1.0);
        elapsed += 1.0;
    }
    FT_ASSERT(game.is_planet_unlocked(PLANET_MARS));

    FT_ASSERT_EQ(iron_reward - 15, game.get_ore(PLANET_MARS, ITEM_IRON_BAR));
    FT_ASSERT_EQ(copper_reward, game.get_ore(PLANET_MARS, ITEM_COPPER_BAR));

    const ft_vector<Pair<int, double> > &resources = game.get_planet_resources(PLANET_MARS);
    bool iron_slot_found = false;
    bool copper_slot_found = false;
    for (size_t i = 0; i < resources.size(); ++i)
    {
        if (resources[i].key == ITEM_IRON_BAR)
            iron_slot_found = true;
        else if (resources[i].key == ITEM_COPPER_BAR)
            copper_slot_found = true;
    }

    FT_ASSERT(iron_slot_found);
    FT_ASSERT(copper_slot_found);

    return 1;
}

int verify_lore_log_retention()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    const size_t limit = Game::LORE_LOG_MAX_ENTRIES;
    const size_t total_entries = limit + 10;

    for (size_t i = 0; i < total_entries; ++i)
    {
        ft_string entry("Lore entry ");
        entry.append(ft_to_string(i));
        game.append_lore_entry(entry);
    }

    const ft_vector<ft_string> &log = game.get_lore_log();
    FT_ASSERT_EQ(limit, log.size());

    ft_string expected_first("Lore entry ");
    expected_first.append(ft_to_string(total_entries - limit));
    FT_ASSERT_EQ(expected_first, log[0]);

    ft_string expected_last("Lore entry ");
    expected_last.append(ft_to_string(total_entries - 1));
    FT_ASSERT_EQ(expected_last, log[log.size() - 1]);

    return 1;
}

int verify_initial_journal_seed()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    const ft_vector<ft_string> &journal = game.get_journal_entries();
    FT_ASSERT(journal.size() >= 10);

    bool found_miner = false;
    bool found_lumen = false;
    bool found_daisy = false;
    bool found_blackthorne = false;
    bool found_zara = false;
    bool found_finn = false;
    bool found_terra = false;
    bool found_mars = false;
    bool found_zalthor = false;
    bool found_convoy_corps = false;

    for (size_t i = 0; i < journal.size(); ++i)
    {
        const ft_string &entry = journal[i];
        if (!found_miner && ft_strstr(entry.c_str(), "Character Profile – Old Miner Joe") != ft_nullptr)
            found_miner = true;
        if (!found_lumen && ft_strstr(entry.c_str(), "Character Profile – Professor Lumen") != ft_nullptr)
            found_lumen = true;
        if (!found_daisy && ft_strstr(entry.c_str(), "Character Profile – Farmer Daisy") != ft_nullptr)
            found_daisy = true;
        if (!found_blackthorne && ft_strstr(entry.c_str(), "Character Profile – Captain Blackthorne") != ft_nullptr)
            found_blackthorne = true;
        if (!found_zara && ft_strstr(entry.c_str(), "Character Profile – Navigator Zara") != ft_nullptr)
            found_zara = true;
        if (!found_finn && ft_strstr(entry.c_str(), "Character Profile – Old Scout Finn") != ft_nullptr)
            found_finn = true;
        if (!found_terra && ft_strstr(entry.c_str(), "Journal – Rebuilding Terra") != ft_nullptr)
            found_terra = true;
        if (!found_mars && ft_strstr(entry.c_str(), "Journal – Mars Garrison Outposts") != ft_nullptr)
            found_mars = true;
        if (!found_zalthor && ft_strstr(entry.c_str(), "Journal – The Zalthor Anomaly") != ft_nullptr)
            found_zalthor = true;
        if (!found_convoy_corps && ft_strstr(entry.c_str(), "Journal – Convoy Corps Charter") != ft_nullptr)
            found_convoy_corps = true;
    }

    FT_ASSERT(found_miner);
    FT_ASSERT(found_lumen);
    FT_ASSERT(found_daisy);
    FT_ASSERT(found_blackthorne);
    FT_ASSERT(found_zara);
    FT_ASSERT(found_finn);
    FT_ASSERT(found_terra);
    FT_ASSERT(found_mars);
    FT_ASSERT(found_zalthor);
    FT_ASSERT(found_convoy_corps);

    return 1;
}

