#include "main_menu_system.hpp"

#include "build_info.hpp"
#include "menu_localization.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/File/file_utils.hpp"
#include "libft/Time/time.hpp"
#include "libft/Printf/printf.hpp"

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
}
