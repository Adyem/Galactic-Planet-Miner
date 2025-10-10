#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft_math_bridge.hpp"
#include "fleets.hpp"
#include "buildings.hpp"
#include "research.hpp"
#include "game_test_scenarios.hpp"

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
