int verify_planet_inventory_save_round_trip()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_FUSION_REACTOR);
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART);
    game.set_ore(PLANET_TERRA, ORE_IRON, 135);
    game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 7);
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 11);

    FT_ASSERT(game.save_campaign_checkpoint(ft_string("inventory_roundtrip")));
    ft_string planet_json = game.get_campaign_planet_checkpoint();
    ft_string fleet_json = game.get_campaign_fleet_checkpoint();
    ft_string research_json = game.get_campaign_research_checkpoint();
    ft_string achievement_json = game.get_campaign_achievement_checkpoint();
    ft_string building_json = game.get_campaign_building_checkpoint();
    ft_string progress_json = game.get_campaign_progress_checkpoint();

    game.set_ore(PLANET_TERRA, ORE_IRON, 0);
    game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 0);
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 0);

    FT_ASSERT(game.load_campaign_from_save(planet_json, fleet_json, research_json,
        achievement_json, building_json, progress_json));

    FT_ASSERT_EQ(135, game.get_ore(PLANET_TERRA, ORE_IRON));
    FT_ASSERT_EQ(7, game.get_ore(PLANET_TERRA, ITEM_FUSION_REACTOR));
    FT_ASSERT_EQ(11, game.get_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART));

    return 1;
}

int verify_planet_inventory_resets_unsaved_items()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    game.set_ore(PLANET_TERRA, ORE_IRON, 88);

    FT_ASSERT(game.save_campaign_checkpoint(ft_string("inventory_cleanup")));
    ft_string planet_json = game.get_campaign_planet_checkpoint();
    ft_string fleet_json = game.get_campaign_fleet_checkpoint();
    ft_string research_json = game.get_campaign_research_checkpoint();
    ft_string achievement_json = game.get_campaign_achievement_checkpoint();
    ft_string building_json = game.get_campaign_building_checkpoint();
    ft_string progress_json = game.get_campaign_progress_checkpoint();

    game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 5);
    FT_ASSERT_EQ(5, game.get_ore(PLANET_TERRA, ITEM_FUSION_REACTOR));

    FT_ASSERT(game.load_campaign_from_save(planet_json, fleet_json, research_json,
        achievement_json, building_json, progress_json));

    FT_ASSERT_EQ(88, game.get_ore(PLANET_TERRA, ORE_IRON));
    FT_ASSERT_EQ(0, game.get_ore(PLANET_TERRA, ITEM_FUSION_REACTOR));

    return 1;
}

int verify_building_save_round_trip()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    game.set_ore(PLANET_TERRA, ORE_IRON, 120);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 120);
    game.set_ore(PLANET_TERRA, ORE_COAL, 120);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 0);

    int generator_uid = game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0);
    FT_ASSERT(generator_uid != 0);
    int smelter_uid = game.place_building(PLANET_TERRA, BUILDING_SMELTER, 0, 1);
    FT_ASSERT(smelter_uid != 0);

    game.tick(2.0);

    int logistic_usage_before = game.get_planet_logistic_usage(PLANET_TERRA);
    double energy_generation_before = game.get_planet_energy_generation(PLANET_TERRA);
    double energy_consumption_before = game.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(logistic_usage_before > 0);
    FT_ASSERT(energy_generation_before > 0.0);
    FT_ASSERT(energy_consumption_before > 0.0);

    FT_ASSERT(game.save_campaign_checkpoint(ft_string("building_roundtrip")));
    ft_string planet_json = game.get_campaign_planet_checkpoint();
    ft_string fleet_json = game.get_campaign_fleet_checkpoint();
    ft_string research_json = game.get_campaign_research_checkpoint();
    ft_string achievement_json = game.get_campaign_achievement_checkpoint();
    ft_string building_json = game.get_campaign_building_checkpoint();
    ft_string progress_json = game.get_campaign_progress_checkpoint();
    FT_ASSERT(building_json.size() > 0);

    FT_ASSERT(game.remove_building(PLANET_TERRA, generator_uid));
    FT_ASSERT(game.remove_building(PLANET_TERRA, smelter_uid));
    game.set_ore(PLANET_TERRA, ORE_IRON, 0);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 0);

    FT_ASSERT(game.load_campaign_from_save(planet_json, fleet_json, research_json,
        achievement_json, building_json, progress_json));

    FT_ASSERT_EQ(logistic_usage_before, game.get_planet_logistic_usage(PLANET_TERRA));
    double restored_generation = game.get_planet_energy_generation(PLANET_TERRA);
    double restored_consumption = game.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(ft_absolute(restored_generation - energy_generation_before) < 0.000001);
    FT_ASSERT(ft_absolute(restored_consumption - energy_consumption_before) < 0.000001);

    int restored_generator = game.get_building_instance(PLANET_TERRA, 2, 0);
    int restored_smelter = game.get_building_instance(PLANET_TERRA, 0, 1);
    FT_ASSERT(restored_generator != 0);
    FT_ASSERT(restored_smelter != 0);
    FT_ASSERT_EQ(generator_uid, restored_generator);
    FT_ASSERT_EQ(smelter_uid, restored_smelter);

    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 0);
    int ore_before = game.get_ore(PLANET_TERRA, ORE_IRON);
    game.tick(3.1);
    int ore_after = game.get_ore(PLANET_TERRA, ORE_IRON);
    int bars_after = game.get_ore(PLANET_TERRA, ITEM_IRON_BAR);
    FT_ASSERT_EQ(ore_before - 3, ore_after);
    FT_ASSERT_EQ(2, bars_after);

    return 1;
}

int verify_campaign_load_accepts_empty_building_payload()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    game.set_ore(PLANET_TERRA, ORE_IRON, 140);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 120);
    game.set_ore(PLANET_TERRA, ORE_COAL, 100);

    int generator_uid = game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0);
    FT_ASSERT(generator_uid != 0);
    int smelter_uid = game.place_building(PLANET_TERRA, BUILDING_SMELTER, 0, 1);
    FT_ASSERT(smelter_uid != 0);

    game.tick(1.5);

    int baseline_iron = game.get_ore(PLANET_TERRA, ORE_IRON);
    int baseline_logistics = game.get_planet_logistic_usage(PLANET_TERRA);
    double baseline_generation = game.get_planet_energy_generation(PLANET_TERRA);
    double baseline_consumption = game.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(baseline_logistics > 0);
    FT_ASSERT(baseline_generation > 0.0);
    FT_ASSERT(baseline_consumption > 0.0);

    FT_ASSERT(game.save_campaign_checkpoint(ft_string("empty_building_payload")));
    ft_string planet_json = game.get_campaign_planet_checkpoint();
    ft_string fleet_json = game.get_campaign_fleet_checkpoint();
    ft_string research_json = game.get_campaign_research_checkpoint();
    ft_string achievement_json = game.get_campaign_achievement_checkpoint();
    ft_string building_json = game.get_campaign_building_checkpoint();
    ft_string progress_json = game.get_campaign_progress_checkpoint();
    FT_ASSERT(building_json.size() > 0);

    game.set_ore(PLANET_TERRA, ORE_IRON, 0);
    FT_ASSERT_EQ(0, game.get_ore(PLANET_TERRA, ORE_IRON));

    ft_string empty_building_payload;
    FT_ASSERT(game.load_campaign_from_save(planet_json, fleet_json, research_json,
        achievement_json, empty_building_payload, progress_json));

    FT_ASSERT_EQ(baseline_iron, game.get_ore(PLANET_TERRA, ORE_IRON));
    FT_ASSERT_EQ(baseline_logistics, game.get_planet_logistic_usage(PLANET_TERRA));
    double restored_generation = game.get_planet_energy_generation(PLANET_TERRA);
    double restored_consumption = game.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(ft_absolute(restored_generation - baseline_generation) < 0.000001);
    FT_ASSERT(ft_absolute(restored_consumption - baseline_consumption) < 0.000001);
    FT_ASSERT_EQ(generator_uid, game.get_building_instance(PLANET_TERRA, 2, 0));
    FT_ASSERT_EQ(smelter_uid, game.get_building_instance(PLANET_TERRA, 0, 1));

    return 1;
}

