#include "main_menu_system.hpp"

#include "menu_localization.hpp"

void main_menu_render_tutorial_overlay(SDL_Renderer &renderer, TTF_Font *menu_font, int output_width,
    const MainMenuTutorialContext *tutorial, const MainMenuPalette &palette)
{
#if GALACTIC_HAVE_SDL2
    if (tutorial == ft_nullptr || !tutorial->visible)
        return;
    if (menu_font == ft_nullptr)
        return;

    const ft_vector<ft_string> *tips_pointer = tutorial->tips;
    if (tips_pointer == ft_nullptr)
        tips_pointer = &get_main_menu_tutorial_tips();

    const ft_vector<ft_string> &tips = *tips_pointer;
    if (tips.empty())
        return;

    const int overlay_padding = 20;
    const int min_overlay_width = 540;
    int       overlay_height = overlay_padding * 2;
    int       max_content_width = 0;

    SDL_Texture *title_texture = ft_nullptr;
    SDL_Rect     title_rect;
    title_rect.x = 0;
    title_rect.y = 0;
    title_rect.w = 0;
    title_rect.h = 0;

    ft_string title_label = menu_localize("main_menu.tutorial.title", "Commander Tips");
    if (!title_label.empty())
    {
        SDL_Color title_color = palette.tutorial_title;
        title_texture = create_text_texture(renderer, *menu_font, title_label, title_color, title_rect);
        if (title_texture != ft_nullptr)
        {
            overlay_height += title_rect.h;
            max_content_width = title_rect.w;
            overlay_height += 18;
        }
    }

    ft_vector<SDL_Texture *> tip_textures;
    ft_vector<SDL_Rect>      tip_rects;
    tip_textures.reserve(tips.size());
    tip_rects.reserve(tips.size());

    SDL_Color primary_tip_color = palette.tutorial_primary;
    SDL_Color bullet_tip_color = palette.tutorial_secondary;

    for (size_t index = 0; index < tips.size(); ++index)
    {
        bool is_primary_tip = index == 0;
        ft_string display_text;
        if (is_primary_tip)
            display_text = tips[index];
        else
        {
            display_text = ft_string("\xE2\x80\xA2 ");
            display_text.append(tips[index]);
        }

        SDL_Rect tip_rect;
        SDL_Color tip_color = is_primary_tip ? primary_tip_color : bullet_tip_color;
        SDL_Texture *tip_texture = create_text_texture(renderer, *menu_font, display_text, tip_color, tip_rect);
        if (tip_texture != ft_nullptr)
        {
            tip_textures.push_back(tip_texture);
            tip_rects.push_back(tip_rect);
            overlay_height += tip_rect.h;
            if (static_cast<int>(tip_rect.w) > max_content_width)
                max_content_width = tip_rect.w;
            if (index + 1U < tips.size())
                overlay_height += 8;
        }
    }

    if (tip_textures.empty() && title_texture == ft_nullptr)
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
    overlay_rect.x = 72;
    overlay_rect.y = 84;
    overlay_rect.w = overlay_width;
    overlay_rect.h = overlay_height;

    SDL_SetRenderDrawColor(&renderer, palette.tutorial_background.r, palette.tutorial_background.g,
        palette.tutorial_background.b, palette.tutorial_background.a);
    SDL_RenderFillRect(&renderer, &overlay_rect);
    SDL_SetRenderDrawColor(&renderer, palette.tutorial_border.r, palette.tutorial_border.g, palette.tutorial_border.b,
        palette.tutorial_border.a);
    SDL_RenderDrawRect(&renderer, &overlay_rect);

    int text_x = overlay_rect.x + overlay_padding;
    int text_y = overlay_rect.y + overlay_padding;

    if (title_texture != ft_nullptr)
    {
        title_rect.x = text_x;
        title_rect.y = text_y;
        SDL_RenderCopy(&renderer, title_texture, ft_nullptr, &title_rect);
        text_y += title_rect.h + 12;
    }

    for (size_t index = 0; index < tip_textures.size(); ++index)
    {
        SDL_Texture *tip_texture = tip_textures[index];
        SDL_Rect      tip_rect = tip_rects[index];
        tip_rect.x = text_x;
        tip_rect.y = text_y;
        SDL_RenderCopy(&renderer, tip_texture, ft_nullptr, &tip_rect);
        text_y += tip_rect.h;
        if (index + 1U < tip_textures.size())
            text_y += 8;
        SDL_DestroyTexture(tip_texture);
    }

    if (title_texture != ft_nullptr)
        SDL_DestroyTexture(title_texture);
#else
    (void)renderer;
    (void)menu_font;
    (void)output_width;
    (void)tutorial;
    (void)palette;
#endif
}

