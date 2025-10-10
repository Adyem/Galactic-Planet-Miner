#include "ui_menu.hpp"

ft_rect::ft_rect() noexcept
    : left(0), top(0), width(0), height(0)
{}

ft_rect::ft_rect(int left_value, int top_value, int width_value, int height_value) noexcept
    : left(left_value), top(top_value), width(width_value), height(height_value)
{}

bool ft_rect::contains(int point_x, int point_y) const noexcept
{
    if (point_x < this->left || point_y < this->top)
        return false;

    const long long right_edge = static_cast<long long>(this->left) + static_cast<long long>(this->width);
    const long long bottom_edge = static_cast<long long>(this->top) + static_cast<long long>(this->height);

    if (point_x >= right_edge || point_y >= bottom_edge)
        return false;
    return true;
}

ft_menu_item::ft_menu_item()
    : identifier(), label(), bounds(), enabled(true), description()
{}

ft_menu_item::ft_menu_item(const ft_string &id, const ft_string &text, const ft_rect &area)
    : identifier(id), label(text), bounds(area), enabled(true), description()
{}

ft_ui_menu::ft_ui_menu()
    : _items()
    , _hovered_index(-1)
    , _selected_index(-1)
    , _device_tracker()
    , _viewport_bounds()
    , _content_bounds()
    , _scroll_offset(0)
    , _max_scroll_offset(0)
{}

void ft_ui_menu::clear()
{
    this->_items.clear();
    this->_hovered_index = -1;
    this->_selected_index = -1;
    this->_device_tracker.reset();
    this->_viewport_bounds = ft_rect();
    this->_content_bounds = ft_rect();
    this->_scroll_offset = 0;
    this->_max_scroll_offset = 0;
}

void ft_ui_menu::set_items(const ft_vector<ft_menu_item> &items)
{
    this->_items.clear();
    size_t count = items.size();
    if (count > 0)
    {
        this->_items.reserve(count);
        for (size_t i = 0; i < count; ++i)
            this->_items.push_back(items[i]);
    }
    this->_hovered_index = -1;
    this->_selected_index = this->find_first_enabled_index();
    this->_device_tracker.reset();
    this->recalculate_content_bounds();
    if (this->_viewport_bounds.width <= 0 || this->_viewport_bounds.height <= 0)
        this->_viewport_bounds = this->_content_bounds;
    this->update_scroll_limits();
    this->scroll_to_index(this->_selected_index);
}

void ft_ui_menu::add_item(const ft_menu_item &item)
{
    this->_items.push_back(item);
    if (this->_selected_index < 0 && item.enabled)
    {
        this->_selected_index = static_cast<int>(this->_items.size()) - 1;
        this->_hovered_index = this->_selected_index;
    }
    this->recalculate_content_bounds();
    if (this->_viewport_bounds.width <= 0 || this->_viewport_bounds.height <= 0)
        this->_viewport_bounds = this->_content_bounds;
    this->update_scroll_limits();
    this->scroll_to_index(this->_selected_index);
}

const ft_vector<ft_menu_item> &ft_ui_menu::get_items() const noexcept
{
    return this->_items;
}

int ft_ui_menu::get_hovered_index() const noexcept
{
    return this->_hovered_index;
}

int ft_ui_menu::get_selected_index() const noexcept
{
    return this->_selected_index;
}

const ft_menu_item *ft_ui_menu::get_selected_item() const
{
    if (this->_selected_index < 0)
        return nullptr;

    const size_t index = static_cast<size_t>(this->_selected_index);
    if (index >= this->_items.size())
        return nullptr;

    return &this->_items[index];
}

const ft_menu_item *ft_ui_menu::get_hovered_item() const
{
    if (this->_hovered_index < 0)
        return nullptr;

    const size_t index = static_cast<size_t>(this->_hovered_index);
    if (index >= this->_items.size())
        return nullptr;

    return &this->_items[index];
}

e_ft_input_device ft_ui_menu::get_active_device() const noexcept
{
    return this->_device_tracker.get_last_device();
}

void ft_ui_menu::set_active_device(e_ft_input_device device) noexcept
{
    this->_device_tracker.set_last_device(device);
}

void ft_ui_menu::set_selected_index(int index)
{
    if (index < 0)
    {
        this->_selected_index = -1;
        this->_hovered_index = -1;
        return;
    }

    const size_t candidate = static_cast<size_t>(index);
    if (candidate >= this->_items.size())
        return;

    if (!this->_items[candidate].enabled)
        return;

    this->_selected_index = static_cast<int>(candidate);
    this->_hovered_index = static_cast<int>(candidate);
    this->scroll_to_index(this->_selected_index);
}

const ft_rect &ft_ui_menu::get_viewport_bounds() const noexcept
{
    return this->_viewport_bounds;
}

int ft_ui_menu::get_scroll_offset() const noexcept
{
    return this->_scroll_offset;
}

bool ft_ui_menu::is_scrolling_enabled() const noexcept
{
    return this->_max_scroll_offset > 0;
}

