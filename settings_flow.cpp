#include "main_menu_system.hpp"

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

#if GALACTIC_HAVE_SDL2
    unsigned char apply_component_levels(unsigned char component, unsigned int brightness_percent,
        unsigned int contrast_percent) noexcept
    {
        long value = static_cast<long>(component);
        long centered = value - 128L;
        long contrasted = (centered * static_cast<long>(contrast_percent) + 50L) / 100L + 128L;
        if (contrasted < 0L)
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
        items.reserve(11U);

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

        ft_rect brightness_rect = base_rect;
        brightness_rect.top += 6 * (base_rect.height + spacing);
        ft_menu_item brightness_item(ft_string("setting:brightness"),
            format_brightness_label(preferences.brightness_percent), brightness_rect);
        items.push_back(brightness_item);

        ft_rect contrast_rect = base_rect;
        contrast_rect.top += 7 * (base_rect.height + spacing);
        ft_menu_item contrast_item(ft_string("setting:contrast"),
            format_contrast_label(preferences.contrast_percent), contrast_rect);
        items.push_back(contrast_item);

        ft_rect anchor_rect = base_rect;
        anchor_rect.top += 8 * (base_rect.height + spacing);

        ft_menu_item anchor_item(ft_string("setting:lore_anchor"), format_lore_anchor_label(preferences.lore_panel_anchor),
            anchor_rect);
        items.push_back(anchor_item);

        ft_rect save_rect = base_rect;
        save_rect.top += 9 * (base_rect.height + spacing);
        ft_menu_item save_item(
            ft_string("action:save"), menu_localize("settings.menu.actions.save", "Save Changes"), save_rect);
        save_item.enabled = allow_save;
        items.push_back(save_item);

        ft_rect cancel_rect = base_rect;
        cancel_rect.top += 10 * (base_rect.height + spacing);
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
        if (item.identifier == "setting:lore_anchor")
        {
            working_preferences.lore_panel_anchor = toggle_lore_anchor(working_preferences.lore_panel_anchor);
            return true;
        }
        return false;
    }
#endif
}

bool run_settings_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    PlayerProfilePreferences &preferences, bool &out_quit_requested)
{
#if GALACTIC_HAVE_SDL2
    out_quit_requested = false;
    if (window == ft_nullptr || renderer == ft_nullptr)
        return false;

    PlayerProfilePreferences baseline = preferences;
    PlayerProfilePreferences working = preferences;

    ft_ui_menu menu;
    rebuild_settings_menu(menu, working, false, ft_nullptr);

    ft_string status_message;
    bool      status_is_error = false;
    bool      running = true;
    bool      saved_changes = false;

    while (running)
    {
        ft_mouse_state    mouse_state;
        ft_keyboard_state keyboard_state;
        bool              activate_requested = false;
        bool              save_requested = false;
        bool              values_changed = false;

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
                else if (event.key.keysym.sym == SDLK_LEFT)
                {
                    if (process_adjustment(menu, working, -1))
                    {
                        status_message.clear();
                        status_is_error = false;
                        values_changed = true;
                    }
                }
                else if (event.key.keysym.sym == SDLK_RIGHT)
                {
                    if (process_adjustment(menu, working, 1))
                    {
                        status_message.clear();
                        status_is_error = false;
                        values_changed = true;
                    }
                }
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

        ft_string previous_identifier;
        const ft_menu_item *selected_item = menu.get_selected_item();
        if (selected_item != ft_nullptr)
            previous_identifier = selected_item->identifier;

        bool allow_save_current = false;
        const ft_vector<ft_menu_item> &current_items = menu.get_items();
        for (size_t index = 0; index < current_items.size(); ++index)
        {
            if (current_items[index].identifier == "action:save")
            {
                allow_save_current = current_items[index].enabled;
                break;
            }
        }

        if (mouse_state.left_released)
        {
            const ft_menu_item *hovered_item = menu.get_hovered_item();
            if (hovered_item != ft_nullptr && hovered_item->enabled)
            {
                if (hovered_item->identifier == "action:save")
                    save_requested = true;
                else if (hovered_item->identifier == "action:cancel")
                {
                    running = false;
                }
                else if (handle_activation(*hovered_item, working))
                {
                    status_message.clear();
                    status_is_error = false;
                    values_changed = true;
                }
            }
        }

        if (activate_requested)
        {
            const ft_menu_item *active_item = menu.get_selected_item();
            if (active_item != ft_nullptr && active_item->enabled)
            {
                if (active_item->identifier == "action:save")
                    save_requested = true;
                else if (active_item->identifier == "action:cancel")
                    running = false;
                else if (handle_activation(*active_item, working))
                {
                    status_message.clear();
                    status_is_error = false;
                    values_changed = true;
                }
            }
        }

        if (save_requested)
        {
            if (preferences.commander_name.empty())
            {
                status_message = menu_localize(
                    "settings.menu.status.no_profile", "No active commander profile to save.");
                status_is_error = true;
            }
            else
            {
                if (save_profile_preferences(window, working))
                {
                    preferences = working;
                    baseline = working;
                    status_message = menu_localize("settings.menu.status.saved", "Settings saved.");
                    status_is_error = false;
                    saved_changes = true;
                }
                else
                {
                    status_message
                        = menu_localize("settings.menu.status.save_failed", "Failed to save settings.");
                    status_is_error = true;
                }
            }
        }

        bool allow_save_now = !preferences_equal(working, baseline);
        if (values_changed || save_requested || allow_save_now != allow_save_current)
            rebuild_settings_menu(menu, working, allow_save_now, &previous_identifier);

        bool has_unsaved_changes = !preferences_equal(working, baseline);
        render_settings_screen(*renderer, menu, title_font, menu_font, status_message, status_is_error, has_unsaved_changes,
            working.colorblind_palette_enabled, working.brightness_percent, working.contrast_percent);

        if (save_requested && !status_is_error)
        {
            running = false;
        }
    }

    return saved_changes;
#else
    (void)window;
    (void)renderer;
    (void)title_font;
    (void)menu_font;
    (void)preferences;
    out_quit_requested = false;
    return false;
#endif
}

namespace settings_flow_testing
{
    unsigned int clamp_ui_scale(unsigned int value) noexcept
    {
        return ::clamp_ui_scale(value);
    }

    unsigned int increment_ui_scale(unsigned int value) noexcept
    {
        return ::increment_ui_scale(value);
    }

    unsigned int decrement_ui_scale(unsigned int value) noexcept
    {
        return ::decrement_ui_scale(value);
    }

    unsigned int clamp_combat_speed(unsigned int value) noexcept
    {
        return ::clamp_combat_speed(value);
    }

    unsigned int increment_combat_speed(unsigned int value) noexcept
    {
        return ::increment_combat_speed(value);
    }

    unsigned int decrement_combat_speed(unsigned int value) noexcept
    {
        return ::decrement_combat_speed(value);
    }

    unsigned int clamp_music_volume(unsigned int value) noexcept
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
}
