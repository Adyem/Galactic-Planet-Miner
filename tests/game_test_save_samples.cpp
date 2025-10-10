int validate_save_system_serialized_samples()
{
    SaveSystem saves;

    ft_map<int, ft_sharedptr<ft_planet> > planets;
    ft_sharedptr<ft_planet> vulcan(new ft_planet_vulcan());
    double large_rate = 123456789012.345678;
    double large_carryover = 6543210.987654;
    int large_stock = 2147480000;
    vulcan->register_resource(ORE_GOLD, large_rate);
    vulcan->set_resource(ORE_GOLD, large_stock);
    vulcan->set_carryover(ORE_GOLD, large_carryover);
    planets.insert(PLANET_VULCAN, vulcan);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    ft_sharedptr<ft_fleet> armada(new ft_fleet(404));
    double travel_time = 654321.987654;
    double veterancy = 87654.321987;
    armada->set_location_travel(PLANET_VULCAN, PLANET_MARS, travel_time);
    armada->set_escort_veterancy(veterancy);
    ft_ship dreadnought;
    dreadnought.id = 9001;
    dreadnought.type = SHIP_CAPITAL_DREADNOUGHT;
    dreadnought.armor = 987654321;
    dreadnought.hp = 1900000000;
    dreadnought.shield = 1850000000;
    dreadnought.max_hp = 2000000000;
    dreadnought.max_shield = 2100000000;
    dreadnought.max_speed = 43210.987654;
    dreadnought.acceleration = 321.987654;
    dreadnought.turn_speed = 210.123456;
    dreadnought.combat_behavior = SHIP_BEHAVIOR_LAST_STAND;
    dreadnought.outnumbered_behavior = SHIP_BEHAVIOR_CHARGE;
    dreadnought.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
    dreadnought.low_hp_behavior = SHIP_BEHAVIOR_FLANK_SWEEP;
    dreadnought.role = SHIP_ROLE_SUPPORT;
    armada->add_ship_snapshot(dreadnought);
    fleets.insert(404, armada);

    ft_string planet_json = saves.serialize_planets(planets);
    FT_ASSERT(planet_json.size() > 0);
    ft_string fleet_json = saves.serialize_fleets(fleets);
    FT_ASSERT(fleet_json.size() > 0);

    json_group *planet_groups = json_read_from_string(planet_json.c_str());
    FT_ASSERT(planet_groups != ft_nullptr);
    json_group *planet_group = planet_groups;
    while (planet_group)
    {
        json_item *id_item = json_find_item(planet_group, "id");
        if (id_item && ft_atoi(id_item->value) == PLANET_VULCAN)
            break;
        planet_group = planet_group->next;
    }
    FT_ASSERT(planet_group != ft_nullptr);
    ft_string rate_key = "rate_";
    rate_key.append(ft_to_string(ORE_GOLD));
    json_item *rate_item = json_find_item(planet_group, rate_key.c_str());
    FT_ASSERT(rate_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(large_rate), ft_atol(rate_item->value));
    ft_string carry_key = "carryover_";
    carry_key.append(ft_to_string(ORE_GOLD));
    json_item *carry_item = json_find_item(planet_group, carry_key.c_str());
    FT_ASSERT(carry_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(large_carryover), ft_atol(carry_item->value));
    ft_string amount_key = "resource_";
    amount_key.append(ft_to_string(ORE_GOLD));
    json_item *amount_item = json_find_item(planet_group, amount_key.c_str());
    FT_ASSERT(amount_item != ft_nullptr);
    FT_ASSERT_EQ(large_stock, ft_atoi(amount_item->value));
    json_free_groups(planet_groups);

    json_group *fleet_groups = json_read_from_string(fleet_json.c_str());
    FT_ASSERT(fleet_groups != ft_nullptr);
    json_group *fleet_group = fleet_groups;
    while (fleet_group)
    {
        json_item *fleet_id = json_find_item(fleet_group, "id");
        if (fleet_id && ft_atoi(fleet_id->value) == 404)
            break;
        fleet_group = fleet_group->next;
    }
    FT_ASSERT(fleet_group != ft_nullptr);
    json_item *travel_item = json_find_item(fleet_group, "travel_time");
    FT_ASSERT(travel_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(travel_time), ft_atol(travel_item->value));
    json_item *veterancy_item = json_find_item(fleet_group, "escort_veterancy");
    FT_ASSERT(veterancy_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(veterancy), ft_atol(veterancy_item->value));
    json_item *ship_count_item = json_find_item(fleet_group, "ship_count");
    FT_ASSERT(ship_count_item != ft_nullptr);
    FT_ASSERT_EQ(1, ft_atoi(ship_count_item->value));
    ft_string ship_base = "ship_";
    ship_base.append(ft_to_string(0));
    ft_string ship_speed_key = ship_base;
    ship_speed_key.append("_max_speed");
    json_item *ship_speed_item = json_find_item(fleet_group, ship_speed_key.c_str());
    FT_ASSERT(ship_speed_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(dreadnought.max_speed), ft_atol(ship_speed_item->value));
    ft_string ship_accel_key = ship_base;
    ship_accel_key.append("_acceleration");
    json_item *ship_accel_item = json_find_item(fleet_group, ship_accel_key.c_str());
    FT_ASSERT(ship_accel_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(dreadnought.acceleration), ft_atol(ship_accel_item->value));
    ft_string ship_turn_key = ship_base;
    ship_turn_key.append("_turn_speed");
    json_item *ship_turn_item = json_find_item(fleet_group, ship_turn_key.c_str());
    FT_ASSERT(ship_turn_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(dreadnought.turn_speed), ft_atol(ship_turn_item->value));
    json_free_groups(fleet_groups);

    ft_map<int, ft_sharedptr<ft_planet> > restored_planets;
    FT_ASSERT(saves.deserialize_planets(planet_json.c_str(), restored_planets));
    Pair<int, ft_sharedptr<ft_planet> > *vulcan_entry = restored_planets.find(PLANET_VULCAN);
    FT_ASSERT(vulcan_entry != ft_nullptr);
    double restored_rate = vulcan_entry->value->get_rate(ORE_GOLD);
    FT_ASSERT(ft_absolute(restored_rate - large_rate) < 0.000001);
    FT_ASSERT_EQ(large_stock, vulcan_entry->value->get_resource(ORE_GOLD));
    const ft_vector<Pair<int, double> > &restored_carry = vulcan_entry->value->get_carryover();
    double carry_value = 0.0;
    for (size_t i = 0; i < restored_carry.size(); ++i)
    {
        if (restored_carry[i].key == ORE_GOLD)
            carry_value = restored_carry[i].value;
    }
    FT_ASSERT(ft_absolute(carry_value - large_carryover) < 0.000001);

    ft_map<int, ft_sharedptr<ft_fleet> > restored_fleets;
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), restored_fleets));
    Pair<int, ft_sharedptr<ft_fleet> > *armada_entry = restored_fleets.find(404);
    FT_ASSERT(armada_entry != ft_nullptr);
    FT_ASSERT(ft_absolute(armada_entry->value->get_travel_time() - travel_time) < 0.000001);
    FT_ASSERT(ft_absolute(armada_entry->value->get_escort_veterancy() - veterancy) < 0.000001);
    const ft_ship *restored_dreadnought = armada_entry->value->get_ship(9001);
    FT_ASSERT(restored_dreadnought != ft_nullptr);
    FT_ASSERT_EQ(dreadnought.armor, restored_dreadnought->armor);
    FT_ASSERT_EQ(dreadnought.hp, restored_dreadnought->hp);
    FT_ASSERT_EQ(dreadnought.shield, restored_dreadnought->shield);
    FT_ASSERT_EQ(dreadnought.max_hp, restored_dreadnought->max_hp);
    FT_ASSERT_EQ(dreadnought.max_shield, restored_dreadnought->max_shield);
    FT_ASSERT(ft_absolute(restored_dreadnought->max_speed - dreadnought.max_speed) < 0.000001);
    FT_ASSERT(ft_absolute(restored_dreadnought->acceleration - dreadnought.acceleration) < 0.000001);
    FT_ASSERT(ft_absolute(restored_dreadnought->turn_speed - dreadnought.turn_speed) < 0.000001);
    FT_ASSERT_EQ(dreadnought.combat_behavior, restored_dreadnought->combat_behavior);
    FT_ASSERT_EQ(dreadnought.outnumbered_behavior, restored_dreadnought->outnumbered_behavior);
    FT_ASSERT_EQ(dreadnought.unescorted_behavior, restored_dreadnought->unescorted_behavior);
    FT_ASSERT_EQ(dreadnought.low_hp_behavior, restored_dreadnought->low_hp_behavior);
    FT_ASSERT_EQ(dreadnought.role, restored_dreadnought->role);

    return 1;
}

