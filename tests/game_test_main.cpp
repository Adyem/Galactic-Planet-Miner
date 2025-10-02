#include "../libft/Networking/http_server.hpp"
#include "../libft/PThread/thread.hpp"
#include "../libft/Time/time.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/pair.hpp"
#include "game_test_scenarios.hpp"

static void run_server()
{
    ft_http_server server;
    server.start("127.0.0.1", 8080, AF_INET, false);
    for (int i = 0; i < 64; ++i)
        server.run_once();
}

int main()
{
    ft_thread server_thread(run_server);
    time_sleep_ms(100);

    if (!verify_backend_roundtrip())
        return 0;

    if (!verify_backend_host_parsing())
        return 0;

    if (!verify_locked_planet_reward_delivery())
        return 0;

    if (!verify_lore_log_retention())
        return 0;

    if (!verify_fractional_resource_accumulation())
        return 0;

    if (!verify_hard_difficulty_fractional_output())
        return 0;

    if (!verify_mine_upgrade_station_bonus())
        return 0;

    if (!verify_set_ore_creates_missing_resource())
        return 0;

    if (!verify_supply_route_key_collisions())
        return 0;

    if (!verify_supply_route_threat_decay())
        return 0;

    if (!verify_trade_relay_convoy_modifiers())
        return 0;

    if (!verify_achievement_catalog())
        return 0;

    if (!verify_achievement_progression())
        return 0;

    if (!verify_quest_achievement_failures())
        return 0;

    if (!verify_convoy_escort_travel_speed())
        return 0;

    if (!verify_convoy_escort_assignment_persistence())
        return 0;

    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    if (!validate_initial_campaign_flow(game))
        return 0;
    if (!validate_order_branch_storyline())
        return 0;
    if (!evaluate_building_and_convoy_systems(game))
        return 0;
    if (!verify_convoy_quest_objectives())
        return 0;
    if (!evaluate_ship_upgrade_research(game))
        return 0;
    if (!verify_supply_contract_automation())
        return 0;
    if (!verify_supply_contract_pending_stock_buffer())
        return 0;
    if (!verify_multiple_convoy_raids())
        return 0;
    if (!verify_supply_route_escalation())
        return 0;
    if (!verify_escort_veterancy_progression())
        return 0;


    if (!verify_building_tick_state_persistence())
        return 0;

    if (!compare_energy_pressure_scenarios())
        return 0;
    if (!verify_crafting_resume_requires_full_cycle())
        return 0;
    if (!compare_storyline_assaults())
        return 0;
    if (!analyze_manual_vs_auto_assault_controls())
        return 0;
    if (!measure_assault_aggression_effects())
        return 0;
    if (!evaluate_focus_fire_cooldowns())
        return 0;
    if (!validate_tactical_pause_behaviors())
        return 0;
    if (!verify_ship_range_defaults())
        return 0;
    if (!verify_design_doc_ship_roster())
        return 0;
    if (!verify_range_aware_combat_power())
        return 0;
    if (!verify_raider_ship_maneuver_profiles())
        return 0;
    if (!verify_combat_tracker_deceleration())
        return 0;
    if (!verify_raider_ships_unavailable_to_players())
        return 0;
    if (!compare_generator_support())
        return 0;
    if (!inspect_support_ship_positioning())
        return 0;
    if (!verify_difficulty_scaling())
        return 0;
    if (!verify_crafting_and_energy_research())
        return 0;
    if (!verify_auxiliary_and_escape_protocol())
        return 0;
    if (!verify_save_system_round_trip())
        return 0;
    if (!verify_save_system_edge_cases())
        return 0;
    if (!verify_save_system_sanitizes_ship_movement_stats())
        return 0;
    if (!verify_save_system_invalid_inputs())
        return 0;
    if (!verify_save_system_rejects_oversized_building_grids())
        return 0;
    if (!verify_save_system_prevents_building_instance_wraparound())
        return 0;
    if (!verify_save_system_compact_building_serialization())
        return 0;
    if (!verify_save_system_rejects_overlarge_ship_ids())
        return 0;
    if (!verify_save_system_limits_inflated_ship_counts())
        return 0;
    if (!verify_save_system_recovers_underreported_ship_counts())
        return 0;
    if (!validate_save_system_serialized_samples())
        return 0;
    if (!verify_save_system_allocation_failures())
        return 0;
    if (!verify_save_system_extreme_scaling())
        return 0;
    if (!verify_save_system_normalizes_non_finite_planet_values())
        return 0;
    if (!verify_save_system_massive_payload())
        return 0;
    if (!verify_save_system_sparse_entries())
        return 0;
    if (!verify_planet_inventory_save_round_trip())
        return 0;
    if (!verify_planet_inventory_resets_unsaved_items())
        return 0;
    if (!verify_building_save_round_trip())
        return 0;
    if (!verify_campaign_load_accepts_empty_building_payload())
        return 0;
    if (!verify_research_save_round_trip())
        return 0;
    if (!verify_achievement_save_round_trip())
        return 0;
    if (!verify_campaign_checkpoint_flow())
        return 0;
    if (!verify_campaign_rejects_invalid_save())
        return 0;
    if (!verify_buildings_unchanged_on_failed_load())
        return 0;
    if (!verify_player_profile_save())
        return 0;
    if (!verify_save_system_prevents_ship_id_wraparound())
        return 0;
    if (!verify_save_system_resolves_duplicate_ship_ids())
        return 0;

    server_thread.join();
    return 0;
}
