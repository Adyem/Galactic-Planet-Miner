        if (!manual_context.visible)
            return false;
        manual_context.visible = false;
        return true;
    };

    auto close_clear_cloud_prompt = [&]() -> bool {
        if (!clear_cloud_context.visible)
            return false;
        clear_cloud_context.visible = false;
        clear_cloud_context.heading.clear();
        clear_cloud_context.lines.clear();
        clear_cloud_context.footer.clear();
        return true;
    };

    auto open_clear_cloud_prompt = [&]() {
        dismiss_tutorial_overlay();
        dismiss_changelog_overlay();
        dismiss_manual_overlay();
        clear_cloud_context.visible = true;
        clear_cloud_context.heading
            = menu_localize("main_menu.clear_cloud.heading", "Clear Cloud Data?");
        clear_cloud_context.lines.clear();
        clear_cloud_context.lines.push_back(menu_localize(
            "main_menu.clear_cloud.summary",
            "This will remove any cloud-synced progress for this commander."));
        clear_cloud_context.lines.push_back(menu_localize(
            "main_menu.clear_cloud.local_remains", "Local saves on this device will remain intact."));
        clear_cloud_context.footer = menu_localize(
            "main_menu.clear_cloud.footer", "Press Enter / A to confirm, Esc / B or click to cancel.");
    };

    auto confirm_clear_cloud_prompt = [&]() -> bool {
        if (!clear_cloud_context.visible)
            return false;

        close_clear_cloud_prompt();

        if (connectivity_status.state != MAIN_MENU_CONNECTIVITY_ONLINE)
        {
            alert_banner.visible = true;
            alert_banner.is_error = true;
            alert_banner.message = menu_localize("main_menu.clear_cloud.requires_online",
                "Cloud data can only be cleared while online. Reconnect and try again.");
            return true;
        }

        ft_string response_body;
        int       status_code = 0;
        bool      success
            = backend_client_clear_cloud_data(backend_host, backend_clear_cloud_path, response_body, status_code);

        alert_banner.visible = true;
        if (success)
        {
            alert_banner.is_error = false;
            if (!response_body.empty())
                alert_banner.message = response_body;
            else
            {
                if (!active_profile_name.empty())
                {
                    ft_vector<StringTableReplacement> replacements;
                    replacements.reserve(1U);
                    StringTableReplacement commander_placeholder;
                    commander_placeholder.key = ft_string("commander");
                    commander_placeholder.value = active_profile_name;
                    replacements.push_back(commander_placeholder);
                    alert_banner.message = menu_localize_format("main_menu.clear_cloud.cleared_named",
                        "Cleared cloud data for \"{{commander}}\".", replacements);
                }
                else
                {
                    alert_banner.message = menu_localize("main_menu.clear_cloud.cleared_generic",
                        "Cleared cloud data for this commander.");
                }
            }
        }
        else
        {
            alert_banner.is_error = true;
            alert_banner.message = menu_localize(
                "main_menu.clear_cloud.failure", "Failed to clear cloud data.");
            if (status_code != 0)
            {
                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(1U);
                StringTableReplacement code_placeholder;
                code_placeholder.key = ft_string("code");
                code_placeholder.value = ft_to_string(status_code);
                replacements.push_back(code_placeholder);
                ft_string suffix
                    = menu_localize_format("main_menu.clear_cloud.error_code", " (HTTP {{code}})", replacements);
                alert_banner.message.append(suffix);
            }
            if (!response_body.empty())
            {
                alert_banner.message.append(" ");
                alert_banner.message.append(response_body);
            }
            main_menu_append_connectivity_failure_log(backend_host, status_code, ft_time_ms());
        }
        return true;
    };

    auto handle_crash_prompt_focus = [&]() {
        if (!crash_prompt_focus_pending)
            return;
        dismiss_tutorial_overlay();
        dismiss_changelog_overlay();
        dismiss_manual_overlay();
        close_clear_cloud_prompt();
        crash_prompt_focus_pending = false;
    };

    refresh_save_alert();
    perform_connectivity_check(ft_time_ms());
    handle_crash_prompt_focus();

    bool running = true;

    auto open_changelog_overlay = [&]() {
        dismiss_tutorial_overlay();
        dismiss_manual_overlay();
        changelog_context.visible = false;
        changelog_context.heading = ft_string();
        changelog_context.lines.clear();
        changelog_context.footer
            = menu_localize("main_menu.overlay.close", "Press Enter / Space or click to close.");

        if (connectivity_status.state != MAIN_MENU_CONNECTIVITY_ONLINE)
        {
            changelog_context.heading
                = menu_localize("main_menu.changelog.offline_heading", "Patch Notes Unavailable");
            changelog_context.lines.push_back(menu_localize(
                "main_menu.changelog.offline_required", "Patch notes require an online connection."));
            if (connectivity_status.last_status_code != 0)
            {
                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(1U);
                StringTableReplacement code_placeholder;
                code_placeholder.key = ft_string("code");
                code_placeholder.value = ft_to_string(connectivity_status.last_status_code);
                replacements.push_back(code_placeholder);
                ft_string status_line = menu_localize_format(
                    "main_menu.changelog.offline_status", "Last backend status: HTTP {{code}}", replacements);
                changelog_context.lines.push_back(status_line);
            }
            changelog_context.lines.push_back(menu_localize(
                "main_menu.changelog.offline_retry", "Reconnect to view the latest updates."));
            changelog_context.visible = true;
            return;
        }

        ft_string notes_body;
        int       status_code = 0;
        bool      success = backend_client_fetch_patch_notes(backend_host, backend_patch_notes_path, notes_body, status_code);

        if (success)
        {
            changelog_context.heading
                = menu_localize("main_menu.changelog.latest_heading", "Latest Patch Notes");
            ft_vector<ft_string> note_lines = main_menu_split_patch_notes(notes_body);
            for (size_t index = 0; index < note_lines.size(); ++index)
                changelog_context.lines.push_back(note_lines[index]);
            if (changelog_context.lines.empty())
                changelog_context.lines.push_back(menu_localize(
                    "main_menu.changelog.empty_backend", "No patch notes were returned by the backend."));
        }
        else
        {
            changelog_context.heading
                = menu_localize("main_menu.changelog.unavailable_heading", "Patch Notes Unavailable");
            changelog_context.lines.push_back(menu_localize(
                "main_menu.changelog.fetch_failure", "Unable to fetch patch notes from the backend."));
            if (status_code != 0)
            {
                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(1U);
                StringTableReplacement code_placeholder;
                code_placeholder.key = ft_string("code");
                code_placeholder.value = ft_to_string(status_code);
                replacements.push_back(code_placeholder);
                ft_string status_line
                    = menu_localize_format("main_menu.changelog.error_status", "HTTP status: {{code}}", replacements);
                changelog_context.lines.push_back(status_line);
            }
            if (!notes_body.empty())
                changelog_context.lines.push_back(notes_body);
            if (changelog_context.lines.size() == 1U)
                changelog_context.lines.push_back(
                    menu_localize("main_menu.changelog.try_later", "Please try again later."));
        }

        if (changelog_context.lines.empty())
            changelog_context.lines.push_back(
                menu_localize("main_menu.changelog.empty_fallback", "Patch notes are currently empty."));

        changelog_context.visible = true;
    };

    auto open_manual_overlay = [&]() {
        dismiss_tutorial_overlay();
        dismiss_changelog_overlay();
        manual_context.visible = false;
        manual_context.heading
            = menu_localize("main_menu.manual.heading", "Galactic Encyclopedia");
        manual_context.lines.clear();

        const ft_vector<ft_string> &manual_lines = get_main_menu_manual_lines();
        for (size_t index = 0; index < manual_lines.size(); ++index)
            manual_context.lines.push_back(manual_lines[index]);

        manual_context.footer
            = menu_localize("main_menu.overlay.close", "Press Enter / Space or click to close.");
        manual_context.visible = true;
    };

    while (running)
    {
        long frame_start_ms = ft_time_ms();
        ft_mouse_state    mouse_state;
        ft_keyboard_state keyboard_state;
        ft_gamepad_state  gamepad_state;
        bool              activate_requested = false;
        bool              tutorial_click_in_progress = false;
        bool              changelog_click_in_progress = false;
        bool              manual_click_in_progress = false;
        bool              clear_cloud_click_in_progress = false;
        bool              crash_click_in_progress = false;

        SaveSystemBackgroundEvent autosave_event;
        while (save_system_background_poll_event(autosave_event))
        {
            if (autosave_event.type == SaveSystemBackgroundEvent::SAVE_SYSTEM_BACKGROUND_EVENT_STARTED)
            {
                main_menu_mark_autosave_in_progress(autosave_status, autosave_event.slot_name,
                    autosave_event.timestamp_ms);
            }
            else if (autosave_event.type == SaveSystemBackgroundEvent::SAVE_SYSTEM_BACKGROUND_EVENT_COMPLETED)
            {
                main_menu_mark_autosave_result(autosave_status, autosave_event.success, autosave_event.slot_name,
                    autosave_event.error_message, autosave_event.timestamp_ms);
            }
        }

        main_menu_autosave_tick(autosave_status, ft_time_ms());
        refresh_crash_prompt(false);
        handle_crash_prompt_focus();
