namespace profile_management_flow_detail
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

        ft_rect viewport = base_rect;
        const int window_height = static_cast<int>(app_constants::kWindowHeight);
        const int reserved_bottom = 220;
        viewport.height = window_height - base_rect.top - reserved_bottom;
        if (viewport.height < base_rect.height)
            viewport.height = base_rect.height;
        menu.set_viewport_bounds(viewport);

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

        const ft_rect &viewport = menu.get_viewport_bounds();
        const bool clip_enabled = viewport.width > 0 && viewport.height > 0;
        SDL_Rect clip_rect;
        clip_rect.x = 0;
        clip_rect.y = 0;
        clip_rect.w = 0;
        clip_rect.h = 0;
        int clip_bottom = 0;

        if (clip_enabled)
        {
            clip_rect.x = viewport.left;
            clip_rect.y = viewport.top;
            clip_rect.w = viewport.width;
            clip_rect.h = viewport.height;
            clip_bottom = clip_rect.y + clip_rect.h;
            SDL_RenderSetClipRect(&renderer, &clip_rect);
        }

        const int scroll_offset = menu.get_scroll_offset();

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
            button_rect.y = item.bounds.top - scroll_offset;
            button_rect.w = item.bounds.width;
            button_rect.h = item.bounds.height;

            if (clip_enabled && (button_rect.y + button_rect.h <= clip_rect.y || button_rect.y >= clip_bottom))
                continue;

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
                    text_rect.y = button_rect.y + (item.bounds.height - text_rect.h) / 2;
                    SDL_RenderCopy(&renderer, text_texture, ft_nullptr, &text_rect);
                    SDL_DestroyTexture(text_texture);
                }
            }
        }

        if (clip_enabled)
            SDL_RenderSetClipRect(&renderer, ft_nullptr);

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
