int verify_building_layout_snapshot()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    FT_ASSERT(game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    FT_ASSERT(game.place_building(PLANET_TERRA, BUILDING_SMELTER, 0, 2) != 0);

    Game::ft_building_layout_snapshot snapshot;
    game.get_building_layout_snapshot(snapshot);

    FT_ASSERT(snapshot.planets.size() >= 1);
    const Game::ft_planet_building_snapshot *terra_snapshot = ft_nullptr;
    for (size_t i = 0; i < snapshot.planets.size(); ++i)
    {
        if (snapshot.planets[i].planet_id == PLANET_TERRA)
        {
            terra_snapshot = &snapshot.planets[i];
            break;
        }
    }
    FT_ASSERT(terra_snapshot != ft_nullptr);
    FT_ASSERT_EQ(PLANET_TERRA, terra_snapshot->planet_id);
    FT_ASSERT(ft_strcmp(terra_snapshot->planet_name.c_str(), "Terra") == 0);
    FT_ASSERT_EQ(4, terra_snapshot->width);
    FT_ASSERT_EQ(4, terra_snapshot->height);
    FT_ASSERT_EQ(9, terra_snapshot->used_plots);
    FT_ASSERT_EQ(1, terra_snapshot->base_logistic);
    FT_ASSERT_EQ(1, terra_snapshot->logistic_capacity);
    FT_ASSERT_EQ(0, terra_snapshot->logistic_usage);
    FT_ASSERT_EQ(1, terra_snapshot->logistic_available);
    FT_ASSERT_EQ(0, terra_snapshot->research_logistic_bonus);
    double generator_delta = terra_snapshot->energy_generation - 6.0;
    if (generator_delta < 0.0)
        generator_delta = -generator_delta;
    FT_ASSERT(generator_delta < 0.0001);
    FT_ASSERT(terra_snapshot->energy_consumption < 0.0001);
    FT_ASSERT(terra_snapshot->support_energy < 0.0001);
    FT_ASSERT(terra_snapshot->energy_deficit_pressure < 0.0001);
    FT_ASSERT(terra_snapshot->mine_multiplier >= 1.0);
    FT_ASSERT(terra_snapshot->convoy_speed_bonus < 0.0001);
    FT_ASSERT(terra_snapshot->convoy_raid_risk_modifier < 0.0001);
    FT_ASSERT(!terra_snapshot->emergency_energy_conservation);

    FT_ASSERT_EQ(static_cast<size_t>(terra_snapshot->width * terra_snapshot->height),
                 terra_snapshot->grid.size());

    const Game::ft_building_instance_snapshot *mine_instance = ft_nullptr;
    const Game::ft_building_instance_snapshot *generator_instance = ft_nullptr;
    const Game::ft_building_instance_snapshot *smelter_instance = ft_nullptr;
    for (size_t i = 0; i < terra_snapshot->instances.size(); ++i)
    {
        const Game::ft_building_instance_snapshot &instance = terra_snapshot->instances[i];
        if (instance.building_id == BUILDING_MINE_CORE)
            mine_instance = &terra_snapshot->instances[i];
        else if (instance.building_id == BUILDING_POWER_GENERATOR)
            generator_instance = &terra_snapshot->instances[i];
        else if (instance.building_id == BUILDING_SMELTER)
            smelter_instance = &terra_snapshot->instances[i];
    }
    FT_ASSERT(mine_instance != ft_nullptr);
    FT_ASSERT(generator_instance != ft_nullptr);
    FT_ASSERT(smelter_instance != ft_nullptr);
    FT_ASSERT(!mine_instance->removable);
    FT_ASSERT(mine_instance->occupies_grid);
    FT_ASSERT(generator_instance->occupies_grid);
    FT_ASSERT(smelter_instance->occupies_grid);
    FT_ASSERT_EQ(2, generator_instance->width);
    FT_ASSERT_EQ(2, generator_instance->height);
    double generator_energy_delta = generator_instance->energy_gain - 6.0;
    if (generator_energy_delta < 0.0)
        generator_energy_delta = -generator_energy_delta;
    FT_ASSERT(generator_energy_delta < 0.0001);
    FT_ASSERT_EQ(0, generator_instance->logistic_cost);
    FT_ASSERT_EQ(0, generator_instance->logistic_gain);
    double smelter_cost_delta = smelter_instance->energy_cost - 2.0;
    if (smelter_cost_delta < 0.0)
        smelter_cost_delta = -smelter_cost_delta;
    FT_ASSERT(smelter_cost_delta < 0.0001);
    FT_ASSERT_EQ(1, smelter_instance->logistic_cost);
    FT_ASSERT_EQ(0, smelter_instance->logistic_gain);
    FT_ASSERT_EQ(1U, smelter_instance->inputs.size());
    FT_ASSERT_EQ(ORE_IRON, smelter_instance->inputs[0].key);
    FT_ASSERT_EQ(3, smelter_instance->inputs[0].value);
    FT_ASSERT_EQ(1U, smelter_instance->outputs.size());
    FT_ASSERT_EQ(ITEM_IRON_BAR, smelter_instance->outputs[0].key);
    FT_ASSERT_EQ(2, smelter_instance->outputs[0].value);

    bool saw_mine_cell = false;
    bool saw_generator_cell = false;
    bool saw_smelter_cell = false;
    for (size_t i = 0; i < terra_snapshot->grid.size(); ++i)
    {
        const Game::ft_building_grid_cell_snapshot &cell = terra_snapshot->grid[i];
        if (cell.x == mine_instance->x && cell.y == mine_instance->y)
        {
            saw_mine_cell = true;
            FT_ASSERT_EQ(mine_instance->instance_id, cell.instance_id);
            FT_ASSERT_EQ(BUILDING_MINE_CORE, cell.building_id);
        }
        else if (cell.x == generator_instance->x && cell.y == generator_instance->y)
        {
            saw_generator_cell = true;
            FT_ASSERT_EQ(generator_instance->instance_id, cell.instance_id);
            FT_ASSERT_EQ(BUILDING_POWER_GENERATOR, cell.building_id);
        }
        else if (cell.x == smelter_instance->x && cell.y == smelter_instance->y)
        {
            saw_smelter_cell = true;
            FT_ASSERT_EQ(smelter_instance->instance_id, cell.instance_id);
            FT_ASSERT_EQ(BUILDING_SMELTER, cell.building_id);
        }
    }
    FT_ASSERT(saw_mine_cell);
    FT_ASSERT(saw_generator_cell);
    FT_ASSERT(saw_smelter_cell);

    return 1;
}

