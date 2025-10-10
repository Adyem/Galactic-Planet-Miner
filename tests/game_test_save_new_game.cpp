int verify_new_game_save_name_rules()
{
    const unsigned int max_length = new_game_flow_testing::max_save_name_length();

    FT_ASSERT(new_game_flow_testing::is_character_allowed('a'));
    FT_ASSERT(new_game_flow_testing::is_character_allowed('Z'));
    FT_ASSERT(new_game_flow_testing::is_character_allowed('5'));
    FT_ASSERT(!new_game_flow_testing::is_character_allowed('!'));

    ft_string save_name;
    FT_ASSERT(new_game_flow_testing::append_character(save_name, 'A'));
    for (unsigned int index = 1U; index < max_length; ++index)
        FT_ASSERT(new_game_flow_testing::append_character(save_name, 'B'));

    FT_ASSERT_EQ(max_length, static_cast<unsigned int>(save_name.size()));
    FT_ASSERT(!new_game_flow_testing::append_character(save_name, 'C'));

    new_game_flow_testing::remove_last_character(save_name);
    FT_ASSERT_EQ(max_length - 1U, static_cast<unsigned int>(save_name.size()));

    FT_ASSERT(!new_game_flow_testing::append_character(save_name, '!'));
    FT_ASSERT(new_game_flow_testing::append_character(save_name, '3'));

    FT_ASSERT(new_game_flow_testing::validate_save_name(save_name));

    ft_string empty_name;
    new_game_flow_testing::remove_last_character(empty_name);
    FT_ASSERT(empty_name.empty());
    FT_ASSERT(!new_game_flow_testing::validate_save_name(empty_name));

    return 1;
}

int verify_new_game_save_creation()
{
    const ft_string commander_name("FlowTesterAlpha");
    const ft_string save_name("FirstCampaign");

    FT_ASSERT(player_profile_delete(commander_name));

    ft_string expected_directory = player_profile_resolve_save_directory(commander_name);
    FT_ASSERT(!expected_directory.empty());

    ft_string expected_path = expected_directory;
    expected_path.append("/");
    expected_path.append(save_name);
    expected_path.append(".json");

    ft_string resolved_path = new_game_flow_testing::compute_save_file_path(commander_name, save_name);
    FT_ASSERT_EQ(expected_path, resolved_path);

    ft_string error_message;
    FT_ASSERT(new_game_flow_testing::create_save_file(commander_name, save_name, error_message));
    FT_ASSERT(error_message.empty());

    FT_ASSERT_EQ(1, file_exists(expected_path.c_str()));

    ft_string duplicate_error;
    FT_ASSERT(!new_game_flow_testing::create_save_file(commander_name, save_name, duplicate_error));
    FT_ASSERT_EQ(ft_string("A save with that name already exists."), duplicate_error);

    ft_string missing_commander_error;
    FT_ASSERT(!new_game_flow_testing::create_save_file(ft_string(), save_name, missing_commander_error));
    FT_ASSERT_EQ(ft_string("Unable to resolve the save file location."), missing_commander_error);

    FT_ASSERT(player_profile_delete(commander_name));

    return 1;
}

int verify_new_game_save_launch_readiness()
{
    const ft_string commander_name("FlowTesterBravo");
    const ft_string save_name("LaunchReady");

    FT_ASSERT(player_profile_delete(commander_name));

    ft_string created_path;
    ft_string error_message;
    FT_ASSERT(new_game_flow_testing::create_save_file_with_path(commander_name, save_name, created_path, error_message));
    FT_ASSERT(error_message.empty());
    FT_ASSERT(!created_path.empty());
    FT_ASSERT_EQ(1, file_exists(created_path.c_str()));
    FT_ASSERT(main_menu_can_launch_campaign(created_path));

    FT_ASSERT(player_profile_delete(commander_name));

    return 1;
}

int verify_tutorial_replay_save_creation()
{
    long timestamp = ft_time_ms();
    ft_string commander("TutorialReplay_");
    commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));

    PlayerProfilePreferences preferences;
    preferences.commander_name = commander;
    FT_ASSERT(player_profile_save(preferences));

    ft_string tutorial_path = player_profile_resolve_tutorial_save_path(commander);
    FT_ASSERT(!tutorial_path.empty());

    if (file_exists(tutorial_path.c_str()) > 0)
        FT_ASSERT_EQ(0, file_delete(tutorial_path.c_str()));

    FT_ASSERT(game_bootstrap_create_tutorial_quicksave(tutorial_path.c_str(), commander));
    FT_ASSERT_EQ(1, file_exists(tutorial_path.c_str()));
    FT_ASSERT(main_menu_can_launch_campaign(tutorial_path));

    json_document document;
    FT_ASSERT_EQ(0, document.read_from_file(tutorial_path.c_str()));

    json_group *metadata_group = document.find_group("metadata");
    FT_ASSERT(metadata_group != ft_nullptr);
    json_item *type_item = document.find_item(metadata_group, "save_type");
    FT_ASSERT(type_item != ft_nullptr);
    FT_ASSERT(type_item->value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(type_item->value, "tutorial"));

    json_group *campaign_group = document.find_group("campaign");
    FT_ASSERT(campaign_group != ft_nullptr);
    json_item *difficulty_item = document.find_item(campaign_group, "difficulty");
    FT_ASSERT(difficulty_item != ft_nullptr);
    FT_ASSERT_EQ(GAME_DIFFICULTY_EASY, ft_atoi(difficulty_item->value));

    json_item *day_item = document.find_item(campaign_group, "day");
    FT_ASSERT(day_item != ft_nullptr);
    FT_ASSERT(day_item->value != ft_nullptr);
    FT_ASSERT_EQ(1, ft_atoi(day_item->value));

    FT_ASSERT(player_profile_delete(commander));

    return 1;
}

