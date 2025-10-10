#include "game_test_menu_shared.hpp"

int verify_load_flow_save_listing()
{
    long timestamp = ft_time_ms();
    ft_string commander("LoadFlowCommander_");
    commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));

    PlayerProfilePreferences preferences;
    preferences.commander_name = commander;
    FT_ASSERT(player_profile_save(preferences));

    ft_string error;
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("gamma"), error));
    FT_ASSERT(error.empty());
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("alpha"), error));
    FT_ASSERT(error.empty());
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("beta"), error));
    FT_ASSERT(error.empty());

    ft_vector<ft_string> slots = load_game_flow_testing::collect_save_slot_names(commander);
    FT_ASSERT_EQ(3U, slots.size());
    FT_ASSERT_EQ(ft_string("alpha"), slots[0]);
    FT_ASSERT_EQ(ft_string("beta"), slots[1]);
    FT_ASSERT_EQ(ft_string("gamma"), slots[2]);

    ft_string beta_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("beta"));
    FT_ASSERT(!beta_path.empty());
    FT_ASSERT(file_exists(beta_path.c_str()) > 0);

    return 1;
}

int verify_load_flow_save_deletion()
{
    long timestamp = ft_time_ms();
    ft_string commander("LoadFlowDelete_");
    commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));

    PlayerProfilePreferences preferences;
    preferences.commander_name = commander;
    FT_ASSERT(player_profile_save(preferences));

    ft_string error;
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("delta"), error));
    FT_ASSERT(error.empty());
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("alpha"), error));
    FT_ASSERT(error.empty());
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("epsilon"), error));
    FT_ASSERT(error.empty());

    ft_vector<ft_string> before = load_game_flow_testing::collect_save_slot_names(commander);
    FT_ASSERT_EQ(3U, before.size());

    ft_string delete_error;
    FT_ASSERT(load_game_flow_testing::delete_save_slot(commander, ft_string("delta"), delete_error));
    FT_ASSERT(delete_error.empty());

    ft_string delta_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("delta"));
    FT_ASSERT(!delta_path.empty());
    FT_ASSERT_EQ(0, file_exists(delta_path.c_str()));

    ft_vector<ft_string> after = load_game_flow_testing::collect_save_slot_names(commander);
    FT_ASSERT_EQ(2U, after.size());
    FT_ASSERT_EQ(ft_string("alpha"), after[0]);
    FT_ASSERT_EQ(ft_string("epsilon"), after[1]);

    ft_string missing_error;
    FT_ASSERT(!load_game_flow_testing::delete_save_slot(commander, ft_string("delta"), missing_error));
    FT_ASSERT(!missing_error.empty());

    return 1;
}

int verify_load_flow_save_rename()
{
    long timestamp = ft_time_ms();
    ft_string commander("LoadFlowRename_");
    commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));

    PlayerProfilePreferences preferences;
    preferences.commander_name = commander;
    FT_ASSERT(player_profile_save(preferences));

    ft_string error;
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("lyra"), error));
    FT_ASSERT(error.empty());
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("orion"), error));
    FT_ASSERT(error.empty());

    ft_vector<ft_string> before = load_game_flow_testing::collect_save_slot_names(commander);
    FT_ASSERT_EQ(2U, before.size());
    FT_ASSERT_EQ(ft_string("lyra"), before[0]);
    FT_ASSERT_EQ(ft_string("orion"), before[1]);

    ft_string rename_error;
    FT_ASSERT(load_game_flow_testing::rename_save_slot(commander, ft_string("lyra"), ft_string("auriga"), rename_error));
    FT_ASSERT(rename_error.empty());

    ft_string old_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("lyra"));
    FT_ASSERT(!old_path.empty());
    FT_ASSERT_EQ(0, file_exists(old_path.c_str()));

    ft_string new_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("auriga"));
    FT_ASSERT(!new_path.empty());
    FT_ASSERT(file_exists(new_path.c_str()) > 0);

    ft_vector<ft_string> after = load_game_flow_testing::collect_save_slot_names(commander);
    FT_ASSERT_EQ(2U, after.size());
    FT_ASSERT_EQ(ft_string("auriga"), after[0]);
    FT_ASSERT_EQ(ft_string("orion"), after[1]);

    ft_string conflict_error;
    FT_ASSERT(!load_game_flow_testing::rename_save_slot(commander, ft_string("orion"), ft_string("auriga"), conflict_error));
    FT_ASSERT(!conflict_error.empty());

    return 1;
}

int verify_load_flow_save_metadata()
{
    long timestamp = ft_time_ms();
    ft_string commander("LoadFlowMetadata_");
    commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));

    menu_localization_reset_for_testing();

    PlayerProfilePreferences preferences;
    preferences.commander_name = commander;
    FT_ASSERT(player_profile_save(preferences));

    ft_string error;
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("alpha"), error));
    FT_ASSERT(error.empty());
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("broken"), error));
    FT_ASSERT(error.empty());

    ft_string alpha_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("alpha"));
    FT_ASSERT(!alpha_path.empty());
    ft_ofstream alpha_stream;
    FT_ASSERT_EQ(0, alpha_stream.open(alpha_path.c_str()));
    ft_string alpha_contents("{\n");
    alpha_contents.append("  \"metadata\": {\"version\": 1, \"save_type\": \"quicksave\"},\n");
    alpha_contents.append("  \"player\": {\"commander_name\": \"Metadata Tester\", \"starting_planet_id\": 1, \"commander_level\": 12},\n");
    alpha_contents.append("  \"campaign\": {\"day\": 7, \"difficulty\": 3}\n");
    alpha_contents.append("}\n");
    FT_ASSERT(alpha_stream.write(alpha_contents.c_str()) >= 0);
    alpha_stream.close();

    ft_string broken_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("broken"));
    FT_ASSERT(!broken_path.empty());
    ft_ofstream broken_stream;
    FT_ASSERT_EQ(0, broken_stream.open(broken_path.c_str()));
    ft_string broken_contents("{\"metadata\":{\"version\":1}}\n");
    FT_ASSERT(broken_stream.write(broken_contents.c_str()) >= 0);
    broken_stream.close();

    ft_vector<ft_string> metadata_labels = load_game_flow_testing::collect_save_slot_metadata_labels(commander);
    FT_ASSERT_EQ(2U, metadata_labels.size());
    ft_string expected_hard = build_expected_metadata_label(
        7, 12, "load_menu.metadata.difficulty.hard", "Hard");
    FT_ASSERT_EQ(expected_hard, metadata_labels[0]);
    FT_ASSERT_EQ(menu_localize("load_menu.metadata.unavailable", "Metadata unavailable"), metadata_labels[1]);

    return 1;
}
