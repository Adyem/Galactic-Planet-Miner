#include "main_menu_system.hpp"

#include "app_constants.hpp"
#include "player_profile.hpp"
#include "ui_input.hpp"

#include "libft/CPP_class/class_nullptr.hpp"

namespace profile_management_flow_detail
{
#if GALACTIC_HAVE_SDL2
    ft_string run_profile_management_flow_impl(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font,
        TTF_Font *menu_font, const ft_string &current_profile, bool &out_quit_requested);
#endif
}

ft_string run_profile_management_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    const ft_string &current_profile, bool &out_quit_requested)
{
#if GALACTIC_HAVE_SDL2
    return profile_management_flow_detail::run_profile_management_flow_impl(window, renderer, title_font, menu_font,
        current_profile, out_quit_requested);
#else
    (void)window;
    (void)renderer;
    (void)title_font;
    (void)menu_font;
    (void)current_profile;
    out_quit_requested = true;
    return ft_string();
#endif
}

#include "profile_management_flow_menu.cpp"
#include "profile_management_flow_loop.cpp"
