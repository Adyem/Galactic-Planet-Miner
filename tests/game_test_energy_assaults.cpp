#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "fleets.hpp"
#include "buildings.hpp"
#include "quests.hpp"
#include "game_test_scenarios.hpp"

static int advance_game_to_climactic(Game &game)
{
    game.set_ore(PLANET_TERRA, ORE_IRON, 20);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 20);
    game.tick(0.0);

    game.create_fleet(1);
    int setup_one = game.create_ship(1, SHIP_SHIELD);
    FT_ASSERT(setup_one != 0);
    game.set_ship_hp(1, setup_one, 80);
    game.create_fleet(2);
    int setup_two = game.create_ship(2, SHIP_SHIELD);
    FT_ASSERT(setup_two != 0);
    game.set_ship_hp(2, setup_two, 60);
    game.tick(0.0);

    game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    game.tick(30.0);
    game.set_ore(PLANET_TERRA, ORE_MITHRIL, 8);
    game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    game.tick(40.0);
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, game.get_active_quest());

    game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    for (int convoy = 0; convoy < 8; ++convoy)
    {
        int moved = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 20);
        FT_ASSERT(moved >= 20);
        double waited = 0.0;
        while (game.get_active_convoy_count() > 0 && waited < 240.0)
        {
            game.tick(4.0);
            waited += 4.0;
        }
        FT_ASSERT(waited < 240.0);
    }
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_HIGH_VALUE_ESCORT));
    FT_ASSERT_EQ(QUEST_CLIMACTIC_BATTLE, game.get_active_quest());
    game.remove_ship(1, setup_one);
    game.remove_ship(2, setup_two);
    return 1;
}

int compare_storyline_assaults()
{
    Game early_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    early_game.set_ore(PLANET_TERRA, ORE_IRON, 200);
    early_game.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    early_game.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(early_game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    early_game.tick(0.0);
    early_game.create_fleet(1);
    int early_capital = early_game.create_ship(1, SHIP_CAPITAL);
    early_game.set_ship_hp(1, early_capital, 220);
    early_game.set_ship_shield(1, early_capital, 100);
    early_game.create_fleet(2);
    int early_guard = early_game.create_ship(2, SHIP_SHIELD);
    early_game.set_ship_hp(2, early_guard, 140);
    early_game.set_ship_shield(2, early_guard, 80);
    FT_ASSERT(early_game.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(early_game.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(early_game.assign_fleet_to_assault(PLANET_TERRA, 2));
    early_game.tick(3.0);
    int early_hp = early_game.get_ship_hp(1, early_capital);
    FT_ASSERT(early_game.is_assault_active(PLANET_TERRA));

    Game narrative_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(advance_game_to_climactic(narrative_game));
    narrative_game.set_ore(PLANET_TERRA, ORE_IRON, 200);
    narrative_game.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    narrative_game.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(narrative_game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    narrative_game.tick(0.0);
    int narrative_capital = narrative_game.create_ship(1, SHIP_CAPITAL);
    narrative_game.set_ship_hp(1, narrative_capital, 220);
    narrative_game.set_ship_shield(1, narrative_capital, 100);
    int narrative_guard = narrative_game.create_ship(2, SHIP_SHIELD);
    narrative_game.set_ship_hp(2, narrative_guard, 140);
    narrative_game.set_ship_shield(2, narrative_guard, 80);
    FT_ASSERT(narrative_game.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(narrative_game.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(narrative_game.assign_fleet_to_assault(PLANET_TERRA, 2));
    narrative_game.tick(3.0);
    int narrative_hp = narrative_game.get_ship_hp(1, narrative_capital);
    FT_ASSERT(narrative_game.is_assault_active(PLANET_TERRA));
    FT_ASSERT(narrative_hp < early_hp);

    ft_vector<ft_ship_spatial_state> early_raider_positions;
    ft_vector<ft_ship_spatial_state> narrative_raider_positions;
    FT_ASSERT(early_game.get_assault_raider_positions(PLANET_TERRA, early_raider_positions));
    FT_ASSERT(narrative_game.get_assault_raider_positions(PLANET_TERRA, narrative_raider_positions));
    int early_capital_count = 0;
    int narrative_capital_count = 0;
    for (size_t idx = 0; idx < early_raider_positions.size(); ++idx)
    {
        if (early_raider_positions[idx].ship_type == SHIP_CAPITAL)
            early_capital_count += 1;
    }
    for (size_t idx = 0; idx < narrative_raider_positions.size(); ++idx)
    {
        if (narrative_raider_positions[idx].ship_type == SHIP_CAPITAL)
            narrative_capital_count += 1;
    }
    FT_ASSERT_EQ(0, early_capital_count);
    FT_ASSERT(narrative_capital_count >= 1);
    FT_ASSERT(narrative_raider_positions.size() > early_raider_positions.size());
    return 1;
}

int verify_narrative_assault_spikes()
{
    Game baseline(ft_string("127.0.0.1:8080"), ft_string("/"));
    baseline.set_ore(PLANET_TERRA, ORE_IRON, 200);
    baseline.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    baseline.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(baseline.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    baseline.tick(0.0);
    baseline.create_fleet(1);
    int baseline_capital = baseline.create_ship(1, SHIP_CAPITAL);
    baseline.set_ship_hp(1, baseline_capital, 220);
    baseline.set_ship_shield(1, baseline_capital, 120);
    baseline.create_fleet(2);
    int baseline_guard = baseline.create_ship(2, SHIP_SHIELD);
    baseline.set_ship_hp(2, baseline_guard, 140);
    baseline.set_ship_shield(2, baseline_guard, 90);
    FT_ASSERT(baseline.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(baseline.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(baseline.assign_fleet_to_assault(PLANET_TERRA, 2));
    int baseline_start_shield = baseline.get_ship_shield(1, baseline_capital);
    int baseline_start_hp = baseline.get_ship_hp(1, baseline_capital);
    baseline.tick(1.2);
    int baseline_end_shield = baseline.get_ship_shield(1, baseline_capital);
    int baseline_end_hp = baseline.get_ship_hp(1, baseline_capital);
    int baseline_shield_loss = baseline_start_shield - baseline_end_shield;
    int baseline_hp_loss = baseline_start_hp - baseline_end_hp;
    if (baseline_shield_loss < 0)
        baseline_shield_loss = 0;
    if (baseline_hp_loss < 0)
        baseline_hp_loss = 0;

    Game narrative(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(advance_game_to_climactic(narrative));
    narrative.set_ore(PLANET_TERRA, ORE_IRON, 220);
    narrative.set_ore(PLANET_TERRA, ORE_COPPER, 220);
    narrative.set_ore(PLANET_TERRA, ORE_COAL, 220);
    FT_ASSERT(narrative.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    narrative.tick(0.0);
    narrative.create_fleet(1);
    int narrative_capital = narrative.create_ship(1, SHIP_CAPITAL);
    narrative.set_ship_hp(1, narrative_capital, 220);
    narrative.set_ship_shield(1, narrative_capital, 120);
    narrative.create_fleet(2);
    int narrative_guard = narrative.create_ship(2, SHIP_SHIELD);
    narrative.set_ship_hp(2, narrative_guard, 140);
    narrative.set_ship_shield(2, narrative_guard, 90);
    FT_ASSERT(narrative.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(narrative.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(narrative.assign_fleet_to_assault(PLANET_TERRA, 2));
    int narrative_start_shield = narrative.get_ship_shield(1, narrative_capital);
    int narrative_start_hp = narrative.get_ship_hp(1, narrative_capital);
    narrative.tick(1.2);
    int narrative_end_shield = narrative.get_ship_shield(1, narrative_capital);
    int narrative_end_hp = narrative.get_ship_hp(1, narrative_capital);
    int narrative_shield_loss = narrative_start_shield - narrative_end_shield;
    int narrative_hp_loss = narrative_start_hp - narrative_end_hp;
    if (narrative_shield_loss < 0)
        narrative_shield_loss = 0;
    if (narrative_hp_loss < 0)
        narrative_hp_loss = 0;
    int baseline_combined = baseline_shield_loss + baseline_hp_loss;
    int narrative_combined = narrative_shield_loss + narrative_hp_loss;
    FT_ASSERT(narrative_combined > baseline_combined + 10);
    FT_ASSERT(narrative_shield_loss > baseline_shield_loss);
    return 1;
}
