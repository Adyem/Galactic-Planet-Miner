#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft_math_bridge.hpp"
#include "fleets.hpp"
#include "buildings.hpp"
#include "research.hpp"
#include "game_test_scenarios.hpp"

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
