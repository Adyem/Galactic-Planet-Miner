#include "main_menu_system.hpp"

namespace profile_entry_flow
{
#if GALACTIC_HAVE_SDL2
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
    bool save_profile_preferences(SDL_Window *window, const ft_string &profile_name) noexcept;
    void render_profile_entry_screen(SDL_Renderer &renderer, TTF_Font *title_font, TTF_Font *menu_font,
        const ft_string &profile_name, const ft_string &status_message, bool status_is_error);
#endif
}

ft_string run_profile_entry_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    const ft_vector<ft_string> *existing_profiles, bool &out_quit_requested)
{
#if GALACTIC_HAVE_SDL2
    using namespace profile_entry_flow;
    if (window == ft_nullptr || renderer == ft_nullptr)
        return ft_string();

    ft_string profile_name;
    ft_string status_message;
    bool status_is_error = false;
    bool running = true;
    bool accepted = false;
    out_quit_requested = false;

    SDL_StartTextInput();

    while (running && !accepted)
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
                    if (!profile_name_is_valid(profile_name))
                    {
                        status_message = ft_string("Please enter a profile name.");
                        status_is_error = true;
                    }
                    else if (existing_profiles != ft_nullptr && profile_name_exists(*existing_profiles, profile_name))
                    {
                        status_message = ft_string("A profile with that name already exists.");
                        status_is_error = true;
                    }
                    else if (save_profile_preferences(window, profile_name))
                    {
                        accepted = true;
                    }
                    else
                    {
                        status_message = ft_string("Unable to save profile. Check write permissions.");
                        status_is_error = true;
                    }
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE)
                {
                    remove_last_profile_character(profile_name);
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
                        const e_profile_append_result result = append_profile_character(profile_name, character);
                        if (result == PROFILE_APPEND_ACCEPTED)
                        {
                            appended_character = true;
                        }
                        else if (result == PROFILE_APPEND_MAX_LENGTH)
                        {
                            rejected_for_length = true;
                        }
                        else
                        {
                            rejected_for_invalid = true;
                        }
                    }

                    if (appended_character)
                    {
                        status_message.clear();
                        status_is_error = false;
                    }
                    else if (rejected_for_length)
                    {
                        status_message = ft_string("Profile names are limited to 24 characters.");
                        status_is_error = true;
                    }
                    else if (rejected_for_invalid)
                    {
                        status_message = ft_string("Use letters and numbers only.");
                        status_is_error = true;
                    }
                }
            }
        }

        render_profile_entry_screen(*renderer, title_font, menu_font, profile_name, status_message, status_is_error);
        SDL_Delay(16);
    }

    SDL_StopTextInput();

    if (!accepted)
        return ft_string();

    return profile_name;
#else
    (void)window;
    (void)renderer;
    (void)title_font;
    (void)menu_font;
    (void)existing_profiles;
    out_quit_requested = true;
    return ft_string();
#endif
}

