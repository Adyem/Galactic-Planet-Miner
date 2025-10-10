#include "main_menu_system.hpp"

#include "menu_localization.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/Time/time.hpp"

namespace
{
    ft_string resolve_sync_channel_label(e_main_menu_sync_channel channel) noexcept
    {
        if (channel == MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS)
            return menu_localize("main_menu.sync.channel.leaderboards", "Leaderboards");
        return menu_localize("main_menu.sync.channel.convoys", "Convoys");
    }

    ft_string format_sync_code_suffix(int status_code)
    {
        if (status_code == 0)
            return ft_string();

        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);

        StringTableReplacement code_placeholder;
        code_placeholder.key = ft_string("code");
        code_placeholder.value = ft_to_string(status_code);
        replacements.push_back(code_placeholder);
        return menu_localize_format("main_menu.sync.code_suffix", " (HTTP {{code}})", replacements);
    }

    ft_string format_sync_timestamp(long timestamp_ms)
    {
        if (timestamp_ms <= 0)
            return menu_localize("main_menu.sync.timestamp.never", "Never");

        long seconds_component = timestamp_ms / 1000L;
        long milliseconds_component = timestamp_ms % 1000L;
        if (milliseconds_component < 0)
            milliseconds_component = -milliseconds_component;

        ft_string iso_text = time_format_iso8601(static_cast<t_time>(seconds_component));
        if (iso_text.empty())
            iso_text = ft_to_string(timestamp_ms);

        ft_string formatted(iso_text);
        formatted.append(".");
        if (milliseconds_component < 100)
            formatted.append("0");
        if (milliseconds_component < 10)
            formatted.append("0");
        formatted.append(ft_to_string(static_cast<int>(milliseconds_component)));
        return formatted;
    }

    MainMenuSyncEntry &resolve_sync_entry(MainMenuSyncStatus &status, e_main_menu_sync_channel channel) noexcept
    {
        if (channel == MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS)
            return status.leaderboards;
        return status.convoys;
    }

    const MainMenuSyncEntry &resolve_sync_entry(
        const MainMenuSyncStatus &status, e_main_menu_sync_channel channel) noexcept
    {
        if (channel == MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS)
            return status.leaderboards;
        return status.convoys;
    }
}

void main_menu_sync_begin(MainMenuSyncStatus &status, e_main_menu_sync_channel channel, long timestamp_ms) noexcept
{
    MainMenuSyncEntry &entry = resolve_sync_entry(status, channel);
    entry.state = MAIN_MENU_SYNC_CHECKING;
    entry.last_attempt_ms = timestamp_ms;
}

void main_menu_sync_apply(MainMenuSyncStatus &status, e_main_menu_sync_channel channel, bool success, int status_code,
    long timestamp_ms) noexcept
{
    MainMenuSyncEntry &entry = resolve_sync_entry(status, channel);
    entry.last_attempt_ms = timestamp_ms;
    entry.last_status_code = status_code;
    if (success)
    {
        entry.state = MAIN_MENU_SYNC_SUCCESS;
        entry.last_success_ms = timestamp_ms;
    }
    else
    {
        entry.state = MAIN_MENU_SYNC_FAILURE;
        entry.last_failure_ms = timestamp_ms;
    }
}

ft_string main_menu_resolve_sync_entry_label(const MainMenuSyncStatus &status, e_main_menu_sync_channel channel) noexcept
{
    const MainMenuSyncEntry &entry = resolve_sync_entry(status, channel);
    ft_vector<StringTableReplacement> replacements;
    replacements.reserve(4U);

    StringTableReplacement channel_placeholder;
    channel_placeholder.key = ft_string("channel");
    channel_placeholder.value = resolve_sync_channel_label(channel);
    replacements.push_back(channel_placeholder);

    if (entry.state == MAIN_MENU_SYNC_CHECKING)
        return menu_localize_format("main_menu.sync.entry.checking", "{{channel}}: Sync in progress...", replacements);

    if (entry.state == MAIN_MENU_SYNC_SUCCESS)
    {
        StringTableReplacement timestamp_placeholder;
        timestamp_placeholder.key = ft_string("timestamp");
        timestamp_placeholder.value = format_sync_timestamp(entry.last_success_ms);
        replacements.push_back(timestamp_placeholder);

        StringTableReplacement code_placeholder;
        code_placeholder.key = ft_string("code");
        code_placeholder.value = format_sync_code_suffix(entry.last_status_code);
        replacements.push_back(code_placeholder);

        return menu_localize_format(
            "main_menu.sync.entry.updated", "{{channel}}: Updated at {{timestamp}}{{code}}", replacements);
    }

    if (entry.state == MAIN_MENU_SYNC_FAILURE)
    {
        StringTableReplacement timestamp_placeholder;
        timestamp_placeholder.key = ft_string("timestamp");
        timestamp_placeholder.value = format_sync_timestamp(entry.last_failure_ms);
        replacements.push_back(timestamp_placeholder);

        StringTableReplacement code_placeholder;
        code_placeholder.key = ft_string("code");
        code_placeholder.value = format_sync_code_suffix(entry.last_status_code);
        replacements.push_back(code_placeholder);

        if (entry.last_success_ms > 0)
        {
            StringTableReplacement previous_placeholder;
            previous_placeholder.key = ft_string("previous");
            previous_placeholder.value = format_sync_timestamp(entry.last_success_ms);
            replacements.push_back(previous_placeholder);
            return menu_localize_format("main_menu.sync.entry.failed_with_previous",
                "{{channel}}: Failed at {{timestamp}}{{code}} (last success {{previous}})", replacements);
        }

        return menu_localize_format(
            "main_menu.sync.entry.failed", "{{channel}}: Failed at {{timestamp}}{{code}}", replacements);
    }

    if (entry.last_success_ms > 0)
    {
        StringTableReplacement timestamp_placeholder;
        timestamp_placeholder.key = ft_string("timestamp");
        timestamp_placeholder.value = format_sync_timestamp(entry.last_success_ms);
        replacements.push_back(timestamp_placeholder);

        StringTableReplacement code_placeholder;
        code_placeholder.key = ft_string("code");
        code_placeholder.value = format_sync_code_suffix(entry.last_status_code);
        replacements.push_back(code_placeholder);

        return menu_localize_format(
            "main_menu.sync.entry.updated", "{{channel}}: Updated at {{timestamp}}{{code}}", replacements);
    }

    return menu_localize_format(
        "main_menu.sync.entry.never", "{{channel}}: Awaiting first sync.", replacements);
}

namespace main_menu_testing
{
    ft_string resolve_sync_entry_label(const MainMenuSyncStatus &status, e_main_menu_sync_channel channel)
    {
        return main_menu_resolve_sync_entry_label(status, channel);
    }

    void begin_sync(MainMenuSyncStatus &status, e_main_menu_sync_channel channel, long timestamp_ms)
    {
        main_menu_sync_begin(status, channel, timestamp_ms);
    }

    void apply_sync(
        MainMenuSyncStatus &status, e_main_menu_sync_channel channel, bool success, int status_code, long timestamp_ms)
    {
        main_menu_sync_apply(status, channel, success, status_code, timestamp_ms);
    }
}
