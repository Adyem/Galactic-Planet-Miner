#include "game_test_menu_shared.hpp"

int verify_main_menu_descriptions()
{
    ft_vector<ft_menu_item> items = build_main_menu_items();
    FT_ASSERT_EQ(10U, items.size());

    FT_ASSERT_EQ(ft_string("new_game"), items[0].identifier);
    FT_ASSERT(items[0].enabled);
    FT_ASSERT(!items[0].description.empty());

    FT_ASSERT_EQ(ft_string("resume"), items[1].identifier);
    FT_ASSERT(!items[1].enabled);
    FT_ASSERT_EQ(ft_string("Jump back into your latest campaign save once one is available."), items[1].description);

    FT_ASSERT_EQ(ft_string("tutorial"), items[2].identifier);
    FT_ASSERT(items[2].enabled);
    FT_ASSERT_EQ(ft_string("Revisit onboarding missions and practice fleet controls."), items[2].description);

    FT_ASSERT_EQ(ft_string("load"), items[3].identifier);
    FT_ASSERT(items[3].enabled);
    FT_ASSERT_EQ(ft_string("Review existing saves and prepare to resume a prior campaign."), items[3].description);

    FT_ASSERT_EQ(ft_string("settings"), items[4].identifier);
    FT_ASSERT(items[4].enabled);
    FT_ASSERT_EQ(ft_string("Adjust gameplay, interface scale, and menu layout preferences for this commander."),
        items[4].description);

    FT_ASSERT_EQ(ft_string("swap_profile"), items[5].identifier);
    FT_ASSERT(items[5].enabled);
    FT_ASSERT(!items[5].description.empty());

    FT_ASSERT_EQ(ft_string("changelog"), items[6].identifier);
    FT_ASSERT(items[6].enabled);
    FT_ASSERT(!items[6].description.empty());

    FT_ASSERT_EQ(ft_string("manual"), items[7].identifier);
    FT_ASSERT(items[7].enabled);
    FT_ASSERT(!items[7].description.empty());

    FT_ASSERT_EQ(ft_string("clear_cloud"), items[8].identifier);
    FT_ASSERT(!items[8].enabled);
    FT_ASSERT_EQ(ft_string("Enable experimental features in Settings to manage cloud data."), items[8].description);

    FT_ASSERT_EQ(ft_string("exit"), items[9].identifier);
    FT_ASSERT(items[9].enabled);
    FT_ASSERT(!items[9].description.empty());

    PlayerProfilePreferences experimental_profile;
    experimental_profile.experimental_features_enabled = true;
    ft_vector<ft_menu_item> experimental_items = build_main_menu_items(&experimental_profile);
    FT_ASSERT_EQ(10U, experimental_items.size());
    FT_ASSERT(experimental_items[8].enabled);
    FT_ASSERT_EQ(ft_string("Remove backend-linked progress for this commander after confirming the action."),
        experimental_items[8].description);

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
    const ft_menu_item           &load_item = items[3];

    ft_mouse_state hover_state;
    hover_state.moved = true;
    hover_state.x = load_item.bounds.left + 4;
    hover_state.y = load_item.bounds.top + 4;
    menu.handle_mouse_input(hover_state);

    description = main_menu_testing::resolve_active_description(menu);
    FT_ASSERT_EQ(ft_string("Review existing saves and prepare to resume a prior campaign."), description);

    return 1;
}

int verify_main_menu_navigation_hints()
{
    ft_ui_menu menu;
    menu.set_items(build_main_menu_items());
    menu.set_viewport_bounds(build_main_menu_viewport());

    ft_string hint = main_menu_testing::resolve_navigation_hint(menu, ft_nullptr);
    FT_ASSERT_EQ(ft_string("Enter / A: Select New Game  |  Arrow Keys / D-Pad Up/D-Pad Down: Navigate  |  Esc / B: Back"), hint);

    const ft_vector<ft_menu_item> &items = menu.get_items();
    FT_ASSERT_EQ(10U, items.size());
    const ft_menu_item &load_item = items[3];

    ft_mouse_state hover_state;
    hover_state.moved = true;
    hover_state.x = load_item.bounds.left + 4;
    hover_state.y = load_item.bounds.top + 4;
    menu.handle_mouse_input(hover_state);

    hint = main_menu_testing::resolve_navigation_hint(menu, ft_nullptr);
    FT_ASSERT_EQ(ft_string("Enter / A: Select Load  |  Arrow Keys / D-Pad Up/D-Pad Down: Navigate  |  Esc / B: Back"), hint);

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

    custom.controller_menu_confirm = PLAYER_PROFILE_CONTROLLER_BUTTON_Y;
    custom.controller_menu_cancel = PLAYER_PROFILE_CONTROLLER_BUTTON_X;
    custom.controller_menu_up = PLAYER_PROFILE_CONTROLLER_BUTTON_LEFTSHOULDER;
    custom.controller_menu_down = PLAYER_PROFILE_CONTROLLER_BUTTON_RIGHTSHOULDER;

    ft_string hint = main_menu_testing::resolve_navigation_hint(menu, &custom);

    FT_ASSERT_EQ(ft_string("E / Y: Select New Game  |  W/S / LB/RB: Navigate  |  Q / X: Back"), hint);

    const ft_vector<ft_menu_item> &items = menu.get_items();
    FT_ASSERT_EQ(10U, items.size());
    const ft_menu_item &load_item = items[3];

    ft_mouse_state hover_state;
    hover_state.moved = true;
    hover_state.x = load_item.bounds.left + 4;
    hover_state.y = load_item.bounds.top + 4;
    menu.handle_mouse_input(hover_state);

    hint = main_menu_testing::resolve_navigation_hint(menu, &custom);
    FT_ASSERT_EQ(ft_string("E / Y: Select Load  |  W/S / LB/RB: Navigate  |  Q / X: Back"), hint);

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
