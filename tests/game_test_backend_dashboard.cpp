int verify_resource_dashboard_overview()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    auto stock_resource = [&](int planet_id, int resource_id, int amount) {
        game.ensure_planet_item_slot(planet_id, resource_id);
        game.set_ore(planet_id, resource_id, amount);
    };

    stock_resource(PLANET_TERRA, ORE_IRON, 140);
    stock_resource(PLANET_TERRA, ORE_COPPER, 80);
    stock_resource(PLANET_TERRA, ORE_COAL, 40);

    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    game.tick(40.0);

    stock_resource(PLANET_MARS, ORE_IRON, 40);
    stock_resource(PLANET_MARS, ORE_COPPER, 20);
    stock_resource(PLANET_MARS, ORE_COAL, 20);

    int dispatched = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ORE_IRON, 20);
    FT_ASSERT_EQ(20, dispatched);

    Game::ft_resource_dashboard dashboard;
    game.get_resource_dashboard(dashboard);

    FT_ASSERT_EQ(game.get_active_convoy_count(), dashboard.total_active_convoys);
    FT_ASSERT(dashboard.total_active_convoys >= 1);
    FT_ASSERT(dashboard.routes.size() >= 1);

    const Game::ft_route_dashboard_entry *route_summary = ft_nullptr;
    for (size_t i = 0; i < dashboard.routes.size(); ++i)
    {
        const Game::ft_route_dashboard_entry &entry = dashboard.routes[i];
        if (entry.origin_planet_id == PLANET_TERRA && entry.destination_planet_id == PLANET_MARS)
        {
            route_summary = &entry;
            break;
        }
    }
    FT_ASSERT(route_summary != ft_nullptr);
    FT_ASSERT(route_summary->route_id != 0);
    FT_ASSERT(route_summary->active_convoys >= 1);
    FT_ASSERT(route_summary->threat_level >= 0.0);
    FT_ASSERT(route_summary->quiet_timer >= 0.0);

    bool found_iron = false;
    bool found_copper = false;
    for (size_t i = 0; i < dashboard.resources.size(); ++i)
    {
        const Game::ft_resource_dashboard_entry &summary = dashboard.resources[i];
        if (summary.resource_id == ORE_IRON)
        {
            found_iron = true;
            FT_ASSERT_EQ(160, summary.total_stock);
            double delta = summary.production_rate - 0.6;
            if (delta < 0.0)
                delta = -delta;
            FT_ASSERT(delta < 0.0001);
        }
        else if (summary.resource_id == ORE_COPPER)
        {
            found_copper = true;
            FT_ASSERT_EQ(100, summary.total_stock);
            double delta = summary.production_rate - 0.6;
            if (delta < 0.0)
                delta = -delta;
            FT_ASSERT(delta < 0.0001);
        }
    }
    FT_ASSERT(found_iron);
    FT_ASSERT(found_copper);

    double average_threat_delta = math_fabs(route_summary->threat_level - dashboard.average_route_threat);
    FT_ASSERT(average_threat_delta < 0.0001);
    double max_threat_delta = math_fabs(route_summary->threat_level - dashboard.maximum_route_threat);
    FT_ASSERT(max_threat_delta < 0.0005);

    return 1;
}

int verify_fleet_management_snapshot()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    game.create_fleet(1200);
    int shield_ship = game.create_ship(1200, SHIP_SHIELD);
    FT_ASSERT(shield_ship != 0);
    int radar_ship = game.create_ship(1200, SHIP_RADAR);
    FT_ASSERT(radar_ship != 0);
    game.set_ship_hp(1200, radar_ship, 90);
    game.set_ship_shield(1200, radar_ship, 55);
    game.set_fleet_location_travel(1200, PLANET_TERRA, PLANET_MARS, 14.5);

    game.create_fleet(1201);
    int salvage_ship = game.create_ship(1201, SHIP_SALVAGE);
    FT_ASSERT(salvage_ship != 0);
    game.set_fleet_location_planet(1201, PLANET_TERRA);

    Game::ft_fleet_management_snapshot snapshot;
    game.get_fleet_management_snapshot(snapshot);

    const Game::ft_fleet_management_entry *travel_entry = ft_nullptr;
    for (size_t i = 0; i < snapshot.traveling_fleets.size(); ++i)
    {
        if (snapshot.traveling_fleets[i].fleet_id == 1200)
        {
            travel_entry = &snapshot.traveling_fleets[i];
            break;
        }
    }
    FT_ASSERT(travel_entry != ft_nullptr);
    FT_ASSERT_EQ(LOCATION_TRAVEL, travel_entry->location_type);
    FT_ASSERT_EQ(PLANET_TERRA, travel_entry->origin_planet_id);
    FT_ASSERT_EQ(PLANET_MARS, travel_entry->destination_planet_id);
    FT_ASSERT(!travel_entry->is_garrison);
    FT_ASSERT_EQ(2, travel_entry->ship_count);
    FT_ASSERT_EQ(2, travel_entry->ships.size());
    double range_delta = travel_entry->average_weapon_range - 292.5;
    if (range_delta < 0.0)
        range_delta = -range_delta;
    FT_ASSERT(range_delta < 0.0001);
    double cadence_delta = travel_entry->average_attack_speed - 0.975;
    if (cadence_delta < 0.0)
        cadence_delta = -cadence_delta;
    FT_ASSERT(cadence_delta < 0.0001);

    bool saw_shield = false;
    bool saw_radar = false;
    for (size_t i = 0; i < travel_entry->ships.size(); ++i)
    {
        const Game::ft_ship_management_entry &ship = travel_entry->ships[i];
        if (ship.ship_type == SHIP_SHIELD)
        {
            saw_shield = true;
            FT_ASSERT_EQ(120, ship.hp);
            FT_ASSERT_EQ(150, ship.max_shield);
            double shield_speed_delta = ship.attack_speed - 0.85;
            if (shield_speed_delta < 0.0)
                shield_speed_delta = -shield_speed_delta;
            FT_ASSERT(shield_speed_delta < 0.0001);
        }
        else if (ship.ship_type == SHIP_RADAR)
        {
            saw_radar = true;
            FT_ASSERT_EQ(90, ship.hp);
            FT_ASSERT_EQ(55, ship.shield);
            double radar_speed_delta = ship.attack_speed - 1.1;
            if (radar_speed_delta < 0.0)
                radar_speed_delta = -radar_speed_delta;
            FT_ASSERT(radar_speed_delta < 0.0001);
            double radar_range_delta = ship.max_range - 320.0;
            if (radar_range_delta < 0.0)
                radar_range_delta = -radar_range_delta;
            FT_ASSERT(radar_range_delta < 0.0001);
        }
    }
    FT_ASSERT(saw_shield);
    FT_ASSERT(saw_radar);

    const Game::ft_fleet_management_entry *station_entry = ft_nullptr;
    for (size_t i = 0; i < snapshot.player_fleets.size(); ++i)
    {
        if (snapshot.player_fleets[i].fleet_id == 1201)
        {
            station_entry = &snapshot.player_fleets[i];
            break;
        }
    }
    FT_ASSERT(station_entry != ft_nullptr);
    FT_ASSERT_EQ(LOCATION_PLANET, station_entry->location_type);
    FT_ASSERT_EQ(1, station_entry->ship_count);
    FT_ASSERT_EQ(1, station_entry->ships.size());
    double salvage_speed_delta = station_entry->average_attack_speed - 0.65;
    if (salvage_speed_delta < 0.0)
        salvage_speed_delta = -salvage_speed_delta;
    FT_ASSERT(salvage_speed_delta < 0.0001);
    const Game::ft_ship_management_entry &salvage_summary = station_entry->ships[0];
    FT_ASSERT_EQ(SHIP_SALVAGE, salvage_summary.ship_type);
    FT_ASSERT_EQ(0, station_entry->station_planet_id);

    return 1;
}

