#include "main_menu_system.hpp"

#include "app_constants.hpp"
#include "build_info.hpp"
#include "menu_localization.hpp"

#include "libft/File/file_utils.hpp"
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

bool main_menu_can_launch_campaign(const ft_string &save_path) noexcept
{
    if (save_path.empty())
        return false;

    int exists_result = file_exists(save_path.c_str());
    if (exists_result <= 0)
        return false;

    return true;
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

ft_vector<ft_menu_item> build_main_menu_items(const PlayerProfilePreferences *preferences)
{
    const ft_rect base_rect(460, 220, 360, 56);
    const int      spacing = 22;

    bool experiments_enabled = false;
    if (preferences != ft_nullptr)
        experiments_enabled = preferences->experimental_features_enabled;

    struct menu_entry
    {
        const char *identifier;
        const char *label_key;
        const char *label_fallback;
        const char *description_key;
        const char *description_fallback;
        bool        enabled;
        bool        experimental;
        const char *gated_description_key;
        const char *gated_description_fallback;
    };

    const menu_entry entries[] = {
        {"new_game", "main_menu.items.new_game.label", "New Game",
            "main_menu.items.new_game.description", "Begin a fresh campaign for the active commander.", true, false,
            ft_nullptr, ft_nullptr},
        {"resume", "main_menu.items.resume.label", "Resume",
            "main_menu.items.resume.description",
            "Jump back into your latest campaign save once one is available.", false, false, ft_nullptr, ft_nullptr},
        {"tutorial", "main_menu.items.tutorial.label", "Replay Tutorial",
            "main_menu.items.tutorial.description",
            "Revisit onboarding missions and practice fleet controls.", true, false, ft_nullptr, ft_nullptr},
        {"load", "main_menu.items.load.label", "Load",
            "main_menu.items.load.description", "Review existing saves and prepare to resume a prior campaign.", true, false,
            ft_nullptr, ft_nullptr},
        {"settings", "main_menu.items.settings.label", "Settings",
            "main_menu.items.settings.description",
            "Adjust gameplay, interface scale, and menu layout preferences for this commander.", true, false, ft_nullptr,
            ft_nullptr},
        {"swap_profile", "main_menu.items.swap_profile.label", "Swap Profile",
            "main_menu.items.swap_profile.description", "Switch to a different commander profile.", true, false, ft_nullptr,
            ft_nullptr},
        {"changelog", "main_menu.items.changelog.label", "Patch Notes",
            "main_menu.items.changelog.description", "Read the latest Galactic Planet Miner updates fetched from HQ.", true,
            false, ft_nullptr, ft_nullptr},
        {"manual", "main_menu.items.manual.label", "Encyclopedia",
            "main_menu.items.manual.description",
            "Open the commander encyclopedia for controls, systems, and lore summaries.", true, false, ft_nullptr,
            ft_nullptr},
        {"clear_cloud", "main_menu.items.clear_cloud.label", "Clear Cloud Data",
            "main_menu.items.clear_cloud.description",
            "Remove backend-linked progress for this commander after confirming the action.", true, true,
            "main_menu.items.clear_cloud.gated", "Enable experimental features in Settings to manage cloud data."},
        {"exit", "main_menu.items.exit.label", "Exit",
            "main_menu.items.exit.description", "Close Galactic Planet Miner.", true, false, ft_nullptr, ft_nullptr},
    };

    ft_vector<ft_menu_item> items;
    items.reserve(sizeof(entries) / sizeof(entries[0]));

    for (size_t index = 0; index < sizeof(entries) / sizeof(entries[0]); ++index)
    {
        ft_rect item_rect = base_rect;
        item_rect.top += static_cast<int>(index) * (base_rect.height + spacing);

        const menu_entry &entry = entries[index];
        ft_string          label = menu_localize(entry.label_key, entry.label_fallback);
        ft_menu_item       item(ft_string(entry.identifier), label, item_rect);
        bool item_enabled = entry.enabled;
        if (entry.experimental && !experiments_enabled)
            item_enabled = false;
        item.enabled = item_enabled;

        ft_string description = menu_localize(entry.description_key, entry.description_fallback);
        if (entry.experimental && !experiments_enabled && entry.gated_description_key != ft_nullptr)
            description = menu_localize(entry.gated_description_key, entry.gated_description_fallback);
        item.description = description;
        items.push_back(item);
    }

    return items;
}

ft_string main_menu_format_resume_description(
    const ft_string &slot_label, const ft_string &metadata_label, bool metadata_known)
{
    ft_vector<StringTableReplacement> replacements;
    replacements.reserve(2U);

    StringTableReplacement slot_placeholder;
    slot_placeholder.key = ft_string("slot");
    slot_placeholder.value = slot_label;
    replacements.push_back(slot_placeholder);

    ft_string metadata_value;
    if (!metadata_label.empty())
    {
        metadata_value.append(" ");
        metadata_value.append(metadata_label);
    }

    StringTableReplacement metadata_placeholder;
    metadata_placeholder.key = ft_string("metadata");
    metadata_placeholder.value = metadata_value;
    replacements.push_back(metadata_placeholder);

    ft_string description = menu_localize_format("main_menu.resume.active_description",
        "Jump back into \"{{slot}}\"{{metadata}} without opening the load menu.", replacements);
    if (!metadata_known && metadata_label.empty())
    {
        ft_string metadata_hint
            = menu_localize("main_menu.resume.metadata_unavailable", " Metadata details unavailable.");
        description.append(metadata_hint);
    }

    return description;
}

void main_menu_apply_resume_state(ft_vector<ft_menu_item> &items, bool resume_enabled, const ft_string &slot_label,
    const ft_string &metadata_label, bool metadata_known)
{
    const ft_string resume_identifier("resume");
    for (size_t index = 0; index < items.size(); ++index)
    {
        if (items[index].identifier != resume_identifier)
            continue;

        if (resume_enabled)
        {
            items[index].enabled = true;
            items[index].description = main_menu_format_resume_description(slot_label, metadata_label, metadata_known);
        }
        else
        {
            items[index].enabled = false;
            items[index].description = menu_localize("main_menu.resume.empty_description",
                "No healthy campaign saves found yet. Create or load a game to enable quick resume.");
        }
        break;
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
