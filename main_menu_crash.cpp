#include "main_menu_system.hpp"

#include "build_info.hpp"
#include "menu_localization.hpp"
#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/CPP_class/class_ofstream.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/File/open_dir.hpp"
#include "libft/File/file_utils.hpp"
#include "libft/Printf/printf.hpp"
#include "libft/Time/time.hpp"

namespace
{
    const char *kCrashMetricQueueDirectoryName = "analytics";
    const char *kCrashMetricQueueFilename = "crash_metrics_queue.log";

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

    bool ensure_directory_exists(const ft_string &path) noexcept
    {
        if (path.empty())
            return false;

        int exists_result = file_dir_exists(path.c_str());
        if (exists_result > 0)
            return true;
        if (exists_result < 0)
            return false;

        if (file_create_directory(path.c_str(), 0755) != 0)
            return false;

        return true;
    }

    ft_string resolve_crash_metric_queue_directory(const ft_string &commander_name) noexcept
    {
        ft_string save_directory = player_profile_resolve_save_directory(commander_name);
        if (save_directory.empty())
            return ft_string();

        save_directory.append("/");
        save_directory.append(kCrashMetricQueueDirectoryName);
        return save_directory;
    }

    ft_string resolve_crash_metric_queue_path(const ft_string &commander_name) noexcept
    {
        ft_string directory = resolve_crash_metric_queue_directory(commander_name);
        if (directory.empty())
            return ft_string();

        directory.append("/");
        directory.append(kCrashMetricQueueFilename);
        return directory;
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

bool main_menu_load_crash_metric_queue(const ft_string &commander_name, ft_vector<ft_string> &out_queue) noexcept
{
    out_queue.clear();

    if (commander_name.empty())
        return false;

    ft_string path = resolve_crash_metric_queue_path(commander_name);
    if (path.empty())
        return false;

    FILE *file = ft_fopen(path.c_str(), "r");
    if (file == ft_nullptr)
        return true;

    char buffer[2048];
    while (true)
    {
        char *line = fgets(buffer, sizeof(buffer), file);
        if (line == ft_nullptr)
            break;

        trim_line_buffer(buffer);
        if (buffer[0] == '\0')
            continue;

        out_queue.push_back(ft_string(buffer));
    }

    ft_fclose(file);
    return true;
}

bool main_menu_save_crash_metric_queue(
    const ft_string &commander_name, const ft_vector<ft_string> &queue) noexcept
{
    if (commander_name.empty())
        return false;

    ft_string save_directory = player_profile_resolve_save_directory(commander_name);
    if (save_directory.empty())
        return false;

    if (!ensure_directory_exists(save_directory))
        return false;

    ft_string queue_directory = resolve_crash_metric_queue_directory(commander_name);
    if (queue_directory.empty())
        return false;

    if (!ensure_directory_exists(queue_directory))
        return false;

    ft_string path = resolve_crash_metric_queue_path(commander_name);
    if (path.empty())
        return false;

    if (queue.empty())
    {
        int delete_result = file_delete(path.c_str());
        if (delete_result == 0)
            return true;

        int exists_result = file_exists(path.c_str());
        if (exists_result <= 0)
            return true;
        return false;
    }

    ft_ofstream stream;
    if (stream.open(path.c_str()) != 0)
        return false;

    for (size_t index = 0; index < queue.size(); ++index)
    {
        if (stream.write(queue[index].c_str()) < 0)
        {
            stream.close();
            return false;
        }
        if (stream.write("\n") < 0)
        {
            stream.close();
            return false;
        }
    }

    stream.close();
    return true;
}

bool main_menu_append_crash_cleanup_failure_log(const ft_string &log_path, long timestamp_ms) noexcept
{
    ft_string target_path("test_failures.log");
    FILE     *file_handle = ft_fopen(target_path.c_str(), "a");
    if (file_handle == ft_nullptr)
        return false;

    long seconds_component = timestamp_ms / 1000L;
    long milliseconds_component = timestamp_ms % 1000L;
    if (milliseconds_component < 0)
        milliseconds_component = -milliseconds_component;

    ft_string timestamp_text = time_format_iso8601(static_cast<t_time>(seconds_component));
    if (timestamp_text.empty())
        timestamp_text = ft_to_string(timestamp_ms);

    ft_string build_label = build_info_format_label();

    ft_string entry("[");
    entry.append(timestamp_text);
    entry.append(".");
    if (milliseconds_component < 100)
        entry.append("0");
    if (milliseconds_component < 10)
        entry.append("0");
    entry.append(ft_to_string(static_cast<int>(milliseconds_component)));
    entry.append("] ");
    if (!build_label.empty())
    {
        entry.append("[");
        entry.append(build_label);
        entry.append("] ");
    }
    entry.append("crash report cleanup failed");
    if (!log_path.empty())
    {
        entry.append(" (log: ");
        entry.append(log_path);
        entry.append(")");
    }
    entry.append("\n");

    int write_result = ft_fprintf(file_handle, "%s", entry.c_str());
    ft_fclose(file_handle);
    if (write_result < 0)
        return false;
    if (static_cast<size_t>(write_result) != entry.size())
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

ft_string main_menu_format_crash_metric_payload(const MainMenuCrashReport &report, bool submission_success,
    int status_code, const ft_string &response_body) noexcept
{
    ft_vector<ft_string> segments;
    segments.reserve(7U);

    ft_string result_segment("Result: ");
    if (submission_success)
        result_segment.append("success");
    else
        result_segment.append("failure");
    segments.push_back(result_segment);

    ft_string status_segment("Status: ");
    status_segment.append(ft_to_string(status_code));
    segments.push_back(status_segment);

    if (!report.summary.empty())
    {
        ft_string summary_segment("Summary: ");
        summary_segment.append(sanitize_crash_payload_value(report.summary));
        segments.push_back(summary_segment);
    }

    if (!report.details_path.empty())
    {
        ft_string details_segment("Details: ");
        details_segment.append(sanitize_crash_payload_value(report.details_path));
        segments.push_back(details_segment);
    }

    ft_string timestamp_label = format_crash_timestamp(report.timestamp_ms);
    if (!timestamp_label.empty())
    {
        ft_string timestamp_segment("Timestamp: ");
        timestamp_segment.append(timestamp_label);
        segments.push_back(timestamp_segment);
    }

    if (!response_body.empty())
    {
        ft_string response_segment("Response: ");
        response_segment.append(sanitize_crash_payload_value(response_body));
        segments.push_back(response_segment);
    }

    ft_string build_label = build_info_format_label();
    if (!build_label.empty())
        segments.push_back(build_label);

    ft_string payload;
    for (size_t index = 0U; index < segments.size(); ++index)
    {
        if (index > 0U)
            payload.append("; ");
        payload.append(segments[index]);
    }

    return payload;
}

void main_menu_crash_metrics_enqueue(ft_vector<ft_string> &queue, const ft_string &payload)
{
    queue.push_back(payload);
}

bool main_menu_crash_metrics_flush(ft_vector<ft_string> &queue, const ft_string &host, const ft_string &path,
    MainMenuCrashMetricSubmitter submitter) noexcept
{
    if (submitter == ft_nullptr)
        return false;

    while (!queue.empty())
    {
        ft_string payload(queue[0]);
        int       status_code = 0;
        if (!submitter(host, path, payload, status_code))
            return false;
        queue.erase(queue.begin());
    }

    return true;
}

void main_menu_build_crash_prompt_overlay(const MainMenuCrashReport &report,
    const MainMenuConnectivityStatus &connectivity, bool cleanup_retry_available,
    MainMenuOverlayContext &out_overlay) noexcept
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

    if (cleanup_retry_available)
    {
        out_overlay.heading = menu_localize("main_menu.crash.cleanup_heading", "Crash Log Cleanup Needed");
        out_overlay.lines.push_back(menu_localize("main_menu.crash.cleanup_help",
            "The crash report was submitted, but the local log still needs to be cleared."));
        out_overlay.footer = menu_localize("main_menu.crash.instructions_cleanup_retry",
            "Press Enter / Space, click, or press A to retry cleanup. Press Esc / B to dismiss.");
    }
    else if (connectivity.state == MAIN_MENU_CONNECTIVITY_ONLINE)
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

    ft_string format_crash_metric_payload(
        const MainMenuCrashReport &report, bool submission_success, int status_code, const ft_string &response_body)
    {
        return main_menu_format_crash_metric_payload(report, submission_success, status_code, response_body);
    }

    void build_crash_prompt_overlay(
        const MainMenuCrashReport &report, const MainMenuConnectivityStatus &connectivity, bool cleanup_retry_available,
        MainMenuOverlayContext &out_overlay)
    {
        main_menu_build_crash_prompt_overlay(report, connectivity, cleanup_retry_available, out_overlay);
    }
}
