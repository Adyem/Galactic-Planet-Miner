#ifndef PLAYER_PROFILE_HPP
#define PLAYER_PROFILE_HPP

#include "../libft/CPP_class/class_string_class.hpp"

struct PlayerProfilePreferences
{
    ft_string    commander_name;
    unsigned int window_width;
    unsigned int window_height;

    PlayerProfilePreferences() noexcept : commander_name(), window_width(1280U), window_height(720U) {}
};

bool player_profile_load_or_create(PlayerProfilePreferences &out_preferences, const ft_string &commander_name) noexcept;
bool player_profile_save(const PlayerProfilePreferences &preferences) noexcept;
ft_string player_profile_resolve_path(const ft_string &commander_name) noexcept;

#endif
