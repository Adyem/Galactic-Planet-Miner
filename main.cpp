#include "app_constants.hpp"
#include "main_menu_system.hpp"
#include "player_profile.hpp"
#include "ui_input.hpp"
#include "ui_menu.hpp"

#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/Libft/libft.hpp"

#if GALACTIC_HAVE_SDL2
#    include <SDL2/SDL.h>
#    include <SDL2/SDL_ttf.h>
#endif

#if GALACTIC_HAVE_SDL2

namespace
{
    const ft_menu_item *menu_item_from_index(const ft_ui_menu &menu, int index)
    {
        if (index < 0)
            return ft_nullptr;

        const ft_vector<ft_menu_item> &items = menu.get_items();
        const size_t                  size = items.size();
        const size_t                  converted_index = static_cast<size_t>(index);

        if (converted_index >= size)
            return ft_nullptr;

        return &items[converted_index];
    }

    void destroy_renderer(SDL_Renderer *renderer)
    {
        if (renderer != ft_nullptr)
            SDL_DestroyRenderer(renderer);
    }

    void destroy_window(SDL_Window *window)
    {
        if (window != ft_nullptr)
            SDL_DestroyWindow(window);
    }
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
        return 1;

    if (TTF_Init() != 0)
    {
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Galactic Planet Miner",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        static_cast<int>(app_constants::kWindowWidth),
        static_cast<int>(app_constants::kWindowHeight),
        SDL_WINDOW_SHOWN);

    if (window == ft_nullptr)
    {
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == ft_nullptr)
    {
        destroy_window(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    TTF_Font *title_font = resolve_font(48);
    TTF_Font *menu_font = resolve_font(28);

    bool quit_requested = false;
    ft_vector<ft_string> available_profiles;
    player_profile_list(available_profiles);

    ft_string active_profile_name;
    if (available_profiles.empty())
    {
        ft_string created_profile = run_profile_entry_flow(window, renderer, title_font, menu_font, &available_profiles,
            quit_requested);
        if (quit_requested || created_profile.empty())
        {
            destroy_renderer(renderer);
            destroy_window(window);
            TTF_Quit();
            SDL_Quit();
            return 0;
        }

        active_profile_name = created_profile;
        player_profile_list(available_profiles);
    }
    else
    {
        active_profile_name = available_profiles[0];
    }

    if (active_profile_name.empty())
    {
        destroy_renderer(renderer);
        destroy_window(window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    apply_profile_preferences(window, active_profile_name);

    ft_ui_menu menu;
    menu.set_items(build_main_menu_items());
    menu.set_viewport_bounds(build_main_menu_viewport());

    bool running = true;

    while (running)
    {
        ft_mouse_state    mouse_state;
        ft_keyboard_state keyboard_state;
        bool              activate_requested = false;

        SDL_Event event;
        while (SDL_PollEvent(&event) == 1)
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                mouse_state.moved = true;
                mouse_state.x = event.motion.x;
                mouse_state.y = event.motion.y;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse_state.left_pressed = true;
                    mouse_state.x = event.button.x;
                    mouse_state.y = event.button.y;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse_state.left_released = true;
                    mouse_state.x = event.button.x;
                    mouse_state.y = event.button.y;
                }
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_UP)
                    keyboard_state.pressed_up = true;
                else if (event.key.keysym.sym == SDLK_DOWN)
                    keyboard_state.pressed_down = true;
                else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE)
                    activate_requested = true;
                else if (event.key.keysym.sym == SDLK_ESCAPE)
                    running = false;
            }
        }

        if (!mouse_state.moved)
        {
            int x = 0;
            int y = 0;
            SDL_GetMouseState(&x, &y);
            mouse_state.x = x;
            mouse_state.y = y;
        }

        menu.handle_mouse_input(mouse_state);
        menu.handle_keyboard_input(keyboard_state);

        auto process_menu_activation = [&](const ft_menu_item &item) {
            if (item.identifier == "exit")
            {
                running = false;
                return;
            }

            if (item.identifier == "swap_profile")
            {
                bool management_quit = false;
                ft_string selected_profile = run_profile_management_flow(window, renderer, title_font, menu_font, active_profile_name,
                    management_quit);
                if (management_quit)
                {
                    running = false;
                    return;
                }

                if (!selected_profile.empty() && selected_profile != active_profile_name)
                {
                    active_profile_name = selected_profile;
                    apply_profile_preferences(window, active_profile_name);
                }

                player_profile_list(available_profiles);
            }
        };

        if (mouse_state.left_released)
        {
            const int index = menu.get_hovered_index();
            const ft_menu_item *hovered_item = menu_item_from_index(menu, index);
            if (hovered_item != ft_nullptr)
            {
                process_menu_activation(*hovered_item);
                if (!running)
                    break;
            }
        }

        if (activate_requested)
        {
            const ft_menu_item *selected_item = menu.get_selected_item();
            if (selected_item != ft_nullptr)
            {
                process_menu_activation(*selected_item);
                if (!running)
                    break;
            }
        }

        int window_width = 0;
        int window_height = 0;
        SDL_GetWindowSize(window, &window_width, &window_height);
        render_main_menu(*renderer, menu, title_font, menu_font, window_width, window_height, active_profile_name);
    }

    destroy_renderer(renderer);
    destroy_window(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}

#else

int main()
{
    return 0;
}

#endif

