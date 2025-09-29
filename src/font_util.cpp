#include "main_menu_system.hpp"

#include "../libft/CPP_class/class_nullptr.hpp"

namespace
{
    struct font_cache_entry
    {
        int       size;
        TTF_Font *font;
        bool      attempted;
    };
}

TTF_Font *resolve_font(int point_size)
{
    static font_cache_entry font_cache[] = {
        {48, ft_nullptr, false},
        {28, ft_nullptr, false}
    };

    for (size_t index = 0; index < sizeof(font_cache) / sizeof(font_cache[0]); ++index)
    {
        font_cache_entry &entry = font_cache[index];
        if (entry.size != point_size)
            continue;

        if (!entry.attempted)
        {
            entry.attempted = true;

            const char *font_paths[] = {
                "fonts/DejaVuSans.ttf",
                "fonts/LiberationSans-Regular.ttf",
                "assets/fonts/DejaVuSans.ttf",
                "assets/fonts/LiberationSans-Regular.ttf",
                "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
                "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
                "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
                "/System/Library/Fonts/SFNSDisplay.ttf",
                "/System/Library/Fonts/Supplemental/Arial.ttf",
                "/Library/Fonts/Arial.ttf",
                "C:/Windows/Fonts/segoeui.ttf",
                "C:/Windows/Fonts/arial.ttf",
                "C:/Windows/Fonts/calibri.ttf"
            };

            const size_t path_count = sizeof(font_paths) / sizeof(font_paths[0]);
            for (size_t path_index = 0; path_index < path_count; ++path_index)
            {
                const char *path = font_paths[path_index];
                if (path == ft_nullptr)
                    continue;

                entry.font = TTF_OpenFont(path, point_size);
                if (entry.font != ft_nullptr)
                    break;
            }
        }

        return entry.font;
    }

    return ft_nullptr;
}

SDL_Texture *create_text_texture(SDL_Renderer &renderer, TTF_Font &font, const ft_string &text,
    const SDL_Color &color, SDL_Rect &out_rect)
{
    if (text.c_str() == ft_nullptr)
        return ft_nullptr;

    SDL_Surface *surface = TTF_RenderUTF8_Blended(&font, text.c_str(), color);
    if (surface == ft_nullptr)
        return ft_nullptr;

    SDL_Texture *texture = SDL_CreateTextureFromSurface(&renderer, surface);
    if (texture != ft_nullptr)
    {
        out_rect.x = 0;
        out_rect.y = 0;
        out_rect.w = surface->w;
        out_rect.h = surface->h;
    }

    SDL_FreeSurface(surface);
    return texture;
}

