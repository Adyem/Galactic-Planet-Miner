bool player_profile_save(const PlayerProfilePreferences &preferences) noexcept
{
    if (preferences.commander_name.empty())
    {
        ft_errno = FT_EINVAL;
        PROFILE_DEBUG_PRINT("[Profile] Cannot save preferences without a commander name.\n");
        return false;
    }
    if (!ensure_profile_directory_exists())
    {
        PROFILE_DEBUG_PRINT("[Profile] Failed to prepare profile directories for \"%s\".\n", preferences.commander_name.c_str());
        return false;
    }
    if (!ensure_profile_save_directory_exists(preferences.commander_name))
    {
        PROFILE_DEBUG_PRINT("[Profile] Failed to prepare save directory for \"%s\".\n", preferences.commander_name.c_str());
        return false;
    }

    ft_string path = player_profile_resolve_path(preferences.commander_name);
    if (path.empty())
    {
        ft_errno = FT_EINVAL;
        PROFILE_DEBUG_PRINT("[Profile] Could not resolve profile path for \"%s\".\n", preferences.commander_name.c_str());
        return false;
    }

    json_document document;
    json_group *group = document.create_group(kProfileGroupName);
    if (group == ft_nullptr)
    {
        log_profile_document_error("Creating profile group", document, path);
        return false;
    }
    document.append_group(group);

    if (!add_string(document, group, "commander_name", preferences.commander_name))
    {
        log_profile_document_error("Adding commander name", document, path);
        return false;
    }
    const unsigned int stored_width = preferences.window_width == 0U ? 1280U : preferences.window_width;
    const unsigned int stored_height = preferences.window_height == 0U ? 720U : preferences.window_height;
    if (!add_int(document, group, "window_width", static_cast<int>(stored_width)))
    {
        log_profile_document_error("Adding window width", document, path);
        return false;
    }
    if (!add_int(document, group, "window_height", static_cast<int>(stored_height)))
    {
        log_profile_document_error("Adding window height", document, path);
        return false;
    }

    unsigned int stored_ui_scale = preferences.ui_scale_percent == 0U ? 100U : preferences.ui_scale_percent;
    stored_ui_scale = clamp_unsigned(stored_ui_scale, PLAYER_PROFILE_UI_SCALE_MIN_PERCENT, PLAYER_PROFILE_UI_SCALE_MAX_PERCENT);
    if (!add_int(document, group, "ui_scale_percent", static_cast<int>(stored_ui_scale)))
    {
        log_profile_document_error("Adding UI scale", document, path);
        return false;
    }

    unsigned int stored_combat_speed = preferences.combat_speed_percent == 0U ? 100U : preferences.combat_speed_percent;
    stored_combat_speed = clamp_unsigned(
        stored_combat_speed, PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT, PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT);
    if (!add_int(document, group, "combat_speed_percent", static_cast<int>(stored_combat_speed)))
    {
        log_profile_document_error("Adding combat speed", document, path);
        return false;
    }

    unsigned int stored_music_volume = clamp_unsigned(
        preferences.music_volume_percent, PLAYER_PROFILE_VOLUME_MIN_PERCENT, PLAYER_PROFILE_VOLUME_MAX_PERCENT);
    if (!add_int(document, group, "music_volume_percent", static_cast<int>(stored_music_volume)))
    {
        log_profile_document_error("Adding music volume", document, path);
        return false;
    }

    unsigned int stored_effects_volume = clamp_unsigned(
        preferences.effects_volume_percent, PLAYER_PROFILE_VOLUME_MIN_PERCENT, PLAYER_PROFILE_VOLUME_MAX_PERCENT);
    if (!add_int(document, group, "effects_volume_percent", static_cast<int>(stored_effects_volume)))
    {
        log_profile_document_error("Adding effects volume", document, path);
        return false;
    }

    unsigned int stored_brightness = clamp_unsigned(preferences.brightness_percent, PLAYER_PROFILE_BRIGHTNESS_MIN_PERCENT,
        PLAYER_PROFILE_BRIGHTNESS_MAX_PERCENT);
    if (!add_int(document, group, "brightness_percent", static_cast<int>(stored_brightness)))
    {
        log_profile_document_error("Adding brightness", document, path);
        return false;
    }

    unsigned int stored_contrast = clamp_unsigned(preferences.contrast_percent, PLAYER_PROFILE_CONTRAST_MIN_PERCENT,
        PLAYER_PROFILE_CONTRAST_MAX_PERCENT);
    if (!add_int(document, group, "contrast_percent", static_cast<int>(stored_contrast)))
    {
        log_profile_document_error("Adding contrast", document, path);
        return false;
    }

    unsigned int stored_anchor = normalize_lore_panel_anchor(preferences.lore_panel_anchor);
    if (!add_int(document, group, "lore_panel_anchor", static_cast<int>(stored_anchor)))
    {
        log_profile_document_error("Adding lore panel anchor", document, path);
        return false;
    }

    ft_string tutorial_value(preferences.menu_tutorial_seen ? "true" : "false");
    if (!add_string(document, group, "menu_tutorial_seen", tutorial_value))
    {
        log_profile_document_error("Adding menu tutorial flag", document, path);
        return false;
    }

    ft_string accessibility_value(preferences.accessibility_preset_enabled ? "true" : "false");
    if (!add_string(document, group, "accessibility_preset_enabled", accessibility_value))
    {
        log_profile_document_error("Adding accessibility preset flag", document, path);
        return false;
    }

    ft_string colorblind_value(preferences.colorblind_palette_enabled ? "true" : "false");
    if (!add_string(document, group, "colorblind_palette_enabled", colorblind_value))
    {
        log_profile_document_error("Adding colorblind palette flag", document, path);
        return false;
    }

    ft_string experimental_value(preferences.experimental_features_enabled ? "true" : "false");
    if (!add_string(document, group, "experimental_features_enabled", experimental_value))
    {
        log_profile_document_error("Adding experimental features flag", document, path);
        return false;
    }

    ft_string analytics_value(preferences.analytics_opt_in ? "true" : "false");
    if (!add_string(document, group, "analytics_opt_in", analytics_value))
    {
        log_profile_document_error("Adding analytics opt-in flag", document, path);
        return false;
    }

    unsigned int stored_achievements_total
        = clamp_unsigned(preferences.achievements_total_count, 0U, PLAYER_PROFILE_ACHIEVEMENTS_MAX_COUNT);
    if (!add_int(document, group, "achievements_total_count", static_cast<int>(stored_achievements_total)))
    {
        log_profile_document_error("Adding achievements total", document, path);
        return false;
    }

    unsigned int stored_achievements_completed
        = clamp_unsigned(preferences.achievements_completed_count, 0U, PLAYER_PROFILE_ACHIEVEMENTS_MAX_COUNT);
    if (stored_achievements_completed > stored_achievements_total)
        stored_achievements_completed = stored_achievements_total;
    if (!add_int(document, group, "achievements_completed_count", static_cast<int>(stored_achievements_completed)))
    {
        log_profile_document_error("Adding achievements completed", document, path);
        return false;
    }

    if (!add_string(document, group, "achievements_highlight_label", preferences.achievements_highlight_label))
    {
        log_profile_document_error("Adding achievements highlight", document, path);
        return false;
    }

    if (!add_string(document, group, "achievements_progress_note", preferences.achievements_progress_note))
    {
        log_profile_document_error("Adding achievements note", document, path);
        return false;
    }

    if (!add_int(document, group, "hotkey_menu_up", preferences.hotkey_menu_up))
    {
        log_profile_document_error("Adding menu hotkey (up)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_down", preferences.hotkey_menu_down))
    {
        log_profile_document_error("Adding menu hotkey (down)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_left", preferences.hotkey_menu_left))
    {
        log_profile_document_error("Adding menu hotkey (left)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_right", preferences.hotkey_menu_right))
    {
        log_profile_document_error("Adding menu hotkey (right)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_confirm", preferences.hotkey_menu_confirm))
    {
        log_profile_document_error("Adding menu hotkey (confirm)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_cancel", preferences.hotkey_menu_cancel))
    {
        log_profile_document_error("Adding menu hotkey (cancel)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_delete", preferences.hotkey_menu_delete))
    {
        log_profile_document_error("Adding menu hotkey (delete)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_rename", preferences.hotkey_menu_rename))
    {
        log_profile_document_error("Adding menu hotkey (rename)", document, path);
        return false;
    }

    const int stored_controller_up
        = normalize_controller_button(static_cast<unsigned int>(preferences.controller_menu_up),
            PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_UP);
    const int stored_controller_down
        = normalize_controller_button(static_cast<unsigned int>(preferences.controller_menu_down),
            PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_DOWN);
    const int stored_controller_left
        = normalize_controller_button(static_cast<unsigned int>(preferences.controller_menu_left),
            PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_LEFT);
    const int stored_controller_right
        = normalize_controller_button(static_cast<unsigned int>(preferences.controller_menu_right),
            PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_RIGHT);
    const int stored_controller_confirm
        = normalize_controller_button(static_cast<unsigned int>(preferences.controller_menu_confirm),
            PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_CONFIRM);
    const int stored_controller_cancel
        = normalize_controller_button(static_cast<unsigned int>(preferences.controller_menu_cancel),
            PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_CANCEL);
    const int stored_controller_delete
        = normalize_controller_button(static_cast<unsigned int>(preferences.controller_menu_delete),
            PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_DELETE);
    const int stored_controller_rename
        = normalize_controller_button(static_cast<unsigned int>(preferences.controller_menu_rename),
            PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_RENAME);

    if (!add_int(document, group, "controller_menu_up", stored_controller_up))
    {
        log_profile_document_error("Adding controller button (up)", document, path);
        return false;
    }
    if (!add_int(document, group, "controller_menu_down", stored_controller_down))
    {
        log_profile_document_error("Adding controller button (down)", document, path);
        return false;
    }
    if (!add_int(document, group, "controller_menu_left", stored_controller_left))
    {
        log_profile_document_error("Adding controller button (left)", document, path);
        return false;
    }
    if (!add_int(document, group, "controller_menu_right", stored_controller_right))
    {
        log_profile_document_error("Adding controller button (right)", document, path);
        return false;
    }
    if (!add_int(document, group, "controller_menu_confirm", stored_controller_confirm))
    {
        log_profile_document_error("Adding controller button (confirm)", document, path);
        return false;
    }
    if (!add_int(document, group, "controller_menu_cancel", stored_controller_cancel))
    {
        log_profile_document_error("Adding controller button (cancel)", document, path);
        return false;
    }
    if (!add_int(document, group, "controller_menu_delete", stored_controller_delete))
    {
        log_profile_document_error("Adding controller button (delete)", document, path);
        return false;
    }
    if (!add_int(document, group, "controller_menu_rename", stored_controller_rename))
    {
        log_profile_document_error("Adding controller button (rename)", document, path);
        return false;
    }

    int stored_device
        = normalize_menu_input_device(static_cast<unsigned int>(preferences.last_menu_input_device));
    if (!add_int(document, group, "last_menu_input_device", stored_device))
    {
        log_profile_document_error("Adding last input device", document, path);
        return false;
    }

    if (document.write_to_file(path.c_str()) != 0)
    {
        log_profile_document_error("Writing profile", document, path);
        return false;
    }
    return true;
}
