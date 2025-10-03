#pragma once

#include "libft/Libft/libft.hpp"
#include "ui_menu.hpp"

#if defined(__has_include)
#  if __has_include(<SDL2/SDL.h>) && __has_include(<SDL2/SDL_ttf.h>)
#    define GALACTIC_HAVE_SDL2 1
#  else
#    define GALACTIC_HAVE_SDL2 0
#  endif
#else
#  define GALACTIC_HAVE_SDL2 0
#endif

#if GALACTIC_HAVE_SDL2
#    include <SDL2/SDL.h>
#    include <SDL2/SDL_ttf.h>
#else
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;

struct SDL_Color
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct SDL_Rect
{
    int x;
    int y;
    int w;
    int h;
};

struct _TTF_Font;
typedef _TTF_Font TTF_Font;
#endif

// Font utilities
TTF_Font *resolve_font(int point_size);
SDL_Texture *create_text_texture(SDL_Renderer &renderer, TTF_Font &font, const ft_string &text,
    const SDL_Color &color, SDL_Rect &out_rect);

// Main menu rendering
ft_vector<ft_menu_item> build_main_menu_items();
ft_rect                 build_main_menu_viewport();
void render_main_menu(SDL_Renderer &renderer, const ft_ui_menu &menu, TTF_Font *title_font, TTF_Font *menu_font,
    int window_width, int window_height, const ft_string &active_profile_name);

// Profile creation and selection flows
ft_string run_profile_entry_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    const ft_vector<ft_string> *existing_profiles, bool &out_quit_requested);
ft_string run_profile_management_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    const ft_string &current_profile, bool &out_quit_requested);
bool run_new_game_creation_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    const ft_string &commander_name, bool &out_quit_requested);

// Profile window preference helpers
bool save_profile_preferences(SDL_Window *window, const ft_string &profile_name) noexcept;
void apply_profile_preferences(SDL_Window *window, const ft_string &profile_name) noexcept;
