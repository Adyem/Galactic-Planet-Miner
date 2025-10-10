            contrasted = 0L;
        else if (contrasted > 255L)
            contrasted = 255L;

        long brightened = (contrasted * static_cast<long>(brightness_percent) + 50L) / 100L;
        if (brightened < 0L)
            brightened = 0L;
        else if (brightened > 255L)
            brightened = 255L;

        return static_cast<unsigned char>(brightened);
    }

    void apply_levels(SDL_Color &color, unsigned int brightness_percent, unsigned int contrast_percent) noexcept
    {
        color.r = apply_component_levels(color.r, brightness_percent, contrast_percent);
        color.g = apply_component_levels(color.g, brightness_percent, contrast_percent);
        color.b = apply_component_levels(color.b, brightness_percent, contrast_percent);
    }

    struct SettingsPalette
    {
        SDL_Color background;
        SDL_Color title;
        SDL_Color info;
        SDL_Color button_idle;
        SDL_Color button_hover;
        SDL_Color button_selected;
        SDL_Color button_disabled;
        SDL_Color button_disabled_hover;
        SDL_Color border_enabled;
        SDL_Color border_disabled;
        SDL_Color text_enabled;
        SDL_Color text_disabled;
        SDL_Color unsaved;
        SDL_Color status_ok;
        SDL_Color status_error;

        SettingsPalette() noexcept
            : background(), title(), info(), button_idle(), button_hover(), button_selected(), button_disabled(),
              button_disabled_hover(), border_enabled(), border_disabled(), text_enabled(), text_disabled(), unsaved(),
              status_ok(), status_error()
        {}
    };

    void apply_levels(SettingsPalette &palette, unsigned int brightness_percent, unsigned int contrast_percent) noexcept
    {
        apply_levels(palette.background, brightness_percent, contrast_percent);
        apply_levels(palette.title, brightness_percent, contrast_percent);
        apply_levels(palette.info, brightness_percent, contrast_percent);
        apply_levels(palette.button_idle, brightness_percent, contrast_percent);
        apply_levels(palette.button_hover, brightness_percent, contrast_percent);
        apply_levels(palette.button_selected, brightness_percent, contrast_percent);
        apply_levels(palette.button_disabled, brightness_percent, contrast_percent);
        apply_levels(palette.button_disabled_hover, brightness_percent, contrast_percent);
        apply_levels(palette.border_enabled, brightness_percent, contrast_percent);
        apply_levels(palette.border_disabled, brightness_percent, contrast_percent);
        apply_levels(palette.text_enabled, brightness_percent, contrast_percent);
        apply_levels(palette.text_disabled, brightness_percent, contrast_percent);
        apply_levels(palette.unsaved, brightness_percent, contrast_percent);
        apply_levels(palette.status_ok, brightness_percent, contrast_percent);
        apply_levels(palette.status_error, brightness_percent, contrast_percent);
    }

    SettingsPalette resolve_settings_palette(
        bool colorblind_enabled, unsigned int brightness_percent, unsigned int contrast_percent) noexcept
    {
        SettingsPalette palette;
        palette.background = {12, 16, 28, 255};
        palette.title = {220, 220, 245, 255};
        palette.info = {170, 180, 210, 255};
        palette.button_idle = {28, 36, 60, 255};
        palette.button_hover = {56, 84, 140, 255};
        palette.button_selected = {40, 64, 112, 255};
        palette.button_disabled = {30, 34, 44, 255};
        palette.button_disabled_hover = {42, 46, 60, 255};
        palette.border_enabled = {90, 110, 160, 255};
        palette.border_disabled = {70, 80, 120, 255};
        palette.text_enabled = {255, 255, 255, 255};
        palette.text_disabled = {188, 196, 210, 255};
        palette.unsaved = {210, 200, 120, 255};
        palette.status_ok = {180, 200, 220, 255};
        palette.status_error = {220, 90, 90, 255};

        if (colorblind_enabled)
        {
            palette.background = {10, 20, 26, 255};
            palette.title = {238, 246, 255, 255};
            palette.info = {198, 210, 232, 255};
            palette.button_idle = {44, 62, 88, 255};
            palette.button_hover = {96, 132, 196, 255};
            palette.button_selected = {72, 112, 176, 255};
            palette.button_disabled = {48, 54, 70, 255};
            palette.button_disabled_hover = {70, 86, 112, 255};
            palette.border_enabled = {130, 164, 220, 255};
            palette.border_disabled = {96, 112, 156, 255};
            palette.text_enabled = {255, 255, 255, 255};
            palette.text_disabled = {210, 216, 230, 255};
            palette.unsaved = {236, 196, 96, 255};
            palette.status_ok = {96, 196, 196, 255};
            palette.status_error = {232, 148, 88, 255};
        }

        brightness_percent = clamp_brightness(brightness_percent);
        contrast_percent = clamp_contrast(contrast_percent);
        apply_levels(palette, brightness_percent, contrast_percent);

        return palette;
    }
#endif

    ft_string format_ui_scale_label(unsigned int value)
    {
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement value_placeholder;
        value_placeholder.key = ft_string("value");
        value_placeholder.value = ft_to_string(static_cast<int>(value));
        replacements.push_back(value_placeholder);
        return menu_localize_format("settings.menu.ui_scale", "UI Scale: {{value}}%", replacements);
    }

    ft_string format_combat_speed_label(unsigned int value)
    {
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement value_placeholder;
        value_placeholder.key = ft_string("value");
        value_placeholder.value = ft_to_string(static_cast<int>(value));
        replacements.push_back(value_placeholder);
        return menu_localize_format("settings.menu.combat_speed", "Combat Speed: {{value}}%", replacements);
    }

    ft_string format_music_volume_label(unsigned int value)
    {
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement value_placeholder;
        value_placeholder.key = ft_string("value");
        value_placeholder.value = ft_to_string(static_cast<int>(value));
        replacements.push_back(value_placeholder);
        return menu_localize_format("settings.menu.music_volume", "Music Volume: {{value}}%", replacements);
    }

    ft_string format_effects_volume_label(unsigned int value)
    {
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement value_placeholder;
        value_placeholder.key = ft_string("value");
        value_placeholder.value = ft_to_string(static_cast<int>(value));
        replacements.push_back(value_placeholder);
        return menu_localize_format("settings.menu.effects_volume", "Effects Volume: {{value}}%", replacements);
    }

    ft_string format_brightness_label(unsigned int value)
    {
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement value_placeholder;
        value_placeholder.key = ft_string("value");
        value_placeholder.value = ft_to_string(static_cast<int>(value));
        replacements.push_back(value_placeholder);
        return menu_localize_format("settings.menu.brightness", "Brightness: {{value}}%", replacements);
    }

    ft_string format_contrast_label(unsigned int value)
    {
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement value_placeholder;
        value_placeholder.key = ft_string("value");
        value_placeholder.value = ft_to_string(static_cast<int>(value));
        replacements.push_back(value_placeholder);
        return menu_localize_format("settings.menu.contrast", "Contrast: {{value}}%", replacements);
    }

    ft_string format_lore_anchor_label(unsigned int anchor)
    {
        const char *position_key = anchor == PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT
            ? "settings.menu.anchor.left"
            : "settings.menu.anchor.right";
        const char *fallback = anchor == PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT ? "Left" : "Right";
        ft_string anchor_label = menu_localize(position_key, fallback);
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement value_placeholder;
        value_placeholder.key = ft_string("value");
        value_placeholder.value = anchor_label;
        replacements.push_back(value_placeholder);
        return menu_localize_format(
            "settings.menu.lore_anchor", "Lore Panel Anchor: {{value}}", replacements);
    }

    int sanitize_controller_button(int button) noexcept
    {
        if (button < PLAYER_PROFILE_CONTROLLER_BUTTON_A || button > PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_RIGHT)
            return PLAYER_PROFILE_CONTROLLER_BUTTON_A;
        return button;
    }

    int increment_controller_button(int button) noexcept
    {
        button = sanitize_controller_button(button);
        if (button >= PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_RIGHT)
            return PLAYER_PROFILE_CONTROLLER_BUTTON_A;
        return button + 1;
    }

    int decrement_controller_button(int button) noexcept
    {
        button = sanitize_controller_button(button);
        if (button <= PLAYER_PROFILE_CONTROLLER_BUTTON_A)
            return PLAYER_PROFILE_CONTROLLER_BUTTON_DPAD_RIGHT;
        return button - 1;
    }

    ft_string resolve_controller_button_label(int button)
    {
        button = sanitize_controller_button(button);

        const char *label_key = "settings.menu.controller.label.unknown";
        const char *fallback = "Button ?";

        switch (button)
        {
            case PLAYER_PROFILE_CONTROLLER_BUTTON_A:
                label_key = "settings.menu.controller.label.a";
                fallback = "A";
                break;
            case PLAYER_PROFILE_CONTROLLER_BUTTON_B:
                label_key = "settings.menu.controller.label.b";
                fallback = "B";
                break;
            case PLAYER_PROFILE_CONTROLLER_BUTTON_X:
                label_key = "settings.menu.controller.label.x";
                fallback = "X";
                break;
            case PLAYER_PROFILE_CONTROLLER_BUTTON_Y:
                label_key = "settings.menu.controller.label.y";
                fallback = "Y";
                break;
            case PLAYER_PROFILE_CONTROLLER_BUTTON_START:
                label_key = "settings.menu.controller.label.start";
                fallback = "Start";
                break;
            case PLAYER_PROFILE_CONTROLLER_BUTTON_BACK:
                label_key = "settings.menu.controller.label.back";
                fallback = "Back";
                break;
