int verify_save_system_massive_payload()
{
    SaveSystem saves;

    const int planet_base_id = 1000;
    const int planet_count = 18;
    const int resource_count = 16;
    const int item_count = 12;
    ft_map<int, ft_sharedptr<ft_planet> > planets;
    for (int planet_index = 0; planet_index < planet_count; ++planet_index)
    {
        int planet_id = planet_base_id + planet_index;
        ft_sharedptr<ft_planet> planet(new ft_planet(planet_id));
        FT_ASSERT(planet);
        for (int resource_index = 0; resource_index < resource_count; ++resource_index)
        {
            int resource_id = 2000 + resource_index;
            double base_rate = static_cast<double>((planet_index + 1)
                * (resource_index + 1));
            double rate;
            if ((resource_index % 3) == 0)
                rate = -base_rate * 0.5;
            else
                rate = base_rate * 0.125 + static_cast<double>(planet_index);
            planet->register_resource(resource_id, rate);
            int amount = (planet_index + 1) * (resource_index + 1) * 7;
            planet->set_resource(resource_id, amount);
            double carry_base = static_cast<double>((planet_index + 1)
                * (resource_index + 1));
            double carry;
            if ((resource_index % 2) == 0)
                carry = 0.25 * carry_base;
            else
                carry = -0.125 * carry_base;
            planet->set_carryover(resource_id, carry);
        }
        for (int item_index = 0; item_index < item_count; ++item_index)
        {
            int item_id = 4000 + item_index;
            planet->ensure_item_slot(item_id);
            int amount = (planet_index + 1) * (item_index + 1) * 11;
            planet->set_resource(item_id, amount);
        }
        planets.insert(planet_id, planet);
    }

    const int fleet_base_id = 900;
    const int fleet_count = 24;
    const int ships_per_fleet = 10;
    const int ship_types[] = {
        SHIP_SHIELD,
        SHIP_RADAR,
        SHIP_SALVAGE,
        SHIP_CAPITAL,
        SHIP_TRANSPORT,
        SHIP_CORVETTE,
        SHIP_INTERCEPTOR,
        SHIP_FRIGATE_SUPPORT
    };
    const int ship_type_count = static_cast<int>(sizeof(ship_types)
        / sizeof(ship_types[0]));
    const int ship_roles[] = {
        SHIP_ROLE_LINE,
        SHIP_ROLE_SUPPORT,
        SHIP_ROLE_TRANSPORT
    };
    const int ship_role_count = static_cast<int>(sizeof(ship_roles)
        / sizeof(ship_roles[0]));
    const int behavior_cycle[] = {
        SHIP_BEHAVIOR_LINE_HOLD,
        SHIP_BEHAVIOR_FLANK_SWEEP,
        SHIP_BEHAVIOR_SCREEN_SUPPORT,
        SHIP_BEHAVIOR_CHARGE,
        SHIP_BEHAVIOR_RETREAT,
        SHIP_BEHAVIOR_WITHDRAW_SUPPORT,
        SHIP_BEHAVIOR_LAST_STAND
    };
    const int behavior_count = static_cast<int>(sizeof(behavior_cycle)
        / sizeof(behavior_cycle[0]));
    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    for (int fleet_index = 0; fleet_index < fleet_count; ++fleet_index)
    {
        int fleet_id = fleet_base_id + fleet_index;
        ft_sharedptr<ft_fleet> fleet(new ft_fleet(fleet_id));
        FT_ASSERT(fleet);
        if ((fleet_index % 3) == 0)
            fleet->set_location_planet(PLANET_TERRA);
        else if ((fleet_index % 3) == 1)
            fleet->set_location_travel(PLANET_TERRA, PLANET_MARS,
                60.0 + static_cast<double>(fleet_index) * 3.5);
        else
            fleet->set_location_misc(MISC_OUTPOST_NEBULA_X);
        double veterancy = static_cast<double>(fleet_index) * 0.75;
        if ((fleet_index % 2) == 1)
            veterancy += 1.0;
        fleet->set_escort_veterancy(veterancy);
        for (int ship_index = 0; ship_index < ships_per_fleet; ++ship_index)
        {
            ft_ship snapshot;
            snapshot.id = 5000 + fleet_index * 100 + ship_index;
            snapshot.type = ship_types[ship_index % ship_type_count];
            snapshot.armor = (fleet_index + 1) * (ship_index + 1) * 3;
            snapshot.hp = (fleet_index + 2) * (ship_index + 5);
            snapshot.shield = (fleet_index + 3) * (ship_index + 2);
            snapshot.max_hp = snapshot.hp + 50;
            snapshot.max_shield = snapshot.shield + 25;
            snapshot.max_speed = 10.0 + static_cast<double>(ship_index) * 2.5
                + static_cast<double>(fleet_index) * 0.25;
            snapshot.acceleration = 1.0
                + static_cast<double>(ship_index) * 0.5
                + static_cast<double>(fleet_index) * 0.1;
            snapshot.turn_speed = 30.0
                + static_cast<double>(ship_index) * 1.75
                + static_cast<double>(fleet_index) * 0.5;
            snapshot.combat_behavior = behavior_cycle[(ship_index + fleet_index)
                % behavior_count];
            snapshot.outnumbered_behavior = behavior_cycle[(ship_index
                + fleet_index + 1) % behavior_count];
            snapshot.unescorted_behavior = behavior_cycle[(ship_index
                + fleet_index + 2) % behavior_count];
            snapshot.low_hp_behavior = behavior_cycle[(ship_index + fleet_index + 3)
                % behavior_count];
            snapshot.role = ship_roles[(ship_index + fleet_index) % ship_role_count];
            fleet->add_ship_snapshot(snapshot);
        }
        fleets.insert(fleet_id, fleet);
    }

    ft_string planet_json = saves.serialize_planets(planets);
    ft_string fleet_json = saves.serialize_fleets(fleets);
    FT_ASSERT(planet_json.size() > 0);
    FT_ASSERT(fleet_json.size() > 0);

    ft_map<int, ft_sharedptr<ft_planet> > restored_planets;
    ft_map<int, ft_sharedptr<ft_fleet> > restored_fleets;
    FT_ASSERT(saves.deserialize_planets(planet_json.c_str(), restored_planets));
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), restored_fleets));
    FT_ASSERT_EQ(planets.size(), restored_planets.size());
    FT_ASSERT_EQ(fleets.size(), restored_fleets.size());

    size_t restored_planet_count = restored_planets.size();
    const Pair<int, ft_sharedptr<ft_planet> > *planet_entries = restored_planets.end();
    planet_entries -= restored_planet_count;
    for (size_t idx = 0; idx < restored_planet_count; ++idx)
    {
        int planet_id = planet_entries[idx].key;
        int planet_index = planet_id - planet_base_id;
        FT_ASSERT(planet_index >= 0 && planet_index < planet_count);
        ft_sharedptr<ft_planet> restored_planet = planet_entries[idx].value;
        FT_ASSERT(restored_planet);
        for (int resource_index = 0; resource_index < resource_count; ++resource_index)
        {
            int resource_id = 2000 + resource_index;
            double base_rate = static_cast<double>((planet_index + 1)
                * (resource_index + 1));
            double expected_rate;
            if ((resource_index % 3) == 0)
                expected_rate = -base_rate * 0.5;
            else
                expected_rate = base_rate * 0.125
                    + static_cast<double>(planet_index);
            double restored_rate = restored_planet->get_rate(resource_id);
            FT_ASSERT(ft_absolute(restored_rate - expected_rate) < 0.000001);
            int expected_amount = (planet_index + 1) * (resource_index + 1) * 7;
            FT_ASSERT_EQ(expected_amount, restored_planet->get_resource(resource_id));
            double carry_base = static_cast<double>((planet_index + 1)
                * (resource_index + 1));
            double expected_carry;
            if ((resource_index % 2) == 0)
                expected_carry = 0.25 * carry_base;
            else
                expected_carry = -0.125 * carry_base;
            double restored_carry = 0.0;
            bool found_carry = false;
            const ft_vector<Pair<int, double> > &carry_entries = restored_planet->get_carryover();
            for (size_t carry_idx = 0; carry_idx < carry_entries.size(); ++carry_idx)
            {
                if (carry_entries[carry_idx].key == resource_id)
                {
                    restored_carry = carry_entries[carry_idx].value;
                    found_carry = true;
                    break;
                }
            }
            FT_ASSERT(found_carry);
            FT_ASSERT(ft_absolute(restored_carry - expected_carry) < 0.000001);
        }
        for (int item_index = 0; item_index < item_count; ++item_index)
        {
            int item_id = 4000 + item_index;
            int expected_amount = (planet_index + 1) * (item_index + 1) * 11;
            FT_ASSERT_EQ(expected_amount, restored_planet->get_resource(item_id));
        }
    }

    size_t restored_fleet_count = restored_fleets.size();
    const Pair<int, ft_sharedptr<ft_fleet> > *fleet_entries = restored_fleets.end();
    fleet_entries -= restored_fleet_count;
    for (size_t idx = 0; idx < restored_fleet_count; ++idx)
    {
        int fleet_id = fleet_entries[idx].key;
        int fleet_index = fleet_id - fleet_base_id;
        FT_ASSERT(fleet_index >= 0 && fleet_index < fleet_count);
        ft_sharedptr<ft_fleet> restored_fleet = fleet_entries[idx].value;
        FT_ASSERT(restored_fleet);
        ft_location location = restored_fleet->get_location();
        if ((fleet_index % 3) == 0)
        {
            FT_ASSERT_EQ(LOCATION_PLANET, location.type);
            FT_ASSERT_EQ(PLANET_TERRA, location.from);
        }
        else if ((fleet_index % 3) == 1)
        {
            FT_ASSERT_EQ(LOCATION_TRAVEL, location.type);
            FT_ASSERT_EQ(PLANET_TERRA, location.from);
            FT_ASSERT_EQ(PLANET_MARS, location.to);
            double expected_travel = 60.0 + static_cast<double>(fleet_index) * 3.5;
            FT_ASSERT(ft_absolute(restored_fleet->get_travel_time() - expected_travel) < 0.000001);
        }
        else
        {
            FT_ASSERT_EQ(LOCATION_MISC, location.type);
            FT_ASSERT_EQ(MISC_OUTPOST_NEBULA_X, location.misc);
        }
        double expected_veterancy = static_cast<double>(fleet_index) * 0.75;
        if ((fleet_index % 2) == 1)
            expected_veterancy += 1.0;
        FT_ASSERT(ft_absolute(restored_fleet->get_escort_veterancy()
            - expected_veterancy) < 0.000001);
        FT_ASSERT_EQ(ships_per_fleet, restored_fleet->get_ship_count());
        for (int ship_index = 0; ship_index < ships_per_fleet; ++ship_index)
        {
            int ship_id = 5000 + fleet_index * 100 + ship_index;
            const ft_ship *restored_ship = restored_fleet->get_ship(ship_id);
            FT_ASSERT(restored_ship != ft_nullptr);
            int expected_type = ship_types[ship_index % ship_type_count];
            FT_ASSERT_EQ(expected_type, restored_ship->type);
            int expected_armor = (fleet_index + 1) * (ship_index + 1) * 3;
            FT_ASSERT_EQ(expected_armor, restored_ship->armor);
            int expected_hp = (fleet_index + 2) * (ship_index + 5);
            FT_ASSERT_EQ(expected_hp, restored_ship->hp);
            int expected_shield = (fleet_index + 3) * (ship_index + 2);
            FT_ASSERT_EQ(expected_shield, restored_ship->shield);
            FT_ASSERT_EQ(expected_hp + 50, restored_ship->max_hp);
            FT_ASSERT_EQ(expected_shield + 25, restored_ship->max_shield);
            double expected_speed = 10.0 + static_cast<double>(ship_index) * 2.5
                + static_cast<double>(fleet_index) * 0.25;
            FT_ASSERT(ft_absolute(restored_ship->max_speed - expected_speed) < 0.000001);
            double expected_acceleration = 1.0
                + static_cast<double>(ship_index) * 0.5
                + static_cast<double>(fleet_index) * 0.1;
            FT_ASSERT(ft_absolute(restored_ship->acceleration - expected_acceleration)
                < 0.000001);
            double expected_turn_speed = 30.0
                + static_cast<double>(ship_index) * 1.75
                + static_cast<double>(fleet_index) * 0.5;
            FT_ASSERT(ft_absolute(restored_ship->turn_speed - expected_turn_speed)
                < 0.000001);
            int expected_behavior = behavior_cycle[(ship_index + fleet_index) % behavior_count];
            FT_ASSERT_EQ(expected_behavior, restored_ship->combat_behavior);
            int expected_outnumbered = behavior_cycle[(ship_index + fleet_index + 1)
                % behavior_count];
            FT_ASSERT_EQ(expected_outnumbered, restored_ship->outnumbered_behavior);
            int expected_unescorted = behavior_cycle[(ship_index + fleet_index + 2)
                % behavior_count];
            FT_ASSERT_EQ(expected_unescorted, restored_ship->unescorted_behavior);
            int expected_low_hp = behavior_cycle[(ship_index + fleet_index + 3)
                % behavior_count];
            FT_ASSERT_EQ(expected_low_hp, restored_ship->low_hp_behavior);
            int expected_role = ship_roles[(ship_index + fleet_index) % ship_role_count];
            FT_ASSERT_EQ(expected_role, restored_ship->role);
        }
    }

    return 1;
}

