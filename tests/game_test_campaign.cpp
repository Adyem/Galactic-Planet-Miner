#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/pair.hpp"
#include "fleets.hpp"
#include "research.hpp"
#include "quests.hpp"
#include "buildings.hpp"
#include "game_test_scenarios.hpp"

static int fast_forward_to_supply_quests(Game &game)
{
    game.set_ore(PLANET_TERRA, ORE_IRON, 20);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 20);
    game.tick(0.0);

    game.create_fleet(1);
    int capital = game.create_ship(1, SHIP_CAPITAL);
    FT_ASSERT(capital != 0);
    game.set_ship_hp(1, capital, 140);
    game.create_fleet(2);
    int escort = game.create_ship(2, SHIP_SHIELD);
    FT_ASSERT(escort != 0);
    game.set_ship_hp(2, escort, 80);
    game.tick(0.0);

    game.set_ore(PLANET_TERRA, ORE_IRON, 120);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 80);
    game.set_ore(PLANET_TERRA, ORE_COAL, 24);
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    game.tick(40.0);
    game.set_ore(PLANET_TERRA, ORE_COAL, 24);
    game.set_ore(PLANET_TERRA, ORE_MITHRIL, 12);
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    game.tick(50.0);
    game.tick(0.0);
    return 1;
}

int validate_initial_campaign_flow(Game &game)
{
    FT_ASSERT_EQ(GAME_DIFFICULTY_STANDARD, game.get_difficulty());
    FT_ASSERT(game.is_planet_unlocked(PLANET_TERRA));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_MARS));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_ZALTHOR));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_VULCAN));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_NOCTARIS_PRIME));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_LUNA));

    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_UNLOCK_MARS));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_UNLOCK_VULCAN));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_UNLOCK_NOCTARIS));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_SOLAR_PANELS));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_CRAFTING_MASTERY));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_UNLOCK_LUNA));
    FT_ASSERT(!game.can_start_research(RESEARCH_UNLOCK_MARS));
    FT_ASSERT(!game.can_start_research(RESEARCH_UNLOCK_ZALTHOR));

    FT_ASSERT_EQ(QUEST_INITIAL_SKIRMISHES, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_INITIAL_SKIRMISHES));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_DEFENSE_OF_TERRA));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_INVESTIGATE_RAIDERS));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_CLIMACTIC_BATTLE));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_CRITICAL_DECISION));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_ORDER_SUPPRESS_RAIDS));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_ORDER_DOMINION));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_REBELLION_NETWORK));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_REBELLION_LIBERATION));
    FT_ASSERT_EQ(QUEST_CHOICE_NONE, game.get_quest_choice(QUEST_CRITICAL_DECISION));
    game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 0);

    int ore = game.add_ore(PLANET_TERRA, ORE_COPPER, 5);
    FT_ASSERT_EQ(5, ore);
    ore = game.sub_ore(PLANET_TERRA, ORE_COPPER, 2);
    FT_ASSERT_EQ(3, ore);
    game.set_ore(PLANET_TERRA, ORE_IRON, 7);
    FT_ASSERT_EQ(7, game.get_ore(PLANET_TERRA, ORE_IRON));

    double rate = game.get_rate(PLANET_TERRA, ORE_IRON);
    FT_ASSERT(rate > 0.49 && rate < 0.51);
    const ft_vector<Pair<int, double> > &terra_res = game.get_planet_resources(PLANET_TERRA);
    FT_ASSERT_EQ(3, static_cast<int>(terra_res.size()));
    FT_ASSERT(game.get_rate(PLANET_MARS, ORE_MITHRIL) < 0.0001);

    game.produce(10.0);
    FT_ASSERT_EQ(12, game.get_ore(PLANET_TERRA, ORE_IRON));
    FT_ASSERT(!game.start_research(RESEARCH_UNLOCK_MARS));

    game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_INITIAL_SKIRMISHES));
    FT_ASSERT_EQ(QUEST_DEFENSE_OF_TERRA, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_DEFENSE_OF_TERRA));
    int parts_after_skirmish = game.get_ore(PLANET_TERRA, ITEM_ENGINE_PART);
    FT_ASSERT(parts_after_skirmish >= 2);
    double defense_timer = game.get_quest_time_remaining(QUEST_DEFENSE_OF_TERRA);
    FT_ASSERT(defense_timer > 179.9 && defense_timer < 180.1);
    size_t lore_before_failure = game.get_lore_log().size();
    game.tick(200.0);
    int parts_after_failure = game.get_ore(PLANET_TERRA, ITEM_ENGINE_PART);
    FT_ASSERT(parts_after_failure < parts_after_skirmish);
    size_t lore_after_failure = game.get_lore_log().size();
    FT_ASSERT(lore_after_failure > lore_before_failure);
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_DEFENSE_OF_TERRA));
    defense_timer = game.get_quest_time_remaining(QUEST_DEFENSE_OF_TERRA);
    FT_ASSERT(defense_timer > 179.9 && defense_timer < 180.1);
    game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    FT_ASSERT(game.can_start_research(RESEARCH_UNLOCK_MARS));
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    FT_ASSERT_EQ(RESEARCH_STATUS_IN_PROGRESS, game.get_research_status(RESEARCH_UNLOCK_MARS));
    double remaining = game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(remaining > 29.9 && remaining < 30.1);
    game.tick(10.0);
    double after = game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(after > 19.9 && after < 20.1);
    game.tick(25.0);
    FT_ASSERT(game.is_planet_unlocked(PLANET_MARS));
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_MARS));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_SOLAR_PANELS));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_CRAFTING_MASTERY));
    double mithril_rate = game.get_rate(PLANET_MARS, ORE_MITHRIL);
    FT_ASSERT(mithril_rate > 0.049 && mithril_rate < 0.051);

    game.set_ore(PLANET_MARS, ORE_IRON, 0);
    game.set_ore(PLANET_MARS, ORE_COPPER, 0);
    game.set_ore(PLANET_MARS, ORE_MITHRIL, 0);
    game.set_ore(PLANET_MARS, ORE_COAL, 0);
    game.produce(40.0);
    FT_ASSERT(game.get_ore(PLANET_MARS, ORE_MITHRIL) >= 2);

    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_UNLOCK_LUNA));
    game.set_ore(PLANET_TERRA, ORE_IRON, 70);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 50);
    game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 12);
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_LUNA));
    game.tick(40.0);
    FT_ASSERT(game.is_planet_unlocked(PLANET_LUNA));
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_LUNA));
    double luna_titanium = game.get_rate(PLANET_LUNA, ORE_TITANIUM);
    FT_ASSERT(luna_titanium > 0.039 && luna_titanium < 0.041);

    game.set_ore(PLANET_TERRA, ORE_COAL, 3);
    game.set_ore(PLANET_MARS, ORE_COAL, 4);
    game.set_ore(PLANET_MARS, ORE_MITHRIL, 9);
    int coal_before = game.get_ore(PLANET_TERRA, ORE_COAL) + game.get_ore(PLANET_MARS, ORE_COAL);
    int mithril_before = game.get_ore(PLANET_TERRA, ORE_MITHRIL) + game.get_ore(PLANET_MARS, ORE_MITHRIL);
    FT_ASSERT(game.can_start_research(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT_EQ(RESEARCH_STATUS_IN_PROGRESS, game.get_research_status(RESEARCH_UNLOCK_ZALTHOR));
    int coal_after = game.get_ore(PLANET_TERRA, ORE_COAL) + game.get_ore(PLANET_MARS, ORE_COAL);
    int mithril_after = game.get_ore(PLANET_TERRA, ORE_MITHRIL) + game.get_ore(PLANET_MARS, ORE_MITHRIL);
    FT_ASSERT_EQ(coal_before - 6, coal_after);
    FT_ASSERT_EQ(mithril_before - 8, mithril_after);
    game.tick(20.0);
    double z_remaining = game.get_research_time_remaining(RESEARCH_UNLOCK_ZALTHOR);
    FT_ASSERT(z_remaining > 19.9 && z_remaining < 20.1);
    game.tick(25.0);
    FT_ASSERT(game.is_planet_unlocked(PLANET_ZALTHOR));
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_UNLOCK_VULCAN));
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_INVESTIGATE_RAIDERS));
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_SECURE_SUPPLY_LINES));
    FT_ASSERT_EQ(0, game.get_total_convoys_delivered());
    FT_ASSERT_EQ(0, game.get_convoy_delivery_streak());
    FT_ASSERT_EQ(0, game.get_convoy_raid_losses());
    game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    for (int shipment = 0; shipment < 8; ++shipment)
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
    FT_ASSERT(game.get_total_convoys_delivered() >= 8);
    FT_ASSERT(game.get_convoy_delivery_streak() >= 8);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_SECURE_SUPPLY_LINES));
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_STEADY_SUPPLY_STREAK));
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_HIGH_VALUE_ESCORT));
    FT_ASSERT_EQ(QUEST_CLIMACTIC_BATTLE, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_CLIMACTIC_BATTLE));

    game.set_ore(PLANET_ZALTHOR, ORE_GOLD, 7);
    game.set_ore(PLANET_MARS, ORE_MITHRIL, 12);
    int gold_before = game.get_ore(PLANET_ZALTHOR, ORE_GOLD);
    int mithril_before_v = game.get_ore(PLANET_MARS, ORE_MITHRIL);
    FT_ASSERT(game.can_start_research(RESEARCH_UNLOCK_VULCAN));
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_VULCAN));
    FT_ASSERT_EQ(RESEARCH_STATUS_IN_PROGRESS, game.get_research_status(RESEARCH_UNLOCK_VULCAN));
    int gold_after = game.get_ore(PLANET_ZALTHOR, ORE_GOLD);
    int mithril_after_v = game.get_ore(PLANET_MARS, ORE_MITHRIL);
    FT_ASSERT_EQ(gold_before - 6, gold_after);
    FT_ASSERT_EQ(mithril_before_v - 8, mithril_after_v);
    game.tick(30.0);
    double v_remaining = game.get_research_time_remaining(RESEARCH_UNLOCK_VULCAN);
    FT_ASSERT(v_remaining > 24.9 && v_remaining < 25.1);
    game.tick(30.0);
    FT_ASSERT(game.is_planet_unlocked(PLANET_VULCAN));
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_VULCAN));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_UNLOCK_NOCTARIS));
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_CLIMACTIC_BATTLE));
    FT_ASSERT_EQ(QUEST_CRITICAL_DECISION, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_AWAITING_CHOICE, game.get_quest_status(QUEST_CRITICAL_DECISION));
    FT_ASSERT(game.resolve_quest_choice(QUEST_CRITICAL_DECISION, QUEST_CHOICE_SPARE_BLACKTHORNE));
    FT_ASSERT_EQ(QUEST_CHOICE_SPARE_BLACKTHORNE, game.get_quest_choice(QUEST_CRITICAL_DECISION));
    int crystal_reward = game.get_ore(PLANET_TERRA, ORE_CRYSTAL);
    FT_ASSERT(crystal_reward >= 3);
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_ORDER_UPRISING));
    FT_ASSERT_EQ(QUEST_REBELLION_FLEET, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_REBELLION_FLEET));

    game.set_ore(PLANET_VULCAN, ORE_TIN, 9);
    game.set_ore(PLANET_VULCAN, ORE_SILVER, 8);
    game.set_ore(PLANET_VULCAN, ORE_TITANIUM, 7);
    int tin_before = game.get_ore(PLANET_VULCAN, ORE_TIN);
    int silver_before = game.get_ore(PLANET_VULCAN, ORE_SILVER);
    int titanium_before = game.get_ore(PLANET_VULCAN, ORE_TITANIUM);
    FT_ASSERT(game.can_start_research(RESEARCH_UNLOCK_NOCTARIS));
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_NOCTARIS));
    FT_ASSERT_EQ(RESEARCH_STATUS_IN_PROGRESS, game.get_research_status(RESEARCH_UNLOCK_NOCTARIS));
    int tin_after = game.get_ore(PLANET_VULCAN, ORE_TIN);
    int silver_after = game.get_ore(PLANET_VULCAN, ORE_SILVER);
    int titanium_after = game.get_ore(PLANET_VULCAN, ORE_TITANIUM);
    FT_ASSERT_EQ(tin_before - 6, tin_after);
    FT_ASSERT_EQ(silver_before - 6, silver_after);
    FT_ASSERT_EQ(titanium_before - 5, titanium_after);
    game.tick(30.0);
    double n_remaining = game.get_research_time_remaining(RESEARCH_UNLOCK_NOCTARIS);
    FT_ASSERT(n_remaining > 29.9 && n_remaining < 30.1);
    game.tick(35.0);
    FT_ASSERT(game.is_planet_unlocked(PLANET_NOCTARIS_PRIME));
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_NOCTARIS));
    game.set_ore(PLANET_NOCTARIS_PRIME, ORE_OBSIDIAN, 6);
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_CHOICE_SPARE_BLACKTHORNE, game.get_quest_choice(QUEST_CRITICAL_DECISION));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_ORDER_UPRISING));
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_REBELLION_FLEET));
    FT_ASSERT_EQ(QUEST_REBELLION_NETWORK, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_REBELLION_NETWORK));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_ORDER_SUPPRESS_RAIDS));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_ORDER_DOMINION));

    game.create_fleet(98);
    FT_ASSERT_EQ(0, game.create_ship(98, SHIP_CAPITAL));
    game.remove_fleet(98);

    game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 12);
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 12);
    game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 12);
    game.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 6);
    game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 6);
    game.set_ore(PLANET_NOCTARIS_PRIME, ORE_OBSIDIAN, 80);
    game.set_ore(PLANET_NOCTARIS_PRIME, ORE_CRYSTAL, 80);
    game.set_ore(PLANET_NOCTARIS_PRIME, ORE_TRITIUM, 60);

    FT_ASSERT(game.start_research(RESEARCH_SOLAR_PANELS));
    game.tick(35.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_SOLAR_PANELS));
    FT_ASSERT(game.start_research(RESEARCH_CRAFTING_MASTERY));
    game.tick(45.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_CRAFTING_MASTERY));

    FT_ASSERT(game.start_research(RESEARCH_INTERSTELLAR_TRADE));
    game.tick(35.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_INTERSTELLAR_TRADE));

    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_COPPER_BAR);
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART);
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_ACCUMULATOR);
    game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 40);
    int terra_relay = game.place_building(PLANET_TERRA, BUILDING_TRADE_RELAY, 2, 0);
    FT_ASSERT(terra_relay != 0);
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_REBELLION_NETWORK));
    FT_ASSERT_EQ(QUEST_REBELLION_LIBERATION, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_REBELLION_LIBERATION));
    FT_ASSERT(game.is_assault_active(PLANET_ZALTHOR));

    game.create_fleet(77);
    int liberation_flagship = game.create_ship(77, SHIP_CAPITAL);
    FT_ASSERT(liberation_flagship != 0);
    game.set_ship_hp(77, liberation_flagship, 200);
    game.set_ship_shield(77, liberation_flagship, 80);
    game.set_ship_armor(77, liberation_flagship, 60);
    FT_ASSERT(game.assign_fleet_to_assault(PLANET_ZALTHOR, 77));
    double liberation_elapsed = 0.0;
    while (game.is_assault_active(PLANET_ZALTHOR) && liberation_elapsed < 180.0)
    {
        game.tick(2.0);
        liberation_elapsed += 2.0;
    }
    FT_ASSERT(!game.is_assault_active(PLANET_ZALTHOR));
    FT_ASSERT(liberation_elapsed < 180.0);
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_REBELLION_LIBERATION));
    FT_ASSERT_EQ(0, game.get_active_quest());

    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 12);
    game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 12);
    game.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 6);

    FT_ASSERT(game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    game.tick(40.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    FT_ASSERT(game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    game.tick(45.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    FT_ASSERT(game.start_research(RESEARCH_SHIELD_TECHNOLOGY));
    game.tick(40.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_SHIELD_TECHNOLOGY));
    FT_ASSERT(game.start_research(RESEARCH_REPAIR_DRONE_TECHNOLOGY));
    game.tick(50.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_REPAIR_DRONE_TECHNOLOGY));
    FT_ASSERT(game.start_research(RESEARCH_ARMAMENT_ENHANCEMENT_I));
    game.tick(40.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_I));
    FT_ASSERT(game.start_research(RESEARCH_ARMAMENT_ENHANCEMENT_II));
    game.tick(50.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_II));
    FT_ASSERT(game.start_research(RESEARCH_CAPITAL_SHIP_INITIATIVE));
    game.tick(60.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_CAPITAL_SHIP_INITIATIVE));

    game.create_fleet(1);
    int ship_a = game.create_ship(1, SHIP_CAPITAL);
    game.set_ship_hp(1, ship_a, 120);
    game.set_ship_shield(1, ship_a, 40);
    game.set_ship_armor(1, ship_a, 30);
    game.create_fleet(1);
    int ship_b = game.create_ship(1, SHIP_SHIELD);
    FT_ASSERT(ship_b != 0);
    FT_ASSERT_EQ(0, game.get_ship_hp(1, ship_b));
    game.set_ship_hp(1, ship_b, 100);
    game.set_ship_shield(1, ship_b, 50);
    game.set_ship_armor(1, ship_b, 20);
    FT_ASSERT_EQ(120, game.get_ship_hp(1, ship_a));
    FT_ASSERT_EQ(40, game.get_ship_shield(1, ship_a));
    FT_ASSERT_EQ(30, game.get_ship_armor(1, ship_a));
    FT_ASSERT_EQ(100, game.get_ship_hp(1, ship_b));
    FT_ASSERT_EQ(50, game.get_ship_shield(1, ship_b));
    FT_ASSERT_EQ(20, game.get_ship_armor(1, ship_b));
    FT_ASSERT_EQ(140, game.add_ship_hp(1, ship_a, 20));
    FT_ASSERT_EQ(80, game.sub_ship_shield(1, ship_b, 20));
    FT_ASSERT_EQ(40, game.sub_ship_armor(1, ship_b, 20));
    FT_ASSERT_EQ(0, game.sub_ship_armor(1, ship_b, 40));
    FT_ASSERT_EQ(70, game.add_ship_shield(1, ship_a, 20));
    FT_ASSERT_EQ(0, game.sub_ship_shield(1, ship_a, 100));
    game.sub_ship_hp(1, ship_a, 40);
    FT_ASSERT_EQ(60, game.get_ship_hp(1, ship_a));
    FT_ASSERT_EQ(90, game.add_ship_hp(1, ship_a, 30));
    game.set_ship_hp(1, ship_b, 80);
    FT_ASSERT_EQ(80, game.get_ship_hp(1, ship_b));
    FT_ASSERT_EQ(90, game.get_ship_hp(1, ship_a));
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_DEFENSE_OF_TERRA));
    FT_ASSERT_EQ(QUEST_INVESTIGATE_RAIDERS, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_INVESTIGATE_RAIDERS));
    game.set_fleet_location_planet(1, PLANET_TERRA);
    ft_location loc1 = game.get_fleet_location(1);
    FT_ASSERT_EQ(LOCATION_PLANET, loc1.type);
    FT_ASSERT_EQ(PLANET_TERRA, loc1.from);

    game.create_fleet(2);
    int ship_c = game.create_ship(2, SHIP_SHIELD);
    FT_ASSERT(ship_c != ship_a);
    FT_ASSERT(ship_c != ship_b);
    game.set_ship_hp(2, ship_c, 120);
    FT_ASSERT_EQ(120, game.get_ship_hp(2, ship_c));
    game.set_fleet_location_travel(2, PLANET_MARS, PLANET_VULCAN, 5.0);
    ft_location loc2 = game.get_fleet_location(2);
    FT_ASSERT_EQ(LOCATION_TRAVEL, loc2.type);
    FT_ASSERT_EQ(PLANET_MARS, loc2.from);
    FT_ASSERT_EQ(PLANET_VULCAN, loc2.to);
    double travel_time = game.get_fleet_travel_time(2);
    FT_ASSERT(travel_time > 4.9 && travel_time < 5.1);

    game.set_fleet_location_misc(1, MISC_OUTPOST_NEBULA_X);
    ft_location loc3 = game.get_fleet_location(1);
    FT_ASSERT_EQ(LOCATION_MISC, loc3.type);
    FT_ASSERT_EQ(MISC_OUTPOST_NEBULA_X, loc3.misc);

    game.remove_ship(1, ship_b);
    FT_ASSERT_EQ(0, game.get_ship_hp(1, ship_b));
    FT_ASSERT_EQ(0, game.add_ship_hp(1, ship_b, 10));

    game.transfer_ship(1, 2, ship_a);
    FT_ASSERT_EQ(0, game.get_ship_hp(1, ship_a));
    FT_ASSERT_EQ(90, game.get_ship_hp(2, ship_a));

    game.set_ore(PLANET_TERRA, ORE_COPPER, 0);
    game.tick(2.0);
    FT_ASSERT_EQ(1, game.get_ore(PLANET_TERRA, ORE_COPPER));
    ft_location loc2a = game.get_fleet_location(2);
    FT_ASSERT_EQ(LOCATION_TRAVEL, loc2a.type);
    travel_time = game.get_fleet_travel_time(2);
    FT_ASSERT(travel_time > 2.9 && travel_time < 3.1);
    game.tick(3.0);
    FT_ASSERT_EQ(2, game.get_ore(PLANET_TERRA, ORE_COPPER));
    ft_location loc4 = game.get_fleet_location(2);
    FT_ASSERT_EQ(LOCATION_PLANET, loc4.type);
    FT_ASSERT_EQ(PLANET_VULCAN, loc4.from);

    game.create_fleet(3);
    int ship_d = game.create_ship(3, SHIP_SHIELD);
    FT_ASSERT(ship_d != 0);
    game.set_ship_hp(3, ship_d, 42);
    game.remove_fleet(3, 2);
    FT_ASSERT_EQ(42, game.get_ship_hp(2, ship_d));
    FT_ASSERT_EQ(0, game.get_ship_hp(3, ship_d));
    FT_ASSERT_EQ(0, game.create_ship(3, SHIP_SHIELD));
    ft_location loc5 = game.get_fleet_location(3);
    FT_ASSERT_EQ(PLANET_TERRA, loc5.from);

    game.create_fleet(4);
    int ship_e = game.create_ship(4, SHIP_SHIELD);
    FT_ASSERT(ship_e != 0);
    game.set_ship_hp(4, ship_e, 37);
    game.remove_fleet(4, -1, PLANET_MARS);
    FT_ASSERT_EQ(0, game.get_ship_hp(4, ship_e));
    FT_ASSERT_EQ(37, game.get_planet_fleet_ship_hp(PLANET_MARS, ship_e));
    ft_location mars_garrison = game.get_planet_fleet_location(PLANET_MARS);
    FT_ASSERT_EQ(LOCATION_PLANET, mars_garrison.type);
    FT_ASSERT_EQ(PLANET_MARS, mars_garrison.from);

    game.create_fleet(5);
    int ship_f = game.create_ship(5, SHIP_SHIELD);
    FT_ASSERT(ship_f != 0);
    game.set_ship_hp(5, ship_f, 18);
    game.remove_fleet(5, -1, PLANET_MARS);
    FT_ASSERT_EQ(37, game.get_planet_fleet_ship_hp(PLANET_MARS, ship_e));
    FT_ASSERT_EQ(18, game.get_planet_fleet_ship_hp(PLANET_MARS, ship_f));

    game.create_fleet(6);
    int ship_g = game.create_ship(6, SHIP_CAPITAL);
    FT_ASSERT(ship_g != 0);
    game.set_ship_hp(6, ship_g, 150);
    game.set_ship_shield(6, ship_g, 70);
    game.set_ship_armor(6, ship_g, 40);
    game.set_ship_shield(2, ship_a, 45);
    game.set_ship_shield(2, ship_c, 30);

    game.set_ore(PLANET_TERRA, ORE_IRON, 0);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 0);
    game.set_ore(PLANET_TERRA, ORE_COAL, 0);
    size_t lore_before = game.get_lore_log().size();
    FT_ASSERT(game.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(game.is_assault_active(PLANET_TERRA));
    double initial_raider_shield = game.get_assault_raider_shield(PLANET_TERRA);
    FT_ASSERT(initial_raider_shield > 79.9 && initial_raider_shield < 80.1);
    FT_ASSERT(game.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(game.assign_fleet_to_assault(PLANET_TERRA, 6));
    FT_ASSERT(!game.assign_fleet_to_assault(PLANET_TERRA, 99));
    FT_ASSERT(game.set_assault_support(PLANET_TERRA, true, true, true));
    game.tick(2.0);
    double shield_after_tick = game.get_assault_raider_shield(PLANET_TERRA);
    FT_ASSERT(shield_after_tick < initial_raider_shield);
    FT_ASSERT(game.get_ship_hp(2, ship_a) > 0);
    for (int step = 0; step < 12 && game.is_assault_active(PLANET_TERRA); ++step)
        game.tick(2.0);
    FT_ASSERT(!game.is_assault_active(PLANET_TERRA));
    double final_raider_hull = game.get_assault_raider_hull(PLANET_TERRA);
    FT_ASSERT(final_raider_hull >= -0.1 && final_raider_hull < 0.1);
    size_t lore_after = game.get_lore_log().size();
    FT_ASSERT(lore_after >= lore_before + 2);
    int terra_iron = game.get_ore(PLANET_TERRA, ORE_IRON);
    FT_ASSERT(terra_iron >= 9);

    game.set_ore(PLANET_MARS, ORE_IRON, 4);
    size_t lore_mid = game.get_lore_log().size();
    FT_ASSERT(game.start_raider_assault(PLANET_MARS, 0.8));
    FT_ASSERT(game.is_assault_active(PLANET_MARS));
    game.tick(1.0);
    FT_ASSERT(!game.is_assault_active(PLANET_MARS));
    FT_ASSERT(game.get_assault_raider_shield(PLANET_MARS) < 0.01);
    int mars_iron = game.get_ore(PLANET_MARS, ORE_IRON);
    FT_ASSERT(mars_iron <= 2);
    size_t lore_end = game.get_lore_log().size();
    FT_ASSERT(lore_end >= lore_mid + 1);
    FT_ASSERT(!game.set_assault_support(PLANET_MARS, true, false, false));
    return 1;
}

int validate_order_branch_storyline()
{
    Game order_game(ft_string("127.0.0.1:8080"), ft_string("/"));

    order_game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    order_game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    order_game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_DEFENSE_OF_TERRA, order_game.get_active_quest());

    order_game.create_fleet(10);
    int capital = order_game.create_ship(10, SHIP_CAPITAL);
    FT_ASSERT(capital != 0);
    order_game.set_ship_hp(10, capital, 160);
    order_game.set_ship_shield(10, capital, 60);
    order_game.set_ship_armor(10, capital, 40);
    order_game.create_fleet(11);
    int escort = order_game.create_ship(11, SHIP_SHIELD);
    FT_ASSERT(escort != 0);
    order_game.set_ship_hp(11, escort, 100);
    order_game.set_ship_shield(11, escort, 50);
    order_game.set_ship_armor(11, escort, 30);
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_INVESTIGATE_RAIDERS, order_game.get_active_quest());

    order_game.set_ore(PLANET_TERRA, ORE_IRON, 120);
    order_game.set_ore(PLANET_TERRA, ORE_COPPER, 80);
    order_game.set_ore(PLANET_TERRA, ORE_COAL, 24);
    FT_ASSERT(order_game.start_research(RESEARCH_UNLOCK_MARS));
    order_game.tick(40.0);
    order_game.set_ore(PLANET_TERRA, ORE_COAL, 24);
    order_game.set_ore(PLANET_TERRA, ORE_MITHRIL, 12);
    FT_ASSERT(order_game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    order_game.tick(50.0);
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, order_game.get_active_quest());

    order_game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    order_game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
    order_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
    order_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 220);
    for (int shipment = 0; shipment < 8; ++shipment)
    {
        int moved = order_game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 20);
        FT_ASSERT(moved >= 20);
        double waited = 0.0;
        while (order_game.get_active_convoy_count() > 0 && waited < 240.0)
        {
            order_game.tick(4.0);
            waited += 4.0;
        }
        FT_ASSERT(waited < 240.0);
    }
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, order_game.get_quest_status(QUEST_HIGH_VALUE_ESCORT));
    FT_ASSERT_EQ(QUEST_CLIMACTIC_BATTLE, order_game.get_active_quest());

    order_game.set_ore(PLANET_ZALTHOR, ORE_GOLD, 7);
    order_game.set_ore(PLANET_MARS, ORE_MITHRIL, 12);
    FT_ASSERT(order_game.start_research(RESEARCH_UNLOCK_VULCAN));
    order_game.tick(65.0);
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, order_game.get_quest_status(QUEST_CLIMACTIC_BATTLE));
    FT_ASSERT_EQ(QUEST_CRITICAL_DECISION, order_game.get_active_quest());

    FT_ASSERT(order_game.resolve_quest_choice(QUEST_CRITICAL_DECISION, QUEST_CHOICE_EXECUTE_BLACKTHORNE));
    FT_ASSERT_EQ(QUEST_CHOICE_EXECUTE_BLACKTHORNE, order_game.get_quest_choice(QUEST_CRITICAL_DECISION));
    FT_ASSERT_EQ(QUEST_ORDER_UPRISING, order_game.get_active_quest());

    order_game.set_ore(PLANET_TERRA, ORE_COAL, 24);
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, order_game.get_quest_status(QUEST_ORDER_UPRISING));
    FT_ASSERT_EQ(QUEST_ORDER_SUPPRESS_RAIDS, order_game.get_active_quest());

    order_game.ensure_planet_item_slot(PLANET_TERRA, ITEM_COPPER_BAR);
    order_game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
    order_game.ensure_planet_item_slot(PLANET_TERRA, ITEM_MITHRIL_BAR);
    order_game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 30);
    order_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 60);
    order_game.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 20);
    FT_ASSERT(order_game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    order_game.tick(45.0);
    int radar = order_game.place_building(PLANET_TERRA, BUILDING_PROXIMITY_RADAR, 0, 1);
    FT_ASSERT(radar != 0);
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, order_game.get_quest_status(QUEST_ORDER_SUPPRESS_RAIDS));
    FT_ASSERT_EQ(QUEST_ORDER_DOMINION, order_game.get_active_quest());
    FT_ASSERT(order_game.is_assault_active(PLANET_MARS));

    FT_ASSERT(order_game.assign_fleet_to_assault(PLANET_MARS, 10));
    FT_ASSERT(order_game.assign_fleet_to_assault(PLANET_MARS, 11));
    double assault_timer = 0.0;
    while (order_game.is_assault_active(PLANET_MARS) && assault_timer < 200.0)
    {
        order_game.tick(2.0);
        assault_timer += 2.0;
    }
    FT_ASSERT(!order_game.is_assault_active(PLANET_MARS));
    order_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, order_game.get_quest_status(QUEST_ORDER_DOMINION));
    FT_ASSERT_EQ(0, order_game.get_active_quest());

    return 1;
}

int evaluate_building_and_convoy_systems(Game &game)
{
    int terra_mine_instance = game.get_building_instance(PLANET_TERRA, 0, 0);
    FT_ASSERT(terra_mine_instance != 0);
    FT_ASSERT(!game.remove_building(PLANET_TERRA, terra_mine_instance));
    FT_ASSERT_EQ(16, game.get_planet_plot_capacity(PLANET_TERRA));
    FT_ASSERT_EQ(1, game.get_planet_plot_usage(PLANET_TERRA));
    FT_ASSERT_EQ(1, game.get_building_count(PLANET_TERRA, BUILDING_MINE_CORE));
    FT_ASSERT(!game.can_place_building(PLANET_TERRA, BUILDING_SMELTER, 0, 0));

    game.set_ore(PLANET_TERRA, ORE_IRON, 200);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    game.set_ore(PLANET_TERRA, ORE_COAL, 200);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 0);
    game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 0);

    FT_ASSERT(!game.can_place_building(PLANET_TERRA, BUILDING_SOLAR_ARRAY, 0, 1));
    FT_ASSERT(game.can_place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0));
    int terra_generator = game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0);
    FT_ASSERT(terra_generator != 0);
    FT_ASSERT(game.place_building(PLANET_TERRA, BUILDING_CONVEYOR, 1, 0) != 0);
    FT_ASSERT(game.place_building(PLANET_TERRA, BUILDING_TRANSFER_NODE, 1, 1) != 0);
    FT_ASSERT(game.place_building(PLANET_TERRA, BUILDING_SMELTER, 0, 2) != 0);
    FT_ASSERT(game.place_building(PLANET_TERRA, BUILDING_PROCESSOR, 2, 2) != 0);

    FT_ASSERT_EQ(4, game.get_planet_logistic_capacity(PLANET_TERRA));
    double terra_energy_gen = game.get_planet_energy_generation(PLANET_TERRA);
    FT_ASSERT(terra_energy_gen > 5.9 && terra_energy_gen < 6.1);
    game.tick(10.0);
    FT_ASSERT(game.get_planet_logistic_usage(PLANET_TERRA) >= 2);
    double terra_energy_use = game.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(terra_energy_use > 4.49 && terra_energy_use < 4.51);
    FT_ASSERT(game.get_ore(PLANET_TERRA, ITEM_IRON_BAR) >= 4);
    FT_ASSERT(game.get_ore(PLANET_TERRA, ITEM_COPPER_BAR) >= 2);

    int conveyor_instance = game.get_building_instance(PLANET_TERRA, 1, 0);
    FT_ASSERT(conveyor_instance != 0);
    FT_ASSERT(game.remove_building(PLANET_TERRA, conveyor_instance));
    FT_ASSERT_EQ(3, game.get_planet_logistic_capacity(PLANET_TERRA));

    game.set_ore(PLANET_VULCAN, ORE_IRON, 150);
    game.set_ore(PLANET_VULCAN, ORE_COPPER, 150);
    game.set_ore(PLANET_VULCAN, ORE_COAL, 150);
    game.set_ore(PLANET_VULCAN, ITEM_IRON_BAR, 0);
    game.set_ore(PLANET_VULCAN, ITEM_COPPER_BAR, 0);
    game.set_ore(PLANET_VULCAN, ITEM_ENGINE_PART, 0);

    FT_ASSERT(game.place_building(PLANET_VULCAN, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    FT_ASSERT(game.place_building(PLANET_VULCAN, BUILDING_CONVEYOR, 1, 0) != 0);
    FT_ASSERT(game.place_building(PLANET_VULCAN, BUILDING_TRANSFER_NODE, 1, 1) != 0);
    FT_ASSERT(game.place_building(PLANET_VULCAN, BUILDING_CRAFTING_BAY, 0, 2) != 0);
    FT_ASSERT_EQ(4, game.get_planet_logistic_capacity(PLANET_VULCAN));

    int moved_iron = game.transfer_ore(PLANET_TERRA, PLANET_VULCAN, ITEM_IRON_BAR, 4);
    int moved_copper = game.transfer_ore(PLANET_TERRA, PLANET_VULCAN, ITEM_COPPER_BAR, 2);
    FT_ASSERT(moved_iron >= 4);
    FT_ASSERT(moved_copper >= 2);
    FT_ASSERT(game.get_active_convoy_count() >= 2);
    FT_ASSERT_EQ(0, game.get_ore(PLANET_VULCAN, ITEM_IRON_BAR));
    FT_ASSERT_EQ(0, game.get_ore(PLANET_VULCAN, ITEM_COPPER_BAR));

    double vulcan_energy = game.get_planet_energy_generation(PLANET_VULCAN);
    FT_ASSERT(vulcan_energy > 5.9 && vulcan_energy < 6.1);
    game.tick(30.0);
    FT_ASSERT(game.get_active_convoy_count() >= 1);
    FT_ASSERT_EQ(0, game.get_ore(PLANET_VULCAN, ITEM_IRON_BAR));
    game.tick(20.0);
    FT_ASSERT_EQ(0, game.get_active_convoy_count());
    FT_ASSERT(game.get_ore(PLANET_VULCAN, ITEM_IRON_BAR) >= 4);
    FT_ASSERT(game.get_ore(PLANET_VULCAN, ITEM_COPPER_BAR) >= 2);
    game.tick(16.0);
    FT_ASSERT(game.get_planet_logistic_usage(PLANET_VULCAN) >= 2);
    double vulcan_use = game.get_planet_energy_consumption(PLANET_VULCAN);
    FT_ASSERT(vulcan_use > 3.9 && vulcan_use < 4.1);
    FT_ASSERT(game.get_ore(PLANET_VULCAN, ITEM_ENGINE_PART) >= 2);
    FT_ASSERT(game.get_planet_logistic_usage(PLANET_TERRA) >= 2);
    double terra_use_after = game.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(terra_use_after > 4.49 && terra_use_after < 4.51);
    double terra_mine_bonus = game.get_planet_mine_multiplier(PLANET_TERRA);
    FT_ASSERT(terra_mine_bonus > 0.99 && terra_mine_bonus < 1.01);

    int terra_capacity_before = game.get_planet_logistic_capacity(PLANET_TERRA);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 80);
    game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 20);
    FT_ASSERT(game.can_start_research(RESEARCH_URBAN_PLANNING_TERRA));
    FT_ASSERT(game.start_research(RESEARCH_URBAN_PLANNING_TERRA));
    double terra_planning_remaining = game.get_research_time_remaining(RESEARCH_URBAN_PLANNING_TERRA);
    FT_ASSERT(terra_planning_remaining > 19.9 && terra_planning_remaining < 20.1);
    game.tick(20.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_URBAN_PLANNING_TERRA));
    FT_ASSERT(game.get_planet_logistic_capacity(PLANET_TERRA) >= terra_capacity_before + 4);

    int mars_capacity_before = game.get_planet_logistic_capacity(PLANET_MARS);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 160);
    game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 40);
    FT_ASSERT(game.can_start_research(RESEARCH_URBAN_PLANNING_MARS));
    FT_ASSERT(game.start_research(RESEARCH_URBAN_PLANNING_MARS));
    double mars_planning_remaining = game.get_research_time_remaining(RESEARCH_URBAN_PLANNING_MARS);
    FT_ASSERT(mars_planning_remaining > 24.9 && mars_planning_remaining < 25.1);
    game.tick(25.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_URBAN_PLANNING_MARS));
    FT_ASSERT(game.get_planet_logistic_capacity(PLANET_MARS) >= mars_capacity_before + 4);

    int zalthor_capacity_before = game.get_planet_logistic_capacity(PLANET_ZALTHOR);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 240);
    game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 80);
    FT_ASSERT(game.can_start_research(RESEARCH_URBAN_PLANNING_ZALTHOR));
    FT_ASSERT(game.start_research(RESEARCH_URBAN_PLANNING_ZALTHOR));
    double zalthor_planning_remaining = game.get_research_time_remaining(RESEARCH_URBAN_PLANNING_ZALTHOR);
    FT_ASSERT(zalthor_planning_remaining > 29.9 && zalthor_planning_remaining < 30.1);
    game.tick(30.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_URBAN_PLANNING_ZALTHOR));
    FT_ASSERT(game.get_planet_logistic_capacity(PLANET_ZALTHOR) >= zalthor_capacity_before + 4);

    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_SOLAR_PANELS));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_CRAFTING_MASTERY));
    game.set_ore(PLANET_TERRA, ORE_IRON, 240);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 240);
    game.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 6);
    FT_ASSERT(game.can_start_research(RESEARCH_SOLAR_PANELS));
    FT_ASSERT(game.start_research(RESEARCH_SOLAR_PANELS));
    double solar_remaining = game.get_research_time_remaining(RESEARCH_SOLAR_PANELS);
    FT_ASSERT(solar_remaining > 24.9 && solar_remaining < 25.1);
    game.tick(25.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_SOLAR_PANELS));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_CRAFTING_MASTERY));
    FT_ASSERT(game.can_place_building(PLANET_TERRA, BUILDING_SOLAR_ARRAY, 0, 1));
    int solar_instance = game.place_building(PLANET_TERRA, BUILDING_SOLAR_ARRAY, 0, 1);
    FT_ASSERT(solar_instance != 0);
    double terra_energy_with_solar = game.get_planet_energy_generation(PLANET_TERRA);
    FT_ASSERT(terra_energy_with_solar > 8.9 && terra_energy_with_solar < 9.1);

    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 260);
    game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 260);
    game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 6);
    FT_ASSERT(game.can_start_research(RESEARCH_CRAFTING_MASTERY));
    FT_ASSERT(game.start_research(RESEARCH_CRAFTING_MASTERY));
    double mastery_remaining = game.get_research_time_remaining(RESEARCH_CRAFTING_MASTERY);
    FT_ASSERT(mastery_remaining > 34.9 && mastery_remaining < 35.1);
    game.tick(35.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_CRAFTING_MASTERY));
    game.tick(10.0);
    double terra_energy_mastery = game.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(terra_energy_mastery > 3.5 && terra_energy_mastery < 3.7);
    FT_ASSERT(terra_energy_mastery < terra_use_after - 0.7);
    FT_ASSERT(game.get_planet_logistic_usage(PLANET_TERRA) >= 2);
    return 1;
}

int evaluate_ship_upgrade_research(Game &game)
{
    FT_ASSERT(game.get_ship_hull_multiplier() > 0.99 && game.get_ship_hull_multiplier() < 1.01);
    FT_ASSERT(game.get_ship_shield_multiplier() > 0.99 && game.get_ship_shield_multiplier() < 1.01);
    FT_ASSERT(game.get_ship_weapon_multiplier() > 0.99 && game.get_ship_weapon_multiplier() < 1.01);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_I));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_II));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_I));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_II));

    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
    game.ensure_planet_item_slot(PLANET_TERRA, ORE_COAL);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 220);
    game.set_ore(PLANET_TERRA, ORE_COAL, 220);
    FT_ASSERT(game.can_start_research(RESEARCH_STRUCTURAL_REINFORCEMENT_I));
    FT_ASSERT(game.start_research(RESEARCH_STRUCTURAL_REINFORCEMENT_I));
    double structural_one = game.get_research_time_remaining(RESEARCH_STRUCTURAL_REINFORCEMENT_I);
    FT_ASSERT(structural_one > 24.9 && structural_one < 25.1);
    game.tick(25.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_I));
    FT_ASSERT(game.get_ship_hull_multiplier() > 1.09 && game.get_ship_hull_multiplier() < 1.11);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_II));
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 260);
    game.set_ore(PLANET_TERRA, ORE_COAL, 260);
    FT_ASSERT(game.can_start_research(RESEARCH_STRUCTURAL_REINFORCEMENT_II));
    FT_ASSERT(game.start_research(RESEARCH_STRUCTURAL_REINFORCEMENT_II));
    double structural_two = game.get_research_time_remaining(RESEARCH_STRUCTURAL_REINFORCEMENT_II);
    FT_ASSERT(structural_two > 34.9 && structural_two < 35.1);
    game.tick(35.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_II));
    FT_ASSERT(game.get_ship_hull_multiplier() > 1.19 && game.get_ship_hull_multiplier() < 1.21);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_III));
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 320);
    game.set_ore(PLANET_TERRA, ORE_COAL, 320);
    FT_ASSERT(game.can_start_research(RESEARCH_STRUCTURAL_REINFORCEMENT_III));
    FT_ASSERT(game.start_research(RESEARCH_STRUCTURAL_REINFORCEMENT_III));
    double structural_three = game.get_research_time_remaining(RESEARCH_STRUCTURAL_REINFORCEMENT_III);
    FT_ASSERT(structural_three > 44.9 && structural_three < 45.1);
    game.tick(45.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_III));
    FT_ASSERT(game.get_ship_hull_multiplier() > 1.29 && game.get_ship_hull_multiplier() < 1.31);

    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_COPPER_BAR);
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_MITHRIL_BAR);
    game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 220);
    game.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 220);
    FT_ASSERT(game.can_start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    FT_ASSERT(game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    double defensive_one = game.get_research_time_remaining(RESEARCH_DEFENSIVE_FORTIFICATION_I);
    FT_ASSERT(defensive_one > 29.9 && defensive_one < 30.1);
    game.tick(30.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    FT_ASSERT(game.get_ship_shield_multiplier() > 1.09 && game.get_ship_shield_multiplier() < 1.11);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 260);
    game.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 260);
    FT_ASSERT(game.can_start_research(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    FT_ASSERT(game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    double defensive_two = game.get_research_time_remaining(RESEARCH_DEFENSIVE_FORTIFICATION_II);
    FT_ASSERT(defensive_two > 39.9 && defensive_two < 40.1);
    game.tick(40.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    FT_ASSERT(game.get_ship_shield_multiplier() > 1.19 && game.get_ship_shield_multiplier() < 1.21);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_III));
    game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 320);
    game.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 320);
    FT_ASSERT(game.can_start_research(RESEARCH_DEFENSIVE_FORTIFICATION_III));
    FT_ASSERT(game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_III));
    double defensive_three = game.get_research_time_remaining(RESEARCH_DEFENSIVE_FORTIFICATION_III);
    FT_ASSERT(defensive_three > 49.9 && defensive_three < 50.1);
    game.tick(50.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_III));
    FT_ASSERT(game.get_ship_shield_multiplier() > 1.29 && game.get_ship_shield_multiplier() < 1.31);

    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART);
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_TITANIUM_BAR);
    game.ensure_planet_item_slot(PLANET_TERRA, ORE_TRITIUM);
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_FUSION_REACTOR);
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 220);
    game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 220);
    game.set_ore(PLANET_TERRA, ORE_TRITIUM, 15);
    game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 6);
    FT_ASSERT(game.can_start_research(RESEARCH_ARMAMENT_ENHANCEMENT_I));
    FT_ASSERT(game.start_research(RESEARCH_ARMAMENT_ENHANCEMENT_I));
    double armament_one = game.get_research_time_remaining(RESEARCH_ARMAMENT_ENHANCEMENT_I);
    FT_ASSERT(armament_one > 34.9 && armament_one < 35.1);
    game.tick(35.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_I));
    FT_ASSERT(game.get_ship_weapon_multiplier() > 1.09 && game.get_ship_weapon_multiplier() < 1.11);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_II));
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 260);
    game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 260);
    game.set_ore(PLANET_TERRA, ORE_TRITIUM, 20);
    FT_ASSERT(game.can_start_research(RESEARCH_ARMAMENT_ENHANCEMENT_II));
    FT_ASSERT(game.start_research(RESEARCH_ARMAMENT_ENHANCEMENT_II));
    double armament_two = game.get_research_time_remaining(RESEARCH_ARMAMENT_ENHANCEMENT_II);
    FT_ASSERT(armament_two > 44.9 && armament_two < 45.1);
    game.tick(45.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_II));
    FT_ASSERT(game.get_ship_weapon_multiplier() > 1.19 && game.get_ship_weapon_multiplier() < 1.21);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_III));
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 320);
    game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 320);
    game.set_ore(PLANET_TERRA, ORE_TRITIUM, 30);
    FT_ASSERT(game.can_start_research(RESEARCH_ARMAMENT_ENHANCEMENT_III));
    FT_ASSERT(game.start_research(RESEARCH_ARMAMENT_ENHANCEMENT_III));
    double armament_three = game.get_research_time_remaining(RESEARCH_ARMAMENT_ENHANCEMENT_III);
    FT_ASSERT(armament_three > 54.9 && armament_three < 55.1);
    game.tick(55.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_III));
    FT_ASSERT(game.get_ship_weapon_multiplier() > 1.29 && game.get_ship_weapon_multiplier() < 1.31);
    return 1;
}

int verify_supply_contract_automation()
{
    Game contract_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    contract_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 60);
    contract_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 10);

    int invalid_contract = contract_game.create_supply_contract(PLANET_TERRA, PLANET_TERRA,
                                                                ITEM_IRON_BAR, 20, 30.0);
    FT_ASSERT_EQ(0, invalid_contract);

    int contract_id = contract_game.create_supply_contract(PLANET_TERRA, PLANET_MARS,
                                                           ITEM_IRON_BAR, 30, 45.0, 25, 2);
    FT_ASSERT(contract_id > 0);

    ft_vector<int> contract_ids;
    contract_game.get_supply_contract_ids(contract_ids);
    bool found = false;
    for (size_t idx = 0; idx < contract_ids.size(); ++idx)
    {
        if (contract_ids[idx] == contract_id)
        {
            found = true;
            break;
        }
    }
    FT_ASSERT(found);

    Game::ft_supply_contract contract_details;
    FT_ASSERT(contract_game.get_supply_contract(contract_id, contract_details));
    FT_ASSERT(contract_details.has_minimum_stock);
    FT_ASSERT_EQ(25, contract_details.minimum_stock);
    FT_ASSERT_EQ(30, contract_details.shipment_size);
    FT_ASSERT_EQ(2, contract_details.max_active_convoys);

    contract_game.tick(30.0);
    FT_ASSERT_EQ(0, contract_game.get_active_convoy_count());

    contract_game.tick(20.0);
    FT_ASSERT(contract_game.get_active_convoy_count() >= 1);

    contract_game.tick(120.0);
    FT_ASSERT_EQ(0, contract_game.get_active_convoy_count());
    FT_ASSERT(contract_game.get_ore(PLANET_MARS, ITEM_IRON_BAR) >= 70);

    FT_ASSERT(contract_game.update_supply_contract(contract_id, 18, 30.0, -1, 3));
    FT_ASSERT(contract_game.get_supply_contract(contract_id, contract_details));
    FT_ASSERT(!contract_details.has_minimum_stock);
    FT_ASSERT_EQ(18, contract_details.shipment_size);
    FT_ASSERT_EQ(3, contract_details.max_active_convoys);

    contract_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 5);
    contract_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 12);

    contract_game.tick(30.0);
    FT_ASSERT(contract_game.get_active_convoy_count() >= 1);

    contract_game.tick(120.0);
    FT_ASSERT(contract_game.get_ore(PLANET_MARS, ITEM_IRON_BAR) >= 17);

    contract_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 90);
    contract_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);

    contract_game.tick(90.0);
    FT_ASSERT_EQ(0, contract_game.get_active_convoy_count());
    FT_ASSERT(contract_game.get_ore(PLANET_MARS, ITEM_IRON_BAR) >= 54);

    FT_ASSERT(contract_game.cancel_supply_contract(contract_id));
    contract_game.get_supply_contract_ids(contract_ids);
    bool still_present = false;
    for (size_t idx = 0; idx < contract_ids.size(); ++idx)
    {
        if (contract_ids[idx] == contract_id)
        {
            still_present = true;
            break;
        }
    }
    FT_ASSERT(!still_present);
    FT_ASSERT(!contract_game.get_supply_contract(contract_id, contract_details));

    contract_game.tick(90.0);
    FT_ASSERT_EQ(0, contract_game.get_active_convoy_count());
    FT_ASSERT(!contract_game.cancel_supply_contract(contract_id));
    return 1;
}

int verify_supply_contract_pending_stock_buffer()
{
    Game contract_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    contract_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 180);
    contract_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);

    int contract_id = contract_game.create_supply_contract(PLANET_TERRA, PLANET_MARS,
                                                           ITEM_IRON_BAR, 30, 5.0, 30, 5);
    FT_ASSERT(contract_id > 0);

    contract_game.tick(5.0);
    FT_ASSERT_EQ(1, contract_game.get_active_convoy_count());

    Game::ft_supply_contract details;
    FT_ASSERT(contract_game.get_supply_contract(contract_id, details));
    FT_ASSERT_EQ(30, details.pending_delivery);

    contract_game.tick(5.0);
    FT_ASSERT_EQ(1, contract_game.get_active_convoy_count());
    FT_ASSERT(contract_game.get_supply_contract(contract_id, details));
    FT_ASSERT_EQ(30, details.pending_delivery);

    contract_game.tick(5.0);
    FT_ASSERT_EQ(1, contract_game.get_active_convoy_count());

    contract_game.tick(60.0);
    FT_ASSERT_EQ(0, contract_game.get_active_convoy_count());
    FT_ASSERT(contract_game.get_supply_contract(contract_id, details));
    FT_ASSERT_EQ(0, details.pending_delivery);
    FT_ASSERT(contract_game.get_ore(PLANET_MARS, ITEM_IRON_BAR) >= 30);

    contract_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 5);
    contract_game.tick(5.0);
    FT_ASSERT(contract_game.get_active_convoy_count() >= 1);
    FT_ASSERT(contract_game.get_supply_contract(contract_id, details));
    FT_ASSERT(details.pending_delivery >= 30);

    return 1;
}

int verify_convoy_quest_objectives()
{
    Game success_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(fast_forward_to_supply_quests(success_game));
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, success_game.get_active_quest());

    success_game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    success_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
    success_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 400);
    FT_ASSERT(success_game.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(success_game.start_raider_assault(PLANET_MARS, 1.0));
    int risky = success_game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 40);
    FT_ASSERT(risky >= 40);
    double elapsed = 0.0;
    while (success_game.get_active_convoy_count() > 0 && elapsed < 400.0)
    {
        success_game.tick(5.0);
        elapsed += 5.0;
    }
    FT_ASSERT(elapsed < 400.0);
    while ((success_game.is_assault_active(PLANET_TERRA) || success_game.is_assault_active(PLANET_MARS)) && elapsed < 560.0)
    {
        success_game.tick(5.0);
        elapsed += 5.0;
    }
    FT_ASSERT(success_game.get_convoy_raid_losses() >= 1);
    FT_ASSERT(!success_game.is_assault_active(PLANET_TERRA));
    FT_ASSERT(!success_game.is_assault_active(PLANET_MARS));

    for (int convoy = 0; convoy < 8; ++convoy)
    {
        int moved = success_game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 20);
        FT_ASSERT(moved >= 20);
        double wait = 0.0;
        while (success_game.get_active_convoy_count() > 0 && wait < 240.0)
        {
            success_game.tick(4.0);
            wait += 4.0;
        }
        FT_ASSERT(wait < 240.0);
    }
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, success_game.get_quest_status(QUEST_SECURE_SUPPLY_LINES));
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, success_game.get_quest_status(QUEST_STEADY_SUPPLY_STREAK));
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, success_game.get_quest_status(QUEST_HIGH_VALUE_ESCORT));
    FT_ASSERT_EQ(QUEST_CLIMACTIC_BATTLE, success_game.get_active_quest());
    FT_ASSERT(success_game.get_convoy_raid_losses() >= 1);

    Game failure_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(fast_forward_to_supply_quests(failure_game));
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, failure_game.get_active_quest());
    failure_game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    failure_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
    failure_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 400);
    for (int loss = 0; loss < 2; ++loss)
    {
        FT_ASSERT(failure_game.start_raider_assault(PLANET_TERRA, 1.0));
        FT_ASSERT(failure_game.start_raider_assault(PLANET_MARS, 1.0));
        int moved = failure_game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 30);
        FT_ASSERT(moved >= 30);
        double wait = 0.0;
        while (failure_game.get_active_convoy_count() > 0 && wait < 400.0)
        {
            failure_game.tick(5.0);
            wait += 5.0;
        }
        FT_ASSERT(wait < 400.0);
        while ((failure_game.is_assault_active(PLANET_TERRA) || failure_game.is_assault_active(PLANET_MARS)) && wait < 560.0)
        {
            failure_game.tick(5.0);
            wait += 5.0;
        }
    }
    FT_ASSERT(failure_game.get_convoy_raid_losses() >= 2);
    FT_ASSERT(!failure_game.is_assault_active(PLANET_TERRA));
    FT_ASSERT(!failure_game.is_assault_active(PLANET_MARS));

    for (int convoy = 0; convoy < 3; ++convoy)
    {
        int moved = failure_game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 20);
        FT_ASSERT(moved >= 20);
        double wait = 0.0;
        while (failure_game.get_active_convoy_count() > 0 && wait < 240.0)
        {
            failure_game.tick(4.0);
            wait += 4.0;
        }
        FT_ASSERT(wait < 240.0);
    }
    FT_ASSERT(failure_game.get_total_convoys_delivered() >= 3);
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, failure_game.get_quest_status(QUEST_SECURE_SUPPLY_LINES));
    FT_ASSERT_EQ(QUEST_SECURE_SUPPLY_LINES, failure_game.get_active_quest());
    return 1;
}

int verify_multiple_convoy_raids()
{
    Game convoy_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    convoy_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    convoy_game.ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);
    convoy_game.set_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 0);
    size_t lore_before = convoy_game.get_lore_log().size();
    int transferred = convoy_game.transfer_ore(PLANET_TERRA, PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 120);
    FT_ASSERT(transferred >= 120);
    FT_ASSERT_EQ(1, convoy_game.get_active_convoy_count());

    convoy_game.tick(300.0);

    double threat_after_raids = convoy_game.get_supply_route_threat_level(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
    FT_ASSERT(threat_after_raids > 0.1);

    const ft_vector<ft_string> &log = convoy_game.get_lore_log();
    size_t raid_entries = 0;
    bool destroyed_recorded = false;
    for (size_t idx = lore_before; idx < log.size(); ++idx)
    {
        const char *entry = log[idx].c_str();
        if (ft_strstr(entry, "Raiders ambushed") != ft_nullptr)
            raid_entries += 1;
        if (ft_strstr(entry, "wiped out") != ft_nullptr || ft_strstr(entry, "failed to arrive") != ft_nullptr)
            destroyed_recorded = true;
    }
    FT_ASSERT(raid_entries >= 2 || destroyed_recorded);
    int deliveries_before = convoy_game.get_total_convoys_delivered();
    for (int convoy = 0; convoy < 3; ++convoy)
    {
        int moved_again = convoy_game.transfer_ore(PLANET_TERRA, PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 20);
        FT_ASSERT(moved_again >= 20);
        double wait = 0.0;
        while (convoy_game.get_active_convoy_count() > 0 && wait < 360.0)
        {
            convoy_game.tick(4.0);
            wait += 4.0;
        }
        FT_ASSERT(wait < 360.0);
    }
    FT_ASSERT(convoy_game.get_total_convoys_delivered() > deliveries_before);
    double threat_after_deliveries = convoy_game.get_supply_route_threat_level(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
    FT_ASSERT(threat_after_deliveries < threat_after_raids);
    return 1;
}

int verify_supply_route_escalation()
{
    Game escalation_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    escalation_game.ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);
    escalation_game.set_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 0);
    escalation_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 400);

    double threat_level = 0.0;
    int convoy_attempts = 0;
    while (threat_level < 5.0 && convoy_attempts < 12)
    {
        int moved = escalation_game.transfer_ore(PLANET_TERRA, PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 40);
        FT_ASSERT(moved > 0);
        double waited = 0.0;
        while (escalation_game.get_active_convoy_count() > 0 && waited < 480.0)
        {
            escalation_game.tick(6.0);
            waited += 6.0;
        }
        FT_ASSERT(waited < 480.0);
        threat_level = escalation_game.get_supply_route_threat_level(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
        convoy_attempts += 1;
    }
    FT_ASSERT(threat_level >= 5.0);

    size_t lore_before = escalation_game.get_lore_log().size();
    bool assault_started = false;
    double accumulated = 0.0;
    while (!assault_started && accumulated < 240.0)
    {
        escalation_game.tick(6.0);
        accumulated += 6.0;
        if (escalation_game.is_assault_active(PLANET_TERRA) ||
            escalation_game.is_assault_active(PLANET_NOCTARIS_PRIME))
        {
            assault_started = true;
        }
    }
    FT_ASSERT(assault_started);
    bool terra_under_attack = escalation_game.is_assault_active(PLANET_TERRA);
    bool noctaris_under_attack = escalation_game.is_assault_active(PLANET_NOCTARIS_PRIME);
    FT_ASSERT(terra_under_attack || noctaris_under_attack);

    const ft_vector<ft_string> &log = escalation_game.get_lore_log();
    bool escalation_logged = false;
    for (size_t idx = lore_before; idx < log.size(); ++idx)
    {
        const char *entry = log[idx].c_str();
        if (ft_strstr(entry, "convoy route") != ft_nullptr && ft_strstr(entry, "assault") != ft_nullptr)
        {
            escalation_logged = true;
            break;
        }
    }
    FT_ASSERT(escalation_logged);

    if (terra_under_attack)
    {
        escalation_game.tick(12.0);
        FT_ASSERT(escalation_game.is_assault_active(PLANET_TERRA));
    }
    return 1;
}

int verify_escort_veterancy_progression()
{
    Game veterancy_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(fast_forward_to_supply_quests(veterancy_game));
    while (veterancy_game.get_active_convoy_count() > 0)
    {
        veterancy_game.tick(2.0);
    }
    const int escort_fleet_id = 9301;
    veterancy_game.create_fleet(escort_fleet_id);
    int shield_uid = veterancy_game.create_ship(escort_fleet_id, SHIP_SHIELD);
    int radar_uid = veterancy_game.create_ship(escort_fleet_id, SHIP_RADAR);
    FT_ASSERT(shield_uid != 0);
    FT_ASSERT(radar_uid != 0);
    veterancy_game.set_fleet_location_planet(escort_fleet_id, PLANET_TERRA);

    veterancy_game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
    veterancy_game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    veterancy_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    veterancy_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);

    int assigned = veterancy_game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS);
    if (assigned != 0)
        FT_ASSERT(veterancy_game.clear_convoy_escort(PLANET_TERRA, PLANET_MARS));

    double travel_times[3] = {0.0, 0.0, 0.0};
    int bonuses[3] = {0, 0, 0};
    for (int run = 0; run < 3; ++run)
    {
        veterancy_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
        veterancy_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
        FT_ASSERT(veterancy_game.assign_convoy_escort(PLANET_TERRA, PLANET_MARS, escort_fleet_id));
        int moved = veterancy_game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 40);
        FT_ASSERT(moved >= 40);
        double elapsed = 0.0;
        while (veterancy_game.get_active_convoy_count() > 0 && elapsed < 400.0)
        {
            veterancy_game.tick(0.5);
            elapsed += 0.5;
        }
        FT_ASSERT(elapsed < 400.0);
        travel_times[run] = elapsed;
        bonuses[run] = veterancy_game.get_fleet_escort_veterancy_bonus(escort_fleet_id);
    }

    FT_ASSERT_EQ(0, bonuses[0]);
    FT_ASSERT_EQ(0, bonuses[1]);
    FT_ASSERT(bonuses[2] >= 1);
    double veterancy_xp = veterancy_game.get_fleet_escort_veterancy(escort_fleet_id);
    FT_ASSERT(veterancy_xp >= 60.0);
    FT_ASSERT(travel_times[2] + 0.99 < travel_times[0]);
    FT_ASSERT(travel_times[2] <= travel_times[1] + 0.5);
    FT_ASSERT_EQ(0, veterancy_game.get_convoy_raid_losses());

    return 1;
}
