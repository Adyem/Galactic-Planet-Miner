int verify_save_system_recovers_corrupt_planet_snapshot()
{
    SaveSystem saves;

    json_document planet_doc;
    json_group *corrupt_planet = planet_doc.create_group("planet_corrupt_recovery");
    FT_ASSERT(corrupt_planet != ft_nullptr);
    planet_doc.append_group(corrupt_planet);

    json_item *planet_id_item = planet_doc.create_item("id", 808);
    FT_ASSERT(planet_id_item != ft_nullptr);
    planet_doc.add_item(corrupt_planet, planet_id_item);

    ft_string infinite_rate = ft_to_string(static_cast<long>(FT_LONG_MAX));
    json_item *rate_item = planet_doc.create_item("rate_101", infinite_rate.c_str());
    FT_ASSERT(rate_item != ft_nullptr);
    planet_doc.add_item(corrupt_planet, rate_item);

    ft_string nan_carry = ft_to_string(static_cast<long>(FT_LONG_MIN));
    json_item *carry_item = planet_doc.create_item("carryover_101", nan_carry.c_str());
    FT_ASSERT(carry_item != ft_nullptr);
    planet_doc.add_item(corrupt_planet, carry_item);

    json_item *resource_item = planet_doc.create_item("resource_101", 2000000);
    FT_ASSERT(resource_item != ft_nullptr);
    planet_doc.add_item(corrupt_planet, resource_item);

    json_item *negative_item = planet_doc.create_item("item_777", -45);
    FT_ASSERT(negative_item != ft_nullptr);
    planet_doc.add_item(corrupt_planet, negative_item);

    json_item *overflow_item = planet_doc.create_item("item_778", 2500000);
    FT_ASSERT(overflow_item != ft_nullptr);
    planet_doc.add_item(corrupt_planet, overflow_item);

    char *planet_raw = planet_doc.write_to_string();
    FT_ASSERT(planet_raw != ft_nullptr);
    ft_string planet_json(planet_raw);
    cma_free(planet_raw);

    ft_map<int, ft_sharedptr<ft_planet> > planets;
    FT_ASSERT(saves.deserialize_planets(planet_json.c_str(), planets));
    FT_ASSERT_EQ(1u, planets.size());

    Pair<int, ft_sharedptr<ft_planet> > *planet_entry = planets.find(808);
    FT_ASSERT(planet_entry != ft_nullptr);
    ft_sharedptr<ft_planet> restored_planet = planet_entry->value;
    FT_ASSERT(restored_planet);

    FT_ASSERT(ft_absolute(restored_planet->get_rate(101)) < 0.000001);
    FT_ASSERT_EQ(1000000, restored_planet->get_resource(101));

    const ft_vector<Pair<int, double> > &carry_snapshot = restored_planet->get_carryover();
    double carry_value = 1.0;
    bool carry_found = false;
    for (size_t i = 0; i < carry_snapshot.size(); ++i)
    {
        if (carry_snapshot[i].key == 101)
        {
            carry_value = carry_snapshot[i].value;
            carry_found = true;
            break;
        }
    }
    FT_ASSERT(carry_found);
    FT_ASSERT(ft_absolute(carry_value) < 0.000001);

    FT_ASSERT_EQ(0, restored_planet->get_resource(777));
    FT_ASSERT_EQ(1000000, restored_planet->get_resource(778));

    return 1;
}

int verify_save_system_sanitizes_corrupt_ship_stats()
{
    SaveSystem saves;

    json_document fleet_doc;
    json_group *fleet_group = fleet_doc.create_group("fleet_corrupt_stats");
    FT_ASSERT(fleet_group != ft_nullptr);
    fleet_doc.append_group(fleet_group);

    json_item *fleet_id_item = fleet_doc.create_item("id", 5021);
    FT_ASSERT(fleet_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_id_item);
    json_item *ship_count_item = fleet_doc.create_item("ship_count", 3);
    FT_ASSERT(ship_count_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_count_item);

    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_id", 710));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_type", SHIP_CAPITAL));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_max_hp", 100));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_hp", 500));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_max_shield", 200));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_shield", 800));

    ft_string speed_inf = ft_to_string(static_cast<long>(FT_LONG_MAX));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_max_speed", speed_inf.c_str()));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_acceleration", 500000));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_deceleration", -125000));
    ft_string turn_nan = ft_to_string(static_cast<long>(FT_LONG_MIN));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_turn_speed", turn_nan.c_str()));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_behavior", SHIP_BEHAVIOR_LAST_STAND));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_outnumbered", SHIP_BEHAVIOR_CHARGE));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_unescorted", SHIP_BEHAVIOR_WITHDRAW_SUPPORT));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_low_hp", SHIP_BEHAVIOR_RETREAT));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_role", SHIP_ROLE_SUPPORT));

    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_1_id", 711));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_1_type", 9999));

    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_2_id", 712));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_2_type", SHIP_SHIELD));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_2_max_hp", 150));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_2_hp", 999));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_2_max_shield", 120));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_2_shield", -50));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_2_max_speed", 1250000));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_2_acceleration", 250000));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_2_turn_speed", 750000));

    char *fleet_raw = fleet_doc.write_to_string();
    FT_ASSERT(fleet_raw != ft_nullptr);
    ft_string fleet_json(fleet_raw);
    cma_free(fleet_raw);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), fleets));
    FT_ASSERT_EQ(1u, fleets.size());

    Pair<int, ft_sharedptr<ft_fleet> > *fleet_entry = fleets.find(5021);
    FT_ASSERT(fleet_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_fleet = fleet_entry->value;
    FT_ASSERT(restored_fleet);
    FT_ASSERT_EQ(2, restored_fleet->get_ship_count());

    const ft_ship *capital_ship = restored_fleet->get_ship(710);
    FT_ASSERT(capital_ship != ft_nullptr);
    FT_ASSERT_EQ(100, capital_ship->max_hp);
    FT_ASSERT_EQ(100, capital_ship->hp);
    FT_ASSERT_EQ(200, capital_ship->max_shield);
    FT_ASSERT_EQ(200, capital_ship->shield);
    FT_ASSERT(ft_absolute(capital_ship->max_speed) < 0.000001);
    FT_ASSERT(ft_absolute(capital_ship->acceleration - 0.5) < 0.000001);
    FT_ASSERT(ft_absolute(capital_ship->deceleration - 0.5) < 0.000001);
    FT_ASSERT(ft_absolute(capital_ship->turn_speed) < 0.000001);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_LAST_STAND, capital_ship->combat_behavior);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_CHARGE, capital_ship->outnumbered_behavior);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_WITHDRAW_SUPPORT, capital_ship->unescorted_behavior);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_RETREAT, capital_ship->low_hp_behavior);
    FT_ASSERT_EQ(SHIP_ROLE_SUPPORT, capital_ship->role);

    FT_ASSERT(restored_fleet->get_ship(711) == ft_nullptr);

    const ft_ship *shield_ship = restored_fleet->get_ship(712);
    FT_ASSERT(shield_ship != ft_nullptr);
    FT_ASSERT_EQ(150, shield_ship->max_hp);
    FT_ASSERT_EQ(150, shield_ship->hp);
    FT_ASSERT_EQ(120, shield_ship->max_shield);
    FT_ASSERT_EQ(0, shield_ship->shield);
    FT_ASSERT(ft_absolute(shield_ship->max_speed - 1.25) < 0.000001);
    FT_ASSERT(ft_absolute(shield_ship->acceleration - 0.25) < 0.000001);
    FT_ASSERT(ft_absolute(shield_ship->turn_speed - 0.75) < 0.000001);

    return 1;
}

