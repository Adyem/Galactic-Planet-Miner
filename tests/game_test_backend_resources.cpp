int verify_fractional_resource_accumulation()
{
    ft_planet_mars planet;
    planet.set_resource(ORE_IRON, 0);
    planet.set_resource(ORE_COPPER, 0);

    for (int i = 0; i < 9; ++i)
        planet.produce(1.0);
    FT_ASSERT_EQ(0, planet.get_resource(ORE_IRON));
    FT_ASSERT_EQ(0, planet.get_resource(ORE_COPPER));

    ft_vector<Pair<int, int> > final_tick = planet.produce(1.0);
    bool found_iron = false;
    for (size_t i = 0; i < final_tick.size(); ++i)
    {
        if (final_tick[i].key == ORE_IRON)
        {
            FT_ASSERT_EQ(1, final_tick[i].value);
            found_iron = true;
        }
    }
    FT_ASSERT(found_iron);
    FT_ASSERT_EQ(1, planet.get_resource(ORE_IRON));
    FT_ASSERT_EQ(1, planet.get_resource(ORE_COPPER));

    for (int i = 0; i < 10; ++i)
        planet.produce(1.0);
    FT_ASSERT_EQ(2, planet.get_resource(ORE_IRON));
    FT_ASSERT_EQ(2, planet.get_resource(ORE_COPPER));

    return 1;
}

int verify_hard_difficulty_fractional_output()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"), GAME_DIFFICULTY_HARD);
    const int planet_id = PLANET_TERRA;
    const int ore_id = ORE_COAL;

    game.set_ore(planet_id, ORE_IRON, 0);
    game.set_ore(planet_id, ORE_COPPER, 0);
    game.set_ore(planet_id, ore_id, 0);

    const int tick_count = 100;
    for (int i = 0; i < tick_count; ++i)
        game.produce(1.0);

    int hard_amount = game.get_ore(planet_id, ore_id);
    FT_ASSERT(hard_amount > 0);

    ft_planet_terra baseline;
    baseline.set_resource(ore_id, 0);
    for (int i = 0; i < tick_count; ++i)
        baseline.produce(1.0);
    int base_amount = baseline.get_resource(ore_id);
    FT_ASSERT(base_amount > 0);

    const double hard_multiplier = 0.85;
    int expected = static_cast<int>(static_cast<double>(base_amount) * hard_multiplier + 0.0000001);
    FT_ASSERT_EQ(expected, hard_amount);

    return 1;
}

int verify_mine_upgrade_station_bonus()
{
    const int planet_id = PLANET_TERRA;
    const int ore_id = ORE_IRON;

    Game baseline(ft_string("127.0.0.1:8080"), ft_string("/"));
    baseline.set_ore(planet_id, ore_id, 0);
    baseline.set_ore(planet_id, ORE_COPPER, 0);
    baseline.set_ore(planet_id, ORE_COAL, 0);

    Game upgraded(ft_string("127.0.0.1:8080"), ft_string("/"));
    upgraded.ensure_planet_item_slot(planet_id, ORE_MITHRIL);
    upgraded.set_ore(planet_id, ore_id, 20);
    upgraded.set_ore(planet_id, ORE_MITHRIL, 4);
    FT_ASSERT(upgraded.place_building(planet_id, BUILDING_UPGRADE_STATION, 1, 0) != 0);
    upgraded.tick(0.0);
    upgraded.set_ore(planet_id, ore_id, 0);
    upgraded.set_ore(planet_id, ORE_COPPER, 0);
    upgraded.set_ore(planet_id, ORE_COAL, 0);
    upgraded.set_ore(planet_id, ORE_MITHRIL, 0);

    const int tick_count = 240;
    for (int i = 0; i < tick_count; ++i)
        baseline.produce(1.0);
    for (int i = 0; i < tick_count; ++i)
        upgraded.produce(1.0);

    int baseline_output = baseline.get_ore(planet_id, ore_id);
    int upgraded_output = upgraded.get_ore(planet_id, ore_id);
    FT_ASSERT(baseline_output > 0);
    FT_ASSERT(upgraded_output > baseline_output);

    double ratio = static_cast<double>(upgraded_output) / static_cast<double>(baseline_output);
    FT_ASSERT(ratio > 1.13);
    FT_ASSERT(ratio < 1.17);

    return 1;
}

int verify_set_ore_creates_missing_resource()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    const int planet_id = PLANET_TERRA;
    const int resource_id = ORE_TRITIUM;

    FT_ASSERT_EQ(0, game.get_ore(planet_id, resource_id));

    const int first_amount = 37;
    game.set_ore(planet_id, resource_id, first_amount);
    FT_ASSERT_EQ(first_amount, game.get_ore(planet_id, resource_id));

    const int second_amount = 12;
    game.set_ore(planet_id, resource_id, second_amount);
    FT_ASSERT_EQ(second_amount, game.get_ore(planet_id, resource_id));

    return 1;
}

