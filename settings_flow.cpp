#include "main_menu_system.hpp"

#include "app_constants.hpp"
#include "player_profile.hpp"
#include "ui_input.hpp"

#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/Libft/libft.hpp"

namespace
{
    const unsigned int kUiScaleStep = 5U;
    const unsigned int kCombatSpeedStep = 5U;
    const unsigned int kVolumeStep = 5U;

    unsigned int clamp_ui_scale(unsigned int value) noexcept
    {
        if (value < PLAYER_PROFILE_UI_SCALE_MIN_PERCENT)
            return PLAYER_PROFILE_UI_SCALE_MIN_PERCENT;
        if (value > PLAYER_PROFILE_UI_SCALE_MAX_PERCENT)
            return PLAYER_PROFILE_UI_SCALE_MAX_PERCENT;
        return value;
    }

    unsigned int clamp_combat_speed(unsigned int value) noexcept
    {
        if (value < PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT)
            return PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT;
        if (value > PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT)
            return PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT;
        return value;
    }

    unsigned int clamp_volume(unsigned int value) noexcept
    {
        if (value < PLAYER_PROFILE_VOLUME_MIN_PERCENT)
            return PLAYER_PROFILE_VOLUME_MIN_PERCENT;
        if (value > PLAYER_PROFILE_VOLUME_MAX_PERCENT)
            return PLAYER_PROFILE_VOLUME_MAX_PERCENT;
        return value;
    }

    unsigned int increment_ui_scale(unsigned int value) noexcept
    {
        if (value >= PLAYER_PROFILE_UI_SCALE_MAX_PERCENT)
            return PLAYER_PROFILE_UI_SCALE_MAX_PERCENT;
        unsigned int candidate = value + kUiScaleStep;
        if (candidate < value)
            return PLAYER_PROFILE_UI_SCALE_MAX_PERCENT;
        return clamp_ui_scale(candidate);
    }

    unsigned int decrement_ui_scale(unsigned int value) noexcept
    {
        if (value <= PLAYER_PROFILE_UI_SCALE_MIN_PERCENT)
            return PLAYER_PROFILE_UI_SCALE_MIN_PERCENT;
        if (value <= kUiScaleStep)
            return PLAYER_PROFILE_UI_SCALE_MIN_PERCENT;
        unsigned int candidate = value - kUiScaleStep;
        if (candidate > value)
            return PLAYER_PROFILE_UI_SCALE_MIN_PERCENT;
        return clamp_ui_scale(candidate);
    }

    unsigned int increment_combat_speed(unsigned int value) noexcept
    {
        if (value >= PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT)
            return PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT;
        unsigned int candidate = value + kCombatSpeedStep;
        if (candidate < value)
            return PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT;
        return clamp_combat_speed(candidate);
    }

    unsigned int decrement_combat_speed(unsigned int value) noexcept
    {
        if (value <= PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT)
            return PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT;
        if (value <= kCombatSpeedStep)
            return PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT;
        unsigned int candidate = value - kCombatSpeedStep;
        if (candidate > value)
            return PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT;
        return clamp_combat_speed(candidate);
    }

    unsigned int increment_volume(unsigned int value) noexcept
    {
        if (value >= PLAYER_PROFILE_VOLUME_MAX_PERCENT)
            return PLAYER_PROFILE_VOLUME_MAX_PERCENT;
        unsigned int candidate = value + kVolumeStep;
        if (candidate < value)
            return PLAYER_PROFILE_VOLUME_MAX_PERCENT;
        return clamp_volume(candidate);
    }

    unsigned int decrement_volume(unsigned int value) noexcept
    {
        if (value <= PLAYER_PROFILE_VOLUME_MIN_PERCENT)
            return PLAYER_PROFILE_VOLUME_MIN_PERCENT;
        if (value <= kVolumeStep)
            return PLAYER_PROFILE_VOLUME_MIN_PERCENT;
        unsigned int candidate = value - kVolumeStep;
        if (candidate > value)
            return PLAYER_PROFILE_VOLUME_MIN_PERCENT;
        return clamp_volume(candidate);
    }

    unsigned int toggle_lore_anchor(unsigned int anchor) noexcept
    {
        if (anchor == PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT)
            return PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT;
        return PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT;
    }

    ft_string format_ui_scale_label(unsigned int value)
    {
        ft_string label("UI Scale: ");
        label.append(ft_to_string(static_cast<int>(value)));
        label.append("%");
        return label;
    }

    ft_string format_combat_speed_label(unsigned int value)
    {
        ft_string label("Combat Speed: ");
        label.append(ft_to_string(static_cast<int>(value)));
        label.append("%");
        return label;
    }

    ft_string format_music_volume_label(unsigned int value)
    {
        ft_string label("Music Volume: ");
        label.append(ft_to_string(static_cast<int>(value)));
        label.append("%");
        return label;
    }

    ft_string format_effects_volume_label(unsigned int value)
    {
        ft_string label("Effects Volume: ");
        label.append(ft_to_string(static_cast<int>(value)));
        label.append("%");
        return label;
    }

    ft_string format_lore_anchor_label(unsigned int anchor)
    {
        ft_string label("Lore Panel Anchor: ");
        if (anchor == PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT)
            label.append("Left");
        else
            label.append("Right");
        return label;
    }

#if GALACTIC_HAVE_SDL2
    bool preferences_equal(const PlayerProfilePreferences &lhs, const PlayerProfilePreferences &rhs) noexcept
    {
        if (lhs.commander_name != rhs.commander_name)
            return false;
        if (lhs.ui_scale_percent != rhs.ui_scale_percent)
            return false;
        if (lhs.combat_speed_percent != rhs.combat_speed_percent)
            return false;
        if (lhs.music_volume_percent != rhs.music_volume_percent)
            return false;
        if (lhs.effects_volume_percent != rhs.effects_volume_percent)
            return false;
        if (lhs.lore_panel_anchor != rhs.lore_panel_anchor)
            return false;
        if (lhs.window_width != rhs.window_width)
            return false;
        if (lhs.window_height != rhs.window_height)
            return false;
        if (lhs.menu_tutorial_seen != rhs.menu_tutorial_seen)
            return false;
        return true;
    }

    void rebuild_settings_menu(ft_ui_menu &menu, const PlayerProfilePreferences &preferences, bool allow_save,
        const ft_string *preferred_identifier)
    {
        const ft_rect base_rect(460, 220, 360, 56);
        const int      spacing = 18;

        ft_vector<ft_menu_item> items;
        items.reserve(7U);

        ft_menu_item ui_scale_item(ft_string("setting:ui_scale"), format_ui_scale_label(preferences.ui_scale_percent), base_rect);
        items.push_back(ui_scale_item);

        ft_rect combat_rect = base_rect;
        combat_rect.top += base_rect.height + spacing;
        ft_menu_item combat_item(
            ft_string("setting:combat_speed"), format_combat_speed_label(preferences.combat_speed_percent), combat_rect);
        items.push_back(combat_item);

        ft_rect anchor_rect = base_rect;
        anchor_rect.top += 4 * (base_rect.height + spacing);

        ft_rect music_rect = base_rect;
        music_rect.top += 2 * (base_rect.height + spacing);
        ft_menu_item music_item(
            ft_string("setting:music_volume"), format_music_volume_label(preferences.music_volume_percent), music_rect);
        items.push_back(music_item);

        ft_rect effects_rect = base_rect;
        effects_rect.top += 3 * (base_rect.height + spacing);
        ft_menu_item effects_item(
            ft_string("setting:effects_volume"), format_effects_volume_label(preferences.effects_volume_percent), effects_rect);
        items.push_back(effects_item);

        ft_menu_item anchor_item(ft_string("setting:lore_anchor"), format_lore_anchor_label(preferences.lore_panel_anchor),
            anchor_rect);
        items.push_back(anchor_item);

        ft_rect save_rect = base_rect;
        save_rect.top += 5 * (base_rect.height + spacing);
        ft_menu_item save_item(ft_string("action:save"), ft_string("Save Changes"), save_rect);
        save_item.enabled = allow_save;
        items.push_back(save_item);

        ft_rect cancel_rect = base_rect;
        cancel_rect.top += 6 * (base_rect.height + spacing);
        ft_menu_item cancel_item(ft_string("action:cancel"), ft_string("Cancel"), cancel_rect);
        items.push_back(cancel_item);

        menu.set_items(items);

        ft_rect viewport = base_rect;
        const int window_height = static_cast<int>(app_constants::kWindowHeight);
        const int reserved_bottom = 220;
        viewport.height = window_height - base_rect.top - reserved_bottom;
        if (viewport.height < base_rect.height)
            viewport.height = base_rect.height;
        menu.set_viewport_bounds(viewport);

        if (preferred_identifier != ft_nullptr)
        {
            const ft_vector<ft_menu_item> &current_items = menu.get_items();
            for (size_t index = 0; index < current_items.size(); ++index)
            {
                if (current_items[index].identifier == *preferred_identifier && current_items[index].enabled)
                {
                    menu.set_selected_index(static_cast<int>(index));
                    break;
                }
            }
        }
    }

    void render_settings_screen(SDL_Renderer &renderer, const ft_ui_menu &menu, TTF_Font *title_font, TTF_Font *menu_font,
        const ft_string &status_message, bool status_is_error, bool has_unsaved_changes)
    {
        SDL_SetRenderDrawColor(&renderer, 12, 16, 28, 255);
        SDL_RenderClear(&renderer);

        int output_width = static_cast<int>(app_constants::kWindowWidth);
        int output_height = static_cast<int>(app_constants::kWindowHeight);
        SDL_GetRendererOutputSize(&renderer, &output_width, &output_height);

        if (title_font != ft_nullptr)
        {
            SDL_Color title_color = {220, 220, 245, 255};
            SDL_Rect  title_rect;
            SDL_Texture *title_texture
                = create_text_texture(renderer, *title_font, ft_string("Commander Settings"), title_color, title_rect);
            if (title_texture != ft_nullptr)
            {
                title_rect.x = output_width / 2 - title_rect.w / 2;
                title_rect.y = 96;
                SDL_RenderCopy(&renderer, title_texture, ft_nullptr, &title_rect);
                SDL_DestroyTexture(title_texture);
            }
        }

        if (menu_font != ft_nullptr)
        {
            SDL_Color info_color = {170, 180, 210, 255};
            SDL_Rect  info_rect;
            ft_string info_text("Use Left/Right or Enter to adjust an option. Esc cancels. Save commits changes.");
            SDL_Texture *info_texture = create_text_texture(renderer, *menu_font, info_text, info_color, info_rect);
            if (info_texture != ft_nullptr)
            {
                info_rect.x = output_width / 2 - info_rect.w / 2;
                info_rect.y = 164;
                SDL_RenderCopy(&renderer, info_texture, ft_nullptr, &info_rect);
                SDL_DestroyTexture(info_texture);
            }
        }

        const ft_vector<ft_menu_item> &items = menu.get_items();
        const int hovered_index = menu.get_hovered_index();
        const int selected_index = menu.get_selected_index();

        const ft_rect &viewport = menu.get_viewport_bounds();
        const bool clip_enabled = viewport.width > 0 && viewport.height > 0;
        SDL_Rect clip_rect;
        clip_rect.x = 0;
        clip_rect.y = 0;
        clip_rect.w = 0;
        clip_rect.h = 0;
        int clip_bottom = 0;

        if (clip_enabled)
        {
            clip_rect.x = viewport.left;
            clip_rect.y = viewport.top;
            clip_rect.w = viewport.width;
            clip_rect.h = viewport.height;
            clip_bottom = clip_rect.y + clip_rect.h;
            SDL_RenderSetClipRect(&renderer, &clip_rect);
        }

        const int scroll_offset = menu.get_scroll_offset();

        for (size_t index = 0; index < items.size(); ++index)
        {
            const ft_menu_item &item = items[index];
            const bool          is_hovered = static_cast<int>(index) == hovered_index;
            const bool          is_selected = static_cast<int>(index) == selected_index;
            const bool          is_disabled = !item.enabled;

            Uint8 r = 28;
            Uint8 g = 36;
            Uint8 b = 60;

            if (is_disabled)
            {
                r = 30;
                g = 34;
                b = 44;
            }
            else if (is_hovered)
            {
                r = 56;
                g = 84;
                b = 140;
            }
            else if (is_selected)
            {
                r = 40;
                g = 64;
                b = 112;
            }

            SDL_Rect button_rect;
            button_rect.x = item.bounds.left;
            button_rect.y = item.bounds.top - scroll_offset;
            button_rect.w = item.bounds.width;
            button_rect.h = item.bounds.height;

            if (clip_enabled && (button_rect.y + button_rect.h <= clip_rect.y || button_rect.y >= clip_bottom))
                continue;

            SDL_SetRenderDrawColor(&renderer, r, g, b, 255);
            SDL_RenderFillRect(&renderer, &button_rect);

            const Uint8 border_r = is_disabled ? 70 : 90;
            const Uint8 border_g = is_disabled ? 80 : 110;
            const Uint8 border_b = is_disabled ? 120 : 160;
            SDL_SetRenderDrawColor(&renderer, border_r, border_g, border_b, 255);
            SDL_RenderDrawRect(&renderer, &button_rect);

            if (menu_font != ft_nullptr)
            {
                SDL_Color text_color = {255, 255, 255, 255};
                if (is_disabled)
                {
                    text_color.r = 188;
                    text_color.g = 196;
                    text_color.b = 210;
                }
                SDL_Rect text_rect;
                SDL_Texture *text_texture = create_text_texture(renderer, *menu_font, item.label, text_color, text_rect);
                if (text_texture != ft_nullptr)
                {
                    text_rect.x = item.bounds.left + 18;
                    text_rect.y = button_rect.y + (button_rect.h - text_rect.h) / 2;
                    SDL_RenderCopy(&renderer, text_texture, ft_nullptr, &text_rect);
                    SDL_DestroyTexture(text_texture);
                }
            }
        }

        if (clip_enabled)
            SDL_RenderSetClipRect(&renderer, ft_nullptr);

        if (menu_font != ft_nullptr && has_unsaved_changes)
        {
            SDL_Color dirty_color = {210, 200, 120, 255};
            SDL_Rect  dirty_rect;
            SDL_Texture *dirty_texture
                = create_text_texture(renderer, *menu_font, ft_string("Unsaved changes"), dirty_color, dirty_rect);
            if (dirty_texture != ft_nullptr)
            {
                dirty_rect.x = 80;
                dirty_rect.y = output_height - dirty_rect.h - 140;
                SDL_RenderCopy(&renderer, dirty_texture, ft_nullptr, &dirty_rect);
                SDL_DestroyTexture(dirty_texture);
            }
        }

        if (menu_font != ft_nullptr && !status_message.empty())
        {
            SDL_Color status_color;
            if (status_is_error)
            {
                status_color.r = 220;
                status_color.g = 90;
                status_color.b = 90;
                status_color.a = 255;
            }
            else
            {
                status_color.r = 180;
                status_color.g = 200;
                status_color.b = 220;
                status_color.a = 255;
            }

            SDL_Rect status_rect;
            SDL_Texture *status_texture
                = create_text_texture(renderer, *menu_font, status_message, status_color, status_rect);
            if (status_texture != ft_nullptr)
            {
                status_rect.x = output_width / 2 - status_rect.w / 2;
                status_rect.y = output_height - status_rect.h - 120;
                SDL_RenderCopy(&renderer, status_texture, ft_nullptr, &status_rect);
                SDL_DestroyTexture(status_texture);
            }
        }

        SDL_RenderPresent(&renderer);
    }

    bool process_adjustment(ft_ui_menu &menu, PlayerProfilePreferences &working_preferences, int direction)
    {
        const ft_menu_item *selected_item = menu.get_selected_item();
        if (selected_item == ft_nullptr)
            return false;

        bool adjusted = false;
        if (selected_item->identifier == "setting:ui_scale")
        {
            if (direction > 0)
                working_preferences.ui_scale_percent = increment_ui_scale(working_preferences.ui_scale_percent);
            else if (direction < 0)
                working_preferences.ui_scale_percent = decrement_ui_scale(working_preferences.ui_scale_percent);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:combat_speed")
        {
            if (direction > 0)
                working_preferences.combat_speed_percent = increment_combat_speed(working_preferences.combat_speed_percent);
            else if (direction < 0)
                working_preferences.combat_speed_percent = decrement_combat_speed(working_preferences.combat_speed_percent);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:music_volume")
        {
            if (direction > 0)
                working_preferences.music_volume_percent = increment_volume(working_preferences.music_volume_percent);
            else if (direction < 0)
                working_preferences.music_volume_percent = decrement_volume(working_preferences.music_volume_percent);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:effects_volume")
        {
            if (direction > 0)
                working_preferences.effects_volume_percent = increment_volume(working_preferences.effects_volume_percent);
            else if (direction < 0)
                working_preferences.effects_volume_percent = decrement_volume(working_preferences.effects_volume_percent);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:lore_anchor")
        {
            working_preferences.lore_panel_anchor = toggle_lore_anchor(working_preferences.lore_panel_anchor);
            adjusted = true;
        }

        return adjusted;
    }

    bool handle_activation(const ft_menu_item &item, PlayerProfilePreferences &working_preferences)
    {
        if (item.identifier == "setting:ui_scale")
        {
            working_preferences.ui_scale_percent = increment_ui_scale(working_preferences.ui_scale_percent);
            return true;
        }
        if (item.identifier == "setting:combat_speed")
        {
            working_preferences.combat_speed_percent = increment_combat_speed(working_preferences.combat_speed_percent);
            return true;
        }
        if (item.identifier == "setting:music_volume")
        {
            working_preferences.music_volume_percent = increment_volume(working_preferences.music_volume_percent);
            return true;
        }
        if (item.identifier == "setting:effects_volume")
        {
            working_preferences.effects_volume_percent = increment_volume(working_preferences.effects_volume_percent);
            return true;
        }
        if (item.identifier == "setting:lore_anchor")
        {
            working_preferences.lore_panel_anchor = toggle_lore_anchor(working_preferences.lore_panel_anchor);
            return true;
        }
        return false;
    }
#endif
}

bool run_settings_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    PlayerProfilePreferences &preferences, bool &out_quit_requested)
{
#if GALACTIC_HAVE_SDL2
    out_quit_requested = false;
    if (window == ft_nullptr || renderer == ft_nullptr)
        return false;

    PlayerProfilePreferences baseline = preferences;
    PlayerProfilePreferences working = preferences;

    ft_ui_menu menu;
    rebuild_settings_menu(menu, working, false, ft_nullptr);

    ft_string status_message;
    bool      status_is_error = false;
    bool      running = true;
    bool      saved_changes = false;

    while (running)
    {
        ft_mouse_state    mouse_state;
        ft_keyboard_state keyboard_state;
        bool              activate_requested = false;
        bool              save_requested = false;
        bool              values_changed = false;

        SDL_Event event;
        while (SDL_PollEvent(&event) == 1)
        {
            if (event.type == SDL_QUIT)
            {
                out_quit_requested = true;
                running = false;
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                mouse_state.moved = true;
                mouse_state.x = event.motion.x;
                mouse_state.y = event.motion.y;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse_state.left_pressed = true;
                    mouse_state.x = event.button.x;
                    mouse_state.y = event.button.y;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse_state.left_released = true;
                    mouse_state.x = event.button.x;
                    mouse_state.y = event.button.y;
                }
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_UP)
                    keyboard_state.pressed_up = true;
                else if (event.key.keysym.sym == SDLK_DOWN)
                    keyboard_state.pressed_down = true;
                else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE)
                    activate_requested = true;
                else if (event.key.keysym.sym == SDLK_LEFT)
                {
                    if (process_adjustment(menu, working, -1))
                    {
                        status_message.clear();
                        status_is_error = false;
                        values_changed = true;
                    }
                }
                else if (event.key.keysym.sym == SDLK_RIGHT)
                {
                    if (process_adjustment(menu, working, 1))
                    {
                        status_message.clear();
                        status_is_error = false;
                        values_changed = true;
                    }
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE)
                    running = false;
            }
        }

        if (!running)
            break;

        if (!mouse_state.moved)
        {
            int x = 0;
            int y = 0;
            SDL_GetMouseState(&x, &y);
            mouse_state.x = x;
            mouse_state.y = y;
        }

        menu.handle_mouse_input(mouse_state);
        menu.handle_keyboard_input(keyboard_state);

        ft_string previous_identifier;
        const ft_menu_item *selected_item = menu.get_selected_item();
        if (selected_item != ft_nullptr)
            previous_identifier = selected_item->identifier;

        bool allow_save_current = false;
        const ft_vector<ft_menu_item> &current_items = menu.get_items();
        for (size_t index = 0; index < current_items.size(); ++index)
        {
            if (current_items[index].identifier == "action:save")
            {
                allow_save_current = current_items[index].enabled;
                break;
            }
        }

        if (mouse_state.left_released)
        {
            const ft_menu_item *hovered_item = menu.get_hovered_item();
            if (hovered_item != ft_nullptr && hovered_item->enabled)
            {
                if (hovered_item->identifier == "action:save")
                    save_requested = true;
                else if (hovered_item->identifier == "action:cancel")
                {
                    running = false;
                }
                else if (handle_activation(*hovered_item, working))
                {
                    status_message.clear();
                    status_is_error = false;
                    values_changed = true;
                }
            }
        }

        if (activate_requested)
        {
            const ft_menu_item *active_item = menu.get_selected_item();
            if (active_item != ft_nullptr && active_item->enabled)
            {
                if (active_item->identifier == "action:save")
                    save_requested = true;
                else if (active_item->identifier == "action:cancel")
                    running = false;
                else if (handle_activation(*active_item, working))
                {
                    status_message.clear();
                    status_is_error = false;
                    values_changed = true;
                }
            }
        }

        if (save_requested)
        {
            if (preferences.commander_name.empty())
            {
                status_message = ft_string("No active commander profile to save.");
                status_is_error = true;
            }
            else
            {
                if (save_profile_preferences(window, working))
                {
                    preferences = working;
                    baseline = working;
                    status_message = ft_string("Settings saved.");
                    status_is_error = false;
                    saved_changes = true;
                }
                else
                {
                    status_message = ft_string("Failed to save settings.");
                    status_is_error = true;
                }
            }
        }

        bool allow_save_now = !preferences_equal(working, baseline);
        if (values_changed || save_requested || allow_save_now != allow_save_current)
            rebuild_settings_menu(menu, working, allow_save_now, &previous_identifier);

        bool has_unsaved_changes = !preferences_equal(working, baseline);
        render_settings_screen(*renderer, menu, title_font, menu_font, status_message, status_is_error, has_unsaved_changes);

        if (save_requested && !status_is_error)
        {
            running = false;
        }
    }

    return saved_changes;
#else
    (void)window;
    (void)renderer;
    (void)title_font;
    (void)menu_font;
    (void)preferences;
    out_quit_requested = false;
    return false;
#endif
}

namespace settings_flow_testing
{
    unsigned int clamp_ui_scale(unsigned int value) noexcept
    {
        return ::clamp_ui_scale(value);
    }

    unsigned int increment_ui_scale(unsigned int value) noexcept
    {
        return ::increment_ui_scale(value);
    }

    unsigned int decrement_ui_scale(unsigned int value) noexcept
    {
        return ::decrement_ui_scale(value);
    }

    unsigned int clamp_combat_speed(unsigned int value) noexcept
    {
        return ::clamp_combat_speed(value);
    }

    unsigned int increment_combat_speed(unsigned int value) noexcept
    {
        return ::increment_combat_speed(value);
    }

    unsigned int decrement_combat_speed(unsigned int value) noexcept
    {
        return ::decrement_combat_speed(value);
    }

    unsigned int clamp_music_volume(unsigned int value) noexcept
    {
        return ::clamp_volume(value);
    }

    unsigned int increment_music_volume(unsigned int value) noexcept
    {
        return ::increment_volume(value);
    }

    unsigned int decrement_music_volume(unsigned int value) noexcept
    {
        return ::decrement_volume(value);
    }

    unsigned int clamp_effects_volume(unsigned int value) noexcept
    {
        return ::clamp_volume(value);
    }

    unsigned int increment_effects_volume(unsigned int value) noexcept
    {
        return ::increment_volume(value);
    }

    unsigned int decrement_effects_volume(unsigned int value) noexcept
    {
        return ::decrement_volume(value);
    }

    unsigned int toggle_lore_anchor(unsigned int anchor) noexcept
    {
        return ::toggle_lore_anchor(anchor);
    }

    ft_string format_ui_scale_option(unsigned int value)
    {
        return ::format_ui_scale_label(value);
    }

    ft_string format_combat_speed_option(unsigned int value)
    {
        return ::format_combat_speed_label(value);
    }

    ft_string format_music_volume_option(unsigned int value)
    {
        return ::format_music_volume_label(value);
    }

    ft_string format_effects_volume_option(unsigned int value)
    {
        return ::format_effects_volume_label(value);
    }

    ft_string format_lore_anchor_option(unsigned int anchor)
    {
        return ::format_lore_anchor_label(anchor);
    }
}
