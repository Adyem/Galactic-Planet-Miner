#include "main_menu_system.hpp"

#include "app_constants.hpp"
#include "player_profile.hpp"
#include "ui_input.hpp"

#include "libft/CPP_class/class_nullptr.hpp"

namespace
{
#if GALACTIC_HAVE_SDL2
    size_t find_profile_index(const ft_vector<ft_string> &profiles, const ft_string &profile_name) noexcept
    {
        for (size_t index = 0; index < profiles.size(); ++index)
        {
            if (profiles[index] == profile_name)
                return index;
        }
        return profiles.size();
    }

    void rebuild_profile_menu(ft_ui_menu &menu, const ft_vector<ft_string> &profiles, const ft_string &current_profile,
        size_t preferred_profile_index, bool select_create_item)
    {
        const ft_rect base_rect(460, 220, 360, 56);
        const int      spacing = 18;

        ft_vector<ft_menu_item> items;
        const size_t             profile_count = profiles.size();
        items.reserve(profile_count + 1U);

        for (size_t index = 0; index < profile_count; ++index)
        {
            ft_rect item_rect = base_rect;
            item_rect.top += static_cast<int>(index) * (base_rect.height + spacing);

            ft_string label = profiles[index];
            if (profiles[index] == current_profile)
                label.append(" (Current)");

            ft_string identifier("profile:");
            identifier.append(ft_to_string(static_cast<int>(index)));

            items.push_back(ft_menu_item(identifier, label, item_rect));
        }

        ft_rect create_rect = base_rect;
        create_rect.top += static_cast<int>(profile_count) * (base_rect.height + spacing);
        items.push_back(ft_menu_item(ft_string("profile:create"), ft_string("Create New Profile"), create_rect));

        menu.set_items(items);

        if (select_create_item)
        {
            menu.set_selected_index(static_cast<int>(profile_count));
        }
        else if (preferred_profile_index < profile_count)
        {
            menu.set_selected_index(static_cast<int>(preferred_profile_index));
        }
        else if (profile_count > 0U)
        {
            menu.set_selected_index(0);
        }
        else
        {
            menu.set_selected_index(static_cast<int>(profile_count));
        }
    }

    void render_profile_management_screen(SDL_Renderer &renderer, const ft_ui_menu &menu, TTF_Font *title_font,
        TTF_Font *menu_font, const ft_string &status_message, bool status_is_error)
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
            SDL_Texture *title_texture = create_text_texture(renderer, *title_font, ft_string("Commander Profiles"), title_color,
                title_rect);
            if (title_texture != ft_nullptr)
            {
                title_rect.x = output_width / 2 - title_rect.w / 2;
                title_rect.y = 96;
                SDL_RenderCopy(&renderer, title_texture, ft_nullptr, &title_rect);
                SDL_DestroyTexture(title_texture);
            }
        }

        if (menu_font != ft_nullptr)
        {
            SDL_Color info_color = {170, 180, 210, 255};
            SDL_Rect  info_rect;
            ft_string info_text("Enter or click to activate a profile. Delete removes the selected profile. Esc cancels.");
            SDL_Texture *info_texture = create_text_texture(renderer, *menu_font, info_text, info_color, info_rect);
            if (info_texture != ft_nullptr)
            {
                info_rect.x = output_width / 2 - info_rect.w / 2;
                info_rect.y = 164;
                SDL_RenderCopy(&renderer, info_texture, ft_nullptr, &info_rect);
                SDL_DestroyTexture(info_texture);
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

        if (menu_font != ft_nullptr && !status_message.empty())
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
                status_rect.y = output_height - status_rect.h - 120;
                SDL_RenderCopy(&renderer, status_texture, ft_nullptr, &status_rect);
                SDL_DestroyTexture(status_texture);
            }
        }

        SDL_RenderPresent(&renderer);
    }
#endif
}

ft_string run_profile_management_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    const ft_string &current_profile, bool &out_quit_requested)
{
#if GALACTIC_HAVE_SDL2
    out_quit_requested = false;
    if (window == ft_nullptr || renderer == ft_nullptr)
        return ft_string();

    ft_vector<ft_string> profiles;
    if (!player_profile_list(profiles))
        return ft_string();

    if (profiles.empty())
    {
        bool creation_quit = false;
        ft_string created_profile = run_profile_entry_flow(window, renderer, title_font, menu_font, &profiles, creation_quit);
        if (creation_quit)
        {
            out_quit_requested = true;
            return ft_string();
        }
        if (created_profile.empty())
            return ft_string();

        player_profile_list(profiles);
    }

    size_t preferred_index = find_profile_index(profiles, current_profile);
    ft_ui_menu menu;
    rebuild_profile_menu(menu, profiles, current_profile, preferred_index, false);
    int last_selected_index = menu.get_selected_index();

    ft_string status_message;
    bool      status_is_error = false;
    bool      running = true;
    int       pending_delete_index = -1;
    ft_string pending_delete_name;

    while (running)
    {
        ft_mouse_state    mouse_state;
        ft_keyboard_state keyboard_state;
        bool              activate_requested = false;
        bool              delete_requested = false;

        SDL_Event event;
        while (SDL_PollEvent(&event) == 1)
        {
            if (event.type == SDL_QUIT)
            {
                out_quit_requested = true;
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
                else if (event.key.keysym.sym == SDLK_DELETE || event.key.keysym.sym == SDLK_BACKSPACE)
                    delete_requested = true;
                else if (event.key.keysym.sym == SDLK_ESCAPE)
                    running = false;
            }
        }

        if (!running)
            break;

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

        int current_selected_index = menu.get_selected_index();
        if (current_selected_index != last_selected_index)
        {
            pending_delete_index = -1;
            pending_delete_name.clear();
            status_message.clear();
            status_is_error = false;
            last_selected_index = current_selected_index;
        }

        size_t profile_count = profiles.size();
        int    activation_index = -1;
        if (mouse_state.left_released)
            activation_index = menu.get_hovered_index();
        if (activation_index < 0 && activate_requested)
            activation_index = menu.get_selected_index();

        if (activation_index >= 0)
        {
            if (static_cast<size_t>(activation_index) < profile_count)
                return profiles[static_cast<size_t>(activation_index)];

            if (static_cast<size_t>(activation_index) == profile_count)
            {
                bool creation_quit = false;
                ft_string new_profile = run_profile_entry_flow(window, renderer, title_font, menu_font, &profiles, creation_quit);
                if (creation_quit)
                {
                    out_quit_requested = true;
                    return ft_string();
                }
                if (!new_profile.empty())
                {
                    player_profile_list(profiles);
                    profile_count = profiles.size();
                    preferred_index = find_profile_index(profiles, new_profile);
                    rebuild_profile_menu(menu, profiles, current_profile, preferred_index, false);
                    last_selected_index = menu.get_selected_index();
                    status_message = ft_string("Created profile: ");
                    status_message.append(new_profile);
                    status_is_error = false;
                    pending_delete_index = -1;
                    pending_delete_name.clear();
                }
            }
        }

        if (delete_requested)
        {
            int selected_index = menu.get_selected_index();
            if (selected_index >= 0 && static_cast<size_t>(selected_index) < profile_count)
            {
                if (profile_count <= 1U)
                {
                    status_message = ft_string("At least one profile must remain.");
                    status_is_error = true;
                    pending_delete_index = -1;
                    pending_delete_name.clear();
                }
                else if (pending_delete_index == selected_index)
                {
                    ft_string name_to_delete = profiles[static_cast<size_t>(selected_index)];
                    if (player_profile_delete(name_to_delete))
                    {
                        player_profile_list(profiles);
                        profile_count = profiles.size();
                        if (profile_count == 0U)
                        {
                            status_message = ft_string("No profiles available.");
                            status_is_error = true;
                            return ft_string();
                        }

                        size_t new_preferred_index = static_cast<size_t>(selected_index);
                        if (new_preferred_index >= profile_count)
                            new_preferred_index = profile_count - 1U;

                        rebuild_profile_menu(menu, profiles, current_profile, new_preferred_index, false);
                        last_selected_index = menu.get_selected_index();
                        status_message = ft_string("Deleted profile: ");
                        status_message.append(name_to_delete);
                        status_is_error = false;
                    }
                    else
                    {
                        status_message = ft_string("Unable to delete profile.");
                        status_is_error = true;
                    }

                    pending_delete_index = -1;
                    pending_delete_name.clear();
                }
                else
                {
                    pending_delete_index = selected_index;
                    pending_delete_name = profiles[static_cast<size_t>(selected_index)];
                    status_message = ft_string("Press Delete again to remove: ");
                    status_message.append(pending_delete_name);
                    status_is_error = true;
                }
            }
        }

        render_profile_management_screen(*renderer, menu, title_font, menu_font, status_message, status_is_error);
        SDL_Delay(16);
    }

    return ft_string();
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

