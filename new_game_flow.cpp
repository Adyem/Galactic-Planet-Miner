#include "main_menu_system.hpp"

#include "menu_localization.hpp"
#include "ui_input.hpp"

namespace new_game_flow
{
    bool append_save_character(ft_string &save_name, char character) noexcept;
    void remove_last_save_character(ft_string &save_name) noexcept;
    bool save_name_is_valid(const ft_string &save_name) noexcept;
    bool create_new_game_save(
        const ft_string &commander_name,
        const ft_string &save_name,
        ft_string &out_error,
        ft_string &out_save_path) noexcept;

#if GALACTIC_HAVE_SDL2
    void render_new_game_screen(SDL_Renderer &renderer, TTF_Font *title_font, TTF_Font *menu_font,
        const ft_string &save_name, const ft_string &status_message, bool status_is_error);
#endif
}

bool run_new_game_creation_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    const ft_string &commander_name, ft_string &out_created_save_path, bool &out_quit_requested)
{
#if GALACTIC_HAVE_SDL2
    out_quit_requested = false;
    out_created_save_path.clear();
    if (window == ft_nullptr || renderer == ft_nullptr)
        return false;

    ft_string save_name;
    ft_string status_message;
    bool status_is_error = false;
    bool running = true;
    bool created = false;
    ft_string created_save_path;

    SDL_StartTextInput();

    while (running && !created)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event) == 1)
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
                out_quit_requested = true;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    running = false;
                }
                else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
                {
                    if (!new_game_flow::save_name_is_valid(save_name))
                    {
                        status_message = menu_localize(
                            "new_game.status.empty", "Please enter a save name.");
                        status_is_error = true;
                    }
                    else
                    {
                        ft_string error_message;
                        if (new_game_flow::create_new_game_save(
                                commander_name, save_name, error_message, created_save_path))
                        {
                            created = true;
                        }
                        else
                        {
                            status_message = error_message;
                            status_is_error = true;
                        }
                    }
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE)
                {
                    new_game_flow::remove_last_save_character(save_name);
                    status_message.clear();
                    status_is_error = false;
                }
            }
            else if (event.type == SDL_TEXTINPUT)
            {
                const char *text = event.text.text;
                if (text != ft_nullptr)
                {
                    bool appended_character = false;
                    bool rejected_for_length = false;
                    bool rejected_for_invalid = false;

                    for (int index = 0; text[index] != '\0'; ++index)
                    {
                        const char character = text[index];
                        if (new_game_flow::append_save_character(save_name, character))
                        {
                            appended_character = true;
                        }
                        else if (!new_game_flow::is_save_character_allowed(character))
                        {
                            rejected_for_invalid = true;
                        }
                        else
                        {
                            rejected_for_length = true;
                        }
                    }

                    if (appended_character)
                    {
                        status_message.clear();
                        status_is_error = false;
                    }
                    else if (rejected_for_length)
                    {
                        status_message = menu_localize(
                            "new_game.status.length", "Save names are limited to 24 characters.");
                        status_is_error = true;
                    }
                    else if (rejected_for_invalid)
                    {
                        status_message = menu_localize(
                            "new_game.status.invalid", "Use letters and numbers only.");
                        status_is_error = true;
                    }
                }
            }
        }

        new_game_flow::render_new_game_screen(
            *renderer, title_font, menu_font, save_name, status_message, status_is_error);
        SDL_Delay(16);
    }

    SDL_StopTextInput();

    if (!created)
        return false;

    out_created_save_path = created_save_path;
    return true;
#else
    (void)window;
    (void)renderer;
    (void)title_font;
    (void)menu_font;
    (void)commander_name;
    (void)out_created_save_path;
    out_quit_requested = true;
    return false;
#endif
}
