#include "main_menu_system.hpp"

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

void render_main_menu(SDL_Renderer &renderer, const ft_ui_menu &menu, TTF_Font *title_font, TTF_Font *menu_font,
    int window_width, int window_height, const ft_string &active_profile_name)
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
        SDL_Texture *title_texture = create_text_texture(renderer, *title_font, ft_string("Galactic Planet Miner"), title_color,
            title_rect);
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

