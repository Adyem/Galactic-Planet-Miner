int verify_save_system_allocation_failures()
{
    SaveSystem saves;
    SaveSystem::set_json_allocation_hook(save_system_test_allocation_hook);

    ft_map<int, ft_sharedptr<ft_planet> > planets;
    ft_sharedptr<ft_planet> terra(new ft_planet_terra());
    terra->register_resource(ORE_IRON, 1.25);
    terra->set_resource(ORE_IRON, 10);
    planets.insert(PLANET_TERRA, terra);

    save_system_trigger_allocation_failure("group", "planet_1");
    ft_string planet_json = saves.serialize_planets(planets);
    FT_ASSERT_EQ(0u, planet_json.size());

    ft_string recovered_planet = saves.serialize_planets(planets);
    FT_ASSERT(recovered_planet.size() > 0);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    ft_sharedptr<ft_fleet> escort(new ft_fleet(31));
    escort->set_location_planet(PLANET_TERRA);
    ft_ship snapshot;
    snapshot.id = 512;
    snapshot.type = SHIP_SHIELD;
    escort->add_ship_snapshot(snapshot);
    fleets.insert(31, escort);

    save_system_trigger_allocation_failure("item", "ship_count");
    ft_string fleet_json = saves.serialize_fleets(fleets);
    FT_ASSERT_EQ(0u, fleet_json.size());

    ft_string recovered_fleet = saves.serialize_fleets(fleets);
    FT_ASSERT(recovered_fleet.size() > 0);

    save_system_reset_allocation_hook();

    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    SaveSystem::set_json_allocation_hook(save_system_test_allocation_hook);
    save_system_trigger_allocation_failure("group", "planet_1");
    FT_ASSERT(!game.save_campaign_checkpoint(ft_string("allocation_failure")));
    save_system_reset_allocation_hook();

    return 1;
}

int verify_save_system_extreme_scaling()
{
    SaveSystem saves;

    ft_map<int, ft_sharedptr<ft_planet> > planets;
    ft_sharedptr<ft_planet> terra(new ft_planet_terra());
    double huge_positive = save_system_test_double_max() / 4.0;
    double huge_negative = -save_system_test_double_max() / 4.0;
    double nan_value = save_system_test_quiet_nan();
    double positive_infinity = save_system_test_positive_infinity();
    double negative_infinity = save_system_test_negative_infinity();

    terra->register_resource(ORE_IRON, huge_positive);
    terra->set_carryover(ORE_IRON, nan_value);
    terra->register_resource(ORE_COPPER, huge_negative);
    terra->set_carryover(ORE_COPPER, positive_infinity);
    terra->register_resource(ORE_GOLD, positive_infinity);
    terra->set_carryover(ORE_GOLD, negative_infinity);
    terra->register_resource(ORE_COAL, negative_infinity);
    planets.insert(PLANET_TERRA, terra);

    ft_string planet_json = saves.serialize_planets(planets);
    FT_ASSERT(planet_json.size() > 0);

    ft_map<int, ft_sharedptr<ft_planet> > restored_planets;
    FT_ASSERT(saves.deserialize_planets(planet_json.c_str(), restored_planets));

    Pair<int, ft_sharedptr<ft_planet> > *terra_entry = restored_planets.find(PLANET_TERRA);
    FT_ASSERT(terra_entry != ft_nullptr);
    ft_sharedptr<ft_planet> restored_terra = terra_entry->value;
    FT_ASSERT(restored_terra);

    const long scale = 1000000;
    const double scale_double = static_cast<double>(scale);
    double expected_max = static_cast<double>(FT_LONG_MAX - 1L)
        / scale_double;
    double expected_min = static_cast<double>(FT_LONG_MIN + 2L)
        / scale_double;

    double restored_large_positive = restored_terra->get_rate(ORE_IRON);
    FT_ASSERT(ft_absolute(restored_large_positive - expected_max) < 0.000001);

    double restored_large_negative = restored_terra->get_rate(ORE_COPPER);
    FT_ASSERT(ft_absolute(restored_large_negative - expected_min) < 0.000001);

    double restored_positive_inf = restored_terra->get_rate(ORE_GOLD);
    FT_ASSERT(ft_absolute(restored_positive_inf) < 0.000001);

    double restored_negative_inf = restored_terra->get_rate(ORE_COAL);
    FT_ASSERT(ft_absolute(restored_negative_inf) < 0.000001);

    const ft_vector<Pair<int, double> > &carry = restored_terra->get_carryover();
    bool found_iron = false;
    bool found_copper = false;
    bool found_gold = false;
    double carry_iron = 0.0;
    double carry_copper = 0.0;
    double carry_gold = 0.0;
    for (size_t i = 0; i < carry.size(); ++i)
    {
        if (carry[i].key == ORE_IRON)
        {
            carry_iron = carry[i].value;
            found_iron = true;
        }
        else if (carry[i].key == ORE_COPPER)
        {
            carry_copper = carry[i].value;
            found_copper = true;
        }
        else if (carry[i].key == ORE_GOLD)
        {
            carry_gold = carry[i].value;
            found_gold = true;
        }
    }

    FT_ASSERT(found_iron);
    FT_ASSERT(ft_absolute(carry_iron) < 0.000001);

    FT_ASSERT(found_copper);
    FT_ASSERT(ft_absolute(carry_copper) < 0.000001);

    FT_ASSERT(found_gold);
    FT_ASSERT(ft_absolute(carry_gold) < 0.000001);

    return 1;
}

int verify_save_system_normalizes_non_finite_planet_values()
{
    SaveSystem saves;

    json_document document;
    json_group *planet_group = document.create_group("planet_non_finite");
    FT_ASSERT(planet_group != ft_nullptr);
    document.append_group(planet_group);

    json_item *id_item = document.create_item("id", 7777);
    FT_ASSERT(id_item != ft_nullptr);
    document.add_item(planet_group, id_item);

    json_item *resource_nan_item = document.create_item("resource_500", 10);
    FT_ASSERT(resource_nan_item != ft_nullptr);
    document.add_item(planet_group, resource_nan_item);

    json_item *resource_inf_item = document.create_item("resource_501", 5);
    FT_ASSERT(resource_inf_item != ft_nullptr);
    document.add_item(planet_group, resource_inf_item);

    ft_string rate_nan_string = ft_to_string(FT_LONG_MIN);
    json_item *rate_nan_item = document.create_item("rate_500",
        rate_nan_string.c_str());
    FT_ASSERT(rate_nan_item != ft_nullptr);
    document.add_item(planet_group, rate_nan_item);

    ft_string rate_infinite_string = ft_to_string(FT_LONG_MAX);
    json_item *rate_infinite_item = document.create_item("rate_501",
        rate_infinite_string.c_str());
    FT_ASSERT(rate_infinite_item != ft_nullptr);
    document.add_item(planet_group, rate_infinite_item);

    long negative_infinity_sentinel = FT_LONG_MIN + 1L;
    ft_string carry_nan_string = ft_to_string(FT_LONG_MIN);
    ft_string carry_negative_inf_string = ft_to_string(negative_infinity_sentinel);

    json_item *carry_nan_item = document.create_item("carryover_500",
        carry_nan_string.c_str());
    FT_ASSERT(carry_nan_item != ft_nullptr);
    document.add_item(planet_group, carry_nan_item);

    json_item *carry_negative_inf_item = document.create_item("carryover_501",
        carry_negative_inf_string.c_str());
    FT_ASSERT(carry_negative_inf_item != ft_nullptr);
    document.add_item(planet_group, carry_negative_inf_item);

    char *serialized_planet = document.write_to_string();
    FT_ASSERT(serialized_planet != ft_nullptr);
    ft_string planet_json(serialized_planet);
    cma_free(serialized_planet);

    ft_map<int, ft_sharedptr<ft_planet> > planets;
    FT_ASSERT(saves.deserialize_planets(planet_json.c_str(), planets));

    Pair<int, ft_sharedptr<ft_planet> > *planet_entry = planets.find(7777);
    FT_ASSERT(planet_entry != ft_nullptr);
    ft_sharedptr<ft_planet> planet = planet_entry->value;
    FT_ASSERT(planet);

    FT_ASSERT(ft_absolute(planet->get_rate(500)) < 0.000001);
    FT_ASSERT(ft_absolute(planet->get_rate(501)) < 0.000001);

    const ft_vector<Pair<int, double> > &carry = planet->get_carryover();
    bool found_nan_resource = false;
    bool found_inf_resource = false;
    double carry_nan_resource = 0.0;
    double carry_inf_resource = 0.0;
    for (size_t idx = 0; idx < carry.size(); ++idx)
    {
        if (carry[idx].key == 500)
        {
            carry_nan_resource = carry[idx].value;
            found_nan_resource = true;
        }
        else if (carry[idx].key == 501)
        {
            carry_inf_resource = carry[idx].value;
            found_inf_resource = true;
        }
    }

    FT_ASSERT(found_nan_resource);
    FT_ASSERT(ft_absolute(carry_nan_resource) < 0.000001);
    FT_ASSERT(found_inf_resource);
    FT_ASSERT(ft_absolute(carry_inf_resource) < 0.000001);

    ft_vector<Pair<int, int> > produced = planet->produce(5.0);
    FT_ASSERT_EQ(0u, produced.size());

    return 1;
}

