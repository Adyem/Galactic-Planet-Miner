#include "main_menu_system.hpp"

#include "menu_localization.hpp"

#include "libft/Libft/libft.hpp"

ft_string main_menu_resolve_description(const ft_ui_menu &menu);
ft_string main_menu_resolve_navigation_hint(
    const ft_ui_menu &menu, const PlayerProfilePreferences *preferences);

void main_menu_render_overlay(SDL_Renderer &renderer, TTF_Font *menu_font, int output_width, int output_height,
    const MainMenuOverlayContext *overlay, const MainMenuPalette &palette);
void main_menu_render_tutorial_overlay(SDL_Renderer &renderer, TTF_Font *menu_font, int output_width,
    const MainMenuTutorialContext *tutorial, const MainMenuPalette &palette);
void main_menu_render_achievements(SDL_Renderer &renderer, TTF_Font *menu_font,
    const MainMenuAchievementsSummary *achievements, int output_width, int output_height,
    const MainMenuPalette &palette);
void main_menu_render_autosave(SDL_Renderer &renderer, TTF_Font *menu_font, const MainMenuAutosaveStatus *autosave,
    int output_height, const MainMenuPalette &palette);

MainMenuPalette resolve_main_menu_palette(
    bool colorblind_enabled, unsigned int brightness_percent, unsigned int contrast_percent);

void render_main_menu(SDL_Renderer &renderer, const ft_ui_menu &menu, TTF_Font *title_font, TTF_Font *menu_font,
    int window_width, int window_height, const ft_string &active_profile_name, const PlayerProfilePreferences *preferences,
    const MainMenuTutorialContext *tutorial,
    const MainMenuOverlayContext *manual, const MainMenuOverlayContext *changelog,
    const MainMenuOverlayContext *cloud_confirmation, const MainMenuOverlayContext *crash_prompt,
    const MainMenuConnectivityStatus *connectivity,
    const MainMenuSyncStatus *sync_status, const MainMenuAchievementsSummary *achievements,
    const MainMenuAutosaveStatus *autosave, const MainMenuPerformanceStats *performance, const MainMenuAlertBanner *alert)
{
#if GALACTIC_HAVE_SDL2
    const bool use_colorblind_palette
        = preferences != ft_nullptr && preferences->colorblind_palette_enabled;
    unsigned int brightness_percent = 100U;
    unsigned int contrast_percent = 100U;
    if (preferences != ft_nullptr)
    {
        brightness_percent = preferences->brightness_percent;
        contrast_percent = preferences->contrast_percent;
    }
    MainMenuPalette palette
        = resolve_main_menu_palette(use_colorblind_palette, brightness_percent, contrast_percent);

    SDL_SetRenderDrawColor(&renderer, palette.background.r, palette.background.g, palette.background.b,
        palette.background.a);
    SDL_RenderClear(&renderer);

    int output_width = window_width;
    int output_height = window_height;
    SDL_GetRendererOutputSize(&renderer, &output_width, &output_height);

    if (title_font != ft_nullptr)
    {
        SDL_Color title_color = palette.title;
        SDL_Rect  title_rect;
        ft_string menu_title = menu_localize("main_menu.title", "Galactic Planet Miner");
        SDL_Texture *title_texture = create_text_texture(renderer, *title_font, menu_title, title_color,
            title_rect);
        if (title_texture != ft_nullptr)
        {
            title_rect.x = output_width / 2 - title_rect.w / 2;
            title_rect.y = 96;
            SDL_RenderCopy(&renderer, title_texture, ft_nullptr, &title_rect);
            SDL_DestroyTexture(title_texture);
        }
    }

    main_menu_render_achievements(renderer, menu_font, achievements, output_width, output_height, palette);

    main_menu_render_autosave(renderer, menu_font, autosave, output_height, palette);

    ft_rect menu_viewport = build_main_menu_viewport();
    SDL_Rect viewport_rect;
    viewport_rect.x = menu_viewport.left;
    viewport_rect.y = menu_viewport.top;
    viewport_rect.w = menu_viewport.width;
    viewport_rect.h = menu_viewport.height;
    SDL_Rect output_rect;
    output_rect.x = viewport_rect.x;
    output_rect.y = viewport_rect.y;
    output_rect.w = viewport_rect.w;
    output_rect.h = viewport_rect.h;
    if (output_rect.x < 0)
        output_rect.x = 0;
    if (output_rect.y < 0)
        output_rect.y = 0;
    if (output_rect.w < 0)
        output_rect.w = 0;
    if (output_rect.h < 0)
        output_rect.h = 0;
    SDL_RenderSetViewport(&renderer, &output_rect);

    menu.render(renderer, menu_font, palette.button_idle, palette.button_hover, palette.button_selected,
        palette.button_disabled, palette.button_disabled_hover, palette.button_border_enabled,
        palette.button_border_disabled, palette.button_text_enabled, palette.button_text_disabled);

    SDL_RenderSetViewport(&renderer, ft_nullptr);

    int next_text_top = viewport_rect.y + viewport_rect.h + 28;

    if (menu_font != ft_nullptr)
    {
        const ft_string description = main_menu_resolve_description(menu);
        if (!description.empty())
        {
            SDL_Color description_color = palette.description;
            SDL_Rect  description_rect;
            SDL_Texture *description_texture
                = create_text_texture(renderer, *menu_font, description, description_color, description_rect);
            if (description_texture != ft_nullptr)
            {
                description_rect.x = output_width / 2 - description_rect.w / 2;
                description_rect.y = next_text_top;
                SDL_RenderCopy(&renderer, description_texture, ft_nullptr, &description_rect);
                SDL_DestroyTexture(description_texture);
                next_text_top = description_rect.y + description_rect.h + 28;
            }
        }
    }

    if (menu_font != ft_nullptr)
    {
        const ft_string hint = main_menu_resolve_navigation_hint(menu, preferences);
        if (!hint.empty())
        {
            SDL_Color hint_color = palette.hint;
            SDL_Rect  hint_rect;
            SDL_Texture *hint_texture = create_text_texture(renderer, *menu_font, hint, hint_color, hint_rect);
            if (hint_texture != ft_nullptr)
            {
                hint_rect.x = output_width / 2 - hint_rect.w / 2;
                hint_rect.y = next_text_top;
                SDL_RenderCopy(&renderer, hint_texture, ft_nullptr, &hint_rect);
                SDL_DestroyTexture(hint_texture);
                next_text_top = hint_rect.y + hint_rect.h + 20;
            }
        }
    }

    if (menu_font != ft_nullptr)
    {
        const ft_string build_label = main_menu_resolve_build_label();
        if (!build_label.empty())
        {
            SDL_Color build_color = palette.build;
            SDL_Rect  build_rect;
            SDL_Texture *build_texture = create_text_texture(renderer, *menu_font, build_label, build_color, build_rect);
            if (build_texture != ft_nullptr)
            {
                build_rect.x = output_width - build_rect.w - 48;
                build_rect.y = output_height - build_rect.h - 40;
                SDL_RenderCopy(&renderer, build_texture, ft_nullptr, &build_rect);
                SDL_DestroyTexture(build_texture);
            }
        }
    }

    main_menu_render_overlay(renderer, menu_font, output_width, output_height, manual, palette);
    main_menu_render_overlay(renderer, menu_font, output_width, output_height, changelog, palette);
    main_menu_render_overlay(renderer, menu_font, output_width, output_height, cloud_confirmation, palette);
    main_menu_render_overlay(renderer, menu_font, output_width, output_height, crash_prompt, palette);
    main_menu_render_tutorial_overlay(renderer, menu_font, output_width, tutorial, palette);

    SDL_RenderPresent(&renderer);
#else
    (void)renderer;
    (void)menu;
    (void)title_font;
    (void)menu_font;
    (void)window_width;
    (void)window_height;
    (void)active_profile_name;
    (void)preferences;
    (void)tutorial;
    (void)manual;
    (void)changelog;
    (void)cloud_confirmation;
    (void)crash_prompt;
    (void)connectivity;
    (void)sync_status;
    (void)achievements;
    (void)autosave;
    (void)performance;
    (void)alert;
#endif
}

namespace main_menu_testing
{
    ft_string resolve_active_description(const ft_ui_menu &menu)
    {
        return main_menu_resolve_description(menu);
    }

    ft_string resolve_navigation_hint(
        const ft_ui_menu &menu, const PlayerProfilePreferences *preferences)
    {
        return main_menu_resolve_navigation_hint(menu, preferences);
    }
}
