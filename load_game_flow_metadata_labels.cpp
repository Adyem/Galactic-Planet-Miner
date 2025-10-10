
        ft_string level_label;
        if (level_valid)
        {
            ft_vector<StringTableReplacement> replacements;
            replacements.reserve(1U);
            StringTableReplacement replacement;
            replacement.key = ft_string("value");
            replacement.value = ft_to_string(level);
            replacements.push_back(replacement);
            level_label = menu_localize_format("load_menu.metadata.level_known", "Level {{value}}", replacements);
        }
        else
            level_label = menu_localize("load_menu.metadata.level_unknown", "Level ?");

        ft_string difficulty_label = format_difficulty_component(difficulty_valid, difficulty);

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

        StringTableReplacement difficulty_replacement;
        difficulty_replacement.key = ft_string("difficulty");
        difficulty_replacement.value = difficulty_label;
        combined_replacements.push_back(difficulty_replacement);

        return menu_localize_format(
            "load_menu.metadata.combined", "{{day}} • {{level}} • {{difficulty}}", combined_replacements);
    }

    ft_string build_save_file_path(const ft_string &commander_name, const ft_string &slot_name)
    {
        if (commander_name.empty() || slot_name.empty())
            return ft_string();

        ft_string save_directory = player_profile_resolve_save_directory(commander_name);
        if (save_directory.empty())
            return ft_string();

        ft_string file_name(slot_name);
        file_name.append(".json");
        return file_path_join(save_directory.c_str(), file_name.c_str());
    }

    bool rename_save_slot(
        const ft_string &commander_name, const ft_string &old_name, const ft_string &new_name, ft_string &out_error) noexcept
    {
        out_error.clear();

        if (commander_name.empty())
        {
            out_error = menu_localize("load_menu.error.no_commander", "Commander profile missing.");
            return false;
        }

        if (old_name.empty())
        {
            out_error = menu_localize("load_menu.error.no_original_name", "Original save name missing.");
            return false;
        }

        if (!save_name_is_valid(new_name))
        {
            out_error = menu_localize("load_menu.error.name_required", "Please enter a new save name.");
            return false;
        }

        if (new_name.size() > static_cast<size_t>(kMaxSaveNameLength))
        {
            out_error = menu_localize(
                "load_menu.error.name_length", "Save names are limited to 24 characters.");
            return false;
        }

        for (size_t index = 0; index < new_name.size(); ++index)
        {
            if (!is_save_character_allowed(new_name[index]))
            {
                out_error = menu_localize("load_menu.error.name_charset", "Use letters and numbers only.");
                return false;
            }
        }

        ft_string source_path = build_save_file_path(commander_name, old_name);
        if (source_path.empty())
        {
            out_error
                = menu_localize("load_menu.error.resolve_existing", "Unable to resolve existing save location.");
            return false;
        }

        int source_exists = file_exists(source_path.c_str());
        if (source_exists <= 0)
        {
            if (source_exists < 0)
                out_error
                    = menu_localize("load_menu.error.verify_existing", "Unable to verify existing save file.");
            else
                out_error = menu_localize("load_menu.error.missing_original", "Original save file not found.");
            return false;
        }

        ft_string destination_path = build_save_file_path(commander_name, new_name);
        if (destination_path.empty())
        {
            out_error
                = menu_localize("load_menu.error.resolve_new", "Unable to resolve new save location.");
            return false;
        }

        int destination_exists = file_exists(destination_path.c_str());
        if (destination_exists < 0)
        {
            out_error
                = menu_localize("load_menu.error.check_conflict", "Unable to check for conflicting saves.");
            return false;
        }
        if (destination_exists > 0)
        {
            out_error = menu_localize("load_menu.error.conflict", "A save with that name already exists.");
            return false;
        }

        if (file_move(source_path.c_str(), destination_path.c_str()) != 0)
        {
            out_error = menu_localize("load_menu.error.rename_failed", "Failed to rename save file.");
            return false;
        }

        return true;
    }

    bool delete_save_slot(const ft_string &commander_name, const ft_string &slot_name, ft_string &out_error) noexcept
    {
        out_error.clear();

        ft_string file_path = build_save_file_path(commander_name, slot_name);
        if (file_path.empty())
        {
            out_error = menu_localize("load_menu.error.resolve_path", "Unable to resolve save location.");
            return false;
        }

        int exists_result = file_exists(file_path.c_str());
        if (exists_result < 0)
        {
            out_error = menu_localize("load_menu.error.verify_path", "Unable to verify save file.");
            return false;
        }
        if (exists_result == 0)
        {
            out_error = menu_localize("load_menu.error.not_found", "Save file not found.");
            return false;
        }

        if (file_delete(file_path.c_str()) != 0)
        {
            out_error = menu_localize("load_menu.error.delete_failed", "Failed to delete save file.");
            return false;
        }

        return true;
    }

    bool collect_save_slots(const ft_string &commander_name, ft_vector<save_slot_entry> &out_slots,
        ft_vector<ft_string> *out_errors = ft_nullptr)
    {
        out_slots.clear();
        if (commander_name.empty())
            return false;

        if (out_errors != ft_nullptr)
            out_errors->clear();

        ft_string save_directory = player_profile_resolve_save_directory(commander_name);
        if (save_directory.empty())
            return false;

        int exists_result = file_dir_exists(save_directory.c_str());
        if (exists_result < 0)
            return false;
        if (exists_result == 0)
            return true;

        file_dir *directory_stream = file_opendir(save_directory.c_str());
        if (directory_stream == ft_nullptr)
            return false;

        file_dirent *entry = ft_nullptr;
        while ((entry = file_readdir(directory_stream)) != ft_nullptr)
        {
            if (entry->d_name[0] == '\0')
                continue;
            if (entry->d_name[0] == '.')
            {
                if (entry->d_name[1] == '\0')
                    continue;
                if (entry->d_name[1] == '.' && entry->d_name[2] == '\0')
                    continue;
            }
            if (!has_save_extension(entry->d_name))
                continue;

            ft_string label;
            strip_save_extension(entry->d_name, label);
            if (label.empty())
                continue;

            save_slot_entry slot;
            slot.identifier = ft_string("save:");
            slot.identifier.append(label);
            slot.label = label;
            slot.file_path = file_path_join(save_directory.c_str(), entry->d_name);
            slot.difficulty_known = false;
            slot.difficulty_value = 0;
            bool metadata_day_valid = false;
            bool metadata_level_valid = false;
            bool metadata_difficulty_valid = false;
            int  metadata_day = 0;
            int  metadata_level = 0;
            int  metadata_difficulty = 0;
            bool metadata_loaded = read_save_metadata(slot.file_path, metadata_day_valid, metadata_day, metadata_level_valid,
                metadata_level, metadata_difficulty_valid, metadata_difficulty);
            if (metadata_loaded)
            {
                slot.difficulty_known = metadata_difficulty_valid;
                slot.difficulty_value = metadata_difficulty;
                if (metadata_day_valid || metadata_level_valid || metadata_difficulty_valid)
                {
                    slot.metadata_label = format_save_metadata_label(
                        metadata_day_valid, metadata_day, metadata_level_valid, metadata_level, metadata_difficulty_valid,
                        metadata_difficulty);
                    slot.metadata_available = true;
                }
                else
                {
                    slot.metadata_label
                        = menu_localize("load_menu.metadata.unavailable", "Metadata unavailable");
                    slot.metadata_available = false;
                    slot.difficulty_known = false;
                    slot.difficulty_value = 0;
