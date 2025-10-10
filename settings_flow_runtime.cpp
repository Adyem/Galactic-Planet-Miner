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
                status_message = menu_localize(
                    "settings.menu.status.no_profile", "No active commander profile to save.");
                status_is_error = true;
            }
            else
            {
                if (save_profile_preferences(window, working))
                {
                    preferences = working;
                    baseline = working;
                    status_message = menu_localize("settings.menu.status.saved", "Settings saved.");
                    status_is_error = false;
                    saved_changes = true;
                }
                else
                {
                    status_message
                        = menu_localize("settings.menu.status.save_failed", "Failed to save settings.");
                    status_is_error = true;
                }
            }
        }

        bool allow_save_now = !preferences_equal(working, baseline);
        if (values_changed || save_requested || allow_save_now != allow_save_current)
            rebuild_settings_menu(menu, working, allow_save_now, &previous_identifier);

        bool has_unsaved_changes = !preferences_equal(working, baseline);
        render_settings_screen(*renderer, menu, title_font, menu_font, status_message, status_is_error, has_unsaved_changes,
            working.colorblind_palette_enabled, working.brightness_percent, working.contrast_percent);

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
