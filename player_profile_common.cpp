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
    const char *kProfileTutorialDirectory = "tutorial";
    const char *kProfileTutorialFilename = "replay.json";

    bool        ensure_directory_exists(const ft_string &path) noexcept;
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

    int normalize_controller_button(unsigned int button, int fallback) noexcept
    {
        const unsigned int min_button = static_cast<unsigned int>(PLAYER_PROFILE_CONTROLLER_BUTTON_A);
        const unsigned int max_button = static_cast<unsigned int>(PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_RIGHT);
        if (button < min_button || button > max_button)
            return fallback;
        return static_cast<int>(button);
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

