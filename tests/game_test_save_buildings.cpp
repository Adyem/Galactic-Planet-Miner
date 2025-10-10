int verify_save_system_invalid_inputs()
{
    SaveSystem saves;

    ft_map<int, ft_sharedptr<ft_planet> > planets;
    ft_sharedptr<ft_planet> terra(new ft_planet_terra());
    terra->register_resource(ORE_IRON, 12.5);
    terra->set_resource(ORE_IRON, 24);
    planets.insert(PLANET_TERRA, terra);
    FT_ASSERT(!saves.deserialize_planets(ft_nullptr, planets));
    Pair<int, ft_sharedptr<ft_planet> > *terra_entry = planets.find(PLANET_TERRA);
    FT_ASSERT(terra_entry != ft_nullptr);
    FT_ASSERT_EQ(24, terra_entry->value->get_resource(ORE_IRON));
    ft_string invalid_payload("not json");
    FT_ASSERT(!saves.deserialize_planets(invalid_payload.c_str(), planets));
    terra_entry = planets.find(PLANET_TERRA);
    FT_ASSERT(terra_entry != ft_nullptr);
    FT_ASSERT_EQ(24, terra_entry->value->get_resource(ORE_IRON));

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    ft_sharedptr<ft_fleet> escort(new ft_fleet(31));
    escort->set_location_planet(PLANET_MARS);
    fleets.insert(31, escort);
    FT_ASSERT(!saves.deserialize_fleets(ft_nullptr, fleets));
    Pair<int, ft_sharedptr<ft_fleet> > *escort_entry = fleets.find(31);
    FT_ASSERT(escort_entry != ft_nullptr);
    FT_ASSERT_EQ(PLANET_MARS, escort_entry->value->get_location().from);
    FT_ASSERT(!saves.deserialize_fleets(invalid_payload.c_str(), fleets));
    escort_entry = fleets.find(31);
    FT_ASSERT(escort_entry != ft_nullptr);
    FT_ASSERT_EQ(PLANET_MARS, escort_entry->value->get_location().from);

    ResearchManager research;
    research.set_duration_scale(2.0);
    FT_ASSERT(research.start(RESEARCH_UNLOCK_MARS));
    ft_vector<int> completed;
    research.tick(3.0, completed);
    double remaining_before = research.get_remaining_time(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(remaining_before > 0.0);
    FT_ASSERT(!saves.deserialize_research(ft_nullptr, research));
    FT_ASSERT(ft_absolute(research.get_remaining_time(RESEARCH_UNLOCK_MARS)
        - remaining_before) < 0.000001);
    FT_ASSERT(ft_absolute(research.get_duration_scale() - 2.0) < 0.000001);
    FT_ASSERT(!saves.deserialize_research(invalid_payload.c_str(), research));
    FT_ASSERT(ft_absolute(research.get_remaining_time(RESEARCH_UNLOCK_MARS)
        - remaining_before) < 0.000001);

    AchievementManager achievements;
    achievements.record_event(ACHIEVEMENT_EVENT_PLANET_UNLOCKED, 1);
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED,
        achievements.get_status(ACHIEVEMENT_SECOND_HOME));
    FT_ASSERT(!saves.deserialize_achievements(ft_nullptr, achievements));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED,
        achievements.get_status(ACHIEVEMENT_SECOND_HOME));
    FT_ASSERT(!saves.deserialize_achievements(invalid_payload.c_str(), achievements));
    FT_ASSERT_EQ(1, achievements.get_progress(ACHIEVEMENT_SECOND_HOME));

    return 1;
}

int verify_save_system_rejects_oversized_building_grids()
{
    SaveSystem saves;
    BuildingManager buildings;

    ft_string excessive_width = save_system_building_payload(2000000, 1);
    FT_ASSERT(excessive_width.size() > 0);
    FT_ASSERT(!saves.deserialize_buildings(excessive_width.c_str(), buildings));
    FT_ASSERT_EQ(0, buildings.get_planet_plot_capacity(99));

    ft_string excessive_height = save_system_building_payload(1, 2000000);
    FT_ASSERT(excessive_height.size() > 0);
    FT_ASSERT(!saves.deserialize_buildings(excessive_height.c_str(), buildings));
    FT_ASSERT_EQ(0, buildings.get_planet_plot_capacity(99));

    ft_string excessive_area = save_system_building_payload(2048, 2048);
    FT_ASSERT(excessive_area.size() > 0);
    FT_ASSERT(!saves.deserialize_buildings(excessive_area.c_str(), buildings));
    FT_ASSERT_EQ(0, buildings.get_planet_plot_capacity(99));

    ft_string valid_payload = save_system_building_payload(4, 4, 37, 5, 12);
    FT_ASSERT(valid_payload.size() > 0);
    FT_ASSERT(saves.deserialize_buildings(valid_payload.c_str(), buildings));
    FT_ASSERT_EQ(16, buildings.get_planet_plot_capacity(99));
    FT_ASSERT_EQ(16, buildings.get_planet_plot_usage(99));
    FT_ASSERT_EQ(5, buildings.get_planet_logistic_capacity(99));
    FT_ASSERT_EQ(5, buildings.get_planet_logistic_usage(99));

    return 1;
}

int verify_save_system_prevents_building_instance_wraparound()
{
    SaveSystem saves;
    BuildingManager buildings;
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    ft_string near_limit_payload = save_system_building_payload(4, 4, 0, 0, 0,
        FT_INT_MAX, PLANET_TERRA);
    FT_ASSERT(near_limit_payload.size() > 0);
    FT_ASSERT(saves.deserialize_buildings(near_limit_payload.c_str(), buildings));

    game.set_ore(PLANET_TERRA, ORE_COAL, 120);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 120);

    FT_ASSERT_EQ(0, buildings.get_building_count(PLANET_TERRA, BUILDING_POWER_GENERATOR));

    int logistic_before = buildings.get_planet_logistic_usage(PLANET_TERRA);
    int coal_before = game.get_ore(PLANET_TERRA, ORE_COAL);
    int copper_before = game.get_ore(PLANET_TERRA, ORE_COPPER);

    int attempt = buildings.place_building(game, PLANET_TERRA,
        BUILDING_POWER_GENERATOR, 0, 0);
    FT_ASSERT_EQ(0, attempt);
    FT_ASSERT_EQ(logistic_before, buildings.get_planet_logistic_usage(PLANET_TERRA));
    FT_ASSERT_EQ(coal_before, game.get_ore(PLANET_TERRA, ORE_COAL));
    FT_ASSERT_EQ(copper_before, game.get_ore(PLANET_TERRA, ORE_COPPER));
    FT_ASSERT_EQ(0, buildings.get_building_count(PLANET_TERRA, BUILDING_POWER_GENERATOR));

    return 1;
}

int verify_save_system_compact_building_serialization()
{
    SaveSystem saves;
    BuildingManager buildings;

    ft_vector<Pair<int, int> > legacy_grid;
    Pair<int, int> cell;
    cell.key = 0;
    cell.value = 7;
    legacy_grid.push_back(cell);
    cell.key = 1;
    cell.value = 7;
    legacy_grid.push_back(cell);
    cell.key = 2;
    cell.value = 2;
    legacy_grid.push_back(cell);
    cell.key = 3;
    cell.value = 2;
    legacy_grid.push_back(cell);
    cell.key = 4;
    cell.value = 2;
    legacy_grid.push_back(cell);
    cell.key = 5;
    cell.value = 9;
    legacy_grid.push_back(cell);

    ft_vector<ft_building_instance> legacy_instances;
    ft_building_instance instance;
    instance.uid = 42;
    instance.definition_id = BUILDING_SMELTER;
    instance.x = 1;
    instance.y = 0;
    instance.progress = 0.5;
    instance.active = true;
    legacy_instances.push_back(instance);

    ft_string legacy_payload = save_system_building_payload(3, 2, 6, 12, 4, 99, 77,
        &legacy_grid, &legacy_instances);
    FT_ASSERT(legacy_payload.size() > 0);
    FT_ASSERT(saves.deserialize_buildings(legacy_payload.c_str(), buildings));

    ft_string serialized = saves.serialize_buildings(buildings);
    FT_ASSERT(serialized.size() > 0);

    json_group *groups = json_read_from_string(serialized.c_str());
    FT_ASSERT(groups != ft_nullptr);

    bool saw_grid = false;
    bool saw_instances = false;
    bool saw_cell_key = false;
    bool saw_instance_key = false;
    const char *grid_value = ft_nullptr;
    const char *instance_value = ft_nullptr;

    json_group *current = groups;
    while (current)
    {
        json_item *type_item = json_find_item(current, "type");
        const char *type_value = ft_nullptr;
        if (type_item)
            type_value = type_item->value;
        if (type_value && ft_strcmp(type_value, "planet") == 0)
        {
            json_item *item = current->items;
            while (item)
            {
                if (item->key)
                {
                    if (ft_strcmp(item->key, "grid") == 0)
                    {
                        saw_grid = true;
                        grid_value = item->value;
                    }
                    else if (ft_strcmp(item->key, "instances") == 0)
                    {
                        saw_instances = true;
                        instance_value = item->value;
                    }
                    else if (ft_strncmp(item->key, "cell_", 5) == 0)
                        saw_cell_key = true;
                    else if (ft_strncmp(item->key, "instance_", 9) == 0)
                        saw_instance_key = true;
                }
                item = item->next;
            }
        }
        current = current->next;
    }

    FT_ASSERT(saw_grid);
    FT_ASSERT(saw_instances);
    FT_ASSERT(!saw_cell_key);
    FT_ASSERT(!saw_instance_key);
    FT_ASSERT(grid_value != ft_nullptr);
    FT_ASSERT(ft_strcmp(grid_value, "7x2 2x3 9") == 0);

    FT_ASSERT(instance_value != ft_nullptr);
    ft_string expected_instances("42,");
    expected_instances.append(ft_to_string(static_cast<long>(instance.definition_id)));
    expected_instances.append(",1,0,");
    long expected_progress = reference_scale_double(instance.progress);
    expected_instances.append(ft_to_string(expected_progress));
    expected_instances.append(",1");
    FT_ASSERT(ft_strcmp(instance_value, expected_instances.c_str()) == 0);

    json_free_groups(groups);

    BuildingManager roundtrip;
    FT_ASSERT(saves.deserialize_buildings(serialized.c_str(), roundtrip));
    FT_ASSERT_EQ(6, roundtrip.get_planet_plot_capacity(77));
    FT_ASSERT_EQ(6, roundtrip.get_planet_plot_usage(77));
    FT_ASSERT_EQ(12, roundtrip.get_planet_logistic_capacity(77));
    FT_ASSERT_EQ(4, roundtrip.get_planet_logistic_usage(77));
    FT_ASSERT_EQ(1, roundtrip.get_building_count(77, instance.definition_id));

    return 1;
}

