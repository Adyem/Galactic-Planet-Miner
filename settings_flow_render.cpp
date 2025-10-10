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

            SDL_Color fill_color = palette.button_idle;
            if (is_disabled)
            {
                fill_color = is_hovered ? palette.button_disabled_hover : palette.button_disabled;
            }
            else if (is_hovered)
            {
                fill_color = palette.button_hover;
            }
            else if (is_selected)
            {
                fill_color = palette.button_selected;
            }

            SDL_Rect button_rect;
            button_rect.x = item.bounds.left;
            button_rect.y = item.bounds.top - scroll_offset;
            button_rect.w = item.bounds.width;
            button_rect.h = item.bounds.height;

            if (clip_enabled && (button_rect.y + button_rect.h <= clip_rect.y || button_rect.y >= clip_bottom))
                continue;

            SDL_SetRenderDrawColor(&renderer, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
            SDL_RenderFillRect(&renderer, &button_rect);

            const SDL_Color border_color = is_disabled ? palette.border_disabled : palette.border_enabled;
            SDL_SetRenderDrawColor(&renderer, border_color.r, border_color.g, border_color.b, border_color.a);
            SDL_RenderDrawRect(&renderer, &button_rect);

            if (menu_font != ft_nullptr)
            {
                SDL_Color text_color = is_disabled ? palette.text_disabled : palette.text_enabled;
                SDL_Rect  text_rect;
                SDL_Texture *text_texture = create_text_texture(renderer, *menu_font, item.label, text_color, text_rect);
                if (text_texture != ft_nullptr)
                {
                    text_rect.x = item.bounds.left + 18;
                    text_rect.y = button_rect.y + (button_rect.h - text_rect.h) / 2;
                    SDL_RenderCopy(&renderer, text_texture, ft_nullptr, &text_rect);
                    SDL_DestroyTexture(text_texture);
                }
            }
        }

        if (clip_enabled)
            SDL_RenderSetClipRect(&renderer, ft_nullptr);

        if (menu_font != ft_nullptr && has_unsaved_changes)
        {
            SDL_Color dirty_color = palette.unsaved;
            SDL_Rect  dirty_rect;
            ft_string dirty_text = menu_localize("settings.menu.unsaved", "Unsaved changes");
            SDL_Texture *dirty_texture
                = create_text_texture(renderer, *menu_font, dirty_text, dirty_color, dirty_rect);
            if (dirty_texture != ft_nullptr)
            {
                dirty_rect.x = 80;
                dirty_rect.y = output_height - dirty_rect.h - 140;
                SDL_RenderCopy(&renderer, dirty_texture, ft_nullptr, &dirty_rect);
                SDL_DestroyTexture(dirty_texture);
            }
        }

        if (menu_font != ft_nullptr && !status_message.empty())
        {
            SDL_Color status_color = status_is_error ? palette.status_error : palette.status_ok;

            SDL_Rect status_rect;
            SDL_Texture *status_texture
                = create_text_texture(renderer, *menu_font, status_message, status_color, status_rect);
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

    bool process_adjustment(ft_ui_menu &menu, PlayerProfilePreferences &working_preferences, int direction)
    {
        const ft_menu_item *selected_item = menu.get_selected_item();
        if (selected_item == ft_nullptr)
            return false;

        bool adjusted = false;
        if (selected_item->identifier == "setting:ui_scale")
        {
            if (direction > 0)
                working_preferences.ui_scale_percent = increment_ui_scale(working_preferences.ui_scale_percent);
            else if (direction < 0)
                working_preferences.ui_scale_percent = decrement_ui_scale(working_preferences.ui_scale_percent);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:combat_speed")
        {
            if (direction > 0)
                working_preferences.combat_speed_percent = increment_combat_speed(working_preferences.combat_speed_percent);
            else if (direction < 0)
                working_preferences.combat_speed_percent = decrement_combat_speed(working_preferences.combat_speed_percent);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:music_volume")
        {
            if (direction > 0)
                working_preferences.music_volume_percent = increment_volume(working_preferences.music_volume_percent);
            else if (direction < 0)
                working_preferences.music_volume_percent = decrement_volume(working_preferences.music_volume_percent);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:effects_volume")
        {
            if (direction > 0)
                working_preferences.effects_volume_percent = increment_volume(working_preferences.effects_volume_percent);
            else if (direction < 0)
                working_preferences.effects_volume_percent = decrement_volume(working_preferences.effects_volume_percent);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:brightness")
        {
            if (direction > 0)
                working_preferences.brightness_percent = increment_brightness(working_preferences.brightness_percent);
            else if (direction < 0)
                working_preferences.brightness_percent = decrement_brightness(working_preferences.brightness_percent);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:contrast")
        {
            if (direction > 0)
                working_preferences.contrast_percent = increment_contrast(working_preferences.contrast_percent);
            else if (direction < 0)
                working_preferences.contrast_percent = decrement_contrast(working_preferences.contrast_percent);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:accessibility_preset")
        {
            working_preferences.accessibility_preset_enabled
                = toggle_accessibility_preset(working_preferences.accessibility_preset_enabled);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:colorblind_palette")
        {
            working_preferences.colorblind_palette_enabled
                = toggle_colorblind_palette(working_preferences.colorblind_palette_enabled);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:experimental_features")
        {
            working_preferences.experimental_features_enabled
                = toggle_experimental_features(working_preferences.experimental_features_enabled);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:analytics_opt_in")
        {
            working_preferences.analytics_opt_in
                = toggle_analytics_opt_in(working_preferences.analytics_opt_in);
            adjusted = true;
        }
        else if (selected_item->identifier == "setting:lore_anchor")
        {
            working_preferences.lore_panel_anchor = toggle_lore_anchor(working_preferences.lore_panel_anchor);
            adjusted = true;
        }

        return adjusted;
    }

    bool handle_activation(const ft_menu_item &item, PlayerProfilePreferences &working_preferences)
    {
        if (item.identifier == "setting:ui_scale")
        {
            working_preferences.ui_scale_percent = increment_ui_scale(working_preferences.ui_scale_percent);
            return true;
        }
        if (item.identifier == "setting:combat_speed")
        {
            working_preferences.combat_speed_percent = increment_combat_speed(working_preferences.combat_speed_percent);
            return true;
        }
        if (item.identifier == "setting:music_volume")
        {
            working_preferences.music_volume_percent = increment_volume(working_preferences.music_volume_percent);
            return true;
        }
        if (item.identifier == "setting:effects_volume")
        {
            working_preferences.effects_volume_percent = increment_volume(working_preferences.effects_volume_percent);
            return true;
        }
        if (item.identifier == "setting:brightness")
        {
            working_preferences.brightness_percent = increment_brightness(working_preferences.brightness_percent);
            return true;
        }
        if (item.identifier == "setting:contrast")
        {
            working_preferences.contrast_percent = increment_contrast(working_preferences.contrast_percent);
            return true;
        }
        if (item.identifier == "setting:accessibility_preset")
        {
            working_preferences.accessibility_preset_enabled
                = toggle_accessibility_preset(working_preferences.accessibility_preset_enabled);
            return true;
        }
        if (item.identifier == "setting:colorblind_palette")
        {
            working_preferences.colorblind_palette_enabled
                = toggle_colorblind_palette(working_preferences.colorblind_palette_enabled);
            return true;
        }
        if (item.identifier == "setting:experimental_features")
        {
            working_preferences.experimental_features_enabled
                = toggle_experimental_features(working_preferences.experimental_features_enabled);
            return true;
        }
        if (item.identifier == "setting:analytics_opt_in")
        {
            working_preferences.analytics_opt_in
                = toggle_analytics_opt_in(working_preferences.analytics_opt_in);
            return true;
        }
        if (item.identifier == "setting:lore_anchor")
        {
            working_preferences.lore_panel_anchor = toggle_lore_anchor(working_preferences.lore_panel_anchor);
            return true;
        }
