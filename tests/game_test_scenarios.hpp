#ifndef GAME_TEST_SCENARIOS_HPP
#define GAME_TEST_SCENARIOS_HPP

#include "game.hpp"

int verify_backend_roundtrip();
int validate_initial_campaign_flow(Game &game);
int evaluate_building_and_convoy_systems(Game &game);
int evaluate_ship_upgrade_research(Game &game);
int verify_supply_contract_automation();
int verify_multiple_convoy_raids();
int compare_energy_pressure_scenarios();
int compare_storyline_assaults();
int analyze_manual_vs_auto_assault_controls();
int measure_assault_aggression_effects();
int evaluate_focus_fire_cooldowns();
int validate_tactical_pause_behaviors();
int compare_generator_support();
int inspect_support_ship_positioning();
int verify_difficulty_scaling();
int verify_crafting_and_energy_research();
int verify_auxiliary_and_escape_protocol();
int verify_fractional_resource_accumulation();
int verify_hard_difficulty_fractional_output();
int verify_supply_route_key_collisions();
int verify_trade_relay_convoy_modifiers();

#endif
