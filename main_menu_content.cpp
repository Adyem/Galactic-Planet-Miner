#include "main_menu_system.hpp"

#include "app_constants.hpp"
#include "menu_localization.hpp"

#include "libft/File/file_utils.hpp"
#include "libft/Libft/libft.hpp"

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
