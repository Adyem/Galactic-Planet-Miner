    bool ensure_directory_exists(const ft_string &path) noexcept
    {
        if (path.empty())
        {
            ft_errno = FT_EINVAL;
            PROFILE_DEBUG_PRINT("[Profile] Cannot ensure directory for an empty path.\n");
            return false;
        }

        int exists_result = file_dir_exists(path.c_str());
        if (exists_result > 0)
            return true;
        if (exists_result < 0)
        {
            log_profile_errno("Checking directory", &path, "file_dir_exists");
            return false;
        }
        if (file_create_directory(path.c_str(), 0755) != 0)
        {
            log_profile_errno("Creating directory", &path, "mkdir");
            return false;
        }
        return true;
    }

    bool ensure_profile_save_directory_exists(const ft_string &commander_name) noexcept
    {
        if (!ensure_profile_directory_exists())
            return false;

        ft_string commander_directory = resolve_commander_directory(commander_name);
        if (commander_directory.empty())
        {
            ft_errno = FT_EINVAL;
            PROFILE_DEBUG_PRINT("[Profile] Could not resolve commander directory for \"%s\".\n", commander_name.c_str());
            return false;
        }
        if (!ensure_directory_exists(commander_directory))
            return false;

        ft_string save_directory = commander_directory;
        save_directory.append("/");
        save_directory.append(kProfileSaveDirectory);
        if (!ensure_directory_exists(save_directory))
            return false;
        return true;
    }

    bool remove_directory_recursive(const ft_string &path) noexcept
    {
        if (path.empty())
            return false;

        int exists_result = file_dir_exists(path.c_str());
        if (exists_result < 0)
        {
            log_profile_errno("Checking directory before removal", &path, "file_dir_exists");
            return false;
        }
        if (exists_result == 0)
            return true;

        file_dir *directory_stream = file_opendir(path.c_str());
        if (directory_stream == ft_nullptr)
        {
            log_profile_errno("Opening directory for recursive removal", &path, "file_opendir");
            return false;
        }

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

            ft_string child_path(path);
            child_path.append("/");
            child_path.append(entry->d_name);

            struct stat child_info;
            if (stat(child_path.c_str(), &child_info) == 0 && S_ISDIR(child_info.st_mode))
            {
                if (!remove_directory_recursive(child_path))
                {
                    file_closedir(directory_stream);
                    log_profile_errno("Removing child directory", &child_path);
                    return false;
                }
            }
            else
            {
                if (file_delete(child_path.c_str()) != 0)
                {
                    file_closedir(directory_stream);
                    log_profile_errno("Deleting file", &child_path, "file_delete");
                    return false;
                }
            }
        }

        file_closedir(directory_stream);
        if (ft_rmdir(path.c_str()) != 0)
        {
            log_profile_errno("Removing directory", &path, "rmdir");
            return false;
        }
        return true;
    }

    bool read_profile_name_from_file(const ft_string &path, ft_string &out_name) noexcept
    {
        out_name.clear();
        if (path.empty())
        {
            ft_errno = FT_EINVAL;
            PROFILE_DEBUG_PRINT("[Profile] Cannot read profile name from an empty path.\n");
            return false;
        }

        json_document document;
        if (document.read_from_file(path.c_str()) != 0)
        {
            log_profile_document_error("Reading profile file", document, path);
            return false;
        }

        json_group *group = document.find_group(kProfileGroupName);
        if (group == ft_nullptr)
        {
            PROFILE_DEBUG_PRINT("[Profile] Profile group missing in \"%s\".\n", path.c_str());
            return false;
        }

        json_item *name_item = document.find_item(group, "commander_name");
        if (name_item == ft_nullptr || name_item->value == ft_nullptr)
        {
            PROFILE_DEBUG_PRINT("[Profile] Commander name missing in \"%s\".\n", path.c_str());
            return false;
        }

        out_name = ft_string(name_item->value);
        return !out_name.empty();
    }

    bool add_string(json_document &document, json_group *group, const char *key, const ft_string &value) noexcept
    {
        if (group == ft_nullptr || key == ft_nullptr)
            return false;
        json_item *item = document.create_item(key, value.c_str());
        if (item == ft_nullptr)
            return false;
        document.add_item(group, item);
        return true;
    }

    bool add_int(json_document &document, json_group *group, const char *key, int value) noexcept
    {
        if (group == ft_nullptr || key == ft_nullptr)
            return false;
        json_item *item = document.create_item(key, value);
        if (item == ft_nullptr)
            return false;
        document.add_item(group, item);
        return true;
    }

    bool read_int(json_document &document, json_group *group, const char *key, unsigned int &out_value) noexcept
    {
        if (group == ft_nullptr || key == ft_nullptr)
            return false;
        json_item *item = document.find_item(group, key);
        if (item == ft_nullptr || item->value == ft_nullptr)
            return false;
        const int parsed = ft_atoi(item->value);
        if (parsed <= 0)
            return false;
        out_value = static_cast<unsigned int>(parsed);
        return true;
    }
}

ft_string player_profile_resolve_path(const ft_string &commander_name) noexcept
{
    ft_string sanitized = sanitize_commander_name(commander_name);
    if (sanitized.empty())
        return ft_string();

    ft_string path(kProfileDirectory);
    path.append("/");
    path.append(sanitized);
    path.append(kProfileExtension);
    return path;
}
ft_string player_profile_resolve_save_directory(const ft_string &commander_name) noexcept
{
    ft_string commander_directory = resolve_commander_directory(commander_name);
    if (commander_directory.empty())
        return ft_string();

    commander_directory.append("/");
    commander_directory.append(kProfileSaveDirectory);
    return commander_directory;
}

ft_string player_profile_resolve_tutorial_save_path(const ft_string &commander_name) noexcept
{
    ft_string commander_directory = resolve_commander_directory(commander_name);
    if (commander_directory.empty())
        return ft_string();

    commander_directory.append("/");
    commander_directory.append(kProfileTutorialDirectory);
    commander_directory.append("/");
    commander_directory.append(kProfileTutorialFilename);
    return commander_directory;
}
}
