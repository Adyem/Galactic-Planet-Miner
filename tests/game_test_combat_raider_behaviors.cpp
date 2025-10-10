int verify_raider_ship_maneuver_profiles()
{
    ft_fleet fleet(3021);

    int corvette_id = fleet.create_ship(SHIP_RAIDER_CORVETTE);
    int destroyer_id = fleet.create_ship(SHIP_RAIDER_DESTROYER);
    int battleship_id = fleet.create_ship(SHIP_RAIDER_BATTLESHIP);

    const ft_ship *corvette = fleet.get_ship(corvette_id);
    const ft_ship *destroyer = fleet.get_ship(destroyer_id);
    const ft_ship *battleship = fleet.get_ship(battleship_id);

    FT_ASSERT(corvette != ft_nullptr);
    FT_ASSERT(destroyer != ft_nullptr);
    FT_ASSERT(battleship != ft_nullptr);

    FT_ASSERT(corvette->deceleration > destroyer->deceleration);
    FT_ASSERT(destroyer->deceleration > battleship->deceleration);

    FT_ASSERT(corvette->turn_speed > destroyer->turn_speed);
    FT_ASSERT(destroyer->turn_speed > battleship->turn_speed);

    FT_ASSERT(corvette->max_speed > destroyer->max_speed);
    FT_ASSERT(destroyer->max_speed >= battleship->max_speed);

    return 1;
}

int verify_combat_tracker_deceleration()
{
    CombatManager manager;
    CombatManager::ft_combat_encounter encounter;
    encounter.defender_operational_ships = 1;
    encounter.raider_operational_ships = 1;
    encounter.defender_line_ships = 1;
    encounter.raider_aggression = 1.0;
    encounter.defender_line = -60.0;

    CombatManager::ft_ship_tracker tracker;
    tracker.base_preferred_radius = 30.0;
    tracker.base_advance_bias = 0.0;
    tracker.base_flank = false;
    tracker.preferred_radius = 30.0;
    tracker.advance_bias = 0.0;
    tracker.flank = false;
    tracker.max_speed = 10.0;
    tracker.acceleration = 2.0;
    tracker.deceleration = 3.0;
    tracker.turn_speed = 90.0;
    tracker.current_speed = 16.0;
    tracker.requires_escort = false;
    tracker.role = SHIP_ROLE_LINE;
    tracker.hp_ratio = 1.0;
    tracker.shield_ratio = 1.0;
    tracker.normal_behavior = SHIP_BEHAVIOR_LINE_HOLD;
    tracker.outnumbered_behavior = 0;
    tracker.unescorted_behavior = 0;
    tracker.low_hp_behavior = 0;
    tracker.heading_x = 0.0;
    tracker.heading_y = 0.0;
    tracker.heading_z = 1.0;
    tracker.drift_speed = 0.0;
    tracker.drift_origin = 0.0;
    tracker.lane_offset = 0.0;
    tracker.vertical_layer = 0.0;
    tracker.spatial.x = 0.0;
    tracker.spatial.y = 0.0;
    tracker.spatial.z = 0.0;

    encounter.defender_tracks.insert(1, tracker);

    manager.update_tracks(encounter.defender_tracks, encounter, 1.0, false, false);
    Pair<int, CombatManager::ft_ship_tracker> *first = encounter.defender_tracks.find(1);
    FT_ASSERT(first != ft_nullptr);
    FT_ASSERT(first->value.current_speed > 12.9);
    FT_ASSERT(first->value.current_speed < 13.1);

    manager.update_tracks(encounter.defender_tracks, encounter, 1.0, false, false);
    Pair<int, CombatManager::ft_ship_tracker> *second = encounter.defender_tracks.find(1);
    FT_ASSERT(second != ft_nullptr);
    FT_ASSERT(second->value.current_speed > 9.9);
    FT_ASSERT(second->value.current_speed < 10.1);

    manager.update_tracks(encounter.defender_tracks, encounter, 1.0, false, false);
    Pair<int, CombatManager::ft_ship_tracker> *third = encounter.defender_tracks.find(1);
    FT_ASSERT(third != ft_nullptr);
    FT_ASSERT(third->value.current_speed > 8.4);
    FT_ASSERT(third->value.current_speed < 8.6);

    return 1;
}

int verify_raider_ships_unavailable_to_players()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    int fleet_id = 6021;
    game.create_fleet(fleet_id);

    int corvette_id = game.create_ship(fleet_id, SHIP_RAIDER_CORVETTE);
    int destroyer_id = game.create_ship(fleet_id, SHIP_RAIDER_DESTROYER);
    int battleship_id = game.create_ship(fleet_id, SHIP_RAIDER_BATTLESHIP);

    FT_ASSERT(corvette_id == 0);
    FT_ASSERT(destroyer_id == 0);
    FT_ASSERT(battleship_id == 0);

    ft_sharedptr<ft_fleet> fleet = game.get_fleet(fleet_id);
    FT_ASSERT(fleet);
    FT_ASSERT(fleet->get_ship_count() == 0);

    return 1;
}

int verify_auto_repair_drone_coordination()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT(game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    FT_ASSERT(game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    FT_ASSERT(game.start_research(RESEARCH_SHIELD_TECHNOLOGY));
    FT_ASSERT(game.start_research(RESEARCH_REPAIR_DRONE_TECHNOLOGY));

    ft_sharedptr<ft_fleet> garrison = game.get_planet_fleet(PLANET_TERRA);
    FT_ASSERT(garrison);

    int drone_id = garrison->create_ship(SHIP_REPAIR_DRONE);
    int escort_id = garrison->create_ship(SHIP_INTERCEPTOR);
    int shield_id = garrison->create_ship(SHIP_SHIELD);
    int corvette_id = garrison->create_ship(SHIP_CORVETTE);
    FT_ASSERT(drone_id != 0);
    FT_ASSERT(escort_id != 0);
    FT_ASSERT(shield_id != 0);
    FT_ASSERT(corvette_id != 0);

    const ft_ship *escort = garrison->get_ship(escort_id);
    FT_ASSERT(escort != ft_nullptr);
    int wounded_hp = escort->max_hp - 60;
    if (wounded_hp < 5)
        wounded_hp = 5;
    garrison->set_ship_hp(escort_id, wounded_hp);
    const ft_ship *wounded = garrison->get_ship(escort_id);
    FT_ASSERT(wounded != ft_nullptr);
    FT_ASSERT(wounded->hp == wounded_hp);

    FT_ASSERT(game.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_AUTO));

    Pair<int, CombatManager::ft_combat_encounter> *encounter_entry = game._combat._encounters.find(PLANET_TERRA);
    FT_ASSERT(encounter_entry != ft_nullptr);
    FT_ASSERT(!encounter_entry->value.modifiers.repair_drones_active);
    FT_ASSERT(!encounter_entry->value.auto_repair_drones_active);

    game.update_combat(500.0);

    const ft_ship *after_tick = garrison->get_ship(escort_id);
    FT_ASSERT(after_tick != ft_nullptr);
    FT_ASSERT(after_tick->hp > wounded_hp);
    FT_ASSERT(after_tick->hp <= after_tick->max_hp);

    encounter_entry = game._combat._encounters.find(PLANET_TERRA);
    FT_ASSERT(encounter_entry != ft_nullptr);
    FT_ASSERT(encounter_entry->value.auto_repair_drones_active);

    garrison->set_ship_hp(escort_id, after_tick->max_hp);
    game.update_combat(0.0);

    encounter_entry = game._combat._encounters.find(PLANET_TERRA);
    FT_ASSERT(encounter_entry != ft_nullptr);
    FT_ASSERT(!encounter_entry->value.auto_repair_drones_active);

    return 1;
}
