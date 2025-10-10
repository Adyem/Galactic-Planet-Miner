#include "main_menu_system.hpp"

#include "menu_localization.hpp"

#if GALACTIC_HAVE_SDL2
namespace
{
    ft_string format_menu_hotkey_label(int key_code, const char *fallback_label)
    {
        if (key_code >= 'a' && key_code <= 'z')
        {
            char uppercase = static_cast<char>(key_code - ('a' - 'A'));
            ft_string label;
            label.append(uppercase);
            return label;
        }
        if (key_code >= 'A' && key_code <= 'Z')
        {
            ft_string label;
            label.append(static_cast<char>(key_code));
            return label;
        }
        if (key_code >= '0' && key_code <= '9')
        {
            ft_string label;
            label.append(static_cast<char>(key_code));
            return label;
        }
        if (key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CONFIRM)
            return menu_localize("main_menu.hotkeys.enter", "Enter");
        if (key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CANCEL)
            return menu_localize("main_menu.hotkeys.escape", "Esc");
        if (key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DELETE)
            return menu_localize("main_menu.hotkeys.backspace", "Backspace");
        if (key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_UP || key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DOWN
            || key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_LEFT || key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_RIGHT)
            return menu_localize("main_menu.hotkeys.arrows", "Arrow Keys");
        if (fallback_label != ft_nullptr)
            return ft_string(fallback_label);
        ft_string label;
        label.append('?');
        return label;
    }

    ft_string format_menu_controller_label(int button_code)
    {
        switch (button_code)
        {
            case PLAYER_PROFILE_CONTROLLER_BUTTON_A:
                return menu_localize("main_menu.controller.a", "A");
            case PLAYER_PROFILE_CONTROLLER_BUTTON_B:
                return menu_localize("main_menu.controller.b", "B");
            case PLAYER_PROFILE_CONTROLLER_BUTTON_X:
                return menu_localize("main_menu.controller.x", "X");
            case PLAYER_PROFILE_CONTROLLER_BUTTON_Y:
                return menu_localize("main_menu.controller.y", "Y");
            case PLAYER_PROFILE_CONTROLLER_BUTTON_START:
                return menu_localize("main_menu.controller.start", "Start");
            case PLAYER_PROFILE_CONTROLLER_BUTTON_BACK:
                return menu_localize("main_menu.controller.back", "Back");
            case PLAYER_PROFILE_CONTROLLER_BUTTON_LEFTSHOULDER:
                return menu_localize("main_menu.controller.lb", "LB");
            case PLAYER_PROFILE_CONTROLLER_BUTTON_RIGHTSHOULDER:
                return menu_localize("main_menu.controller.rb", "RB");
            case PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_UP:
                return menu_localize("main_menu.controller.dpad_up", "D-Pad Up");
            case PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_DOWN:
                return menu_localize("main_menu.controller.dpad_down", "D-Pad Down");
            case PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_LEFT:
                return menu_localize("main_menu.controller.dpad_left", "D-Pad Left");
            case PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_RIGHT:
                return menu_localize("main_menu.controller.dpad_right", "D-Pad Right");
            default:
                break;
        }
        return menu_localize("main_menu.controller.unknown", "Button ?");
    }
}
#endif

ft_string main_menu_resolve_description(const ft_ui_menu &menu)
{
    const ft_menu_item *hovered = menu.get_hovered_item();
    if (hovered != ft_nullptr && !hovered->description.empty())
        return hovered->description;

    const ft_menu_item *selected = menu.get_selected_item();
    if (selected != ft_nullptr)
        return selected->description;

    return ft_string();
}

ft_string main_menu_resolve_navigation_hint(const ft_ui_menu &menu, const PlayerProfilePreferences *preferences)
{
#if GALACTIC_HAVE_SDL2
    const ft_menu_item *hovered = menu.get_hovered_item();
    const ft_menu_item *selected = menu.get_selected_item();

    const ft_menu_item *reference = hovered != ft_nullptr ? hovered : selected;

    if (reference == ft_nullptr)
    {
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(6U);

        StringTableReplacement confirm_placeholder;
        confirm_placeholder.key = ft_string("confirm");
        confirm_placeholder.value = menu_localize("main_menu.hotkeys.enter", "Enter");
        replacements.push_back(confirm_placeholder);

        StringTableReplacement controller_confirm_placeholder;
        controller_confirm_placeholder.key = ft_string("controller_confirm");
        controller_confirm_placeholder.value
            = format_menu_controller_label(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_CONFIRM);
        replacements.push_back(controller_confirm_placeholder);

        StringTableReplacement navigate_placeholder;
        navigate_placeholder.key = ft_string("navigate");
        navigate_placeholder.value = menu_localize("main_menu.hotkeys.arrows", "Arrow Keys");
        replacements.push_back(navigate_placeholder);

        StringTableReplacement controller_nav_placeholder;
        controller_nav_placeholder.key = ft_string("controller_navigate");
        controller_nav_placeholder.value = format_menu_controller_label(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_UP);
        replacements.push_back(controller_nav_placeholder);

        StringTableReplacement cancel_placeholder;
        cancel_placeholder.key = ft_string("cancel");
        cancel_placeholder.value = menu_localize("main_menu.hotkeys.escape", "Esc");
        replacements.push_back(cancel_placeholder);

        StringTableReplacement controller_cancel_placeholder;
        controller_cancel_placeholder.key = ft_string("controller_cancel");
        controller_cancel_placeholder.value
            = format_menu_controller_label(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_CANCEL);
        replacements.push_back(controller_cancel_placeholder);

        return menu_localize_format("main_menu.navigation.empty",
            "{{confirm}} / {{controller_confirm}}: Select  |  {{navigate}} / {{controller_navigate}}: Navigate  |  {{cancel}} / {{controller_cancel}}: Back",
            replacements);
    }

    ft_string keyboard_confirm;
    ft_string keyboard_cancel;
    ft_string navigation_label;
    ft_string controller_confirm;
    ft_string controller_cancel;
    ft_string controller_navigation_label;

    if (preferences != ft_nullptr)
    {
        keyboard_confirm = format_menu_hotkey_label(preferences->hotkey_menu_confirm, "Enter");
        keyboard_cancel = format_menu_hotkey_label(preferences->hotkey_menu_cancel, "Esc");
        navigation_label = format_menu_hotkey_label(preferences->hotkey_menu_up, "Arrow Keys");
        controller_confirm = format_menu_controller_label(preferences->controller_menu_confirm);
        controller_cancel = format_menu_controller_label(preferences->controller_menu_cancel);
        controller_navigation_label = format_menu_controller_label(preferences->controller_menu_up);
    }
    else
    {
        keyboard_confirm = menu_localize("main_menu.hotkeys.enter", "Enter");
        keyboard_cancel = menu_localize("main_menu.hotkeys.escape", "Esc");
        navigation_label = menu_localize("main_menu.hotkeys.arrows", "Arrow Keys");
        controller_confirm = format_menu_controller_label(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_CONFIRM);
        controller_cancel = format_menu_controller_label(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_CANCEL);
        ft_string controller_nav_up = format_menu_controller_label(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_UP);
        ft_string controller_nav_down = format_menu_controller_label(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_DOWN);
        if (controller_nav_up == controller_nav_down)
            controller_navigation_label = controller_nav_up;
        else
        {
            controller_navigation_label = controller_nav_up;
            controller_navigation_label.append(" / ");
            controller_navigation_label.append(controller_nav_down);
        }
    }

    ft_vector<StringTableReplacement> replacements;
    replacements.reserve(6U);

    StringTableReplacement confirm_placeholder;
    confirm_placeholder.key = ft_string("confirm");
    confirm_placeholder.value = keyboard_confirm;
    replacements.push_back(confirm_placeholder);

    StringTableReplacement controller_confirm_placeholder;
    controller_confirm_placeholder.key = ft_string("controller_confirm");
    controller_confirm_placeholder.value = controller_confirm;
    replacements.push_back(controller_confirm_placeholder);

    StringTableReplacement navigate_placeholder;
    navigate_placeholder.key = ft_string("navigate");
    navigate_placeholder.value = navigation_label;
    replacements.push_back(navigate_placeholder);

    StringTableReplacement controller_nav_placeholder;
    controller_nav_placeholder.key = ft_string("controller_navigate");
    controller_nav_placeholder.value = controller_navigation_label;
    replacements.push_back(controller_nav_placeholder);

    StringTableReplacement cancel_placeholder;
    cancel_placeholder.key = ft_string("cancel");
    cancel_placeholder.value = keyboard_cancel;
    replacements.push_back(cancel_placeholder);

    StringTableReplacement controller_cancel_placeholder;
    controller_cancel_placeholder.key = ft_string("controller_cancel");
    controller_cancel_placeholder.value = controller_cancel;
    replacements.push_back(controller_cancel_placeholder);

    if (reference->confirm_handler != ft_nullptr)
    {
        return menu_localize_format("main_menu.navigation.action",
            "{{confirm}} / {{controller_confirm}}: {{action}}  |  {{navigate}} / {{controller_navigate}}: Navigate  |  {{cancel}} / {{controller_cancel}}: Back",
            replacements);
    }

    return menu_localize_format("main_menu.navigation.browse",
        "{{navigate}} / {{controller_navigate}}: Navigate  |  {{cancel}} / {{controller_cancel}}: Back",
        replacements);
#else
    (void)menu;
    (void)preferences;
    return ft_string();
#endif
}

