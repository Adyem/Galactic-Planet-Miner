#ifndef UI_MENU_HPP
#define UI_MENU_HPP

#include "ui_input.hpp"
#include "libft/Template/vector.hpp"
#include "libft/CPP_class/class_string_class.hpp"

struct ft_rect
{
    int left;
    int top;
    int width;
    int height;

    ft_rect() noexcept;
    ft_rect(int left_value, int top_value, int width_value, int height_value) noexcept;
    bool contains(int point_x, int point_y) const noexcept;
};

struct ft_menu_item
{
    ft_string identifier;
    ft_string label;
    ft_rect   bounds;
    bool      enabled;
    ft_string description;

    ft_menu_item();
    ft_menu_item(const ft_string &id, const ft_string &text, const ft_rect &area);
};

class ft_ui_menu
{
private:
    ft_vector<ft_menu_item> _items;
    int                     _hovered_index;
    int                     _selected_index;
    ft_input_device_tracker _device_tracker;
    ft_rect                 _viewport_bounds;
    ft_rect                 _content_bounds;
    int                     _scroll_offset;
    int                     _max_scroll_offset;

    int  find_first_enabled_index() const;
    int  find_item_at(int x, int y) const;
    void move_selection(int direction);
    void synchronize_hover_with_selection();
    void recalculate_content_bounds();
    void update_scroll_limits();
    void clamp_scroll_offset();
    void scroll_to_index(int index);

public:
    ft_ui_menu();

    void                          clear();
    void                          set_items(const ft_vector<ft_menu_item> &items);
    void                          add_item(const ft_menu_item &item);
    const ft_vector<ft_menu_item> &get_items() const noexcept;
    int                           get_hovered_index() const noexcept;
    int                           get_selected_index() const noexcept;
    const ft_menu_item           *get_selected_item() const;
    const ft_menu_item           *get_hovered_item() const;
    e_ft_input_device             get_active_device() const noexcept;
    void                          set_active_device(e_ft_input_device device) noexcept;
    void                          set_selected_index(int index);
    const ft_rect                &get_viewport_bounds() const noexcept;
    int                           get_scroll_offset() const noexcept;
    bool                          is_scrolling_enabled() const noexcept;
    void                          set_viewport_bounds(const ft_rect &viewport);

    void handle_mouse_input(const ft_mouse_state &mouse);
    void handle_keyboard_input(const ft_keyboard_state &keyboard);
};

#endif
