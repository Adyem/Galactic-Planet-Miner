#if GALACTIC_HAVE_SDL2
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
#endif
