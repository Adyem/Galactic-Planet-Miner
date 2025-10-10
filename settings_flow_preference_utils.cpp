#include "app_constants.hpp"
#include "menu_localization.hpp"
#include "player_profile.hpp"
#include "ui_input.hpp"

#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/Libft/libft.hpp"

namespace
{
    const unsigned int kUiScaleStep = 5U;
    const unsigned int kCombatSpeedStep = 5U;
    const unsigned int kVolumeStep = 5U;
    const unsigned int kBrightnessStep = 5U;
    const unsigned int kContrastStep = 5U;

    unsigned int clamp_ui_scale(unsigned int value) noexcept
    {
        if (value < PLAYER_PROFILE_UI_SCALE_MIN_PERCENT)
            return PLAYER_PROFILE_UI_SCALE_MIN_PERCENT;
        if (value > PLAYER_PROFILE_UI_SCALE_MAX_PERCENT)
            return PLAYER_PROFILE_UI_SCALE_MAX_PERCENT;
        return value;
    }

    unsigned int clamp_combat_speed(unsigned int value) noexcept
    {
        if (value < PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT)
            return PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT;
        if (value > PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT)
            return PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT;
        return value;
    }

    unsigned int clamp_volume(unsigned int value) noexcept
    {
        if (value < PLAYER_PROFILE_VOLUME_MIN_PERCENT)
            return PLAYER_PROFILE_VOLUME_MIN_PERCENT;
        if (value > PLAYER_PROFILE_VOLUME_MAX_PERCENT)
            return PLAYER_PROFILE_VOLUME_MAX_PERCENT;
        return value;
    }

    unsigned int clamp_brightness(unsigned int value) noexcept
    {
        if (value < PLAYER_PROFILE_BRIGHTNESS_MIN_PERCENT)
            return PLAYER_PROFILE_BRIGHTNESS_MIN_PERCENT;
        if (value > PLAYER_PROFILE_BRIGHTNESS_MAX_PERCENT)
            return PLAYER_PROFILE_BRIGHTNESS_MAX_PERCENT;
        return value;
    }

    unsigned int clamp_contrast(unsigned int value) noexcept
    {
        if (value < PLAYER_PROFILE_CONTRAST_MIN_PERCENT)
            return PLAYER_PROFILE_CONTRAST_MIN_PERCENT;
        if (value > PLAYER_PROFILE_CONTRAST_MAX_PERCENT)
            return PLAYER_PROFILE_CONTRAST_MAX_PERCENT;
        return value;
    }

    unsigned int increment_ui_scale(unsigned int value) noexcept
    {
        if (value >= PLAYER_PROFILE_UI_SCALE_MAX_PERCENT)
            return PLAYER_PROFILE_UI_SCALE_MAX_PERCENT;
        unsigned int candidate = value + kUiScaleStep;
        if (candidate < value)
            return PLAYER_PROFILE_UI_SCALE_MAX_PERCENT;
        return clamp_ui_scale(candidate);
    }

    unsigned int decrement_ui_scale(unsigned int value) noexcept
    {
        if (value <= PLAYER_PROFILE_UI_SCALE_MIN_PERCENT)
            return PLAYER_PROFILE_UI_SCALE_MIN_PERCENT;
        if (value <= kUiScaleStep)
            return PLAYER_PROFILE_UI_SCALE_MIN_PERCENT;
        unsigned int candidate = value - kUiScaleStep;
        if (candidate > value)
            return PLAYER_PROFILE_UI_SCALE_MIN_PERCENT;
        return clamp_ui_scale(candidate);
    }

    unsigned int increment_combat_speed(unsigned int value) noexcept
    {
        if (value >= PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT)
            return PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT;
        unsigned int candidate = value + kCombatSpeedStep;
        if (candidate < value)
            return PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT;
        return clamp_combat_speed(candidate);
    }

    unsigned int decrement_combat_speed(unsigned int value) noexcept
    {
        if (value <= PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT)
            return PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT;
        if (value <= kCombatSpeedStep)
            return PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT;
        unsigned int candidate = value - kCombatSpeedStep;
        if (candidate > value)
            return PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT;
        return clamp_combat_speed(candidate);
    }

    unsigned int increment_volume(unsigned int value) noexcept
    {
        if (value >= PLAYER_PROFILE_VOLUME_MAX_PERCENT)
            return PLAYER_PROFILE_VOLUME_MAX_PERCENT;
        unsigned int candidate = value + kVolumeStep;
        if (candidate < value)
            return PLAYER_PROFILE_VOLUME_MAX_PERCENT;
        return clamp_volume(candidate);
    }

    unsigned int decrement_volume(unsigned int value) noexcept
    {
        if (value <= PLAYER_PROFILE_VOLUME_MIN_PERCENT)
            return PLAYER_PROFILE_VOLUME_MIN_PERCENT;
        if (value <= kVolumeStep)
            return PLAYER_PROFILE_VOLUME_MIN_PERCENT;
        unsigned int candidate = value - kVolumeStep;
        if (candidate > value)
            return PLAYER_PROFILE_VOLUME_MIN_PERCENT;
        return clamp_volume(candidate);
    }

    unsigned int increment_brightness(unsigned int value) noexcept
    {
        if (value >= PLAYER_PROFILE_BRIGHTNESS_MAX_PERCENT)
            return PLAYER_PROFILE_BRIGHTNESS_MAX_PERCENT;
        unsigned int candidate = value + kBrightnessStep;
        if (candidate < value)
            return PLAYER_PROFILE_BRIGHTNESS_MAX_PERCENT;
        return clamp_brightness(candidate);
    }

    unsigned int decrement_brightness(unsigned int value) noexcept
    {
        if (value <= PLAYER_PROFILE_BRIGHTNESS_MIN_PERCENT)
            return PLAYER_PROFILE_BRIGHTNESS_MIN_PERCENT;
        if (value <= kBrightnessStep)
            return PLAYER_PROFILE_BRIGHTNESS_MIN_PERCENT;
        unsigned int candidate = value - kBrightnessStep;
        if (candidate > value)
            return PLAYER_PROFILE_BRIGHTNESS_MIN_PERCENT;
        return clamp_brightness(candidate);
    }

    unsigned int increment_contrast(unsigned int value) noexcept
    {
        if (value >= PLAYER_PROFILE_CONTRAST_MAX_PERCENT)
            return PLAYER_PROFILE_CONTRAST_MAX_PERCENT;
        unsigned int candidate = value + kContrastStep;
        if (candidate < value)
            return PLAYER_PROFILE_CONTRAST_MAX_PERCENT;
        return clamp_contrast(candidate);
    }

    unsigned int decrement_contrast(unsigned int value) noexcept
    {
        if (value <= PLAYER_PROFILE_CONTRAST_MIN_PERCENT)
            return PLAYER_PROFILE_CONTRAST_MIN_PERCENT;
        if (value <= kContrastStep)
            return PLAYER_PROFILE_CONTRAST_MIN_PERCENT;
        unsigned int candidate = value - kContrastStep;
        if (candidate > value)
            return PLAYER_PROFILE_CONTRAST_MIN_PERCENT;
        return clamp_contrast(candidate);
    }

    unsigned int toggle_lore_anchor(unsigned int anchor) noexcept
    {
        if (anchor == PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT)
            return PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT;
        return PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT;
    }

    bool toggle_accessibility_preset(bool enabled) noexcept
    {
        return !enabled;
    }

    ft_string format_accessibility_preset_label(bool enabled)
    {
        ft_string prefix
            = menu_localize("settings.menu.accessibility.label", "Accessibility Preset: ");
        const char *state_key = enabled ? "settings.menu.toggle.on" : "settings.menu.toggle.off";
        const char *fallback = enabled ? "On" : "Off";
        ft_string state = menu_localize(state_key, fallback);
        prefix.append(state);
        return prefix;
    }

    bool toggle_colorblind_palette(bool enabled) noexcept
    {
        return !enabled;
    }

    ft_string format_colorblind_palette_label(bool enabled)
    {
        ft_string prefix
            = menu_localize("settings.menu.colorblind.label", "Colorblind Palette: ");
        const char *state_key = enabled ? "settings.menu.toggle.on" : "settings.menu.toggle.off";
        const char *fallback = enabled ? "On" : "Off";
        ft_string state = menu_localize(state_key, fallback);
        prefix.append(state);
        return prefix;
    }

    bool toggle_experimental_features(bool enabled) noexcept
    {
        return !enabled;
    }

    ft_string format_experimental_features_label(bool enabled)
    {
        ft_string prefix
            = menu_localize("settings.menu.experimental.label", "Experimental Features: ");
        const char *state_key = enabled ? "settings.menu.toggle.on" : "settings.menu.toggle.off";
        const char *fallback = enabled ? "On" : "Off";
        ft_string state = menu_localize(state_key, fallback);
        prefix.append(state);
        return prefix;
    }

    bool toggle_analytics_opt_in(bool enabled) noexcept
    {
        return !enabled;
    }

    ft_string format_analytics_opt_in_label(bool enabled)
    {
        ft_string prefix
            = menu_localize("settings.menu.analytics.label", "Analytics Sharing: ");
        const char *state_key = enabled ? "settings.menu.toggle.on" : "settings.menu.toggle.off";
        const char *fallback = enabled ? "On" : "Off";
        ft_string state = menu_localize(state_key, fallback);
        prefix.append(state);
        return prefix;
    }

#if GALACTIC_HAVE_SDL2
    unsigned char apply_component_levels(unsigned char component, unsigned int brightness_percent,
        unsigned int contrast_percent) noexcept
    {
        long value = static_cast<long>(component);
        long centered = value - 128L;
        long contrasted = (centered * static_cast<long>(contrast_percent) + 50L) / 100L + 128L;
        if (contrasted < 0L)
