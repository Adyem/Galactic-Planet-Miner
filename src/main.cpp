#include "ui_menu.hpp"
#include "player_profile.hpp"

#include "../libft/CPP_class/class_nullptr.hpp"
#include "../libft/Libft/libft.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace
{
    constexpr unsigned int kWindowWidth = 1280U;
    constexpr unsigned int kWindowHeight = 720U;
    static const unsigned int kMaxProfileNameLength = 24U;

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

    bool save_profile_preferences(SDL_Window *window, const ft_string &profile_name) noexcept
    {
        if (window == ft_nullptr)
            return false;

        PlayerProfilePreferences preferences;
        preferences.commander_name = profile_name;

        int window_width = static_cast<int>(kWindowWidth);
        int window_height = static_cast<int>(kWindowHeight);
        SDL_GetWindowSize(window, &window_width, &window_height);

        if (window_width > 0)
            preferences.window_width = static_cast<unsigned int>(window_width);
        if (window_height > 0)
            preferences.window_height = static_cast<unsigned int>(window_height);

        return player_profile_save(preferences);
    }

    ft_vector<ft_menu_item> build_main_menu_items()
    {
        const ft_rect base_rect(460, 220, 360, 56);
        const int      spacing = 22;

        const char *identifiers[] = {"new_game", "load", "settings", "swap_profile", "exit"};
        const char *labels[] = {"New Game", "Load", "Settings", "Swap Profile", "Exit"};

        ft_vector<ft_menu_item> items;
        items.reserve(5U);

        for (int index = 0; index < 5; ++index)
        {
            ft_rect item_rect = base_rect;
            item_rect.top += index * (base_rect.height + spacing);
            items.push_back(ft_menu_item(
                ft_string(identifiers[index]),
                ft_string(labels[index]),
                item_rect));
        }

        return items;
    }

    TTF_Font *resolve_font(int point_size)
    {
        struct font_cache_entry
        {
            int       size;
            TTF_Font *font;
            bool      attempted;
        };

        static font_cache_entry font_cache[] = {
            {48, ft_nullptr, false},
            {28, ft_nullptr, false}
        };

        for (size_t index = 0; index < sizeof(font_cache) / sizeof(font_cache[0]); ++index)
        {
            font_cache_entry &entry = font_cache[index];
            if (entry.size != point_size)
                continue;

            if (!entry.attempted)
            {
                entry.attempted = true;

                const char *font_paths[] = {
                    "fonts/DejaVuSans.ttf",
                    "fonts/LiberationSans-Regular.ttf",
                    "assets/fonts/DejaVuSans.ttf",
                    "assets/fonts/LiberationSans-Regular.ttf",
                    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
                    "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
                    "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
                    "/System/Library/Fonts/SFNSDisplay.ttf",
                    "/System/Library/Fonts/Supplemental/Arial.ttf",
                    "/Library/Fonts/Arial.ttf",
                    "C:/Windows/Fonts/segoeui.ttf",
                    "C:/Windows/Fonts/arial.ttf",
                    "C:/Windows/Fonts/calibri.ttf"
                };

                const size_t path_count = sizeof(font_paths) / sizeof(font_paths[0]);
                for (size_t path_index = 0; path_index < path_count; ++path_index)
                {
                    const char *path = font_paths[path_index];
                    if (path == ft_nullptr)
                        continue;

                    entry.font = TTF_OpenFont(path, point_size);
                    if (entry.font != ft_nullptr)
                        break;
                }
            }

            return entry.font;
        }

        return ft_nullptr;
    }

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

    void handle_menu_activation(const ft_menu_item &item, bool &running)
    {
        if (item.identifier == "exit")
            running = false;
    }

    SDL_Texture *create_text_texture(SDL_Renderer &renderer, TTF_Font &font, const ft_string &text, const SDL_Color &color, SDL_Rect &out_rect)
    {
        if (text.c_str() == ft_nullptr)
            return ft_nullptr;

        SDL_Surface *surface = TTF_RenderUTF8_Blended(&font, text.c_str(), color);
        if (surface == ft_nullptr)
            return ft_nullptr;

        SDL_Texture *texture = SDL_CreateTextureFromSurface(&renderer, surface);
        if (texture != ft_nullptr)
        {
            out_rect.x = 0;
            out_rect.y = 0;
            out_rect.w = surface->w;
            out_rect.h = surface->h;
        }

        SDL_FreeSurface(surface);
        return texture;
    }

    void render_menu(SDL_Renderer &renderer, const ft_ui_menu &menu, TTF_Font *title_font, TTF_Font *menu_font, int window_width, int window_height, const ft_string &active_profile_name)
    {
        SDL_SetRenderDrawColor(&renderer, 12, 16, 28, 255);
        SDL_RenderClear(&renderer);

        int output_width = window_width;
        int output_height = window_height;
        SDL_GetRendererOutputSize(&renderer, &output_width, &output_height);

        if (title_font != ft_nullptr)
        {
            SDL_Color title_color = {220, 220, 245, 255};
            SDL_Rect  title_rect;
            SDL_Texture *title_texture = create_text_texture(renderer, *title_font, ft_string("Galactic Planet Miner"), title_color, title_rect);
            if (title_texture != ft_nullptr)
            {
                title_rect.x = output_width / 2 - title_rect.w / 2;
                title_rect.y = 96;
                SDL_RenderCopy(&renderer, title_texture, ft_nullptr, &title_rect);
                SDL_DestroyTexture(title_texture);
            }
        }

        if (menu_font != ft_nullptr && !active_profile_name.empty())
        {
            ft_string profile_label("Profile: ");
            profile_label.append(active_profile_name);

            SDL_Color profile_color = {200, 210, 230, 255};
            SDL_Rect  profile_rect;
            SDL_Texture *profile_texture = create_text_texture(renderer, *menu_font, profile_label, profile_color, profile_rect);
            if (profile_texture != ft_nullptr)
            {
                profile_rect.x = output_width / 2 - profile_rect.w / 2;
                profile_rect.y = 164;
                SDL_RenderCopy(&renderer, profile_texture, ft_nullptr, &profile_rect);
                SDL_DestroyTexture(profile_texture);
            }
        }

        const ft_vector<ft_menu_item> &items = menu.get_items();
        const int hovered_index = menu.get_hovered_index();
        const int selected_index = menu.get_selected_index();

        for (size_t index = 0; index < items.size(); ++index)
        {
            const ft_menu_item &item = items[index];

            const bool is_hovered = static_cast<int>(index) == hovered_index;
            const bool is_selected = static_cast<int>(index) == selected_index;

            const Uint8 r = is_hovered ? 56 : (is_selected ? 40 : 28);
            const Uint8 g = is_hovered ? 84 : (is_selected ? 64 : 36);
            const Uint8 b = is_hovered ? 140 : (is_selected ? 112 : 60);

            SDL_Rect button_rect;
            button_rect.x = item.bounds.left;
            button_rect.y = item.bounds.top;
            button_rect.w = item.bounds.width;
            button_rect.h = item.bounds.height;

            SDL_SetRenderDrawColor(&renderer, r, g, b, 255);
            SDL_RenderFillRect(&renderer, &button_rect);

            SDL_SetRenderDrawColor(&renderer, 90, 110, 160, 255);
            SDL_RenderDrawRect(&renderer, &button_rect);

            if (menu_font != ft_nullptr)
            {
                SDL_Color text_color = {255, 255, 255, 255};
                SDL_Rect  text_rect;
                SDL_Texture *text_texture = create_text_texture(renderer, *menu_font, item.label, text_color, text_rect);
                if (text_texture != ft_nullptr)
                {
                    text_rect.x = item.bounds.left + (item.bounds.width - text_rect.w) / 2;
                    text_rect.y = item.bounds.top + (item.bounds.height - text_rect.h) / 2;
                    SDL_RenderCopy(&renderer, text_texture, ft_nullptr, &text_rect);
                    SDL_DestroyTexture(text_texture);
                }
            }
        }

        SDL_RenderPresent(&renderer);
    }

    void render_profile_entry_screen(SDL_Renderer &renderer, TTF_Font *title_font, TTF_Font *menu_font, const ft_string &profile_name, const ft_string &status_message, bool status_is_error)
    {
        SDL_SetRenderDrawColor(&renderer, 12, 16, 28, 255);
        SDL_RenderClear(&renderer);

        int output_width = static_cast<int>(kWindowWidth);
        int output_height = static_cast<int>(kWindowHeight);
        SDL_GetRendererOutputSize(&renderer, &output_width, &output_height);

        if (title_font != ft_nullptr)
        {
            SDL_Color title_color = {220, 220, 245, 255};
            SDL_Rect  title_rect;
            SDL_Texture *title_texture = create_text_texture(renderer, *title_font, ft_string("Commander Profile"), title_color, title_rect);
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
            SDL_Texture *instruction_texture = create_text_texture(renderer, *menu_font, instructions, instruction_color, instruction_rect);
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

    ft_string run_profile_entry_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font)
    {
        if (window == ft_nullptr || renderer == ft_nullptr)
            return ft_string();

        ft_string profile_name;
        ft_string status_message;
        bool status_is_error = false;
        bool running = true;
        bool accepted = false;

        SDL_StartTextInput();

        while (running && !accepted)
        {
            SDL_Event event;
            while (SDL_PollEvent(&event) == 1)
            {
                if (event.type == SDL_QUIT)
                {
                    running = false;
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
        static_cast<int>(kWindowWidth),
        static_cast<int>(kWindowHeight),
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
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    TTF_Font *title_font = resolve_font(48);
    TTF_Font *menu_font = resolve_font(28);

    ft_string active_profile_name = run_profile_entry_flow(window, renderer, title_font, menu_font);
    if (active_profile_name.empty())
    {
        if (renderer != ft_nullptr)
            SDL_DestroyRenderer(renderer);
        if (window != ft_nullptr)
            SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    ft_ui_menu menu;
    menu.set_items(build_main_menu_items());

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

        if (mouse_state.left_released)
        {
            const int index = menu.get_hovered_index();
            const ft_menu_item *hovered_item = menu_item_from_index(menu, index);
            if (hovered_item != ft_nullptr)
                handle_menu_activation(*hovered_item, running);
        }

        if (activate_requested)
        {
            const ft_menu_item *selected_item = menu.get_selected_item();
            if (selected_item != ft_nullptr)
                handle_menu_activation(*selected_item, running);
        }

        int window_width = 0;
        int window_height = 0;
        SDL_GetWindowSize(window, &window_width, &window_height);
        render_menu(*renderer, menu, title_font, menu_font, window_width, window_height, active_profile_name);
    }

    if (renderer != ft_nullptr)
        SDL_DestroyRenderer(renderer);

    if (window != ft_nullptr)
        SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();

    return 0;
}
