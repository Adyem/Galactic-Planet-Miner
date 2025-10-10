#include "../libft/Libft/libft.hpp"
#include "../libft/Math/math.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "buildings.hpp"
#include "game_test_scenarios.hpp"
#include "research.hpp"

int verify_radar_static_defense_support()
{
    Game baseline(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT_EQ(0, baseline.get_planet_escort_rating(PLANET_TERRA));
    double baseline_modifier = baseline.get_planet_convoy_raid_risk_modifier(PLANET_TERRA);
    FT_ASSERT(math_fabs(baseline_modifier) < 0.000001);

    Game radar(ft_string("127.0.0.1:8080"), ft_string("/"));
    radar.ensure_planet_item_slot(PLANET_TERRA, ITEM_COPPER_BAR);
    radar.ensure_planet_item_slot(PLANET_TERRA, ITEM_MITHRIL_BAR);
    radar.set_ore(PLANET_TERRA, ORE_IRON, 120);
    radar.set_ore(PLANET_TERRA, ORE_COPPER, 120);
    radar.set_ore(PLANET_TERRA, ORE_COAL, 120);
    radar.set_ore(PLANET_TERRA, ORE_MITHRIL, 120);
    radar.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 60);
    radar.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 40);

    FT_ASSERT(radar.start_research(RESEARCH_UNLOCK_MARS));
    radar.tick(30.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, radar.get_research_status(RESEARCH_UNLOCK_MARS));
    FT_ASSERT(radar.start_research(RESEARCH_UNLOCK_ZALTHOR));
    radar.tick(40.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, radar.get_research_status(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT(radar.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    radar.tick(30.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, radar.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_I));

    int first_radar = radar.place_building(PLANET_TERRA, BUILDING_PROXIMITY_RADAR, 0, 0);
    if (first_radar == 0)
        first_radar = radar.place_building(PLANET_TERRA, BUILDING_PROXIMITY_RADAR, 1, 0);
    FT_ASSERT(first_radar != 0);
    radar.tick(0.0);

    int proximity_rating = radar.get_planet_escort_rating(PLANET_TERRA);
    double proximity_modifier = radar.get_planet_convoy_raid_risk_modifier(PLANET_TERRA);
    FT_ASSERT(proximity_rating >= 2);
    FT_ASSERT(proximity_modifier > 0.07 && proximity_modifier < 0.09);

    FT_ASSERT(radar.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    radar.tick(40.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, radar.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    int mobile_radar = radar.place_building(PLANET_TERRA, BUILDING_MOBILE_RADAR, 2, 0);
    if (mobile_radar == 0)
        mobile_radar = radar.place_building(PLANET_TERRA, BUILDING_MOBILE_RADAR, 3, 0);
    FT_ASSERT(mobile_radar != 0);
    radar.tick(0.0);

    int enhanced_rating = radar.get_planet_escort_rating(PLANET_TERRA);
    double enhanced_modifier = radar.get_planet_convoy_raid_risk_modifier(PLANET_TERRA);
    FT_ASSERT(enhanced_rating > proximity_rating);
    FT_ASSERT(enhanced_modifier > proximity_modifier);
    FT_ASSERT(enhanced_modifier < 0.21);
    return 1;
}
