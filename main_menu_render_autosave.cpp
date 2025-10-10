#include "main_menu_system.hpp"

#include "libft/Time/time.hpp"

#if GALACTIC_HAVE_SDL2
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
