    {
        return ::clamp_volume(value);
    }

    unsigned int increment_music_volume(unsigned int value) noexcept
    {
        return ::increment_volume(value);
    }

    unsigned int decrement_music_volume(unsigned int value) noexcept
    {
        return ::decrement_volume(value);
    }

    unsigned int clamp_effects_volume(unsigned int value) noexcept
    {
        return ::clamp_volume(value);
    }

    unsigned int increment_effects_volume(unsigned int value) noexcept
    {
        return ::increment_volume(value);
    }

    unsigned int decrement_effects_volume(unsigned int value) noexcept
    {
        return ::decrement_volume(value);
    }

    unsigned int clamp_brightness(unsigned int value) noexcept
    {
        return ::clamp_brightness(value);
    }

    unsigned int increment_brightness(unsigned int value) noexcept
    {
        return ::increment_brightness(value);
    }

    unsigned int decrement_brightness(unsigned int value) noexcept
    {
        return ::decrement_brightness(value);
    }

    unsigned int clamp_contrast(unsigned int value) noexcept
    {
        return ::clamp_contrast(value);
    }

    unsigned int increment_contrast(unsigned int value) noexcept
    {
        return ::increment_contrast(value);
    }

    unsigned int decrement_contrast(unsigned int value) noexcept
    {
        return ::decrement_contrast(value);
    }

    unsigned int toggle_lore_anchor(unsigned int anchor) noexcept
    {
        return ::toggle_lore_anchor(anchor);
    }

    ft_string format_ui_scale_option(unsigned int value)
    {
        return ::format_ui_scale_label(value);
    }

    ft_string format_combat_speed_option(unsigned int value)
    {
        return ::format_combat_speed_label(value);
    }

    ft_string format_music_volume_option(unsigned int value)
    {
        return ::format_music_volume_label(value);
    }

    ft_string format_effects_volume_option(unsigned int value)
    {
        return ::format_effects_volume_label(value);
    }

    ft_string format_brightness_option(unsigned int value)
    {
        return ::format_brightness_label(value);
    }

    ft_string format_contrast_option(unsigned int value)
    {
        return ::format_contrast_label(value);
    }

    ft_string format_lore_anchor_option(unsigned int anchor)
    {
        return ::format_lore_anchor_label(anchor);
    }

    bool toggle_accessibility_preset(bool enabled) noexcept
    {
        return ::toggle_accessibility_preset(enabled);
    }

    ft_string format_accessibility_preset_option(bool enabled)
    {
        return ::format_accessibility_preset_label(enabled);
    }

    bool toggle_colorblind_palette(bool enabled) noexcept
    {
        return ::toggle_colorblind_palette(enabled);
    }

    ft_string format_colorblind_palette_option(bool enabled)
    {
        return ::format_colorblind_palette_label(enabled);
    }

    bool toggle_experimental_features(bool enabled) noexcept
    {
        return ::toggle_experimental_features(enabled);
    }

    ft_string format_experimental_features_option(bool enabled)
    {
        return ::format_experimental_features_label(enabled);
    }

    bool toggle_analytics_opt_in(bool enabled) noexcept
    {
        return ::toggle_analytics_opt_in(enabled);
    }

    ft_string format_analytics_opt_in_option(bool enabled)
    {
        return ::format_analytics_opt_in_label(enabled);
    }

    ft_string format_controller_up_option(int button)
    {
        return ::format_controller_option_label(
            "settings.menu.controller.up", "Controller Up: {{value}}", button);
    }

    ft_string format_controller_down_option(int button)
    {
        return ::format_controller_option_label(
            "settings.menu.controller.down", "Controller Down: {{value}}", button);
    }

    ft_string format_controller_left_option(int button)
    {
        return ::format_controller_option_label(
            "settings.menu.controller.left", "Controller Left: {{value}}", button);
    }

    ft_string format_controller_right_option(int button)
    {
        return ::format_controller_option_label(
            "settings.menu.controller.right", "Controller Right: {{value}}", button);
    }

    ft_string format_controller_confirm_option(int button)
    {
        return ::format_controller_option_label(
            "settings.menu.controller.confirm", "Controller Confirm: {{value}}", button);
    }

    ft_string format_controller_cancel_option(int button)
    {
        return ::format_controller_option_label(
            "settings.menu.controller.cancel", "Controller Cancel: {{value}}", button);
    }

    ft_string format_controller_delete_option(int button)
    {
        return ::format_controller_option_label(
            "settings.menu.controller.delete", "Controller Delete: {{value}}", button);
    }

    ft_string format_controller_rename_option(int button)
    {
        return ::format_controller_option_label(
            "settings.menu.controller.rename", "Controller Rename: {{value}}", button);
    }

    int increment_controller_button_option(int button) noexcept
    {
        return ::increment_controller_button(button);
    }

    int decrement_controller_button_option(int button) noexcept
    {
        return ::decrement_controller_button(button);
    }
}
