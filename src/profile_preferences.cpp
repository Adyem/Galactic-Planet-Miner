#include "main_menu_system.hpp"

#include "app_constants.hpp"
#include "player_profile.hpp"

#include "../libft/CPP_class/class_nullptr.hpp"

bool save_profile_preferences(SDL_Window *window, const ft_string &profile_name) noexcept
{
    if (window == ft_nullptr)
        return false;

    PlayerProfilePreferences preferences;
    preferences.commander_name = profile_name;

    int window_width = static_cast<int>(app_constants::kWindowWidth);
    int window_height = static_cast<int>(app_constants::kWindowHeight);
    SDL_GetWindowSize(window, &window_width, &window_height);

    if (window_width > 0)
        preferences.window_width = static_cast<unsigned int>(window_width);
    if (window_height > 0)
        preferences.window_height = static_cast<unsigned int>(window_height);

    return player_profile_save(preferences);
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

