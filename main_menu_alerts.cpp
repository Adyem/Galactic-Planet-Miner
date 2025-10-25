                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(2U);
                StringTableReplacement count_placeholder;
                count_placeholder.key = ft_string("count");
                count_placeholder.value = ft_to_string(static_cast<int>(additional));
                replacements.push_back(count_placeholder);
                StringTableReplacement suffix_placeholder;
                suffix_placeholder.key = ft_string("suffix");
                if (additional > 1U)
                    suffix_placeholder.value = ft_string("s");
                replacements.push_back(suffix_placeholder);
                ft_string extra = menu_localize_format("main_menu.alerts.audit_additional",
                    " ({{count}} more issue{{suffix}})", replacements);
                alert_banner.message.append(extra);
            }
        }
        else
        {
            alert_banner.visible = false;
            alert_banner.is_error = false;
            alert_banner.message.clear();
        }

        ft_string latest_slot;
        ft_string latest_path;
        ft_string latest_metadata;
        bool      latest_metadata_available = false;
        if (resolve_latest_resume_slot(active_profile_name, latest_slot, latest_path, latest_metadata,
                latest_metadata_available))
        {
            resume_available = true;
            resume_slot_label = latest_slot;
            resume_save_path = latest_path;
            resume_metadata_label = latest_metadata;
            resume_metadata_known = latest_metadata_available;
            if (!active_profile_name.empty())
                main_menu_preload_commander_portrait(active_profile_name);
        }
        else
        {
            resume_available = false;
            resume_metadata_known = false;
            resume_slot_label.clear();
            resume_save_path.clear();
            resume_metadata_label.clear();
        }

        update_resume_menu_entry();
    };

    auto clear_crash_prompt_overlay = [&]() {
        crash_prompt_context.visible = false;
        crash_prompt_context.heading.clear();
        crash_prompt_context.lines.clear();
        crash_prompt_context.footer.clear();
        crash_cleanup_retry_available = false;
    };

    auto refresh_crash_prompt = [&](bool force_reload) {
        long current_ms = ft_time_ms();
        if (!force_reload && current_ms < next_crash_prompt_check_ms)
        {
            if (crash_report_status.available && !crash_prompt_dismissed)
                main_menu_build_crash_prompt_overlay(
                    crash_report_status, connectivity_status, crash_cleanup_retry_available, crash_prompt_context);
            return;
        }

        next_crash_prompt_check_ms = current_ms + crash_prompt_poll_interval_ms;

        MainMenuCrashReport loaded_report;
        bool                loaded = main_menu_load_crash_report(crash_report_log_path, loaded_report);
        bool                previously_visible = crash_report_status.available && !crash_prompt_dismissed;

        if (!loaded)
        {
            crash_report_status = MainMenuCrashReport();
            crash_prompt_last_timestamp = 0L;
            crash_prompt_last_summary.clear();
            crash_prompt_last_details.clear();
            crash_prompt_dismissed = false;
            clear_crash_prompt_overlay();
            crash_prompt_focus_pending = false;
            return;
        }

        bool is_new_entry = !crash_report_status.available
            || loaded_report.timestamp_ms != crash_prompt_last_timestamp
            || loaded_report.summary != crash_prompt_last_summary
            || loaded_report.details_path != crash_prompt_last_details;

        crash_report_status = loaded_report;
        crash_prompt_last_timestamp = loaded_report.timestamp_ms;
        crash_prompt_last_summary = loaded_report.summary;
        crash_prompt_last_details = loaded_report.details_path;

        if (is_new_entry)
        {
            crash_prompt_dismissed = false;
            crash_cleanup_retry_available = false;
        }

        if (crash_prompt_dismissed)
        {
            clear_crash_prompt_overlay();
            crash_prompt_focus_pending = false;
            return;
        }

        main_menu_build_crash_prompt_overlay(
            crash_report_status, connectivity_status, crash_cleanup_retry_available, crash_prompt_context);
        bool currently_visible = crash_prompt_context.visible;
        if (!currently_visible)
            return;
        if (is_new_entry && !previously_visible)
            crash_prompt_focus_pending = true;
    };

    auto dismiss_crash_prompt = [&]() -> bool {
        if (!crash_prompt_context.visible)
            return false;
        clear_crash_prompt_overlay();
        crash_prompt_dismissed = true;
        crash_prompt_focus_pending = false;
        return true;
    };

    auto retry_crash_cleanup = [&]() -> bool {
        if (!crash_prompt_context.visible)
            return false;

        alert_banner.visible = true;
        if (!main_menu_clear_crash_report(crash_report_log_path))
        {
            alert_banner.is_error = true;
            alert_banner.message = menu_localize("main_menu.crash.clear_failure",
                "Crash report submitted, but the local log could not be cleared. Please try again.");
            main_menu_append_crash_cleanup_failure_log(crash_report_log_path, ft_time_ms());
            crash_prompt_dismissed = false;
            crash_cleanup_retry_available = true;
            crash_prompt_focus_pending = true;
            main_menu_build_crash_prompt_overlay(
                crash_report_status, connectivity_status, crash_cleanup_retry_available, crash_prompt_context);
        }
        else
        {
            alert_banner.is_error = false;
            alert_banner.message
                = menu_localize("main_menu.crash.clear_success", "Crash report log cleared. Thank you!");
            crash_report_status = MainMenuCrashReport();
            crash_prompt_last_timestamp = 0L;
            crash_prompt_last_summary.clear();
            crash_prompt_last_details.clear();
            crash_prompt_dismissed = false;
            crash_cleanup_retry_available = false;
            clear_crash_prompt_overlay();
            crash_prompt_focus_pending = false;
        }

        next_crash_prompt_check_ms = 0L;
        return true;
    };

    auto flush_crash_metric_queue = [&]() {
        if (!active_preferences.analytics_opt_in)
            return;
        if (crash_metric_queue.empty())
            return;
        if (connectivity_status.state != MAIN_MENU_CONNECTIVITY_ONLINE)
            return;

        main_menu_crash_metrics_flush(
            crash_metric_queue, backend_host, backend_crash_metrics_path, backend_client_submit_crash_metric);
        persist_crash_metric_queue();
    };

    auto submit_crash_report = [&]() -> bool {
        if (!crash_prompt_context.visible)
            return false;

        if (crash_cleanup_retry_available)
            return retry_crash_cleanup();

        if (!crash_report_status.available)
        {
            clear_crash_prompt_overlay();
            crash_prompt_dismissed = false;
            crash_prompt_focus_pending = false;
            alert_banner.visible = true;
            alert_banner.is_error = true;
            alert_banner.message
                = menu_localize("main_menu.crash.log_missing", "Crash report details were unavailable.");
            crash_cleanup_retry_available = false;
            return true;
        }

        if (connectivity_status.state != MAIN_MENU_CONNECTIVITY_ONLINE)
        {
            alert_banner.visible = true;
            alert_banner.is_error = true;
            alert_banner.message = menu_localize("main_menu.crash.requires_online",
                "Crash reports require an online connection. Reconnect and try again.");
            crash_cleanup_retry_available = false;
            return true;
        }

        ft_string payload = main_menu_format_crash_submission_payload(crash_report_status);
        ft_string response_body;
        int       status_code = 0;
        bool      success = backend_client_submit_crash_report(
            backend_host, backend_crash_report_path, payload, response_body, status_code);

        if (active_preferences.analytics_opt_in)
        {
            flush_crash_metric_queue();
            ft_string metric_payload = main_menu_format_crash_metric_payload(
                crash_report_status, success, status_code, response_body);
            int metric_status_code = 0;
            bool metric_success = backend_client_submit_crash_metric(
                backend_host, backend_crash_metrics_path, metric_payload, metric_status_code);
            if (!metric_success)
            {
                main_menu_crash_metrics_enqueue(crash_metric_queue, metric_payload);
                persist_crash_metric_queue();
            }
            (void)metric_status_code;
        }

        alert_banner.visible = true;
        if (success)
        {
            if (!main_menu_clear_crash_report(crash_report_log_path))
            {
                alert_banner.is_error = true;
                alert_banner.message = menu_localize("main_menu.crash.clear_failure",
                    "Crash report submitted, but the local log could not be cleared. Please try again.");
                main_menu_append_crash_cleanup_failure_log(crash_report_log_path, ft_time_ms());
                crash_prompt_dismissed = false;
                crash_cleanup_retry_available = true;
                crash_prompt_focus_pending = true;
                main_menu_build_crash_prompt_overlay(
                    crash_report_status, connectivity_status, crash_cleanup_retry_available, crash_prompt_context);
            }
            else
            {
                alert_banner.is_error = false;
                if (!response_body.empty())
                    alert_banner.message = response_body;
                else
                    alert_banner.message = menu_localize(
                        "main_menu.crash.submit_success", "Crash report submitted successfully. Thank you!");
                crash_report_status = MainMenuCrashReport();
                crash_prompt_last_timestamp = 0L;
                crash_prompt_last_summary.clear();
                crash_prompt_last_details.clear();
                crash_prompt_dismissed = false;
                crash_cleanup_retry_available = false;
                clear_crash_prompt_overlay();
                crash_prompt_focus_pending = false;
            }
        }
        else
        {
            alert_banner.is_error = true;
            alert_banner.message
                = menu_localize("main_menu.crash.submit_failure", "Failed to submit crash report.");
            if (status_code != 0)
            {
                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(1U);
                StringTableReplacement code_placeholder;
                code_placeholder.key = ft_string("code");
                code_placeholder.value = ft_to_string(status_code);
                replacements.push_back(code_placeholder);
                ft_string suffix
                    = menu_localize_format("main_menu.crash.error_code", " (HTTP {{code}})", replacements);
                alert_banner.message.append(suffix);
            }
            if (!response_body.empty())
            {
                alert_banner.message.append(" ");
                alert_banner.message.append(response_body);
            }
            crash_cleanup_retry_available = false;
        }

        next_crash_prompt_check_ms = 0L;
        return true;
    };

    auto perform_connectivity_check = [&](long now_ms) {
        main_menu_mark_connectivity_checking(connectivity_status, now_ms);
        main_menu_sync_begin(sync_status, MAIN_MENU_SYNC_CHANNEL_CONVOYS, now_ms);
        main_menu_sync_begin(sync_status, MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS, now_ms);
        main_menu_performance_begin_latency_sample(performance_stats, now_ms);
        int  status_code = 0;
        bool success = backend_client_ping(backend_host, backend_path, status_code);
        long result_ms = ft_time_ms();
        main_menu_apply_connectivity_result(connectivity_status, success, status_code, result_ms);
        main_menu_sync_apply(sync_status, MAIN_MENU_SYNC_CHANNEL_CONVOYS, success, status_code, result_ms);
        main_menu_sync_apply(sync_status, MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS, success, status_code, result_ms);
        long latency_ms = result_ms - now_ms;
        main_menu_performance_complete_latency_sample(performance_stats, success, latency_ms, result_ms);
        if (!success)
            main_menu_append_connectivity_failure_log(backend_host, status_code, result_ms);
        else
            flush_crash_metric_queue();
        refresh_crash_prompt(true);
        next_connectivity_check_ms = result_ms + connectivity_interval_ms;
    };

    auto attempt_campaign_launch = [&](const ft_string &save_path) -> bool {
        if (!main_menu_can_launch_campaign(save_path))
        {
            alert_banner.visible = true;
            alert_banner.is_error = true;
            alert_banner.message = menu_localize("main_menu.alerts.launch_failure",
                "Unable to launch the selected save. It may have been moved or deleted.");
            return false;
        }

        return true;
    };

    auto dismiss_tutorial_overlay = [&]() -> bool {
        if (!tutorial_context.visible)
            return false;
        tutorial_context.visible = false;
        if (!active_preferences.menu_tutorial_seen)
        {
            active_preferences.menu_tutorial_seen = true;
            player_profile_save(active_preferences);
        }
        return true;
    };

    auto dismiss_changelog_overlay = [&]() -> bool {
        if (!changelog_context.visible)
            return false;
        changelog_context.visible = false;
        return true;
    };

    auto dismiss_manual_overlay = [&]() -> bool {
