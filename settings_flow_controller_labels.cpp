            case PLAYER_PROFILE_CONTROLLER_BUTTON_LEFTSHOULDER:
                label_key = "settings.menu.controller.label.lb";
                fallback = "LB";
                break;
            case PLAYER_PROFILE_CONTROLLER_BUTTON_RIGHTSHOULDER:
                label_key = "settings.menu.controller.label.rb";
                fallback = "RB";
                break;
            case PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_UP:
                label_key = "settings.menu.controller.label.dpad_up";
                fallback = "D-Pad Up";
                break;
            case PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_DOWN:
                label_key = "settings.menu.controller.label.dpad_down";
                fallback = "D-Pad Down";
                break;
            case PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_LEFT:
                label_key = "settings.menu.controller.label.dpad_left";
                fallback = "D-Pad Left";
                break;
            case PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_RIGHT:
                label_key = "settings.menu.controller.label.dpad_right";
                fallback = "D-Pad Right";
                break;
            default:
                break;
        }

        return menu_localize(label_key, fallback);
    }

    ft_string format_controller_option_label(const char *key, const char *fallback, int button)
    {
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement value_placeholder;
        value_placeholder.key = ft_string("value");
        value_placeholder.value = resolve_controller_button_label(button);
        replacements.push_back(value_placeholder);
        return menu_localize_format(key, fallback, replacements);
    }

#if GALACTIC_HAVE_SDL2
    bool preferences_equal(const PlayerProfilePreferences &lhs, const PlayerProfilePreferences &rhs) noexcept
    {
        if (lhs.commander_name != rhs.commander_name)
            return false;
        if (lhs.ui_scale_percent != rhs.ui_scale_percent)
            return false;
        if (lhs.combat_speed_percent != rhs.combat_speed_percent)
            return false;
        if (lhs.music_volume_percent != rhs.music_volume_percent)
            return false;
        if (lhs.effects_volume_percent != rhs.effects_volume_percent)
            return false;
        if (lhs.brightness_percent != rhs.brightness_percent)
            return false;
        if (lhs.contrast_percent != rhs.contrast_percent)
            return false;
        if (lhs.lore_panel_anchor != rhs.lore_panel_anchor)
            return false;
        if (lhs.window_width != rhs.window_width)
            return false;
        if (lhs.window_height != rhs.window_height)
            return false;
        if (lhs.menu_tutorial_seen != rhs.menu_tutorial_seen)
            return false;
        if (lhs.colorblind_palette_enabled != rhs.colorblind_palette_enabled)
            return false;
        if (lhs.accessibility_preset_enabled != rhs.accessibility_preset_enabled)
            return false;
        if (lhs.experimental_features_enabled != rhs.experimental_features_enabled)
            return false;
        if (lhs.analytics_opt_in != rhs.analytics_opt_in)
            return false;
        if (lhs.last_menu_input_device != rhs.last_menu_input_device)
            return false;
        return true;
    }

    void rebuild_settings_menu(ft_ui_menu &menu, const PlayerProfilePreferences &preferences, bool allow_save,
        const ft_string *preferred_identifier)
    {
        const ft_rect base_rect(460, 220, 360, 56);
        const int      spacing = 18;

        ft_vector<ft_menu_item> items;
        items.reserve(13U);

        ft_menu_item ui_scale_item(ft_string("setting:ui_scale"), format_ui_scale_label(preferences.ui_scale_percent), base_rect);
        items.push_back(ui_scale_item);

        ft_rect combat_rect = base_rect;
        combat_rect.top += base_rect.height + spacing;
        ft_menu_item combat_item(
            ft_string("setting:combat_speed"), format_combat_speed_label(preferences.combat_speed_percent), combat_rect);
        items.push_back(combat_item);

        ft_rect music_rect = base_rect;
        music_rect.top += 2 * (base_rect.height + spacing);
        ft_menu_item music_item(
            ft_string("setting:music_volume"), format_music_volume_label(preferences.music_volume_percent), music_rect);
        items.push_back(music_item);

        ft_rect effects_rect = base_rect;
        effects_rect.top += 3 * (base_rect.height + spacing);
        ft_menu_item effects_item(
            ft_string("setting:effects_volume"), format_effects_volume_label(preferences.effects_volume_percent), effects_rect);
        items.push_back(effects_item);

        ft_rect accessibility_rect = base_rect;
        accessibility_rect.top += 4 * (base_rect.height + spacing);
        ft_menu_item accessibility_item(ft_string("setting:accessibility_preset"),
            format_accessibility_preset_label(preferences.accessibility_preset_enabled), accessibility_rect);
        items.push_back(accessibility_item);

        ft_rect colorblind_rect = base_rect;
        colorblind_rect.top += 5 * (base_rect.height + spacing);
        ft_menu_item colorblind_item(ft_string("setting:colorblind_palette"),
            format_colorblind_palette_label(preferences.colorblind_palette_enabled), colorblind_rect);
        items.push_back(colorblind_item);

        ft_rect experiments_rect = base_rect;
        experiments_rect.top += 6 * (base_rect.height + spacing);
        ft_menu_item experiments_item(ft_string("setting:experimental_features"),
            format_experimental_features_label(preferences.experimental_features_enabled), experiments_rect);
        items.push_back(experiments_item);

        ft_rect analytics_rect = base_rect;
        analytics_rect.top += 7 * (base_rect.height + spacing);
        ft_menu_item analytics_item(ft_string("setting:analytics_opt_in"),
            format_analytics_opt_in_label(preferences.analytics_opt_in), analytics_rect);
        items.push_back(analytics_item);

        ft_rect brightness_rect = base_rect;
        brightness_rect.top += 8 * (base_rect.height + spacing);
        ft_menu_item brightness_item(ft_string("setting:brightness"),
            format_brightness_label(preferences.brightness_percent), brightness_rect);
        items.push_back(brightness_item);

        ft_rect contrast_rect = base_rect;
        contrast_rect.top += 9 * (base_rect.height + spacing);
        ft_menu_item contrast_item(ft_string("setting:contrast"),
            format_contrast_label(preferences.contrast_percent), contrast_rect);
        items.push_back(contrast_item);

        ft_rect anchor_rect = base_rect;
        anchor_rect.top += 10 * (base_rect.height + spacing);

        ft_menu_item anchor_item(ft_string("setting:lore_anchor"), format_lore_anchor_label(preferences.lore_panel_anchor),
            anchor_rect);
        items.push_back(anchor_item);

        ft_rect save_rect = base_rect;
        save_rect.top += 11 * (base_rect.height + spacing);
        ft_menu_item save_item(
            ft_string("action:save"), menu_localize("settings.menu.actions.save", "Save Changes"), save_rect);
        save_item.enabled = allow_save;
        items.push_back(save_item);

        ft_rect cancel_rect = base_rect;
        cancel_rect.top += 12 * (base_rect.height + spacing);
        ft_menu_item cancel_item(
            ft_string("action:cancel"), menu_localize("settings.menu.actions.cancel", "Cancel"), cancel_rect);
        items.push_back(cancel_item);

        menu.set_items(items);

        ft_rect viewport = base_rect;
        const int window_height = static_cast<int>(app_constants::kWindowHeight);
        const int reserved_bottom = 220;
        viewport.height = window_height - base_rect.top - reserved_bottom;
        if (viewport.height < base_rect.height)
            viewport.height = base_rect.height;
        menu.set_viewport_bounds(viewport);

        if (preferred_identifier != ft_nullptr)
        {
            const ft_vector<ft_menu_item> &current_items = menu.get_items();
            for (size_t index = 0; index < current_items.size(); ++index)
            {
                if (current_items[index].identifier == *preferred_identifier && current_items[index].enabled)
                {
                    menu.set_selected_index(static_cast<int>(index));
                    break;
                }
            }
        }
    }

    void render_settings_screen(SDL_Renderer &renderer, const ft_ui_menu &menu, TTF_Font *title_font, TTF_Font *menu_font,
        const ft_string &status_message, bool status_is_error, bool has_unsaved_changes, bool use_colorblind_palette,
        unsigned int brightness_percent, unsigned int contrast_percent)
    {
        SettingsPalette palette = resolve_settings_palette(use_colorblind_palette, brightness_percent, contrast_percent);

        SDL_SetRenderDrawColor(
            &renderer, palette.background.r, palette.background.g, palette.background.b, palette.background.a);
        SDL_RenderClear(&renderer);

        int output_width = static_cast<int>(app_constants::kWindowWidth);
        int output_height = static_cast<int>(app_constants::kWindowHeight);
        SDL_GetRendererOutputSize(&renderer, &output_width, &output_height);

        if (title_font != ft_nullptr)
        {
            SDL_Color title_color = palette.title;
            SDL_Rect  title_rect;
            ft_string title_text = menu_localize("settings.menu.title", "Commander Settings");
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

        if (menu_font != ft_nullptr)
        {
            SDL_Color info_color = palette.info;
            SDL_Rect  info_rect;
            ft_string info_text = menu_localize("settings.menu.instructions",
                "Use Left/Right or Enter to adjust an option. Esc cancels. Save commits changes.");
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
