#include "main_menu_system.hpp"

#include "build_info.hpp"
#include "menu_localization.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/File/file_utils.hpp"
#include "libft/Time/time.hpp"
#include "libft/Printf/printf.hpp"

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

    const MainMenuSyncEntry &resolve_sync_entry(const MainMenuSyncStatus &status, e_main_menu_sync_channel channel) noexcept
    {
        if (channel == MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS)
            return status.leaderboards;
        return status.convoys;
    }

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
            entry.append(" (no response)");
        entry.append("\n");

        write_result = ft_fprintf(file_handle, "%s", entry.c_str());
        ft_fclose(file_handle);
        if (write_result < 0)
            return false;
        if (static_cast<size_t>(write_result) != entry.size())
            return false;
        return true;
    }
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
        return menu_localize("main_menu.connectivity.checking", "Checking backend...");

    ft_string label;
    if (status.state == MAIN_MENU_CONNECTIVITY_ONLINE)
        label = menu_localize("main_menu.connectivity.online", "Backend Online");
    else
        label = menu_localize("main_menu.connectivity.offline", "Backend Offline");

    if (status.last_status_code != 0)
    {
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement code_placeholder;
        code_placeholder.key = ft_string("code");
        code_placeholder.value = ft_to_string(status.last_status_code);
        replacements.push_back(code_placeholder);
        ft_string suffix
            = menu_localize_format("main_menu.connectivity.code_suffix", " (HTTP {{code}})", replacements);
        label.append(suffix);
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

bool main_menu_append_connectivity_failure_log(const ft_string &host, int status_code, long timestamp_ms) noexcept
{
    ft_string log_path("test_failures.log");

    return write_connectivity_failure_entry(host, status_code, timestamp_ms, log_path);
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

    void apply_connectivity_result(
        MainMenuConnectivityStatus &status, bool success, int status_code, long timestamp_ms)
    {
        main_menu_apply_connectivity_result(status, success, status_code, timestamp_ms);
    }

    bool append_connectivity_failure_log(
        const ft_string &host, int status_code, long timestamp_ms, const ft_string &log_path)
    {
        return write_connectivity_failure_entry(host, status_code, timestamp_ms, log_path);
    }

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
