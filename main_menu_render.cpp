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

    const ft_vector<ft_menu_item> &items = menu.get_items();
    const int                      hovered_index = menu.get_hovered_index();
    const int                      selected_index = menu.get_selected_index();
    const ft_rect                 &viewport = menu.get_viewport_bounds();
    const int                      scroll_offset = menu.get_scroll_offset();

    const bool clip_enabled = viewport.width > 0 && viewport.height > 0;
    SDL_Rect   clip_rect;
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

    auto set_draw_color = [&](const SDL_Color &color) {
        SDL_SetRenderDrawColor(&renderer, color.r, color.g, color.b, color.a);
    };

    for (size_t index = 0; index < items.size(); ++index)
    {
        const ft_menu_item &item = items[index];

        SDL_Rect button_rect;
        button_rect.x = item.bounds.left;
        button_rect.y = item.bounds.top - scroll_offset;
        button_rect.w = item.bounds.width;
        button_rect.h = item.bounds.height;

        if (clip_enabled && (button_rect.y + button_rect.h <= clip_rect.y || button_rect.y >= clip_bottom))
            continue;

        const bool        is_hovered = static_cast<int>(index) == hovered_index;
        const bool        is_selected = static_cast<int>(index) == selected_index;
        const bool        is_enabled = item.enabled;
        const SDL_Color  &fill_color = !is_enabled
                ? (is_hovered ? palette.button_disabled_hover : palette.button_disabled)
                : (is_hovered ? palette.button_hover : (is_selected ? palette.button_selected : palette.button_idle));
        const SDL_Color  &border_color = is_enabled ? palette.button_border_enabled : palette.button_border_disabled;
        const SDL_Color  &text_color = is_enabled ? palette.button_text_enabled : palette.button_text_disabled;

        set_draw_color(fill_color);
        SDL_RenderFillRect(&renderer, &button_rect);

        set_draw_color(border_color);
        SDL_RenderDrawRect(&renderer, &button_rect);

        if (menu_font != ft_nullptr && !item.label.empty())
        {
            SDL_Rect text_rect;
            SDL_Texture *text_texture
                = create_text_texture(renderer, *menu_font, item.label, text_color, text_rect);
            if (text_texture != ft_nullptr)
            {
                text_rect.x = item.bounds.left + (item.bounds.width - text_rect.w) / 2;
                text_rect.y = button_rect.y + (item.bounds.height - text_rect.h) / 2;
                SDL_RenderCopy(&renderer, text_texture, ft_nullptr, &text_rect);
                SDL_DestroyTexture(text_texture);
            }
        }
    }

    if (clip_enabled)
        SDL_RenderSetClipRect(&renderer, ft_nullptr);

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

    auto align_right = [&](int content_width) {
        const int margin = 48;
        int       x = output_width - content_width - margin;
        if (x < margin)
            x = margin;
        return x;
    };

    int sidebar_top = 220;

    if (menu_font != ft_nullptr)
    {
        ft_string profile_label = menu_localize("main_menu.profile_prefix", "Profile: ");
        if (!active_profile_name.empty())
            profile_label.append(active_profile_name);
        SDL_Rect profile_rect;
        SDL_Texture *profile_texture
            = create_text_texture(renderer, *menu_font, profile_label, palette.profile, profile_rect);
        if (profile_texture != ft_nullptr)
        {
            profile_rect.x = align_right(profile_rect.w);
            profile_rect.y = 164;
            SDL_RenderCopy(&renderer, profile_texture, ft_nullptr, &profile_rect);
            SDL_DestroyTexture(profile_texture);
            sidebar_top = profile_rect.y + profile_rect.h + 24;
        }
    }

    if (sidebar_top < 220)
        sidebar_top = 220;

    auto advance_sidebar = [&](int consumed_height) {
        sidebar_top += consumed_height;
        sidebar_top += 20;
    };

    if (menu_font != ft_nullptr && alert != ft_nullptr && alert->visible && !alert->message.empty())
    {
        SDL_Color alert_text_color = alert->is_error ? palette.alert_text_error : palette.alert_text_info;
        SDL_Color alert_background = alert->is_error ? palette.alert_background_error : palette.alert_background_info;
        SDL_Color alert_border = alert->is_error ? palette.alert_border_error : palette.alert_border_info;
        SDL_Rect  alert_text_rect;
        SDL_Texture *alert_texture
            = create_text_texture(renderer, *menu_font, alert->message, alert_text_color, alert_text_rect);
        if (alert_texture != ft_nullptr)
        {
            const int padding = 14;
            int       panel_width = alert_text_rect.w + padding * 2;
            const int available_width = output_width - 96;
            if (panel_width > available_width)
                panel_width = available_width;
            int panel_x = align_right(panel_width);
            SDL_Rect panel_rect;
            panel_rect.x = panel_x;
            panel_rect.y = sidebar_top;
            panel_rect.w = panel_width;
            panel_rect.h = alert_text_rect.h + padding * 2;
            set_draw_color(alert_background);
            SDL_RenderFillRect(&renderer, &panel_rect);
            set_draw_color(alert_border);
            SDL_RenderDrawRect(&renderer, &panel_rect);

            alert_text_rect.x = panel_rect.x + (panel_rect.w - alert_text_rect.w) / 2;
            alert_text_rect.y = panel_rect.y + (panel_rect.h - alert_text_rect.h) / 2;
            if (alert_text_rect.x < panel_rect.x + padding)
                alert_text_rect.x = panel_rect.x + padding;
            SDL_RenderCopy(&renderer, alert_texture, ft_nullptr, &alert_text_rect);
            SDL_DestroyTexture(alert_texture);
            advance_sidebar(panel_rect.h);
        }
    }

    if (menu_font != ft_nullptr && connectivity != ft_nullptr)
    {
        ft_string connectivity_label = main_menu_resolve_connectivity_label(*connectivity);
        if (!connectivity_label.empty())
        {
            SDL_Color connectivity_color = main_menu_resolve_connectivity_color(*connectivity);
            SDL_Rect  connectivity_rect;
            SDL_Texture *connectivity_texture = create_text_texture(
                renderer, *menu_font, connectivity_label, connectivity_color, connectivity_rect);
            if (connectivity_texture != ft_nullptr)
            {
                connectivity_rect.x = align_right(connectivity_rect.w);
                connectivity_rect.y = sidebar_top;
                SDL_RenderCopy(&renderer, connectivity_texture, ft_nullptr, &connectivity_rect);
                SDL_DestroyTexture(connectivity_texture);
                advance_sidebar(connectivity_rect.h);
            }
        }
    }

    auto render_sync_panel = [&](const MainMenuSyncStatus &status) {
        ft_string heading = menu_localize("main_menu.sync.heading", "Sync Status");
        SDL_Rect  heading_rect;
        SDL_Texture *heading_texture
            = create_text_texture(renderer, *menu_font, heading, palette.sync_heading, heading_rect);

        const MainMenuSyncEntry &convoys_entry = status.convoys;
        const MainMenuSyncEntry &leader_entry = status.leaderboards;

        ft_string convoys_label = main_menu_resolve_sync_entry_label(status, MAIN_MENU_SYNC_CHANNEL_CONVOYS);
        ft_string leader_label = main_menu_resolve_sync_entry_label(status, MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS);

        auto resolve_entry_color = [&](const MainMenuSyncEntry &entry) {
            if (entry.state == MAIN_MENU_SYNC_CHECKING)
                return palette.sync_pending;
            if (entry.state == MAIN_MENU_SYNC_SUCCESS)
                return palette.sync_success;
            if (entry.state == MAIN_MENU_SYNC_FAILURE)
                return palette.sync_failure;
            return palette.sync_idle;
        };

        SDL_Color convoys_color = resolve_entry_color(convoys_entry);
        SDL_Color leader_color = resolve_entry_color(leader_entry);

        SDL_Rect convoys_rect;
        SDL_Texture *convoys_texture
            = create_text_texture(renderer, *menu_font, convoys_label, convoys_color, convoys_rect);
        SDL_Rect leader_rect;
        SDL_Texture *leader_texture
            = create_text_texture(renderer, *menu_font, leader_label, leader_color, leader_rect);

        int content_width = 0;
        int panel_height = 0;
        const int padding = 14;
        const int line_spacing = 6;
        panel_height += padding * 2;

        if (heading_texture != ft_nullptr)
        {
            panel_height += heading_rect.h;
            content_width = heading_rect.w;
            if (convoys_texture != ft_nullptr || leader_texture != ft_nullptr)
                panel_height += 8;
        }

        if (convoys_texture != ft_nullptr)
        {
            panel_height += convoys_rect.h;
            if (convoys_rect.w > content_width)
                content_width = convoys_rect.w;
            if (leader_texture != ft_nullptr)
                panel_height += line_spacing;
        }

        if (leader_texture != ft_nullptr)
        {
            panel_height += leader_rect.h;
            if (leader_rect.w > content_width)
                content_width = leader_rect.w;
        }

        if (content_width <= 0 || panel_height <= padding * 2)
        {
            if (heading_texture != ft_nullptr)
                SDL_DestroyTexture(heading_texture);
            if (convoys_texture != ft_nullptr)
                SDL_DestroyTexture(convoys_texture);
            if (leader_texture != ft_nullptr)
                SDL_DestroyTexture(leader_texture);
            return;
        }

        int panel_width = content_width + padding * 2;
        const int available_width = output_width - 96;
        if (panel_width > available_width)
            panel_width = available_width;
        SDL_Rect panel_rect;
        panel_rect.w = panel_width;
        panel_rect.h = panel_height;
        panel_rect.x = align_right(panel_rect.w);
        panel_rect.y = sidebar_top;

        set_draw_color(palette.sync_background);
        SDL_RenderFillRect(&renderer, &panel_rect);
        set_draw_color(palette.sync_border);
        SDL_RenderDrawRect(&renderer, &panel_rect);

        int text_x = panel_rect.x + padding;
        int text_y = panel_rect.y + padding;

        if (heading_texture != ft_nullptr)
        {
            heading_rect.x = text_x;
            heading_rect.y = text_y;
            SDL_RenderCopy(&renderer, heading_texture, ft_nullptr, &heading_rect);
            text_y += heading_rect.h + 8;
            SDL_DestroyTexture(heading_texture);
        }

        if (convoys_texture != ft_nullptr)
        {
            convoys_rect.x = text_x;
            convoys_rect.y = text_y;
            SDL_RenderCopy(&renderer, convoys_texture, ft_nullptr, &convoys_rect);
            text_y += convoys_rect.h;
            SDL_DestroyTexture(convoys_texture);
            if (leader_texture != ft_nullptr)
                text_y += line_spacing;
        }

        if (leader_texture != ft_nullptr)
        {
            leader_rect.x = text_x;
            leader_rect.y = text_y;
            SDL_RenderCopy(&renderer, leader_texture, ft_nullptr, &leader_rect);
            text_y += leader_rect.h;
            SDL_DestroyTexture(leader_texture);
        }

        advance_sidebar(panel_rect.h);
    };

    if (menu_font != ft_nullptr && sync_status != ft_nullptr)
        render_sync_panel(*sync_status);

    auto render_performance_panel = [&](const MainMenuPerformanceStats &stats) {
        ft_string heading = menu_localize("main_menu.performance.heading", "Performance");
        SDL_Rect  heading_rect;
        SDL_Texture *heading_texture
            = create_text_texture(renderer, *menu_font, heading, palette.performance_heading, heading_rect);

        ft_string fps_label = main_menu_format_performance_fps_label(stats);
        SDL_Rect  fps_rect;
        SDL_Texture *fps_texture
            = create_text_texture(renderer, *menu_font, fps_label, palette.performance_value, fps_rect);

        ft_string latency_label = main_menu_format_performance_latency_label(stats);
        SDL_Color latency_color = palette.performance_value;
        if (stats.latency_sampled && !stats.latency_successful)
            latency_color = palette.performance_warning;
        SDL_Rect latency_rect;
        SDL_Texture *latency_texture
            = create_text_texture(renderer, *menu_font, latency_label, latency_color, latency_rect);

        int content_width = 0;
        int panel_height = 0;
        const int padding = 14;
        const int line_spacing = 6;
        panel_height += padding * 2;

        if (heading_texture != ft_nullptr)
        {
            panel_height += heading_rect.h;
            content_width = heading_rect.w;
            if (fps_texture != ft_nullptr || latency_texture != ft_nullptr)
                panel_height += 8;
        }

        if (fps_texture != ft_nullptr)
        {
            panel_height += fps_rect.h;
            if (fps_rect.w > content_width)
                content_width = fps_rect.w;
            if (latency_texture != ft_nullptr)
                panel_height += line_spacing;
        }

        if (latency_texture != ft_nullptr)
        {
            panel_height += latency_rect.h;
            if (latency_rect.w > content_width)
                content_width = latency_rect.w;
        }

        if (content_width <= 0 || panel_height <= padding * 2)
        {
            if (heading_texture != ft_nullptr)
                SDL_DestroyTexture(heading_texture);
            if (fps_texture != ft_nullptr)
                SDL_DestroyTexture(fps_texture);
            if (latency_texture != ft_nullptr)
                SDL_DestroyTexture(latency_texture);
            return;
        }

        int panel_width = content_width + padding * 2;
        const int available_width = output_width - 96;
        if (panel_width > available_width)
            panel_width = available_width;
        SDL_Rect panel_rect;
        panel_rect.w = panel_width;
        panel_rect.h = panel_height;
        panel_rect.x = align_right(panel_rect.w);
        panel_rect.y = sidebar_top;

        set_draw_color(palette.performance_background);
        SDL_RenderFillRect(&renderer, &panel_rect);
        set_draw_color(palette.performance_border);
        SDL_RenderDrawRect(&renderer, &panel_rect);

        int text_x = panel_rect.x + padding;
        int text_y = panel_rect.y + padding;

        if (heading_texture != ft_nullptr)
        {
            heading_rect.x = text_x;
            heading_rect.y = text_y;
            SDL_RenderCopy(&renderer, heading_texture, ft_nullptr, &heading_rect);
            text_y += heading_rect.h + 8;
            SDL_DestroyTexture(heading_texture);
        }

        if (fps_texture != ft_nullptr)
        {
            fps_rect.x = text_x;
            fps_rect.y = text_y;
            SDL_RenderCopy(&renderer, fps_texture, ft_nullptr, &fps_rect);
            text_y += fps_rect.h;
            SDL_DestroyTexture(fps_texture);
            if (latency_texture != ft_nullptr)
                text_y += line_spacing;
        }

        if (latency_texture != ft_nullptr)
        {
            latency_rect.x = text_x;
            latency_rect.y = text_y;
            SDL_RenderCopy(&renderer, latency_texture, ft_nullptr, &latency_rect);
            text_y += latency_rect.h;
            SDL_DestroyTexture(latency_texture);
        }

        advance_sidebar(panel_rect.h);
    };

    if (menu_font != ft_nullptr && performance != ft_nullptr)
        render_performance_panel(*performance);

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
