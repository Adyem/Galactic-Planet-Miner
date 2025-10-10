bool player_profile_list(ft_vector<ft_string> &out_profiles) noexcept
{
    out_profiles.clear();
    if (!ensure_profile_directory_exists())
    {
        PROFILE_DEBUG_PRINT("[Profile] Unable to prepare profile directory for listing.\n");
        return false;
    }

    file_dir *directory_stream = file_opendir(kProfileDirectory);
    if (directory_stream == ft_nullptr)
    {
        ft_string profile_directory(kProfileDirectory);
        log_profile_errno("Opening profile directory", &profile_directory, "file_opendir");
        return false;
    }

    const size_t extension_length = ft_strlen(kProfileExtension);
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

        const char *name = entry->d_name;
        size_t name_length = ft_strlen(name);
        if (name_length <= extension_length)
            continue;
        bool matches_extension = true;
        for (size_t index = 0; index < extension_length; ++index)
        {
            char expected = kProfileExtension[index];
            char actual = name[name_length - extension_length + index];
            if (expected != actual)
            {
                matches_extension = false;
                break;
            }
        }
        if (!matches_extension)
            continue;

        ft_string profile_path(kProfileDirectory);
        profile_path.append("/");
        profile_path.append(name);

        ft_string commander_name;
        if (!read_profile_name_from_file(profile_path, commander_name))
        {
            PROFILE_DEBUG_PRINT("[Profile] Skipping unreadable profile file \"%s\".\n", profile_path.c_str());
            continue;
        }

        out_profiles.push_back(commander_name);
    }

    file_closedir(directory_stream);
    if (out_profiles.size() > 1U)
        ft_sort(out_profiles.begin(), out_profiles.end());
    return true;
}
