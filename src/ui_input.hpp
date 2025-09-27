#ifndef UI_INPUT_HPP
#define UI_INPUT_HPP

enum e_ft_input_device
{
    FT_INPUT_DEVICE_NONE = 0,
    FT_INPUT_DEVICE_MOUSE,
    FT_INPUT_DEVICE_KEYBOARD
};

struct ft_mouse_state
{
    int  x;
    int  y;
    bool moved;
    bool left_pressed;
    bool left_released;

    ft_mouse_state() noexcept;
    bool has_activity() const noexcept;
};

struct ft_keyboard_state
{
    bool pressed_up;
    bool pressed_down;

    ft_keyboard_state() noexcept;
    bool has_activity() const noexcept;
};

class ft_input_device_tracker
{
private:
    e_ft_input_device   _last_device;
    unsigned long long  _event_counter;

public:
    ft_input_device_tracker() noexcept;

    void                 notify_mouse_activity() noexcept;
    void                 notify_keyboard_activity() noexcept;
    e_ft_input_device    get_last_device() const noexcept;
    unsigned long long   get_event_counter() const noexcept;
    void                 reset() noexcept;
};

#endif
