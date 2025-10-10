#include "main_menu_system.hpp"

#include "app_constants.hpp"

#if GALACTIC_HAVE_SDL2
namespace
{
    unsigned int clamp_percent(unsigned int value, unsigned int min_value, unsigned int max_value) noexcept
    {
        if (value < min_value)
            return min_value;
        if (value > max_value)
            return max_value;
        return value;
    }

    unsigned char apply_component_levels(
        unsigned char component, unsigned int brightness_percent, unsigned int contrast_percent) noexcept
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

    void apply_levels(
        MainMenuPalette &palette, unsigned int brightness_percent, unsigned int contrast_percent) noexcept
    {
        apply_levels(palette.background, brightness_percent, contrast_percent);
        apply_levels(palette.title, brightness_percent, contrast_percent);
        apply_levels(palette.profile, brightness_percent, contrast_percent);
        apply_levels(palette.button_idle, brightness_percent, contrast_percent);
        apply_levels(palette.button_hover, brightness_percent, contrast_percent);
        apply_levels(palette.button_selected, brightness_percent, contrast_percent);
        apply_levels(palette.button_disabled, brightness_percent, contrast_percent);
        apply_levels(palette.button_disabled_hover, brightness_percent, contrast_percent);
        apply_levels(palette.button_border_enabled, brightness_percent, contrast_percent);
        apply_levels(palette.button_border_disabled, brightness_percent, contrast_percent);
        apply_levels(palette.button_text_enabled, brightness_percent, contrast_percent);
        apply_levels(palette.button_text_disabled, brightness_percent, contrast_percent);
        apply_levels(palette.description, brightness_percent, contrast_percent);
        apply_levels(palette.hint, brightness_percent, contrast_percent);
        apply_levels(palette.build, brightness_percent, contrast_percent);
        apply_levels(palette.alert_text_error, brightness_percent, contrast_percent);
        apply_levels(palette.alert_text_info, brightness_percent, contrast_percent);
        apply_levels(palette.alert_background_error, brightness_percent, contrast_percent);
        apply_levels(palette.alert_border_error, brightness_percent, contrast_percent);
        apply_levels(palette.alert_background_info, brightness_percent, contrast_percent);
        apply_levels(palette.alert_border_info, brightness_percent, contrast_percent);
        apply_levels(palette.overlay_heading, brightness_percent, contrast_percent);
        apply_levels(palette.overlay_line, brightness_percent, contrast_percent);
        apply_levels(palette.overlay_footer, brightness_percent, contrast_percent);
        apply_levels(palette.overlay_background, brightness_percent, contrast_percent);
        apply_levels(palette.overlay_border, brightness_percent, contrast_percent);
        apply_levels(palette.tutorial_title, brightness_percent, contrast_percent);
        apply_levels(palette.tutorial_primary, brightness_percent, contrast_percent);
        apply_levels(palette.tutorial_secondary, brightness_percent, contrast_percent);
        apply_levels(palette.tutorial_background, brightness_percent, contrast_percent);
        apply_levels(palette.tutorial_border, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_icon_idle, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_icon_in_progress, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_icon_success, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_icon_failure, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_text_in_progress, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_text_success, brightness_percent, contrast_percent);
        apply_levels(palette.autosave_text_failure, brightness_percent, contrast_percent);
        apply_levels(palette.sync_heading, brightness_percent, contrast_percent);
        apply_levels(palette.sync_idle, brightness_percent, contrast_percent);
        apply_levels(palette.sync_pending, brightness_percent, contrast_percent);
        apply_levels(palette.sync_success, brightness_percent, contrast_percent);
        apply_levels(palette.sync_failure, brightness_percent, contrast_percent);
        apply_levels(palette.sync_background, brightness_percent, contrast_percent);
        apply_levels(palette.sync_border, brightness_percent, contrast_percent);
        apply_levels(palette.achievements_heading, brightness_percent, contrast_percent);
        apply_levels(palette.achievements_primary, brightness_percent, contrast_percent);
        apply_levels(palette.achievements_secondary, brightness_percent, contrast_percent);
        apply_levels(palette.achievements_background, brightness_percent, contrast_percent);
        apply_levels(palette.achievements_border, brightness_percent, contrast_percent);
        apply_levels(palette.performance_heading, brightness_percent, contrast_percent);
        apply_levels(palette.performance_value, brightness_percent, contrast_percent);
        apply_levels(palette.performance_warning, brightness_percent, contrast_percent);
        apply_levels(palette.performance_background, brightness_percent, contrast_percent);
        apply_levels(palette.performance_border, brightness_percent, contrast_percent);
    }
}
#endif

struct MainMenuPalette
{
    SDL_Color background;
    SDL_Color title;
    SDL_Color profile;
    SDL_Color button_idle;
    SDL_Color button_hover;
    SDL_Color button_selected;
    SDL_Color button_disabled;
    SDL_Color button_disabled_hover;
    SDL_Color button_border_enabled;
    SDL_Color button_border_disabled;
    SDL_Color button_text_enabled;
    SDL_Color button_text_disabled;
    SDL_Color description;
    SDL_Color hint;
    SDL_Color build;
    SDL_Color alert_text_error;
    SDL_Color alert_text_info;
    SDL_Color alert_background_error;
    SDL_Color alert_border_error;
    SDL_Color alert_background_info;
    SDL_Color alert_border_info;
    SDL_Color overlay_heading;
    SDL_Color overlay_line;
    SDL_Color overlay_footer;
    SDL_Color overlay_background;
    SDL_Color overlay_border;
    SDL_Color tutorial_title;
    SDL_Color tutorial_primary;
    SDL_Color tutorial_secondary;
    SDL_Color tutorial_background;
    SDL_Color tutorial_border;
    SDL_Color autosave_icon_idle;
    SDL_Color autosave_icon_in_progress;
    SDL_Color autosave_icon_success;
    SDL_Color autosave_icon_failure;
    SDL_Color autosave_text_in_progress;
    SDL_Color autosave_text_success;
    SDL_Color autosave_text_failure;
    SDL_Color sync_heading;
    SDL_Color sync_idle;
    SDL_Color sync_pending;
    SDL_Color sync_success;
    SDL_Color sync_failure;
    SDL_Color sync_background;
    SDL_Color sync_border;
    SDL_Color achievements_heading;
    SDL_Color achievements_primary;
    SDL_Color achievements_secondary;
    SDL_Color achievements_background;
    SDL_Color achievements_border;
    SDL_Color performance_heading;
    SDL_Color performance_value;
    SDL_Color performance_warning;
    SDL_Color performance_background;
    SDL_Color performance_border;

    MainMenuPalette() noexcept;
};

MainMenuPalette::MainMenuPalette() noexcept
    : background()
    , title()
    , profile()
    , button_idle()
    , button_hover()
    , button_selected()
    , button_disabled()
    , button_disabled_hover()
    , button_border_enabled()
    , button_border_disabled()
    , button_text_enabled()
    , button_text_disabled()
    , description()
    , hint()
    , build()
    , alert_text_error()
    , alert_text_info()
    , alert_background_error()
    , alert_border_error()
    , alert_background_info()
    , alert_border_info()
    , overlay_heading()
    , overlay_line()
    , overlay_footer()
    , overlay_background()
    , overlay_border()
    , tutorial_title()
    , tutorial_primary()
    , tutorial_secondary()
    , tutorial_background()
    , tutorial_border()
    , autosave_icon_idle()
    , autosave_icon_in_progress()
    , autosave_icon_success()
    , autosave_icon_failure()
    , autosave_text_in_progress()
    , autosave_text_success()
    , autosave_text_failure()
    , sync_heading()
    , sync_idle()
    , sync_pending()
    , sync_success()
    , sync_failure()
    , sync_background()
    , sync_border()
    , achievements_heading()
    , achievements_primary()
    , achievements_secondary()
    , achievements_background()
    , achievements_border()
    , performance_heading()
    , performance_value()
    , performance_warning()
    , performance_background()
    , performance_border()
{
}

MainMenuPalette resolve_main_menu_palette(
    bool colorblind_enabled, unsigned int brightness_percent, unsigned int contrast_percent)
{
#if GALACTIC_HAVE_SDL2
    MainMenuPalette palette;

    palette.background = {8, 16, 22, 255};
    palette.title = {232, 238, 250, 255};
    palette.profile = {200, 210, 232, 255};
    palette.button_idle = {34, 52, 72, 255};
    palette.button_hover = {94, 130, 184, 255};
    palette.button_selected = {68, 108, 168, 255};
    palette.button_disabled = {44, 52, 70, 255};
    palette.button_disabled_hover = {66, 80, 104, 255};
    palette.button_border_enabled = {120, 154, 212, 255};
    palette.button_border_disabled = {90, 110, 160, 255};
    palette.button_text_enabled = {255, 255, 255, 255};
    palette.button_text_disabled = {188, 196, 210, 255};
    palette.description = {210, 220, 240, 255};
    palette.hint = {165, 176, 204, 255};
    palette.build = {140, 150, 184, 255};
    palette.alert_text_error = {255, 206, 206, 255};
    palette.alert_text_info = {200, 230, 255, 255};
    palette.alert_background_error = {84, 32, 32, 235};
    palette.alert_border_error = {164, 80, 80, 255};
    palette.alert_background_info = {26, 64, 88, 235};
    palette.alert_border_info = {88, 138, 176, 255};
    palette.overlay_heading = {236, 242, 255, 255};
    palette.overlay_line = {208, 216, 236, 255};
    palette.overlay_footer = {184, 196, 224, 255};
    palette.overlay_background = {18, 24, 44, 238};
    palette.overlay_border = {92, 112, 166, 255};
    palette.tutorial_title = {234, 238, 250, 255};
    palette.tutorial_primary = {210, 220, 240, 255};
    palette.tutorial_secondary = {200, 208, 232, 255};
    palette.tutorial_background = {18, 24, 44, 235};
    palette.tutorial_border = {90, 110, 160, 255};
    palette.autosave_icon_idle = {74, 88, 116, 255};
    palette.autosave_icon_in_progress = {128, 172, 228, 255};
    palette.autosave_icon_success = {104, 188, 148, 255};
    palette.autosave_icon_failure = {220, 96, 96, 255};
    palette.autosave_text_in_progress = {188, 206, 238, 255};
    palette.autosave_text_success = {210, 240, 220, 255};
    palette.autosave_text_failure = {255, 214, 214, 255};
    palette.sync_heading = {228, 236, 252, 255};
    palette.sync_idle = {186, 198, 224, 255};
    palette.sync_pending = {196, 210, 240, 255};
    palette.sync_success = {206, 238, 220, 255};
    palette.sync_failure = {255, 214, 214, 255};
    palette.sync_background = {22, 30, 54, 235};
    palette.sync_border = {92, 116, 170, 255};
    palette.achievements_heading = {242, 248, 255, 255};
    palette.achievements_primary = {210, 220, 240, 255};
    palette.achievements_secondary = {194, 206, 232, 255};
    palette.achievements_background = {24, 34, 58, 235};
    palette.achievements_border = {96, 120, 168, 255};
    palette.performance_heading = {236, 244, 255, 255};
    palette.performance_value = {204, 214, 236, 255};
    palette.performance_warning = {255, 226, 196, 255};
    palette.performance_background = {20, 30, 50, 230};
    palette.performance_border = {92, 120, 170, 255};

    if (colorblind_enabled)
    {
        palette.background = {10, 20, 26, 255};
        palette.title = {240, 246, 255, 255};
        palette.profile = {212, 220, 240, 255};
        palette.button_idle = {44, 62, 88, 255};
        palette.button_hover = {104, 140, 204, 255};
        palette.button_selected = {78, 118, 184, 255};
        palette.button_disabled = {52, 60, 80, 255};
        palette.button_disabled_hover = {74, 88, 112, 255};
        palette.button_border_enabled = {134, 170, 228, 255};
        palette.button_border_disabled = {102, 120, 168, 255};
        palette.button_text_enabled = {255, 255, 255, 255};
        palette.button_text_disabled = {216, 222, 234, 255};
        palette.description = {216, 228, 248, 255};
        palette.hint = {176, 192, 220, 255};
        palette.build = {156, 170, 204, 255};
        palette.alert_text_error = {255, 224, 200, 255};
        palette.alert_text_info = {210, 242, 255, 255};
        palette.alert_background_error = {112, 60, 30, 240};
        palette.alert_border_error = {204, 128, 64, 255};
        palette.alert_background_info = {26, 78, 104, 240};
        palette.alert_border_info = {96, 160, 200, 255};
        palette.overlay_heading = {240, 246, 255, 255};
        palette.overlay_line = {212, 224, 244, 255};
        palette.overlay_footer = {192, 206, 232, 255};
        palette.overlay_background = {28, 44, 70, 238};
        palette.overlay_border = {120, 156, 210, 255};
        palette.tutorial_title = {240, 246, 255, 255};
        palette.tutorial_primary = {214, 234, 248, 255};
        palette.tutorial_secondary = {200, 218, 240, 255};
        palette.tutorial_background = {28, 44, 70, 235};
        palette.tutorial_border = {120, 156, 210, 255};
        palette.autosave_icon_idle = {88, 112, 144, 255};
        palette.autosave_icon_in_progress = {156, 196, 246, 255};
        palette.autosave_icon_success = {128, 212, 168, 255};
        palette.autosave_icon_failure = {232, 124, 116, 255};
        palette.autosave_text_in_progress = {204, 222, 248, 255};
        palette.autosave_text_success = {222, 246, 228, 255};
        palette.autosave_text_failure = {255, 226, 222, 255};
        palette.sync_heading = {240, 246, 255, 255};
        palette.sync_idle = {202, 216, 240, 255};
        palette.sync_pending = {210, 232, 248, 255};
        palette.sync_success = {226, 248, 232, 255};
        palette.sync_failure = {255, 228, 220, 255};
        palette.sync_background = {32, 48, 78, 235};
        palette.sync_border = {126, 162, 214, 255};
        palette.achievements_heading = {248, 252, 255, 255};
        palette.achievements_primary = {214, 234, 248, 255};
        palette.achievements_secondary = {202, 220, 240, 255};
        palette.achievements_background = {36, 54, 84, 235};
        palette.achievements_border = {138, 178, 226, 255};
        palette.performance_heading = {244, 250, 255, 255};
        palette.performance_value = {210, 222, 244, 255};
        palette.performance_warning = {255, 232, 208, 255};
        palette.performance_background = {32, 52, 80, 230};
        palette.performance_border = {132, 168, 220, 255};
    }

    brightness_percent = clamp_percent(
        brightness_percent, PLAYER_PROFILE_BRIGHTNESS_MIN_PERCENT, PLAYER_PROFILE_BRIGHTNESS_MAX_PERCENT);
    contrast_percent = clamp_percent(
        contrast_percent, PLAYER_PROFILE_CONTRAST_MIN_PERCENT, PLAYER_PROFILE_CONTRAST_MAX_PERCENT);
    apply_levels(palette, brightness_percent, contrast_percent);

    return palette;
#else
    (void)colorblind_enabled;
    (void)brightness_percent;
    (void)contrast_percent;
    return MainMenuPalette();
#endif
}
