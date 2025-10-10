#include "main_menu_system.hpp"

#include "libft/Time/time.hpp"

#if GALACTIC_HAVE_SDL2
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

void main_menu_render_autosave(SDL_Renderer &renderer, TTF_Font *menu_font, const MainMenuAutosaveStatus *autosave,
    int output_height, const MainMenuPalette &palette)
{
    if (menu_font == ft_nullptr)
        return;
    if (autosave == ft_nullptr)
        return;
    if (!main_menu_autosave_is_visible(*autosave, ft_time_ms()))
        return;

    ft_string autosave_label = main_menu_resolve_autosave_label(*autosave);
    SDL_Color icon_color = palette.autosave_icon_idle;
    SDL_Color text_color = palette.autosave_text_in_progress;
    if (autosave->state == MAIN_MENU_AUTOSAVE_SUCCEEDED)
    {
        icon_color = palette.autosave_icon_success;
        text_color = palette.autosave_text_success;
    }
    else if (autosave->state == MAIN_MENU_AUTOSAVE_FAILED)
    {
        icon_color = palette.autosave_icon_failure;
        text_color = palette.autosave_text_failure;
    }

    SDL_Rect icon_rect;
    icon_rect.w = 12;
    icon_rect.h = 12;
    icon_rect.x = 48;
    icon_rect.y = output_height - 140;
    SDL_SetRenderDrawColor(&renderer, icon_color.r, icon_color.g, icon_color.b, icon_color.a);
    SDL_RenderFillRect(&renderer, &icon_rect);

    SDL_Rect text_rect;
    SDL_Texture *text_texture = create_text_texture(renderer, *menu_font, autosave_label, text_color, text_rect);
    if (text_texture != ft_nullptr)
    {
        text_rect.x = icon_rect.x + icon_rect.w + 12;
        text_rect.y = icon_rect.y - text_rect.h / 2 + icon_rect.h / 2;
        SDL_RenderCopy(&renderer, text_texture, ft_nullptr, &text_rect);
        SDL_DestroyTexture(text_texture);
    }
}
#else
void main_menu_render_autosave(SDL_Renderer &renderer, TTF_Font *menu_font, const MainMenuAutosaveStatus *autosave,
    int output_height, const MainMenuPalette &palette)
{
    (void)renderer;
    (void)menu_font;
    (void)autosave;
    (void)output_height;
    (void)palette;
}
#endif
