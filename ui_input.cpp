#include "ui_input.hpp"

ft_mouse_state::ft_mouse_state() noexcept
    : x(0), y(0), moved(false), left_pressed(false), left_released(false)
{}

bool ft_mouse_state::has_activity() const noexcept
{
    return this->moved || this->left_pressed || this->left_released;
}

ft_keyboard_state::ft_keyboard_state() noexcept
    : pressed_up(false), pressed_down(false), pressed_confirm(false), pressed_delete(false)
{}

bool ft_keyboard_state::has_activity() const noexcept
{
    return this->pressed_up || this->pressed_down || this->pressed_confirm || this->pressed_delete;
}

ft_input_device_tracker::ft_input_device_tracker() noexcept
    : _last_device(FT_INPUT_DEVICE_NONE), _event_counter(0ULL)
{}

void ft_input_device_tracker::notify_mouse_activity() noexcept
{
    ++this->_event_counter;
    this->_last_device = FT_INPUT_DEVICE_MOUSE;
}

void ft_input_device_tracker::notify_keyboard_activity() noexcept
{
    ++this->_event_counter;
    this->_last_device = FT_INPUT_DEVICE_KEYBOARD;
}

e_ft_input_device ft_input_device_tracker::get_last_device() const noexcept
{
    return this->_last_device;
}

unsigned long long ft_input_device_tracker::get_event_counter() const noexcept
{
    return this->_event_counter;
}

void ft_input_device_tracker::reset() noexcept
{
    this->_last_device = FT_INPUT_DEVICE_NONE;
    this->_event_counter = 0ULL;
}
