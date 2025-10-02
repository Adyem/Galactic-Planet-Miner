#include "player_profile.hpp"

#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/JSon/document.hpp"
#include "libft/File/open_dir.hpp"
#include "libft/File/file_utils.hpp"
#include "libft/Template/algorithm.hpp"

#include <sys/stat.h>
#ifdef _WIN32
#    include <direct.h>
#    define ft_rmdir _rmdir
#else
#    include <unistd.h>
#    define ft_rmdir rmdir
#endif

namespace
{
    const char *kProfileDirectory = "profiles";
    const char *kProfileGroupName = "profile";
    const char *kProfileExtension = ".prof";
    const char *kProfileSaveDirectory = "saves";

    bool ensure_profile_directory_exists() noexcept
    {
        int exists_result = file_dir_exists(kProfileDirectory);
        if (exists_result == 0)
            return true;
        if (exists_result < 0)
            return false;
        if (file_create_directory(kProfileDirectory, 0755) != 0)
            return false;
        return true;
    }

    ft_string sanitize_commander_name(const ft_string &commander_name) noexcept
    {
        const char *raw = commander_name.c_str();
        if (raw == ft_nullptr)
            return ft_string("Commander");

        ft_string sanitized;
        for (size_t index = 0; raw[index] != '\0'; ++index)
        {
            const char character = raw[index];
            const bool is_digit = (character >= '0' && character <= '9');
            const bool is_upper = (character >= 'A' && character <= 'Z');
            const bool is_lower = (character >= 'a' && character <= 'z');
            const bool is_allowed_symbol = (character == '-' || character == '_' || character == ' ');
            const bool is_forbidden = (character == '/' || character == '\\' || character == ':' || character == '*' ||
                                       character == '?' || character == '"' || character == '<' || character == '>' ||
                                       character == '|');

            if ((is_digit || is_upper || is_lower || is_allowed_symbol) && !is_forbidden)
                sanitized.append(character);
            else
                sanitized.append('_');
        }

        if (sanitized.empty())
            sanitized = ft_string("Commander");
        return sanitized;
    }

    ft_string resolve_commander_directory(const ft_string &commander_name) noexcept
    {
        ft_string sanitized = sanitize_commander_name(commander_name);
        if (sanitized.empty())
            return ft_string();

        ft_string path(kProfileDirectory);
        path.append("/");
        path.append(sanitized);
        return path;
    }

    bool ensure_directory_exists(const ft_string &path) noexcept
    {
        if (path.empty())
            return false;

        int exists_result = file_dir_exists(path.c_str());
        if (exists_result == 0)
            return true;
        if (exists_result < 0)
            return false;
        if (file_create_directory(path.c_str(), 0755) != 0)
            return false;
        return true;
    }

    bool ensure_profile_save_directory_exists(const ft_string &commander_name) noexcept
    {
        if (!ensure_profile_directory_exists())
            return false;

        ft_string commander_directory = resolve_commander_directory(commander_name);
        if (commander_directory.empty())
            return false;
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
            return false;
        if (exists_result == 0)
            return true;

        file_dir *directory_stream = file_opendir(path.c_str());
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

            ft_string child_path(path);
            child_path.append("/");
            child_path.append(entry->d_name);

            struct stat child_info;
            if (stat(child_path.c_str(), &child_info) == 0 && S_ISDIR(child_info.st_mode))
            {
                if (!remove_directory_recursive(child_path))
                {
                    file_closedir(directory_stream);
                    return false;
                }
            }
            else
            {
                if (file_delete(child_path.c_str()) != 0)
                {
                    file_closedir(directory_stream);
                    return false;
                }
            }
        }

        file_closedir(directory_stream);
        if (ft_rmdir(path.c_str()) != 0)
            return false;
        return true;
    }

    bool read_profile_name_from_file(const ft_string &path, ft_string &out_name) noexcept
    {
        out_name.clear();
        if (path.empty())
            return false;

        json_document document;
        if (document.read_from_file(path.c_str()) != 0)
            return false;

        json_group *group = document.find_group(kProfileGroupName);
        if (group == ft_nullptr)
            return false;

        json_item *name_item = document.find_item(group, "commander_name");
        if (name_item == ft_nullptr || name_item->value == ft_nullptr)
            return false;

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

bool player_profile_save(const PlayerProfilePreferences &preferences) noexcept
{
    if (preferences.commander_name.empty())
        return false;
    if (!ensure_profile_directory_exists())
        return false;
    if (!ensure_profile_save_directory_exists(preferences.commander_name))
        return false;

    ft_string path = player_profile_resolve_path(preferences.commander_name);
    if (path.empty())
        return false;

    json_document document;
    json_group *group = document.create_group(kProfileGroupName);
    if (group == ft_nullptr)
        return false;
    document.append_group(group);

    if (!add_string(document, group, "commander_name", preferences.commander_name))
        return false;
    const unsigned int stored_width = preferences.window_width == 0U ? 1280U : preferences.window_width;
    const unsigned int stored_height = preferences.window_height == 0U ? 720U : preferences.window_height;
    if (!add_int(document, group, "window_width", static_cast<int>(stored_width)))
        return false;
    if (!add_int(document, group, "window_height", static_cast<int>(stored_height)))
        return false;

    if (document.write_to_file(path.c_str()) != 0)
        return false;
    return true;
}

bool player_profile_load_or_create(PlayerProfilePreferences &out_preferences, const ft_string &commander_name) noexcept
{
    out_preferences = PlayerProfilePreferences();
    out_preferences.commander_name = commander_name;

    if (commander_name.empty())
        return false;
    if (!ensure_profile_directory_exists())
        return false;

    ft_string path = player_profile_resolve_path(commander_name);
    if (path.empty())
        return false;

    json_document document;
    if (document.read_from_file(path.c_str()) != 0)
        return player_profile_save(out_preferences);

    json_group *group = document.find_group(kProfileGroupName);
    if (group == ft_nullptr)
        return player_profile_save(out_preferences);

    json_item *name_item = document.find_item(group, "commander_name");
    if (name_item != ft_nullptr && name_item->value != ft_nullptr)
        out_preferences.commander_name = ft_string(name_item->value);

    unsigned int parsed_width = out_preferences.window_width;
    unsigned int parsed_height = out_preferences.window_height;
    read_int(document, group, "window_width", parsed_width);
    read_int(document, group, "window_height", parsed_height);

    out_preferences.window_width = parsed_width;
    out_preferences.window_height = parsed_height;
    return true;
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

bool player_profile_list(ft_vector<ft_string> &out_profiles) noexcept
{
    out_profiles.clear();
    if (!ensure_profile_directory_exists())
        return false;

    file_dir *directory_stream = file_opendir(kProfileDirectory);
    if (directory_stream == ft_nullptr)
        return false;

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
            continue;

        out_profiles.push_back(commander_name);
    }

    file_closedir(directory_stream);
    if (out_profiles.size() > 1U)
        ft_sort(out_profiles.begin(), out_profiles.end());
    return true;
}

bool player_profile_delete(const ft_string &commander_name) noexcept
{
    if (commander_name.empty())
        return false;

    if (!ensure_profile_directory_exists())
        return false;

    bool success = true;

    ft_string path = player_profile_resolve_path(commander_name);
    if (!path.empty())
    {
        if (file_delete(path.c_str()) != 0)
        {
            struct stat path_info;
            if (stat(path.c_str(), &path_info) == 0)
                success = false;
        }
    }

    ft_string commander_directory = resolve_commander_directory(commander_name);
    if (!commander_directory.empty())
    {
        if (!remove_directory_recursive(commander_directory))
        {
            int exists_result = file_dir_exists(commander_directory.c_str());
            if (exists_result > 0)
                success = false;
        }
    }

    return success;
}
