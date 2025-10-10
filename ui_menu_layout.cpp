#include "ui_menu.hpp"

void ft_ui_menu::set_viewport_bounds(const ft_rect &viewport)
{
    this->_viewport_bounds = viewport;
    this->update_scroll_limits();
    this->scroll_to_index(this->_selected_index);
}

int ft_ui_menu::find_item_at(int x, int y) const
{
    if (this->_viewport_bounds.width > 0 && this->_viewport_bounds.height > 0)
    {
        const int viewport_right = this->_viewport_bounds.left + this->_viewport_bounds.width;
        const int viewport_bottom = this->_viewport_bounds.top + this->_viewport_bounds.height;

        if (x < this->_viewport_bounds.left || x >= viewport_right)
            return -1;
        if (y < this->_viewport_bounds.top || y >= viewport_bottom)
            return -1;
    }

    const int adjusted_y = y + this->_scroll_offset;

    for (size_t index = 0; index < this->_items.size(); ++index)
    {
        if (this->_items[index].bounds.contains(x, adjusted_y))
            return static_cast<int>(index);
    }
    return -1;
}

void ft_ui_menu::recalculate_content_bounds()
{
    if (this->_items.empty())
    {
        this->_content_bounds = ft_rect();
        return;
    }

    int min_left = this->_items[0].bounds.left;
    int min_top = this->_items[0].bounds.top;
    int max_right = this->_items[0].bounds.left + this->_items[0].bounds.width;
    int max_bottom = this->_items[0].bounds.top + this->_items[0].bounds.height;

    for (size_t index = 1; index < this->_items.size(); ++index)
    {
        const ft_rect &bounds = this->_items[index].bounds;

        if (bounds.left < min_left)
            min_left = bounds.left;

        const int right = bounds.left + bounds.width;
        if (right > max_right)
            max_right = right;

        if (bounds.top < min_top)
            min_top = bounds.top;

        const int bottom = bounds.top + bounds.height;
        if (bottom > max_bottom)
            max_bottom = bottom;
    }

    this->_content_bounds.left = min_left;
    this->_content_bounds.top = min_top;
    this->_content_bounds.width = max_right - min_left;
    this->_content_bounds.height = max_bottom - min_top;
}

void ft_ui_menu::update_scroll_limits()
{
    if (this->_viewport_bounds.width <= 0 || this->_viewport_bounds.height <= 0)
    {
        this->_max_scroll_offset = 0;
        this->_scroll_offset = 0;
        return;
    }

    const int viewport_bottom = this->_viewport_bounds.top + this->_viewport_bounds.height;
    const int content_bottom = this->_content_bounds.top + this->_content_bounds.height;

    int overflow = content_bottom - viewport_bottom;
    if (overflow < 0)
        overflow = 0;

    this->_max_scroll_offset = overflow;
    this->clamp_scroll_offset();
}

void ft_ui_menu::clamp_scroll_offset()
{
    if (this->_scroll_offset < 0)
        this->_scroll_offset = 0;
    if (this->_scroll_offset > this->_max_scroll_offset)
        this->_scroll_offset = this->_max_scroll_offset;
}

void ft_ui_menu::scroll_to_index(int index)
{
    if (index < 0)
    {
        this->clamp_scroll_offset();
        return;
    }

    const size_t converted = static_cast<size_t>(index);
    if (converted >= this->_items.size())
    {
        this->clamp_scroll_offset();
        return;
    }

    if (this->_viewport_bounds.height <= 0)
    {
        this->clamp_scroll_offset();
        return;
    }

    if (this->_max_scroll_offset <= 0)
    {
        this->_scroll_offset = 0;
        return;
    }

    const int viewport_top = this->_viewport_bounds.top;
    const int viewport_bottom = viewport_top + this->_viewport_bounds.height;

    const ft_rect &bounds = this->_items[converted].bounds;

    int visible_top = bounds.top - this->_scroll_offset;
    int visible_bottom = visible_top + bounds.height;

    if (visible_top < viewport_top)
        this->_scroll_offset -= (viewport_top - visible_top);
    else if (visible_bottom > viewport_bottom)
        this->_scroll_offset += (visible_bottom - viewport_bottom);

    this->clamp_scroll_offset();
}

