bool player_profile_delete(const ft_string &commander_name) noexcept
{
    if (commander_name.empty())
    {
        ft_errno = FT_EINVAL;
        PROFILE_DEBUG_PRINT("[Profile] Cannot delete profile without a commander name.\n");
        return false;
    }

    if (!ensure_profile_directory_exists())
    {
        PROFILE_DEBUG_PRINT("[Profile] Failed to prepare profile directories while deleting \"%s\".\n", commander_name.c_str());
        return false;
    }

    bool success = true;

    ft_string path = player_profile_resolve_path(commander_name);
    if (!path.empty())
    {
        if (file_delete(path.c_str()) != 0)
        {
            struct stat path_info;
            if (stat(path.c_str(), &path_info) == 0)
            {
                log_profile_errno("Deleting profile file", &path, "file_delete");
                success = false;
            }
        }
    }

    ft_string commander_directory = resolve_commander_directory(commander_name);
    if (!commander_directory.empty())
    {
        if (!remove_directory_recursive(commander_directory))
        {
            int exists_result = file_dir_exists(commander_directory.c_str());
            if (exists_result > 0)
            {
                log_profile_errno("Removing commander directory", &commander_directory);
                success = false;
            }
        }
    }

    return success;
}
