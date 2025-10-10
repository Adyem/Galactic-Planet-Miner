#include "main_menu_system.hpp"

#include "app_constants.hpp"
#include "menu_localization.hpp"

#include "libft/Libft/libft.hpp"
#include "libft/Time/time.hpp"

namespace
{
#if GALACTIC_HAVE_SDL2
    unsigned int clamp_percent(unsigned int value, unsigned int min_value, unsigned int max_value) noexcept
    {
        if (value < min_value)
            return min_value;
        if (value > max_value)
            return max_value;
        return value;
    }

    unsigned char apply_component_levels(unsigned char component, unsigned int brightness_percent,
        unsigned int contrast_percent) noexcept
    {
        long value = static_cast<long>(component);
        long centered = value - 128L;
        long contrasted = (centered * static_cast<long>(contrast_percent) + 50L) / 100L + 128L;
        if (contrasted < 0L)
            contrasted = 0L;
        else if (contrasted > 255L)
            contrasted = 255L;

        long brightened = (contrasted * static_cast<long>(brightness_percent) + 50L) / 100L;
        if (brightened < 0L)
            brightened = 0L;
        else if (brightened > 255L)
            brightened = 255L;

        return static_cast<unsigned char>(brightened);
    }

    void apply_levels(SDL_Color &color, unsigned int brightness_percent, unsigned int contrast_percent) noexcept
    {
        color.r = apply_component_levels(color.r, brightness_percent, contrast_percent);
        color.g = apply_component_levels(color.g, brightness_percent, contrast_percent);
        color.b = apply_component_levels(color.b, brightness_percent, contrast_percent);
    }

    struct MainMenuPalette
    {
        SDL_Color background;
        SDL_Color title;
        SDL_Color profile;
        SDL_Color button_idle;
        SDL_Color button_hover;
        SDL_Color button_selected;
        SDL_Color button_disabled;
        SDL_Color button_disabled_hover;
        SDL_Color button_border_enabled;
        SDL_Color button_border_disabled;
        SDL_Color button_text_enabled;
        SDL_Color button_text_disabled;
        SDL_Color description;
        SDL_Color hint;
        SDL_Color build;
        SDL_Color alert_text_error;
        SDL_Color alert_text_info;
        SDL_Color alert_background_error;
        SDL_Color alert_border_error;
        SDL_Color alert_background_info;
        SDL_Color alert_border_info;
        SDL_Color overlay_heading;
        SDL_Color overlay_line;
        SDL_Color overlay_footer;
        SDL_Color overlay_background;
        SDL_Color overlay_border;
        SDL_Color tutorial_title;
        SDL_Color tutorial_primary;
        SDL_Color tutorial_secondary;
        SDL_Color tutorial_background;
        SDL_Color tutorial_border;
        SDL_Color autosave_icon_idle;
        SDL_Color autosave_icon_in_progress;
        SDL_Color autosave_icon_success;
        SDL_Color autosave_icon_failure;
        SDL_Color autosave_text_in_progress;
        SDL_Color autosave_text_success;
        SDL_Color autosave_text_failure;
        SDL_Color sync_heading;
        SDL_Color sync_idle;
        SDL_Color sync_pending;
        SDL_Color sync_success;
        SDL_Color sync_failure;
        SDL_Color sync_background;
        SDL_Color sync_border;
        SDL_Color achievements_heading;
        SDL_Color achievements_primary;
        SDL_Color achievements_secondary;
        SDL_Color achievements_background;
        SDL_Color achievements_border;
        SDL_Color performance_heading;
        SDL_Color performance_value;
        SDL_Color performance_warning;
        SDL_Color performance_background;
        SDL_Color performance_border;

        MainMenuPalette() noexcept
            : background(), title(), profile(), button_idle(), button_hover(), button_selected(), button_disabled(),
              button_disabled_hover(), button_border_enabled(), button_border_disabled(), button_text_enabled(),
              button_text_disabled(), description(), hint(), build(), alert_text_error(), alert_text_info(),
              alert_background_error(), alert_border_error(), alert_background_info(), alert_border_info(),
              overlay_heading(), overlay_line(), overlay_footer(), overlay_background(), overlay_border(), tutorial_title(),
              tutorial_primary(), tutorial_secondary(), tutorial_background(), tutorial_border(), autosave_icon_idle(),
              autosave_icon_in_progress(), autosave_icon_success(), autosave_icon_failure(), autosave_text_in_progress(),
              autosave_text_success(), autosave_text_failure(), sync_heading(), sync_idle(), sync_pending(), sync_success(),
              sync_failure(), sync_background(), sync_border(), achievements_heading(), achievements_primary(),
              achievements_secondary(), achievements_background(), achievements_border(), performance_heading(),
              performance_value(), performance_warning(), performance_background(), performance_border()
        {}
    };

    void apply_levels(MainMenuPalette &palette, unsigned int brightness_percent, unsigned int contrast_percent) noexcept
    {
        apply_levels(palette.background, brightness_percent, contrast_percent);
        apply_levels(palette.title, brightness_percent, contrast_percent);
        apply_levels(palette.profile, brightness_percent, contrast_percent);
        apply_levels(palette.button_idle, brightness_percent, contrast_percent);
        apply_levels(palette.button_hover, brightness_percent, contrast_percent);
        apply_levels(palette.button_selected, brightness_percent, contrast_percent);
        apply_levels(palette.button_disabled, brightness_percent, contrast_percent);
        apply_levels(palette.button_disabled_hover, brightness_percent, contrast_percent);
        apply_levels(palette.button_border_enabled, brightness_percent, contrast_percent);
        apply_levels(palette.button_border_disabled, brightness_percent, contrast_percent);
        apply_levels(palette.button_text_enabled, brightness_percent, contrast_percent);
        apply_levels(palette.button_text_disabled, brightness_percent, contrast_percent);
        apply_levels(palette.description, brightness_percent, contrast_percent);
        apply_levels(palette.hint, brightness_percent, contrast_percent);
        apply_levels(palette.build, brightness_percent, contrast_percent);
        apply_levels(palette.alert_text_error, brightness_percent, contrast_percent);
        apply_levels(palette.alert_text_info, brightness_percent, contrast_percent);
        apply_levels(palette.alert_background_error, brightness_percent, contrast_percent);
        apply_levels(palette.alert_border_error, brightness_percent, contrast_percent);
        apply_levels(palette.alert_background_info, brightness_percent, contrast_percent);
        apply_levels(palette.alert_border_info, brightness_percent, contrast_percent);
        apply_levels(palette.overlay_heading, brightness_percent, contrast_percent);
        apply_levels(palette.overlay_line, brightness_percent, contrast_percent);
        apply_levels(palette.overlay_footer, brightness_percent, contrast_percent);
        apply_levels(palette.overlay_background, brightness_percent, contrast_percent);
        apply_levels(palette.overlay_border, brightness_percent, contrast_percent);
        apply_levels(palette.tutorial_title, brightness_percent, contrast_percent);
        apply_levels(palette.tutorial_primary, brightness_percent, contrast_percent);
        apply_levels(palette.tutorial_secondary, brightness_percent, contrast_percent);
        apply_levels(palette.tutorial_background, brightness_percent, contrast_percent);
        apply_levels(palette.tutorial_border, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_icon_idle, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_icon_in_progress, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_icon_success, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_icon_failure, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_text_in_progress, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_text_success, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_text_failure, brightness_percent, contrast_percent);
        apply_levels(palette.sync_heading, brightness_percent, contrast_percent);
        apply_levels(palette.sync_idle, brightness_percent, contrast_percent);
        apply_levels(palette.sync_pending, brightness_percent, contrast_percent);
        apply_levels(palette.sync_success, brightness_percent, contrast_percent);
        apply_levels(palette.sync_failure, brightness_percent, contrast_percent);
        apply_levels(palette.sync_background, brightness_percent, contrast_percent);
        apply_levels(palette.sync_border, brightness_percent, contrast_percent);
        apply_levels(palette.achievements_heading, brightness_percent, contrast_percent);
        apply_levels(palette.achievements_primary, brightness_percent, contrast_percent);
        apply_levels(palette.achievements_secondary, brightness_percent, contrast_percent);
        apply_levels(palette.achievements_background, brightness_percent, contrast_percent);
        apply_levels(palette.achievements_border, brightness_percent, contrast_percent);
        apply_levels(palette.performance_heading, brightness_percent, contrast_percent);
        apply_levels(palette.performance_value, brightness_percent, contrast_percent);
        apply_levels(palette.performance_warning, brightness_percent, contrast_percent);
        apply_levels(palette.performance_background, brightness_percent, contrast_percent);
        apply_levels(palette.performance_border, brightness_percent, contrast_percent);
    }

    MainMenuPalette resolve_main_menu_palette(
        bool colorblind_enabled, unsigned int brightness_percent, unsigned int contrast_percent)
    {
        MainMenuPalette palette;

        palette.background = {8, 16, 22, 255};
        palette.title = {232, 238, 250, 255};
        palette.profile = {200, 210, 232, 255};
        palette.button_idle = {34, 52, 72, 255};
        palette.button_hover = {94, 130, 184, 255};
        palette.button_selected = {68, 108, 168, 255};
        palette.button_disabled = {44, 52, 70, 255};
        palette.button_disabled_hover = {66, 80, 104, 255};
        palette.button_border_enabled = {120, 154, 212, 255};
        palette.button_border_disabled = {90, 110, 160, 255};
        palette.button_text_enabled = {255, 255, 255, 255};
        palette.button_text_disabled = {188, 196, 210, 255};
        palette.description = {210, 220, 240, 255};
        palette.hint = {165, 176, 204, 255};
        palette.build = {140, 150, 184, 255};
        palette.alert_text_error = {255, 206, 206, 255};
        palette.alert_text_info = {200, 230, 255, 255};
        palette.alert_background_error = {84, 32, 32, 235};
        palette.alert_border_error = {164, 80, 80, 255};
        palette.alert_background_info = {26, 64, 88, 235};
        palette.alert_border_info = {88, 138, 176, 255};
        palette.overlay_heading = {236, 242, 255, 255};
        palette.overlay_line = {208, 216, 236, 255};
        palette.overlay_footer = {184, 196, 224, 255};
        palette.overlay_background = {18, 24, 44, 238};
        palette.overlay_border = {92, 112, 166, 255};
        palette.tutorial_title = {234, 238, 250, 255};
        palette.tutorial_primary = {210, 220, 240, 255};
        palette.tutorial_secondary = {200, 208, 232, 255};
        palette.tutorial_background = {18, 24, 44, 235};
        palette.tutorial_border = {90, 110, 160, 255};
        palette.autosave_icon_idle = {74, 88, 116, 255};
        palette.autosave_icon_in_progress = {128, 172, 228, 255};
        palette.autosave_icon_success = {104, 188, 148, 255};
        palette.autosave_icon_failure = {220, 96, 96, 255};
        palette.autosave_text_in_progress = {188, 206, 238, 255};
        palette.autosave_text_success = {210, 240, 220, 255};
        palette.autosave_text_failure = {255, 214, 214, 255};
        palette.sync_heading = {228, 236, 252, 255};
        palette.sync_idle = {186, 198, 224, 255};
        palette.sync_pending = {196, 210, 240, 255};
        palette.sync_success = {206, 238, 220, 255};
        palette.sync_failure = {255, 214, 214, 255};
        palette.sync_background = {22, 30, 54, 235};
        palette.sync_border = {92, 116, 170, 255};
        palette.achievements_heading = {242, 248, 255, 255};
        palette.achievements_primary = {210, 220, 240, 255};
        palette.achievements_secondary = {194, 206, 232, 255};
        palette.achievements_background = {24, 34, 58, 235};
        palette.achievements_border = {96, 120, 168, 255};
        palette.performance_heading = {236, 244, 255, 255};
        palette.performance_value = {204, 214, 236, 255};
        palette.performance_warning = {255, 226, 196, 255};
        palette.performance_background = {20, 30, 50, 230};
        palette.performance_border = {92, 120, 170, 255};

        if (colorblind_enabled)
        {
            palette.background = {10, 20, 26, 255};
            palette.title = {240, 246, 255, 255};
            palette.profile = {212, 220, 240, 255};
            palette.button_idle = {44, 62, 88, 255};
            palette.button_hover = {104, 140, 204, 255};
            palette.button_selected = {78, 118, 184, 255};
            palette.button_disabled = {52, 60, 80, 255};
            palette.button_disabled_hover = {74, 88, 112, 255};
            palette.button_border_enabled = {134, 170, 228, 255};
            palette.button_border_disabled = {102, 120, 168, 255};
            palette.button_text_enabled = {255, 255, 255, 255};
            palette.button_text_disabled = {216, 222, 234, 255};
            palette.description = {216, 228, 248, 255};
            palette.hint = {176, 192, 220, 255};
            palette.build = {156, 170, 204, 255};
            palette.alert_text_error = {255, 224, 200, 255};
            palette.alert_text_info = {210, 242, 255, 255};
            palette.alert_background_error = {112, 60, 30, 240};
            palette.alert_border_error = {204, 128, 64, 255};
            palette.alert_background_info = {26, 78, 104, 240};
            palette.alert_border_info = {96, 160, 200, 255};
            palette.overlay_heading = {240, 246, 255, 255};
            palette.overlay_line = {212, 224, 244, 255};
            palette.overlay_footer = {192, 206, 232, 255};
            palette.overlay_background = {28, 44, 70, 238};
            palette.overlay_border = {120, 156, 210, 255};
            palette.tutorial_title = {240, 246, 255, 255};
            palette.tutorial_primary = {214, 234, 248, 255};
            palette.tutorial_secondary = {200, 218, 240, 255};
            palette.tutorial_background = {28, 44, 70, 235};
            palette.tutorial_border = {120, 156, 210, 255};
            palette.autosave_icon_idle = {88, 112, 144, 255};
            palette.autosave_icon_in_progress = {156, 196, 246, 255};
            palette.autosave_icon_success = {128, 212, 168, 255};
            palette.autosave_icon_failure = {232, 124, 116, 255};
            palette.autosave_text_in_progress = {204, 222, 248, 255};
            palette.autosave_text_success = {222, 246, 228, 255};
            palette.autosave_text_failure = {255, 226, 222, 255};
            palette.sync_heading = {240, 246, 255, 255};
            palette.sync_idle = {202, 216, 240, 255};
            palette.sync_pending = {210, 232, 248, 255};
            palette.sync_success = {226, 248, 232, 255};
            palette.sync_failure = {255, 228, 220, 255};
            palette.sync_background = {32, 48, 78, 235};
            palette.sync_border = {126, 162, 214, 255};
            palette.achievements_heading = {248, 252, 255, 255};
            palette.achievements_primary = {214, 234, 248, 255};
            palette.achievements_secondary = {202, 220, 240, 255};
            palette.achievements_background = {36, 54, 84, 235};
            palette.achievements_border = {138, 178, 226, 255};
            palette.performance_heading = {244, 250, 255, 255};
            palette.performance_value = {210, 222, 244, 255};
            palette.performance_warning = {255, 232, 208, 255};
            palette.performance_background = {32, 52, 80, 230};
            palette.performance_border = {132, 168, 220, 255};
        }

        brightness_percent = clamp_percent(brightness_percent, PLAYER_PROFILE_BRIGHTNESS_MIN_PERCENT,
            PLAYER_PROFILE_BRIGHTNESS_MAX_PERCENT);
        contrast_percent = clamp_percent(contrast_percent, PLAYER_PROFILE_CONTRAST_MIN_PERCENT,
            PLAYER_PROFILE_CONTRAST_MAX_PERCENT);
        apply_levels(palette, brightness_percent, contrast_percent);

        return palette;
    }
#endif
}

namespace
{
    ft_string resolve_menu_description(const ft_ui_menu &menu)
    {
        const ft_menu_item *hovered = menu.get_hovered_item();
        if (hovered != ft_nullptr && !hovered->description.empty())
            return hovered->description;

        const ft_menu_item *selected = menu.get_selected_item();
        if (selected != ft_nullptr)
            return selected->description;

        return ft_string();
    }

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

    ft_string resolve_menu_navigation_hint(const ft_ui_menu &menu, const PlayerProfilePreferences *preferences)
    {
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
            controller_nav_placeholder.value
                = format_menu_controller_label(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_UP);
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
            keyboard_confirm = format_menu_hotkey_label(preferences->hotkey_menu_confirm,
                "Enter");
            keyboard_cancel = format_menu_hotkey_label(preferences->hotkey_menu_cancel,
                "Esc");
            navigation_label = format_menu_hotkey_label(preferences->hotkey_menu_up,
                "Arrow Keys");
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
            ft_string controller_nav_up
                = format_menu_controller_label(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_UP);
            ft_string controller_nav_down
                = format_menu_controller_label(PLAYER_PROFILE_DEFAULT_CONTROLLER_MENU_DOWN);
            if (controller_nav_up == controller_nav_down)
                controller_navigation_label = controller_nav_up;
            else
            {
                controller_navigation_label = controller_nav_up;
                controller_navigation_label.append("/");
                controller_navigation_label.append(controller_nav_down);
            }
        }

        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(7U);
        StringTableReplacement confirm_placeholder;
        confirm_placeholder.key = ft_string("confirm");
        confirm_placeholder.value = keyboard_confirm;
        replacements.push_back(confirm_placeholder);
        StringTableReplacement item_placeholder;
        item_placeholder.key = ft_string("item");
        item_placeholder.value = reference->label;
        replacements.push_back(item_placeholder);
        StringTableReplacement navigate_placeholder;
        navigate_placeholder.key = ft_string("navigate");
        navigate_placeholder.value = navigation_label;
        replacements.push_back(navigate_placeholder);
        StringTableReplacement cancel_placeholder;
        cancel_placeholder.key = ft_string("cancel");
        cancel_placeholder.value = keyboard_cancel;
        replacements.push_back(cancel_placeholder);
        StringTableReplacement controller_confirm_placeholder;
        controller_confirm_placeholder.key = ft_string("controller_confirm");
        controller_confirm_placeholder.value = controller_confirm;
        replacements.push_back(controller_confirm_placeholder);
        StringTableReplacement controller_cancel_placeholder;
        controller_cancel_placeholder.key = ft_string("controller_cancel");
        controller_cancel_placeholder.value = controller_cancel;
        replacements.push_back(controller_cancel_placeholder);
        StringTableReplacement controller_nav_placeholder;
        controller_nav_placeholder.key = ft_string("controller_navigate");
        controller_nav_placeholder.value = controller_navigation_label;
        replacements.push_back(controller_nav_placeholder);
        return menu_localize_format("main_menu.navigation.active",
            "{{confirm}} / {{controller_confirm}}: Select {{item}}  |  {{navigate}} / {{controller_navigate}}: Navigate  |  {{cancel}} / {{controller_cancel}}: Back",
            replacements);
    }

#if GALACTIC_HAVE_SDL2
    void render_menu_overlay(SDL_Renderer &renderer, TTF_Font *menu_font, int output_width, int output_height,
        const MainMenuOverlayContext *overlay, const MainMenuPalette &palette)
    {
        if (overlay == ft_nullptr || !overlay->visible)
            return;
        if (menu_font == ft_nullptr)
            return;

        const int overlay_padding = 24;
        const int min_overlay_width = 540;
        const int blank_line_height = 18;
        const int line_spacing = 6;
        int       overlay_height = overlay_padding * 2;
        int       max_content_width = 0;

        SDL_Texture *heading_texture = ft_nullptr;
        SDL_Rect     heading_rect;
        heading_rect.x = 0;
        heading_rect.y = 0;
        heading_rect.w = 0;
        heading_rect.h = 0;

        if (!overlay->heading.empty())
        {
            SDL_Color heading_color = palette.overlay_heading;
            heading_texture = create_text_texture(renderer, *menu_font, overlay->heading, heading_color, heading_rect);
            if (heading_texture != ft_nullptr)
            {
                overlay_height += heading_rect.h;
                max_content_width = heading_rect.w;
                overlay_height += 16;
            }
        }

        const ft_vector<ft_string> &lines = overlay->lines;
        ft_vector<SDL_Texture *>     line_textures;
        ft_vector<SDL_Rect>          line_rects;
        line_textures.reserve(lines.size());
        line_rects.reserve(lines.size());

        for (size_t index = 0; index < lines.size(); ++index)
        {
            SDL_Texture *line_texture = ft_nullptr;
            SDL_Rect     line_rect;
            line_rect.x = 0;
            line_rect.y = 0;
            line_rect.w = 0;
            line_rect.h = 0;

            if (lines[index].empty())
            {
                line_rect.h = blank_line_height;
                overlay_height += line_rect.h;
                line_textures.push_back(ft_nullptr);
                line_rects.push_back(line_rect);
            }
            else
            {
                SDL_Color line_color = palette.overlay_line;
                line_texture = create_text_texture(renderer, *menu_font, lines[index], line_color, line_rect);
                if (line_texture != ft_nullptr)
                {
                    overlay_height += line_rect.h;
                    if (static_cast<int>(line_rect.w) > max_content_width)
                        max_content_width = line_rect.w;
                }
                else
                {
                    line_rect.h = blank_line_height;
                    overlay_height += line_rect.h;
                }
                line_textures.push_back(line_texture);
                line_rects.push_back(line_rect);
            }

            if (index + 1U < lines.size())
                overlay_height += line_spacing;
        }

        SDL_Texture *footer_texture = ft_nullptr;
        SDL_Rect     footer_rect;
        footer_rect.x = 0;
        footer_rect.y = 0;
        footer_rect.w = 0;
        footer_rect.h = 0;

        if (!overlay->footer.empty())
        {
            SDL_Color footer_color = palette.overlay_footer;
            footer_texture = create_text_texture(renderer, *menu_font, overlay->footer, footer_color, footer_rect);
            if (footer_texture != ft_nullptr)
            {
                overlay_height += footer_rect.h;
                if (static_cast<int>(footer_rect.w) > max_content_width)
                    max_content_width = footer_rect.w;
                overlay_height += 12;
            }
        }

        if (heading_texture == ft_nullptr && line_textures.empty() && footer_texture == ft_nullptr)
            return;

        int overlay_width = overlay_padding * 2 + max_content_width;
        if (overlay_width < min_overlay_width)
            overlay_width = min_overlay_width;
        const int available_width = output_width - 2 * overlay_padding;
        if (available_width > 0 && overlay_width > available_width)
            overlay_width = available_width;
        if (overlay_width <= 0)
            overlay_width = min_overlay_width;

        SDL_Rect overlay_rect;
        overlay_rect.w = overlay_width;
        overlay_rect.h = overlay_height;
        overlay_rect.x = output_width / 2 - overlay_rect.w / 2;
        overlay_rect.y = output_height / 2 - overlay_rect.h / 2;
        if (overlay_rect.x < 48)
            overlay_rect.x = 48;
        if (overlay_rect.y < 48)
            overlay_rect.y = 48;

        SDL_SetRenderDrawColor(&renderer, palette.overlay_background.r, palette.overlay_background.g,
            palette.overlay_background.b, palette.overlay_background.a);
        SDL_RenderFillRect(&renderer, &overlay_rect);
        SDL_SetRenderDrawColor(&renderer, palette.overlay_border.r, palette.overlay_border.g, palette.overlay_border.b,
            palette.overlay_border.a);
        SDL_RenderDrawRect(&renderer, &overlay_rect);

        int text_x = overlay_rect.x + overlay_padding;
        int text_y = overlay_rect.y + overlay_padding;

        if (heading_texture != ft_nullptr)
        {
            heading_rect.x = text_x;
            heading_rect.y = text_y;
            SDL_RenderCopy(&renderer, heading_texture, ft_nullptr, &heading_rect);
            text_y += heading_rect.h + 12;
        }

        for (size_t index = 0; index < line_textures.size(); ++index)
        {
            SDL_Texture *line_texture = line_textures[index];
            SDL_Rect      line_rect = line_rects[index];
            line_rect.x = text_x;
            line_rect.y = text_y;
            if (line_texture != ft_nullptr)
                SDL_RenderCopy(&renderer, line_texture, ft_nullptr, &line_rect);
            text_y += line_rect.h;
            if (index + 1U < line_textures.size())
                text_y += line_spacing;
            if (line_texture != ft_nullptr)
                SDL_DestroyTexture(line_texture);
        }

        if (footer_texture != ft_nullptr)
        {
            footer_rect.x = text_x;
            footer_rect.y = text_y + 12;
            SDL_RenderCopy(&renderer, footer_texture, ft_nullptr, &footer_rect);
            SDL_DestroyTexture(footer_texture);
        }

        if (heading_texture != ft_nullptr)
            SDL_DestroyTexture(heading_texture);
    }

    void render_menu_tutorial_overlay(SDL_Renderer &renderer, TTF_Font *menu_font, int output_width,
        const MainMenuTutorialContext *tutorial, const MainMenuPalette &palette)
    {
        if (tutorial == ft_nullptr || !tutorial->visible)
            return;
        if (menu_font == ft_nullptr)
            return;

        const ft_vector<ft_string> *tips_pointer = tutorial->tips;
        if (tips_pointer == ft_nullptr)
            tips_pointer = &get_main_menu_tutorial_tips();

        const ft_vector<ft_string> &tips = *tips_pointer;
        if (tips.empty())
            return;

        const int overlay_padding = 20;
        const int min_overlay_width = 540;
        int       overlay_height = overlay_padding * 2;
        int       max_content_width = 0;

        SDL_Texture *title_texture = ft_nullptr;
        SDL_Rect     title_rect;
        title_rect.x = 0;
        title_rect.y = 0;
        title_rect.w = 0;
        title_rect.h = 0;

        ft_string title_label = menu_localize("main_menu.tutorial.title", "Commander Tips");
        if (!title_label.empty())
        {
            SDL_Color title_color = palette.tutorial_title;
            title_texture = create_text_texture(renderer, *menu_font, title_label, title_color, title_rect);
            if (title_texture != ft_nullptr)
            {
                overlay_height += title_rect.h;
                max_content_width = title_rect.w;
                overlay_height += 18;
            }
        }

        ft_vector<SDL_Texture *> tip_textures;
        ft_vector<SDL_Rect>      tip_rects;
        tip_textures.reserve(tips.size());
        tip_rects.reserve(tips.size());

        SDL_Color primary_tip_color = palette.tutorial_primary;
        SDL_Color bullet_tip_color = palette.tutorial_secondary;

        for (size_t index = 0; index < tips.size(); ++index)
        {
            bool is_primary_tip = index == 0;
            ft_string display_text;
            if (is_primary_tip)
                display_text = tips[index];
            else
            {
                display_text = ft_string("\xE2\x80\xA2 ");
                display_text.append(tips[index]);
            }

            SDL_Rect tip_rect;
            SDL_Color tip_color = is_primary_tip ? primary_tip_color : bullet_tip_color;
            SDL_Texture *tip_texture = create_text_texture(renderer, *menu_font, display_text, tip_color, tip_rect);
            if (tip_texture != ft_nullptr)
            {
                tip_textures.push_back(tip_texture);
                tip_rects.push_back(tip_rect);
                overlay_height += tip_rect.h;
                if (static_cast<int>(tip_rect.w) > max_content_width)
                    max_content_width = tip_rect.w;
                if (index + 1U < tips.size())
                    overlay_height += 8;
            }
        }

        if (tip_textures.empty() && title_texture == ft_nullptr)
        {
            return;
        }

        int overlay_width = overlay_padding * 2 + max_content_width;
        if (overlay_width < min_overlay_width)
            overlay_width = min_overlay_width;
        const int available_width = output_width - 2 * overlay_padding;
        if (available_width > 0 && overlay_width > available_width)
            overlay_width = available_width;
        if (overlay_width <= 0)
            overlay_width = min_overlay_width;

        SDL_Rect overlay_rect;
        overlay_rect.x = 72;
        overlay_rect.y = 84;
        overlay_rect.w = overlay_width;
        overlay_rect.h = overlay_height;

        SDL_SetRenderDrawColor(&renderer, palette.tutorial_background.r, palette.tutorial_background.g,
            palette.tutorial_background.b, palette.tutorial_background.a);
        SDL_RenderFillRect(&renderer, &overlay_rect);
        SDL_SetRenderDrawColor(&renderer, palette.tutorial_border.r, palette.tutorial_border.g, palette.tutorial_border.b,
            palette.tutorial_border.a);
        SDL_RenderDrawRect(&renderer, &overlay_rect);

        int text_x = overlay_rect.x + overlay_padding;
        int text_y = overlay_rect.y + overlay_padding;

        if (title_texture != ft_nullptr)
        {
            title_rect.x = text_x;
            title_rect.y = text_y;
            SDL_RenderCopy(&renderer, title_texture, ft_nullptr, &title_rect);
            text_y += title_rect.h + 12;
        }

        for (size_t index = 0; index < tip_textures.size(); ++index)
        {
            SDL_Texture *tip_texture = tip_textures[index];
            SDL_Rect      tip_rect = tip_rects[index];
            tip_rect.x = text_x;
            tip_rect.y = text_y;
            SDL_RenderCopy(&renderer, tip_texture, ft_nullptr, &tip_rect);
            text_y += tip_rect.h;
            if (index + 1U < tip_textures.size())
                text_y += 8;
            SDL_DestroyTexture(tip_texture);
        }

        if (title_texture != ft_nullptr)
            SDL_DestroyTexture(title_texture);
    }
#endif
}

void render_main_menu(SDL_Renderer &renderer, const ft_ui_menu &menu, TTF_Font *title_font, TTF_Font *menu_font,
    int window_width, int window_height, const ft_string &active_profile_name, const PlayerProfilePreferences *preferences,
    const MainMenuTutorialContext *tutorial,
    const MainMenuOverlayContext *manual, const MainMenuOverlayContext *changelog,
    const MainMenuOverlayContext *cloud_confirmation, const MainMenuOverlayContext *crash_prompt,
    const MainMenuConnectivityStatus *connectivity,
    const MainMenuSyncStatus *sync_status, const MainMenuAchievementsSummary *achievements,
    const MainMenuAutosaveStatus *autosave, const MainMenuPerformanceStats *performance, const MainMenuAlertBanner *alert)
{
#if GALACTIC_HAVE_SDL2
    const bool use_colorblind_palette
        = preferences != ft_nullptr && preferences->colorblind_palette_enabled;
    unsigned int brightness_percent = 100U;
    unsigned int contrast_percent = 100U;
    if (preferences != ft_nullptr)
    {
        brightness_percent = preferences->brightness_percent;
        contrast_percent = preferences->contrast_percent;
    }
    MainMenuPalette palette
        = resolve_main_menu_palette(use_colorblind_palette, brightness_percent, contrast_percent);

    SDL_SetRenderDrawColor(&renderer, palette.background.r, palette.background.g, palette.background.b,
        palette.background.a);
    SDL_RenderClear(&renderer);

    int output_width = window_width;
    int output_height = window_height;
    SDL_GetRendererOutputSize(&renderer, &output_width, &output_height);

    if (title_font != ft_nullptr)
    {
        SDL_Color title_color = palette.title;
        SDL_Rect  title_rect;
        ft_string menu_title = menu_localize("main_menu.title", "Galactic Planet Miner");
        SDL_Texture *title_texture = create_text_texture(renderer, *title_font, menu_title, title_color,
            title_rect);
        if (title_texture != ft_nullptr)
        {
            title_rect.x = output_width / 2 - title_rect.w / 2;
            title_rect.y = 96;
            SDL_RenderCopy(&renderer, title_texture, ft_nullptr, &title_rect);
            SDL_DestroyTexture(title_texture);
        }
    }

    if (menu_font != ft_nullptr && achievements != ft_nullptr)
    {
        SDL_Color heading_color = palette.achievements_heading;
        SDL_Color line_primary_color = palette.achievements_primary;
        SDL_Color line_secondary_color = palette.achievements_secondary;
        SDL_Rect  heading_rect;
        ft_string heading_label = menu_localize("main_menu.achievements.heading", "Achievements");
        SDL_Texture *heading_texture
            = create_text_texture(renderer, *menu_font, heading_label, heading_color, heading_rect);
        ft_vector<ft_string> lines = main_menu_collect_achievement_lines(*achievements);
        ft_vector<SDL_Texture *> line_textures;
        ft_vector<SDL_Rect>      line_rects;
        line_textures.reserve(lines.size());
        line_rects.reserve(lines.size());

        int panel_padding = 12;
        int line_spacing = 2;
        int panel_height = panel_padding * 2;
        int panel_width = 0;
        if (heading_texture != ft_nullptr)
        {
            panel_height += heading_rect.h;
            panel_height += 10;
            panel_width = heading_rect.w;
        }

        for (size_t index = 0; index < lines.size(); ++index)
        {
            SDL_Color line_color = (index == 0) ? line_primary_color : line_secondary_color;
            SDL_Rect  line_rect;
            SDL_Texture *line_texture = create_text_texture(renderer, *menu_font, lines[index], line_color, line_rect);
            if (line_texture != ft_nullptr)
            {
                line_textures.push_back(line_texture);
                line_rects.push_back(line_rect);
                panel_height += line_rect.h;
                if (static_cast<int>(line_rect.w) > panel_width)
                    panel_width = line_rect.w;
                if (index + 1U < lines.size())
                    panel_height += line_spacing;
            }
        }

        if (!line_textures.empty() || heading_texture != ft_nullptr)
        {
            int min_panel_width = 360;
            int overlay_margin = 32;
            panel_width = panel_padding * 2 + panel_width;
            if (panel_width < min_panel_width)
                panel_width = min_panel_width;
            SDL_Rect panel_rect;
            panel_rect.x = overlay_margin;
            panel_rect.y = 140;
            panel_rect.w = panel_width;
            panel_rect.h = panel_height;
            SDL_SetRenderDrawColor(&renderer, palette.achievements_background.r, palette.achievements_background.g,
                palette.achievements_background.b, palette.achievements_background.a);
            SDL_RenderFillRect(&renderer, &panel_rect);
            SDL_SetRenderDrawColor(&renderer, palette.achievements_border.r, palette.achievements_border.g,
                palette.achievements_border.b, palette.achievements_border.a);
            SDL_RenderDrawRect(&renderer, &panel_rect);

            int text_x = panel_rect.x + panel_padding;
            int text_y = panel_rect.y + panel_padding;
            if (heading_texture != ft_nullptr)
            {
                heading_rect.x = text_x;
                heading_rect.y = text_y;
                SDL_RenderCopy(&renderer, heading_texture, ft_nullptr, &heading_rect);
                text_y += heading_rect.h + 10;
            }

            for (size_t index = 0; index < line_textures.size(); ++index)
            {
                SDL_Texture *line_texture = line_textures[index];
                SDL_Rect      line_rect = line_rects[index];
                line_rect.x = text_x;
                line_rect.y = text_y;
                SDL_RenderCopy(&renderer, line_texture, ft_nullptr, &line_rect);
                text_y += line_rect.h;
                if (index + 1U < line_textures.size())
                    text_y += line_spacing;
                SDL_DestroyTexture(line_texture);
            }
        }

        for (size_t index = 0; index < line_textures.size(); ++index)
        {
            SDL_Texture *line_texture = line_textures[index];
            if (line_texture != ft_nullptr)
                SDL_DestroyTexture(line_texture);
        }
        if (heading_texture != ft_nullptr)
            SDL_DestroyTexture(heading_texture);
    }

    if (menu_font != ft_nullptr && autosave != ft_nullptr && main_menu_autosave_is_visible(*autosave, ft_time_ms()))
    {
        ft_string autosave_label = main_menu_resolve_autosave_label(*autosave);
        SDL_Color icon_color = palette.autosave_icon_idle;
        SDL_Color text_color = palette.autosave_text_in_progress;
        if (autosave->state == MAIN_MENU_AUTOSAVE_SUCCEEDED)
        {
            icon_color = palette.autosave_icon_success;
            text_color = palette.autosave_text_success;
        }
        else if (autosave->state == MAIN_MENU_AUTOSAVE_FAILED)
        {
            icon_color = palette.autosave_icon_failure;
            text_color = palette.autosave_text_failure;
        }

        SDL_Rect icon_rect;
        icon_rect.w = 12;
        icon_rect.h = 12;
        icon_rect.x = 48;
        icon_rect.y = output_height - 140;
        SDL_SetRenderDrawColor(&renderer, icon_color.r, icon_color.g, icon_color.b, icon_color.a);
        SDL_RenderFillRect(&renderer, &icon_rect);

        SDL_Rect text_rect;
        SDL_Texture *text_texture = create_text_texture(renderer, *menu_font, autosave_label, text_color, text_rect);
        if (text_texture != ft_nullptr)
        {
            text_rect.x = icon_rect.x + icon_rect.w + 12;
            text_rect.y = icon_rect.y - text_rect.h / 2 + icon_rect.h / 2;
            SDL_RenderCopy(&renderer, text_texture, ft_nullptr, &text_rect);
            SDL_DestroyTexture(text_texture);
        }
    }

    ft_rect menu_viewport = build_main_menu_viewport();
    SDL_Rect viewport_rect;
    viewport_rect.x = menu_viewport.left;
    viewport_rect.y = menu_viewport.top;
    viewport_rect.w = menu_viewport.width;
    viewport_rect.h = menu_viewport.height;
    SDL_Rect output_rect;
    output_rect.x = viewport_rect.x;
    output_rect.y = viewport_rect.y;
    output_rect.w = viewport_rect.w;
    output_rect.h = viewport_rect.h;
    if (output_rect.x < 0)
        output_rect.x = 0;
    if (output_rect.y < 0)
        output_rect.y = 0;
    if (output_rect.w < 0)
        output_rect.w = 0;
    if (output_rect.h < 0)
        output_rect.h = 0;
    SDL_RenderSetViewport(&renderer, &output_rect);

    menu.render(renderer, menu_font, palette.button_idle, palette.button_hover, palette.button_selected,
        palette.button_disabled, palette.button_disabled_hover, palette.button_border_enabled,
        palette.button_border_disabled, palette.button_text_enabled, palette.button_text_disabled);

    SDL_RenderSetViewport(&renderer, ft_nullptr);

    int next_text_top = viewport_rect.y + viewport_rect.h + 28;

    if (menu_font != ft_nullptr)
    {
        const ft_string description = resolve_menu_description(menu);
        if (!description.empty())
        {
            SDL_Color description_color = palette.description;
            SDL_Rect  description_rect;
            SDL_Texture *description_texture
                = create_text_texture(renderer, *menu_font, description, description_color, description_rect);
            if (description_texture != ft_nullptr)
            {
                description_rect.x = output_width / 2 - description_rect.w / 2;
                description_rect.y = next_text_top;
                SDL_RenderCopy(&renderer, description_texture, ft_nullptr, &description_rect);
                SDL_DestroyTexture(description_texture);
                next_text_top = description_rect.y + description_rect.h + 28;
            }
        }
    }

    if (menu_font != ft_nullptr)
    {
        const ft_string hint = resolve_menu_navigation_hint(menu, preferences);
        if (!hint.empty())
        {
            SDL_Color hint_color = palette.hint;
            SDL_Rect  hint_rect;
            SDL_Texture *hint_texture = create_text_texture(renderer, *menu_font, hint, hint_color, hint_rect);
            if (hint_texture != ft_nullptr)
            {
                hint_rect.x = output_width / 2 - hint_rect.w / 2;
                hint_rect.y = next_text_top;
                SDL_RenderCopy(&renderer, hint_texture, ft_nullptr, &hint_rect);
                SDL_DestroyTexture(hint_texture);
                next_text_top = hint_rect.y + hint_rect.h + 20;
            }
        }
    }

    if (menu_font != ft_nullptr)
    {
        const ft_string build_label = main_menu_resolve_build_label();
        if (!build_label.empty())
        {
            SDL_Color build_color = palette.build;
            SDL_Rect  build_rect;
            SDL_Texture *build_texture = create_text_texture(renderer, *menu_font, build_label, build_color, build_rect);
            if (build_texture != ft_nullptr)
            {
                build_rect.x = output_width - build_rect.w - 48;
                build_rect.y = output_height - build_rect.h - 40;
                SDL_RenderCopy(&renderer, build_texture, ft_nullptr, &build_rect);
                SDL_DestroyTexture(build_texture);
            }
        }
    }

    render_menu_overlay(renderer, menu_font, output_width, output_height, manual, palette);
    render_menu_overlay(renderer, menu_font, output_width, output_height, changelog, palette);
    render_menu_overlay(renderer, menu_font, output_width, output_height, cloud_confirmation, palette);
    render_menu_overlay(renderer, menu_font, output_width, output_height, crash_prompt, palette);
    render_menu_tutorial_overlay(renderer, menu_font, output_width, tutorial, palette);

    SDL_RenderPresent(&renderer);
#else
    (void)renderer;
    (void)menu;
    (void)title_font;
    (void)menu_font;
    (void)window_width;
    (void)window_height;
    (void)active_profile_name;
    (void)preferences;
    (void)tutorial;
    (void)manual;
    (void)changelog;
    (void)cloud_confirmation;
    (void)crash_prompt;
    (void)connectivity;
    (void)sync_status;
    (void)achievements;
    (void)autosave;
    (void)performance;
    (void)alert;
#endif
}

namespace main_menu_testing
{
    ft_string resolve_active_description(const ft_ui_menu &menu)
    {
        return resolve_menu_description(menu);
    }

    ft_string resolve_navigation_hint(
        const ft_ui_menu &menu, const PlayerProfilePreferences *preferences)
    {
        return resolve_menu_navigation_hint(menu, preferences);
    }
}
