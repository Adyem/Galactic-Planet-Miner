int verify_player_profile_save()
{
    const ft_string commander_name("Cmdr/Test:Save?");

    PlayerProfilePreferences preferences;
    preferences.commander_name = commander_name;
    preferences.window_width = 1920U;
    preferences.window_height = 1080U;
    preferences.ui_scale_percent = 125U;
    preferences.combat_speed_percent = 140U;
    preferences.lore_panel_anchor = PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT;
    preferences.menu_tutorial_seen = true;
    preferences.last_menu_input_device = PLAYER_PROFILE_INPUT_DEVICE_GAMEPAD;
    preferences.achievements_total_count = 24U;
    preferences.achievements_completed_count = 6U;
    preferences.achievements_highlight_label = ft_string("Research Pioneer");
    preferences.achievements_progress_note = ft_string("Deliver one more convoy.");

    FT_ASSERT(player_profile_delete(commander_name));

    FT_ASSERT(player_profile_save(preferences));

    ft_string profile_path = player_profile_resolve_path(commander_name);
    FT_ASSERT(!profile_path.empty());

    json_document document;
    FT_ASSERT_EQ(0, document.read_from_file(profile_path.c_str()));

    json_group *group = document.find_group("profile");
    FT_ASSERT(group != ft_nullptr);

    json_item *name_item = document.find_item(group, "commander_name");
    FT_ASSERT(name_item != ft_nullptr);
    FT_ASSERT(name_item->value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(name_item->value, commander_name.c_str()));

    json_item *width_item = document.find_item(group, "window_width");
    FT_ASSERT(width_item != ft_nullptr);
    FT_ASSERT(width_item->value != ft_nullptr);
    FT_ASSERT_EQ(1920, ft_atoi(width_item->value));

    json_item *height_item = document.find_item(group, "window_height");
    FT_ASSERT(height_item != ft_nullptr);
    FT_ASSERT(height_item->value != ft_nullptr);
    FT_ASSERT_EQ(1080, ft_atoi(height_item->value));

    json_item *ui_scale_item = document.find_item(group, "ui_scale_percent");
    FT_ASSERT(ui_scale_item != ft_nullptr);
    FT_ASSERT(ui_scale_item->value != ft_nullptr);
    FT_ASSERT_EQ(125, ft_atoi(ui_scale_item->value));

    json_item *combat_speed_item = document.find_item(group, "combat_speed_percent");
    FT_ASSERT(combat_speed_item != ft_nullptr);
    FT_ASSERT(combat_speed_item->value != ft_nullptr);
    FT_ASSERT_EQ(140, ft_atoi(combat_speed_item->value));

    json_item *anchor_item = document.find_item(group, "lore_panel_anchor");
    FT_ASSERT(anchor_item != ft_nullptr);
    FT_ASSERT(anchor_item->value != ft_nullptr);
    FT_ASSERT_EQ(PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT, ft_atoi(anchor_item->value));

    json_item *tutorial_item = document.find_item(group, "menu_tutorial_seen");
    FT_ASSERT(tutorial_item != ft_nullptr);
    FT_ASSERT(tutorial_item->value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(tutorial_item->value, "true"));

    json_item *device_item = document.find_item(group, "last_menu_input_device");
    FT_ASSERT(device_item != ft_nullptr);
    FT_ASSERT(device_item->value != ft_nullptr);
    FT_ASSERT_EQ(PLAYER_PROFILE_INPUT_DEVICE_GAMEPAD, ft_atoi(device_item->value));

    json_item *achievement_total_item = document.find_item(group, "achievements_total_count");
    FT_ASSERT(achievement_total_item != ft_nullptr);
    FT_ASSERT(achievement_total_item->value != ft_nullptr);
    FT_ASSERT_EQ(24, ft_atoi(achievement_total_item->value));

    json_item *achievement_completed_item = document.find_item(group, "achievements_completed_count");
    FT_ASSERT(achievement_completed_item != ft_nullptr);
    FT_ASSERT(achievement_completed_item->value != ft_nullptr);
    FT_ASSERT_EQ(6, ft_atoi(achievement_completed_item->value));

    json_item *achievement_highlight_item = document.find_item(group, "achievements_highlight_label");
    FT_ASSERT(achievement_highlight_item != ft_nullptr);
    FT_ASSERT(achievement_highlight_item->value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(achievement_highlight_item->value, "Research Pioneer"));

    json_item *achievement_note_item = document.find_item(group, "achievements_progress_note");
    FT_ASSERT(achievement_note_item != ft_nullptr);
    FT_ASSERT(achievement_note_item->value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp(achievement_note_item->value, "Deliver one more convoy."));

    PlayerProfilePreferences loaded;
    FT_ASSERT(player_profile_load_or_create(loaded, commander_name));
    FT_ASSERT_EQ(preferences.commander_name, loaded.commander_name);
    FT_ASSERT_EQ(preferences.window_width, loaded.window_width);
    FT_ASSERT_EQ(preferences.window_height, loaded.window_height);
    FT_ASSERT_EQ(preferences.ui_scale_percent, loaded.ui_scale_percent);
    FT_ASSERT_EQ(preferences.combat_speed_percent, loaded.combat_speed_percent);
    FT_ASSERT_EQ(preferences.lore_panel_anchor, loaded.lore_panel_anchor);
    FT_ASSERT(loaded.menu_tutorial_seen);
    FT_ASSERT_EQ(preferences.last_menu_input_device, loaded.last_menu_input_device);
    FT_ASSERT_EQ(preferences.achievements_total_count, loaded.achievements_total_count);
    FT_ASSERT_EQ(preferences.achievements_completed_count, loaded.achievements_completed_count);
    FT_ASSERT_EQ(preferences.achievements_highlight_label, loaded.achievements_highlight_label);
    FT_ASSERT_EQ(preferences.achievements_progress_note, loaded.achievements_progress_note);

    FT_ASSERT(player_profile_delete(commander_name));

    return 1;
}
