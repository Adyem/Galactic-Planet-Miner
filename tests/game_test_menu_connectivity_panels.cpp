int verify_main_menu_sync_panel()
{
    MainMenuSyncStatus status;

    ft_string convoys_initial
        = main_menu_testing::resolve_sync_entry_label(status, MAIN_MENU_SYNC_CHANNEL_CONVOYS);
    FT_ASSERT_EQ(ft_string("Convoys: Awaiting first sync."), convoys_initial);
    ft_string leader_initial
        = main_menu_testing::resolve_sync_entry_label(status, MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS);
    FT_ASSERT_EQ(ft_string("Leaderboards: Awaiting first sync."), leader_initial);

    main_menu_testing::begin_sync(status, MAIN_MENU_SYNC_CHANNEL_CONVOYS, 1000L);
    ft_string convoys_checking
        = main_menu_testing::resolve_sync_entry_label(status, MAIN_MENU_SYNC_CHANNEL_CONVOYS);
    FT_ASSERT_EQ(ft_string("Convoys: Sync in progress..."), convoys_checking);

    main_menu_testing::apply_sync(status, MAIN_MENU_SYNC_CHANNEL_CONVOYS, true, 204, 1500L);
    ft_string convoys_success
        = main_menu_testing::resolve_sync_entry_label(status, MAIN_MENU_SYNC_CHANNEL_CONVOYS);
    FT_ASSERT_EQ(ft_string("Convoys: Updated at 1970-01-01T00:00:01Z.500 (HTTP 204)"), convoys_success);

    main_menu_testing::begin_sync(status, MAIN_MENU_SYNC_CHANNEL_CONVOYS, 2000L);
    main_menu_testing::apply_sync(status, MAIN_MENU_SYNC_CHANNEL_CONVOYS, false, 503, 2600L);
    ft_string convoys_failure
        = main_menu_testing::resolve_sync_entry_label(status, MAIN_MENU_SYNC_CHANNEL_CONVOYS);
    FT_ASSERT_EQ(ft_string("Convoys: Failed at 1970-01-01T00:00:02Z.600 (HTTP 503) (last success 1970-01-01T00:00:01Z.500)"),
        convoys_failure);

    main_menu_testing::begin_sync(status, MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS, 4000L);
    main_menu_testing::apply_sync(status, MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS, false, 0, 4200L);
    ft_string leader_failure
        = main_menu_testing::resolve_sync_entry_label(status, MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS);
    FT_ASSERT_EQ(ft_string("Leaderboards: Failed at 1970-01-01T00:00:04Z.200"), leader_failure);

    main_menu_testing::apply_sync(status, MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS, true, 200, 5000L);
    ft_string leader_success
        = main_menu_testing::resolve_sync_entry_label(status, MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS);
    FT_ASSERT_EQ(ft_string("Leaderboards: Updated at 1970-01-01T00:00:05Z.000 (HTTP 200)"), leader_success);

    return 1;
}

int verify_main_menu_autosave_indicator()
{
    save_system_background_reset();

    MainMenuAutosaveStatus status;
    FT_ASSERT(!main_menu_autosave_is_visible(status, 1000L));
    FT_ASSERT(main_menu_resolve_autosave_label(status).empty());

    const ft_string slot_name("omega");
    const long      start_timestamp = 2000L;
    main_menu_mark_autosave_in_progress(status, slot_name, start_timestamp);
    ft_string in_progress_label = main_menu_resolve_autosave_label(status);
    FT_ASSERT_EQ(ft_string("Autosaving (omega)..."), in_progress_label);
    FT_ASSERT(main_menu_autosave_is_visible(status, start_timestamp + 10L));

    main_menu_mark_autosave_result(status, true, slot_name, ft_string(), start_timestamp + 500L);
    ft_string success_label = main_menu_resolve_autosave_label(status);
    FT_ASSERT_EQ(ft_string("Autosave complete (omega)."), success_label);
    FT_ASSERT(main_menu_autosave_is_visible(status, start_timestamp + 1500L));

    main_menu_autosave_tick(status, start_timestamp + 6000L);
    FT_ASSERT(!main_menu_autosave_is_visible(status, start_timestamp + 6000L));
    FT_ASSERT(main_menu_resolve_autosave_label(status).empty());

    const ft_string error_message("Disk full");
    main_menu_mark_autosave_result(status, false, slot_name, error_message, start_timestamp + 7000L);
    ft_string failure_label = main_menu_resolve_autosave_label(status);
    FT_ASSERT(ft_strstr(failure_label.c_str(), "Autosave failed") != ft_nullptr);
    FT_ASSERT(ft_strstr(failure_label.c_str(), "Disk full") != ft_nullptr);
    FT_ASSERT(main_menu_autosave_is_visible(status, start_timestamp + 7200L));

    main_menu_autosave_tick(status, start_timestamp + 12000L);
    FT_ASSERT(!main_menu_autosave_is_visible(status, start_timestamp + 12000L));
    FT_ASSERT(main_menu_resolve_autosave_label(status).empty());

    return 1;
}

int verify_main_menu_achievement_summary()
{
    menu_localization_reset_for_testing();

    PlayerProfilePreferences preferences;
    preferences.achievements_total_count = 30U;
    preferences.achievements_completed_count = 12U;
    preferences.achievements_highlight_label = ft_string("Research Pioneer");
    preferences.achievements_progress_note = ft_string("Deliver one more convoy.");

    MainMenuAchievementsSummary summary = main_menu_testing::build_achievements_summary(preferences);
    FT_ASSERT_EQ(30U, summary.total_count);
    FT_ASSERT_EQ(12U, summary.completed_count);
    FT_ASSERT(summary.has_highlight);
    FT_ASSERT(summary.has_progress_note);

    ft_string completion = main_menu_testing::format_achievements_completion(summary);
    FT_ASSERT(ft_strstr(completion.c_str(), "12") != ft_nullptr);
    FT_ASSERT(ft_strstr(completion.c_str(), "30") != ft_nullptr);

    ft_vector<ft_string> lines = main_menu_testing::collect_achievement_lines(summary);
    FT_ASSERT_EQ(2U, lines.size());
    FT_ASSERT(ft_strstr(lines[0].c_str(), "Research Pioneer") != ft_nullptr);
    FT_ASSERT(ft_strstr(lines[1].c_str(), "Deliver one more convoy") != ft_nullptr);

    PlayerProfilePreferences defaults;
    defaults.achievements_total_count = 0U;
    defaults.achievements_completed_count = 0U;
    MainMenuAchievementsSummary default_summary = main_menu_testing::build_achievements_summary(defaults);
    FT_ASSERT(default_summary.total_count >= default_summary.completed_count);
    ft_vector<ft_string> default_lines = main_menu_testing::collect_achievement_lines(default_summary);
    FT_ASSERT(default_lines.size() >= 1U);

    return 1;
}

int verify_main_menu_performance_overlay()
{
    menu_localization_reset_for_testing();

    MainMenuPerformanceStats stats;

    ft_string initial_fps = main_menu_testing::format_performance_fps(stats);
    FT_ASSERT_EQ(ft_string("FPS: --"), initial_fps);
    ft_string initial_latency = main_menu_testing::format_performance_latency(stats);
    FT_ASSERT_EQ(ft_string("Latency: --"), initial_latency);

    long frame_start = 0L;
    for (int index = 0; index < 40; ++index)
    {
        long frame_end = frame_start + 16L;
        main_menu_testing::performance_record_frame(stats, frame_start, frame_end);
        frame_start = frame_end;
    }

    FT_ASSERT(stats.has_fps);
    FT_ASSERT(stats.fps_value >= 30U);
    ft_string fps_label = main_menu_testing::format_performance_fps(stats);
    ft_string expected_fps("FPS: ");
    expected_fps.append(ft_to_string(static_cast<int>(stats.fps_value)));
    FT_ASSERT_EQ(expected_fps, fps_label);

    main_menu_testing::performance_begin_latency(stats, 1000L);
    ft_string pending_latency = main_menu_testing::format_performance_latency(stats);
    FT_ASSERT_EQ(ft_string("Latency: Measuring..."), pending_latency);

    main_menu_testing::performance_complete_latency(stats, true, 42L, 1100L);
    FT_ASSERT(stats.latency_sampled);
    FT_ASSERT(stats.latency_successful);
    ft_string success_latency = main_menu_testing::format_performance_latency(stats);
    FT_ASSERT_EQ(ft_string("Latency: 42 ms"), success_latency);

    main_menu_testing::performance_complete_latency(stats, false, 0L, 1500L);
    FT_ASSERT(stats.latency_sampled);
    FT_ASSERT(!stats.latency_successful);
    ft_string offline_latency = main_menu_testing::format_performance_latency(stats);
    FT_ASSERT_EQ(ft_string("Latency: Offline"), offline_latency);

    return 1;
}

int verify_main_menu_save_alerts()
{
    long timestamp = ft_time_ms();
    ft_string commander("MainMenuSaveAlerts_");
    commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));

    PlayerProfilePreferences preferences;
    preferences.commander_name = commander;
    FT_ASSERT(player_profile_save(preferences));

    ft_string error;
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("stable"), error));
    FT_ASSERT(error.empty());
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("corrupt"), error));
    FT_ASSERT(error.empty());

    ft_string stable_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("stable"));
    FT_ASSERT(!stable_path.empty());
    ft_ofstream stable_stream;
    FT_ASSERT_EQ(0, stable_stream.open(stable_path.c_str()));
    ft_string stable_contents("{\n");
    stable_contents.append("  \"metadata\": {\"version\": 1, \"save_type\": \"quicksave\"},\n");
    stable_contents.append("  \"player\": {\"commander_name\": \"Alert Tester\", \"starting_planet_id\": 1, \"commander_level\": 4},\n");
    stable_contents.append("  \"campaign\": {\"day\": 3, \"difficulty\": 2}\n");
    stable_contents.append("}\n");
    FT_ASSERT(stable_stream.write(stable_contents.c_str()) >= 0);
    stable_stream.close();

    ft_string corrupt_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("corrupt"));
    FT_ASSERT(!corrupt_path.empty());
    ft_ofstream corrupt_stream;
    FT_ASSERT_EQ(0, corrupt_stream.open(corrupt_path.c_str()));
    ft_string corrupt_contents("{\"metadata\":\n");
    FT_ASSERT(corrupt_stream.write(corrupt_contents.c_str()) >= 0);
    corrupt_stream.close();

    ft_vector<ft_string> slot_errors = load_game_flow_testing::collect_save_slot_errors(commander);
    FT_ASSERT_EQ(1U, slot_errors.size());
    FT_ASSERT(ft_strstr(slot_errors[0].c_str(), "corrupt") != ft_nullptr);

    ft_vector<ft_string> audit_errors;
    FT_ASSERT(audit_save_directory_for_errors(commander, audit_errors));
    FT_ASSERT_EQ(1U, audit_errors.size());
    FT_ASSERT(ft_strstr(audit_errors[0].c_str(), "corrupt") != ft_nullptr);

    return 1;
}
