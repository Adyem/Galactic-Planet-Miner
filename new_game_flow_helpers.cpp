#include "main_menu_system.hpp"

#include "app_constants.hpp"
#include "game_bootstrap.hpp"
#include "menu_localization.hpp"
#include "player_profile.hpp"

#include "libft/File/file_utils.hpp"
#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/Libft/libft.hpp"

namespace new_game_flow
{
    inline constexpr unsigned int kMaxSaveNameLength = 24U;

    bool is_save_character_allowed(char character) noexcept
    {
        const bool is_lower = character >= 'a' && character <= 'z';
        const bool is_upper = character >= 'A' && character <= 'Z';
        const bool is_digit = character >= '0' && character <= '9';
        return is_lower || is_upper || is_digit;
    }

    bool append_save_character(ft_string &save_name, char character) noexcept
    {
        if (!is_save_character_allowed(character))
            return false;

        if (save_name.size() >= static_cast<size_t>(kMaxSaveNameLength))
            return false;

        save_name.append(character);
        return true;
    }

    void remove_last_save_character(ft_string &save_name) noexcept
    {
        const size_t current_size = save_name.size();
        if (current_size == 0U)
            return;
        save_name.erase(current_size - 1U, 1U);
    }

    bool save_name_is_valid(const ft_string &save_name) noexcept
    {
        return !save_name.empty();
    }

    ft_string build_save_file_path(const ft_string &commander_name, const ft_string &save_name)
    {
        if (commander_name.empty())
            return ft_string();

        ft_string base_directory = player_profile_resolve_save_directory(commander_name);
        if (base_directory.empty())
            return ft_string();

        ft_string file_name = save_name;
        file_name.append(".json");

        ft_string full_path = file_path_join(base_directory.c_str(), file_name.c_str());
        return full_path;
    }

    bool create_new_game_save(const ft_string &commander_name, const ft_string &save_name, ft_string &out_error,
        ft_string &out_save_path) noexcept
    {
        out_error.clear();
        out_save_path.clear();

        ft_string full_path = build_save_file_path(commander_name, save_name);
        if (full_path.empty())
        {
            out_error = menu_localize(
                "new_game.error.resolve_location", "Unable to resolve the save file location.");
            return false;
        }

        int exists_result = file_exists(full_path.c_str());
        if (exists_result < 0)
        {
            out_error = menu_localize("new_game.error.check_existing", "Unable to check existing saves.");
            return false;
        }

        if (exists_result > 0)
        {
            out_error = menu_localize("new_game.error.conflict", "A save with that name already exists.");
            return false;
        }

        if (!game_bootstrap_create_quicksave_with_commander(full_path.c_str(), commander_name))
        {
            out_error = menu_localize("new_game.error.create_failure", "Failed to create the save file.");
            return false;
        }

        out_save_path = full_path;
        return true;
    }

#if GALACTIC_HAVE_SDL2
    void render_new_game_screen(SDL_Renderer &renderer, TTF_Font *title_font, TTF_Font *menu_font,
        const ft_string &save_name, const ft_string &status_message, bool status_is_error)
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
            ft_string title_text = menu_localize("new_game.title", "Create New Game");
            SDL_Texture *title_texture
                = create_text_texture(renderer, *title_font, title_text, title_color, title_rect);
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
            bool is_placeholder = save_name.empty();
            ft_string display_text;
            if (is_placeholder)
                display_text = menu_localize("new_game.placeholder", "Enter save name");
            else
                display_text = save_name;

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

            ft_string instructions = menu_localize(
                "new_game.instructions", "Letters and numbers only. Press Enter to confirm.");
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
                SDL_Texture *status_texture = create_text_texture(renderer, *menu_font, status_message, status_color,
                    status_rect);
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

namespace new_game_flow_testing
{
    unsigned int max_save_name_length() noexcept
    {
        return new_game_flow::kMaxSaveNameLength;
    }

    bool is_character_allowed(char character) noexcept
    {
        return new_game_flow::is_save_character_allowed(character);
    }

    bool append_character(ft_string &save_name, char character) noexcept
    {
        return new_game_flow::append_save_character(save_name, character);
    }

    void remove_last_character(ft_string &save_name) noexcept
    {
        new_game_flow::remove_last_save_character(save_name);
    }

    bool validate_save_name(const ft_string &save_name) noexcept
    {
        return new_game_flow::save_name_is_valid(save_name);
    }

    ft_string compute_save_file_path(const ft_string &commander_name, const ft_string &save_name)
    {
        return new_game_flow::build_save_file_path(commander_name, save_name);
    }

    bool create_save_file(const ft_string &commander_name, const ft_string &save_name, ft_string &out_error) noexcept
    {
        ft_string created_path;
        return new_game_flow::create_new_game_save(commander_name, save_name, out_error, created_path);
    }

    bool create_save_file_with_path(const ft_string &commander_name, const ft_string &save_name,
        ft_string &out_save_path, ft_string &out_error) noexcept
    {
        return new_game_flow::create_new_game_save(commander_name, save_name, out_error, out_save_path);
    }
}
