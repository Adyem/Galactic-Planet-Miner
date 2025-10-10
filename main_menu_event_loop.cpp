
        SDL_Event event;
        while (SDL_PollEvent(&event) == 1)
        {
            if (event.type == SDL_QUIT)
            {
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
                    mouse_state.x = event.button.x;
                    mouse_state.y = event.button.y;
                    if (tutorial_context.visible)
                        tutorial_click_in_progress = true;
                    else if (clear_cloud_context.visible)
                        clear_cloud_click_in_progress = true;
                    else if (manual_context.visible)
                        manual_click_in_progress = true;
                    else if (changelog_context.visible)
                        changelog_click_in_progress = true;
                    else if (crash_prompt_context.visible)
                        crash_click_in_progress = true;
                    else
                        mouse_state.left_pressed = true;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse_state.x = event.button.x;
                    mouse_state.y = event.button.y;
                    if (tutorial_click_in_progress || tutorial_context.visible)
                    {
                        dismiss_tutorial_overlay();
                        tutorial_click_in_progress = false;
                    }
                    else if (manual_click_in_progress || manual_context.visible)
                    {
                        dismiss_manual_overlay();
                        manual_click_in_progress = false;
                    }
                    else if (changelog_click_in_progress || changelog_context.visible)
                    {
                        dismiss_changelog_overlay();
                        changelog_click_in_progress = false;
                    }
                    else if (clear_cloud_click_in_progress || clear_cloud_context.visible)
                    {
                        close_clear_cloud_prompt();
                        clear_cloud_click_in_progress = false;
                    }
                    else if (crash_click_in_progress || crash_prompt_context.visible)
                    {
                        submit_crash_report();
                        crash_click_in_progress = false;
                        handle_crash_prompt_focus();
                    }
                    else
                        mouse_state.left_released = true;
                }
            }
            else if (event.type == SDL_KEYDOWN)
            {
                auto matches_key = [&](SDL_Keycode key, int mapping) {
                    if (mapping == 0)
                        return false;
                    return key == static_cast<SDL_Keycode>(mapping);
                };

                SDL_Keycode key = event.key.keysym.sym;
                if (matches_key(key, active_preferences.hotkey_menu_up))
                    keyboard_state.pressed_up = true;
                else if (matches_key(key, active_preferences.hotkey_menu_down))
                    keyboard_state.pressed_down = true;
                else if (matches_key(key, active_preferences.hotkey_menu_confirm))
                {
                    if (clear_cloud_context.visible)
                    {
                        confirm_clear_cloud_prompt();
                        clear_cloud_click_in_progress = false;
                    }
                    else if (crash_prompt_context.visible)
                    {
                        submit_crash_report();
                        crash_click_in_progress = false;
                        handle_crash_prompt_focus();
                    }
                    else if (manual_context.visible)
                    {
                        dismiss_manual_overlay();
                        manual_click_in_progress = false;
                    }
                    else if (changelog_context.visible)
                    {
                        dismiss_changelog_overlay();
                        changelog_click_in_progress = false;
                    }
                    else if (!dismiss_tutorial_overlay())
                        activate_requested = true;
                    tutorial_click_in_progress = false;
                }
                else if (matches_key(key, active_preferences.hotkey_menu_cancel))
                {
                    if (clear_cloud_context.visible)
                        close_clear_cloud_prompt();
                    else if (crash_prompt_context.visible)
                        dismiss_crash_prompt();
                    else if (!dismiss_manual_overlay() && !dismiss_changelog_overlay())
                        running = false;
                }
            }
            else if (event.type == SDL_CONTROLLERBUTTONDOWN)
            {
                auto matches_button = [&](int button, int mapping) {
                    if (mapping < PLAYER_PROFILE_CONTROLLER_BUTTON_A
                        || mapping > PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_RIGHT)
                        return false;
                    return button == mapping;
                };

                const int button = static_cast<int>(event.cbutton.button);
                if (matches_button(button, active_preferences.controller_menu_up))
                    gamepad_state.pressed_up = true;
                else if (matches_button(button, active_preferences.controller_menu_down))
                    gamepad_state.pressed_down = true;
                else if (matches_button(button, active_preferences.controller_menu_confirm))
                {
                    gamepad_state.pressed_confirm = true;
                    if (clear_cloud_context.visible)
                    {
                        confirm_clear_cloud_prompt();
                        clear_cloud_click_in_progress = false;
                    }
                    else if (crash_prompt_context.visible)
                    {
                        submit_crash_report();
                        crash_click_in_progress = false;
                        handle_crash_prompt_focus();
                    }
                    else if (manual_context.visible)
                    {
                        dismiss_manual_overlay();
                        manual_click_in_progress = false;
                    }
                    else if (changelog_context.visible)
                    {
                        dismiss_changelog_overlay();
                        changelog_click_in_progress = false;
                    }
                    else if (!dismiss_tutorial_overlay())
                        activate_requested = true;
                    tutorial_click_in_progress = false;
                }
                else if (matches_button(button, active_preferences.controller_menu_cancel))
                {
                    gamepad_state.pressed_cancel = true;
                    if (clear_cloud_context.visible)
                        close_clear_cloud_prompt();
                    else if (crash_prompt_context.visible)
                        dismiss_crash_prompt();
                    else if (!dismiss_manual_overlay() && !dismiss_changelog_overlay())
                        running = false;
                }
            }
        }

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
        menu.handle_gamepad_input(gamepad_state);

        e_ft_input_device active_device = menu.get_active_device();
        if (active_device != FT_INPUT_DEVICE_NONE
            && active_device != static_cast<e_ft_input_device>(active_preferences.last_menu_input_device))
        {
            active_preferences.last_menu_input_device = static_cast<int>(active_device);
        }

        auto process_menu_activation = [&](const ft_menu_item &item) {
            if (clear_cloud_context.visible)
                return;

            if (item.identifier == "new_game")
            {
                bool      creation_quit = false;
                ft_string created_save_path;
                bool      created_save = run_new_game_creation_flow(window, renderer, title_font, menu_font,
                    active_profile_name, created_save_path, creation_quit);
                if (creation_quit)
                {
                    running = false;
                    return;
                }

                if (created_save)
                {
                    player_profile_list(available_profiles);
                    refresh_save_alert();
                    if (!created_save_path.empty() && attempt_campaign_launch(created_save_path))
                    {
                        running = false;
                        return;
                    }
                }
                return;
            }

            if (item.identifier == "resume")
            {
                if (!resume_available || resume_save_path.empty())
                    return;

                if (attempt_campaign_launch(resume_save_path))
                {
                    running = false;
                    return;
                }

                refresh_save_alert();
                return;
            }

            if (item.identifier == "load")
            {
                bool      load_quit = false;
                ft_string selected_save_path;
                bool      loaded = run_load_game_flow(window, renderer, title_font, menu_font, active_profile_name,
                    selected_save_path, load_quit);
                if (load_quit)
                {
                    running = false;
                    return;
                }
