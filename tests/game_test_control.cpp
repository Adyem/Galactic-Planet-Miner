#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft_math_bridge.hpp"
#include "fleets.hpp"
#include "buildings.hpp"
#include "research.hpp"
#include "game_test_scenarios.hpp"

namespace
{
    int unlock_shield_support(Game &game)
    {
        game.ensure_planet_item_slot(PLANET_TERRA, ITEM_COPPER_BAR);
        game.ensure_planet_item_slot(PLANET_TERRA, ITEM_MITHRIL_BAR);
        game.ensure_planet_item_slot(PLANET_TERRA, ITEM_ACCUMULATOR);
        game.set_ore(PLANET_TERRA, ORE_IRON, 400);
        game.set_ore(PLANET_TERRA, ORE_COPPER, 400);
        game.set_ore(PLANET_TERRA, ORE_COAL, 400);
        game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 160);
        game.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 160);
        game.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 60);
        FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
        game.tick(60.0);
        FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_MARS));
        FT_ASSERT(game.start_research(RESEARCH_UNLOCK_ZALTHOR));
        game.tick(60.0);
        FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_ZALTHOR));
        FT_ASSERT(game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
        game.tick(60.0);
        FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_I));
        FT_ASSERT(game.start_research(RESEARCH_SHIELD_TECHNOLOGY));
        game.tick(60.0);
        FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_SHIELD_TECHNOLOGY));
        return 1;
    }
}

int analyze_manual_vs_auto_assault_controls()
{
    Game auto_mode(ft_string("127.0.0.1:8080"), ft_string("/"));
    auto_mode.set_ore(PLANET_TERRA, ORE_IRON, 200);
    auto_mode.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    auto_mode.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(auto_mode.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    auto_mode.tick(0.0);
    auto_mode.create_fleet(1);
    int auto_capital = auto_mode.create_ship(1, SHIP_CAPITAL);
    auto_mode.set_ship_hp(1, auto_capital, 220);
    auto_mode.set_ship_shield(1, auto_capital, 100);
    auto_mode.create_fleet(2);
    int auto_guard = auto_mode.create_ship(2, SHIP_SHIELD);
    auto_mode.set_ship_hp(2, auto_guard, 140);
    auto_mode.set_ship_shield(2, auto_guard, 80);
    FT_ASSERT(auto_mode.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(auto_mode.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(auto_mode.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(!auto_mode.set_assault_support(PLANET_TERRA, true, false, false));
    FT_ASSERT(!auto_mode.trigger_assault_focus_fire(PLANET_TERRA));
    FT_ASSERT(!auto_mode.request_assault_tactical_pause(PLANET_TERRA));

    Game manual_mode(ft_string("127.0.0.1:8080"), ft_string("/"));
    manual_mode.set_ore(PLANET_TERRA, ORE_IRON, 200);
    manual_mode.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    manual_mode.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(manual_mode.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    manual_mode.tick(0.0);
    manual_mode.create_fleet(1);
    int manual_capital = manual_mode.create_ship(1, SHIP_CAPITAL);
    manual_mode.set_ship_hp(1, manual_capital, 220);
    manual_mode.set_ship_shield(1, manual_capital, 100);
    manual_mode.create_fleet(2);
    int manual_guard = manual_mode.create_ship(2, SHIP_SHIELD);
    manual_mode.set_ship_hp(2, manual_guard, 140);
    manual_mode.set_ship_shield(2, manual_guard, 80);
    FT_ASSERT(manual_mode.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(manual_mode.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(manual_mode.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(manual_mode.set_assault_support(PLANET_TERRA, true, false, true));
    FT_ASSERT(manual_mode.set_assault_support(PLANET_TERRA, false, false, false));
    FT_ASSERT(manual_mode.trigger_assault_focus_fire(PLANET_TERRA));
    FT_ASSERT(!manual_mode.trigger_assault_focus_fire(PLANET_TERRA));
    FT_ASSERT(manual_mode.request_assault_tactical_pause(PLANET_TERRA));
    FT_ASSERT(!manual_mode.request_assault_tactical_pause(PLANET_TERRA));
    ft_vector<ft_ship_spatial_state> manual_raider_positions_start;
    ft_vector<ft_ship_spatial_state> manual_defender_positions_start;
    FT_ASSERT(manual_mode.get_assault_raider_positions(PLANET_TERRA, manual_raider_positions_start));
    FT_ASSERT(manual_mode.get_assault_defender_positions(PLANET_TERRA, manual_defender_positions_start));
    FT_ASSERT(manual_raider_positions_start.size() > 0);
    FT_ASSERT(manual_defender_positions_start.size() > 0);
    double raider_highest = -1000.0;
    double raider_lowest = 1000.0;
    bool raider_forward = false;
    for (size_t idx = 0; idx < manual_raider_positions_start.size(); ++idx)
    {
        const ft_ship_spatial_state &state = manual_raider_positions_start[idx];
        if (state.z > raider_highest)
            raider_highest = state.z;
        if (state.z < raider_lowest)
            raider_lowest = state.z;
        if (state.z > 5.0)
            raider_forward = true;
    }
    FT_ASSERT(raider_forward);
    FT_ASSERT(raider_highest - raider_lowest > 4.0);
    double defender_closest = -1000.0;
    double defender_farthest = 1000.0;
    bool defender_backline = false;
    for (size_t idx = 0; idx < manual_defender_positions_start.size(); ++idx)
    {
        const ft_ship_spatial_state &state = manual_defender_positions_start[idx];
        if (state.z > defender_closest)
            defender_closest = state.z;
        if (state.z < defender_farthest)
            defender_farthest = state.z;
        if (state.z < -35.0)
            defender_backline = true;
    }
    FT_ASSERT(defender_closest < -0.5);
    FT_ASSERT(defender_backline);
    FT_ASSERT(defender_closest - defender_farthest > 4.0);
    manual_mode.tick(0.25);
    auto_mode.tick(0.25);
    ft_vector<ft_ship_spatial_state> manual_raider_positions_mid;
    ft_vector<ft_ship_spatial_state> manual_defender_positions_mid;
    FT_ASSERT(manual_mode.get_assault_raider_positions(PLANET_TERRA, manual_raider_positions_mid));
    FT_ASSERT(manual_mode.get_assault_defender_positions(PLANET_TERRA, manual_defender_positions_mid));
    bool raider_progressed = false;
    for (size_t idx = 0; idx < manual_raider_positions_mid.size(); ++idx)
    {
        const ft_ship_spatial_state &after_state = manual_raider_positions_mid[idx];
        for (size_t j = 0; j < manual_raider_positions_start.size(); ++j)
        {
            if (manual_raider_positions_start[j].ship_uid == after_state.ship_uid)
            {
                if (after_state.z + 0.05 < manual_raider_positions_start[j].z)
                    raider_progressed = true;
                break;
            }
        }
        if (raider_progressed)
            break;
    }
    FT_ASSERT(raider_progressed);
    bool defender_shifted = false;
    for (size_t idx = 0; idx < manual_defender_positions_mid.size(); ++idx)
    {
        const ft_ship_spatial_state &after_state = manual_defender_positions_mid[idx];
        for (size_t j = 0; j < manual_defender_positions_start.size(); ++j)
        {
            if (manual_defender_positions_start[j].ship_uid == after_state.ship_uid)
            {
                double delta_x = math_fabs(after_state.x - manual_defender_positions_start[j].x);
                double delta_z = math_fabs(after_state.z - manual_defender_positions_start[j].z);
                if (delta_x > 0.05 || delta_z > 0.05)
                    defender_shifted = true;
                break;
            }
        }
        if (defender_shifted)
            break;
    }
    FT_ASSERT(defender_shifted);
    manual_mode.tick(0.75);
    auto_mode.tick(0.75);
    FT_ASSERT(manual_mode.is_assault_active(PLANET_TERRA));
    FT_ASSERT(auto_mode.is_assault_active(PLANET_TERRA));
    double manual_shield_after = manual_mode.get_assault_raider_shield(PLANET_TERRA);
    double auto_shield_after = auto_mode.get_assault_raider_shield(PLANET_TERRA);
    FT_ASSERT(manual_shield_after + 0.5 < auto_shield_after);
    int manual_hp_after = manual_mode.get_ship_hp(1, manual_capital);
    int auto_hp_after = auto_mode.get_ship_hp(1, auto_capital);
    FT_ASSERT(manual_hp_after >= auto_hp_after);
    FT_ASSERT(!manual_mode.trigger_assault_focus_fire(PLANET_TERRA));
    FT_ASSERT(!manual_mode.request_assault_tactical_pause(PLANET_TERRA));
    FT_ASSERT(manual_mode.set_assault_control_mode(PLANET_TERRA, ASSAULT_CONTROL_AUTO));
    FT_ASSERT(!manual_mode.trigger_assault_focus_fire(PLANET_TERRA));
    FT_ASSERT(!manual_mode.request_assault_tactical_pause(PLANET_TERRA));
    FT_ASSERT(!manual_mode.set_assault_support(PLANET_TERRA, false, true, false));
    FT_ASSERT(manual_mode.set_assault_control_mode(PLANET_TERRA, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(manual_mode.trigger_assault_focus_fire(PLANET_TERRA));
    FT_ASSERT(manual_mode.request_assault_tactical_pause(PLANET_TERRA));
    FT_ASSERT(manual_mode.set_assault_support(PLANET_TERRA, false, true, false));
    manual_mode.tick(0.5);
    auto_mode.tick(0.5);
    FT_ASSERT(manual_mode.is_assault_active(PLANET_TERRA));
    FT_ASSERT(auto_mode.is_assault_active(PLANET_TERRA));
    return 1;
}

int measure_assault_aggression_effects()
{
    Game cautious_assault(ft_string("127.0.0.1:8080"), ft_string("/"));
    cautious_assault.set_ore(PLANET_TERRA, ORE_IRON, 200);
    cautious_assault.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    cautious_assault.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(cautious_assault.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    cautious_assault.tick(0.0);
    cautious_assault.create_fleet(1);
    int cautious_capital = cautious_assault.create_ship(1, SHIP_CAPITAL);
    cautious_assault.set_ship_hp(1, cautious_capital, 220);
    cautious_assault.set_ship_shield(1, cautious_capital, 100);
    cautious_assault.create_fleet(2);
    int cautious_guard = cautious_assault.create_ship(2, SHIP_SHIELD);
    cautious_assault.set_ship_hp(2, cautious_guard, 140);
    cautious_assault.set_ship_shield(2, cautious_guard, 80);
    FT_ASSERT(cautious_assault.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(cautious_assault.set_assault_aggression(PLANET_TERRA, 0.65));
    FT_ASSERT(cautious_assault.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(cautious_assault.assign_fleet_to_assault(PLANET_TERRA, 2));
    for (int step = 0; step < 6; ++step)
        cautious_assault.tick(1.5);
    FT_ASSERT(cautious_assault.is_assault_active(PLANET_TERRA));

    Game ferocious_assault(ft_string("127.0.0.1:8080"), ft_string("/"));
    ferocious_assault.set_ore(PLANET_TERRA, ORE_IRON, 200);
    ferocious_assault.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    ferocious_assault.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(ferocious_assault.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    ferocious_assault.tick(0.0);
    ferocious_assault.create_fleet(1);
    int ferocious_capital = ferocious_assault.create_ship(1, SHIP_CAPITAL);
    ferocious_assault.set_ship_hp(1, ferocious_capital, 220);
    ferocious_assault.set_ship_shield(1, ferocious_capital, 100);
    ferocious_assault.create_fleet(2);
    int ferocious_guard = ferocious_assault.create_ship(2, SHIP_SHIELD);
    ferocious_assault.set_ship_hp(2, ferocious_guard, 140);
    ferocious_assault.set_ship_shield(2, ferocious_guard, 80);
    FT_ASSERT(ferocious_assault.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(ferocious_assault.set_assault_aggression(PLANET_TERRA, 1.55));
    FT_ASSERT(ferocious_assault.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(ferocious_assault.assign_fleet_to_assault(PLANET_TERRA, 2));
    for (int step = 0; step < 6; ++step)
        ferocious_assault.tick(1.5);
    FT_ASSERT(ferocious_assault.is_assault_active(PLANET_TERRA));

    ft_vector<ft_ship_spatial_state> cautious_positions;
    ft_vector<ft_ship_spatial_state> ferocious_positions;
    FT_ASSERT(cautious_assault.get_assault_raider_positions(PLANET_TERRA, cautious_positions));
    FT_ASSERT(ferocious_assault.get_assault_raider_positions(PLANET_TERRA, ferocious_positions));
    FT_ASSERT(cautious_positions.size() > 0);
    FT_ASSERT(ferocious_positions.size() > 0);
    double cautious_sum = 0.0;
    double ferocious_sum = 0.0;
    for (size_t idx = 0; idx < cautious_positions.size(); ++idx)
        cautious_sum += cautious_positions[idx].z;
    for (size_t idx = 0; idx < ferocious_positions.size(); ++idx)
        ferocious_sum += ferocious_positions[idx].z;
    double cautious_avg = cautious_sum / static_cast<double>(cautious_positions.size());
    double ferocious_avg = ferocious_sum / static_cast<double>(ferocious_positions.size());
    FT_ASSERT(ferocious_avg + 8.0 < cautious_avg);
    int cautious_capital_hp = cautious_assault.get_ship_hp(1, cautious_capital);
    int ferocious_capital_hp = ferocious_assault.get_ship_hp(1, ferocious_capital);
    FT_ASSERT(ferocious_capital_hp < cautious_capital_hp);

    Game idle_aggression(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(!idle_aggression.set_assault_aggression(PLANET_TERRA, 1.1));
    return 1;
}

int evaluate_focus_fire_cooldowns()
{
    Game focus_baseline(ft_string("127.0.0.1:8080"), ft_string("/"));
    focus_baseline.set_ore(PLANET_TERRA, ORE_IRON, 200);
    focus_baseline.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    focus_baseline.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(focus_baseline.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    focus_baseline.tick(0.0);
    focus_baseline.create_fleet(1);
    int focus_baseline_capital = focus_baseline.create_ship(1, SHIP_CAPITAL);
    focus_baseline.set_ship_hp(1, focus_baseline_capital, 220);
    focus_baseline.set_ship_shield(1, focus_baseline_capital, 100);
    focus_baseline.create_fleet(2);
    int focus_baseline_guard = focus_baseline.create_ship(2, SHIP_SHIELD);
    focus_baseline.set_ship_hp(2, focus_baseline_guard, 140);
    focus_baseline.set_ship_shield(2, focus_baseline_guard, 80);
    FT_ASSERT(focus_baseline.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    double baseline_focus_shield_start = focus_baseline.get_assault_raider_shield(PLANET_TERRA);
    double baseline_focus_hull_start = focus_baseline.get_assault_raider_hull(PLANET_TERRA);
    FT_ASSERT(focus_baseline.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(focus_baseline.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(focus_baseline.set_assault_support(PLANET_TERRA, false, false, false));
    focus_baseline.tick(1.0);
    double baseline_focus_shield_end = focus_baseline.get_assault_raider_shield(PLANET_TERRA);
    double baseline_focus_hull_end = focus_baseline.get_assault_raider_hull(PLANET_TERRA);
    double baseline_focus_shield_delta = baseline_focus_shield_start - baseline_focus_shield_end;
    double baseline_focus_hull_delta = baseline_focus_hull_start - baseline_focus_hull_end;
    FT_ASSERT(baseline_focus_shield_delta > 0.0);

    Game focus_burst(ft_string("127.0.0.1:8080"), ft_string("/"));
    focus_burst.set_ore(PLANET_TERRA, ORE_IRON, 200);
    focus_burst.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    focus_burst.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(focus_burst.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    focus_burst.tick(0.0);
    focus_burst.create_fleet(1);
    int focus_burst_capital = focus_burst.create_ship(1, SHIP_CAPITAL);
    focus_burst.set_ship_hp(1, focus_burst_capital, 220);
    focus_burst.set_ship_shield(1, focus_burst_capital, 100);
    focus_burst.create_fleet(2);
    int focus_burst_guard = focus_burst.create_ship(2, SHIP_SHIELD);
    focus_burst.set_ship_hp(2, focus_burst_guard, 140);
    focus_burst.set_ship_shield(2, focus_burst_guard, 80);
    FT_ASSERT(focus_burst.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    double burst_focus_shield_start = focus_burst.get_assault_raider_shield(PLANET_TERRA);
    double burst_focus_hull_start = focus_burst.get_assault_raider_hull(PLANET_TERRA);
    FT_ASSERT(math_fabs(burst_focus_shield_start - baseline_focus_shield_start) < 0.01);
    FT_ASSERT(math_fabs(burst_focus_hull_start - baseline_focus_hull_start) < 0.01);
    FT_ASSERT(focus_burst.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(focus_burst.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(focus_burst.set_assault_support(PLANET_TERRA, false, false, false));
    FT_ASSERT(focus_burst.trigger_assault_focus_fire(PLANET_TERRA));
    focus_burst.tick(1.0);
    double burst_focus_shield_end = focus_burst.get_assault_raider_shield(PLANET_TERRA);
    double burst_focus_hull_end = focus_burst.get_assault_raider_hull(PLANET_TERRA);
    double burst_focus_shield_delta = burst_focus_shield_start - burst_focus_shield_end;
    double burst_focus_hull_delta = burst_focus_hull_start - burst_focus_hull_end;
    FT_ASSERT(burst_focus_shield_delta > baseline_focus_shield_delta * 1.15);
    FT_ASSERT(burst_focus_hull_delta >= baseline_focus_hull_delta);

    Game focus_cooldown_balanced(ft_string("127.0.0.1:8080"), ft_string("/"));
    focus_cooldown_balanced.set_ore(PLANET_TERRA, ORE_IRON, 240);
    focus_cooldown_balanced.set_ore(PLANET_TERRA, ORE_COPPER, 240);
    focus_cooldown_balanced.set_ore(PLANET_TERRA, ORE_COAL, 240);
    FT_ASSERT(focus_cooldown_balanced.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    focus_cooldown_balanced.tick(0.0);
    double focus_balanced_pressure = focus_cooldown_balanced.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(focus_balanced_pressure < 0.1);
    focus_cooldown_balanced.create_fleet(1);
    int focus_balanced_guard = focus_cooldown_balanced.create_ship(1, SHIP_SHIELD);
    focus_cooldown_balanced.set_ship_hp(1, focus_balanced_guard, 20);
    focus_cooldown_balanced.set_ship_shield(1, focus_balanced_guard, 380);
    FT_ASSERT(focus_cooldown_balanced.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(focus_cooldown_balanced.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(focus_cooldown_balanced.trigger_assault_focus_fire(PLANET_TERRA));
    focus_cooldown_balanced.tick(4.0);
    FT_ASSERT(focus_cooldown_balanced.is_assault_active(PLANET_TERRA));
    FT_ASSERT(!focus_cooldown_balanced.trigger_assault_focus_fire(PLANET_TERRA));

    Game focus_cooldown_stressed(ft_string("127.0.0.1:8080"), ft_string("/"));
    focus_cooldown_stressed.set_ore(PLANET_TERRA, ORE_IRON, 280);
    focus_cooldown_stressed.set_ore(PLANET_TERRA, ORE_COPPER, 280);
    focus_cooldown_stressed.set_ore(PLANET_TERRA, ORE_COAL, 280);
    FT_ASSERT(focus_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_TRANSFER_NODE, 0, 3) != 0);
    FT_ASSERT(focus_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 0) != 0);
    FT_ASSERT(focus_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 2) != 0);
    FT_ASSERT(focus_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_PROCESSOR, 0, 1) != 0);
    focus_cooldown_stressed.tick(0.0);
    double focus_stressed_pressure = focus_cooldown_stressed.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(focus_stressed_pressure > 6.4);
    focus_cooldown_stressed.create_fleet(1);
    int focus_stressed_guard = focus_cooldown_stressed.create_ship(1, SHIP_SHIELD);
    focus_cooldown_stressed.set_ship_hp(1, focus_stressed_guard, 20);
    focus_cooldown_stressed.set_ship_shield(1, focus_stressed_guard, 380);
    FT_ASSERT(focus_cooldown_stressed.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(focus_cooldown_stressed.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(focus_cooldown_stressed.trigger_assault_focus_fire(PLANET_TERRA));
    focus_cooldown_stressed.tick(4.0);
    FT_ASSERT(focus_cooldown_stressed.is_assault_active(PLANET_TERRA));
    FT_ASSERT(!focus_cooldown_stressed.trigger_assault_focus_fire(PLANET_TERRA));

    focus_cooldown_balanced.tick(5.8);
    focus_cooldown_stressed.tick(5.8);
    FT_ASSERT(focus_cooldown_balanced.is_assault_active(PLANET_TERRA));
    FT_ASSERT(focus_cooldown_stressed.is_assault_active(PLANET_TERRA));
    FT_ASSERT(focus_cooldown_stressed.trigger_assault_focus_fire(PLANET_TERRA));
    FT_ASSERT(!focus_cooldown_balanced.trigger_assault_focus_fire(PLANET_TERRA));
    focus_cooldown_balanced.tick(2.5);
    FT_ASSERT(focus_cooldown_balanced.is_assault_active(PLANET_TERRA));
    FT_ASSERT(focus_cooldown_balanced.trigger_assault_focus_fire(PLANET_TERRA));
    return 1;
}

int validate_tactical_pause_behaviors()
{
    Game pause_baseline(ft_string("127.0.0.1:8080"), ft_string("/"));
    pause_baseline.set_ore(PLANET_TERRA, ORE_IRON, 200);
    pause_baseline.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    pause_baseline.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(pause_baseline.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    pause_baseline.tick(0.0);
    pause_baseline.create_fleet(1);
    int pause_baseline_capital = pause_baseline.create_ship(1, SHIP_CAPITAL);
    pause_baseline.set_ship_hp(1, pause_baseline_capital, 220);
    pause_baseline.set_ship_shield(1, pause_baseline_capital, 100);
    pause_baseline.create_fleet(2);
    int pause_baseline_guard = pause_baseline.create_ship(2, SHIP_SHIELD);
    pause_baseline.set_ship_hp(2, pause_baseline_guard, 140);
    pause_baseline.set_ship_shield(2, pause_baseline_guard, 80);
    FT_ASSERT(pause_baseline.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(pause_baseline.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(pause_baseline.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(pause_baseline.set_assault_support(PLANET_TERRA, false, false, false));
    pause_baseline.tick(2.0);
    int pause_baseline_hp = pause_baseline.get_ship_hp(1, pause_baseline_capital);
    int pause_baseline_shield = pause_baseline.get_ship_shield(1, pause_baseline_capital);

    Game pause_active(ft_string("127.0.0.1:8080"), ft_string("/"));
    pause_active.set_ore(PLANET_TERRA, ORE_IRON, 200);
    pause_active.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    pause_active.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(pause_active.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    pause_active.tick(0.0);
    pause_active.create_fleet(1);
    int pause_active_capital = pause_active.create_ship(1, SHIP_CAPITAL);
    pause_active.set_ship_hp(1, pause_active_capital, 220);
    pause_active.set_ship_shield(1, pause_active_capital, 100);
    pause_active.create_fleet(2);
    int pause_active_guard = pause_active.create_ship(2, SHIP_SHIELD);
    pause_active.set_ship_hp(2, pause_active_guard, 140);
    pause_active.set_ship_shield(2, pause_active_guard, 80);
    FT_ASSERT(pause_active.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(pause_active.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(pause_active.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(pause_active.set_assault_support(PLANET_TERRA, false, false, false));
    FT_ASSERT(pause_active.request_assault_tactical_pause(PLANET_TERRA));
    pause_active.tick(2.0);
    int pause_active_hp = pause_active.get_ship_hp(1, pause_active_capital);
    int pause_active_shield = pause_active.get_ship_shield(1, pause_active_capital);
    FT_ASSERT(pause_active_hp >= pause_baseline_hp);
    FT_ASSERT(pause_active_shield > pause_baseline_shield);

    Game pause_cooldown_balanced(ft_string("127.0.0.1:8080"), ft_string("/"));
    pause_cooldown_balanced.set_ore(PLANET_TERRA, ORE_IRON, 240);
    pause_cooldown_balanced.set_ore(PLANET_TERRA, ORE_COPPER, 240);
    pause_cooldown_balanced.set_ore(PLANET_TERRA, ORE_COAL, 240);
    FT_ASSERT(pause_cooldown_balanced.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    pause_cooldown_balanced.tick(0.0);
    double pause_balanced_pressure = pause_cooldown_balanced.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(pause_balanced_pressure < 0.1);
    pause_cooldown_balanced.create_fleet(1);
    int pause_balanced_guard = pause_cooldown_balanced.create_ship(1, SHIP_SHIELD);
    pause_cooldown_balanced.set_ship_hp(1, pause_balanced_guard, 20);
    pause_cooldown_balanced.set_ship_shield(1, pause_balanced_guard, 420);
    FT_ASSERT(pause_cooldown_balanced.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(pause_cooldown_balanced.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(pause_cooldown_balanced.request_assault_tactical_pause(PLANET_TERRA));
    pause_cooldown_balanced.tick(2.0);
    FT_ASSERT(pause_cooldown_balanced.is_assault_active(PLANET_TERRA));
    FT_ASSERT(!pause_cooldown_balanced.request_assault_tactical_pause(PLANET_TERRA));

    Game pause_cooldown_stressed(ft_string("127.0.0.1:8080"), ft_string("/"));
    pause_cooldown_stressed.set_ore(PLANET_TERRA, ORE_IRON, 280);
    pause_cooldown_stressed.set_ore(PLANET_TERRA, ORE_COPPER, 280);
    pause_cooldown_stressed.set_ore(PLANET_TERRA, ORE_COAL, 280);
    FT_ASSERT(pause_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_TRANSFER_NODE, 0, 3) != 0);
    FT_ASSERT(pause_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 0) != 0);
    FT_ASSERT(pause_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 2) != 0);
    FT_ASSERT(pause_cooldown_stressed.place_building(PLANET_TERRA, BUILDING_PROCESSOR, 0, 1) != 0);
    pause_cooldown_stressed.tick(0.0);
    double pause_stressed_pressure = pause_cooldown_stressed.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(pause_stressed_pressure > 6.4);
    pause_cooldown_stressed.create_fleet(1);
    int pause_stressed_guard = pause_cooldown_stressed.create_ship(1, SHIP_SHIELD);
    pause_cooldown_stressed.set_ship_hp(1, pause_stressed_guard, 20);
    pause_cooldown_stressed.set_ship_shield(1, pause_stressed_guard, 420);
    FT_ASSERT(pause_cooldown_stressed.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(pause_cooldown_stressed.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(pause_cooldown_stressed.request_assault_tactical_pause(PLANET_TERRA));
    pause_cooldown_stressed.tick(2.0);
    FT_ASSERT(pause_cooldown_stressed.is_assault_active(PLANET_TERRA));
    FT_ASSERT(!pause_cooldown_stressed.request_assault_tactical_pause(PLANET_TERRA));

    pause_cooldown_balanced.tick(15.8);
    pause_cooldown_stressed.tick(15.8);
    FT_ASSERT(pause_cooldown_balanced.is_assault_active(PLANET_TERRA));
    FT_ASSERT(pause_cooldown_stressed.is_assault_active(PLANET_TERRA));
    FT_ASSERT(pause_cooldown_stressed.request_assault_tactical_pause(PLANET_TERRA));
    FT_ASSERT(!pause_cooldown_balanced.request_assault_tactical_pause(PLANET_TERRA));
    pause_cooldown_balanced.tick(3.0);
    FT_ASSERT(pause_cooldown_balanced.is_assault_active(PLANET_TERRA));
    FT_ASSERT(pause_cooldown_balanced.request_assault_tactical_pause(PLANET_TERRA));
    return 1;
}

int verify_sunflare_targeting_rules()
{
    Game rules(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(unlock_shield_support(rules));
    FT_ASSERT(!rules.set_assault_sunflare_target(PLANET_TERRA, 1, 1));
    rules.create_fleet(1);
    int capital = rules.create_ship(1, SHIP_CAPITAL);
    rules.set_ship_hp(1, capital, 240);
    rules.set_ship_shield(1, capital, 160);
    rules.sub_ship_shield(1, capital, 120);
    rules.create_fleet(2);
    int sloop = rules.create_ship(2, SHIP_SUNFLARE_SLOOP);
    rules.set_ship_hp(2, sloop, 80);
    rules.set_ship_shield(2, sloop, 50);
    FT_ASSERT(rules.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(rules.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(rules.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(!rules.set_assault_sunflare_target(PLANET_TERRA, 1, capital));
    FT_ASSERT(rules.set_assault_control_mode(PLANET_TERRA, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(!rules.set_assault_sunflare_target(PLANET_TERRA, 1, capital));
    FT_ASSERT(rules.set_assault_support(PLANET_TERRA, true, false, false));
    FT_ASSERT(!rules.set_assault_sunflare_target(PLANET_TERRA, 99, capital));
    FT_ASSERT(!rules.set_assault_sunflare_target(PLANET_TERRA, 1, 999999));
    FT_ASSERT(rules.set_assault_sunflare_target(PLANET_TERRA, 1, capital));
    FT_ASSERT(rules.set_assault_sunflare_target(PLANET_TERRA, 0, 0));
    FT_ASSERT(rules.set_assault_support(PLANET_TERRA, false, false, false));
    FT_ASSERT(!rules.set_assault_sunflare_target(PLANET_TERRA, 1, capital));
    return 1;
}
