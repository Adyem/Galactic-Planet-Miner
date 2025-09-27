#include "player_profile.hpp"

#include "../libft/CPP_class/class_nullptr.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/JSon/document.hpp"
#include "../libft/File/open_dir.hpp"

#include <sys/stat.h>

namespace
{
    const char *kProfileDirectory = "profiles";
    const char *kProfileGroupName = "profile";
    const char *kProfileExtension = ".prof";

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
