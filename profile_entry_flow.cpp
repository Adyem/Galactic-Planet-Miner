#include "main_menu_system.hpp"

#include "app_constants.hpp"

#include "libft/CPP_class/class_nullptr.hpp"

namespace
{
#if GALACTIC_HAVE_SDL2
    constexpr unsigned int kMaxProfileNameLength = 24U;

    enum e_profile_append_result
    {
        PROFILE_APPEND_REJECTED = 0,
        PROFILE_APPEND_ACCEPTED = 1,
        PROFILE_APPEND_MAX_LENGTH = 2
    };

    bool is_profile_character_allowed(char character) noexcept
    {
        const bool is_lower = (character >= 'a' && character <= 'z');
        const bool is_upper = (character >= 'A' && character <= 'Z');
        const bool is_digit = (character >= '0' && character <= '9');
        return is_lower || is_upper || is_digit;
    }

    e_profile_append_result append_profile_character(ft_string &profile_name, char character) noexcept
    {
        if (!is_profile_character_allowed(character))
            return PROFILE_APPEND_REJECTED;

        if (profile_name.size() >= static_cast<size_t>(kMaxProfileNameLength))
            return PROFILE_APPEND_MAX_LENGTH;

        profile_name.append(character);
        return PROFILE_APPEND_ACCEPTED;
    }

    void remove_last_profile_character(ft_string &profile_name) noexcept
    {
        const size_t current_size = profile_name.size();
        if (current_size == 0U)
            return;
        profile_name.erase(current_size - 1U, 1U);
    }

    bool profile_name_is_valid(const ft_string &profile_name) noexcept
    {
        return !profile_name.empty();
    }

    bool profile_name_exists(const ft_vector<ft_string> &profiles, const ft_string &candidate) noexcept
    {
        for (size_t index = 0; index < profiles.size(); ++index)
        {
            if (profiles[index] == candidate)
                return true;
        }
        return false;
    }

    void render_profile_entry_screen(SDL_Renderer &renderer, TTF_Font *title_font, TTF_Font *menu_font,
        const ft_string &profile_name, const ft_string &status_message, bool status_is_error)
    {
        SDL_SetRenderDrawColor(&renderer, 12, 16, 28, 255);
        SDL_RenderClear(&renderer);

        int output_width = static_cast<int>(app_constants::kWindowWidth);
        int output_height = static_cast<int>(app_constants::kWindowHeight);
        SDL_GetRendererOutputSize(&renderer, &output_width, &output_height);

        if (title_font != ft_nullptr)
        {
            SDL_Color title_color = {220, 220, 245, 255};
            SDL_Rect  title_rect;
            SDL_Texture *title_texture = create_text_texture(renderer, *title_font, ft_string("Commander Profile"), title_color,
                title_rect);
            if (title_texture != ft_nullptr)
            {
                title_rect.x = output_width / 2 - title_rect.w / 2;
                title_rect.y = 96;
                SDL_RenderCopy(&renderer, title_texture, ft_nullptr, &title_rect);
                SDL_DestroyTexture(title_texture);
            }
        }

        SDL_Rect input_rect;
        input_rect.w = 520;
        input_rect.h = 68;
        input_rect.x = output_width / 2 - input_rect.w / 2;
        input_rect.y = output_height / 2 - input_rect.h / 2;

        SDL_SetRenderDrawColor(&renderer, 18, 24, 44, 240);
        SDL_RenderFillRect(&renderer, &input_rect);
        SDL_SetRenderDrawColor(&renderer, 90, 110, 160, 255);
        SDL_RenderDrawRect(&renderer, &input_rect);

        int caret_x = input_rect.x + 18;

        if (menu_font != ft_nullptr)
        {
            bool is_placeholder = profile_name.empty();
            ft_string display_text;
            if (is_placeholder)
                display_text = ft_string("Enter profile name");
            else
                display_text = profile_name;

            SDL_Color text_color;
            if (is_placeholder)
            {
                text_color.r = 140;
                text_color.g = 150;
                text_color.b = 170;
                text_color.a = 255;
            }
            else
            {
                text_color.r = 230;
                text_color.g = 235;
                text_color.b = 245;
                text_color.a = 255;
            }

            SDL_Rect text_rect;
            SDL_Texture *text_texture = create_text_texture(renderer, *menu_font, display_text, text_color, text_rect);
            if (text_texture != ft_nullptr)
            {
                text_rect.x = input_rect.x + 18;
                text_rect.y = input_rect.y + (input_rect.h - text_rect.h) / 2;
                SDL_RenderCopy(&renderer, text_texture, ft_nullptr, &text_rect);
                caret_x = text_rect.x + text_rect.w + 4;
                SDL_DestroyTexture(text_texture);
            }

            if (!is_placeholder)
            {
                SDL_Rect caret_rect;
                caret_rect.x = caret_x;
                caret_rect.y = input_rect.y + 8;
                caret_rect.w = 2;
                caret_rect.h = input_rect.h - 16;
                SDL_SetRenderDrawColor(&renderer, 210, 220, 250, 255);
                SDL_RenderFillRect(&renderer, &caret_rect);
            }

            ft_string instructions("Letters and numbers only. Press Enter to confirm.");
            SDL_Color instruction_color = {170, 180, 210, 255};
            SDL_Rect  instruction_rect;
            SDL_Texture *instruction_texture = create_text_texture(renderer, *menu_font, instructions, instruction_color,
                instruction_rect);
            if (instruction_texture != ft_nullptr)
            {
                instruction_rect.x = output_width / 2 - instruction_rect.w / 2;
                instruction_rect.y = input_rect.y + input_rect.h + 36;
                SDL_RenderCopy(&renderer, instruction_texture, ft_nullptr, &instruction_rect);
                SDL_DestroyTexture(instruction_texture);
            }

            if (!status_message.empty())
            {
                SDL_Color status_color;
                if (status_is_error)
                {
                    status_color.r = 220;
                    status_color.g = 90;
                    status_color.b = 90;
                    status_color.a = 255;
                }
                else
                {
                    status_color.r = 180;
                    status_color.g = 200;
                    status_color.b = 220;
                    status_color.a = 255;
                }

                SDL_Rect status_rect;
                SDL_Texture *status_texture = create_text_texture(renderer, *menu_font, status_message, status_color, status_rect);
                if (status_texture != ft_nullptr)
                {
                    status_rect.x = output_width / 2 - status_rect.w / 2;
                    status_rect.y = input_rect.y + input_rect.h + 80;
                    SDL_RenderCopy(&renderer, status_texture, ft_nullptr, &status_rect);
                    SDL_DestroyTexture(status_texture);
                }
            }
        }

        SDL_RenderPresent(&renderer);
    }
#endif
}

ft_string run_profile_entry_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    const ft_vector<ft_string> *existing_profiles, bool &out_quit_requested)
{
#if GALACTIC_HAVE_SDL2
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

