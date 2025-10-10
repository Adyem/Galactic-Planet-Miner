int verify_save_system_prevents_ship_id_wraparound()
{
    SaveSystem saves;

    json_document fleet_doc;
    json_group *fleet_group = fleet_doc.create_group("fleet_max_ship_id");
    FT_ASSERT(fleet_group != ft_nullptr);
    fleet_doc.append_group(fleet_group);

    json_item *fleet_id_item = fleet_doc.create_item("id", 7100);
    FT_ASSERT(fleet_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_id_item);
    json_item *ship_count_item = fleet_doc.create_item("ship_count", 1);
    FT_ASSERT(ship_count_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_count_item);

    int max_valid_ship_id = FT_INT_MAX - 1;
    json_item *ship_id_item = fleet_doc.create_item("ship_0_id", max_valid_ship_id);
    FT_ASSERT(ship_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_id_item);
    json_item *ship_type_item = fleet_doc.create_item("ship_0_type", SHIP_CAPITAL);
    FT_ASSERT(ship_type_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_type_item);

    char *fleet_raw = fleet_doc.write_to_string();
    FT_ASSERT(fleet_raw != ft_nullptr);
    ft_string fleet_json(fleet_raw);
    cma_free(fleet_raw);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), fleets));
    FT_ASSERT_EQ(1u, fleets.size());

    Pair<int, ft_sharedptr<ft_fleet> > *fleet_entry = fleets.find(7100);
    FT_ASSERT(fleet_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_fleet = fleet_entry->value;
    FT_ASSERT(restored_fleet);
    FT_ASSERT_EQ(1, restored_fleet->get_ship_count());
    const ft_ship *loaded_ship = restored_fleet->get_ship(max_valid_ship_id);
    FT_ASSERT(loaded_ship != ft_nullptr);
    FT_ASSERT_EQ(max_valid_ship_id, loaded_ship->id);

    int wrapped_id = restored_fleet->create_ship(SHIP_SHIELD);
    FT_ASSERT_EQ(0, wrapped_id);

    return 1;
}

int verify_save_system_resolves_duplicate_ship_ids()
{
    SaveSystem saves;

    json_document fleet_doc;
    json_group *fleet_one = fleet_doc.create_group("fleet_duplicate_one");
    FT_ASSERT(fleet_one != ft_nullptr);
    fleet_doc.append_group(fleet_one);
    json_item *fleet_one_id = fleet_doc.create_item("id", 6100);
    FT_ASSERT(fleet_one_id != ft_nullptr);
    fleet_doc.add_item(fleet_one, fleet_one_id);
    json_item *fleet_one_ship_count = fleet_doc.create_item("ship_count", 3);
    FT_ASSERT(fleet_one_ship_count != ft_nullptr);
    fleet_doc.add_item(fleet_one, fleet_one_ship_count);
    json_item *fleet_one_ship_zero = fleet_doc.create_item("ship_0_id", 1200);
    FT_ASSERT(fleet_one_ship_zero != ft_nullptr);
    fleet_doc.add_item(fleet_one, fleet_one_ship_zero);
    json_item *fleet_one_ship_zero_type = fleet_doc.create_item("ship_0_type", SHIP_SHIELD);
    FT_ASSERT(fleet_one_ship_zero_type != ft_nullptr);
    fleet_doc.add_item(fleet_one, fleet_one_ship_zero_type);
    json_item *fleet_one_ship_one = fleet_doc.create_item("ship_1_id", 1300);
    FT_ASSERT(fleet_one_ship_one != ft_nullptr);
    fleet_doc.add_item(fleet_one, fleet_one_ship_one);
    json_item *fleet_one_ship_one_type = fleet_doc.create_item("ship_1_type", SHIP_RADAR);
    FT_ASSERT(fleet_one_ship_one_type != ft_nullptr);
    fleet_doc.add_item(fleet_one, fleet_one_ship_one_type);
    json_item *fleet_one_ship_two = fleet_doc.create_item("ship_2_id", 1200);
    FT_ASSERT(fleet_one_ship_two != ft_nullptr);
    fleet_doc.add_item(fleet_one, fleet_one_ship_two);
    json_item *fleet_one_ship_two_type = fleet_doc.create_item("ship_2_type", SHIP_CORVETTE);
    FT_ASSERT(fleet_one_ship_two_type != ft_nullptr);
    fleet_doc.add_item(fleet_one, fleet_one_ship_two_type);

    json_group *fleet_two = fleet_doc.create_group("fleet_duplicate_two");
    FT_ASSERT(fleet_two != ft_nullptr);
    fleet_doc.append_group(fleet_two);
    json_item *fleet_two_id = fleet_doc.create_item("id", 6101);
    FT_ASSERT(fleet_two_id != ft_nullptr);
    fleet_doc.add_item(fleet_two, fleet_two_id);
    json_item *fleet_two_ship_count = fleet_doc.create_item("ship_count", 2);
    FT_ASSERT(fleet_two_ship_count != ft_nullptr);
    fleet_doc.add_item(fleet_two, fleet_two_ship_count);
    json_item *fleet_two_ship_zero = fleet_doc.create_item("ship_0_id", 1300);
    FT_ASSERT(fleet_two_ship_zero != ft_nullptr);
    fleet_doc.add_item(fleet_two, fleet_two_ship_zero);
    json_item *fleet_two_ship_zero_type = fleet_doc.create_item("ship_0_type", SHIP_SHIELD);
    FT_ASSERT(fleet_two_ship_zero_type != ft_nullptr);
    fleet_doc.add_item(fleet_two, fleet_two_ship_zero_type);
    json_item *fleet_two_ship_one = fleet_doc.create_item("ship_1_id", 1400);
    FT_ASSERT(fleet_two_ship_one != ft_nullptr);
    fleet_doc.add_item(fleet_two, fleet_two_ship_one);
    json_item *fleet_two_ship_one_type = fleet_doc.create_item("ship_1_type", SHIP_TRANSPORT);
    FT_ASSERT(fleet_two_ship_one_type != ft_nullptr);
    fleet_doc.add_item(fleet_two, fleet_two_ship_one_type);

    char *raw = fleet_doc.write_to_string();
    FT_ASSERT(raw != ft_nullptr);
    ft_string fleet_json(raw);
    cma_free(raw);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), fleets));
    FT_ASSERT_EQ(2u, fleets.size());

    Pair<int, ft_sharedptr<ft_fleet> > *fleet_one_entry = fleets.find(6100);
    FT_ASSERT(fleet_one_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_one = fleet_one_entry->value;
    FT_ASSERT(restored_one);
    FT_ASSERT_EQ(3, restored_one->get_ship_count());
    const ft_ship *ship_1200 = restored_one->get_ship(1200);
    FT_ASSERT(ship_1200 != ft_nullptr);
    FT_ASSERT_EQ(SHIP_SHIELD, ship_1200->type);
    const ft_ship *ship_1300 = restored_one->get_ship(1300);
    FT_ASSERT(ship_1300 != ft_nullptr);
    FT_ASSERT_EQ(SHIP_RADAR, ship_1300->type);
    const ft_ship *remapped_alpha = restored_one->get_ship(1301);
    FT_ASSERT(remapped_alpha != ft_nullptr);
    FT_ASSERT_EQ(SHIP_CORVETTE, remapped_alpha->type);

    Pair<int, ft_sharedptr<ft_fleet> > *fleet_two_entry = fleets.find(6101);
    FT_ASSERT(fleet_two_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_two = fleet_two_entry->value;
    FT_ASSERT(restored_two);
    FT_ASSERT_EQ(2, restored_two->get_ship_count());
    FT_ASSERT(restored_two->get_ship(1300) == ft_nullptr);
    const ft_ship *remapped_beta = restored_two->get_ship(1302);
    FT_ASSERT(remapped_beta != ft_nullptr);
    FT_ASSERT_EQ(SHIP_SHIELD, remapped_beta->type);
    const ft_ship *ship_1400 = restored_two->get_ship(1400);
    FT_ASSERT(ship_1400 != ft_nullptr);
    FT_ASSERT_EQ(SHIP_TRANSPORT, ship_1400->type);

    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    game._escape_pod_protocol = true;
    game._escape_pod_rescued.insert(1300, false);
    Pair<int, bool> *escape_before = game._escape_pod_rescued.find(1300);
    FT_ASSERT(escape_before != ft_nullptr);
    FT_ASSERT(!escape_before->value);

    FT_ASSERT(game.load_campaign_from_save(ft_string(), fleet_json, ft_string(), ft_string(), ft_string(), ft_string()));

    ft_sharedptr<ft_fleet> loaded_one = game.get_fleet(6100);
    FT_ASSERT(loaded_one);
    FT_ASSERT(loaded_one->get_ship(1200) != ft_nullptr);
    FT_ASSERT(loaded_one->get_ship(1300) != ft_nullptr);
    FT_ASSERT(loaded_one->get_ship(1301) != ft_nullptr);

    ft_sharedptr<ft_fleet> loaded_two = game.get_fleet(6101);
    FT_ASSERT(loaded_two);
    FT_ASSERT(loaded_two->get_ship(1300) == ft_nullptr);
    FT_ASSERT(loaded_two->get_ship(1302) != ft_nullptr);
    FT_ASSERT(loaded_two->get_ship(1400) != ft_nullptr);

    Pair<int, bool> *escape_after = game._escape_pod_rescued.find(1300);
    FT_ASSERT(escape_after != ft_nullptr);
    FT_ASSERT(!escape_after->value);
    FT_ASSERT(game._escape_pod_rescued.find(1302) == ft_nullptr);

    return 1;
}

