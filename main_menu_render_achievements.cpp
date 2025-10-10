#include "main_menu_system.hpp"

#include "menu_localization.hpp"

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

void main_menu_render_achievements(SDL_Renderer &renderer, TTF_Font *menu_font,
    const MainMenuAchievementsSummary *achievements, int output_width, int output_height,
    const MainMenuPalette &palette)
{
    if (menu_font == ft_nullptr)
        return;
    if (achievements == ft_nullptr)
        return;

    SDL_Color heading_color = palette.achievements_heading;
    SDL_Color line_primary_color = palette.achievements_primary;
    SDL_Color line_secondary_color = palette.achievements_secondary;
    SDL_Rect  heading_rect;
    ft_string heading_label = menu_localize("main_menu.achievements.heading", "Achievements");
    SDL_Texture *heading_texture
        = create_text_texture(renderer, *menu_font, heading_label, heading_color, heading_rect);
    ft_vector<ft_string> lines = main_menu_collect_achievement_lines(*achievements);
    ft_vector<SDL_Texture *> line_textures;
    ft_vector<SDL_Rect>      line_rects;
    line_textures.reserve(lines.size());
    line_rects.reserve(lines.size());

    int panel_padding = 12;
    int line_spacing = 2;
    int panel_height = panel_padding * 2;
    int panel_width = 0;
    if (heading_texture != ft_nullptr)
    {
        panel_height += heading_rect.h;
        panel_height += 10;
        panel_width = heading_rect.w;
    }

    for (size_t index = 0; index < lines.size(); ++index)
    {
        SDL_Color line_color = (index == 0) ? line_primary_color : line_secondary_color;
        SDL_Rect  line_rect;
        SDL_Texture *line_texture = create_text_texture(renderer, *menu_font, lines[index], line_color, line_rect);
        if (line_texture != ft_nullptr)
        {
            line_textures.push_back(line_texture);
            line_rects.push_back(line_rect);
            panel_height += line_rect.h;
            if (static_cast<int>(line_rect.w) > panel_width)
                panel_width = line_rect.w;
            if (index + 1U < lines.size())
                panel_height += line_spacing;
        }
    }

    if (!line_textures.empty() || heading_texture != ft_nullptr)
    {
        int min_panel_width = 360;
        int overlay_margin = 32;
        panel_width = panel_padding * 2 + panel_width;
        if (panel_width < min_panel_width)
            panel_width = min_panel_width;
        SDL_Rect panel_rect;
        panel_rect.x = overlay_margin;
        panel_rect.y = 140;
        panel_rect.w = panel_width;
        panel_rect.h = panel_height;
        SDL_SetRenderDrawColor(&renderer, palette.achievements_background.r, palette.achievements_background.g,
            palette.achievements_background.b, palette.achievements_background.a);
        SDL_RenderFillRect(&renderer, &panel_rect);
        SDL_SetRenderDrawColor(&renderer, palette.achievements_border.r, palette.achievements_border.g,
            palette.achievements_border.b, palette.achievements_border.a);
        SDL_RenderDrawRect(&renderer, &panel_rect);

        int text_x = panel_rect.x + panel_padding;
        int text_y = panel_rect.y + panel_padding;
        if (heading_texture != ft_nullptr)
        {
            heading_rect.x = text_x;
            heading_rect.y = text_y;
            SDL_RenderCopy(&renderer, heading_texture, ft_nullptr, &heading_rect);
            text_y += heading_rect.h + 10;
        }

        for (size_t index = 0; index < line_textures.size(); ++index)
        {
            SDL_Texture *line_texture = line_textures[index];
            SDL_Rect      line_rect = line_rects[index];
            line_rect.x = text_x;
            line_rect.y = text_y;
            SDL_RenderCopy(&renderer, line_texture, ft_nullptr, &line_rect);
            text_y += line_rect.h;
            if (index + 1U < line_textures.size())
                text_y += line_spacing;
            SDL_DestroyTexture(line_texture);
            line_textures[index] = ft_nullptr;
        }
    }

    for (size_t index = 0; index < line_textures.size(); ++index)
    {
        SDL_Texture *line_texture = line_textures[index];
        if (line_texture != ft_nullptr)
            SDL_DestroyTexture(line_texture);
    }
    if (heading_texture != ft_nullptr)
        SDL_DestroyTexture(heading_texture);
    (void)output_width;
    (void)output_height;
}
#else
void main_menu_render_achievements(SDL_Renderer &renderer, TTF_Font *menu_font,
    const MainMenuAchievementsSummary *achievements, int output_width, int output_height,
    const MainMenuPalette &palette)
{
    (void)renderer;
    (void)menu_font;
    (void)achievements;
    (void)output_width;
    (void)output_height;
    (void)palette;
}
#endif
