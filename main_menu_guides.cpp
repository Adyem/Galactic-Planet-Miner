#include "main_menu_system.hpp"

#include "build_info.hpp"
#include "menu_localization.hpp"

#include "libft/Libft/libft.hpp"

namespace
{
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

const ft_vector<ft_string> &get_main_menu_tutorial_tips()
{
    static bool initialized = false;
    static ft_vector<ft_string> tips;

    if (!initialized)
    {
        const char *tip_keys[] = {
            "main_menu.tutorial.tips.0",
            "main_menu.tutorial.tips.1",
            "main_menu.tutorial.tips.2",
            "main_menu.tutorial.tips.3",
        };
        const char *tip_fallbacks[] = {
            "Press Enter or click anywhere to dismiss these tips.",
            "Use Arrow Keys / D-Pad or the mouse to highlight menu entries.",
            "Choose New Game to start a fresh campaign for this commander.",
            "Swap Profile lets you switch between saved commanders.",
        };
        const size_t tip_count = sizeof(tip_keys) / sizeof(tip_keys[0]);
        tips.reserve(tip_count);
        for (size_t index = 0; index < tip_count; ++index)
            tips.push_back(menu_localize(tip_keys[index], tip_fallbacks[index]));
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
        const char *line_keys[] = {
            "main_menu.manual.lines.0",
            "main_menu.manual.lines.1",
            "main_menu.manual.lines.2",
            "main_menu.manual.lines.3",
            "main_menu.manual.lines.4",
        };
        const char *line_fallbacks[] = {
            "Browse ship loadouts, building efficiencies, and resource yields.",
            "Review campaign objectives with quick strategy tips for each branch.",
            "Study combat controls, hotkeys, and fleet formation guidance.",
            "Access lore entries and encyclopedia cross-links without leaving the menu.",
            "Reconnect to the network if encyclopedia updates appear out of date.",
        };
        const size_t line_count = sizeof(line_keys) / sizeof(line_keys[0]);
        lines.reserve(line_count);
        for (size_t index = 0; index < line_count; ++index)
            lines.push_back(menu_localize(line_keys[index], line_fallbacks[index]));
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

ft_string main_menu_resolve_build_label()
{
    return build_info_format_label();
}

namespace main_menu_testing
{
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

    ft_vector<ft_string> split_patch_note_lines(const ft_string &body)
    {
        return main_menu_split_patch_notes(body);
    }

    ft_string resolve_build_label()
    {
        return main_menu_resolve_build_label();
    }
}
