
                if (loaded)
                {
                    if (attempt_campaign_launch(selected_save_path))
                    {
                        running = false;
                        return;
                    }
                }
                refresh_save_alert();
                return;
            }

            if (item.identifier == "tutorial")
            {
                ft_string tutorial_path = player_profile_resolve_tutorial_save_path(active_profile_name);
                if (tutorial_path.empty())
                {
                    alert_banner.visible = true;
                    alert_banner.is_error = true;
                    alert_banner.message = menu_localize("main_menu.tutorial.replay.resolve_failure",
                        "Unable to prepare a tutorial mission for this commander.");
                    return;
                }

                if (!game_bootstrap_create_tutorial_quicksave(tutorial_path.c_str(), active_profile_name))
                {
                    alert_banner.visible = true;
                    alert_banner.is_error = true;
                    alert_banner.message = menu_localize("main_menu.tutorial.replay.create_failure",
                        "Unable to prepare the tutorial mission. Try again later.");
                    return;
                }

                if (attempt_campaign_launch(tutorial_path))
                {
                    running = false;
                    return;
                }

                return;
            }

            if (item.identifier == "exit")
            {
                running = false;
                return;
            }

            if (item.identifier == "swap_profile")
            {
                bool management_quit = false;
                ft_string selected_profile = run_profile_management_flow(window, renderer, title_font, menu_font, active_profile_name,
                    management_quit);
                if (management_quit)
                {
                    running = false;
                    return;
                }

                if (!selected_profile.empty() && selected_profile != active_profile_name)
                {
                    active_profile_name = selected_profile;
                    apply_profile_preferences(window, active_profile_name);
                    refresh_fonts();
                    if (!player_profile_load_or_create(active_preferences, active_profile_name))
                    {
                        active_preferences = PlayerProfilePreferences();
                        active_preferences.commander_name = active_profile_name;
                    }
                    main_menu_audio_apply_preferences(active_preferences);
                    tutorial_context.visible = !active_preferences.menu_tutorial_seen;
                    refresh_achievement_summary();
                    perform_connectivity_check(ft_time_ms());
                }

                player_profile_list(available_profiles);
                refresh_save_alert();
            }

            if (item.identifier == "settings")
            {
                bool settings_quit = false;
                bool saved = run_settings_flow(window, renderer, title_font, menu_font, active_preferences, settings_quit);
                if (settings_quit)
                {
                    running = false;
                    return;
                }

                if (saved)
                {
                    apply_profile_preferences(window, active_profile_name);
                    refresh_fonts();
                    main_menu_audio_apply_preferences(active_preferences);
                    refresh_achievement_summary();
                    menu_items = build_main_menu_items(&active_preferences);
                    refresh_save_alert();
                }
                return;
            }

            if (item.identifier == "clear_cloud")
            {
                if (connectivity_status.state != MAIN_MENU_CONNECTIVITY_ONLINE)
                {
                    alert_banner.visible = true;
                    alert_banner.is_error = true;
            alert_banner.message = menu_localize("main_menu.clear_cloud.requires_online",
                "Cloud data can only be cleared while online. Reconnect and try again.");
            return;
        }
                open_clear_cloud_prompt();
                return;
            }

            if (item.identifier == "changelog")
            {
                open_changelog_overlay();
                return;
            }

            if (item.identifier == "manual")
            {
                open_manual_overlay();
                return;
            }
        };

        if (mouse_state.left_released)
        {
            const int index = menu.get_hovered_index();
            const ft_menu_item *hovered_item = menu_item_from_index(menu, index);
            if (hovered_item != ft_nullptr)
            {
                process_menu_activation(*hovered_item);
                if (!running)
                    break;
            }
        }

        if (activate_requested)
        {
            const ft_menu_item *selected_item = menu.get_selected_item();
            if (selected_item != ft_nullptr)
            {
                process_menu_activation(*selected_item);
                if (!running)
                    break;
            }
        }

        int window_width = 0;
        int window_height = 0;
        SDL_GetWindowSize(window, &window_width, &window_height);
        long current_time_ms = ft_time_ms();
        if (current_time_ms >= next_connectivity_check_ms)
        {
            perform_connectivity_check(current_time_ms);
            handle_crash_prompt_focus();
        }

        render_main_menu(*renderer, menu, title_font, menu_font, window_width, window_height, active_profile_name,
            &active_preferences, &tutorial_context, &manual_context, &changelog_context, &clear_cloud_context,
            &crash_prompt_context,
            &connectivity_status, &sync_status, &achievements_summary, &autosave_status, &performance_stats, &alert_banner);

        long frame_end_ms = ft_time_ms();
        main_menu_performance_record_frame(performance_stats, frame_start_ms, frame_end_ms);
    }

    if (window != ft_nullptr && !active_profile_name.empty())
        save_profile_preferences(window, active_preferences);

    destroy_renderer(renderer);
    destroy_window(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}

