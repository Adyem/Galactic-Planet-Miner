#include "main_menu_system.hpp"

#include "app_constants.hpp"
#include "build_info.hpp"

#include "libft/File/file_utils.hpp"
#include "libft/Time/time.hpp"
#include "libft/Printf/printf.hpp"

namespace
{
    struct commander_portrait_cache_entry
    {
        ft_string commander_key;
        ft_string asset_path;
        bool      attempted;
        bool      loaded;
        size_t    cached_size;

        commander_portrait_cache_entry() noexcept
            : commander_key(), asset_path(), attempted(false), loaded(false), cached_size(0)
        {}
    };

    ft_vector<commander_portrait_cache_entry> g_commander_portrait_cache;

    bool portrait_candidate_exists(const ft_vector<ft_string> &names, const ft_string &candidate) noexcept
    {
        for (size_t index = 0; index < names.size(); ++index)
        {
            if (names[index] == candidate)
                return true;
        }
        return false;
    }

    void portrait_add_candidate(ft_vector<ft_string> &names, const ft_string &candidate) noexcept
    {
        if (candidate.empty())
            return;
        if (!portrait_candidate_exists(names, candidate))
            names.push_back(candidate);
    }

    ft_string portrait_build_lowercase(const ft_string &value) noexcept
    {
        ft_string lower(value);
        char     *mutable_text = lower.print();
        if (mutable_text != ft_nullptr)
            ft_to_lower(mutable_text);
        return lower;
    }

    ft_string portrait_build_underscored(const ft_string &value) noexcept
    {
        ft_string    result;
        const char  *raw = value.c_str();
        if (raw == ft_nullptr)
            return result;
        for (size_t index = 0; raw[index] != '\0'; ++index)
        {
            const char character = raw[index];
            if (character == ' ' || character == '-' || character == '.' || character == '\\')
                result.append('_');
            else
                result.append(character);
        }
        return result;
    }

    ft_string portrait_resolve_commander_key(const ft_string &commander_name) noexcept
    {
        ft_string path = player_profile_resolve_path(commander_name);
        const char *raw_path = path.c_str();
        if (raw_path == ft_nullptr || *raw_path == '\0')
            return ft_string("Commander");

        const char *start = raw_path;
        const char *forward_slash = ft_strrchr(raw_path, '/');
        if (forward_slash != ft_nullptr && *(forward_slash + 1) != '\0')
            start = forward_slash + 1;
        const char *backslash = ft_strrchr(raw_path, '\\');
        if (backslash != ft_nullptr && *(backslash + 1) != '\0' && backslash > start)
            start = backslash + 1;

        const char *end = ft_strrchr(start, '.');
        if (end == ft_nullptr || end <= start)
            end = start + ft_strlen(start);

        ft_string key;
        while (start < end && *start != '\0')
        {
            key.append(*start);
            start += 1;
        }

        if (key.empty())
            key = ft_string("Commander");
        return key;
    }

    struct commander_portrait_resolution
    {
        ft_string commander_key;
        ft_string resolved_path;
        ft_string primary_candidate;
        bool      found_existing;

        commander_portrait_resolution() noexcept
            : commander_key(), resolved_path(), primary_candidate(), found_existing(false)
        {}
    };

    commander_portrait_resolution resolve_commander_portrait_resolution(const ft_string &commander_name) noexcept
    {
        commander_portrait_resolution result;
        result.commander_key = portrait_resolve_commander_key(commander_name);

        ft_vector<ft_string> candidates;
        portrait_add_candidate(candidates, result.commander_key);
        portrait_add_candidate(candidates, portrait_build_lowercase(result.commander_key));
        ft_string underscored = portrait_build_underscored(result.commander_key);
        portrait_add_candidate(candidates, underscored);
        portrait_add_candidate(candidates, portrait_build_lowercase(underscored));

        const char *directories[] = {"assets/portraits", "portraits", "data/portraits", "data/assets/portraits"};
        const size_t directory_count = sizeof(directories) / sizeof(directories[0]);
        const char *extensions[] = {".png", ".jpg", ".jpeg", ".bmp"};
        const size_t extension_count = sizeof(extensions) / sizeof(extensions[0]);

        bool primary_set = false;
        for (size_t dir_index = 0; dir_index < directory_count; ++dir_index)
        {
            const char *directory = directories[dir_index];
            if (directory == ft_nullptr)
                continue;
            for (size_t name_index = 0; name_index < candidates.size(); ++name_index)
            {
                const ft_string &name = candidates[name_index];
                if (name.empty())
                    continue;
                for (size_t ext_index = 0; ext_index < extension_count; ++ext_index)
                {
                    const char *extension = extensions[ext_index];
                    if (extension == ft_nullptr)
                        continue;
                    ft_string candidate(directory);
                    candidate.append("/");
                    candidate.append(name);
                    candidate.append(extension);
                    if (!primary_set)
                    {
                        result.primary_candidate = candidate;
                        primary_set = true;
                    }
                    if (file_exists(candidate.c_str()) > 0)
                    {
                        result.resolved_path = candidate;
                        result.found_existing = true;
                        return result;
                    }
                }
            }
        }

        const char *fallback_names[] = {"default", "commander", "profile"};
        const size_t fallback_count = sizeof(fallback_names) / sizeof(fallback_names[0]);

        for (size_t dir_index = 0; dir_index < directory_count; ++dir_index)
        {
            const char *directory = directories[dir_index];
            if (directory == ft_nullptr)
                continue;
            for (size_t fallback_index = 0; fallback_index < fallback_count; ++fallback_index)
            {
                const char *fallback_name = fallback_names[fallback_index];
                if (fallback_name == ft_nullptr)
                    continue;
                for (size_t ext_index = 0; ext_index < extension_count; ++ext_index)
                {
                    const char *extension = extensions[ext_index];
                    if (extension == ft_nullptr)
                        continue;
                    ft_string candidate(directory);
                    candidate.append("/");
                    candidate.append(fallback_name);
                    candidate.append(extension);
                    if (!primary_set)
                    {
                        result.primary_candidate = candidate;
                        primary_set = true;
                    }
                    if (file_exists(candidate.c_str()) > 0)
                    {
                        result.resolved_path = candidate;
                        result.found_existing = true;
                        return result;
                    }
                }
            }
        }

        if (!primary_set)
        {
            result.primary_candidate = ft_string("assets/portraits/");
            result.primary_candidate.append(result.commander_key);
            result.primary_candidate.append(".png");
        }

        result.resolved_path = result.primary_candidate;
        result.found_existing = false;
        return result;
    }

    commander_portrait_cache_entry *find_portrait_cache_entry(const ft_string &commander_key) noexcept
    {
        for (size_t index = 0; index < g_commander_portrait_cache.size(); ++index)
        {
            if (g_commander_portrait_cache[index].commander_key == commander_key)
                return &g_commander_portrait_cache[index];
        }
        return ft_nullptr;
    }

    bool load_portrait_size(const ft_string &path, size_t &out_size) noexcept
    {
        out_size = 0U;
        if (path.empty())
            return false;

        FILE *file = ft_fopen(path.c_str(), "rb");
        if (file == ft_nullptr)
            return false;

        unsigned char buffer[4096];
        while (true)
        {
            size_t read_count = fread(buffer, 1, sizeof(buffer), file);
            if (read_count > 0U)
                out_size += read_count;
            if (read_count < sizeof(buffer))
            {
                if (ferror(file))
                {
                    ft_fclose(file);
                    out_size = 0U;
                    return false;
                }
                break;
            }
        }

        ft_fclose(file);
        return true;
    }
}

namespace
{
    bool write_connectivity_failure_entry(const ft_string &host, int status_code, long timestamp_ms,
        const ft_string &target_path) noexcept
    {
        FILE    *file_handle;
        ft_string timestamp_text;
        ft_string entry;
        ft_string build_label;
        long      seconds_component;
        long      milliseconds_component;
        int       write_result;

        file_handle = ft_fopen(target_path.c_str(), "a");
        if (file_handle == ft_nullptr)
            return false;

        seconds_component = timestamp_ms / 1000;
        milliseconds_component = timestamp_ms % 1000;
        if (milliseconds_component < 0)
            milliseconds_component = -milliseconds_component;

        timestamp_text = time_format_iso8601(static_cast<t_time>(seconds_component));
        if (timestamp_text.empty())
            timestamp_text = ft_to_string(timestamp_ms);
        build_label = build_info_format_label();

        entry = ft_string("[");
        entry.append(timestamp_text);
        entry.append(".");
        if (milliseconds_component < 100)
            entry.append("0");
        if (milliseconds_component < 10)
            entry.append("0");
        entry.append(ft_to_string(milliseconds_component));
        entry.append("] ");
        if (!build_label.empty())
        {
            entry.append("[");
            entry.append(build_label);
            entry.append("] ");
        }
        entry.append("backend ping to ");
        entry.append(host);
        entry.append(" failed");
        if (status_code != 0)
        {
            entry.append(" (HTTP ");
            entry.append(ft_to_string(status_code));
            entry.append(")");
        }
        else
            entry.append(" (no status)");
        entry.append("\n");

        write_result = ft_fprintf(file_handle, "%s", entry.c_str());
        ft_fclose(file_handle);
        if (write_result < 0)
            return false;
        if (static_cast<size_t>(write_result) != entry.size())
            return false;
        return true;
    }

    void copy_range_to_string(ft_string &target, const char *begin, const char *end)
    {
        target.clear();
        const char *cursor = begin;
        while (cursor < end)
        {
            target.append(*cursor);
            cursor += 1;
        }
    }

    void split_patch_note_lines_internal(const ft_string &body, ft_vector<ft_string> &out_lines)
    {
        out_lines.clear();
        const char *cstr = body.c_str();
        if (cstr == ft_nullptr)
            return;
        const char *cursor = cstr;
        const char *line_start = cstr;
        const char *end = cstr + body.size();

        while (cursor < end)
        {
            if (*cursor == '\r' || *cursor == '\n')
            {
                ft_string line;
                copy_range_to_string(line, line_start, cursor);
                out_lines.push_back(line);
                if (*cursor == '\r' && (cursor + 1) < end && *(cursor + 1) == '\n')
                    cursor += 1;
                cursor += 1;
                line_start = cursor;
                continue;
            }
            cursor += 1;
        }

        if (line_start < end)
        {
            ft_string line;
            copy_range_to_string(line, line_start, end);
            out_lines.push_back(line);
        }
        else if (body.size() > 0 && (*(end - 1) == '\n' || *(end - 1) == '\r'))
        {
            out_lines.push_back(ft_string());
        }
    }
}

bool main_menu_append_connectivity_failure_log(const ft_string &host, int status_code, long timestamp_ms) noexcept
{
    ft_string log_path("test_failures.log");

    return write_connectivity_failure_entry(host, status_code, timestamp_ms, log_path);
}

bool main_menu_can_launch_campaign(const ft_string &save_path) noexcept
{
    if (save_path.empty())
        return false;

    int exists_result = file_exists(save_path.c_str());
    if (exists_result <= 0)
        return false;

    return true;
}

bool main_menu_preload_commander_portrait(const ft_string &commander_name) noexcept
{
    commander_portrait_resolution resolution = resolve_commander_portrait_resolution(commander_name);

    commander_portrait_cache_entry *entry = find_portrait_cache_entry(resolution.commander_key);
    if (entry == ft_nullptr)
    {
        commander_portrait_cache_entry new_entry;
        new_entry.commander_key = resolution.commander_key;
        g_commander_portrait_cache.push_back(new_entry);
        size_t cache_size = g_commander_portrait_cache.size();
        if (cache_size == 0U)
            return false;
        entry = &g_commander_portrait_cache[cache_size - 1U];
    }

    if (entry->attempted)
        return entry->loaded;

    entry->attempted = true;
    entry->asset_path = resolution.resolved_path;
    entry->cached_size = 0U;

    size_t loaded_size = 0U;
    if (load_portrait_size(resolution.resolved_path, loaded_size))
    {
        entry->cached_size = loaded_size;
        entry->loaded = true;
        return true;
    }

    entry->loaded = false;
    entry->cached_size = 0U;
    return false;
}

ft_rect build_main_menu_viewport()
{
    const ft_rect base_rect(460, 220, 360, 56);

    ft_rect viewport = base_rect;
    const int window_height = static_cast<int>(app_constants::kWindowHeight);
    const int reserved_bottom = 180;
    viewport.height = window_height - base_rect.top - reserved_bottom;
    if (viewport.height < base_rect.height)
        viewport.height = base_rect.height;

    return viewport;
}

ft_vector<ft_menu_item> build_main_menu_items()
{
    const ft_rect base_rect(460, 220, 360, 56);
    const int      spacing = 22;

    struct menu_entry
    {
        const char *identifier;
        const char *label;
        const char *description;
        bool        enabled;
    };

    const menu_entry entries[] = {
        {"new_game", "New Game", "Begin a fresh campaign for the active commander.", true},
        {"resume", "Resume", "Jump back into your latest campaign save once one is available.", false},
        {"load", "Load", "Review existing saves and prepare to resume a prior campaign.", true},
        {"settings", "Settings", "Adjust gameplay, interface scale, and menu layout preferences for this commander.", true},
        {"swap_profile", "Swap Profile", "Switch to a different commander profile.", true},
        {"changelog", "Patch Notes", "Read the latest Galactic Planet Miner updates fetched from HQ.", true},
        {"manual", "Encyclopedia", "Open the commander encyclopedia for controls, systems, and lore summaries.", true},
        {"clear_cloud", "Clear Cloud Data",
            "Remove backend-linked progress for this commander after confirming the action.", true},
        {"exit", "Exit", "Close Galactic Planet Miner.", true},
    };

    ft_vector<ft_menu_item> items;
    items.reserve(sizeof(entries) / sizeof(entries[0]));

    for (size_t index = 0; index < sizeof(entries) / sizeof(entries[0]); ++index)
    {
        ft_rect item_rect = base_rect;
        item_rect.top += static_cast<int>(index) * (base_rect.height + spacing);

        const menu_entry &entry = entries[index];
        ft_menu_item      item(ft_string(entry.identifier), ft_string(entry.label), item_rect);
        item.enabled = entry.enabled;
        item.description = ft_string(entry.description);
        items.push_back(item);
    }

    return items;
}

const ft_vector<ft_string> &get_main_menu_tutorial_tips()
{
    static bool initialized = false;
    static ft_vector<ft_string> tips;

    if (!initialized)
    {
        const char *raw_tips[] = {
            "Press Enter or click anywhere to dismiss these tips.",
            "Use Arrow Keys / D-Pad or the mouse to highlight menu entries.",
            "Choose New Game to start a fresh campaign for this commander.",
            "Swap Profile lets you switch between saved commanders.",
        };
        const size_t tip_count = sizeof(raw_tips) / sizeof(raw_tips[0]);
        tips.reserve(tip_count);
        for (size_t index = 0; index < tip_count; ++index)
        {
            ft_string tip(raw_tips[index]);
            tips.push_back(tip);
        }
        initialized = true;
    }

    return tips;
}

const ft_vector<ft_string> &get_main_menu_manual_lines()
{
    static bool initialized = false;
    static ft_vector<ft_string> lines;

    if (!initialized)
    {
        const char *raw_lines[] = {
            "Browse ship loadouts, building efficiencies, and resource yields.",
            "Review campaign objectives with quick strategy tips for each branch.",
            "Study combat controls, hotkeys, and fleet formation guidance.",
            "Access lore entries and encyclopedia cross-links without leaving the menu.",
            "Reconnect to the network if encyclopedia updates appear out of date.",
        };
        const size_t line_count = sizeof(raw_lines) / sizeof(raw_lines[0]);
        lines.reserve(line_count);
        for (size_t index = 0; index < line_count; ++index)
            lines.push_back(ft_string(raw_lines[index]));
        initialized = true;
    }

    return lines;
}

ft_vector<ft_string> main_menu_split_patch_notes(const ft_string &body)
{
    ft_vector<ft_string> lines;
    split_patch_note_lines_internal(body, lines);
    return lines;
}

void main_menu_mark_connectivity_checking(MainMenuConnectivityStatus &status, long timestamp_ms) noexcept
{
    status.state = MAIN_MENU_CONNECTIVITY_CHECKING;
    status.last_attempt_ms = timestamp_ms;
}

void main_menu_apply_connectivity_result(MainMenuConnectivityStatus &status, bool success, int status_code,
    long timestamp_ms) noexcept
{
    if (success)
        status.state = MAIN_MENU_CONNECTIVITY_ONLINE;
    else
        status.state = MAIN_MENU_CONNECTIVITY_OFFLINE;
    status.last_status_code = status_code;
    status.last_result_ms = timestamp_ms;
}

ft_string main_menu_resolve_connectivity_label(const MainMenuConnectivityStatus &status)
{
    if (status.state == MAIN_MENU_CONNECTIVITY_CHECKING)
        return ft_string("Checking backend...");

    ft_string label;
    if (status.state == MAIN_MENU_CONNECTIVITY_ONLINE)
        label = ft_string("Backend Online");
    else
        label = ft_string("Backend Offline");

    if (status.last_status_code != 0)
    {
        label.append(" (HTTP ");
        label.append(ft_to_string(status.last_status_code));
        label.append(")");
    }

    return label;
}

SDL_Color main_menu_resolve_connectivity_color(const MainMenuConnectivityStatus &status)
{
    SDL_Color color;
    color.r = 182;
    color.g = 192;
    color.b = 212;
    color.a = 255;

    if (status.state == MAIN_MENU_CONNECTIVITY_ONLINE)
    {
        color.r = 108;
        color.g = 210;
        color.b = 156;
        return color;
    }
    if (status.state == MAIN_MENU_CONNECTIVITY_OFFLINE)
    {
        color.r = 220;
        color.g = 120;
        color.b = 120;
        return color;
    }
    return color;
}

ft_string main_menu_resolve_build_label()
{
    return build_info_format_label();
}

namespace
{
    ft_string resolve_menu_description(const ft_ui_menu &menu)
    {
        const ft_menu_item *hovered = menu.get_hovered_item();
        if (hovered != ft_nullptr && !hovered->description.empty())
            return hovered->description;

        const ft_menu_item *selected = menu.get_selected_item();
        if (selected != ft_nullptr)
            return selected->description;

        return ft_string();
    }

    ft_string format_menu_hotkey_label(int key_code, const char *fallback_label)
    {
        if (key_code >= 'a' && key_code <= 'z')
        {
            char uppercase = static_cast<char>(key_code - ('a' - 'A'));
            ft_string label;
            label.append(uppercase);
            return label;
        }
        if (key_code >= 'A' && key_code <= 'Z')
        {
            ft_string label;
            label.append(static_cast<char>(key_code));
            return label;
        }
        if (key_code >= '0' && key_code <= '9')
        {
            ft_string label;
            label.append(static_cast<char>(key_code));
            return label;
        }
        if (key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CONFIRM)
            return ft_string("Enter");
        if (key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CANCEL)
            return ft_string("Esc");
        if (key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DELETE)
            return ft_string("Backspace");
        if (key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_UP || key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DOWN
            || key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_LEFT || key_code == PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_RIGHT)
            return ft_string("Arrow Keys");
        if (fallback_label != ft_nullptr)
            return ft_string(fallback_label);
        ft_string label;
        label.append('?');
        return label;
    }

    ft_string resolve_menu_navigation_hint(const ft_ui_menu &menu, const PlayerProfilePreferences *preferences)
    {
        const ft_menu_item *hovered = menu.get_hovered_item();
        const ft_menu_item *selected = menu.get_selected_item();

        const ft_menu_item *reference = hovered != ft_nullptr ? hovered : selected;

        if (reference == ft_nullptr)
        {
            ft_string hint("Use Arrow Keys / D-Pad to choose an option. Enter / A confirms. Esc / B exits.");
            return hint;
        }

        if (!reference->enabled)
        {
            ft_string hint(reference->label);
            hint.append(" is not available yet. Esc / B: Back");
            return hint;
        }

        ft_string keyboard_confirm;
        ft_string keyboard_cancel;
        ft_string navigation_label;

        if (preferences != ft_nullptr)
        {
            keyboard_confirm = format_menu_hotkey_label(preferences->hotkey_menu_confirm, "Enter");
            keyboard_cancel = format_menu_hotkey_label(preferences->hotkey_menu_cancel, "Esc");
            ft_string nav_up = format_menu_hotkey_label(preferences->hotkey_menu_up, "Arrow Keys");
            ft_string nav_down = format_menu_hotkey_label(preferences->hotkey_menu_down, "Arrow Keys");
            if (nav_up == nav_down)
                navigation_label = nav_up;
            else
            {
                navigation_label = nav_up;
                navigation_label.append("/");
                navigation_label.append(nav_down);
            }
        }
        else
        {
            keyboard_confirm = ft_string("Enter");
            keyboard_cancel = ft_string("Esc");
            navigation_label = ft_string("Arrow Keys");
        }

        ft_string hint;
        hint.append(keyboard_confirm);
        hint.append(" / A: Select ");
        hint.append(reference->label);
        hint.append("  |  ");
        hint.append(navigation_label);
        hint.append(" / D-Pad: Navigate  |  ");
        hint.append(keyboard_cancel);
        hint.append(" / B: Back");
        return hint;
    }

#if GALACTIC_HAVE_SDL2
    void render_menu_overlay(SDL_Renderer &renderer, TTF_Font *menu_font, int output_width, int output_height,
        const MainMenuOverlayContext *overlay)
    {
        if (overlay == ft_nullptr || !overlay->visible)
            return;
        if (menu_font == ft_nullptr)
            return;

        const int overlay_padding = 24;
        const int min_overlay_width = 540;
        const int blank_line_height = 18;
        const int line_spacing = 6;
        int       overlay_height = overlay_padding * 2;
        int       max_content_width = 0;

        SDL_Texture *heading_texture = ft_nullptr;
        SDL_Rect     heading_rect;
        heading_rect.x = 0;
        heading_rect.y = 0;
        heading_rect.w = 0;
        heading_rect.h = 0;

        if (!overlay->heading.empty())
        {
            SDL_Color heading_color = {236, 242, 255, 255};
            heading_texture = create_text_texture(renderer, *menu_font, overlay->heading, heading_color, heading_rect);
            if (heading_texture != ft_nullptr)
            {
                overlay_height += heading_rect.h;
                max_content_width = heading_rect.w;
                overlay_height += 16;
            }
        }

        const ft_vector<ft_string> &lines = overlay->lines;
        ft_vector<SDL_Texture *>     line_textures;
        ft_vector<SDL_Rect>          line_rects;
        line_textures.reserve(lines.size());
        line_rects.reserve(lines.size());

        for (size_t index = 0; index < lines.size(); ++index)
        {
            SDL_Texture *line_texture = ft_nullptr;
            SDL_Rect     line_rect;
            line_rect.x = 0;
            line_rect.y = 0;
            line_rect.w = 0;
            line_rect.h = 0;

            if (lines[index].empty())
            {
                line_rect.h = blank_line_height;
                overlay_height += line_rect.h;
                line_textures.push_back(ft_nullptr);
                line_rects.push_back(line_rect);
            }
            else
            {
                SDL_Color line_color = {208, 216, 236, 255};
                line_texture = create_text_texture(renderer, *menu_font, lines[index], line_color, line_rect);
                if (line_texture != ft_nullptr)
                {
                    overlay_height += line_rect.h;
                    if (static_cast<int>(line_rect.w) > max_content_width)
                        max_content_width = line_rect.w;
                }
                else
                {
                    line_rect.h = blank_line_height;
                    overlay_height += line_rect.h;
                }
                line_textures.push_back(line_texture);
                line_rects.push_back(line_rect);
            }

            if (index + 1U < lines.size())
                overlay_height += line_spacing;
        }

        SDL_Texture *footer_texture = ft_nullptr;
        SDL_Rect     footer_rect;
        footer_rect.x = 0;
        footer_rect.y = 0;
        footer_rect.w = 0;
        footer_rect.h = 0;

        if (!overlay->footer.empty())
        {
            SDL_Color footer_color = {184, 196, 224, 255};
            footer_texture = create_text_texture(renderer, *menu_font, overlay->footer, footer_color, footer_rect);
            if (footer_texture != ft_nullptr)
            {
                overlay_height += footer_rect.h;
                if (static_cast<int>(footer_rect.w) > max_content_width)
                    max_content_width = footer_rect.w;
                overlay_height += 12;
            }
        }

        if (heading_texture == ft_nullptr && line_textures.empty() && footer_texture == ft_nullptr)
            return;

        int overlay_width = overlay_padding * 2 + max_content_width;
        if (overlay_width < min_overlay_width)
            overlay_width = min_overlay_width;
        const int available_width = output_width - 2 * overlay_padding;
        if (available_width > 0 && overlay_width > available_width)
            overlay_width = available_width;
        if (overlay_width <= 0)
            overlay_width = min_overlay_width;

        SDL_Rect overlay_rect;
        overlay_rect.w = overlay_width;
        overlay_rect.h = overlay_height;
        overlay_rect.x = output_width / 2 - overlay_rect.w / 2;
        overlay_rect.y = output_height / 2 - overlay_rect.h / 2;
        if (overlay_rect.x < 48)
            overlay_rect.x = 48;
        if (overlay_rect.y < 48)
            overlay_rect.y = 48;

        SDL_SetRenderDrawColor(&renderer, 18, 24, 44, 238);
        SDL_RenderFillRect(&renderer, &overlay_rect);
        SDL_SetRenderDrawColor(&renderer, 92, 112, 166, 255);
        SDL_RenderDrawRect(&renderer, &overlay_rect);

        int text_x = overlay_rect.x + overlay_padding;
        int text_y = overlay_rect.y + overlay_padding;

        if (heading_texture != ft_nullptr)
        {
            heading_rect.x = text_x;
            heading_rect.y = text_y;
            SDL_RenderCopy(&renderer, heading_texture, ft_nullptr, &heading_rect);
            text_y += heading_rect.h + 12;
        }

        for (size_t index = 0; index < line_textures.size(); ++index)
        {
            SDL_Texture *line_texture = line_textures[index];
            SDL_Rect      line_rect = line_rects[index];
            if (line_texture != ft_nullptr)
            {
                line_rect.x = text_x;
                line_rect.y = text_y;
                SDL_RenderCopy(&renderer, line_texture, ft_nullptr, &line_rect);
                text_y += line_rect.h;
            }
            else
                text_y += line_rect.h;

            if (index + 1U < line_textures.size())
                text_y += line_spacing;
        }

        if (footer_texture != ft_nullptr)
        {
            text_y += 6;
            footer_rect.x = text_x;
            footer_rect.y = text_y;
            SDL_RenderCopy(&renderer, footer_texture, ft_nullptr, &footer_rect);
        }

        if (heading_texture != ft_nullptr)
            SDL_DestroyTexture(heading_texture);
        for (size_t index = 0; index < line_textures.size(); ++index)
        {
            if (line_textures[index] != ft_nullptr)
                SDL_DestroyTexture(line_textures[index]);
        }
        if (footer_texture != ft_nullptr)
            SDL_DestroyTexture(footer_texture);
    }

    void render_menu_tutorial_overlay(SDL_Renderer &renderer, TTF_Font *menu_font, int output_width,
        const MainMenuTutorialContext *tutorial)
    {
        if (tutorial == ft_nullptr || !tutorial->visible)
            return;
        if (menu_font == ft_nullptr)
            return;
        if (tutorial->tips == ft_nullptr)
            return;

        const ft_vector<ft_string> &tips = *tutorial->tips;
        if (tips.empty())
            return;

        const int overlay_padding = 20;
        const int min_overlay_width = 520;
        int       overlay_height = overlay_padding * 2;
        int       max_content_width = 0;

        SDL_Color title_color = {234, 238, 250, 255};
        SDL_Color primary_tip_color = {210, 220, 240, 255};
        SDL_Color bullet_tip_color = {200, 208, 232, 255};

        SDL_Rect  title_rect;
        SDL_Texture *title_texture = create_text_texture(renderer, *menu_font, ft_string("Menu Tips"), title_color, title_rect);
        if (title_texture != ft_nullptr)
        {
            overlay_height += title_rect.h;
            max_content_width = title_rect.w;
            overlay_height += 12;
        }

        ft_vector<SDL_Texture *> tip_textures;
        ft_vector<SDL_Rect>       tip_rects;
        tip_textures.reserve(tips.size());
        tip_rects.reserve(tips.size());

        for (size_t index = 0; index < tips.size(); ++index)
        {
            const bool   is_primary_tip = index == 0;
            ft_string     display_text;
            if (is_primary_tip)
                display_text = tips[index];
            else
            {
                display_text = ft_string("\xE2\x80\xA2 ");
                display_text.append(tips[index]);
            }

            SDL_Rect tip_rect;
            SDL_Color tip_color = is_primary_tip ? primary_tip_color : bullet_tip_color;
            SDL_Texture *tip_texture = create_text_texture(renderer, *menu_font, display_text, tip_color, tip_rect);
            if (tip_texture != ft_nullptr)
            {
                tip_textures.push_back(tip_texture);
                tip_rects.push_back(tip_rect);
                overlay_height += tip_rect.h;
                if (static_cast<int>(tip_rect.w) > max_content_width)
                    max_content_width = tip_rect.w;
                if (index + 1U < tips.size())
                    overlay_height += 8;
            }
        }

        if (tip_textures.empty() && title_texture == ft_nullptr)
        {
            return;
        }

        int overlay_width = overlay_padding * 2 + max_content_width;
        if (overlay_width < min_overlay_width)
            overlay_width = min_overlay_width;
        const int available_width = output_width - 2 * overlay_padding;
        if (available_width > 0 && overlay_width > available_width)
            overlay_width = available_width;
        if (overlay_width <= 0)
            overlay_width = min_overlay_width;

        SDL_Rect overlay_rect;
        overlay_rect.x = 72;
        overlay_rect.y = 84;
        overlay_rect.w = overlay_width;
        overlay_rect.h = overlay_height;

        SDL_SetRenderDrawColor(&renderer, 18, 24, 44, 235);
        SDL_RenderFillRect(&renderer, &overlay_rect);
        SDL_SetRenderDrawColor(&renderer, 90, 110, 160, 255);
        SDL_RenderDrawRect(&renderer, &overlay_rect);

        int text_x = overlay_rect.x + overlay_padding;
        int text_y = overlay_rect.y + overlay_padding;

        if (title_texture != ft_nullptr)
        {
            title_rect.x = text_x;
            title_rect.y = text_y;
            SDL_RenderCopy(&renderer, title_texture, ft_nullptr, &title_rect);
            text_y += title_rect.h + 12;
        }

        for (size_t index = 0; index < tip_textures.size(); ++index)
        {
            SDL_Texture *tip_texture = tip_textures[index];
            SDL_Rect      tip_rect = tip_rects[index];
            tip_rect.x = text_x;
            tip_rect.y = text_y;
            SDL_RenderCopy(&renderer, tip_texture, ft_nullptr, &tip_rect);
            text_y += tip_rect.h;
            if (index + 1U < tip_textures.size())
                text_y += 8;
            SDL_DestroyTexture(tip_texture);
        }

        if (title_texture != ft_nullptr)
            SDL_DestroyTexture(title_texture);
    }
#endif
}

void render_main_menu(SDL_Renderer &renderer, const ft_ui_menu &menu, TTF_Font *title_font, TTF_Font *menu_font,
    int window_width, int window_height, const ft_string &active_profile_name, const MainMenuTutorialContext *tutorial,
    const MainMenuOverlayContext *manual, const MainMenuOverlayContext *changelog,
    const MainMenuOverlayContext *cloud_confirmation, const MainMenuConnectivityStatus *connectivity,
    const MainMenuAlertBanner *alert)
{
#if GALACTIC_HAVE_SDL2
    SDL_SetRenderDrawColor(&renderer, 12, 16, 28, 255);
    SDL_RenderClear(&renderer);

    int output_width = window_width;
    int output_height = window_height;
    SDL_GetRendererOutputSize(&renderer, &output_width, &output_height);

    if (title_font != ft_nullptr)
    {
        SDL_Color title_color = {220, 220, 245, 255};
        SDL_Rect  title_rect;
        SDL_Texture *title_texture = create_text_texture(renderer, *title_font, ft_string("Galactic Planet Miner"), title_color,
            title_rect);
        if (title_texture != ft_nullptr)
        {
            title_rect.x = output_width / 2 - title_rect.w / 2;
            title_rect.y = 96;
            SDL_RenderCopy(&renderer, title_texture, ft_nullptr, &title_rect);
            SDL_DestroyTexture(title_texture);
        }
    }

    if (menu_font != ft_nullptr && !active_profile_name.empty())
    {
        ft_string profile_label("Profile: ");
        profile_label.append(active_profile_name);

        SDL_Color profile_color = {200, 210, 230, 255};
        SDL_Rect  profile_rect;
        SDL_Texture *profile_texture = create_text_texture(renderer, *menu_font, profile_label, profile_color, profile_rect);
        if (profile_texture != ft_nullptr)
        {
            profile_rect.x = output_width / 2 - profile_rect.w / 2;
            profile_rect.y = 164;
            SDL_RenderCopy(&renderer, profile_texture, ft_nullptr, &profile_rect);
            SDL_DestroyTexture(profile_texture);
        }
    }

    if (menu_font != ft_nullptr && connectivity != ft_nullptr)
    {
        const ft_string connectivity_label = main_menu_resolve_connectivity_label(*connectivity);
        if (!connectivity_label.empty())
        {
            SDL_Color status_color = main_menu_resolve_connectivity_color(*connectivity);
            SDL_Rect  status_rect;
            SDL_Texture *status_texture = create_text_texture(renderer, *menu_font, connectivity_label, status_color, status_rect);
            if (status_texture != ft_nullptr)
            {
                status_rect.x = output_width - status_rect.w - 48;
                status_rect.y = 40;
                SDL_RenderCopy(&renderer, status_texture, ft_nullptr, &status_rect);
                SDL_DestroyTexture(status_texture);
            }
        }
    }

    const ft_vector<ft_menu_item> &items = menu.get_items();
    const int hovered_index = menu.get_hovered_index();
    const int selected_index = menu.get_selected_index();

    const ft_rect &viewport = menu.get_viewport_bounds();
    const bool clip_enabled = viewport.width > 0 && viewport.height > 0;
    SDL_Rect clip_rect;
    clip_rect.x = 0;
    clip_rect.y = 0;
    clip_rect.w = 0;
    clip_rect.h = 0;
    int clip_bottom = 0;

    if (alert != ft_nullptr && alert->visible && menu_font != ft_nullptr && !alert->message.empty())
    {
        SDL_Color text_color;
        if (alert->is_error)
        {
            text_color.r = 255;
            text_color.g = 206;
            text_color.b = 206;
            text_color.a = 255;
        }
        else
        {
            text_color.r = 200;
            text_color.g = 230;
            text_color.b = 255;
            text_color.a = 255;
        }

        SDL_Rect text_rect;
        SDL_Texture *text_texture = create_text_texture(renderer, *menu_font, alert->message, text_color, text_rect);
        if (text_texture != ft_nullptr)
        {
            const int padding_x = 24;
            const int padding_y = 12;

            SDL_Rect alert_rect;
            alert_rect.w = text_rect.w + padding_x * 2;
            alert_rect.h = text_rect.h + padding_y * 2;
            alert_rect.x = output_width / 2 - alert_rect.w / 2;
            int preferred_bottom = viewport.top - 24;
            if (preferred_bottom < 180)
                preferred_bottom = 180;
            alert_rect.y = preferred_bottom - alert_rect.h;
            if (alert_rect.y < 120)
                alert_rect.y = 120;

            SDL_Color background_color;
            SDL_Color border_color;
            if (alert->is_error)
            {
                background_color.r = 84;
                background_color.g = 32;
                background_color.b = 32;
                background_color.a = 235;
                border_color.r = 164;
                border_color.g = 80;
                border_color.b = 80;
                border_color.a = 255;
            }
            else
            {
                background_color.r = 26;
                background_color.g = 64;
                background_color.b = 88;
                background_color.a = 235;
                border_color.r = 88;
                border_color.g = 138;
                border_color.b = 176;
                border_color.a = 255;
            }

            SDL_SetRenderDrawColor(&renderer, background_color.r, background_color.g, background_color.b, background_color.a);
            SDL_RenderFillRect(&renderer, &alert_rect);
            SDL_SetRenderDrawColor(&renderer, border_color.r, border_color.g, border_color.b, border_color.a);
            SDL_RenderDrawRect(&renderer, &alert_rect);

            text_rect.x = alert_rect.x + padding_x;
            text_rect.y = alert_rect.y + padding_y;
            SDL_RenderCopy(&renderer, text_texture, ft_nullptr, &text_rect);
            SDL_DestroyTexture(text_texture);
        }
    }

    if (clip_enabled)
    {
        clip_rect.x = viewport.left;
        clip_rect.y = viewport.top;
        clip_rect.w = viewport.width;
        clip_rect.h = viewport.height;
        clip_bottom = clip_rect.y + clip_rect.h;
        SDL_RenderSetClipRect(&renderer, &clip_rect);
    }

    const int scroll_offset = menu.get_scroll_offset();

    for (size_t index = 0; index < items.size(); ++index)
    {
        const ft_menu_item &item = items[index];

        const bool  is_hovered = static_cast<int>(index) == hovered_index;
        const bool  is_selected = static_cast<int>(index) == selected_index;
        const bool  is_disabled = !item.enabled;

        Uint8 r = 28;
        Uint8 g = 36;
        Uint8 b = 60;

        if (is_disabled)
        {
            r = 30;
            g = 34;
            b = 44;
            if (is_hovered)
            {
                r = 42;
                g = 46;
                b = 60;
            }
        }
        else if (is_hovered)
        {
            r = 56;
            g = 84;
            b = 140;
        }
        else if (is_selected)
        {
            r = 40;
            g = 64;
            b = 112;
        }

        SDL_Rect button_rect;
        button_rect.x = item.bounds.left;
        button_rect.y = item.bounds.top - scroll_offset;
        button_rect.w = item.bounds.width;
        button_rect.h = item.bounds.height;

        if (clip_enabled && (button_rect.y + button_rect.h <= clip_rect.y || button_rect.y >= clip_bottom))
            continue;

        SDL_SetRenderDrawColor(&renderer, r, g, b, 255);
        SDL_RenderFillRect(&renderer, &button_rect);

        const Uint8 border_r = is_disabled ? 70 : 90;
        const Uint8 border_g = is_disabled ? 80 : 110;
        const Uint8 border_b = is_disabled ? 120 : 160;
        SDL_SetRenderDrawColor(&renderer, border_r, border_g, border_b, 255);
        SDL_RenderDrawRect(&renderer, &button_rect);

        if (menu_font != ft_nullptr)
        {
            SDL_Color text_color = {255, 255, 255, 255};
            if (is_disabled)
            {
                text_color.r = 188;
                text_color.g = 196;
                text_color.b = 210;
            }
            SDL_Rect  text_rect;
            SDL_Texture *text_texture = create_text_texture(renderer, *menu_font, item.label, text_color, text_rect);
            if (text_texture != ft_nullptr)
            {
                text_rect.x = item.bounds.left + (item.bounds.width - text_rect.w) / 2;
                text_rect.y = button_rect.y + (item.bounds.height - text_rect.h) / 2;
                SDL_RenderCopy(&renderer, text_texture, ft_nullptr, &text_rect);
                SDL_DestroyTexture(text_texture);
            }
        }
    }

    if (clip_enabled)
        SDL_RenderSetClipRect(&renderer, ft_nullptr);

    int next_text_top = viewport.top + viewport.height + 32;

    if (menu_font != ft_nullptr)
    {
        const ft_string description = resolve_menu_description(menu);
        if (!description.empty())
        {
            SDL_Color description_color = {210, 220, 240, 255};
            SDL_Rect  description_rect;
            SDL_Texture *description_texture = create_text_texture(renderer, *menu_font, description, description_color,
                description_rect);
            if (description_texture != ft_nullptr)
            {
                const int description_top = viewport.top + viewport.height + 32;
                description_rect.x = output_width / 2 - description_rect.w / 2;
                description_rect.y = description_top;
                SDL_RenderCopy(&renderer, description_texture, ft_nullptr, &description_rect);
                SDL_DestroyTexture(description_texture);
                next_text_top = description_rect.y + description_rect.h + 24;
            }
        }
    }

    if (menu_font != ft_nullptr)
    {
        const ft_string hint = resolve_menu_navigation_hint(menu, ft_nullptr);
        if (!hint.empty())
        {
            SDL_Color hint_color = {165, 176, 204, 255};
            SDL_Rect  hint_rect;
            SDL_Texture *hint_texture = create_text_texture(renderer, *menu_font, hint, hint_color, hint_rect);
            if (hint_texture != ft_nullptr)
            {
                hint_rect.x = output_width / 2 - hint_rect.w / 2;
                hint_rect.y = next_text_top;
                SDL_RenderCopy(&renderer, hint_texture, ft_nullptr, &hint_rect);
                SDL_DestroyTexture(hint_texture);
                next_text_top = hint_rect.y + hint_rect.h + 20;
            }
        }
    }

    if (menu_font != ft_nullptr)
    {
        const ft_string build_label = main_menu_resolve_build_label();
        if (!build_label.empty())
        {
            SDL_Color build_color = {140, 150, 184, 255};
            SDL_Rect  build_rect;
            SDL_Texture *build_texture = create_text_texture(renderer, *menu_font, build_label, build_color, build_rect);
            if (build_texture != ft_nullptr)
            {
                build_rect.x = output_width - build_rect.w - 48;
                build_rect.y = output_height - build_rect.h - 40;
                SDL_RenderCopy(&renderer, build_texture, ft_nullptr, &build_rect);
                SDL_DestroyTexture(build_texture);
            }
        }
    }

    render_menu_overlay(renderer, menu_font, output_width, output_height, manual);
    render_menu_overlay(renderer, menu_font, output_width, output_height, changelog);
    render_menu_overlay(renderer, menu_font, output_width, output_height, cloud_confirmation);
    render_menu_tutorial_overlay(renderer, menu_font, output_width, tutorial);

    SDL_RenderPresent(&renderer);
#else
    (void)renderer;
    (void)menu;
    (void)title_font;
    (void)menu_font;
    (void)window_width;
    (void)window_height;
    (void)active_profile_name;
    (void)tutorial;
    (void)manual;
    (void)changelog;
    (void)cloud_confirmation;
    (void)connectivity;
    (void)alert;
#endif
}

namespace main_menu_testing
{
    ft_string resolve_active_description(const ft_ui_menu &menu)
    {
        return resolve_menu_description(menu);
    }

    ft_string resolve_navigation_hint(
        const ft_ui_menu &menu, const PlayerProfilePreferences *preferences)
    {
        return resolve_menu_navigation_hint(menu, preferences);
    }

    ft_vector<ft_string> collect_tutorial_tips()
    {
        const ft_vector<ft_string> &tips = get_main_menu_tutorial_tips();
        ft_vector<ft_string>        copy;
        copy.reserve(tips.size());
        for (size_t index = 0; index < tips.size(); ++index)
            copy.push_back(tips[index]);
        return copy;
    }

    ft_vector<ft_string> collect_manual_lines()
    {
        const ft_vector<ft_string> &lines = get_main_menu_manual_lines();
        ft_vector<ft_string>        copy;
        copy.reserve(lines.size());
        for (size_t index = 0; index < lines.size(); ++index)
            copy.push_back(lines[index]);
        return copy;
    }

    ft_string resolve_connectivity_label(const MainMenuConnectivityStatus &status)
    {
        return main_menu_resolve_connectivity_label(status);
    }

    SDL_Color resolve_connectivity_color(const MainMenuConnectivityStatus &status)
    {
        return main_menu_resolve_connectivity_color(status);
    }

    void mark_connectivity_checking(MainMenuConnectivityStatus &status, long timestamp_ms)
    {
        main_menu_mark_connectivity_checking(status, timestamp_ms);
    }

    void apply_connectivity_result(MainMenuConnectivityStatus &status, bool success, int status_code, long timestamp_ms)
    {
        main_menu_apply_connectivity_result(status, success, status_code, timestamp_ms);
    }

    bool append_connectivity_failure_log(const ft_string &host, int status_code, long timestamp_ms, const ft_string &log_path)
    {
        return write_connectivity_failure_entry(host, status_code, timestamp_ms, log_path);
    }

    ft_string resolve_build_label()
    {
        return main_menu_resolve_build_label();
    }

    ft_vector<ft_string> split_patch_note_lines(const ft_string &body)
    {
        return main_menu_split_patch_notes(body);
    }

    void reset_commander_portrait_cache()
    {
        g_commander_portrait_cache.clear();
    }

    bool commander_portrait_attempted(const ft_string &commander_name)
    {
        commander_portrait_cache_entry *entry
            = find_portrait_cache_entry(portrait_resolve_commander_key(commander_name));
        if (entry == ft_nullptr)
            return false;
        return entry->attempted;
    }

    bool commander_portrait_loaded(const ft_string &commander_name)
    {
        commander_portrait_cache_entry *entry
            = find_portrait_cache_entry(portrait_resolve_commander_key(commander_name));
        if (entry == ft_nullptr)
            return false;
        return entry->loaded;
    }

    size_t commander_portrait_cached_size(const ft_string &commander_name)
    {
        commander_portrait_cache_entry *entry
            = find_portrait_cache_entry(portrait_resolve_commander_key(commander_name));
        if (entry == ft_nullptr)
            return 0U;
        return entry->cached_size;
    }

    ft_string resolve_cached_portrait_path(const ft_string &commander_name)
    {
        commander_portrait_cache_entry *entry
            = find_portrait_cache_entry(portrait_resolve_commander_key(commander_name));
        if (entry == ft_nullptr)
            return ft_string();
        return entry->asset_path;
    }

    ft_string resolve_commander_portrait_filename(const ft_string &commander_name)
    {
        return portrait_resolve_commander_key(commander_name);
    }
}

