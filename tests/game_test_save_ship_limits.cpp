int verify_save_system_rejects_overlarge_ship_ids()
{
    SaveSystem saves;

    ft_fleet baseline;
    int baseline_id = baseline.create_ship(SHIP_SHIELD);
    FT_ASSERT(baseline_id > 0);
    FT_ASSERT(baseline_id < FT_INT_MAX);

    json_document fleet_doc;
    json_group *fleet_group = fleet_doc.create_group("fleet_overlarge_ship");
    FT_ASSERT(fleet_group != ft_nullptr);
    fleet_doc.append_group(fleet_group);

    json_item *fleet_id_item = fleet_doc.create_item("id", 5100);
    FT_ASSERT(fleet_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_id_item);
    json_item *ship_count_item = fleet_doc.create_item("ship_count", 1);
    FT_ASSERT(ship_count_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_count_item);
    json_item *ship_id_item = fleet_doc.create_item("ship_0_id", FT_INT_MAX);
    FT_ASSERT(ship_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_id_item);

    char *fleet_raw = fleet_doc.write_to_string();
    FT_ASSERT(fleet_raw != ft_nullptr);
    ft_string fleet_json(fleet_raw);
    cma_free(fleet_raw);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    FT_ASSERT(!saves.deserialize_fleets(fleet_json.c_str(), fleets));

    ft_fleet followup;
    int next_id = followup.create_ship(SHIP_SHIELD);
    FT_ASSERT_EQ(baseline_id + 1, next_id);

    return 1;
}

int verify_save_system_limits_inflated_ship_counts()
{
    SaveSystem saves;

    json_document fleet_doc;
    json_group *fleet_group = fleet_doc.create_group("fleet_inflated");
    FT_ASSERT(fleet_group != ft_nullptr);
    fleet_doc.append_group(fleet_group);

    json_item *fleet_id_item = fleet_doc.create_item("id", 612);
    FT_ASSERT(fleet_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_id_item);
    json_item *ship_count_item = fleet_doc.create_item("ship_count", 512);
    FT_ASSERT(ship_count_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_count_item);

    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_id", 3100));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_type", SHIP_CAPITAL));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_hp", 300));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_1_id", 3200));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_1_type", SHIP_SHIELD));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_1_role", SHIP_ROLE_SUPPORT));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_90_id", FT_INT_MAX));

    char *fleet_raw = fleet_doc.write_to_string();
    FT_ASSERT(fleet_raw != ft_nullptr);
    ft_string fleet_json(fleet_raw);
    cma_free(fleet_raw);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), fleets));
    FT_ASSERT_EQ(1u, fleets.size());

    Pair<int, ft_sharedptr<ft_fleet> > *fleet_entry = fleets.find(612);
    FT_ASSERT(fleet_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_fleet = fleet_entry->value;
    FT_ASSERT(restored_fleet);
    FT_ASSERT_EQ(2, restored_fleet->get_ship_count());
    const ft_ship *ship_zero = restored_fleet->get_ship(3100);
    FT_ASSERT(ship_zero != ft_nullptr);
    FT_ASSERT_EQ(SHIP_CAPITAL, ship_zero->type);
    FT_ASSERT_EQ(300, ship_zero->hp);
    const ft_ship *ship_one = restored_fleet->get_ship(3200);
    FT_ASSERT(ship_one != ft_nullptr);
    FT_ASSERT_EQ(SHIP_SHIELD, ship_one->type);
    FT_ASSERT_EQ(SHIP_ROLE_SUPPORT, ship_one->role);
    const ft_ship *missing_ship = restored_fleet->get_ship(FT_INT_MAX);
    FT_ASSERT(missing_ship == ft_nullptr);

    return 1;
}

int verify_save_system_recovers_underreported_ship_counts()
{
    SaveSystem saves;

    json_document fleet_doc;
    json_group *fleet_group = fleet_doc.create_group("fleet_underreported");
    FT_ASSERT(fleet_group != ft_nullptr);
    fleet_doc.append_group(fleet_group);

    json_item *fleet_id_item = fleet_doc.create_item("id", 812);
    FT_ASSERT(fleet_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_id_item);
    json_item *ship_count_item = fleet_doc.create_item("ship_count", 1);
    FT_ASSERT(ship_count_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_count_item);

    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_id", 4100));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_type", SHIP_CAPITAL));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_hp", 300));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_1_id", 4200));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_1_type", SHIP_SHIELD));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_1_role", SHIP_ROLE_SUPPORT));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_2_id", 4300));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_2_type", SHIP_TRANSPORT));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_2_hp", 75));

    char *fleet_raw = fleet_doc.write_to_string();
    FT_ASSERT(fleet_raw != ft_nullptr);
    ft_string fleet_json(fleet_raw);
    cma_free(fleet_raw);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), fleets));
    FT_ASSERT_EQ(1u, fleets.size());

    Pair<int, ft_sharedptr<ft_fleet> > *fleet_entry = fleets.find(812);
    FT_ASSERT(fleet_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_fleet = fleet_entry->value;
    FT_ASSERT(restored_fleet);
    FT_ASSERT_EQ(3, restored_fleet->get_ship_count());

    const ft_ship *ship_zero = restored_fleet->get_ship(4100);
    FT_ASSERT(ship_zero != ft_nullptr);
    FT_ASSERT_EQ(SHIP_CAPITAL, ship_zero->type);
    FT_ASSERT_EQ(300, ship_zero->hp);

    const ft_ship *ship_one = restored_fleet->get_ship(4200);
    FT_ASSERT(ship_one != ft_nullptr);
    FT_ASSERT_EQ(SHIP_SHIELD, ship_one->type);
    FT_ASSERT_EQ(SHIP_ROLE_SUPPORT, ship_one->role);

    const ft_ship *ship_two = restored_fleet->get_ship(4300);
    FT_ASSERT(ship_two != ft_nullptr);
    FT_ASSERT_EQ(SHIP_TRANSPORT, ship_two->type);
    FT_ASSERT_EQ(75, ship_two->hp);

    return 1;
}

