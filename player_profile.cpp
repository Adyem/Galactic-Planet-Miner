#include "player_profile.hpp"

#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/Errno/errno.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/JSon/document.hpp"
#include "libft/File/open_dir.hpp"
#include "libft/File/file_utils.hpp"
#include "libft/Printf/printf.hpp"
#include "libft/Template/algorithm.hpp"

#include <sys/stat.h>
#ifdef _WIN32
#    include <direct.h>
#    define ft_rmdir _rmdir
#else
#    include <unistd.h>
#    define ft_rmdir rmdir
#endif

#if DEBUG
#    define PROFILE_DEBUG_PRINT(...) pf_printf_fd(2, __VA_ARGS__)
#else
#    define PROFILE_DEBUG_PRINT(...) do { } while (0)
#endif

namespace
{
    const char *kProfileRootDirectory = "data";
    const char *kProfileDirectory = "data/profiles";

    bool ensure_directory_exists(const ft_string &path) noexcept;
    const char *kProfileGroupName = "profile";
    const char *kProfileExtension = ".prof";
    const char *kProfileSaveDirectory = "saves";

    unsigned int clamp_unsigned(unsigned int value, unsigned int min_value, unsigned int max_value) noexcept
    {
        if (value < min_value)
            return min_value;
        if (value > max_value)
            return max_value;
        return value;
    }

    unsigned int normalize_lore_panel_anchor(unsigned int anchor) noexcept
    {
        if (anchor == PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT)
            return PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT;
        return PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT;
    }

    int normalize_menu_input_device(unsigned int device) noexcept
    {
        if (device == static_cast<unsigned int>(PLAYER_PROFILE_INPUT_DEVICE_MOUSE))
            return PLAYER_PROFILE_INPUT_DEVICE_MOUSE;
        if (device == static_cast<unsigned int>(PLAYER_PROFILE_INPUT_DEVICE_GAMEPAD))
            return PLAYER_PROFILE_INPUT_DEVICE_GAMEPAD;
        if (device == static_cast<unsigned int>(PLAYER_PROFILE_INPUT_DEVICE_KEYBOARD))
            return PLAYER_PROFILE_INPUT_DEVICE_KEYBOARD;
        return PLAYER_PROFILE_INPUT_DEVICE_KEYBOARD;
    }

    void log_profile_errno(const char *stage, const ft_string *path = ft_nullptr, const char *detail = ft_nullptr) noexcept
    {
#if DEBUG
        int error_code = ft_errno;
        const char *error_message = ft_strerror(error_code);
        if (error_message == ft_nullptr)
            error_message = "Unknown error";
        if (path != ft_nullptr && !path->empty())
        {
            if (detail != ft_nullptr && detail[0] != '\0')
            {
                PROFILE_DEBUG_PRINT("[Profile] %s for \"%s\" (%s) failed: %s (%d)\n", stage, path->c_str(), detail,
                    error_message, error_code);
                return;
            }
            PROFILE_DEBUG_PRINT("[Profile] %s for \"%s\" failed: %s (%d)\n", stage, path->c_str(), error_message, error_code);
            return;
        }
        if (detail != ft_nullptr && detail[0] != '\0')
        {
            PROFILE_DEBUG_PRINT("[Profile] %s (%s) failed: %s (%d)\n", stage, detail, error_message, error_code);
            return;
        }
        PROFILE_DEBUG_PRINT("[Profile] %s failed: %s (%d)\n", stage, error_message, error_code);
#else
        (void)stage;
        (void)path;
        (void)detail;
#endif
    }

    void log_profile_document_error(const char *stage, const json_document &document, const ft_string &path) noexcept
    {
#if DEBUG
        int error_code = document.get_error();
        const char *error_message = document.get_error_str();
        if (error_message == ft_nullptr)
            error_message = "Unknown error";
        if (!path.empty())
        {
            PROFILE_DEBUG_PRINT("[Profile] %s for \"%s\" failed: %s (%d)\n", stage, path.c_str(), error_message, error_code);
            return;
        }
        PROFILE_DEBUG_PRINT("[Profile] %s failed: %s (%d)\n", stage, error_message, error_code);
#else
        (void)stage;
        (void)document;
        (void)path;
#endif
    }

    bool ensure_profile_directory_exists() noexcept
    {
        ft_string root_directory(kProfileRootDirectory);
        if (!ensure_directory_exists(root_directory))
            return false;

        int exists_result = file_dir_exists(kProfileDirectory);
        if (exists_result > 0)
            return true;
        if (exists_result < 0)
        {
            ft_string profile_directory(kProfileDirectory);
            log_profile_errno("Checking commander profile directory", &profile_directory, "file_dir_exists");
            return false;
        }
        if (file_create_directory(kProfileDirectory, 0755) != 0)
        {
            ft_string profile_directory(kProfileDirectory);
            log_profile_errno("Creating commander profile directory", &profile_directory, "mkdir");
            return false;
        }
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

bool player_profile_save(const PlayerProfilePreferences &preferences) noexcept
{
    if (preferences.commander_name.empty())
    {
        ft_errno = FT_EINVAL;
        PROFILE_DEBUG_PRINT("[Profile] Cannot save preferences without a commander name.\n");
        return false;
    }
    if (!ensure_profile_directory_exists())
    {
        PROFILE_DEBUG_PRINT("[Profile] Failed to prepare profile directories for \"%s\".\n", preferences.commander_name.c_str());
        return false;
    }
    if (!ensure_profile_save_directory_exists(preferences.commander_name))
    {
        PROFILE_DEBUG_PRINT("[Profile] Failed to prepare save directory for \"%s\".\n", preferences.commander_name.c_str());
        return false;
    }

    ft_string path = player_profile_resolve_path(preferences.commander_name);
    if (path.empty())
    {
        ft_errno = FT_EINVAL;
        PROFILE_DEBUG_PRINT("[Profile] Could not resolve profile path for \"%s\".\n", preferences.commander_name.c_str());
        return false;
    }

    json_document document;
    json_group *group = document.create_group(kProfileGroupName);
    if (group == ft_nullptr)
    {
        log_profile_document_error("Creating profile group", document, path);
        return false;
    }
    document.append_group(group);

    if (!add_string(document, group, "commander_name", preferences.commander_name))
    {
        log_profile_document_error("Adding commander name", document, path);
        return false;
    }
    const unsigned int stored_width = preferences.window_width == 0U ? 1280U : preferences.window_width;
    const unsigned int stored_height = preferences.window_height == 0U ? 720U : preferences.window_height;
    if (!add_int(document, group, "window_width", static_cast<int>(stored_width)))
    {
        log_profile_document_error("Adding window width", document, path);
        return false;
    }
    if (!add_int(document, group, "window_height", static_cast<int>(stored_height)))
    {
        log_profile_document_error("Adding window height", document, path);
        return false;
    }

    unsigned int stored_ui_scale = preferences.ui_scale_percent == 0U ? 100U : preferences.ui_scale_percent;
    stored_ui_scale = clamp_unsigned(stored_ui_scale, PLAYER_PROFILE_UI_SCALE_MIN_PERCENT, PLAYER_PROFILE_UI_SCALE_MAX_PERCENT);
    if (!add_int(document, group, "ui_scale_percent", static_cast<int>(stored_ui_scale)))
    {
        log_profile_document_error("Adding UI scale", document, path);
        return false;
    }

    unsigned int stored_combat_speed = preferences.combat_speed_percent == 0U ? 100U : preferences.combat_speed_percent;
    stored_combat_speed = clamp_unsigned(
        stored_combat_speed, PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT, PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT);
    if (!add_int(document, group, "combat_speed_percent", static_cast<int>(stored_combat_speed)))
    {
        log_profile_document_error("Adding combat speed", document, path);
        return false;
    }

    unsigned int stored_music_volume = clamp_unsigned(
        preferences.music_volume_percent, PLAYER_PROFILE_VOLUME_MIN_PERCENT, PLAYER_PROFILE_VOLUME_MAX_PERCENT);
    if (!add_int(document, group, "music_volume_percent", static_cast<int>(stored_music_volume)))
    {
        log_profile_document_error("Adding music volume", document, path);
        return false;
    }

    unsigned int stored_effects_volume = clamp_unsigned(
        preferences.effects_volume_percent, PLAYER_PROFILE_VOLUME_MIN_PERCENT, PLAYER_PROFILE_VOLUME_MAX_PERCENT);
    if (!add_int(document, group, "effects_volume_percent", static_cast<int>(stored_effects_volume)))
    {
        log_profile_document_error("Adding effects volume", document, path);
        return false;
    }

    unsigned int stored_anchor = normalize_lore_panel_anchor(preferences.lore_panel_anchor);
    if (!add_int(document, group, "lore_panel_anchor", static_cast<int>(stored_anchor)))
    {
        log_profile_document_error("Adding lore panel anchor", document, path);
        return false;
    }

    ft_string tutorial_value(preferences.menu_tutorial_seen ? "true" : "false");
    if (!add_string(document, group, "menu_tutorial_seen", tutorial_value))
    {
        log_profile_document_error("Adding menu tutorial flag", document, path);
        return false;
    }

    ft_string accessibility_value(preferences.accessibility_preset_enabled ? "true" : "false");
    if (!add_string(document, group, "accessibility_preset_enabled", accessibility_value))
    {
        log_profile_document_error("Adding accessibility preset flag", document, path);
        return false;
    }

    if (!add_int(document, group, "hotkey_menu_up", preferences.hotkey_menu_up))
    {
        log_profile_document_error("Adding menu hotkey (up)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_down", preferences.hotkey_menu_down))
    {
        log_profile_document_error("Adding menu hotkey (down)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_left", preferences.hotkey_menu_left))
    {
        log_profile_document_error("Adding menu hotkey (left)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_right", preferences.hotkey_menu_right))
    {
        log_profile_document_error("Adding menu hotkey (right)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_confirm", preferences.hotkey_menu_confirm))
    {
        log_profile_document_error("Adding menu hotkey (confirm)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_cancel", preferences.hotkey_menu_cancel))
    {
        log_profile_document_error("Adding menu hotkey (cancel)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_delete", preferences.hotkey_menu_delete))
    {
        log_profile_document_error("Adding menu hotkey (delete)", document, path);
        return false;
    }
    if (!add_int(document, group, "hotkey_menu_rename", preferences.hotkey_menu_rename))
    {
        log_profile_document_error("Adding menu hotkey (rename)", document, path);
        return false;
    }

    int stored_device
        = normalize_menu_input_device(static_cast<unsigned int>(preferences.last_menu_input_device));
    if (!add_int(document, group, "last_menu_input_device", stored_device))
    {
        log_profile_document_error("Adding last input device", document, path);
        return false;
    }

    if (document.write_to_file(path.c_str()) != 0)
    {
        log_profile_document_error("Writing profile", document, path);
        return false;
    }
    return true;
}

bool player_profile_load_or_create(PlayerProfilePreferences &out_preferences, const ft_string &commander_name) noexcept
{
    out_preferences = PlayerProfilePreferences();
    out_preferences.commander_name = commander_name;

    if (commander_name.empty())
    {
        ft_errno = FT_EINVAL;
        PROFILE_DEBUG_PRINT("[Profile] Cannot load preferences for an empty commander name.\n");
        return false;
    }
    if (!ensure_profile_directory_exists())
    {
        PROFILE_DEBUG_PRINT("[Profile] Failed to prepare base directory for \"%s\".\n", commander_name.c_str());
        return false;
    }

    ft_string path = player_profile_resolve_path(commander_name);
    if (path.empty())
    {
        ft_errno = FT_EINVAL;
        PROFILE_DEBUG_PRINT("[Profile] Could not resolve profile path for \"%s\".\n", commander_name.c_str());
        return false;
    }

    json_document document;
    if (document.read_from_file(path.c_str()) != 0)
    {
        log_profile_document_error("Reading profile", document, path);
        return player_profile_save(out_preferences);
    }

    json_group *group = document.find_group(kProfileGroupName);
    if (group == ft_nullptr)
    {
        PROFILE_DEBUG_PRINT("[Profile] Profile group missing in \"%s\". Recreating defaults.\n", path.c_str());
        return player_profile_save(out_preferences);
    }

    json_item *name_item = document.find_item(group, "commander_name");
    if (name_item != ft_nullptr && name_item->value != ft_nullptr)
        out_preferences.commander_name = ft_string(name_item->value);

    unsigned int parsed_width = out_preferences.window_width;
    unsigned int parsed_height = out_preferences.window_height;
    read_int(document, group, "window_width", parsed_width);
    read_int(document, group, "window_height", parsed_height);

    out_preferences.window_width = parsed_width;
    out_preferences.window_height = parsed_height;
    unsigned int parsed_ui_scale = out_preferences.ui_scale_percent;
    unsigned int parsed_combat_speed = out_preferences.combat_speed_percent;
    unsigned int parsed_music_volume = out_preferences.music_volume_percent;
    unsigned int parsed_effects_volume = out_preferences.effects_volume_percent;
    unsigned int parsed_anchor = out_preferences.lore_panel_anchor;
    bool         parsed_tutorial_seen = out_preferences.menu_tutorial_seen;
    bool         parsed_accessibility_enabled = out_preferences.accessibility_preset_enabled;
    unsigned int parsed_menu_up = static_cast<unsigned int>(out_preferences.hotkey_menu_up);
    unsigned int parsed_menu_down = static_cast<unsigned int>(out_preferences.hotkey_menu_down);
    unsigned int parsed_menu_left = static_cast<unsigned int>(out_preferences.hotkey_menu_left);
    unsigned int parsed_menu_right = static_cast<unsigned int>(out_preferences.hotkey_menu_right);
    unsigned int parsed_menu_confirm = static_cast<unsigned int>(out_preferences.hotkey_menu_confirm);
    unsigned int parsed_menu_cancel = static_cast<unsigned int>(out_preferences.hotkey_menu_cancel);
    unsigned int parsed_menu_delete = static_cast<unsigned int>(out_preferences.hotkey_menu_delete);
    unsigned int parsed_menu_rename = static_cast<unsigned int>(out_preferences.hotkey_menu_rename);
    unsigned int parsed_input_device = static_cast<unsigned int>(out_preferences.last_menu_input_device);

    read_int(document, group, "ui_scale_percent", parsed_ui_scale);
    read_int(document, group, "combat_speed_percent", parsed_combat_speed);
    read_int(document, group, "music_volume_percent", parsed_music_volume);
    read_int(document, group, "effects_volume_percent", parsed_effects_volume);
    read_int(document, group, "lore_panel_anchor", parsed_anchor);
    read_int(document, group, "hotkey_menu_up", parsed_menu_up);
    read_int(document, group, "hotkey_menu_down", parsed_menu_down);
    read_int(document, group, "hotkey_menu_left", parsed_menu_left);
    read_int(document, group, "hotkey_menu_right", parsed_menu_right);
    read_int(document, group, "hotkey_menu_confirm", parsed_menu_confirm);
    read_int(document, group, "hotkey_menu_cancel", parsed_menu_cancel);
    read_int(document, group, "hotkey_menu_delete", parsed_menu_delete);
    read_int(document, group, "hotkey_menu_rename", parsed_menu_rename);
    read_int(document, group, "last_menu_input_device", parsed_input_device);

    if (parsed_ui_scale == 0U)
        parsed_ui_scale = 100U;
    if (parsed_combat_speed == 0U)
        parsed_combat_speed = 100U;

    out_preferences.ui_scale_percent = clamp_unsigned(parsed_ui_scale, PLAYER_PROFILE_UI_SCALE_MIN_PERCENT, PLAYER_PROFILE_UI_SCALE_MAX_PERCENT);
    out_preferences.combat_speed_percent = clamp_unsigned(parsed_combat_speed, PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT, PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT);
    out_preferences.music_volume_percent = clamp_unsigned(
        parsed_music_volume, PLAYER_PROFILE_VOLUME_MIN_PERCENT, PLAYER_PROFILE_VOLUME_MAX_PERCENT);
    out_preferences.effects_volume_percent = clamp_unsigned(
        parsed_effects_volume, PLAYER_PROFILE_VOLUME_MIN_PERCENT, PLAYER_PROFILE_VOLUME_MAX_PERCENT);
    out_preferences.lore_panel_anchor = normalize_lore_panel_anchor(parsed_anchor);
    out_preferences.hotkey_menu_up
        = parsed_menu_up == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_UP : static_cast<int>(parsed_menu_up);
    out_preferences.hotkey_menu_down
        = parsed_menu_down == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DOWN : static_cast<int>(parsed_menu_down);
    out_preferences.hotkey_menu_left
        = parsed_menu_left == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_LEFT : static_cast<int>(parsed_menu_left);
    out_preferences.hotkey_menu_right
        = parsed_menu_right == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_RIGHT : static_cast<int>(parsed_menu_right);
    out_preferences.hotkey_menu_confirm
        = parsed_menu_confirm == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CONFIRM : static_cast<int>(parsed_menu_confirm);
    out_preferences.hotkey_menu_cancel
        = parsed_menu_cancel == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CANCEL : static_cast<int>(parsed_menu_cancel);
    out_preferences.hotkey_menu_delete
        = parsed_menu_delete == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DELETE : static_cast<int>(parsed_menu_delete);
    out_preferences.hotkey_menu_rename
        = parsed_menu_rename == 0U ? PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_RENAME : static_cast<int>(parsed_menu_rename);
    out_preferences.last_menu_input_device = normalize_menu_input_device(parsed_input_device);

    json_item *tutorial_item = document.find_item(group, "menu_tutorial_seen");
    if (tutorial_item != ft_nullptr && tutorial_item->value != ft_nullptr)
    {
        ft_string normalized(tutorial_item->value);
        ft_to_lower(normalized.print());
        const char *normalized_value = normalized.c_str();
        if (ft_strcmp(normalized_value, "true") == 0 || ft_strcmp(normalized_value, "1") == 0
            || ft_strcmp(normalized_value, "yes") == 0)
            parsed_tutorial_seen = true;
        else if (ft_strcmp(normalized_value, "false") == 0 || ft_strcmp(normalized_value, "0") == 0
            || ft_strcmp(normalized_value, "no") == 0)
            parsed_tutorial_seen = false;
    }
    out_preferences.menu_tutorial_seen = parsed_tutorial_seen;

    json_item *accessibility_item = document.find_item(group, "accessibility_preset_enabled");
    if (accessibility_item != ft_nullptr && accessibility_item->value != ft_nullptr)
    {
        ft_string normalized(accessibility_item->value);
        ft_to_lower(normalized.print());
        const char *normalized_value = normalized.c_str();
        if (ft_strcmp(normalized_value, "true") == 0 || ft_strcmp(normalized_value, "1") == 0
            || ft_strcmp(normalized_value, "yes") == 0 || ft_strcmp(normalized_value, "on") == 0)
            parsed_accessibility_enabled = true;
        else if (ft_strcmp(normalized_value, "false") == 0 || ft_strcmp(normalized_value, "0") == 0
            || ft_strcmp(normalized_value, "no") == 0 || ft_strcmp(normalized_value, "off") == 0)
            parsed_accessibility_enabled = false;
    }
    out_preferences.accessibility_preset_enabled = parsed_accessibility_enabled;
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
