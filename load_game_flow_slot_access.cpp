                            status_message = menu_localize_format(
                                "load_menu.status.rename_success",
                                "Renamed save \"{{from}}\" to \"{{to}}\".",
                                replacements);
                            status_is_error = false;

                            for (size_t index = 0; index < slots.size(); ++index)
                            {
                                if (slots[index].identifier == new_identifier)
                                {
                                    menu.set_selected_index(static_cast<int>(index));
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (rename_error.empty())
                        status_message
                            = menu_localize("load_menu.error.rename_failed", "Failed to rename save file.");
                    else
                        status_message = rename_error;
                    ft_vector<StringTableReplacement> replacements;
                    replacements.reserve(1U);
                    StringTableReplacement replacement;
                    replacement.key = ft_string("name");
                    replacement.value = rename_input;
                    replacements.push_back(replacement);
                    status_message.append(menu_localize_format(
                        "load_menu.status.current_name", " Current name: \"{{name}}\".", replacements));
                    status_is_error = true;
                }
            }

            keyboard_state.pressed_confirm = false;
            rename_confirm_now = false;
        }

        if (keyboard_state.pressed_delete)
        {
            if (selected_slot != ft_nullptr)
            {
                if (delete_confirmation_active && selected_slot->identifier == delete_confirmation_identifier)
                    confirm_delete_now = true;
                else
                    begin_delete_confirmation(*selected_slot);
            }
            else
            {
                if (!slots.empty())
                {
                    status_message = menu_localize(
                        "load_menu.status.delete_select", "Select a save before pressing Delete.");
                    status_is_error = true;
                }
                clear_delete_confirmation();
            }
            keyboard_state.pressed_delete = false;
        }

        if (delete_confirmation_active && keyboard_state.pressed_confirm)
        {
            if (selected_slot != ft_nullptr && selected_slot->identifier == delete_confirmation_identifier)
                confirm_delete_now = true;
            keyboard_state.pressed_confirm = false;
        }

        if (confirm_delete_now)
        {
            ft_string delete_error;
            bool deleted = delete_save_slot(commander_name, delete_confirmation_label, delete_error);
            if (deleted)
            {
                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(1U);
                StringTableReplacement replacement;
                replacement.key = ft_string("slot");
                replacement.value = delete_confirmation_label;
                replacements.push_back(replacement);
                status_message = menu_localize_format(
                    "load_menu.status.delete_success", "Deleted save \"{{slot}}\".", replacements);
                status_is_error = false;
            }
            else
            {
                if (delete_error.empty())
                    status_message
                        = menu_localize("load_menu.error.delete_failed", "Failed to delete save file.");
                else
                    status_message = delete_error;
                status_is_error = true;
            }

            clear_delete_confirmation();

            bool refreshed = collect_save_slots(commander_name, slots);
            if (!refreshed)
            {
                status_message = menu_localize(
                    "load_menu.status.refresh_failed_delete",
                    "Unable to refresh save directory after deletion.");
                status_is_error = true;
                slots.clear();
            }

            rebuild_load_menu(menu, slots, !slots.empty());
            if (slots.empty() && refreshed)
            {
                status_message = menu_localize(
                    "load_menu.status.no_saves", "No campaign saves found for this commander.");
                status_is_error = false;
            }

            skip_menu_activation = true;
        }

        auto process_selection = [&](const ft_menu_item &item) {
            if (item.identifier == "action:cancel")
            {
                running = false;
                return;
            }
            if (item.identifier.compare(0, 5, "save:") == 0)
            {
                for (size_t index = 0; index < slots.size(); ++index)
                {
                    if (slots[index].identifier == item.identifier)
                    {
                        out_selected_save = slots[index].file_path;
                        loaded = true;
                        running = false;
                        return;
                    }
                }
            }
        };

        if (!skip_menu_activation)
        {
            if (mouse_state.left_released)
            {
                const int hovered_index = menu.get_hovered_index();
                const ft_menu_item *hovered_item = menu_item_from_index(menu, hovered_index);
                if (hovered_item != ft_nullptr)
                    process_selection(*hovered_item);
            }

            const ft_menu_item *selected_item = menu.get_selected_item();
            if (keyboard_state.pressed_confirm && selected_item != ft_nullptr)
                process_selection(*selected_item);
        }
        else
            keyboard_state.pressed_confirm = false;

        int window_width = 0;
        int window_height = 0;
        SDL_GetWindowSize(window, &window_width, &window_height);
        (void)window_width;
        (void)window_height;

        render_load_screen(*renderer, menu, title_font, menu_font, commander_name, status_message, status_is_error);
        SDL_RenderPresent(renderer);
    }

    if (rename_active)
        SDL_StopTextInput();

    return loaded;
#endif
}

namespace load_game_flow_testing
{
    ft_vector<ft_string> collect_save_slot_names(const ft_string &commander_name)
    {
        ft_vector<save_slot_entry> slots;
        ft_vector<ft_string>        names;
        if (!collect_save_slots(commander_name, slots))
            return names;
        names.reserve(slots.size());
        for (size_t index = 0; index < slots.size(); ++index)
            names.push_back(slots[index].label);
        return names;
    }

    ft_vector<ft_string> collect_save_slot_metadata_labels(const ft_string &commander_name)
    {
        ft_vector<save_slot_entry> slots;
        ft_vector<ft_string>        labels;
        if (!collect_save_slots(commander_name, slots))
            return labels;
        labels.reserve(slots.size());
        for (size_t index = 0; index < slots.size(); ++index)
            labels.push_back(slots[index].metadata_label);
        return labels;
    }

    ft_string resolve_save_file_path(const ft_string &commander_name, const ft_string &slot_name)
    {
        if (commander_name.empty() || slot_name.empty())
            return ft_string();
        return build_save_file_path(commander_name, slot_name);
    }

    ft_vector<ft_string> collect_save_slot_errors(const ft_string &commander_name)
    {
        ft_vector<save_slot_entry> slots;
        ft_vector<ft_string>        errors;
        if (!collect_save_slots(commander_name, slots, &errors))
            errors.clear();
        return errors;
    }

    bool delete_save_slot(const ft_string &commander_name, const ft_string &slot_name, ft_string &out_error)
    {
        return ::delete_save_slot(commander_name, slot_name, out_error);
    }

    bool rename_save_slot(const ft_string &commander_name, const ft_string &old_name, const ft_string &new_name,
        ft_string &out_error)
    {
        return ::rename_save_slot(commander_name, old_name, new_name, out_error);
    }
}
