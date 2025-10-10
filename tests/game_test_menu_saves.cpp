#include "game_test_menu_shared.hpp"

int verify_save_system_background_queue()
{
    save_system_background_reset();

    SaveSystemBackgroundEvent event;
    FT_ASSERT(!save_system_background_poll_event(event));

    const ft_string slot("alpha");
    save_system_background_push_started(slot, 100L);
    save_system_background_push_completed(slot, false, ft_string("write failure"), 200L);

    FT_ASSERT(save_system_background_poll_event(event));
    FT_ASSERT_EQ(SaveSystemBackgroundEvent::SAVE_SYSTEM_BACKGROUND_EVENT_STARTED, event.type);
    FT_ASSERT_EQ(slot, event.slot_name);
    FT_ASSERT(!event.success);
    FT_ASSERT_EQ(100L, event.timestamp_ms);

    FT_ASSERT(save_system_background_poll_event(event));
    FT_ASSERT_EQ(SaveSystemBackgroundEvent::SAVE_SYSTEM_BACKGROUND_EVENT_COMPLETED, event.type);
    FT_ASSERT_EQ(slot, event.slot_name);
    FT_ASSERT(!event.success);
    FT_ASSERT_EQ(ft_string("write failure"), event.error_message);
    FT_ASSERT_EQ(200L, event.timestamp_ms);

    FT_ASSERT(!save_system_background_poll_event(event));

    return 1;
}

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

int verify_resume_latest_save_resolution()
{
    long timestamp = ft_time_ms();
    ft_string commander("ResumeQuickEntry_");
    commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));

    menu_localization_reset_for_testing();

    PlayerProfilePreferences preferences;
    preferences.commander_name = commander;
    FT_ASSERT(player_profile_save(preferences));

    ft_string error;
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("alpha"), error));
    FT_ASSERT(error.empty());
    time_sleep_ms(50);

    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("beta"), error));
    FT_ASSERT(error.empty());

    ft_string beta_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("beta"));
    FT_ASSERT(!beta_path.empty());
    ft_ofstream beta_stream;
    FT_ASSERT_EQ(0, beta_stream.open(beta_path.c_str()));
    ft_string beta_contents("{\n");
    beta_contents.append("  \"metadata\": {\"version\": 1, \"save_type\": \"quicksave\"},\n");
    beta_contents.append("  \"player\": {\"commander_name\": \"Resume Tester\", \"starting_planet_id\": 2, \"commander_level\": 9},\n");
    beta_contents.append("  \"campaign\": {\"day\": 11, \"difficulty\": 1}\n");
    beta_contents.append("}\n");
    FT_ASSERT(beta_stream.write(beta_contents.c_str()) >= 0);
    beta_stream.close();

    time_sleep_ms(50);

    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("corrupt"), error));
    FT_ASSERT(error.empty());

    ft_string corrupt_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("corrupt"));
    FT_ASSERT(!corrupt_path.empty());
    ft_ofstream corrupt_stream;
    FT_ASSERT_EQ(0, corrupt_stream.open(corrupt_path.c_str()));
    ft_string corrupt_contents("{\"metadata\":\n");
    FT_ASSERT(corrupt_stream.write(corrupt_contents.c_str()) >= 0);
    corrupt_stream.close();

    ft_string tutorial_path = player_profile_resolve_tutorial_save_path(commander);
    FT_ASSERT(!tutorial_path.empty());
    FT_ASSERT(game_bootstrap_create_tutorial_quicksave(tutorial_path.c_str(), commander));

    ft_string slot_name;
    ft_string save_path;
    ft_string metadata_label;
    bool      metadata_available = false;
    FT_ASSERT(resolve_latest_resume_slot(commander, slot_name, save_path, metadata_label, metadata_available));
    FT_ASSERT_EQ(ft_string("beta"), slot_name);
    FT_ASSERT_EQ(beta_path, save_path);
    FT_ASSERT(metadata_available);
    ft_string expected_easy = build_expected_metadata_label(
        11, 9, "load_menu.metadata.difficulty.easy", "Easy");
    FT_ASSERT_EQ(expected_easy, metadata_label);

    ft_vector<ft_string> errors;
    FT_ASSERT(audit_save_directory_for_errors(commander, errors));
    FT_ASSERT_EQ(1U, errors.size());

    return 1;
}

int verify_main_menu_resume_entry_integration()
{
    menu_localization_reset_for_testing();

    ft_vector<ft_menu_item> items = build_main_menu_items();
    size_t resume_index = items.size();
    for (size_t index = 0; index < items.size(); ++index)
    {
        if (items[index].identifier == ft_string("resume"))
        {
            resume_index = index;
            break;
        }
    }
    FT_ASSERT(resume_index < items.size());

    ft_string disabled_description = menu_localize("main_menu.resume.empty_description",
        "No healthy campaign saves found yet. Create or load a game to enable quick resume.");
    FT_ASSERT_EQ(disabled_description, items[resume_index].description);
    FT_ASSERT(!items[resume_index].enabled);

    main_menu_apply_resume_state(items, false, ft_string(), ft_string(), false);
    FT_ASSERT(!items[resume_index].enabled);
    FT_ASSERT_EQ(disabled_description, items[resume_index].description);

    ft_string resume_slot("beta");
    ft_string metadata_label
        = build_expected_metadata_label(11, 9, "load_menu.metadata.difficulty.easy", "Easy");

    main_menu_apply_resume_state(items, true, resume_slot, metadata_label, true);
    FT_ASSERT(items[resume_index].enabled);

    ft_vector<StringTableReplacement> replacements;
    replacements.reserve(2U);
    StringTableReplacement slot_placeholder;
    slot_placeholder.key = ft_string("slot");
    slot_placeholder.value = resume_slot;
    replacements.push_back(slot_placeholder);

    StringTableReplacement metadata_placeholder;
    metadata_placeholder.key = ft_string("metadata");
    ft_string metadata_suffix(" (");
    metadata_suffix.append(metadata_label);
    metadata_suffix.append(")");
    metadata_placeholder.value = metadata_suffix;
    replacements.push_back(metadata_placeholder);

    ft_string expected_description = menu_localize_format("main_menu.resume.active_description",
        "Jump back into \"{{slot}}\"{{metadata}} without opening the load menu.", replacements);
    FT_ASSERT_EQ(expected_description, items[resume_index].description);

    main_menu_apply_resume_state(items, true, resume_slot, ft_string(), false);

    ft_vector<StringTableReplacement> unknown_replacements;
    unknown_replacements.reserve(2U);
    StringTableReplacement unknown_slot_placeholder;
    unknown_slot_placeholder.key = ft_string("slot");
    unknown_slot_placeholder.value = resume_slot;
    unknown_replacements.push_back(unknown_slot_placeholder);

    StringTableReplacement unknown_metadata_placeholder;
    unknown_metadata_placeholder.key = ft_string("metadata");
    unknown_metadata_placeholder.value = ft_string();
    unknown_replacements.push_back(unknown_metadata_placeholder);

    ft_string unknown_expected = menu_localize_format("main_menu.resume.active_description",
        "Jump back into \"{{slot}}\"{{metadata}} without opening the load menu.", unknown_replacements);
    ft_string metadata_hint
        = menu_localize("main_menu.resume.metadata_unavailable", " Metadata details unavailable.");
    unknown_expected.append(metadata_hint);
    FT_ASSERT_EQ(unknown_expected, items[resume_index].description);

    return 1;
}

int verify_load_resume_menu_flow_integration()
{
    menu_localization_reset_for_testing();

    long timestamp = ft_time_ms();
    ft_string commander("MenuFlowIntegration_");
    commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));

    PlayerProfilePreferences preferences;
    preferences.commander_name = commander;
    FT_ASSERT(player_profile_save(preferences));

    ft_string error;
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("alpha"), error));
    FT_ASSERT(error.empty());
    time_sleep_ms(25);
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("beta"), error));
    FT_ASSERT(error.empty());
    time_sleep_ms(25);
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("corrupt"), error));
    FT_ASSERT(error.empty());

    ft_string alpha_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("alpha"));
    FT_ASSERT(!alpha_path.empty());
    ft_ofstream alpha_stream;
    FT_ASSERT_EQ(0, alpha_stream.open(alpha_path.c_str()));
    ft_string alpha_contents("{\n");
    alpha_contents.append("  \"metadata\": {\"version\": 1, \"save_type\": \"quicksave\"},\n");
    alpha_contents.append("  \"player\": {\"commander_name\": \"Flow Tester\", \"starting_planet_id\": 1, \"commander_level\": 6},\n");
    alpha_contents.append("  \"campaign\": {\"day\": 5, \"difficulty\": 2}\n");
    alpha_contents.append("}\n");
    FT_ASSERT(alpha_stream.write(alpha_contents.c_str()) >= 0);
    alpha_stream.close();

    ft_string beta_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("beta"));
    FT_ASSERT(!beta_path.empty());
    ft_ofstream beta_stream;
    FT_ASSERT_EQ(0, beta_stream.open(beta_path.c_str()));
    ft_string beta_contents("{\n");
    beta_contents.append("  \"metadata\": {\"version\": 1, \"save_type\": \"quicksave\"},\n");
    beta_contents.append("  \"player\": {\"commander_name\": \"Flow Tester\", \"starting_planet_id\": 1, \"commander_level\": 9},\n");
    beta_contents.append("  \"campaign\": {\"day\": 11, \"difficulty\": 1}\n");
    beta_contents.append("}\n");
    FT_ASSERT(beta_stream.write(beta_contents.c_str()) >= 0);
    beta_stream.close();

    ft_string corrupt_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("corrupt"));
    FT_ASSERT(!corrupt_path.empty());
    ft_ofstream corrupt_stream;
    FT_ASSERT_EQ(0, corrupt_stream.open(corrupt_path.c_str()));
    FT_ASSERT(corrupt_stream.write("{\"metadata\":\n") >= 0);
    corrupt_stream.close();

    ft_vector<ft_string> slot_names = load_game_flow_testing::collect_save_slot_names(commander);
    FT_ASSERT_EQ(3U, slot_names.size());
    FT_ASSERT_EQ(ft_string("alpha"), slot_names[0]);
    FT_ASSERT_EQ(ft_string("beta"), slot_names[1]);
    FT_ASSERT_EQ(ft_string("corrupt"), slot_names[2]);

    ft_vector<ft_string> metadata_labels = load_game_flow_testing::collect_save_slot_metadata_labels(commander);
    FT_ASSERT_EQ(3U, metadata_labels.size());
    ft_string expected_standard = build_expected_metadata_label(
        5, 6, "load_menu.metadata.difficulty.standard", "Standard");
    FT_ASSERT_EQ(expected_standard, metadata_labels[0]);
    ft_string expected_easy = build_expected_metadata_label(
        11, 9, "load_menu.metadata.difficulty.easy", "Easy");
    FT_ASSERT_EQ(expected_easy, metadata_labels[1]);
    FT_ASSERT_EQ(menu_localize("load_menu.metadata.unavailable", "Metadata unavailable"), metadata_labels[2]);

    ft_vector<ft_string> slot_errors = load_game_flow_testing::collect_save_slot_errors(commander);
    FT_ASSERT_EQ(1U, slot_errors.size());
    FT_ASSERT(ft_strstr(slot_errors[0].c_str(), "corrupt") != ft_nullptr);

    ft_string slot_name;
    ft_string save_path;
    ft_string resume_metadata;
    bool      metadata_known = false;
    FT_ASSERT(resolve_latest_resume_slot(commander, slot_name, save_path, resume_metadata, metadata_known));
    FT_ASSERT_EQ(ft_string("beta"), slot_name);
    FT_ASSERT_EQ(beta_path, save_path);
    FT_ASSERT(metadata_known);
    FT_ASSERT_EQ(expected_easy, resume_metadata);

    ft_vector<ft_menu_item> menu_items = build_main_menu_items();
    main_menu_apply_resume_state(menu_items, true, slot_name, resume_metadata, metadata_known);
    size_t resume_index = menu_items.size();
    for (size_t index = 0; index < menu_items.size(); ++index)
    {
        if (menu_items[index].identifier == ft_string("resume"))
        {
            resume_index = index;
            break;
        }
    }
    FT_ASSERT(resume_index < menu_items.size());
    FT_ASSERT(menu_items[resume_index].enabled);
    FT_ASSERT_EQ(main_menu_format_resume_description(slot_name, resume_metadata, metadata_known),
        menu_items[resume_index].description);

    FT_ASSERT(player_profile_delete(commander));

    return 1;
}
