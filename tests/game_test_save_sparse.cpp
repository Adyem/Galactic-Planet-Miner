int verify_save_system_sparse_entries()
{
    SaveSystem saves;

    json_document planet_doc;
    json_group *valid_planet = planet_doc.create_group("planet_valid");
    FT_ASSERT(valid_planet != ft_nullptr);
    planet_doc.append_group(valid_planet);
    json_item *planet_id_item = planet_doc.create_item("id", 1234);
    FT_ASSERT(planet_id_item != ft_nullptr);
    planet_doc.add_item(valid_planet, planet_id_item);
    json_item *planet_amount = planet_doc.create_item("resource_2000", 9001);
    FT_ASSERT(planet_amount != ft_nullptr);
    planet_doc.add_item(valid_planet, planet_amount);
    json_item *planet_rate = planet_doc.create_item("rate_2000", 2500000);
    FT_ASSERT(planet_rate != ft_nullptr);
    planet_doc.add_item(valid_planet, planet_rate);
    json_item *planet_carry = planet_doc.create_item("carryover_2000", -1250000);
    FT_ASSERT(planet_carry != ft_nullptr);
    planet_doc.add_item(valid_planet, planet_carry);
    json_item *planet_item = planet_doc.create_item("item_4000", 33);
    FT_ASSERT(planet_item != ft_nullptr);
    planet_doc.add_item(valid_planet, planet_item);

    json_group *missing_id = planet_doc.create_group("planet_missing_id");
    FT_ASSERT(missing_id != ft_nullptr);
    planet_doc.append_group(missing_id);
    json_item *missing_amount = planet_doc.create_item("resource_1", 15);
    FT_ASSERT(missing_amount != ft_nullptr);
    planet_doc.add_item(missing_id, missing_amount);

    json_group *invalid_planet = planet_doc.create_group("planet_invalid_values");
    FT_ASSERT(invalid_planet != ft_nullptr);
    planet_doc.append_group(invalid_planet);
    json_item *invalid_id = planet_doc.create_item("id", 5678);
    FT_ASSERT(invalid_id != ft_nullptr);
    planet_doc.add_item(invalid_planet, invalid_id);
    json_item *invalid_amount_item = planet_doc.create_item("resource_9000", -42);
    FT_ASSERT(invalid_amount_item != ft_nullptr);
    planet_doc.add_item(invalid_planet, invalid_amount_item);
    json_item *invalid_rate_item = planet_doc.create_item("rate_9000", "invalid");
    FT_ASSERT(invalid_rate_item != ft_nullptr);
    planet_doc.add_item(invalid_planet, invalid_rate_item);
    ft_string nan_sentinel = ft_to_string(FT_LONG_MIN);
    json_item *invalid_carry_item = planet_doc.create_item("carryover_9000",
        nan_sentinel.c_str());
    FT_ASSERT(invalid_carry_item != ft_nullptr);
    planet_doc.add_item(invalid_planet, invalid_carry_item);

    char *planet_raw = planet_doc.write_to_string();
    FT_ASSERT(planet_raw != ft_nullptr);
    ft_string planet_json(planet_raw);
    cma_free(planet_raw);

    ft_map<int, ft_sharedptr<ft_planet> > planets;
    FT_ASSERT(saves.deserialize_planets(planet_json.c_str(), planets));
    FT_ASSERT_EQ(2u, planets.size());

    Pair<int, ft_sharedptr<ft_planet> > *valid_entry = planets.find(1234);
    FT_ASSERT(valid_entry != ft_nullptr);
    ft_sharedptr<ft_planet> restored_valid = valid_entry->value;
    FT_ASSERT(restored_valid);
    FT_ASSERT_EQ(9001, restored_valid->get_resource(2000));
    FT_ASSERT(ft_absolute(restored_valid->get_rate(2000) - 2.5) < 0.000001);
    const ft_vector<Pair<int, double> > &valid_carry = restored_valid->get_carryover();
    bool found_carry = false;
    double carry_value = 0.0;
    for (size_t idx = 0; idx < valid_carry.size(); ++idx)
    {
        if (valid_carry[idx].key == 2000)
        {
            carry_value = valid_carry[idx].value;
            found_carry = true;
            break;
        }
    }
    FT_ASSERT(found_carry);
    FT_ASSERT(ft_absolute(carry_value + 1.25) < 0.000001);
    FT_ASSERT_EQ(33, restored_valid->get_resource(4000));

    Pair<int, ft_sharedptr<ft_planet> > *invalid_entry = planets.find(5678);
    FT_ASSERT(invalid_entry != ft_nullptr);
    ft_sharedptr<ft_planet> restored_invalid = invalid_entry->value;
    FT_ASSERT(restored_invalid);
    FT_ASSERT_EQ(-42, restored_invalid->get_resource(9000));
    FT_ASSERT(ft_absolute(restored_invalid->get_rate(9000)) < 0.000001);
    const ft_vector<Pair<int, double> > &invalid_carry = restored_invalid->get_carryover();
    bool found_invalid_carry = false;
    double invalid_carry_value = 0.0;
    for (size_t idx = 0; idx < invalid_carry.size(); ++idx)
    {
        if (invalid_carry[idx].key == 9000)
        {
            invalid_carry_value = invalid_carry[idx].value;
            found_invalid_carry = true;
            break;
        }
    }
    FT_ASSERT(found_invalid_carry);
    FT_ASSERT(ft_absolute(invalid_carry_value) < 0.000001);

    json_document fleet_doc;
    json_group *valid_fleet = fleet_doc.create_group("fleet_valid");
    FT_ASSERT(valid_fleet != ft_nullptr);
    fleet_doc.append_group(valid_fleet);
    json_item *fleet_id_item = fleet_doc.create_item("id", 2500);
    FT_ASSERT(fleet_id_item != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_id_item);
    json_item *fleet_type_item = fleet_doc.create_item("location_type", LOCATION_TRAVEL);
    FT_ASSERT(fleet_type_item != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_type_item);
    json_item *fleet_from_item = fleet_doc.create_item("location_from", PLANET_TERRA);
    FT_ASSERT(fleet_from_item != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_from_item);
    json_item *fleet_to_item = fleet_doc.create_item("location_to", PLANET_MARS);
    FT_ASSERT(fleet_to_item != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_to_item);
    json_item *fleet_travel_item = fleet_doc.create_item("travel_time", 12750000);
    FT_ASSERT(fleet_travel_item != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_travel_item);
    json_item *fleet_veterancy_item = fleet_doc.create_item("escort_veterancy", 18500000);
    FT_ASSERT(fleet_veterancy_item != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_veterancy_item);
    json_item *fleet_ship_count = fleet_doc.create_item("ship_count", 3);
    FT_ASSERT(fleet_ship_count != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_ship_count);

    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_id", 9100));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_type", SHIP_CAPITAL));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_armor", 450));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_hp", 620));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_shield", 500));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_max_hp", 700));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_max_shield", 560));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_max_speed", 24750000));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_acceleration", 4500000));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_turn_speed", 32500000));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_behavior", SHIP_BEHAVIOR_LAST_STAND));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_outnumbered", SHIP_BEHAVIOR_CHARGE));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_unescorted", SHIP_BEHAVIOR_WITHDRAW_SUPPORT));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_low_hp", SHIP_BEHAVIOR_RETREAT));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_role", SHIP_ROLE_SUPPORT));

    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_1_id", 9101));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_1_type", SHIP_SHIELD));

    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_2_id", 9102));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_2_type", SHIP_RADAR));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_2_max_speed", 16250000));

    json_group *missing_fleet = fleet_doc.create_group("fleet_missing_id");
    FT_ASSERT(missing_fleet != ft_nullptr);
    fleet_doc.append_group(missing_fleet);
    fleet_doc.add_item(missing_fleet, fleet_doc.create_item("ship_count", 1));
    fleet_doc.add_item(missing_fleet, fleet_doc.create_item("ship_0_id", 9999));

    json_group *sparse_fleet = fleet_doc.create_group("fleet_sparse");
    FT_ASSERT(sparse_fleet != ft_nullptr);
    fleet_doc.append_group(sparse_fleet);
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("id", 2600));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("location_type", LOCATION_MISC));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("location_misc", MISC_ASTEROID_HIDEOUT));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("escort_veterancy", -5000));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_count", 4));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_0_id", 9200));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_0_type", SHIP_TRANSPORT));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_0_hp", 180));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_0_behavior", SHIP_BEHAVIOR_SCREEN_SUPPORT));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_0_role", SHIP_ROLE_TRANSPORT));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_3_id", 9203));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_3_type", SHIP_INTERCEPTOR));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_3_max_speed", 36500000));

    char *fleet_raw = fleet_doc.write_to_string();
    FT_ASSERT(fleet_raw != ft_nullptr);
    ft_string fleet_json(fleet_raw);
    cma_free(fleet_raw);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), fleets));
    FT_ASSERT_EQ(2u, fleets.size());

    Pair<int, ft_sharedptr<ft_fleet> > *valid_fleet_entry = fleets.find(2500);
    FT_ASSERT(valid_fleet_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_fleet_valid = valid_fleet_entry->value;
    FT_ASSERT(restored_fleet_valid);
    ft_location valid_location = restored_fleet_valid->get_location();
    FT_ASSERT_EQ(LOCATION_TRAVEL, valid_location.type);
    FT_ASSERT_EQ(PLANET_TERRA, valid_location.from);
    FT_ASSERT_EQ(PLANET_MARS, valid_location.to);
    FT_ASSERT(ft_absolute(restored_fleet_valid->get_travel_time() - 12.75) < 0.000001);
    FT_ASSERT(ft_absolute(restored_fleet_valid->get_escort_veterancy() - 18.5) < 0.000001);
    FT_ASSERT_EQ(3, restored_fleet_valid->get_ship_count());
    const ft_ship *valid_ship_zero = restored_fleet_valid->get_ship(9100);
    FT_ASSERT(valid_ship_zero != ft_nullptr);
    FT_ASSERT_EQ(SHIP_CAPITAL, valid_ship_zero->type);
    FT_ASSERT_EQ(450, valid_ship_zero->armor);
    FT_ASSERT_EQ(620, valid_ship_zero->hp);
    FT_ASSERT_EQ(500, valid_ship_zero->shield);
    FT_ASSERT_EQ(700, valid_ship_zero->max_hp);
    FT_ASSERT_EQ(560, valid_ship_zero->max_shield);
    FT_ASSERT(ft_absolute(valid_ship_zero->max_speed - 24.75) < 0.000001);
    FT_ASSERT(ft_absolute(valid_ship_zero->acceleration - 4.5) < 0.000001);
    FT_ASSERT(ft_absolute(valid_ship_zero->turn_speed - 32.5) < 0.000001);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_LAST_STAND, valid_ship_zero->combat_behavior);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_CHARGE, valid_ship_zero->outnumbered_behavior);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_WITHDRAW_SUPPORT, valid_ship_zero->unescorted_behavior);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_RETREAT, valid_ship_zero->low_hp_behavior);
    FT_ASSERT_EQ(SHIP_ROLE_SUPPORT, valid_ship_zero->role);
    const ft_ship *valid_ship_one = restored_fleet_valid->get_ship(9101);
    FT_ASSERT(valid_ship_one != ft_nullptr);
    FT_ASSERT_EQ(SHIP_SHIELD, valid_ship_one->type);
    FT_ASSERT_EQ(0, valid_ship_one->hp);
    const ft_ship *valid_ship_two = restored_fleet_valid->get_ship(9102);
    FT_ASSERT(valid_ship_two != ft_nullptr);
    FT_ASSERT_EQ(SHIP_RADAR, valid_ship_two->type);
    FT_ASSERT(ft_absolute(valid_ship_two->max_speed - 16.25) < 0.000001);

    Pair<int, ft_sharedptr<ft_fleet> > *sparse_fleet_entry = fleets.find(2600);
    FT_ASSERT(sparse_fleet_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_sparse = sparse_fleet_entry->value;
    FT_ASSERT(restored_sparse);
    ft_location sparse_location = restored_sparse->get_location();
    FT_ASSERT_EQ(LOCATION_MISC, sparse_location.type);
    FT_ASSERT_EQ(MISC_ASTEROID_HIDEOUT, sparse_location.misc);
    FT_ASSERT(ft_absolute(restored_sparse->get_escort_veterancy()) < 0.000001);
    FT_ASSERT_EQ(2, restored_sparse->get_ship_count());
    const ft_ship *sparse_ship_zero = restored_sparse->get_ship(9200);
    FT_ASSERT(sparse_ship_zero != ft_nullptr);
    FT_ASSERT_EQ(SHIP_TRANSPORT, sparse_ship_zero->type);
    FT_ASSERT_EQ(180, sparse_ship_zero->hp);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_SCREEN_SUPPORT, sparse_ship_zero->combat_behavior);
    FT_ASSERT_EQ(SHIP_ROLE_TRANSPORT, sparse_ship_zero->role);
    const ft_ship *sparse_ship_three = restored_sparse->get_ship(9203);
    FT_ASSERT(sparse_ship_three != ft_nullptr);
    FT_ASSERT_EQ(SHIP_INTERCEPTOR, sparse_ship_three->type);
    FT_ASSERT(ft_absolute(sparse_ship_three->max_speed - 36.5) < 0.000001);

    return 1;
}

