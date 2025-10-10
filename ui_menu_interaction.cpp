#include "ui_menu.hpp"

void ft_ui_menu::handle_mouse_input(const ft_mouse_state &mouse)
{
    if (!mouse.has_activity())
        return;

    this->_device_tracker.notify_mouse_activity();

    const int hovered = this->find_item_at(mouse.x, mouse.y);
    this->_hovered_index = hovered;

    if (hovered >= 0 && mouse.left_pressed)
    {
        const size_t index = static_cast<size_t>(hovered);
        if (index < this->_items.size() && this->_items[index].enabled)
        {
            this->_selected_index = hovered;
            this->scroll_to_index(this->_selected_index);
        }
    }
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
    this->scroll_to_index(this->_selected_index);
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
        start_index = direction > 0 ? -1 : static_cast<int>(count);

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
            this->scroll_to_index(candidate);
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

