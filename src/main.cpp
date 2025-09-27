#include "ui_menu.hpp"
#include "game_bootstrap.hpp"
#include "player_profile.hpp"

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

#include <cstddef>

namespace
{
    constexpr unsigned int kVirtualReferenceWidth = 1280U;
    constexpr unsigned int      kVirtualReferenceHeight = 720U;
    ft_string                    g_commander_name;
    PlayerProfilePreferences     g_active_profile;

    float compute_axis_scale(unsigned int actual_extent, unsigned int reference_extent);

    void render_name_prompt(sf::RenderWindow &window, const sf::Font &font, const ft_string &current_input)
    {
        window.clear(sf::Color::Black);

        const sf::Vector2u window_size = window.getSize();
        const float        half_width = static_cast<float>(window_size.x) / 2.0f;
        const float        half_height = static_cast<float>(window_size.y) / 2.0f;
        const float        scale_y = compute_axis_scale(window_size.y, kVirtualReferenceHeight);

        float prompt_size_f = 40.0f * scale_y;
        if (prompt_size_f < 20.0f)
            prompt_size_f = 20.0f;
        const unsigned int prompt_size = static_cast<unsigned int>(prompt_size_f + 0.5f);

        sf::Text prompt_text;
        prompt_text.setFont(font);
        prompt_text.setString("Enter Commander Name");
        prompt_text.setCharacterSize(prompt_size);
        prompt_text.setFillColor(sf::Color::White);
        const sf::FloatRect prompt_bounds = prompt_text.getLocalBounds();
        prompt_text.setOrigin(prompt_bounds.left + prompt_bounds.width / 2.0f, prompt_bounds.top + prompt_bounds.height / 2.0f);
        prompt_text.setPosition(half_width, half_height - (80.0f * scale_y));

        sf::Text input_text;
        input_text.setFont(font);
        ft_string display_value = current_input;
        if (display_value.empty())
            display_value = ft_string("<Commander>");
        input_text.setString(display_value.c_str());
        float input_size_f = 32.0f * scale_y;
        if (input_size_f < 18.0f)
            input_size_f = 18.0f;
        input_text.setCharacterSize(static_cast<unsigned int>(input_size_f + 0.5f));
        input_text.setFillColor(sf::Color::White);
        const sf::FloatRect input_bounds = input_text.getLocalBounds();
        input_text.setOrigin(input_bounds.left + input_bounds.width / 2.0f, input_bounds.top + input_bounds.height / 2.0f);
        input_text.setPosition(half_width, half_height);

        sf::Text hint_text;
        hint_text.setFont(font);
        hint_text.setString("Press Enter to confirm");
        float hint_size_f = 20.0f * scale_y;
        if (hint_size_f < 14.0f)
            hint_size_f = 14.0f;
        hint_text.setCharacterSize(static_cast<unsigned int>(hint_size_f + 0.5f));
        hint_text.setFillColor(sf::Color(180, 180, 180));
        const sf::FloatRect hint_bounds = hint_text.getLocalBounds();
        hint_text.setOrigin(hint_bounds.left + hint_bounds.width / 2.0f, hint_bounds.top + hint_bounds.height / 2.0f);
        hint_text.setPosition(half_width, half_height + (60.0f * scale_y));

        window.draw(prompt_text);
        window.draw(input_text);
        window.draw(hint_text);
        window.display();
    }

    ft_string prompt_for_commander_name(sf::RenderWindow &window, const sf::Font &font)
    {
        ft_string input_value;

        while (window.isOpen())
        {
            bool confirm_requested = false;

            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }
                else if (event.type == sf::Event::Resized)
                {
                    unsigned int width = event.size.width;
                    unsigned int height = event.size.height;

                    if (width == 0U)
                        width = 1U;
                    if (height == 0U)
                        height = 1U;

                    const sf::Vector2u adjusted_size(width, height);
                    sf::View            updated_view(sf::FloatRect(0.0f, 0.0f, static_cast<float>(adjusted_size.x), static_cast<float>(adjusted_size.y)));
                    window.setView(updated_view);
                }
                else if (event.type == sf::Event::TextEntered)
                {
                    const sf::Uint32 code_point = event.text.unicode;
                    if (code_point == 8U)
                    {
                        if (!input_value.empty())
                            input_value.erase(input_value.size() - 1U, 1U);
                    }
                    else if (code_point == 13U)
                    {
                        confirm_requested = true;
                    }
                    else if (code_point >= 32U && code_point < 127U)
                    {
                        input_value.append(static_cast<char>(code_point));
                    }
                }
                else if (event.type == sf::Event::KeyPressed)
                {
                    if (event.key.code == sf::Keyboard::Escape)
                        window.close();
                    else if (event.key.code == sf::Keyboard::Return)
                        confirm_requested = true;
                    else if (event.key.code == sf::Keyboard::BackSpace)
                    {
                        if (!input_value.empty())
                            input_value.erase(input_value.size() - 1U, 1U);
                    }
                }
            }

            if (!window.isOpen())
                break;

            if (confirm_requested)
            {
                if (input_value.empty())
                    return ft_string("Commander");
                return input_value;
            }

            render_name_prompt(window, font, input_value);
        }

        return ft_string();
    }

    float compute_axis_scale(unsigned int actual_extent, unsigned int reference_extent)
    {
        if (reference_extent == 0U)
            return 1.0f;
        return static_cast<float>(actual_extent) / static_cast<float>(reference_extent);
    }

    ft_rect scale_virtual_rect(const ft_rect &virtual_rect, const sf::Vector2u &window_size)
    {
        const float scale_x = compute_axis_scale(window_size.x, kVirtualReferenceWidth);
        const float scale_y = compute_axis_scale(window_size.y, kVirtualReferenceHeight);

        const float scaled_left_f = static_cast<float>(virtual_rect.left) * scale_x;
        const float scaled_top_f = static_cast<float>(virtual_rect.top) * scale_y;
        const float scaled_width_f = static_cast<float>(virtual_rect.width) * scale_x;
        const float scaled_height_f = static_cast<float>(virtual_rect.height) * scale_y;

        int scaled_left = static_cast<int>(scaled_left_f + 0.5f);
        int scaled_top = static_cast<int>(scaled_top_f + 0.5f);
        int scaled_width = static_cast<int>(scaled_width_f + 0.5f);
        int scaled_height = static_cast<int>(scaled_height_f + 0.5f);

        if (scaled_width <= 0)
            scaled_width = 1;
        if (scaled_height <= 0)
            scaled_height = 1;

        return ft_rect(scaled_left, scaled_top, scaled_width, scaled_height);
    }

    ft_vector<ft_menu_item> build_main_menu_items(const sf::Vector2u &window_size)
    {
        const int menu_item_width = 360;
        const int menu_item_height = 56;
        const int menu_item_spacing = 22;
        const int menu_origin_y = 220;
        const int menu_origin_x = static_cast<int>(kVirtualReferenceWidth / 2U) - (menu_item_width / 2);

        const char *identifiers[] = {"new_game", "load", "settings", "swap_profile", "exit"};
        const char *labels[] = {"New Game", "Load", "Settings", "Swap Profile", "Exit"};

        ft_vector<ft_menu_item> items;
        items.reserve(5U);

        for (int index = 0; index < 5; ++index)
        {
            const int top = menu_origin_y + index * (menu_item_height + menu_item_spacing);
            const ft_rect virtual_rect(menu_origin_x, top, menu_item_width, menu_item_height);
            const ft_rect scaled_rect = scale_virtual_rect(virtual_rect, window_size);

            items.push_back(ft_menu_item(
                ft_string(identifiers[index]),
                ft_string(labels[index]),
                scaled_rect));
        }

        return items;
    }

    void update_menu_for_window_size(ft_ui_menu &menu, const sf::Vector2u &window_size)
    {
        const ft_menu_item *selected_item = menu.get_selected_item();
        bool                 had_selection = (selected_item != nullptr);
        ft_string            selected_identifier;

        if (had_selection)
            selected_identifier = selected_item->identifier;

        ft_vector<ft_menu_item> scaled_items = build_main_menu_items(window_size);
        menu.set_items(scaled_items);

        if (!had_selection)
            return;

        const ft_vector<ft_menu_item> &items = menu.get_items();
        for (size_t index = 0; index < items.size(); ++index)
        {
            if (items[index].identifier == selected_identifier)
            {
                menu.set_selected_index(static_cast<int>(index));
                break;
            }
        }
    }

    void apply_profile_window_preferences(sf::RenderWindow &window, ft_ui_menu &menu)
    {
        unsigned int desired_width = g_active_profile.window_width;
        unsigned int desired_height = g_active_profile.window_height;

        const sf::Vector2u current_size = window.getSize();
        if (desired_width == 0U)
            desired_width = current_size.x != 0U ? current_size.x : 1280U;
        if (desired_height == 0U)
            desired_height = current_size.y != 0U ? current_size.y : 720U;

        sf::Vector2u desired_size(desired_width, desired_height);
        if (desired_size.x == 0U)
            desired_size.x = 1U;
        if (desired_size.y == 0U)
            desired_size.y = 1U;

        if (desired_size != current_size)
        {
            window.setSize(desired_size);
            sf::View updated_view(sf::FloatRect(0.0f, 0.0f, static_cast<float>(desired_size.x), static_cast<float>(desired_size.y)));
            window.setView(updated_view);
        }

        update_menu_for_window_size(menu, window.getSize());
        const sf::Vector2u applied_size = window.getSize();
        g_active_profile.window_width = applied_size.x;
        g_active_profile.window_height = applied_size.y;
    }

    bool load_profile_for_commander(const ft_string &commander_name, sf::RenderWindow &window, ft_ui_menu &menu)
    {
        PlayerProfilePreferences loaded_profile;
        if (!player_profile_load_or_create(loaded_profile, commander_name))
        {
            g_commander_name = commander_name;
            g_active_profile = PlayerProfilePreferences();
            g_active_profile.commander_name = g_commander_name;
            apply_profile_window_preferences(window, menu);
            return false;
        }

        g_active_profile = loaded_profile;
        if (g_active_profile.commander_name.empty())
            g_active_profile.commander_name = commander_name;
        g_commander_name = g_active_profile.commander_name;
        if (g_commander_name.empty())
            g_commander_name = commander_name;

        apply_profile_window_preferences(window, menu);
        return true;
    }

    void persist_active_profile()
    {
        if (g_commander_name.empty())
            return;
        g_active_profile.commander_name = g_commander_name;
        player_profile_save(g_active_profile);
    }

    const sf::Font &resolve_menu_font(const sf::Font *provided_font)
    {
#if FT_SFML_HAS_DEFAULT_FONT
        if (provided_font == ft_nullptr)
            return sf::Font::getDefaultFont();

        const sf::Font::Info font_info = provided_font->getInfo();
        if (font_info.family.empty())
            return sf::Font::getDefaultFont();

        return *provided_font;
#else
        if (provided_font != ft_nullptr)
        {
            const sf::Font::Info font_info = provided_font->getInfo();
            if (!font_info.family.empty())
                return *provided_font;
        }

        static sf::Font fallback_font;
        static bool     fallback_attempted = false;

        if (!fallback_attempted)
        {
            fallback_attempted = true;

            const char *paths[] = {
                "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
                "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
                "/System/Library/Fonts/SFNSDisplay.ttf",
                "/System/Library/Fonts/SFNS.ttf",
                "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
                "/Library/Fonts/Arial.ttf",
                "C:\\Windows\\Fonts\\segoeui.ttf",
                "C:\\Windows\\Fonts\\arial.ttf",
                "C:\\Windows\\Fonts\\calibri.ttf"
            };

            const size_t path_count = sizeof(paths) / sizeof(paths[0]);
            bool        loaded = false;
            for (size_t index = 0; index < path_count; ++index)
            {
                if (fallback_font.loadFromFile(paths[index]))
                {
                    loaded = true;
                    break;
                }
            }

            if (!loaded)
            {
                const char *windows_directory = ft_getenv("WINDIR");
                if (windows_directory != ft_nullptr && windows_directory[0] != '\0')
                {
                    ft_string fonts_directory(windows_directory);
                    const size_t dir_length = fonts_directory.size();
                    if (dir_length > 0U)
                    {
                        const char last_character = fonts_directory[dir_length - 1U];
                        if (last_character != '\\' && last_character != '/')
                            fonts_directory.append("\\");
                    }
                    fonts_directory.append("Fonts\\");

                    const char *windows_fonts[] = {"segoeui.ttf", "arial.ttf", "calibri.ttf"};
                    const size_t windows_font_count = sizeof(windows_fonts) / sizeof(windows_fonts[0]);
                    for (size_t index = 0; index < windows_font_count; ++index)
                    {
                        ft_string candidate = fonts_directory;
                        candidate.append(windows_fonts[index]);
                        if (fallback_font.loadFromFile(candidate.c_str()))
                        {
                            loaded = true;
                            break;
                        }
                    }
                }
            }
        }

        return fallback_font;
#endif
    }

    void render_menu(sf::RenderWindow &window, const ft_ui_menu &menu, const sf::Font &font, const sf::Vector2u &window_size)
    {
        window.clear(sf::Color::Black);

        const ft_vector<ft_menu_item> &items = menu.get_items();
        const int hovered_index = menu.get_hovered_index();
        const int selected_index = menu.get_selected_index();

        const float scale_y = compute_axis_scale(window_size.y, kVirtualReferenceHeight);
        float       character_size_f = 32.0f * scale_y;
        if (character_size_f < 18.0f)
            character_size_f = 18.0f;
        const unsigned int character_size = static_cast<unsigned int>(character_size_f + 0.5f);

        const size_t count = items.size();
        for (size_t index = 0; index < count; ++index)
        {
            const ft_menu_item &item = items[index];

            sf::RectangleShape background_shape;
            background_shape.setPosition(static_cast<float>(item.bounds.left), static_cast<float>(item.bounds.top));
            background_shape.setSize(sf::Vector2f(static_cast<float>(item.bounds.width), static_cast<float>(item.bounds.height)));

            const bool is_selected = static_cast<int>(index) == selected_index;
            const bool is_hovered = static_cast<int>(index) == hovered_index;

            if (is_selected)
                background_shape.setFillColor(sf::Color(60, 60, 60));
            else if (is_hovered)
                background_shape.setFillColor(sf::Color(40, 40, 40));
            else
                background_shape.setFillColor(sf::Color(20, 20, 20));

            window.draw(background_shape);

            sf::Text label;
            label.setFont(font);
            label.setString(item.label.c_str());
            label.setCharacterSize(character_size);
            label.setFillColor(sf::Color::White);

            const sf::FloatRect bounds = label.getLocalBounds();
            const float origin_x = bounds.left + (bounds.width / 2.0f);
            const float origin_y = bounds.top + (bounds.height / 2.0f);
            label.setOrigin(origin_x, origin_y);

            const float position_x = static_cast<float>(item.bounds.left) + static_cast<float>(item.bounds.width) / 2.0f;
            const float position_y = static_cast<float>(item.bounds.top) + static_cast<float>(item.bounds.height) / 2.0f;
            label.setPosition(position_x, position_y);

            window.draw(label);
        }

        window.display();
    }

    const char *kQuickSavePath = "quicksave.json";

    const ft_menu_item *menu_item_from_index(const ft_ui_menu &menu, int index)
    {
        if (index < 0)
            return ft_nullptr;

        const ft_vector<ft_menu_item> &items = menu.get_items();
        const size_t unsigned_index = static_cast<size_t>(index);
        if (unsigned_index >= items.size())
            return ft_nullptr;

        return &items[unsigned_index];
    }

    void handle_menu_activation(const ft_menu_item &item, sf::RenderWindow &window, ft_ui_menu &menu, const sf::Font &font)
    {
        if (item.identifier == "exit")
        {
            window.close();
            return;
        }

        if (item.identifier == "new_game")
        {
            if (g_commander_name.empty())
                g_commander_name = ft_string("Commander");
            game_bootstrap_create_quicksave_with_commander(kQuickSavePath, g_commander_name);
            return;
        }

        if (item.identifier == "swap_profile")
        {
            ft_string new_commander = prompt_for_commander_name(window, font);
            if (!window.isOpen())
                return;
            if (new_commander.empty())
                new_commander = ft_string("Commander");
            if (new_commander == g_commander_name)
                return;
            load_profile_for_commander(new_commander, window, menu);
            persist_active_profile();
            return;
        }
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280U, 720U), "Galactic Planet Miner", sf::Style::Default);
    window.setVerticalSyncEnabled(true);
    window.setKeyRepeatEnabled(false);

    ft_ui_menu menu;
    update_menu_for_window_size(menu, window.getSize());

    const sf::Font &menu_font = resolve_menu_font(ft_nullptr);

    if (window.isOpen())
    {
        g_commander_name = prompt_for_commander_name(window, menu_font);
        if (!window.isOpen())
            return 0;
        if (g_commander_name.empty())
            g_commander_name = ft_string("Commander");
        load_profile_for_commander(g_commander_name, window, menu);
        persist_active_profile();
    }

    while (window.isOpen())
    {
        ft_mouse_state mouse_state;
        ft_keyboard_state keyboard_state;
        bool confirm_pressed = false;

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
            else if (event.type == sf::Event::Resized)
            {
                unsigned int width = event.size.width;
                unsigned int height = event.size.height;

                if (width == 0U)
                    width = 1U;
                if (height == 0U)
                    height = 1U;

                const sf::Vector2u adjusted_size(width, height);
                sf::View            updated_view(sf::FloatRect(0.0f, 0.0f, static_cast<float>(adjusted_size.x), static_cast<float>(adjusted_size.y)));
                window.setView(updated_view);
                update_menu_for_window_size(menu, adjusted_size);
                g_active_profile.window_width = adjusted_size.x;
                g_active_profile.window_height = adjusted_size.y;
                persist_active_profile();
            }
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Up)
                    keyboard_state.pressed_up = true;
                else if (event.key.code == sf::Keyboard::Down)
                    keyboard_state.pressed_down = true;
                else if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space)
                    confirm_pressed = true;
                else if (event.key.code == sf::Keyboard::Escape)
                    window.close();
            }
        }

        if (!mouse_state.moved)
        {
            const sf::Vector2i cursor = sf::Mouse::getPosition(window);
            mouse_state.x = cursor.x;
            mouse_state.y = cursor.y;
        }

        menu.handle_mouse_input(mouse_state);
        menu.handle_keyboard_input(keyboard_state);

        if (mouse_state.left_pressed)
        {
            const int hovered_index = menu.get_hovered_index();
            const ft_menu_item *hovered_item = menu_item_from_index(menu, hovered_index);
            if (hovered_item != ft_nullptr)
                handle_menu_activation(*hovered_item, window, menu, menu_font);
        }

        if (confirm_pressed)
        {
            const ft_menu_item *selected_item = menu.get_selected_item();
            if (selected_item != ft_nullptr)
                handle_menu_activation(*selected_item, window, menu, menu_font);
        }

        render_menu(window, menu, menu_font, window.getSize());
    }

    return 0;
}
