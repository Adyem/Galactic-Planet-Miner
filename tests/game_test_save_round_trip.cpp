int verify_save_system_round_trip()
{
    SaveSystem saves;
    ft_map<int, ft_sharedptr<ft_planet> > planets;
    ft_sharedptr<ft_planet> terra(new ft_planet_terra());
    terra->register_resource(ORE_IRON, 1234567.890123);
    terra->set_resource(ORE_IRON, 4200);
    terra->set_carryover(ORE_IRON, 0.765432);
    terra->register_resource(ORE_COPPER, 345.678901);
    terra->set_resource(ORE_COPPER, 9001);
    terra->set_carryover(ORE_COPPER, 0.123456);
    terra->ensure_item_slot(ITEM_ENGINE_PART);
    terra->set_resource(ITEM_ENGINE_PART, 37);
    terra->ensure_item_slot(ITEM_FUSION_REACTOR);
    terra->set_resource(ITEM_FUSION_REACTOR, 2);
    planets.insert(PLANET_TERRA, terra);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    ft_sharedptr<ft_fleet> fleet(new ft_fleet(88));
    fleet->set_location_travel(PLANET_TERRA, PLANET_MARS, 512.204);
    fleet->set_escort_veterancy(3.141592);
    ft_ship cruiser;
    cruiser.id = 501;
    cruiser.type = SHIP_CAPITAL;
    cruiser.hp = 275;
    cruiser.max_hp = 420;
    cruiser.shield = 180;
    cruiser.max_shield = 220;
    cruiser.max_speed = 987.654321;
    cruiser.acceleration = 12.345678;
    cruiser.turn_speed = 210.987654;
    cruiser.combat_behavior = SHIP_BEHAVIOR_CHARGE;
    cruiser.role = SHIP_ROLE_LINE;
    fleet->add_ship_snapshot(cruiser);
    ft_ship escort;
    escort.id = 777;
    escort.type = SHIP_SHIELD;
    escort.hp = 120;
    escort.max_hp = 160;
    escort.shield = 240;
    escort.max_shield = 260;
    escort.max_speed = 654.321098;
    escort.acceleration = 9.876543;
    escort.turn_speed = 198.765432;
    escort.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
    escort.role = SHIP_ROLE_SUPPORT;
    fleet->add_ship_snapshot(escort);
    fleets.insert(88, fleet);

    ft_string planet_json = saves.serialize_planets(planets);
    ft_string fleet_json = saves.serialize_fleets(fleets);
    FT_ASSERT(planet_json.size() > 0);
    FT_ASSERT(fleet_json.size() > 0);

    ft_map<int, ft_sharedptr<ft_planet> > restored_planets;
    ft_map<int, ft_sharedptr<ft_fleet> > restored_fleets;
    FT_ASSERT(saves.deserialize_planets(planet_json.c_str(), restored_planets));
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), restored_fleets));

    Pair<int, ft_sharedptr<ft_planet> > *terra_entry = restored_planets.find(PLANET_TERRA);
    FT_ASSERT(terra_entry != ft_nullptr);
    ft_sharedptr<ft_planet> restored_terra = terra_entry->value;
    FT_ASSERT(restored_terra);
    double iron_rate = restored_terra->get_rate(ORE_IRON);
    FT_ASSERT(ft_absolute(iron_rate - 1234567.890123) < 0.000001);
    FT_ASSERT_EQ(4200, restored_terra->get_resource(ORE_IRON));
    const ft_vector<Pair<int, double> > &terra_carry = restored_terra->get_carryover();
    double iron_carry = 0.0;
    for (size_t i = 0; i < terra_carry.size(); ++i)
    {
        if (terra_carry[i].key == ORE_IRON)
            iron_carry = terra_carry[i].value;
    }
    FT_ASSERT(ft_absolute(iron_carry - 0.765432) < 0.000001);
    FT_ASSERT_EQ(37, restored_terra->get_resource(ITEM_ENGINE_PART));
    FT_ASSERT_EQ(2, restored_terra->get_resource(ITEM_FUSION_REACTOR));

    Pair<int, ft_sharedptr<ft_fleet> > *fleet_entry = restored_fleets.find(88);
    FT_ASSERT(fleet_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_fleet = fleet_entry->value;
    FT_ASSERT(restored_fleet);
    ft_location restored_loc = restored_fleet->get_location();
    FT_ASSERT_EQ(LOCATION_TRAVEL, restored_loc.type);
    FT_ASSERT_EQ(PLANET_TERRA, restored_loc.from);
    FT_ASSERT_EQ(PLANET_MARS, restored_loc.to);
    FT_ASSERT(ft_absolute(restored_fleet->get_travel_time() - 512.204) < 0.000001);
    FT_ASSERT(ft_absolute(restored_fleet->get_escort_veterancy() - 3.141592) < 0.000001);
    const ft_ship *restored_cruiser = restored_fleet->get_ship(501);
    FT_ASSERT(restored_cruiser != ft_nullptr);
    FT_ASSERT(ft_absolute(restored_cruiser->max_speed - 987.654321) < 0.000001);
    FT_ASSERT_EQ(420, restored_cruiser->max_hp);
    const ft_ship *restored_escort = restored_fleet->get_ship(777);
    FT_ASSERT(restored_escort != ft_nullptr);
    FT_ASSERT(ft_absolute(restored_escort->acceleration - 9.876543) < 0.000001);
    FT_ASSERT_EQ(SHIP_ROLE_SUPPORT, restored_escort->role);

    return 1;
}

int verify_save_system_edge_cases()
{
    SaveSystem saves;

    ft_map<int, ft_sharedptr<ft_planet> > empty_planets;
    ft_string empty_planet_json = saves.serialize_planets(empty_planets);
    if (empty_planet_json.size() > 0)
    {
        ft_map<int, ft_sharedptr<ft_planet> > roundtrip_empty_planets;
        FT_ASSERT(saves.deserialize_planets(empty_planet_json.c_str(), roundtrip_empty_planets));
        FT_ASSERT_EQ(0u, roundtrip_empty_planets.size());
    }

    ft_map<int, ft_sharedptr<ft_fleet> > empty_fleets;
    ft_string empty_fleet_json = saves.serialize_fleets(empty_fleets);
    if (empty_fleet_json.size() > 0)
    {
        ft_map<int, ft_sharedptr<ft_fleet> > roundtrip_empty_fleets;
        FT_ASSERT(saves.deserialize_fleets(empty_fleet_json.c_str(), roundtrip_empty_fleets));
        FT_ASSERT_EQ(0u, roundtrip_empty_fleets.size());
    }

    json_document planet_doc;
    json_group *planet_group = planet_doc.create_group("planet_sparse");
    FT_ASSERT(planet_group != ft_nullptr);
    planet_doc.append_group(planet_group);
    json_item *planet_id_item = planet_doc.create_item("id", PLANET_MARS);
    FT_ASSERT(planet_id_item != ft_nullptr);
    planet_doc.add_item(planet_group, planet_id_item);
    json_item *amount_item = planet_doc.create_item("resource_1", 12);
    FT_ASSERT(amount_item != ft_nullptr);
    planet_doc.add_item(planet_group, amount_item);
    char *sparse_planet_raw = planet_doc.write_to_string();
    FT_ASSERT(sparse_planet_raw != ft_nullptr);
    ft_string sparse_planet_json(sparse_planet_raw);
    cma_free(sparse_planet_raw);
    ft_map<int, ft_sharedptr<ft_planet> > sparse_planets;
    FT_ASSERT(saves.deserialize_planets(sparse_planet_json.c_str(), sparse_planets));
    Pair<int, ft_sharedptr<ft_planet> > *sparse_entry = sparse_planets.find(PLANET_MARS);
    FT_ASSERT(sparse_entry != ft_nullptr);
    FT_ASSERT_EQ(12, sparse_entry->value->get_resource(ORE_IRON));
    FT_ASSERT(ft_absolute(sparse_entry->value->get_rate(ORE_IRON)) < 0.000001);

    json_document fleet_doc;
    json_group *fleet_group = fleet_doc.create_group("fleet_sparse");
    FT_ASSERT(fleet_group != ft_nullptr);
    fleet_doc.append_group(fleet_group);
    json_item *fleet_id_item = fleet_doc.create_item("id", 55);
    FT_ASSERT(fleet_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_id_item);
    json_item *fleet_ship_count = fleet_doc.create_item("ship_count", 1);
    FT_ASSERT(fleet_ship_count != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_ship_count);
    json_item *fleet_ship_id = fleet_doc.create_item("ship_0_id", 333);
    FT_ASSERT(fleet_ship_id != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_ship_id);
    char *sparse_fleet_raw = fleet_doc.write_to_string();
    FT_ASSERT(sparse_fleet_raw != ft_nullptr);
    ft_string sparse_fleet_json(sparse_fleet_raw);
    cma_free(sparse_fleet_raw);
    ft_map<int, ft_sharedptr<ft_fleet> > sparse_fleets;
    FT_ASSERT(saves.deserialize_fleets(sparse_fleet_json.c_str(), sparse_fleets));
    Pair<int, ft_sharedptr<ft_fleet> > *sparse_fleet_entry = sparse_fleets.find(55);
    FT_ASSERT(sparse_fleet_entry != ft_nullptr);
    FT_ASSERT_EQ(1, sparse_fleet_entry->value->get_ship_count());
    const ft_ship *sparse_ship = sparse_fleet_entry->value->get_ship(333);
    FT_ASSERT(sparse_ship != ft_nullptr);
    FT_ASSERT_EQ(0, sparse_ship->hp);
    FT_ASSERT_EQ(0, sparse_ship->shield);
    ft_location sparse_location = sparse_fleet_entry->value->get_location();
    FT_ASSERT_EQ(LOCATION_PLANET, sparse_location.type);
    FT_ASSERT_EQ(PLANET_TERRA, sparse_location.from);
    FT_ASSERT_EQ(PLANET_TERRA, sparse_location.to);
    FT_ASSERT(ft_absolute(sparse_fleet_entry->value->get_escort_veterancy()) < 0.000001);

    return 1;
}

int verify_save_system_sanitizes_ship_movement_stats()
{
    SaveSystem saves;

    json_document fleet_doc;
    json_group *fleet_group = fleet_doc.create_group("fleet_sanitize_movement");
    FT_ASSERT(fleet_group != ft_nullptr);
    fleet_doc.append_group(fleet_group);

    json_item *fleet_id_item = fleet_doc.create_item("id", 930);
    FT_ASSERT(fleet_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_id_item);
    json_item *ship_count_item = fleet_doc.create_item("ship_count", 1);
    FT_ASSERT(ship_count_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_count_item);

    int ship_uid = 8101;
    json_item *ship_id_item = fleet_doc.create_item("ship_0_id", ship_uid);
    FT_ASSERT(ship_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_id_item);
    json_item *ship_type_item = fleet_doc.create_item("ship_0_type", SHIP_CAPITAL);
    FT_ASSERT(ship_type_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_type_item);

    long max_speed_nan = FT_LONG_MIN;
    ft_string max_speed_string = ft_to_string(max_speed_nan);
    json_item *max_speed_item = fleet_doc.create_item("ship_0_max_speed", max_speed_string.c_str());
    FT_ASSERT(max_speed_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, max_speed_item);

    long acceleration_inf = FT_LONG_MAX;
    ft_string acceleration_string = ft_to_string(acceleration_inf);
    json_item *acceleration_item = fleet_doc.create_item("ship_0_acceleration", acceleration_string.c_str());
    FT_ASSERT(acceleration_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, acceleration_item);

    long turn_speed_negative = reference_scale_double(-125.0);
    ft_string turn_speed_string = ft_to_string(turn_speed_negative);
    json_item *turn_speed_item = fleet_doc.create_item("ship_0_turn_speed", turn_speed_string.c_str());
    FT_ASSERT(turn_speed_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, turn_speed_item);

    char *fleet_raw = fleet_doc.write_to_string();
    FT_ASSERT(fleet_raw != ft_nullptr);
    ft_string fleet_json(fleet_raw);
    cma_free(fleet_raw);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), fleets));
    FT_ASSERT_EQ(1u, fleets.size());

    Pair<int, ft_sharedptr<ft_fleet> > *fleet_entry = fleets.find(930);
    FT_ASSERT(fleet_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_fleet = fleet_entry->value;
    FT_ASSERT(restored_fleet);
    FT_ASSERT_EQ(1, restored_fleet->get_ship_count());

    const ft_ship *restored_ship = restored_fleet->get_ship(ship_uid);
    FT_ASSERT(restored_ship != ft_nullptr);
    FT_ASSERT(!save_system_test_is_nan(restored_ship->max_speed));
    FT_ASSERT(!save_system_test_is_infinite(restored_ship->max_speed));
    FT_ASSERT(!save_system_test_is_nan(restored_ship->acceleration));
    FT_ASSERT(!save_system_test_is_infinite(restored_ship->acceleration));
    FT_ASSERT(!save_system_test_is_nan(restored_ship->turn_speed));
    FT_ASSERT(!save_system_test_is_infinite(restored_ship->turn_speed));
    FT_ASSERT(restored_ship->max_speed >= 0.0);
    FT_ASSERT(restored_ship->acceleration >= 0.0);
    FT_ASSERT(restored_ship->turn_speed >= 0.0);
    FT_ASSERT(ft_absolute(restored_ship->max_speed) < 0.000001);
    FT_ASSERT(ft_absolute(restored_ship->acceleration) < 0.000001);
    FT_ASSERT(ft_absolute(restored_ship->turn_speed) < 0.000001);

    CombatManager manager;
    CombatManager::ft_ship_tracker tracker;
    CombatManager::ft_combat_encounter encounter;
    manager.initialize_tracker(tracker, ship_uid, *restored_ship, true, encounter);

    FT_ASSERT(!save_system_test_is_nan(tracker.max_speed));
    FT_ASSERT(!save_system_test_is_infinite(tracker.max_speed));
    FT_ASSERT(!save_system_test_is_nan(tracker.acceleration));
    FT_ASSERT(!save_system_test_is_infinite(tracker.acceleration));
    FT_ASSERT(!save_system_test_is_nan(tracker.turn_speed));
    FT_ASSERT(!save_system_test_is_infinite(tracker.turn_speed));
    FT_ASSERT(ft_absolute(tracker.max_speed - 4.0) < 0.000001);
    FT_ASSERT(ft_absolute(tracker.acceleration - 0.5) < 0.000001);
    FT_ASSERT(ft_absolute(tracker.turn_speed - 10.0) < 0.000001);

    return 1;
}

