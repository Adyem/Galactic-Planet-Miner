static bool run_main_prelaunch_verifications()
{
    if (!verify_backend_roundtrip())
        return false;

    if (!verify_backend_patch_notes_fetch())
        return false;

    if (!verify_backend_clear_cloud_data())
        return false;

    if (!verify_backend_host_parsing())
        return false;

    if (!verify_locked_planet_reward_delivery())
        return false;

    if (!verify_lore_log_retention())
        return false;

    if (!verify_initial_journal_seed())
        return false;

    if (!verify_fractional_resource_accumulation())
        return false;

    if (!verify_hard_difficulty_fractional_output())
        return false;

    if (!verify_mine_upgrade_station_bonus())
        return false;

    if (!verify_set_ore_creates_missing_resource())
        return false;

    if (!verify_supply_route_key_collisions())
        return false;

    if (!verify_supply_route_threat_decay())
        return false;

    if (!verify_celestial_barrens_salvage_event())
        return false;

    if (!verify_imperium_pressure_threshold())
        return false;

    if (!verify_nebula_outpost_scan_event())
        return false;

    if (!verify_resource_lore_rotation())
        return false;

    if (!verify_nanomaterial_resource_lore())
        return false;

    if (!verify_raider_lore_rotation())
        return false;

    if (!verify_resource_dashboard_overview())
        return false;

    if (!verify_fleet_management_snapshot())
        return false;

    if (!verify_building_layout_snapshot())
        return false;

    if (!verify_quest_log_snapshot())
        return false;

    if (!verify_player_preference_application())
        return false;
    if (!verify_menu_preference_snapshot())
        return false;
    if (!verify_main_menu_descriptions())
        return false;
    if (!verify_main_menu_description_focus())
        return false;
    if (!verify_main_menu_navigation_hints())
        return false;
    if (!verify_main_menu_navigation_hints_custom())
        return false;
    if (!verify_main_menu_device_preference_seed())
        return false;
    if (!verify_main_menu_tutorial_tips())
        return false;
    if (!verify_main_menu_manual_sections())
        return false;
    if (!verify_main_menu_connectivity_indicator())
        return false;
    if (!verify_main_menu_connectivity_failure_logging())
        return false;
    if (!verify_main_menu_crash_prompt())
        return false;
    if (!verify_main_menu_crash_metric_queue())
        return false;
    if (!verify_main_menu_sync_panel())
        return false;
    if (!verify_main_menu_autosave_indicator())
        return false;
    if (!verify_main_menu_achievement_summary())
        return false;
    if (!verify_main_menu_performance_overlay())
        return false;
    if (!verify_save_system_background_queue())
        return false;
    if (!verify_main_menu_build_label())
        return false;
    if (!verify_main_menu_save_alerts())
        return false;

    if (!verify_resume_latest_save_resolution())
        return false;

    if (!verify_main_menu_resume_entry_integration())
        return false;

    if (!verify_load_resume_menu_flow_integration())
        return false;

    if (!verify_accessibility_toggle_persistence())
        return false;

    if (!verify_main_menu_commander_portrait_preload())
        return false;

    if (!verify_main_menu_campaign_launch_guard())
        return false;

    if (!verify_main_menu_patch_notes_split())
        return false;

    if (!verify_settings_flow_helpers())
        return false;

    if (!verify_main_menu_audio_respects_settings())
        return false;

    if (!verify_load_flow_save_listing())
        return false;

    if (!verify_load_flow_save_deletion())
        return false;

    if (!verify_load_flow_save_rename())
        return false;

    if (!verify_load_flow_save_metadata())
        return false;

    if (!verify_quick_quest_completion_bonus())
        return false;

    if (!verify_trade_relay_convoy_modifiers())
        return false;

    if (!verify_achievement_catalog())
        return false;

    if (!verify_achievement_progression())
        return false;

    if (!verify_quest_achievement_failures())
        return false;

    if (!verify_convoy_escort_travel_speed())
        return false;

    if (!verify_convoy_escort_assignment_persistence())
        return false;

    if (!verify_convoy_escort_rating_excludes_active_escort())
        return false;

    return true;
}
