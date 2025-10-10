#include "main_menu_system.hpp"

#include "build_info.hpp"
#include "menu_localization.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/File/file_utils.hpp"
#include "libft/Time/time.hpp"

namespace
{
    ft_string format_crash_timestamp(long timestamp_ms)
    {
        if (timestamp_ms <= 0)
            return menu_localize("main_menu.crash.timestamp_unknown", "Unknown time");

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

    void trim_line_buffer(char *buffer)
    {
        if (buffer == ft_nullptr)
            return;
        size_t length = ft_strlen(buffer);
        while (length > 0)
        {
            char character = buffer[length - 1];
            if (character != '\n' && character != '\r')
                break;
            buffer[length - 1] = '\0';
            length -= 1;
        }
    }

    ft_string sanitize_crash_payload_value(const ft_string &value)
    {
        ft_string sanitized;
        for (size_t index = 0; index < value.size(); ++index)
        {
            char character = value[index];
            if (character == '\n' || character == '\r' || character == ';' || character == '=')
                sanitized.append(' ');
            else
                sanitized.append(character);
        }
        return sanitized;
    }
}

bool main_menu_load_crash_report(const ft_string &log_path, MainMenuCrashReport &out_report) noexcept
{
    out_report = MainMenuCrashReport();
    if (log_path.empty())
        return false;

    FILE *file = ft_fopen(log_path.c_str(), "r");
    if (file == ft_nullptr)
        return false;

    char     buffer[1024];
    ft_string lines[3];
    size_t    line_index = 0U;

    while (line_index < 3U)
    {
        char *result = fgets(buffer, sizeof(buffer), file);
        if (result == ft_nullptr)
            break;
        trim_line_buffer(buffer);
        lines[line_index] = ft_string(buffer);
        line_index += 1U;
    }

    ft_fclose(file);

    if (line_index == 0U)
        return false;

    out_report.available = true;
    out_report.summary = lines[0];
    if (line_index > 1U)
        out_report.details_path = lines[1];
    if (line_index > 2U)
    {
        out_report.timestamp_ms = ft_atol(lines[2].c_str());
        if (out_report.timestamp_ms < 0L)
            out_report.timestamp_ms = 0L;
    }

    return true;
}

bool main_menu_clear_crash_report(const ft_string &log_path) noexcept
{
    if (log_path.empty())
        return false;

    if (file_delete(log_path.c_str()) != 0)
        return false;

    return true;
}

ft_string main_menu_format_crash_submission_payload(const MainMenuCrashReport &report) noexcept
{
    ft_string payload;

    if (!report.summary.empty())
    {
        payload.append("Summary: ");
        payload.append(sanitize_crash_payload_value(report.summary));
        payload.append("; ");
    }

    if (!report.details_path.empty())
    {
        payload.append("Details: ");
        payload.append(sanitize_crash_payload_value(report.details_path));
        payload.append("; ");
    }

    ft_string timestamp_label = format_crash_timestamp(report.timestamp_ms);
    if (!timestamp_label.empty())
    {
        payload.append("Timestamp: ");
        payload.append(timestamp_label);
        payload.append("; ");
    }

    ft_string build_label = build_info_format_label();
    if (!build_label.empty())
        payload.append(build_label);

    return payload;
}

void main_menu_build_crash_prompt_overlay(const MainMenuCrashReport &report,
    const MainMenuConnectivityStatus &connectivity, MainMenuOverlayContext &out_overlay) noexcept
{
    out_overlay.visible = false;
    out_overlay.heading.clear();
    out_overlay.lines.clear();
    out_overlay.footer.clear();

    if (!report.available)
        return;

    ft_vector<StringTableReplacement> timestamp_replacements;
    timestamp_replacements.reserve(1U);
    StringTableReplacement timestamp_placeholder;
    timestamp_placeholder.key = ft_string("timestamp");
    timestamp_placeholder.value = format_crash_timestamp(report.timestamp_ms);
    timestamp_replacements.push_back(timestamp_placeholder);
    ft_string timestamp_line
        = menu_localize_format("main_menu.crash.timestamp_line", "Crash detected at {{timestamp}}.", timestamp_replacements);
    out_overlay.lines.push_back(timestamp_line);

    if (!report.summary.empty())
    {
        ft_vector<StringTableReplacement> summary_replacements;
        summary_replacements.reserve(1U);
        StringTableReplacement summary_placeholder;
        summary_placeholder.key = ft_string("summary");
        summary_placeholder.value = report.summary;
        summary_replacements.push_back(summary_placeholder);
        ft_string summary_line
            = menu_localize_format("main_menu.crash.summary_line", "Summary: {{summary}}", summary_replacements);
        out_overlay.lines.push_back(summary_line);
    }
    else
    {
        out_overlay.lines.push_back(
            menu_localize("main_menu.crash.summary_missing", "Summary details unavailable."));
    }

    if (!report.details_path.empty())
    {
        ft_vector<StringTableReplacement> detail_replacements;
        detail_replacements.reserve(1U);
        StringTableReplacement path_placeholder;
        path_placeholder.key = ft_string("path");
        path_placeholder.value = report.details_path;
        detail_replacements.push_back(path_placeholder);
        ft_string details_line
            = menu_localize_format("main_menu.crash.details_line", "Details stored at {{path}}.", detail_replacements);
        out_overlay.lines.push_back(details_line);
    }

    if (connectivity.state == MAIN_MENU_CONNECTIVITY_ONLINE)
    {
        out_overlay.heading = menu_localize("main_menu.crash.heading", "Crash Report Ready");
        out_overlay.lines.push_back(menu_localize(
            "main_menu.crash.help_line", "Submitting crash reports helps us improve stability."));
        out_overlay.footer = menu_localize("main_menu.crash.instructions_online",
            "Press Enter / Space, click, or press A to submit. Press Esc / B to dismiss.");
    }
    else
    {
        out_overlay.heading = menu_localize("main_menu.crash.offline_heading", "Crash Report Pending");
        out_overlay.lines.push_back(
            menu_localize("main_menu.crash.offline_help", "Reconnect to submit this report."));
        out_overlay.footer = menu_localize("main_menu.crash.instructions_offline",
            "Reconnect to submit this report. Press Esc / B to dismiss.");
    }

    out_overlay.visible = true;
}

namespace main_menu_testing
{
    ft_string format_crash_submission_payload(const MainMenuCrashReport &report)
    {
        return main_menu_format_crash_submission_payload(report);
    }

    void build_crash_prompt_overlay(
        const MainMenuCrashReport &report, const MainMenuConnectivityStatus &connectivity, MainMenuOverlayContext &out_overlay)
    {
        main_menu_build_crash_prompt_overlay(report, connectivity, out_overlay);
    }
}
