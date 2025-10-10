#include "main_menu_system.hpp"

#include "menu_localization.hpp"
#include "libft/Libft/libft.hpp"

namespace
{
    const long kAutosaveResultVisibleDurationMs = 4000;

    ft_string format_autosave_slot_suffix(const ft_string &slot_name)
    {
        if (slot_name.empty())
            return ft_string();

        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement slot_placeholder;
        slot_placeholder.key = ft_string("slot");
        slot_placeholder.value = slot_name;
        replacements.push_back(slot_placeholder);
        return menu_localize_format("main_menu.autosave.slot_suffix", " ({{slot}})", replacements);
    }
}

void main_menu_mark_autosave_in_progress(
    MainMenuAutosaveStatus &status, const ft_string &slot_name, long timestamp_ms) noexcept
{
    status.state = MAIN_MENU_AUTOSAVE_IN_PROGRESS;
    status.active_slot = slot_name;
    status.last_error.clear();
    status.last_change_ms = timestamp_ms;
}

void main_menu_mark_autosave_result(MainMenuAutosaveStatus &status, bool success, const ft_string &slot_name,
    const ft_string &error_message, long timestamp_ms) noexcept
{
    status.active_slot = slot_name;
    status.last_change_ms = timestamp_ms;
    if (success)
    {
        status.state = MAIN_MENU_AUTOSAVE_SUCCEEDED;
        status.last_error.clear();
    }
    else
    {
        status.state = MAIN_MENU_AUTOSAVE_FAILED;
        status.last_error = error_message;
    }
}

void main_menu_autosave_tick(MainMenuAutosaveStatus &status, long timestamp_ms) noexcept
{
    if (status.state != MAIN_MENU_AUTOSAVE_SUCCEEDED && status.state != MAIN_MENU_AUTOSAVE_FAILED)
        return;

    long delta = timestamp_ms - status.last_change_ms;
    if (delta < 0)
        delta = 0;
    if (delta >= kAutosaveResultVisibleDurationMs)
    {
        status.state = MAIN_MENU_AUTOSAVE_IDLE;
        status.active_slot.clear();
        status.last_error.clear();
        status.last_change_ms = timestamp_ms;
    }
}

bool main_menu_autosave_is_visible(const MainMenuAutosaveStatus &status, long timestamp_ms) noexcept
{
    if (status.state == MAIN_MENU_AUTOSAVE_IDLE)
        return false;
    if (status.state == MAIN_MENU_AUTOSAVE_IN_PROGRESS)
        return true;

    long delta = timestamp_ms - status.last_change_ms;
    if (delta < 0)
        delta = 0;
    return delta < kAutosaveResultVisibleDurationMs;
}

ft_string main_menu_resolve_autosave_label(const MainMenuAutosaveStatus &status)
{
    if (status.state == MAIN_MENU_AUTOSAVE_IDLE)
        return ft_string();

    ft_string slot_suffix = format_autosave_slot_suffix(status.active_slot);
    ft_vector<StringTableReplacement> replacements;
    if (!slot_suffix.empty())
    {
        StringTableReplacement slot_placeholder;
        slot_placeholder.key = ft_string("slot");
        slot_placeholder.value = slot_suffix;
        replacements.push_back(slot_placeholder);
    }

    ft_string label;
    if (status.state == MAIN_MENU_AUTOSAVE_IN_PROGRESS)
        label = menu_localize_format("main_menu.autosave.in_progress", "Autosaving{{slot}}...", replacements);
    else if (status.state == MAIN_MENU_AUTOSAVE_SUCCEEDED)
        label = menu_localize_format("main_menu.autosave.success", "Autosave complete{{slot}}.", replacements);
    else
    {
        label = menu_localize_format("main_menu.autosave.failure", "Autosave failed{{slot}}.", replacements);
        if (!status.last_error.empty())
        {
            label.append(" ");
            label.append(status.last_error);
        }
    }

    return label;
}
