bool player_profile_load_or_create(PlayerProfilePreferences &out_preferences, const ft_string &commander_name) noexcept
{
    out_preferences = PlayerProfilePreferences();
    out_preferences.commander_name = commander_name;

    if (commander_name.empty())
    {
        ft_errno = FT_EINVAL;
        PROFILE_DEBUG_PRINT("[Profile] Cannot load preferences for an empty commander name.\n");
        return false;
    }
    if (!ensure_profile_directory_exists())
    {
        PROFILE_DEBUG_PRINT("[Profile] Failed to prepare base directory for \"%s\".\n", commander_name.c_str());
        return false;
    }

    ft_string path = player_profile_resolve_path(commander_name);
    if (path.empty())
    {
        ft_errno = FT_EINVAL;
        PROFILE_DEBUG_PRINT("[Profile] Could not resolve profile path for \"%s\".\n", commander_name.c_str());
        return false;
    }

    json_document document;
    if (document.read_from_file(path.c_str()) != 0)
    {
        log_profile_document_error("Reading profile", document, path);
        return player_profile_save(out_preferences);
    }

    json_group *group = document.find_group(kProfileGroupName);
    if (group == ft_nullptr)
    {
        PROFILE_DEBUG_PRINT("[Profile] Profile group missing in \"%s\". Recreating defaults.\n", path.c_str());
        return player_profile_save(out_preferences);
    }

    json_item *name_item = document.find_item(group, "commander_name");
    if (name_item != ft_nullptr && name_item->value != ft_nullptr)
        out_preferences.commander_name = ft_string(name_item->value);

    unsigned int parsed_width = out_preferences.window_width;
    unsigned int parsed_height = out_preferences.window_height;
    read_int(document, group, "window_width", parsed_width);
    read_int(document, group, "window_height", parsed_height);

    out_preferences.window_width = parsed_width;
    out_preferences.window_height = parsed_height;
    unsigned int parsed_ui_scale = out_preferences.ui_scale_percent;
    unsigned int parsed_combat_speed = out_preferences.combat_speed_percent;
    unsigned int parsed_music_volume = out_preferences.music_volume_percent;
    unsigned int parsed_effects_volume = out_preferences.effects_volume_percent;
    unsigned int parsed_anchor = out_preferences.lore_panel_anchor;
    bool         parsed_tutorial_seen = out_preferences.menu_tutorial_seen;
    bool         parsed_accessibility_enabled = out_preferences.accessibility_preset_enabled;
    bool         parsed_colorblind_enabled = out_preferences.colorblind_palette_enabled;
    bool         parsed_experimental_enabled = out_preferences.experimental_features_enabled;
    bool         parsed_analytics_opt_in = out_preferences.analytics_opt_in;
    unsigned int parsed_brightness = out_preferences.brightness_percent;
    unsigned int parsed_contrast = out_preferences.contrast_percent;
    unsigned int parsed_achievements_total = out_preferences.achievements_total_count;
    unsigned int parsed_achievements_completed = out_preferences.achievements_completed_count;
    ft_string    parsed_achievements_highlight = out_preferences.achievements_highlight_label;
    ft_string    parsed_achievements_note = out_preferences.achievements_progress_note;
    unsigned int parsed_menu_up = static_cast<unsigned int>(out_preferences.hotkey_menu_up);
    unsigned int parsed_menu_down = static_cast<unsigned int>(out_preferences.hotkey_menu_down);
    unsigned int parsed_menu_left = static_cast<unsigned int>(out_preferences.hotkey_menu_left);
    unsigned int parsed_menu_right = static_cast<unsigned int>(out_preferences.hotkey_menu_right);
    unsigned int parsed_menu_confirm = static_cast<unsigned int>(out_preferences.hotkey_menu_confirm);
    unsigned int parsed_menu_cancel = static_cast<unsigned int>(out_preferences.hotkey_menu_cancel);
    unsigned int parsed_menu_delete = static_cast<unsigned int>(out_preferences.hotkey_menu_delete);
    unsigned int parsed_menu_rename = static_cast<unsigned int>(out_preferences.hotkey_menu_rename);
    unsigned int parsed_controller_up = static_cast<unsigned int>(out_preferences.controller_menu_up);
    unsigned int parsed_controller_down = static_cast<unsigned int>(out_preferences.controller_menu_down);
    unsigned int parsed_controller_left = static_cast<unsigned int>(out_preferences.controller_menu_left);
    unsigned int parsed_controller_right = static_cast<unsigned int>(out_preferences.controller_menu_right);
    unsigned int parsed_controller_confirm = static_cast<unsigned int>(out_preferences.controller_menu_confirm);
    unsigned int parsed_controller_cancel = static_cast<unsigned int>(out_preferences.controller_menu_cancel);
    unsigned int parsed_controller_delete = static_cast<unsigned int>(out_preferences.controller_menu_delete);
    unsigned int parsed_controller_rename = static_cast<unsigned int>(out_preferences.controller_menu_rename);
    unsigned int parsed_input_device = static_cast<unsigned int>(out_preferences.last_menu_input_device);

    read_int(document, group, "ui_scale_percent", parsed_ui_scale);
    read_int(document, group, "combat_speed_percent", parsed_combat_speed);
    read_int(document, group, "music_volume_percent", parsed_music_volume);
    read_int(document, group, "effects_volume_percent", parsed_effects_volume);
    read_int(document, group, "brightness_percent", parsed_brightness);
    read_int(document, group, "contrast_percent", parsed_contrast);
    read_int(document, group, "lore_panel_anchor", parsed_anchor);
    read_int(document, group, "hotkey_menu_up", parsed_menu_up);
    read_int(document, group, "hotkey_menu_down", parsed_menu_down);
    read_int(document, group, "hotkey_menu_left", parsed_menu_left);
    read_int(document, group, "hotkey_menu_right", parsed_menu_right);
    read_int(document, group, "hotkey_menu_confirm", parsed_menu_confirm);
    read_int(document, group, "hotkey_menu_cancel", parsed_menu_cancel);
    read_int(document, group, "hotkey_menu_delete", parsed_menu_delete);
    read_int(document, group, "hotkey_menu_rename", parsed_menu_rename);
    read_int(document, group, "controller_menu_up", parsed_controller_up);
    read_int(document, group, "controller_menu_down", parsed_controller_down);
    read_int(document, group, "controller_menu_left", parsed_controller_left);
    read_int(document, group, "controller_menu_right", parsed_controller_right);
    read_int(document, group, "controller_menu_confirm", parsed_controller_confirm);
    read_int(document, group, "controller_menu_cancel", parsed_controller_cancel);
    read_int(document, group, "controller_menu_delete", parsed_controller_delete);
    read_int(document, group, "controller_menu_rename", parsed_controller_rename);
    read_int(document, group, "last_menu_input_device", parsed_input_device);

    if (parsed_ui_scale == 0U)
        parsed_ui_scale = 100U;
    if (parsed_combat_speed == 0U)
        parsed_combat_speed = 100U;

    out_preferences.ui_scale_percent = clamp_unsigned(parsed_ui_scale, PLAYER_PROFILE_UI_SCALE_MIN_PERCENT, PLAYER_PROFILE_UI_SCALE_MAX_PERCENT);
    out_preferences.combat_speed_percent = clamp_unsigned(parsed_combat_speed, PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT, PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT);
    out_preferences.music_volume_percent = clamp_unsigned(
        parsed_music_volume, PLAYER_PROFILE_VOLUME_MIN_PERCENT, PLAYER_PROFILE_VOLUME_MAX_PERCENT);
    out_preferences.effects_volume_percent = clamp_unsigned(
        parsed_effects_volume, PLAYER_PROFILE_VOLUME_MIN_PERCENT, PLAYER_PROFILE_VOLUME_MAX_PERCENT);
    out_preferences.brightness_percent = clamp_unsigned(
        parsed_brightness, PLAYER_PROFILE_BRIGHTNESS_MIN_PERCENT, PLAYER_PROFILE_BRIGHTNESS_MAX_PERCENT);
    out_preferences.contrast_percent = clamp_unsigned(
        parsed_contrast, PLAYER_PROFILE_CONTRAST_MIN_PERCENT, PLAYER_PROFILE_CONTRAST_MAX_PERCENT);
    out_preferences.lore_panel_anchor = normalize_lore_panel_anchor(parsed_anchor);
    out_preferences.hotkey_menu_up
        = parsed_menu_up == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_UP : static_cast<int>(parsed_menu_up);
    out_preferences.hotkey_menu_down
        = parsed_menu_down == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DOWN : static_cast<int>(parsed_menu_down);
    out_preferences.hotkey_menu_left
        = parsed_menu_left == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_LEFT : static_cast<int>(parsed_menu_left);
    out_preferences.hotkey_menu_right
        = parsed_menu_right == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_RIGHT : static_cast<int>(parsed_menu_right);
    out_preferences.hotkey_menu_confirm
        = parsed_menu_confirm == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CONFIRM : static_cast<int>(parsed_menu_confirm);
    out_preferences.hotkey_menu_cancel
        = parsed_menu_cancel == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CANCEL : static_cast<int>(parsed_menu_cancel);
    out_preferences.hotkey_menu_delete
        = parsed_menu_delete == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DELETE : static_cast<int>(parsed_menu_delete);
    out_preferences.hotkey_menu_rename
        = parsed_menu_rename == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_RENAME : static_cast<int>(parsed_menu_rename);
    out_preferences.controller_menu_up
        = normalize_controller_button(parsed_controller_up, PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_UP);
    out_preferences.controller_menu_down
        = normalize_controller_button(parsed_controller_down, PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_DOWN);
    out_preferences.controller_menu_left
        = normalize_controller_button(parsed_controller_left, PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_LEFT);
    out_preferences.controller_menu_right
        = normalize_controller_button(parsed_controller_right, PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_RIGHT);
    out_preferences.controller_menu_confirm
        = normalize_controller_button(parsed_controller_confirm, PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_CONFIRM);
    out_preferences.controller_menu_cancel
        = normalize_controller_button(parsed_controller_cancel, PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_CANCEL);
    out_preferences.controller_menu_delete
        = normalize_controller_button(parsed_controller_delete, PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_DELETE);
    out_preferences.controller_menu_rename
        = normalize_controller_button(parsed_controller_rename, PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_RENAME);
    out_preferences.last_menu_input_device = normalize_menu_input_device(parsed_input_device);

    json_item *achievements_total_item = document.find_item(group, "achievements_total_count");
    if (achievements_total_item != ft_nullptr && achievements_total_item->value != ft_nullptr)
    {
        long parsed_value = ft_atol(achievements_total_item->value);
        if (parsed_value < 0L)
            parsed_value = 0L;
        if (parsed_value > static_cast<long>(PLAYER_PROFILE_ACHIEVEMENTS_MAX_COUNT))
            parsed_value = static_cast<long>(PLAYER_PROFILE_ACHIEVEMENTS_MAX_COUNT);
        parsed_achievements_total = static_cast<unsigned int>(parsed_value);
    }

    json_item *achievements_completed_item = document.find_item(group, "achievements_completed_count");
    if (achievements_completed_item != ft_nullptr && achievements_completed_item->value != ft_nullptr)
    {
        long parsed_value = ft_atol(achievements_completed_item->value);
        if (parsed_value < 0L)
            parsed_value = 0L;
        if (parsed_value > static_cast<long>(PLAYER_PROFILE_ACHIEVEMENTS_MAX_COUNT))
            parsed_value = static_cast<long>(PLAYER_PROFILE_ACHIEVEMENTS_MAX_COUNT);
        parsed_achievements_completed = static_cast<unsigned int>(parsed_value);
    }

    json_item *achievements_highlight_item = document.find_item(group, "achievements_highlight_label");
    if (achievements_highlight_item != ft_nullptr && achievements_highlight_item->value != ft_nullptr)
        parsed_achievements_highlight = ft_string(achievements_highlight_item->value);

    json_item *achievements_note_item = document.find_item(group, "achievements_progress_note");
    if (achievements_note_item != ft_nullptr && achievements_note_item->value != ft_nullptr)
        parsed_achievements_note = ft_string(achievements_note_item->value);

    json_item *tutorial_item = document.find_item(group, "menu_tutorial_seen");
    if (tutorial_item != ft_nullptr && tutorial_item->value != ft_nullptr)
    {
        ft_string normalized(tutorial_item->value);
        ft_to_lower(normalized.print());
        const char *normalized_value = normalized.c_str();
        if (ft_strcmp(normalized_value, "true") == 0 || ft_strcmp(normalized_value, "1") == 0
            || ft_strcmp(normalized_value, "yes") == 0)
            parsed_tutorial_seen = true;
        else if (ft_strcmp(normalized_value, "false") == 0 || ft_strcmp(normalized_value, "0") == 0
            || ft_strcmp(normalized_value, "no") == 0)
            parsed_tutorial_seen = false;
    }
    out_preferences.menu_tutorial_seen = parsed_tutorial_seen;

    json_item *accessibility_item = document.find_item(group, "accessibility_preset_enabled");
    if (accessibility_item != ft_nullptr && accessibility_item->value != ft_nullptr)
    {
        ft_string normalized(accessibility_item->value);
        ft_to_lower(normalized.print());
        const char *normalized_value = normalized.c_str();
        if (ft_strcmp(normalized_value, "true") == 0 || ft_strcmp(normalized_value, "1") == 0
            || ft_strcmp(normalized_value, "yes") == 0 || ft_strcmp(normalized_value, "on") == 0)
            parsed_accessibility_enabled = true;
        else if (ft_strcmp(normalized_value, "false") == 0 || ft_strcmp(normalized_value, "0") == 0
            || ft_strcmp(normalized_value, "no") == 0 || ft_strcmp(normalized_value, "off") == 0)
            parsed_accessibility_enabled = false;
    }
    out_preferences.accessibility_preset_enabled = parsed_accessibility_enabled;

    json_item *colorblind_item = document.find_item(group, "colorblind_palette_enabled");
    if (colorblind_item != ft_nullptr && colorblind_item->value != ft_nullptr)
    {
        ft_string normalized(colorblind_item->value);
        ft_to_lower(normalized.print());
        const char *normalized_value = normalized.c_str();
        if (ft_strcmp(normalized_value, "true") == 0 || ft_strcmp(normalized_value, "1") == 0
            || ft_strcmp(normalized_value, "yes") == 0 || ft_strcmp(normalized_value, "on") == 0)
            parsed_colorblind_enabled = true;
        else if (ft_strcmp(normalized_value, "false") == 0 || ft_strcmp(normalized_value, "0") == 0
            || ft_strcmp(normalized_value, "no") == 0 || ft_strcmp(normalized_value, "off") == 0)
            parsed_colorblind_enabled = false;
    }
    out_preferences.colorblind_palette_enabled = parsed_colorblind_enabled;

    json_item *experimental_item = document.find_item(group, "experimental_features_enabled");
    if (experimental_item != ft_nullptr && experimental_item->value != ft_nullptr)
    {
        ft_string normalized(experimental_item->value);
        ft_to_lower(normalized.print());
        const char *normalized_value = normalized.c_str();
        if (ft_strcmp(normalized_value, "true") == 0 || ft_strcmp(normalized_value, "1") == 0
            || ft_strcmp(normalized_value, "yes") == 0 || ft_strcmp(normalized_value, "on") == 0)
            parsed_experimental_enabled = true;
        else if (ft_strcmp(normalized_value, "false") == 0 || ft_strcmp(normalized_value, "0") == 0
            || ft_strcmp(normalized_value, "no") == 0 || ft_strcmp(normalized_value, "off") == 0)
            parsed_experimental_enabled = false;
    }
    out_preferences.experimental_features_enabled = parsed_experimental_enabled;

    json_item *analytics_item = document.find_item(group, "analytics_opt_in");
    if (analytics_item != ft_nullptr && analytics_item->value != ft_nullptr)
    {
        ft_string normalized(analytics_item->value);
        ft_to_lower(normalized.print());
        const char *normalized_value = normalized.c_str();
        if (ft_strcmp(normalized_value, "true") == 0 || ft_strcmp(normalized_value, "1") == 0
            || ft_strcmp(normalized_value, "yes") == 0 || ft_strcmp(normalized_value, "on") == 0)
            parsed_analytics_opt_in = true;
        else if (ft_strcmp(normalized_value, "false") == 0 || ft_strcmp(normalized_value, "0") == 0
            || ft_strcmp(normalized_value, "no") == 0 || ft_strcmp(normalized_value, "off") == 0)
            parsed_analytics_opt_in = false;
    }
    out_preferences.analytics_opt_in = parsed_analytics_opt_in;
    if (parsed_achievements_completed > parsed_achievements_total)
        parsed_achievements_completed = parsed_achievements_total;
    out_preferences.achievements_total_count
        = clamp_unsigned(parsed_achievements_total, 0U, PLAYER_PROFILE_ACHIEVEMENTS_MAX_COUNT);
    out_preferences.achievements_completed_count
        = clamp_unsigned(parsed_achievements_completed, 0U, PLAYER_PROFILE_ACHIEVEMENTS_MAX_COUNT);
    out_preferences.achievements_highlight_label = parsed_achievements_highlight;
    out_preferences.achievements_progress_note = parsed_achievements_note;
    return true;
