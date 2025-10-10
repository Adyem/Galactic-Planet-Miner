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
                if (rename_active)
                {
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        rename_cancel_requested = true;
                    else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
                        rename_confirm_now = true;
                    else if (event.key.keysym.sym == SDLK_BACKSPACE || event.key.keysym.sym == SDLK_DELETE)
                    {
                        remove_last_save_character(rename_input);
                        update_rename_status();
                    }
                    continue;
                }

                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    if (delete_confirmation_active)
                    {
                        clear_delete_confirmation();
                        status_message
                            = menu_localize("load_menu.status.delete_canceled", "Deletion canceled.");
                        status_is_error = false;
                    }
                    else
                    {
                        running = false;
                        break;
                    }
                }
                else if (event.key.keysym.sym == SDLK_UP)
                    keyboard_state.pressed_up = true;
                else if (event.key.keysym.sym == SDLK_DOWN)
                    keyboard_state.pressed_down = true;
                else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE)
                    keyboard_state.pressed_confirm = true;
                else if (event.key.keysym.sym == SDLK_DELETE || event.key.keysym.sym == SDLK_BACKSPACE)
                    keyboard_state.pressed_delete = true;
                else if (event.key.keysym.sym == SDLK_r)
                    rename_requested = true;
            }
            else if (event.type == SDL_TEXTINPUT)
            {
                if (rename_active)
                {
                    const char *text = event.text.text;
                    if (text != ft_nullptr)
                    {
                        bool appended_character = false;
                        bool rejected_for_length = false;
                        bool rejected_for_invalid = false;

                        for (int index = 0; text[index] != '\0'; ++index)
                        {
                            const char character = text[index];
                            if (append_save_character(rename_input, character))
                                appended_character = true;
                            else if (!is_save_character_allowed(character))
                                rejected_for_invalid = true;
                            else
                                rejected_for_length = true;
                        }

                        if (appended_character)
                        {
                            update_rename_status();
                        }
                        else if (rejected_for_length)
                        {
                            ft_vector<StringTableReplacement> replacements;
                            replacements.reserve(1U);
                            StringTableReplacement replacement;
                            replacement.key = ft_string("name");
                            replacement.value = rename_input;
                            replacements.push_back(replacement);
                            status_message = menu_localize(
                                "load_menu.error.name_length", "Save names are limited to 24 characters.");
                            status_message.append(menu_localize_format(
                                "load_menu.status.current_name", " Current name: \"{{name}}\".", replacements));
                            status_is_error = true;
                        }
                        else if (rejected_for_invalid)
                        {
                            ft_vector<StringTableReplacement> replacements;
                            replacements.reserve(1U);
                            StringTableReplacement replacement;
                            replacement.key = ft_string("name");
                            replacement.value = rename_input;
                            replacements.push_back(replacement);
                            status_message = menu_localize(
                                "load_menu.error.name_charset", "Use letters and numbers only.");
                            status_message.append(menu_localize_format(
                                "load_menu.status.current_name", " Current name: \"{{name}}\".", replacements));
                            status_is_error = true;
                        }
                    }
                }
            }
        }

        if (!running)
            break;

        if (rename_cancel_requested)
            cancel_rename(menu_localize("load_menu.status.rename_canceled", "Rename canceled."), false);

        if (!mouse_state.moved)
        {
            int x = 0;
            int y = 0;
            SDL_GetMouseState(&x, &y);
            mouse_state.x = x;
            mouse_state.y = y;
        }

        const int previous_selected_index = menu.get_selected_index();
        menu.handle_mouse_input(mouse_state);
        menu.handle_keyboard_input(keyboard_state);
        const int current_selected_index = menu.get_selected_index();
        if (delete_confirmation_active && current_selected_index != previous_selected_index)
        {
            clear_delete_confirmation();
            status_message = menu_localize("load_menu.status.delete_canceled", "Deletion canceled.");
            status_is_error = false;
        }
        if (delete_confirmation_active && (current_selected_index < 0
                                              || current_selected_index >= static_cast<int>(slots.size())))
        {
            clear_delete_confirmation();
            status_message = menu_localize("load_menu.status.delete_canceled", "Deletion canceled.");
            status_is_error = false;
        }

        ft_string selected_identifier;
        const ft_menu_item *current_selected_item = menu.get_selected_item();
        if (current_selected_item != ft_nullptr)
            selected_identifier = current_selected_item->identifier;

        const save_slot_entry *selected_slot = ft_nullptr;
        if (!selected_identifier.empty() && selected_identifier.compare(0, 5, "save:") == 0)
        {
            for (size_t index = 0; index < slots.size(); ++index)
            {
                if (slots[index].identifier == selected_identifier)
                {
                    selected_slot = &slots[index];
                    break;
                }
            }
        }

        if (rename_active)
        {
            if (selected_slot == ft_nullptr || selected_identifier != rename_identifier)
                cancel_rename(menu_localize("load_menu.status.rename_canceled", "Rename canceled."), false);
        }

        if (rename_requested)
        {
            if (selected_slot != ft_nullptr)
                begin_rename(*selected_slot);
            else if (!slots.empty())
            {
                status_message = menu_localize(
                    "load_menu.status.rename_select", "Select a save before pressing R to rename.");
                status_is_error = true;
            }
        }

        bool skip_menu_activation = false;
        bool confirm_delete_now = false;

        if (rename_active)
            skip_menu_activation = true;

        if (rename_confirm_now && rename_active)
        {
            skip_menu_activation = true;
            if (rename_input == rename_original_label)
            {
                cancel_rename(menu_localize("load_menu.status.rename_unchanged", "Save name unchanged."), false);
            }
            else
            {
                const ft_string previous_label = rename_original_label;
                const ft_string new_label = rename_input;
                ft_string rename_error;
                bool renamed = ::rename_save_slot(commander_name, previous_label, new_label, rename_error);
                if (renamed)
                {
                    ft_string new_identifier("save:");
                    new_identifier.append(new_label);
                    clear_rename_state();

                    bool refreshed = collect_save_slots(commander_name, slots);
                    if (!refreshed)
                    {
                        status_message = menu_localize(
                            "load_menu.status.refresh_failed_rename",
                            "Unable to refresh save directory after rename.");
                        status_is_error = true;
                        slots.clear();
                        rebuild_load_menu(menu, slots, false);
                    }
                    else
                    {
                        rebuild_load_menu(menu, slots, !slots.empty());
                        if (slots.empty())
                        {
                            status_message
                                = menu_localize("load_menu.status.no_saves",
                                    "No campaign saves found for this commander.");
                            status_is_error = false;
                        }
                        else
                        {
                            ft_vector<StringTableReplacement> replacements;
                            replacements.reserve(2U);
                            StringTableReplacement from_replacement;
                            from_replacement.key = ft_string("from");
                            from_replacement.value = previous_label;
                            replacements.push_back(from_replacement);
                            StringTableReplacement to_replacement;
                            to_replacement.key = ft_string("to");
                            to_replacement.value = new_label;
                            replacements.push_back(to_replacement);
