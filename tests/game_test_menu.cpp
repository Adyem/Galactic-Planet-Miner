#include "../libft/System_utils/test_runner.hpp"
#include "../libft/CPP_class/class_nullptr.hpp"
#include "../libft/CPP_class/class_ofstream.hpp"
#include "../libft/File/file_utils.hpp"
#include "../libft/File/open_dir.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Time/time.hpp"
#include "game_test_scenarios.hpp"
#include "main_menu_system.hpp"
#include "build_info.hpp"
#include "menu_localization.hpp"

#include <errno.h>

namespace
{
    bool ensure_directory_exists(const char *path) noexcept
    {
        if (path == ft_nullptr)
            return false;
        int result = file_create_directory(path, 0755);
        if (result == 0)
            return true;
        if (ft_errno == ERRNO_OFFSET + EEXIST)
            return true;
        return false;
    }

    ft_string build_expected_metadata_label(int day, int level, const char *difficulty_key,
        const char *difficulty_fallback)
    {
        ft_vector<StringTableReplacement> value_replacements;
        value_replacements.reserve(1U);
        StringTableReplacement value_replacement;
        value_replacement.key = ft_string("value");
        value_replacement.value = ft_to_string(day);
        value_replacements.push_back(value_replacement);
        ft_string day_label
            = menu_localize_format("load_menu.metadata.day_known", "Day {{value}}", value_replacements);

        value_replacements[0].value = ft_to_string(level);
        ft_string level_label
            = menu_localize_format("load_menu.metadata.level_known", "Level {{value}}", value_replacements);

        ft_string difficulty_name = menu_localize(difficulty_key, difficulty_fallback);
        ft_vector<StringTableReplacement> difficulty_replacements;
        difficulty_replacements.reserve(1U);
        StringTableReplacement difficulty_replacement;
        difficulty_replacement.key = ft_string("difficulty");
        difficulty_replacement.value = difficulty_name;
        difficulty_replacements.push_back(difficulty_replacement);
        ft_string difficulty_label = menu_localize_format(
            "load_menu.metadata.difficulty_known", "Difficulty: {{difficulty}}", difficulty_replacements);

        ft_vector<StringTableReplacement> combined_replacements;
        combined_replacements.reserve(3U);
        StringTableReplacement day_replacement;
        day_replacement.key = ft_string("day");
        day_replacement.value = day_label;
        combined_replacements.push_back(day_replacement);

        StringTableReplacement level_replacement;
        level_replacement.key = ft_string("level");
        level_replacement.value = level_label;
        combined_replacements.push_back(level_replacement);

        StringTableReplacement combined_difficulty_replacement;
        combined_difficulty_replacement.key = ft_string("difficulty");
        combined_difficulty_replacement.value = difficulty_label;
        combined_replacements.push_back(combined_difficulty_replacement);

        return menu_localize_format(
            "load_menu.metadata.combined", "{{day}} • {{level}} • {{difficulty}}", combined_replacements);
    }
}

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
    FT_ASSERT_EQ(PLAYER_PROFILE_INPUT_DEVICE_KEYBOARD, defaults.last_menu_input_device);

    PlayerProfilePreferences custom = profile_preferences_testing::build_window_preference_snapshot(
        ft_string("Vesta"), 1920U, 1080U);
    FT_ASSERT_EQ(ft_string("Vesta"), custom.commander_name);
    FT_ASSERT_EQ(1920U, custom.window_width);
    FT_ASSERT_EQ(1080U, custom.window_height);
    FT_ASSERT_EQ(100U, custom.music_volume_percent);
    FT_ASSERT_EQ(100U, custom.effects_volume_percent);
    FT_ASSERT_EQ(PLAYER_PROFILE_INPUT_DEVICE_KEYBOARD, custom.last_menu_input_device);

    PlayerProfilePreferences mixed = profile_preferences_testing::build_window_preference_snapshot(
        ft_string("Vega"), 2048U, 0U);
    FT_ASSERT_EQ(2048U, mixed.window_width);
    FT_ASSERT_EQ(720U, mixed.window_height);
    FT_ASSERT_EQ(100U, mixed.music_volume_percent);
    FT_ASSERT_EQ(100U, mixed.effects_volume_percent);
    FT_ASSERT_EQ(PLAYER_PROFILE_INPUT_DEVICE_KEYBOARD, mixed.last_menu_input_device);

    return 1;
}

int verify_main_menu_descriptions()
{
    ft_vector<ft_menu_item> items = build_main_menu_items();
    FT_ASSERT_EQ(9U, items.size());

    FT_ASSERT_EQ(ft_string("new_game"), items[0].identifier);
    FT_ASSERT(items[0].enabled);
    FT_ASSERT(!items[0].description.empty());

    FT_ASSERT_EQ(ft_string("resume"), items[1].identifier);
    FT_ASSERT(!items[1].enabled);
    FT_ASSERT_EQ(ft_string("Jump back into your latest campaign save once one is available."), items[1].description);

    FT_ASSERT_EQ(ft_string("load"), items[2].identifier);
    FT_ASSERT(items[2].enabled);
    FT_ASSERT_EQ(ft_string("Review existing saves and prepare to resume a prior campaign."), items[2].description);

    FT_ASSERT_EQ(ft_string("settings"), items[3].identifier);
    FT_ASSERT(items[3].enabled);
    FT_ASSERT_EQ(ft_string("Adjust gameplay, interface scale, and menu layout preferences for this commander."),
        items[3].description);

    FT_ASSERT_EQ(ft_string("swap_profile"), items[4].identifier);
    FT_ASSERT(items[4].enabled);
    FT_ASSERT(!items[4].description.empty());

    FT_ASSERT_EQ(ft_string("changelog"), items[5].identifier);
    FT_ASSERT(items[5].enabled);
    FT_ASSERT(!items[5].description.empty());

    FT_ASSERT_EQ(ft_string("manual"), items[6].identifier);
    FT_ASSERT(items[6].enabled);
    FT_ASSERT(!items[6].description.empty());

    FT_ASSERT_EQ(ft_string("clear_cloud"), items[7].identifier);
    FT_ASSERT(items[7].enabled);
    FT_ASSERT_EQ(ft_string("Remove backend-linked progress for this commander after confirming the action."),
        items[7].description);

    FT_ASSERT_EQ(ft_string("exit"), items[8].identifier);
    FT_ASSERT(items[8].enabled);
    FT_ASSERT(!items[8].description.empty());

    return 1;
}

int verify_main_menu_description_focus()
{
    ft_ui_menu menu;
    menu.set_items(build_main_menu_items());
    menu.set_viewport_bounds(build_main_menu_viewport());

    ft_string description = main_menu_testing::resolve_active_description(menu);
    FT_ASSERT_EQ(ft_string("Begin a fresh campaign for the active commander."), description);

    const ft_vector<ft_menu_item> &items = menu.get_items();
    const ft_menu_item           &load_item = items[2];

    ft_mouse_state hover_state;
    hover_state.moved = true;
    hover_state.x = load_item.bounds.left + 4;
    hover_state.y = load_item.bounds.top + 4;
    menu.handle_mouse_input(hover_state);

    description = main_menu_testing::resolve_active_description(menu);
    FT_ASSERT_EQ(ft_string("Review existing saves and prepare to resume a prior campaign."), description);

    const ft_menu_item *selected = menu.get_selected_item();
    FT_ASSERT(selected != ft_nullptr);
    FT_ASSERT_EQ(ft_string("new_game"), selected->identifier);

    ft_mouse_state click_state;
    click_state.moved = true;
    click_state.left_pressed = true;
    click_state.x = load_item.bounds.left + 6;
    click_state.y = load_item.bounds.top + 6;
    menu.handle_mouse_input(click_state);

    selected = menu.get_selected_item();
    FT_ASSERT(selected != ft_nullptr);
    FT_ASSERT_EQ(ft_string("new_game"), selected->identifier);

    return 1;
}

int verify_main_menu_navigation_hints()
{
    ft_ui_menu menu;
    menu.set_items(build_main_menu_items());
    menu.set_viewport_bounds(build_main_menu_viewport());

    ft_string hint = main_menu_testing::resolve_navigation_hint(menu, ft_nullptr);
    FT_ASSERT_EQ(ft_string("Enter / A: Select New Game  |  Arrow Keys / D-Pad: Navigate  |  Esc / B: Back"), hint);

    const ft_vector<ft_menu_item> &items = menu.get_items();
    FT_ASSERT_EQ(9U, items.size());

    const ft_menu_item &load_item = items[2];

    ft_mouse_state hover_state;
    hover_state.moved = true;
    hover_state.x = load_item.bounds.left + 4;
    hover_state.y = load_item.bounds.top + 4;
    menu.handle_mouse_input(hover_state);

    hint = main_menu_testing::resolve_navigation_hint(menu, ft_nullptr);
    FT_ASSERT_EQ(ft_string("Enter / A: Select Load  |  Arrow Keys / D-Pad: Navigate  |  Esc / B: Back"), hint);

    return 1;
}

int verify_main_menu_navigation_hints_custom()
{
    ft_ui_menu menu;
    menu.set_items(build_main_menu_items());
    menu.set_viewport_bounds(build_main_menu_viewport());

    PlayerProfilePreferences custom;
    custom.hotkey_menu_up = 'w';
    custom.hotkey_menu_down = 's';
    custom.hotkey_menu_confirm = 'e';
    custom.hotkey_menu_cancel = 'q';

    ft_string hint = main_menu_testing::resolve_navigation_hint(menu, &custom);
    FT_ASSERT_EQ(ft_string("E / A: Select New Game  |  W/S / D-Pad: Navigate  |  Q / B: Back"), hint);

    const ft_vector<ft_menu_item> &items = menu.get_items();
    FT_ASSERT_EQ(9U, items.size());
    const ft_menu_item &load_item = items[2];

    ft_mouse_state hover_state;
    hover_state.moved = true;
    hover_state.x = load_item.bounds.left + 4;
    hover_state.y = load_item.bounds.top + 4;
    menu.handle_mouse_input(hover_state);

    hint = main_menu_testing::resolve_navigation_hint(menu, &custom);
    FT_ASSERT_EQ(ft_string("E / A: Select Load  |  W/S / D-Pad: Navigate  |  Q / B: Back"), hint);

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

int verify_main_menu_tutorial_tips()
{
    ft_vector<ft_string> tips = main_menu_testing::collect_tutorial_tips();
    FT_ASSERT_EQ(4U, tips.size());
    FT_ASSERT_EQ(ft_string("Press Enter or click anywhere to dismiss these tips."), tips[0]);
    FT_ASSERT_EQ(ft_string("Use Arrow Keys / D-Pad or the mouse to highlight menu entries."), tips[1]);
    FT_ASSERT_EQ(ft_string("Choose New Game to start a fresh campaign for this commander."), tips[2]);
    FT_ASSERT_EQ(ft_string("Swap Profile lets you switch between saved commanders."), tips[3]);

    return 1;
}

int verify_main_menu_manual_sections()
{
    ft_vector<ft_string> lines = main_menu_testing::collect_manual_lines();
    FT_ASSERT_EQ(5U, lines.size());
    FT_ASSERT_EQ(ft_string("Browse ship loadouts, building efficiencies, and resource yields."), lines[0]);
    FT_ASSERT_EQ(ft_string("Review campaign objectives with quick strategy tips for each branch."), lines[1]);
    FT_ASSERT_EQ(ft_string("Study combat controls, hotkeys, and fleet formation guidance."), lines[2]);
    FT_ASSERT_EQ(ft_string("Access lore entries and encyclopedia cross-links without leaving the menu."), lines[3]);
    FT_ASSERT_EQ(ft_string("Reconnect to the network if encyclopedia updates appear out of date."), lines[4]);

    return 1;
}

int verify_main_menu_connectivity_indicator()
{
    MainMenuConnectivityStatus status;
    main_menu_testing::mark_connectivity_checking(status, 120);

    ft_string checking_label = main_menu_testing::resolve_connectivity_label(status);
    FT_ASSERT_EQ(ft_string("Checking backend..."), checking_label);
    SDL_Color checking_color = main_menu_testing::resolve_connectivity_color(status);
    FT_ASSERT_EQ(182, static_cast<int>(checking_color.r));
    FT_ASSERT_EQ(192, static_cast<int>(checking_color.g));
    FT_ASSERT_EQ(212, static_cast<int>(checking_color.b));
    FT_ASSERT_EQ(255, static_cast<int>(checking_color.a));

    main_menu_testing::apply_connectivity_result(status, true, 204, 240);
    ft_string online_label = main_menu_testing::resolve_connectivity_label(status);
    FT_ASSERT_EQ(ft_string("Backend Online (HTTP 204)"), online_label);
    SDL_Color online_color = main_menu_testing::resolve_connectivity_color(status);
    FT_ASSERT_EQ(108, static_cast<int>(online_color.r));
    FT_ASSERT_EQ(210, static_cast<int>(online_color.g));
    FT_ASSERT_EQ(156, static_cast<int>(online_color.b));
    FT_ASSERT_EQ(255, static_cast<int>(online_color.a));

    main_menu_testing::apply_connectivity_result(status, false, 503, 360);
    ft_string offline_label = main_menu_testing::resolve_connectivity_label(status);
    FT_ASSERT_EQ(ft_string("Backend Offline (HTTP 503)"), offline_label);
    SDL_Color offline_color = main_menu_testing::resolve_connectivity_color(status);
    FT_ASSERT_EQ(220, static_cast<int>(offline_color.r));
    FT_ASSERT_EQ(120, static_cast<int>(offline_color.g));
    FT_ASSERT_EQ(120, static_cast<int>(offline_color.b));
    FT_ASSERT_EQ(255, static_cast<int>(offline_color.a));

    main_menu_testing::apply_connectivity_result(status, false, 0, 480);
    ft_string offline_no_code = main_menu_testing::resolve_connectivity_label(status);
    FT_ASSERT_EQ(ft_string("Backend Offline"), offline_no_code);

    return 1;
}

int verify_main_menu_connectivity_failure_logging()
{
    const ft_string log_path("tests/tmp_backend_ping_failures.log");
    const ft_string host("127.0.0.1:8080");
    const long      timestamp_ms = 1717171717123L;
    const int       status_code = 503;

    file_delete(log_path.c_str());

    FT_ASSERT(main_menu_testing::append_connectivity_failure_log(host, status_code, timestamp_ms, log_path));

    FILE *log_file = ft_fopen(log_path.c_str(), "r");
    FT_ASSERT(log_file != ft_nullptr);

    char   buffer[256];
    size_t read_count = fread(buffer, 1, sizeof(buffer) - 1, log_file);
    buffer[read_count] = '\0';
    ft_fclose(log_file);

    ft_string contents(buffer);
    ft_string expected("[2024-05-31T16:08:37Z.123] ");
    expected.append("[");
    expected.append(build_info_format_label());
    expected.append("] backend ping to 127.0.0.1:8080 failed (HTTP 503)\n");
    FT_ASSERT_EQ(expected, contents);

    FT_ASSERT_EQ(0, file_delete(log_path.c_str()));

    return 1;
}

int verify_main_menu_build_label()
{
    const ft_string expected = build_info_format_label();
    ft_string        label = main_menu_testing::resolve_build_label();
    FT_ASSERT_EQ(expected, label);
    return 1;
}

int verify_main_menu_patch_notes_split()
{
    ft_string body("Update 42\r\n- Added things\n\nThanks, commanders!\n");
    ft_vector<ft_string> lines = main_menu_testing::split_patch_note_lines(body);
    FT_ASSERT_EQ(5U, lines.size());
    FT_ASSERT_EQ(ft_string("Update 42"), lines[0]);
    FT_ASSERT_EQ(ft_string("- Added things"), lines[1]);
    FT_ASSERT(lines[2].empty());
    FT_ASSERT_EQ(ft_string("Thanks, commanders!"), lines[3]);
    FT_ASSERT(lines[4].empty());
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

int verify_main_menu_save_alerts()
{
    long timestamp = ft_time_ms();
    ft_string commander("MainMenuSaveAlerts_");
    commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));

    PlayerProfilePreferences preferences;
    preferences.commander_name = commander;
    FT_ASSERT(player_profile_save(preferences));

    ft_string error;
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("stable"), error));
    FT_ASSERT(error.empty());
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("corrupt"), error));
    FT_ASSERT(error.empty());

    ft_string stable_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("stable"));
    FT_ASSERT(!stable_path.empty());
    ft_ofstream stable_stream;
    FT_ASSERT_EQ(0, stable_stream.open(stable_path.c_str()));
    ft_string stable_contents("{\n");
    stable_contents.append("  \"metadata\": {\"version\": 1, \"save_type\": \"quicksave\"},\n");
    stable_contents.append("  \"player\": {\"commander_name\": \"Alert Tester\", \"starting_planet_id\": 1, \"commander_level\": 4},\n");
    stable_contents.append("  \"campaign\": {\"day\": 3, \"difficulty\": 2}\n");
    stable_contents.append("}\n");
    FT_ASSERT(stable_stream.write(stable_contents.c_str()) >= 0);
    stable_stream.close();

    ft_string corrupt_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("corrupt"));
    FT_ASSERT(!corrupt_path.empty());
    ft_ofstream corrupt_stream;
    FT_ASSERT_EQ(0, corrupt_stream.open(corrupt_path.c_str()));
    ft_string corrupt_contents("{\"metadata\":\n");
    FT_ASSERT(corrupt_stream.write(corrupt_contents.c_str()) >= 0);
    corrupt_stream.close();

    ft_vector<ft_string> slot_errors = load_game_flow_testing::collect_save_slot_errors(commander);
    FT_ASSERT_EQ(1U, slot_errors.size());
    FT_ASSERT(ft_strstr(slot_errors[0].c_str(), "corrupt") != ft_nullptr);

    ft_vector<ft_string> audit_errors;
    FT_ASSERT(audit_save_directory_for_errors(commander, audit_errors));
    FT_ASSERT_EQ(1U, audit_errors.size());
    FT_ASSERT(ft_strstr(audit_errors[0].c_str(), "corrupt") != ft_nullptr);

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

int verify_main_menu_commander_portrait_preload()
{
    main_menu_testing::reset_commander_portrait_cache();

    FT_ASSERT(ensure_directory_exists("assets"));
    FT_ASSERT(ensure_directory_exists("assets/portraits"));

    ft_string default_path("assets/portraits/default.png");
    ft_ofstream default_stream;
    FT_ASSERT_EQ(0, default_stream.open(default_path.c_str()));
    FT_ASSERT(default_stream.write("default portrait data") >= 0);
    default_stream.close();

    long timestamp = ft_time_ms();

    ft_string missing_commander("PortraitMissing_");
    missing_commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));
    PlayerProfilePreferences missing_preferences;
    missing_preferences.commander_name = missing_commander;
    FT_ASSERT(player_profile_save(missing_preferences));

    FT_ASSERT(main_menu_preload_commander_portrait(missing_commander));
    FT_ASSERT(main_menu_testing::commander_portrait_attempted(missing_commander));
    FT_ASSERT(main_menu_testing::commander_portrait_loaded(missing_commander));
    ft_string missing_cached_path = main_menu_testing::resolve_cached_portrait_path(missing_commander);
    FT_ASSERT_EQ(default_path, missing_cached_path);
    FT_ASSERT(main_menu_testing::commander_portrait_cached_size(missing_commander) > 0U);

    ft_string specific_commander("PortraitSpecific_");
    specific_commander.append(ft_to_string(static_cast<int>((timestamp + 1) % 1000000L)));
    PlayerProfilePreferences specific_preferences;
    specific_preferences.commander_name = specific_commander;
    FT_ASSERT(player_profile_save(specific_preferences));

    ft_string portrait_filename = main_menu_testing::resolve_commander_portrait_filename(specific_commander);
    FT_ASSERT(!portrait_filename.empty());
    ft_string specific_path("assets/portraits/");
    specific_path.append(portrait_filename);
    specific_path.append(".png");

    ft_ofstream specific_stream;
    FT_ASSERT_EQ(0, specific_stream.open(specific_path.c_str()));
    FT_ASSERT(specific_stream.write("specific portrait data") >= 0);
    specific_stream.close();

    FT_ASSERT(main_menu_preload_commander_portrait(specific_commander));
    FT_ASSERT(main_menu_testing::commander_portrait_attempted(specific_commander));
    FT_ASSERT(main_menu_testing::commander_portrait_loaded(specific_commander));
    ft_string specific_cached_path = main_menu_testing::resolve_cached_portrait_path(specific_commander);
    FT_ASSERT_EQ(specific_path, specific_cached_path);
    FT_ASSERT(main_menu_testing::commander_portrait_cached_size(specific_commander) > 0U);

    FT_ASSERT(player_profile_delete(missing_commander));
    FT_ASSERT(player_profile_delete(specific_commander));
    FT_ASSERT_EQ(0, file_delete(default_path.c_str()));
    FT_ASSERT_EQ(0, file_delete(specific_path.c_str()));

    main_menu_testing::reset_commander_portrait_cache();

    return 1;
}

int verify_main_menu_campaign_launch_guard()
{
    FT_ASSERT(!main_menu_can_launch_campaign(ft_string()));

    ft_string invalid_path("data/saves/does-not-exist.json");
    FT_ASSERT(!main_menu_can_launch_campaign(invalid_path));

    long timestamp = ft_time_ms();
    ft_string commander("LaunchGuard_");
    commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));

    PlayerProfilePreferences preferences;
    preferences.commander_name = commander;
    FT_ASSERT(player_profile_save(preferences));

    ft_string error;
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("launchslot"), error));
    FT_ASSERT(error.empty());

    ft_string save_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("launchslot"));
    FT_ASSERT(!save_path.empty());
    FT_ASSERT(main_menu_can_launch_campaign(save_path));

    FT_ASSERT(player_profile_delete(commander));

    return 1;
}
