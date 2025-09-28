#include "ui_menu.hpp"

#include "../libft/CPP_class/class_nullptr.hpp"
#include "../libft/Libft/libft.hpp"

#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#if defined(__has_include)
# if __has_include(<SFML/Graphics/DefaultFont.hpp>)
#  include <SFML/Graphics/DefaultFont.hpp>
#  define FT_SFML_HAS_DEFAULT_FONT 1
# endif
#endif

#ifndef FT_SFML_HAS_DEFAULT_FONT
# define FT_SFML_HAS_DEFAULT_FONT 0
#endif

#if !((SFML_VERSION_MAJOR > 2) || (SFML_VERSION_MAJOR == 2 && SFML_VERSION_MINOR >= 6))
#error "Galactic Planet Miner now requires SFML 2.6 or newer"
#endif

namespace
{
    constexpr unsigned int kWindowWidth = 1280U;
    constexpr unsigned int kWindowHeight = 720U;

    ft_vector<ft_menu_item> build_main_menu_items()
    {
        const ft_rect base_rect(460, 220, 360, 56);
        const int      spacing = 22;

        const char *identifiers[] = {"new_game", "load", "settings", "swap_profile", "exit"};
        const char *labels[] = {"New Game", "Load", "Settings", "Swap Profile", "Exit"};

        ft_vector<ft_menu_item> items;
        items.reserve(5U);

        for (int index = 0; index < 5; ++index)
        {
            ft_rect item_rect = base_rect;
            item_rect.top += index * (base_rect.height + spacing);
            items.push_back(ft_menu_item(
                ft_string(identifiers[index]),
                ft_string(labels[index]),
                item_rect));
        }

        return items;
    }

    const sf::Font &resolve_menu_font()
    {
#if FT_SFML_HAS_DEFAULT_FONT
        return sf::Font::getDefaultFont();
#else
        static sf::Font font;
        static bool     loaded = false;

        if (!loaded)
        {
            loaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
            if (!loaded)
                loaded = font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
        }

        return font;
#endif
    }

    const ft_menu_item *menu_item_from_index(const ft_ui_menu &menu, int index)
    {
        if (index < 0)
            return ft_nullptr;

        const ft_vector<ft_menu_item> &items = menu.get_items();
        const size_t                  size = items.size();
        const size_t                  converted_index = static_cast<size_t>(index);

        if (converted_index >= size)
            return ft_nullptr;

        return &items[converted_index];
    }

    void handle_menu_activation(const ft_menu_item &item, sf::RenderWindow &window)
    {
        if (item.identifier == "exit")
            window.close();
    }

    void render_menu(sf::RenderWindow &window, const ft_ui_menu &menu, const sf::Font &font)
    {
        window.clear(sf::Color(12, 16, 28));

        sf::Text title;
        title.setFont(font);
        title.setString("Galactic Planet Miner");
        title.setCharacterSize(48U);
        title.setFillColor(sf::Color(220, 220, 245));
        const sf::Vector2u size = window.getSize();
        title.setPosition(static_cast<float>(size.x) / 2.0f - title.getLocalBounds().width / 2.0f, 96.0f);
        window.draw(title);

        const ft_vector<ft_menu_item> &items = menu.get_items();
        const int hovered_index = menu.get_hovered_index();
        const int selected_index = menu.get_selected_index();

        for (size_t index = 0; index < items.size(); ++index)
        {
            const ft_menu_item &item = items[index];

            sf::RectangleShape button;
            button.setPosition(static_cast<float>(item.bounds.left), static_cast<float>(item.bounds.top));
            button.setSize(sf::Vector2f(static_cast<float>(item.bounds.width), static_cast<float>(item.bounds.height)));

            const bool is_hovered = static_cast<int>(index) == hovered_index;
            const bool is_selected = static_cast<int>(index) == selected_index;

            if (is_hovered)
                button.setFillColor(sf::Color(56, 84, 140));
            else if (is_selected)
                button.setFillColor(sf::Color(40, 64, 112));
            else
                button.setFillColor(sf::Color(28, 36, 60));

            button.setOutlineColor(sf::Color(90, 110, 160));
            button.setOutlineThickness(2.0f);
            window.draw(button);

            sf::Text label;
            label.setFont(font);
            label.setString(item.label.c_str());
            label.setCharacterSize(28U);
            label.setFillColor(sf::Color::White);
            const sf::FloatRect bounds = label.getLocalBounds();
            const float         label_x = static_cast<float>(item.bounds.left) + (static_cast<float>(item.bounds.width) - bounds.width) / 2.0f - bounds.left;
            const float         label_y = static_cast<float>(item.bounds.top) + (static_cast<float>(item.bounds.height) - bounds.height) / 2.0f - bounds.top;
            label.setPosition(label_x, label_y);
            window.draw(label);
        }

        window.display();
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(kWindowWidth, kWindowHeight), "Galactic Planet Miner", sf::Style::Default);
    window.setVerticalSyncEnabled(true);
    window.setKeyRepeatEnabled(false);

    ft_ui_menu menu;
    menu.set_items(build_main_menu_items());

    const sf::Font &menu_font = resolve_menu_font();

    while (window.isOpen())
    {
        ft_mouse_state    mouse_state;
        ft_keyboard_state keyboard_state;
        bool              activate_requested = false;

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            else if (event.type == sf::Event::MouseMoved)
            {
                mouse_state.moved = true;
                mouse_state.x = event.mouseMove.x;
                mouse_state.y = event.mouseMove.y;
            }
            else if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    mouse_state.left_pressed = true;
                    mouse_state.x = event.mouseButton.x;
                    mouse_state.y = event.mouseButton.y;
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    mouse_state.left_released = true;
                    mouse_state.x = event.mouseButton.x;
                    mouse_state.y = event.mouseButton.y;
                }
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Up)
                    keyboard_state.pressed_up = true;
                else if (event.key.code == sf::Keyboard::Down)
                    keyboard_state.pressed_down = true;
                else if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space)
                    activate_requested = true;
                else if (event.key.code == sf::Keyboard::Escape)
                    window.close();
            }
        }

        if (!mouse_state.moved)
        {
            const sf::Vector2i position = sf::Mouse::getPosition(window);
            mouse_state.x = position.x;
            mouse_state.y = position.y;
        }

        menu.handle_mouse_input(mouse_state);
        menu.handle_keyboard_input(keyboard_state);

        if (mouse_state.left_released)
        {
            const int index = menu.get_hovered_index();
            const ft_menu_item *hovered_item = menu_item_from_index(menu, index);
            if (hovered_item != ft_nullptr)
                handle_menu_activation(*hovered_item, window);
        }

        if (activate_requested)
        {
            const ft_menu_item *selected_item = menu.get_selected_item();
            if (selected_item != ft_nullptr)
                handle_menu_activation(*selected_item, window);
        }

        render_menu(window, menu, menu_font);
    }

    return 0;
}
