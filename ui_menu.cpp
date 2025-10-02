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
    : identifier(), label(), bounds(), enabled(true)
{}

ft_menu_item::ft_menu_item(const ft_string &id, const ft_string &text, const ft_rect &area)
    : identifier(id), label(text), bounds(area), enabled(true)
{}

ft_ui_menu::ft_ui_menu()
    : _items(), _hovered_index(-1), _selected_index(-1), _device_tracker()
{}

void ft_ui_menu::clear()
{
    this->_items.clear();
    this->_hovered_index = -1;
    this->_selected_index = -1;
    this->_device_tracker.reset();
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
}

void ft_ui_menu::add_item(const ft_menu_item &item)
{
    this->_items.push_back(item);
    if (this->_selected_index < 0 && item.enabled)
    {
        this->_selected_index = static_cast<int>(this->_items.size()) - 1;
        this->_hovered_index = this->_selected_index;
    }
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

e_ft_input_device ft_ui_menu::get_active_device() const noexcept
{
    return this->_device_tracker.get_last_device();
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
}

void ft_ui_menu::handle_mouse_input(const ft_mouse_state &mouse)
{
    if (!mouse.has_activity())
        return;

    this->_device_tracker.notify_mouse_activity();

    const int hovered = this->find_item_at(mouse.x, mouse.y);
    this->_hovered_index = hovered;

    if (hovered >= 0 && mouse.left_pressed)
        this->_selected_index = hovered;
}

void ft_ui_menu::handle_keyboard_input(const ft_keyboard_state &keyboard)
{
    if (!keyboard.has_activity())
        return;

    this->_device_tracker.notify_keyboard_activity();

    if (keyboard.pressed_up)
        this->move_selection(-1);

    if (keyboard.pressed_down)
        this->move_selection(1);

    this->synchronize_hover_with_selection();
}

int ft_ui_menu::find_first_enabled_index() const
{
    for (size_t index = 0; index < this->_items.size(); ++index)
    {
        if (this->_items[index].enabled)
            return static_cast<int>(index);
    }
    return -1;
}

int ft_ui_menu::find_item_at(int x, int y) const
{
    for (size_t index = 0; index < this->_items.size(); ++index)
    {
        if (!this->_items[index].enabled)
            continue;

        if (this->_items[index].bounds.contains(x, y))
            return static_cast<int>(index);
    }
    return -1;
}

void ft_ui_menu::move_selection(int direction)
{
    if (direction == 0)
        return;

    const size_t count = this->_items.size();
    if (count == 0)
    {
        this->_selected_index = -1;
        this->_hovered_index = -1;
        return;
    }

    int start_index = this->_selected_index;
    if (start_index < 0 || static_cast<size_t>(start_index) >= count)
    {
        start_index = direction > 0 ? -1 : static_cast<int>(count);
    }

    int candidate = start_index;
    for (size_t step = 0; step < count; ++step)
    {
        candidate += direction;
        if (candidate < 0)
            candidate = static_cast<int>(count) - 1;
        else if (candidate >= static_cast<int>(count))
            candidate = 0;

        if (this->_items[static_cast<size_t>(candidate)].enabled)
        {
            this->_selected_index = candidate;
            this->_hovered_index = candidate;
            return;
        }
    }
}

void ft_ui_menu::synchronize_hover_with_selection()
{
    if (this->_selected_index < 0)
    {
        this->_hovered_index = -1;
        return;
    }

    const size_t index = static_cast<size_t>(this->_selected_index);
    if (index >= this->_items.size())
    {
        this->_hovered_index = -1;
        return;
    }

    this->_hovered_index = static_cast<int>(index);
}
