#include "game_test_menu_shared.hpp"

int verify_main_menu_connectivity_indicator()
{
    MainMenuConnectivityStatus status;
    main_menu_testing::mark_connectivity_checking(status, 120);

    ft_string checking_label = main_menu_testing::resolve_connectivity_label(status);
    FT_ASSERT_EQ(ft_string("Checking backend..."), checking_label);
    SDL_Color checking_color = main_menu_testing::resolve_connectivity_color(status);
    FT_ASSERT_EQ(182, static_cast<int>(checking_color.r));
    FT_ASSERT_EQ(192, static_cast<int>(checking_color.g));
    FT_ASSERT_EQ(212, static_cast<int>(checking_color.b));
    FT_ASSERT_EQ(255, static_cast<int>(checking_color.a));

    main_menu_testing::apply_connectivity_result(status, true, 204, 240);
    ft_string online_label = main_menu_testing::resolve_connectivity_label(status);
    FT_ASSERT_EQ(ft_string("Backend Online (HTTP 204)"), online_label);
    SDL_Color online_color = main_menu_testing::resolve_connectivity_color(status);
    FT_ASSERT_EQ(108, static_cast<int>(online_color.r));
    FT_ASSERT_EQ(210, static_cast<int>(online_color.g));
    FT_ASSERT_EQ(156, static_cast<int>(online_color.b));
    FT_ASSERT_EQ(255, static_cast<int>(online_color.a));

    main_menu_testing::apply_connectivity_result(status, false, 503, 360);
    ft_string offline_label = main_menu_testing::resolve_connectivity_label(status);
    FT_ASSERT_EQ(ft_string("Backend Offline (HTTP 503)"), offline_label);
    SDL_Color offline_color = main_menu_testing::resolve_connectivity_color(status);
    FT_ASSERT_EQ(220, static_cast<int>(offline_color.r));
    FT_ASSERT_EQ(120, static_cast<int>(offline_color.g));
    FT_ASSERT_EQ(120, static_cast<int>(offline_color.b));
    FT_ASSERT_EQ(255, static_cast<int>(offline_color.a));

    main_menu_testing::apply_connectivity_result(status, false, 0, 480);
    ft_string offline_no_code = main_menu_testing::resolve_connectivity_label(status);
    FT_ASSERT_EQ(ft_string("Backend Offline"), offline_no_code);

    return 1;
}

int verify_main_menu_connectivity_failure_logging()
{
    const ft_string log_path("tests/tmp_backend_ping_failures.log");
    const ft_string host("127.0.0.1:8080");
    const long      timestamp_ms = 1717171717123L;
    const int       status_code = 503;

    file_delete(log_path.c_str());

    FT_ASSERT(main_menu_testing::append_connectivity_failure_log(host, status_code, timestamp_ms, log_path));

    FILE *log_file = ft_fopen(log_path.c_str(), "r");
    FT_ASSERT(log_file != ft_nullptr);

    char   buffer[256];
    size_t read_count = fread(buffer, 1, sizeof(buffer) - 1, log_file);
    buffer[read_count] = '\0';
    ft_fclose(log_file);

    ft_string contents(buffer);
    ft_string expected("[2024-05-31T16:08:37Z.123] ");
    expected.append("[");
    expected.append(build_info_format_label());
    expected.append("] backend ping to 127.0.0.1:8080 failed (HTTP 503)\n");
    FT_ASSERT_EQ(expected, contents);

    FT_ASSERT_EQ(0, file_delete(log_path.c_str()));

    return 1;
}

int verify_main_menu_crash_prompt()
{
    const ft_string log_path("tests/tmp_crash_report.log");
    file_delete(log_path.c_str());

    ft_ofstream crash_stream;
    FT_ASSERT_EQ(0, crash_stream.open(log_path.c_str()));
    ft_string crash_contents("1717171717123\n");
    crash_contents.append("Segfault near asteroid belt\n");
    crash_contents.append("logs/crash_dump.txt\n");
    FT_ASSERT(crash_stream.write(crash_contents.c_str()) >= 0);
    crash_stream.close();

    MainMenuCrashReport report;
    FT_ASSERT(main_menu_load_crash_report(log_path, report));
    FT_ASSERT(report.available);
    FT_ASSERT_EQ(1717171717123L, report.timestamp_ms);
    FT_ASSERT_EQ(ft_string("Segfault near asteroid belt"), report.summary);
    FT_ASSERT_EQ(ft_string("logs/crash_dump.txt"), report.details_path);

    long seconds_component = report.timestamp_ms / 1000L;
    long milliseconds_component = report.timestamp_ms % 1000L;
    if (milliseconds_component < 0)
        milliseconds_component = -milliseconds_component;
    ft_string iso_timestamp = time_format_iso8601(static_cast<t_time>(seconds_component));
    if (iso_timestamp.empty())
        iso_timestamp = ft_to_string(report.timestamp_ms);
    ft_string expected_timestamp_value(iso_timestamp);
    expected_timestamp_value.append(".");
    if (milliseconds_component < 100)
        expected_timestamp_value.append("0");
    if (milliseconds_component < 10)
        expected_timestamp_value.append("0");
    expected_timestamp_value.append(ft_to_string(static_cast<int>(milliseconds_component)));
    ft_vector<StringTableReplacement> timestamp_replacements;
    timestamp_replacements.reserve(1U);
    StringTableReplacement timestamp_placeholder;
    timestamp_placeholder.key = ft_string("timestamp");
    timestamp_placeholder.value = expected_timestamp_value;
    timestamp_replacements.push_back(timestamp_placeholder);
    ft_string expected_timestamp_line = menu_localize_format(
        "main_menu.crash.timestamp_line", "Crash detected at {{timestamp}}.", timestamp_replacements);

    MainMenuOverlayContext      overlay;
    MainMenuConnectivityStatus  status;
    status.state = MAIN_MENU_CONNECTIVITY_OFFLINE;
    main_menu_testing::build_crash_prompt_overlay(report, status, overlay);
    FT_ASSERT(overlay.visible);
    FT_ASSERT_EQ(ft_string("Crash Report Pending"), overlay.heading);
    FT_ASSERT_EQ(4U, overlay.lines.size());
    FT_ASSERT_EQ(expected_timestamp_line, overlay.lines[0]);
    FT_ASSERT_EQ(ft_string("Summary: Segfault near asteroid belt"), overlay.lines[1]);
    FT_ASSERT_EQ(ft_string("Details stored at logs/crash_dump.txt."), overlay.lines[2]);
    FT_ASSERT_EQ(ft_string("Reconnect to submit this report."), overlay.lines[3]);
    FT_ASSERT_EQ(ft_string("Reconnect to submit this report. Press Esc / B to dismiss."), overlay.footer);

    status.state = MAIN_MENU_CONNECTIVITY_ONLINE;
    overlay = MainMenuOverlayContext();
    main_menu_testing::build_crash_prompt_overlay(report, status, overlay);
    FT_ASSERT(overlay.visible);
    FT_ASSERT_EQ(ft_string("Crash Report Ready"), overlay.heading);
    FT_ASSERT_EQ(4U, overlay.lines.size());
    FT_ASSERT_EQ(expected_timestamp_line, overlay.lines[0]);
    FT_ASSERT_EQ(ft_string("Summary: Segfault near asteroid belt"), overlay.lines[1]);
    FT_ASSERT_EQ(ft_string("Details stored at logs/crash_dump.txt."), overlay.lines[2]);
    FT_ASSERT_EQ(ft_string("Submitting crash reports helps us improve stability."), overlay.lines[3]);
    FT_ASSERT_EQ(ft_string("Press Enter / Space, click, or press A to submit. Press Esc / B to dismiss."), overlay.footer);

    ft_string payload = main_menu_testing::format_crash_submission_payload(report);
    ft_string expected_payload(
        "timestamp_ms=1717171717123;summary=Segfault near asteroid belt;details=logs/crash_dump.txt;build=");
    expected_payload.append(build_info_format_label());
    FT_ASSERT_EQ(expected_payload, payload);

    FT_ASSERT(main_menu_clear_crash_report(log_path));
    FT_ASSERT_EQ(false, file_exists(log_path.c_str()));

    file_delete(log_path.c_str());

    return 1;
}

#include "game_test_menu_connectivity_panels.cpp"
