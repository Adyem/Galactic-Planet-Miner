namespace profile_management_flow_detail
{
#if GALACTIC_HAVE_SDL2
    ft_string run_profile_management_flow_impl(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font,
        TTF_Font *menu_font, const ft_string &current_profile, bool &out_quit_requested)
    {
        out_quit_requested = false;
        if (window == ft_nullptr || renderer == ft_nullptr)
            return ft_string();

        ft_vector<ft_string> profiles;
        if (!player_profile_list(profiles))
            return ft_string();

        if (profiles.empty())
        {
            bool creation_quit = false;
            ft_string created_profile = run_profile_entry_flow(window, renderer, title_font, menu_font, &profiles, creation_quit);
            if (creation_quit)
            {
                out_quit_requested = true;
                return ft_string();
            }
            if (created_profile.empty())
                return ft_string();

            player_profile_list(profiles);
        }

        size_t preferred_index = find_profile_index(profiles, current_profile);
        ft_ui_menu menu;
        rebuild_profile_menu(menu, profiles, current_profile, preferred_index, false);
        int last_selected_index = menu.get_selected_index();

        ft_string status_message;
        bool      status_is_error = false;
        bool      running = true;
        int       pending_delete_index = -1;
        ft_string pending_delete_name;

        while (running)
        {
            ft_mouse_state    mouse_state;
            ft_keyboard_state keyboard_state;
            bool              activate_requested = false;
            bool              delete_requested = false;

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
                    else if (event.key.keysym.sym == SDLK_DELETE || event.key.keysym.sym == SDLK_BACKSPACE)
                        delete_requested = true;
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

            int current_selected_index = menu.get_selected_index();
            if (current_selected_index != last_selected_index)
            {
                pending_delete_index = -1;
                pending_delete_name.clear();
                status_message.clear();
                status_is_error = false;
                last_selected_index = current_selected_index;
            }

            size_t profile_count = profiles.size();
            int    activation_index = -1;
            if (mouse_state.left_released)
                activation_index = menu.get_hovered_index();
            if (activation_index < 0 && activate_requested)
                activation_index = menu.get_selected_index();

            if (activation_index >= 0)
            {
                if (static_cast<size_t>(activation_index) < profile_count)
                    return profiles[static_cast<size_t>(activation_index)];

                if (static_cast<size_t>(activation_index) == profile_count)
                {
                    bool creation_quit = false;
                    ft_string new_profile = run_profile_entry_flow(window, renderer, title_font, menu_font, &profiles,
                        creation_quit);
                    if (creation_quit)
                    {
                        out_quit_requested = true;
                        return ft_string();
                    }
                    if (!new_profile.empty())
                    {
                        player_profile_list(profiles);
                        profile_count = profiles.size();
                        preferred_index = find_profile_index(profiles, new_profile);
                        rebuild_profile_menu(menu, profiles, current_profile, preferred_index, false);
                        last_selected_index = menu.get_selected_index();
                        status_message = ft_string("Created profile: ");
                        status_message.append(new_profile);
                        status_is_error = false;
                        pending_delete_index = -1;
                        pending_delete_name.clear();
                    }
                }
            }

            if (delete_requested)
            {
                int selected_index = menu.get_selected_index();
                if (selected_index >= 0 && static_cast<size_t>(selected_index) < profile_count)
                {
                    if (profile_count <= 1U)
                    {
                        status_message = ft_string("At least one profile must remain.");
                        status_is_error = true;
                        pending_delete_index = -1;
                        pending_delete_name.clear();
                    }
                    else if (pending_delete_index == selected_index)
                    {
                        ft_string name_to_delete = profiles[static_cast<size_t>(selected_index)];
                        if (player_profile_delete(name_to_delete))
                        {
                            player_profile_list(profiles);
                            profile_count = profiles.size();
                            if (profile_count == 0U)
                            {
                                status_message = ft_string("No profiles available.");
                                status_is_error = true;
                                return ft_string();
                            }

                            size_t new_preferred_index = static_cast<size_t>(selected_index);
                            if (new_preferred_index >= profile_count)
                                new_preferred_index = profile_count - 1U;

                            rebuild_profile_menu(menu, profiles, current_profile, new_preferred_index, false);
                            last_selected_index = menu.get_selected_index();
                            status_message = ft_string("Deleted profile: ");
                            status_message.append(name_to_delete);
                            status_is_error = false;
                        }
                        else
                        {
                            status_message = ft_string("Unable to delete profile.");
                            status_is_error = true;
                        }

                        pending_delete_index = -1;
                        pending_delete_name.clear();
                    }
                    else
                    {
                        pending_delete_index = selected_index;
                        pending_delete_name = profiles[static_cast<size_t>(selected_index)];
                        status_message = ft_string("Press Delete again to remove: ");
                        status_message.append(pending_delete_name);
                        status_is_error = true;
                    }
                }
            }

            render_profile_management_screen(*renderer, menu, title_font, menu_font, status_message, status_is_error);
            SDL_Delay(16);
        }

        return ft_string();
    }
#endif
}
