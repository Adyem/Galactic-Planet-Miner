#include "main_menu_system.hpp"

#if GALACTIC_HAVE_SDL2
namespace
{
    void destroy_texture(SDL_Texture *texture)
    {
        if (texture != ft_nullptr)
            SDL_DestroyTexture(texture);
    }
}
#endif

void main_menu_render_overlay(SDL_Renderer &renderer, TTF_Font *menu_font, int output_width, int output_height,
    const MainMenuOverlayContext *overlay, const MainMenuPalette &palette)
{
#if GALACTIC_HAVE_SDL2
    if (overlay == ft_nullptr || !overlay->visible)
        return;
    if (menu_font == ft_nullptr)
        return;

    const int overlay_padding = 16;
    const int min_overlay_width = 520;
    const int line_spacing = 6;

    int overlay_height = overlay_padding * 2;
    int max_content_width = 0;

    SDL_Texture *heading_texture = ft_nullptr;
    SDL_Rect     heading_rect;
    heading_rect.x = 0;
    heading_rect.y = 0;
    heading_rect.w = 0;
    heading_rect.h = 0;

    if (!overlay->heading.empty())
    {
        SDL_Color heading_color = palette.overlay_heading;
        heading_texture = create_text_texture(renderer, *menu_font, overlay->heading, heading_color, heading_rect);
        if (heading_texture != ft_nullptr)
        {
            overlay_height += heading_rect.h;
            max_content_width = heading_rect.w;
            overlay_height += 18;
        }
    }

    ft_vector<SDL_Texture *> line_textures;
    ft_vector<SDL_Rect>      line_rects;
    line_textures.reserve(overlay->lines.size());
    line_rects.reserve(overlay->lines.size());

    SDL_Color line_color = palette.overlay_line;

    for (size_t index = 0; index < overlay->lines.size(); ++index)
    {
        SDL_Rect line_rect;
        SDL_Texture *line_texture = create_text_texture(renderer, *menu_font, overlay->lines[index], line_color, line_rect);
        if (line_texture != ft_nullptr)
        {
            line_textures.push_back(line_texture);
            line_rects.push_back(line_rect);
            overlay_height += line_rect.h;
            if (static_cast<int>(line_rect.w) > max_content_width)
                max_content_width = line_rect.w;
            if (index + 1U < overlay->lines.size())
                overlay_height += line_spacing;
        }
    }

    SDL_Texture *footer_texture = ft_nullptr;
    SDL_Rect     footer_rect;
    footer_rect.x = 0;
    footer_rect.y = 0;
    footer_rect.w = 0;
    footer_rect.h = 0;

    if (!overlay->footer.empty())
    {
        SDL_Color footer_color = palette.overlay_footer;
        footer_texture = create_text_texture(renderer, *menu_font, overlay->footer, footer_color, footer_rect);
        if (footer_texture != ft_nullptr)
        {
            overlay_height += footer_rect.h;
            if (static_cast<int>(footer_rect.w) > max_content_width)
                max_content_width = footer_rect.w;
            overlay_height += 12;
        }
    }

    if (heading_texture == ft_nullptr && line_textures.empty() && footer_texture == ft_nullptr)
        return;

    int overlay_width = overlay_padding * 2 + max_content_width;
    if (overlay_width < min_overlay_width)
        overlay_width = min_overlay_width;
    const int available_width = output_width - 2 * overlay_padding;
    if (available_width > 0 && overlay_width > available_width)
        overlay_width = available_width;
    if (overlay_width <= 0)
        overlay_width = min_overlay_width;

    SDL_Rect overlay_rect;
    overlay_rect.w = overlay_width;
    overlay_rect.h = overlay_height;
    overlay_rect.x = output_width / 2 - overlay_rect.w / 2;
    overlay_rect.y = output_height / 2 - overlay_rect.h / 2;
    if (overlay_rect.x < 48)
        overlay_rect.x = 48;
    if (overlay_rect.y < 48)
        overlay_rect.y = 48;

    SDL_SetRenderDrawColor(&renderer, palette.overlay_background.r, palette.overlay_background.g,
        palette.overlay_background.b, palette.overlay_background.a);
    SDL_RenderFillRect(&renderer, &overlay_rect);
    SDL_SetRenderDrawColor(&renderer, palette.overlay_border.r, palette.overlay_border.g, palette.overlay_border.b,
        palette.overlay_border.a);
    SDL_RenderDrawRect(&renderer, &overlay_rect);

    int text_x = overlay_rect.x + overlay_padding;
    int text_y = overlay_rect.y + overlay_padding;

    if (heading_texture != ft_nullptr)
    {
        heading_rect.x = text_x;
        heading_rect.y = text_y;
        SDL_RenderCopy(&renderer, heading_texture, ft_nullptr, &heading_rect);
        text_y += heading_rect.h + 12;
    }

    for (size_t index = 0; index < line_textures.size(); ++index)
    {
        SDL_Texture *line_texture = line_textures[index];
        SDL_Rect      line_rect = line_rects[index];
        line_rect.x = text_x;
        line_rect.y = text_y;
        if (line_texture != ft_nullptr)
            SDL_RenderCopy(&renderer, line_texture, ft_nullptr, &line_rect);
        text_y += line_rect.h;
        if (index + 1U < line_textures.size())
            text_y += line_spacing;
        destroy_texture(line_texture);
    }

    if (footer_texture != ft_nullptr)
    {
        footer_rect.x = text_x;
        footer_rect.y = text_y + 12;
        SDL_RenderCopy(&renderer, footer_texture, ft_nullptr, &footer_rect);
    }

    destroy_texture(footer_texture);
    destroy_texture(heading_texture);
#else
    (void)renderer;
    (void)menu_font;
    (void)output_width;
    (void)output_height;
    (void)overlay;
    (void)palette;
#endif
}

