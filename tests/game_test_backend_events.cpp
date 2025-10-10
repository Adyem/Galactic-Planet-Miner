int verify_celestial_barrens_salvage_event()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    game.unlock_planet(PLANET_MARS);
    game.unlock_planet(PLANET_ZALTHOR);
    game.ensure_planet_item_slot(PLANET_ZALTHOR, ORE_CRYSTAL);
    game.ensure_planet_item_slot(PLANET_ZALTHOR, ORE_GOLD);
    game.ensure_planet_item_slot(PLANET_ZALTHOR, ORE_TRITIUM);
    game.set_ore(PLANET_ZALTHOR, ORE_CRYSTAL, 0);
    game.set_ore(PLANET_ZALTHOR, ORE_GOLD, 0);
    game.set_ore(PLANET_ZALTHOR, ORE_TRITIUM, 0);

    Game::ft_supply_route *route = game.ensure_supply_route(PLANET_MARS, PLANET_ZALTHOR);
    FT_ASSERT(route != ft_nullptr);
    route->threat_level = 5.0;
    route->quiet_timer = 120.0;

    Game::ft_supply_convoy convoy;
    convoy.route_id = route->id;
    convoy.origin_planet_id = PLANET_MARS;
    convoy.destination_planet_id = PLANET_ZALTHOR;
    convoy.escort_rating = 2;

    size_t lore_before = game.get_lore_log().size();
    int crystal_before = game.get_ore(PLANET_ZALTHOR, ORE_CRYSTAL);
    double threat_before = route->threat_level;

    ft_seed_random_engine(97531u);
    FT_ASSERT(game.handle_celestial_barrens_salvage(convoy));

    int crystal_after = game.get_ore(PLANET_ZALTHOR, ORE_CRYSTAL);
    FT_ASSERT_EQ(crystal_before + 4, crystal_after);
    double threat_after = route->threat_level;
    FT_ASSERT(threat_after > threat_before);
    FT_ASSERT(route->quiet_timer >= 0.0 && route->quiet_timer < 0.0001);

    const ft_vector<ft_string> &log = game.get_lore_log();
    FT_ASSERT(log.size() >= lore_before + 2);
    bool found_salvage_entry = false;
    bool found_resource_snippet = false;
    for (size_t i = lore_before; i < log.size(); ++i)
    {
        const ft_string &entry = log[i];
        if (!found_salvage_entry && ft_strstr(entry.c_str(), "Celestial Barrens") != ft_nullptr)
            found_salvage_entry = true;
        if (!found_resource_snippet && ft_strstr(entry.c_str(), "crystal") != ft_nullptr)
            found_resource_snippet = true;
    }
    FT_ASSERT(found_salvage_entry);
    FT_ASSERT(found_resource_snippet);
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_CELESTIAL_BARRENS));
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_IMPERIUM_PRESSURE));
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_RESOURCE_CRYSTAL_INTRIGUE));

    return 1;
}

int verify_imperium_pressure_threshold()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    game.unlock_planet(PLANET_MARS);
    game.unlock_planet(PLANET_ZALTHOR);

    Game::ft_supply_route *route = game.ensure_supply_route(PLANET_MARS, PLANET_ZALTHOR);
    FT_ASSERT(route != ft_nullptr);

    const ft_vector<ft_string> &initial_journal = game.get_journal_entries();
    size_t initial_count = initial_journal.size();

    route->threat_level = 2.5;
    game.maybe_unlock_imperium_pressure(*route);
    FT_ASSERT(!game.is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_IMPERIUM_PRESSURE));
    FT_ASSERT_EQ(initial_count, game.get_journal_entries().size());

    route->threat_level = 3.4;
    game.maybe_unlock_imperium_pressure(*route);
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_IMPERIUM_PRESSURE));

    const ft_vector<ft_string> &after_unlock = game.get_journal_entries();
    FT_ASSERT(after_unlock.size() == initial_count + 1);
    bool found_entry = false;
    for (size_t i = 0; i < after_unlock.size(); ++i)
    {
        if (ft_strstr(after_unlock[i].c_str(), "Imperium Pressure") != ft_nullptr)
        {
            found_entry = true;
            break;
        }
    }
    FT_ASSERT(found_entry);

    size_t post_unlock_count = after_unlock.size();
    route->threat_level = 9.0;
    game.maybe_unlock_imperium_pressure(*route);
    FT_ASSERT_EQ(post_unlock_count, game.get_journal_entries().size());

    return 1;
}

int verify_nebula_outpost_scan_event()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    game.unlock_planet(PLANET_NOCTARIS_PRIME);
    game.ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);

    Game::ft_supply_route *route = game.ensure_supply_route(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
    FT_ASSERT(route != ft_nullptr);
    route->threat_level = 4.2;
    route->quiet_timer = 90.0;

    int contract_id = game.create_supply_contract(PLANET_TERRA, PLANET_NOCTARIS_PRIME,
        ITEM_IRON_BAR, 30, 100.0, -1, 2);
    FT_ASSERT(contract_id > 0);
    Pair<int, Game::ft_supply_contract> *contract_entry = game._supply_contracts.find(contract_id);
    FT_ASSERT(contract_entry != ft_nullptr);
    contract_entry->value.elapsed_seconds = 0.0;

    Game::ft_supply_convoy convoy;
    convoy.route_id = route->id;
    convoy.contract_id = contract_id;
    convoy.origin_planet_id = PLANET_TERRA;
    convoy.destination_planet_id = PLANET_NOCTARIS_PRIME;
    convoy.escort_rating = 1;

    size_t lore_before = game.get_lore_log().size();
    double threat_before = route->threat_level;

    ft_seed_random_engine(97531u);
    FT_ASSERT(game.handle_nebula_outpost_scan(convoy));

    double threat_after = route->threat_level;
    FT_ASSERT(threat_after < threat_before);
    FT_ASSERT(route->quiet_timer >= 0.0 && route->quiet_timer < 0.0001);
    double elapsed = contract_entry->value.elapsed_seconds;
    FT_ASSERT(elapsed > 9.9 && elapsed < 10.1);

    const ft_vector<ft_string> &log = game.get_lore_log();
    FT_ASSERT(log.size() > lore_before);
    bool found_scan_entry = false;
    for (size_t i = lore_before; i < log.size(); ++i)
    {
        if (ft_strstr(log[i].c_str(), "Nebula-X") != ft_nullptr)
        {
            found_scan_entry = true;
            break;
        }
    }
    FT_ASSERT(found_scan_entry);
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_NEBULA_OUTPOST));

    return 1;
}

