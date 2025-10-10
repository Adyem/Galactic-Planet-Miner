                }
                slot.metadata_error = false;
            }
            else
            {
                slot.metadata_label
                    = menu_localize("load_menu.metadata.unreadable", "Unable to read metadata");
                slot.metadata_available = false;
                slot.metadata_error = true;
                slot.difficulty_known = false;
                slot.difficulty_value = 0;
                if (out_errors != ft_nullptr)
                {
                    ft_vector<StringTableReplacement> replacements;
                    replacements.reserve(1U);
                    StringTableReplacement replacement;
                    replacement.key = ft_string("slot");
                    replacement.value = label;
                    replacements.push_back(replacement);
                    out_errors->push_back(menu_localize_format(
                        "load_menu.errors.metadata_parse",
                        "Save \"{{slot}}\" could not be read (metadata parsing failed).",
                        replacements));
                }
            }
            out_slots.push_back(slot);
        }

        file_closedir(directory_stream);

        ft_sort(out_slots.begin(), out_slots.end(), [](const save_slot_entry &lhs, const save_slot_entry &rhs) {
            const int compare = ft_strcmp(lhs.label.c_str(), rhs.label.c_str());
            return compare < 0;
        });

        return true;
    }

#if GALACTIC_HAVE_SDL2
    void rebuild_load_menu(ft_ui_menu &menu, const ft_vector<save_slot_entry> &slots, bool has_selection)
    {
        const ft_rect base_rect(460, 220, 360, 72);
        const int      spacing = 18;

        ft_vector<ft_menu_item> items;
        const size_t             slot_count = slots.size();
        items.reserve(slot_count + 1U);

        for (size_t index = 0; index < slot_count; ++index)
        {
            ft_rect item_rect = base_rect;
            item_rect.top += static_cast<int>(index) * (base_rect.height + spacing);

            const save_slot_entry &slot = slots[index];
            ft_menu_item           item(slot.identifier, slot.label, item_rect);
            item.enabled = true;
            if (!slot.metadata_label.empty())
                item.description = slot.metadata_label;
            items.push_back(item);
        }

        ft_rect cancel_rect = base_rect;
        cancel_rect.top += static_cast<int>(slot_count) * (base_rect.height + spacing);
        ft_menu_item cancel_item(ft_string("action:cancel"),
            menu_localize("load_menu.menu.cancel", "Cancel"), cancel_rect);
        items.push_back(cancel_item);

        menu.set_items(items);

        ft_rect viewport = base_rect;
        const int window_height = static_cast<int>(app_constants::kWindowHeight);
        const int reserved_bottom = 220;
        viewport.height = window_height - base_rect.top - reserved_bottom;
        if (viewport.height < base_rect.height)
            viewport.height = base_rect.height;
        menu.set_viewport_bounds(viewport);

        if (has_selection && slot_count > 0U)
            menu.set_selected_index(0);
        else
            menu.set_selected_index(static_cast<int>(slot_count));
    }

    void render_load_screen(SDL_Renderer &renderer, const ft_ui_menu &menu, TTF_Font *title_font, TTF_Font *menu_font,
        const ft_string &commander_name, const ft_string &status_message, bool status_is_error)
    {
        SDL_SetRenderDrawColor(&renderer, 12, 16, 28, 255);
        SDL_RenderClear(&renderer);

        int output_width = static_cast<int>(app_constants::kWindowWidth);
        int output_height = static_cast<int>(app_constants::kWindowHeight);
        SDL_GetRendererOutputSize(&renderer, &output_width, &output_height);

        if (title_font != ft_nullptr)
        {
            ft_string heading = menu_localize("load_menu.menu.heading", "Load Campaign");
            SDL_Color title_color = {220, 220, 245, 255};
            SDL_Rect  title_rect;
            SDL_Texture *title_texture = create_text_texture(renderer, *title_font, heading, title_color, title_rect);
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
            ft_string info_text = menu_localize("load_menu.menu.instructions",
                "Select a save to prepare for campaign resume. Enter loads; Delete removes; R renames; Esc cancels.");
            if (!commander_name.empty())
            {
                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(1U);
                StringTableReplacement replacement;
                replacement.key = ft_string("commander");
                replacement.value = commander_name;
                replacements.push_back(replacement);
                info_text.append(menu_localize_format("load_menu.menu.instructions_commander",
                    " Commander: {{commander}}.", replacements));
            }
            SDL_Color info_color = {170, 180, 210, 255};
            SDL_Rect  info_rect;
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
        SDL_Rect clip_rect = {0, 0, 0, 0};
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
            const bool          is_hovered = static_cast<int>(index) == hovered_index;
            const bool          is_selected = static_cast<int>(index) == selected_index;
            const bool          is_disabled = !item.enabled;

            Uint8 r = 28;
            Uint8 g = 36;
            Uint8 b = 60;

            if (is_disabled)
            {
                r = 30;
                g = 34;
                b = 44;
                if (is_hovered)
                {
                    r = 42;
                    g = 46;
                    b = 60;
                }
            }
            else if (is_hovered)
            {
                r = 56;
                g = 84;
                b = 140;
            }
            else if (is_selected)
            {
                r = 40;
                g = 64;
                b = 112;
            }

            SDL_Rect button_rect;
            button_rect.x = item.bounds.left;
            button_rect.y = item.bounds.top - scroll_offset;
            button_rect.w = item.bounds.width;
            button_rect.h = item.bounds.height;
            if (clip_enabled && (button_rect.y + button_rect.h <= clip_rect.y || button_rect.y >= clip_bottom))
                continue;

            SDL_SetRenderDrawColor(&renderer, r, g, b, 255);
            SDL_RenderFillRect(&renderer, &button_rect);
            const Uint8 border_r = is_disabled ? 70 : 90;
            const Uint8 border_g = is_disabled ? 80 : 110;
            const Uint8 border_b = is_disabled ? 120 : 160;
            SDL_SetRenderDrawColor(&renderer, border_r, border_g, border_b, 255);
            SDL_RenderDrawRect(&renderer, &button_rect);

            if (menu_font != ft_nullptr)
            {
                SDL_Color text_color = {255, 255, 255, 255};
                if (is_disabled)
                {
                    text_color.r = 188;
                    text_color.g = 196;
                    text_color.b = 210;
                }
                SDL_Rect     text_rect;
                SDL_Texture *text_texture = create_text_texture(renderer, *menu_font, item.label, text_color, text_rect);
                if (text_texture != ft_nullptr)
                {
                    const int text_left = item.bounds.left + 18;
                    int       text_top = button_rect.y + 12;
                    text_rect.x = text_left;
                    text_rect.y = text_top;
                    SDL_RenderCopy(&renderer, text_texture, ft_nullptr, &text_rect);
                    SDL_DestroyTexture(text_texture);
                    text_top = text_rect.y + text_rect.h + 6;

                    if (!item.description.empty())
                    {
                        SDL_Color description_color = {210, 220, 240, 255};
                        if (is_disabled)
                        {
                            description_color.r = 170;
                            description_color.g = 178;
                            description_color.b = 196;
                        }
                        SDL_Rect     description_rect;
                        SDL_Texture *description_texture = create_text_texture(
                            renderer, *menu_font, item.description, description_color, description_rect);
                        if (description_texture != ft_nullptr)
                        {
                            description_rect.x = text_left;
                            description_rect.y = text_top;
                            SDL_RenderCopy(&renderer, description_texture, ft_nullptr, &description_rect);
                            SDL_DestroyTexture(description_texture);
                        }
                    }
                }
            }
