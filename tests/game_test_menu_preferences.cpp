#include "game_test_menu_shared.hpp"

int verify_menu_preference_snapshot()
{
    PlayerProfilePreferences defaults = profile_preferences_testing::build_window_preference_snapshot(
        ft_string("Cassiopeia"), 0U, 0U);
    FT_ASSERT_EQ(ft_string("Cassiopeia"), defaults.commander_name);
    FT_ASSERT_EQ(1280U, defaults.window_width);
    FT_ASSERT_EQ(720U, defaults.window_height);
    FT_ASSERT_EQ(100U, defaults.music_volume_percent);
    FT_ASSERT_EQ(100U, defaults.effects_volume_percent);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_UP, defaults.hotkey_menu_up);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DOWN, defaults.hotkey_menu_down);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CONFIRM, defaults.hotkey_menu_confirm);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CANCEL, defaults.hotkey_menu_cancel);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DELETE, defaults.hotkey_menu_delete);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_RENAME, defaults.hotkey_menu_rename);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_UP, defaults.controller_menu_up);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_DOWN, defaults.controller_menu_down);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_LEFT, defaults.controller_menu_left);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_RIGHT, defaults.controller_menu_right);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_CONFIRM, defaults.controller_menu_confirm);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_CANCEL, defaults.controller_menu_cancel);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_DELETE, defaults.controller_menu_delete);
    FT_ASSERT_EQ(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_RENAME, defaults.controller_menu_rename);
    FT_ASSERT_EQ(PLAYER_PROFILE_INPUT_DEVICE_KEYBOARD, defaults.last_menu_input_device);
    FT_ASSERT(!defaults.experimental_features_enabled);
    FT_ASSERT(!defaults.analytics_opt_in);

    PlayerProfilePreferences custom = profile_preferences_testing::build_window_preference_snapshot(
        ft_string("Vesta"), 1920U, 1080U);
    FT_ASSERT_EQ(ft_string("Vesta"), custom.commander_name);
    FT_ASSERT_EQ(1920U, custom.window_width);
    FT_ASSERT_EQ(1080U, custom.window_height);
    FT_ASSERT_EQ(100U, custom.music_volume_percent);
    FT_ASSERT_EQ(100U, custom.effects_volume_percent);
    FT_ASSERT_EQ(PLAYER_PROFILE_INPUT_DEVICE_KEYBOARD, custom.last_menu_input_device);
    FT_ASSERT(!custom.experimental_features_enabled);
    FT_ASSERT(!custom.analytics_opt_in);

    PlayerProfilePreferences mixed = profile_preferences_testing::build_window_preference_snapshot(
        ft_string("Vega"), 2048U, 0U);
    FT_ASSERT_EQ(2048U, mixed.window_width);
    FT_ASSERT_EQ(720U, mixed.window_height);
    FT_ASSERT_EQ(100U, mixed.music_volume_percent);
    FT_ASSERT_EQ(100U, mixed.effects_volume_percent);
    FT_ASSERT_EQ(PLAYER_PROFILE_INPUT_DEVICE_KEYBOARD, mixed.last_menu_input_device);
    FT_ASSERT(!mixed.experimental_features_enabled);
    FT_ASSERT(!mixed.analytics_opt_in);

    return 1;
}

int verify_main_menu_device_preference_seed()
{
    ft_ui_menu menu;
    menu.set_items(build_main_menu_items());
    menu.set_viewport_bounds(build_main_menu_viewport());

    FT_ASSERT_EQ(FT_INPUT_DEVICE_NONE, menu.get_active_device());

    menu.set_active_device(FT_INPUT_DEVICE_MOUSE);
    FT_ASSERT_EQ(FT_INPUT_DEVICE_MOUSE, menu.get_active_device());

    menu.set_active_device(FT_INPUT_DEVICE_GAMEPAD);
    FT_ASSERT_EQ(FT_INPUT_DEVICE_GAMEPAD, menu.get_active_device());

    menu.set_active_device(FT_INPUT_DEVICE_NONE);
    FT_ASSERT_EQ(FT_INPUT_DEVICE_NONE, menu.get_active_device());

    return 1;
}

int verify_settings_flow_helpers()
{
    FT_ASSERT_EQ(75U, settings_flow_testing::clamp_ui_scale(60U));
    FT_ASSERT_EQ(150U, settings_flow_testing::clamp_ui_scale(180U));
    FT_ASSERT_EQ(105U, settings_flow_testing::increment_ui_scale(100U));
    FT_ASSERT_EQ(150U, settings_flow_testing::increment_ui_scale(148U));
    FT_ASSERT_EQ(145U, settings_flow_testing::decrement_ui_scale(150U));
    FT_ASSERT_EQ(75U, settings_flow_testing::decrement_ui_scale(70U));

    FT_ASSERT_EQ(50U, settings_flow_testing::clamp_combat_speed(10U));
    FT_ASSERT_EQ(200U, settings_flow_testing::clamp_combat_speed(240U));
    FT_ASSERT_EQ(155U, settings_flow_testing::increment_combat_speed(150U));
    FT_ASSERT_EQ(200U, settings_flow_testing::increment_combat_speed(198U));
    FT_ASSERT_EQ(145U, settings_flow_testing::decrement_combat_speed(150U));
    FT_ASSERT_EQ(50U, settings_flow_testing::decrement_combat_speed(45U));

    FT_ASSERT_EQ(0U, settings_flow_testing::clamp_music_volume(0U));
    FT_ASSERT_EQ(100U, settings_flow_testing::clamp_music_volume(180U));
    FT_ASSERT_EQ(55U, settings_flow_testing::increment_music_volume(50U));
    FT_ASSERT_EQ(100U, settings_flow_testing::increment_music_volume(100U));
    FT_ASSERT_EQ(45U, settings_flow_testing::decrement_music_volume(50U));
    FT_ASSERT_EQ(0U, settings_flow_testing::decrement_music_volume(3U));

    FT_ASSERT_EQ(0U, settings_flow_testing::clamp_effects_volume(0U));
    FT_ASSERT_EQ(100U, settings_flow_testing::clamp_effects_volume(260U));
    FT_ASSERT_EQ(65U, settings_flow_testing::increment_effects_volume(60U));
    FT_ASSERT_EQ(100U, settings_flow_testing::increment_effects_volume(100U));
    FT_ASSERT_EQ(60U, settings_flow_testing::decrement_effects_volume(65U));
    FT_ASSERT_EQ(0U, settings_flow_testing::decrement_effects_volume(1U));

    FT_ASSERT(settings_flow_testing::toggle_experimental_features(false));
    FT_ASSERT(!settings_flow_testing::toggle_experimental_features(true));
    FT_ASSERT_EQ(ft_string("Experimental Features: Off"),
        settings_flow_testing::format_experimental_features_option(false));
    FT_ASSERT_EQ(ft_string("Experimental Features: On"),
        settings_flow_testing::format_experimental_features_option(true));

    FT_ASSERT(settings_flow_testing::toggle_analytics_opt_in(false));
    FT_ASSERT(!settings_flow_testing::toggle_analytics_opt_in(true));
    FT_ASSERT_EQ(ft_string("Analytics Sharing: Off"),
        settings_flow_testing::format_analytics_opt_in_option(false));
    FT_ASSERT_EQ(ft_string("Analytics Sharing: On"),
        settings_flow_testing::format_analytics_opt_in_option(true));

    FT_ASSERT_EQ(50U, settings_flow_testing::clamp_brightness(20U));
    FT_ASSERT_EQ(150U, settings_flow_testing::clamp_brightness(180U));
    FT_ASSERT_EQ(105U, settings_flow_testing::increment_brightness(100U));
    FT_ASSERT_EQ(150U, settings_flow_testing::increment_brightness(148U));
    FT_ASSERT_EQ(95U, settings_flow_testing::decrement_brightness(100U));
    FT_ASSERT_EQ(50U, settings_flow_testing::decrement_brightness(54U));

    FT_ASSERT_EQ(50U, settings_flow_testing::clamp_contrast(40U));
    FT_ASSERT_EQ(150U, settings_flow_testing::clamp_contrast(180U));
    FT_ASSERT_EQ(135U, settings_flow_testing::increment_contrast(130U));
    FT_ASSERT_EQ(150U, settings_flow_testing::increment_contrast(148U));
    FT_ASSERT_EQ(95U, settings_flow_testing::decrement_contrast(100U));
    FT_ASSERT_EQ(50U, settings_flow_testing::decrement_contrast(48U));

    FT_ASSERT_EQ(PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT,
        settings_flow_testing::toggle_lore_anchor(PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT));
    FT_ASSERT_EQ(PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT,
        settings_flow_testing::toggle_lore_anchor(PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT));
    FT_ASSERT(settings_flow_testing::toggle_accessibility_preset(false));
    FT_ASSERT(!settings_flow_testing::toggle_accessibility_preset(true));
    FT_ASSERT(settings_flow_testing::toggle_colorblind_palette(false));
    FT_ASSERT(!settings_flow_testing::toggle_colorblind_palette(true));

    FT_ASSERT_EQ(ft_string("UI Scale: 110%"), settings_flow_testing::format_ui_scale_option(110U));
    FT_ASSERT_EQ(ft_string("Combat Speed: 95%"), settings_flow_testing::format_combat_speed_option(95U));
    FT_ASSERT_EQ(ft_string("Music Volume: 80%"), settings_flow_testing::format_music_volume_option(80U));
    FT_ASSERT_EQ(ft_string("Effects Volume: 40%"), settings_flow_testing::format_effects_volume_option(40U));
    FT_ASSERT_EQ(ft_string("Brightness: 120%"), settings_flow_testing::format_brightness_option(120U));
    FT_ASSERT_EQ(ft_string("Contrast: 90%"), settings_flow_testing::format_contrast_option(90U));
    FT_ASSERT_EQ(ft_string("Lore Panel Anchor: Right"),
        settings_flow_testing::format_lore_anchor_option(PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT));
    FT_ASSERT_EQ(ft_string("Accessibility Preset: Off"), settings_flow_testing::format_accessibility_preset_option(false));
    FT_ASSERT_EQ(ft_string("Accessibility Preset: On"), settings_flow_testing::format_accessibility_preset_option(true));
    FT_ASSERT_EQ(ft_string("Colorblind Palette: Off"),
        settings_flow_testing::format_colorblind_palette_option(false));
    FT_ASSERT_EQ(ft_string("Colorblind Palette: On"),
        settings_flow_testing::format_colorblind_palette_option(true));
    FT_ASSERT_EQ(ft_string("Controller Up: D-Pad Up"),
        settings_flow_testing::format_controller_up_option(PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_UP));
    FT_ASSERT_EQ(ft_string("Controller Down: D-Pad Down"),
        settings_flow_testing::format_controller_down_option(PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_DOWN));
    FT_ASSERT_EQ(ft_string("Controller Left: D-Pad Left"),
        settings_flow_testing::format_controller_left_option(PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_LEFT));
    FT_ASSERT_EQ(ft_string("Controller Right: D-Pad Right"),
        settings_flow_testing::format_controller_right_option(PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_RIGHT));
    FT_ASSERT_EQ(ft_string("Controller Confirm: A"),
        settings_flow_testing::format_controller_confirm_option(PLAYER_PROFILE_CONTROLLER_BUTTON_A));
    FT_ASSERT_EQ(ft_string("Controller Cancel: B"),
        settings_flow_testing::format_controller_cancel_option(PLAYER_PROFILE_CONTROLLER_BUTTON_B));
    FT_ASSERT_EQ(ft_string("Controller Delete: X"),
        settings_flow_testing::format_controller_delete_option(PLAYER_PROFILE_CONTROLLER_BUTTON_X));
    FT_ASSERT_EQ(ft_string("Controller Rename: Y"),
        settings_flow_testing::format_controller_rename_option(PLAYER_PROFILE_CONTROLLER_BUTTON_Y));
    FT_ASSERT_EQ(PLAYER_PROFILE_CONTROLLER_BUTTON_B,
        settings_flow_testing::increment_controller_button_option(PLAYER_PROFILE_CONTROLLER_BUTTON_A));
    FT_ASSERT_EQ(PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_RIGHT,
        settings_flow_testing::decrement_controller_button_option(PLAYER_PROFILE_CONTROLLER_BUTTON_A));

    return 1;
}

int verify_main_menu_audio_respects_settings()
{
    main_menu_audio_reset();
    main_menu_audio_set_global_mute(false);

    PlayerProfilePreferences preferences;
    preferences.music_volume_percent = 80U;
    preferences.effects_volume_percent = 40U;
    main_menu_audio_apply_preferences(preferences);

    FT_ASSERT_EQ(80U, main_menu_audio_get_music_volume());
    FT_ASSERT_EQ(40U, main_menu_audio_get_effects_volume());

    main_menu_audio_queue_event(MAIN_MENU_AUDIO_CUE_NAVIGATE);

    MainMenuAudioEvent event;
    FT_ASSERT(main_menu_audio_poll_event(event));
    FT_ASSERT_EQ(MAIN_MENU_AUDIO_CUE_NAVIGATE, event.cue);
    FT_ASSERT_EQ(40U, event.volume_percent);
    FT_ASSERT(!main_menu_audio_poll_event(event));

    main_menu_audio_set_global_mute(true);
    main_menu_audio_queue_event(MAIN_MENU_AUDIO_CUE_CONFIRM);
    FT_ASSERT(!main_menu_audio_poll_event(event));

    main_menu_audio_set_global_mute(false);
    main_menu_audio_set_effects_volume(0U);
    main_menu_audio_queue_event(MAIN_MENU_AUDIO_CUE_ERROR);
    FT_ASSERT(!main_menu_audio_poll_event(event));

    main_menu_audio_set_effects_volume(25U);
    main_menu_audio_queue_event(MAIN_MENU_AUDIO_CUE_ERROR);
    FT_ASSERT(main_menu_audio_poll_event(event));
    FT_ASSERT_EQ(MAIN_MENU_AUDIO_CUE_ERROR, event.cue);
    FT_ASSERT_EQ(25U, event.volume_percent);
    FT_ASSERT(!main_menu_audio_poll_event(event));

    main_menu_audio_reset();
    return 1;
}

int verify_accessibility_toggle_persistence()
{
    long timestamp = ft_time_ms();
    ft_string commander("AccessibilityIntegration_");
    commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));

    PlayerProfilePreferences preferences;
    preferences.commander_name = commander;
    FT_ASSERT(player_profile_save(preferences));

    preferences.accessibility_preset_enabled
        = settings_flow_testing::toggle_accessibility_preset(preferences.accessibility_preset_enabled);
    preferences.colorblind_palette_enabled
        = settings_flow_testing::toggle_colorblind_palette(preferences.colorblind_palette_enabled);
    FT_ASSERT(preferences.accessibility_preset_enabled);
    FT_ASSERT(preferences.colorblind_palette_enabled);

    FT_ASSERT(player_profile_save(preferences));

    PlayerProfilePreferences reloaded;
    FT_ASSERT(player_profile_load_or_create(reloaded, commander));
    FT_ASSERT(reloaded.accessibility_preset_enabled);
    FT_ASSERT(reloaded.colorblind_palette_enabled);

    ft_string accessibility_label
        = settings_flow_testing::format_accessibility_preset_option(reloaded.accessibility_preset_enabled);
    FT_ASSERT_EQ(ft_string("Accessibility Preset: On"), accessibility_label);
    ft_string colorblind_label
        = settings_flow_testing::format_colorblind_palette_option(reloaded.colorblind_palette_enabled);
    FT_ASSERT_EQ(ft_string("Colorblind Palette: On"), colorblind_label);

    FT_ASSERT(player_profile_delete(commander));

    return 1;
}
