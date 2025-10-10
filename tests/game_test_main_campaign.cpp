static bool run_main_campaign_verifications(Game &game)
{
    if (!validate_initial_campaign_flow(game))
        return false;
    if (!validate_order_branch_storyline())
        return false;
    if (!verify_order_final_verdict_outcomes())
        return false;
    if (!verify_rebellion_final_push_outcomes())
        return false;
    if (!evaluate_building_and_convoy_systems(game))
        return false;
    if (!verify_convoy_quest_objectives())
        return false;
    if (!verify_side_quest_rotation())
        return false;
    if (!evaluate_ship_upgrade_research(game))
        return false;
    if (!verify_supply_contract_automation())
        return false;
    if (!verify_supply_contract_pending_stock_buffer())
        return false;
    if (!verify_multiple_convoy_raids())
        return false;
    if (!verify_supply_route_escalation())
        return false;
    if (!verify_late_campaign_raider_focus())
        return false;
    if (!verify_escort_veterancy_progression())
        return false;
    if (!verify_combat_victory_journal_rewards())
        return false;

    return true;
}
