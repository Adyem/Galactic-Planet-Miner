#pragma once

#include "main_menu_system.hpp"

#include "app_constants.hpp"

#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/Libft/libft.hpp"

namespace profile_entry_flow
{
#if GALACTIC_HAVE_SDL2
    inline constexpr unsigned int kMaxProfileNameLength = 24U;

    enum e_profile_append_result
    {
        PROFILE_APPEND_REJECTED = 0,
        PROFILE_APPEND_ACCEPTED = 1,
        PROFILE_APPEND_MAX_LENGTH = 2
    };

    bool is_profile_character_allowed(char character) noexcept;
    e_profile_append_result append_profile_character(ft_string &profile_name, char character) noexcept;
    void remove_last_profile_character(ft_string &profile_name) noexcept;
    bool profile_name_is_valid(const ft_string &profile_name) noexcept;
    bool profile_name_exists(const ft_vector<ft_string> &profiles, const ft_string &candidate) noexcept;
    void render_profile_entry_screen(SDL_Renderer &renderer, TTF_Font *title_font, TTF_Font *menu_font,
        const ft_string &profile_name, const ft_string &status_message, bool status_is_error);
#endif
}

