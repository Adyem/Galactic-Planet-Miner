#include "main_menu_system.hpp"

#include "app_constants.hpp"
#include "player_profile.hpp"

#include "libft/CPP_class/class_nullptr.hpp"

namespace profile_preferences_testing
{
    PlayerProfilePreferences build_window_preference_snapshot(
        const ft_string &profile_name, unsigned int window_width, unsigned int window_height) noexcept
    {
        PlayerProfilePreferences preferences;
        preferences.commander_name = profile_name;

        if (window_width > 0U)
            preferences.window_width = window_width;
        if (window_height > 0U)
            preferences.window_height = window_height;

        return preferences;
    }
}

#if GALACTIC_HAVE_SDL2

bool save_profile_preferences(SDL_Window *window, const ft_string &profile_name) noexcept
{
    if (window == ft_nullptr || profile_name.empty())
        return false;

    int window_width = static_cast<int>(app_constants::kWindowWidth);
    int window_height = static_cast<int>(app_constants::kWindowHeight);
    SDL_GetWindowSize(window, &window_width, &window_height);

    const unsigned int stored_width = window_width > 0 ? static_cast<unsigned int>(window_width) : 0U;
    const unsigned int stored_height = window_height > 0 ? static_cast<unsigned int>(window_height) : 0U;

    PlayerProfilePreferences preferences;
    if (!player_profile_load_or_create(preferences, profile_name))
        return false;

    if (stored_width > 0U)
        preferences.window_width = stored_width;
    if (stored_height > 0U)
        preferences.window_height = stored_height;

    return player_profile_save(preferences);
}

bool save_profile_preferences(SDL_Window *window, const PlayerProfilePreferences &preferences) noexcept
{
    if (preferences.commander_name.empty())
        return false;

    PlayerProfilePreferences merged = preferences;
    if (window != ft_nullptr)
    {
        int window_width = static_cast<int>(app_constants::kWindowWidth);
        int window_height = static_cast<int>(app_constants::kWindowHeight);
        SDL_GetWindowSize(window, &window_width, &window_height);
        if (window_width > 0)
            merged.window_width = static_cast<unsigned int>(window_width);
        if (window_height > 0)
            merged.window_height = static_cast<unsigned int>(window_height);
    }

    return player_profile_save(merged);
}

void apply_profile_preferences(SDL_Window *window, const ft_string &profile_name) noexcept
{
    if (window == ft_nullptr || profile_name.empty())
        return;

    PlayerProfilePreferences preferences;
    if (!player_profile_load_or_create(preferences, profile_name))
        return;

    int window_width = static_cast<int>(preferences.window_width);
    int window_height = static_cast<int>(preferences.window_height);
    if (window_width > 0 && window_height > 0)
        SDL_SetWindowSize(window, window_width, window_height);
}

#else

bool save_profile_preferences(SDL_Window *window, const ft_string &profile_name) noexcept
{
    (void)window;
    (void)profile_name;
    return false;
}

bool save_profile_preferences(SDL_Window *window, const PlayerProfilePreferences &preferences) noexcept
{
    (void)window;
    if (preferences.commander_name.empty())
        return false;
    return player_profile_save(preferences);
}

void apply_profile_preferences(SDL_Window *window, const ft_string &profile_name) noexcept
{
    (void)window;
    (void)profile_name;
}

#endif

