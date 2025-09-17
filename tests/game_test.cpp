#include "../libft/Networking/http_server.hpp"
#include "../libft/PThread/thread.hpp"
#include "../libft/Time/time.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/pair.hpp"
#include "game.hpp"
#include "fleets.hpp"
#include "research.hpp"
#include "quests.hpp"
#include "buildings.hpp"
#include <cmath>

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

    BackendClient client(ft_string("127.0.0.1:8080"), ft_string("/"));
    ft_string response;
    client.send_state(ft_string("test"), response);
    const char *resp = response.c_str();
    FT_ASSERT(response.size() >= 4);
    FT_ASSERT_EQ(0, ft_strcmp(resp + response.size() - 4, "test"));

    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
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
    FT_ASSERT_EQ(0, game.get_active_quest());

    game.create_fleet(98);
    FT_ASSERT_EQ(0, game.create_ship(98, SHIP_CAPITAL));
    game.remove_fleet(98);

    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 200);
    game.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 200);
    game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 100);
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 120);
    game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 120);
    game.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 60);
    game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 10);
    game.set_ore(PLANET_TERRA, ORE_COAL, 120);
    game.set_ore(PLANET_NOCTARIS_PRIME, ORE_OBSIDIAN, 80);
    game.set_ore(PLANET_NOCTARIS_PRIME, ORE_CRYSTAL, 80);
    game.set_ore(PLANET_NOCTARIS_PRIME, ORE_TRITIUM, 60);

    FT_ASSERT(game.start_research(RESEARCH_SOLAR_PANELS));
    game.tick(35.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_SOLAR_PANELS));
    FT_ASSERT(game.start_research(RESEARCH_CRAFTING_MASTERY));
    game.tick(45.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_CRAFTING_MASTERY));

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
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 260);
    game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 260);
    game.set_ore(PLANET_TERRA, ORE_TRITIUM, 40);
    game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 6);
    FT_ASSERT(game.start_research(RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT));
    game.tick(60.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT));
    FT_ASSERT(game.start_research(RESEARCH_ESCAPE_POD_LIFELINE));
    game.tick(50.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_ESCAPE_POD_LIFELINE));
    FT_ASSERT(game.start_research(RESEARCH_EMERGENCY_ENERGY_CONSERVATION));
    game.tick(35.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_EMERGENCY_ENERGY_CONSERVATION));
    FT_ASSERT(game.start_research(RESEARCH_FASTER_CRAFTING));
    game.tick(45.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_FASTER_CRAFTING));
    FT_ASSERT(game.start_research(RESEARCH_TRITIUM_EXTRACTION));
    game.tick(55.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_TRITIUM_EXTRACTION));

    Game roster_locked(ft_string("127.0.0.1:8080"), ft_string("/"));
    roster_locked.create_fleet(1);
    FT_ASSERT_EQ(0, roster_locked.create_ship(1, SHIP_TRANSPORT));
    FT_ASSERT_EQ(0, roster_locked.create_ship(1, SHIP_CORVETTE));
    FT_ASSERT_EQ(0, roster_locked.create_ship(1, SHIP_INTERCEPTOR));
    FT_ASSERT_EQ(0, roster_locked.create_ship(1, SHIP_REPAIR_DRONE));
    FT_ASSERT_EQ(0, roster_locked.create_ship(1, SHIP_FRIGATE_ESCORT));
    FT_ASSERT_EQ(0, roster_locked.create_ship(1, SHIP_SUNFLARE_SLOOP));

    game.create_fleet(11);
    int unlocked_transport = game.create_ship(11, SHIP_TRANSPORT);
    FT_ASSERT(unlocked_transport != 0);
    FT_ASSERT_EQ(190, game.get_ship_hp(11, unlocked_transport));
    FT_ASSERT_EQ(60, game.get_ship_shield(11, unlocked_transport));
    int unlocked_corvette = game.create_ship(11, SHIP_CORVETTE);
    FT_ASSERT(unlocked_corvette != 0);
    FT_ASSERT_EQ(220, game.get_ship_hp(11, unlocked_corvette));
    FT_ASSERT_EQ(90, game.get_ship_shield(11, unlocked_corvette));
    int unlocked_interceptor = game.create_ship(11, SHIP_INTERCEPTOR);
    FT_ASSERT(unlocked_interceptor != 0);
    FT_ASSERT_EQ(170, game.get_ship_hp(11, unlocked_interceptor));
    FT_ASSERT_EQ(80, game.get_ship_shield(11, unlocked_interceptor));
    int unlocked_drone = game.create_ship(11, SHIP_REPAIR_DRONE);
    FT_ASSERT(unlocked_drone != 0);
    FT_ASSERT_EQ(110, game.get_ship_hp(11, unlocked_drone));
    FT_ASSERT_EQ(90, game.get_ship_shield(11, unlocked_drone));
    int unlocked_sloop = game.create_ship(11, SHIP_SUNFLARE_SLOOP);
    FT_ASSERT(unlocked_sloop != 0);
    FT_ASSERT_EQ(240, game.get_ship_hp(11, unlocked_sloop));
    FT_ASSERT_EQ(200, game.get_ship_shield(11, unlocked_sloop));
    int unlocked_escort = game.create_ship(11, SHIP_FRIGATE_ESCORT);
    FT_ASSERT(unlocked_escort != 0);
    FT_ASSERT_EQ(280, game.get_ship_hp(11, unlocked_escort));
    FT_ASSERT_EQ(130, game.get_ship_shield(11, unlocked_escort));
    int unlocked_support = game.create_ship(11, SHIP_FRIGATE_SUPPORT);
    FT_ASSERT(unlocked_support != 0);
    FT_ASSERT_EQ(260, game.get_ship_hp(11, unlocked_support));
    FT_ASSERT_EQ(150, game.get_ship_shield(11, unlocked_support));
    int unlocked_battleship = game.create_ship(11, SHIP_CAPITAL);
    FT_ASSERT(unlocked_battleship != 0);
    int unlocked_carrier = game.create_ship(11, SHIP_CAPITAL_CARRIER);
    FT_ASSERT(unlocked_carrier != 0);
    FT_ASSERT_EQ(0, game.create_ship(11, SHIP_CAPITAL_DREADNOUGHT));
    game.remove_fleet(11);

    game.create_fleet(12);
    int support_corvette = game.create_ship(12, SHIP_CORVETTE);
    game.set_ship_hp(12, support_corvette, 150);
    game.set_ship_shield(12, support_corvette, 30);
    int support_sloop = game.create_ship(12, SHIP_SUNFLARE_SLOOP);
    int support_drone = game.create_ship(12, SHIP_REPAIR_DRONE);
    game.set_ship_hp(12, support_drone, 60);
    FT_ASSERT(game.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(game.assign_fleet_to_assault(PLANET_TERRA, 12));
    FT_ASSERT(game.set_assault_support(PLANET_TERRA, true, true, true));
    int initial_support_shield = game.get_ship_shield(12, support_corvette);
    int initial_support_hp = game.get_ship_hp(12, support_corvette);
    game.tick(2.0);
    int post_support_shield = game.get_ship_shield(12, support_corvette);
    int post_support_hp = game.get_ship_hp(12, support_corvette);
    FT_ASSERT(post_support_shield > initial_support_shield);
    FT_ASSERT(post_support_hp > initial_support_hp);
    while (game.is_assault_active(PLANET_TERRA))
        game.tick(2.0);
    game.remove_fleet(12);

    game.create_fleet(97);
    int rescue_ship = game.create_ship(97, SHIP_SHIELD);
    game.set_ship_hp(97, rescue_ship, 15);
    FT_ASSERT_EQ(10, game.sub_ship_hp(97, rescue_ship, 20));
    FT_ASSERT_EQ(10, game.get_ship_hp(97, rescue_ship));
    FT_ASSERT_EQ(0, game.sub_ship_hp(97, rescue_ship, 15));
    game.remove_fleet(97);

    game.set_ore(PLANET_TERRA, ORE_COPPER, 10);
    game.set_ore(PLANET_MARS, ORE_COPPER, 0);
    int moved = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ORE_COPPER, 4);
    FT_ASSERT_EQ(4, moved);
    FT_ASSERT_EQ(6, game.get_ore(PLANET_TERRA, ORE_COPPER));
    FT_ASSERT_EQ(0, game.get_ore(PLANET_MARS, ORE_COPPER));
    FT_ASSERT_EQ(1, game.get_active_convoy_count());
    game.tick(12.0);
    FT_ASSERT_EQ(1, game.get_active_convoy_count());
    FT_ASSERT_EQ(0, game.get_ore(PLANET_MARS, ORE_COPPER));
    game.tick(18.0);
    FT_ASSERT_EQ(0, game.get_active_convoy_count());
    FT_ASSERT_EQ(4, game.get_ore(PLANET_MARS, ORE_COPPER));

    game.set_ore(PLANET_TERRA, ORE_IRON, 3);
    game.set_ore(PLANET_MARS, ORE_IRON, 0);
    int over = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ORE_IRON, 10);
    FT_ASSERT_EQ(3, over);
    FT_ASSERT_EQ(0, game.get_ore(PLANET_TERRA, ORE_IRON));
    FT_ASSERT_EQ(0, game.get_ore(PLANET_MARS, ORE_IRON));
    FT_ASSERT_EQ(1, game.get_active_convoy_count());
    game.tick(30.0);
    FT_ASSERT_EQ(0, game.get_active_convoy_count());
    FT_ASSERT_EQ(3, game.get_ore(PLANET_MARS, ORE_IRON));

    game.set_ore(PLANET_TERRA, ORE_COAL, 9);
    game.set_ore(PLANET_MARS, ORE_COAL, 0);
    FT_ASSERT(game.start_raider_assault(PLANET_MARS, 1.0));
    int contested = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ORE_COAL, 5);
    FT_ASSERT_EQ(5, contested);
    FT_ASSERT_EQ(4, game.get_ore(PLANET_TERRA, ORE_COAL));
    FT_ASSERT_EQ(1, game.get_active_convoy_count());
    game.tick(12.0);
    FT_ASSERT_EQ(0, game.get_active_convoy_count());
    FT_ASSERT(game.get_ore(PLANET_MARS, ORE_COAL) < contested);
    while (game.is_assault_active(PLANET_MARS))
        game.tick(4.0);

    game.create_fleet(1);
    int ship_a = game.create_ship(1, SHIP_SHIELD);
    int ship_b = game.create_ship(1, SHIP_SHIELD);
    FT_ASSERT(ship_a != ship_b);
    game.set_ship_hp(1, ship_a, 100);
    game.set_ship_shield(1, ship_a, 50);
    game.add_ship_armor(1, ship_a, 25);
    FT_ASSERT_EQ(100, game.get_ship_hp(1, ship_a));
    FT_ASSERT_EQ(50, game.get_ship_shield(1, ship_a));
    FT_ASSERT_EQ(25, game.get_ship_armor(1, ship_a));
    FT_ASSERT_EQ(0, game.sub_ship_armor(1, ship_a, 30));
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

    Game energy_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    energy_game.set_ore(PLANET_TERRA, ORE_IRON, 200);
    energy_game.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    energy_game.set_ore(PLANET_TERRA, ORE_COAL, 50);
    FT_ASSERT(energy_game.place_building(PLANET_TERRA, BUILDING_SMELTER, 1, 0) != 0);
    energy_game.tick(0.0);
    double deficit_pressure = energy_game.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(deficit_pressure > 1.9 && deficit_pressure < 2.1);
    energy_game.create_fleet(1);
    int deficit_capital = energy_game.create_ship(1, SHIP_CAPITAL);
    energy_game.set_ship_hp(1, deficit_capital, 220);
    energy_game.set_ship_shield(1, deficit_capital, 100);
    energy_game.create_fleet(2);
    int deficit_guard = energy_game.create_ship(2, SHIP_SHIELD);
    energy_game.set_ship_hp(2, deficit_guard, 140);
    energy_game.set_ship_shield(2, deficit_guard, 80);
    FT_ASSERT(energy_game.start_raider_assault(PLANET_TERRA, 1.0));
    double energy_raider_shield = energy_game.get_assault_raider_shield(PLANET_TERRA);
    double energy_raider_hull = energy_game.get_assault_raider_hull(PLANET_TERRA);
    FT_ASSERT(energy_game.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(energy_game.assign_fleet_to_assault(PLANET_TERRA, 2));
    energy_game.tick(3.0);
    int deficit_hp = energy_game.get_ship_hp(1, deficit_capital);
    FT_ASSERT(energy_game.is_assault_active(PLANET_TERRA));

    Game balanced_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    balanced_game.set_ore(PLANET_TERRA, ORE_IRON, 200);
    balanced_game.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    balanced_game.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(balanced_game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    FT_ASSERT(balanced_game.place_building(PLANET_TERRA, BUILDING_SMELTER, 1, 0) != 0);
    balanced_game.tick(0.0);
    double balanced_pressure = balanced_game.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(balanced_pressure < 0.1);
    balanced_game.create_fleet(1);
    int balanced_capital = balanced_game.create_ship(1, SHIP_CAPITAL);
    balanced_game.set_ship_hp(1, balanced_capital, 220);
    balanced_game.set_ship_shield(1, balanced_capital, 100);
    balanced_game.create_fleet(2);
    int balanced_guard = balanced_game.create_ship(2, SHIP_SHIELD);
    balanced_game.set_ship_hp(2, balanced_guard, 140);
    balanced_game.set_ship_shield(2, balanced_guard, 80);
    FT_ASSERT(balanced_game.start_raider_assault(PLANET_TERRA, 1.0));
    double balanced_raider_shield = balanced_game.get_assault_raider_shield(PLANET_TERRA);
    double balanced_raider_hull = balanced_game.get_assault_raider_hull(PLANET_TERRA);
    FT_ASSERT(balanced_game.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(balanced_game.assign_fleet_to_assault(PLANET_TERRA, 2));
    balanced_game.tick(3.0);
    int balanced_hp = balanced_game.get_ship_hp(1, balanced_capital);
    FT_ASSERT(balanced_game.is_assault_active(PLANET_TERRA));
    FT_ASSERT(energy_raider_shield > balanced_raider_shield);
    FT_ASSERT(energy_raider_hull > balanced_raider_hull);
    FT_ASSERT(balanced_hp > deficit_hp);

    ft_vector<ft_ship_spatial_state> energy_raider_positions;
    ft_vector<ft_ship_spatial_state> balanced_raider_positions;
    FT_ASSERT(energy_game.get_assault_raider_positions(PLANET_TERRA, energy_raider_positions));
    FT_ASSERT(balanced_game.get_assault_raider_positions(PLANET_TERRA, balanced_raider_positions));
    FT_ASSERT(energy_raider_positions.size() >= balanced_raider_positions.size());
    int energy_radar_count = 0;
    int energy_shield_count = 0;
    int balanced_radar_count = 0;
    int balanced_shield_count = 0;
    for (size_t idx = 0; idx < energy_raider_positions.size(); ++idx)
    {
        if (energy_raider_positions[idx].ship_type == SHIP_RADAR)
            energy_radar_count += 1;
        else if (energy_raider_positions[idx].ship_type == SHIP_SHIELD)
            energy_shield_count += 1;
    }
    for (size_t idx = 0; idx < balanced_raider_positions.size(); ++idx)
    {
        if (balanced_raider_positions[idx].ship_type == SHIP_RADAR)
            balanced_radar_count += 1;
        else if (balanced_raider_positions[idx].ship_type == SHIP_SHIELD)
            balanced_shield_count += 1;
    }
    FT_ASSERT(energy_radar_count > balanced_radar_count);
    FT_ASSERT(energy_shield_count >= balanced_shield_count);

    Game surge_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    surge_game.set_ore(PLANET_TERRA, ORE_IRON, 320);
    surge_game.set_ore(PLANET_TERRA, ORE_COPPER, 320);
    surge_game.set_ore(PLANET_TERRA, ORE_COAL, 320);
    FT_ASSERT(surge_game.place_building(PLANET_TERRA, BUILDING_TRANSFER_NODE, 0, 3) != 0);
    FT_ASSERT(surge_game.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 0) != 0);
    FT_ASSERT(surge_game.place_building(PLANET_TERRA, BUILDING_SMELTER, 2, 2) != 0);
    FT_ASSERT(surge_game.place_building(PLANET_TERRA, BUILDING_PROCESSOR, 0, 1) != 0);
    FT_ASSERT(surge_game.place_building(PLANET_TERRA, BUILDING_SMELTER, 1, 2) != 0);
    surge_game.tick(0.0);
    double surge_pressure = surge_game.get_planet_energy_pressure(PLANET_TERRA);
    FT_ASSERT(surge_pressure > 7.4);
    surge_game.create_fleet(1);
    int surge_capital = surge_game.create_ship(1, SHIP_CAPITAL);
    surge_game.set_ship_hp(1, surge_capital, 220);
    surge_game.set_ship_shield(1, surge_capital, 100);
    FT_ASSERT(surge_game.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(surge_game.assign_fleet_to_assault(PLANET_TERRA, 1));
    surge_game.tick(0.5);
    ft_vector<ft_ship_spatial_state> surge_raider_positions;
    FT_ASSERT(surge_game.get_assault_raider_positions(PLANET_TERRA, surge_raider_positions));
    int surge_radar_count = 0;
    int surge_shield_count = 0;
    for (size_t idx = 0; idx < surge_raider_positions.size(); ++idx)
    {
        if (surge_raider_positions[idx].ship_type == SHIP_RADAR)
            surge_radar_count += 1;
        else if (surge_raider_positions[idx].ship_type == SHIP_SHIELD)
            surge_shield_count += 1;
    }
    FT_ASSERT(surge_raider_positions.size() >= energy_raider_positions.size());
    FT_ASSERT(surge_radar_count >= energy_radar_count + 1);
    FT_ASSERT(surge_shield_count >= energy_shield_count + 1);

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
    narrative_game.set_ore(PLANET_TERRA, ORE_IRON, 20);
    narrative_game.set_ore(PLANET_TERRA, ORE_COPPER, 20);
    narrative_game.tick(0.0);
    narrative_game.create_fleet(1);
    int narrative_setup_one = narrative_game.create_ship(1, SHIP_SHIELD);
    narrative_game.set_ship_hp(1, narrative_setup_one, 80);
    narrative_game.create_fleet(2);
    int narrative_setup_two = narrative_game.create_ship(2, SHIP_SHIELD);
    narrative_game.set_ship_hp(2, narrative_setup_two, 60);
    narrative_game.tick(0.0);
    narrative_game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    narrative_game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    narrative_game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    FT_ASSERT(narrative_game.start_research(RESEARCH_UNLOCK_MARS));
    narrative_game.tick(30.0);
    narrative_game.set_ore(PLANET_TERRA, ORE_MITHRIL, 8);
    narrative_game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    FT_ASSERT(narrative_game.start_research(RESEARCH_UNLOCK_ZALTHOR));
    narrative_game.tick(40.0);
    narrative_game.tick(0.0);
    FT_ASSERT_EQ(QUEST_CLIMACTIC_BATTLE, narrative_game.get_active_quest());
    narrative_game.remove_ship(1, narrative_setup_one);
    narrative_game.remove_ship(2, narrative_setup_two);
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
                double delta_x = std::fabs(after_state.x - manual_defender_positions_start[j].x);
                double delta_z = std::fabs(after_state.z - manual_defender_positions_start[j].z);
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
    FT_ASSERT(std::fabs(burst_focus_shield_start - baseline_focus_shield_start) < 0.01);
    FT_ASSERT(std::fabs(burst_focus_hull_start - baseline_focus_hull_start) < 0.01);
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

    Game generator_off(ft_string("127.0.0.1:8080"), ft_string("/"));
    generator_off.set_ore(PLANET_TERRA, ORE_IRON, 200);
    generator_off.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    generator_off.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(generator_off.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    generator_off.tick(0.0);
    generator_off.create_fleet(1);
    int generator_off_capital = generator_off.create_ship(1, SHIP_CAPITAL);
    generator_off.set_ship_hp(1, generator_off_capital, 220);
    generator_off.set_ship_shield(1, generator_off_capital, 100);
    generator_off.create_fleet(2);
    int generator_off_guard = generator_off.create_ship(2, SHIP_SHIELD);
    generator_off.set_ship_hp(2, generator_off_guard, 140);
    generator_off.set_ship_shield(2, generator_off_guard, 80);
    FT_ASSERT(generator_off.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(generator_off.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(generator_off.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(generator_off.set_assault_support(PLANET_TERRA, false, false, false));
    generator_off.tick(3.0);
    int generator_off_hp = generator_off.get_ship_hp(1, generator_off_capital);
    int generator_off_shield = generator_off.get_ship_shield(1, generator_off_capital);

    Game generator_on(ft_string("127.0.0.1:8080"), ft_string("/"));
    generator_on.set_ore(PLANET_TERRA, ORE_IRON, 200);
    generator_on.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    generator_on.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(generator_on.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    generator_on.tick(0.0);
    generator_on.create_fleet(1);
    int generator_on_capital = generator_on.create_ship(1, SHIP_CAPITAL);
    generator_on.set_ship_hp(1, generator_on_capital, 220);
    generator_on.set_ship_shield(1, generator_on_capital, 100);
    generator_on.create_fleet(2);
    int generator_on_guard = generator_on.create_ship(2, SHIP_SHIELD);
    generator_on.set_ship_hp(2, generator_on_guard, 140);
    generator_on.set_ship_shield(2, generator_on_guard, 80);
    FT_ASSERT(generator_on.start_raider_assault(PLANET_TERRA, 1.0, ASSAULT_CONTROL_ACTIVE));
    FT_ASSERT(generator_on.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(generator_on.assign_fleet_to_assault(PLANET_TERRA, 2));
    FT_ASSERT(generator_on.set_assault_support(PLANET_TERRA, false, false, true));
    generator_on.tick(3.0);
    int generator_on_hp = generator_on.get_ship_hp(1, generator_on_capital);
    int generator_on_shield = generator_on.get_ship_shield(1, generator_on_capital);
    FT_ASSERT(generator_on_hp >= generator_off_hp);
    FT_ASSERT(generator_on_shield > generator_off_shield);

    Game support_alone(ft_string("127.0.0.1:8080"), ft_string("/"));
    support_alone.set_ore(PLANET_TERRA, ORE_IRON, 200);
    support_alone.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    support_alone.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(support_alone.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    support_alone.tick(0.0);
    support_alone.create_fleet(1);
    int lone_salvage = support_alone.create_ship(1, SHIP_SALVAGE);
    support_alone.set_ship_hp(1, lone_salvage, 90);
    support_alone.set_ship_shield(1, lone_salvage, 30);
    FT_ASSERT(support_alone.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(support_alone.assign_fleet_to_assault(PLANET_TERRA, 1));
    support_alone.tick(0.5);
    ft_vector<ft_ship_spatial_state> support_alone_positions;
    FT_ASSERT(support_alone.get_assault_defender_positions(PLANET_TERRA, support_alone_positions));
    double unescorted_salvage_z = -1000.0;
    for (size_t idx = 0; idx < support_alone_positions.size(); ++idx)
    {
        if (support_alone_positions[idx].ship_type == SHIP_SALVAGE)
        {
            unescorted_salvage_z = support_alone_positions[idx].z;
            break;
        }
    }
    FT_ASSERT(unescorted_salvage_z < 0.0);

    Game support_escorted(ft_string("127.0.0.1:8080"), ft_string("/"));
    support_escorted.set_ore(PLANET_TERRA, ORE_IRON, 200);
    support_escorted.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    support_escorted.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(support_escorted.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    support_escorted.tick(0.0);
    support_escorted.create_fleet(1);
    int escorted_salvage = support_escorted.create_ship(1, SHIP_SALVAGE);
    support_escorted.set_ship_hp(1, escorted_salvage, 90);
    support_escorted.set_ship_shield(1, escorted_salvage, 30);
    support_escorted.create_fleet(2);
    int escort_guard = support_escorted.create_ship(2, SHIP_SHIELD);
    support_escorted.set_ship_hp(2, escort_guard, 120);
    support_escorted.set_ship_shield(2, escort_guard, 70);
    FT_ASSERT(support_escorted.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(support_escorted.assign_fleet_to_assault(PLANET_TERRA, 1));
    FT_ASSERT(support_escorted.assign_fleet_to_assault(PLANET_TERRA, 2));
    support_escorted.tick(0.5);
    ft_vector<ft_ship_spatial_state> escorted_positions;
    FT_ASSERT(support_escorted.get_assault_defender_positions(PLANET_TERRA, escorted_positions));
    double escorted_salvage_z = -1000.0;
    for (size_t idx = 0; idx < escorted_positions.size(); ++idx)
    {
        if (escorted_positions[idx].ship_type == SHIP_SALVAGE)
        {
            escorted_salvage_z = escorted_positions[idx].z;
            break;
        }
    }
    FT_ASSERT(escorted_salvage_z > unescorted_salvage_z + 1.2);

    Game outnumbered_line(ft_string("127.0.0.1:8080"), ft_string("/"));
    outnumbered_line.set_ore(PLANET_TERRA, ORE_IRON, 200);
    outnumbered_line.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    outnumbered_line.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(outnumbered_line.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    outnumbered_line.tick(0.0);
    outnumbered_line.create_fleet(1);
    int lone_guard = outnumbered_line.create_ship(1, SHIP_SHIELD);
    outnumbered_line.set_ship_hp(1, lone_guard, 130);
    outnumbered_line.set_ship_shield(1, lone_guard, 70);
    FT_ASSERT(outnumbered_line.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(outnumbered_line.assign_fleet_to_assault(PLANET_TERRA, 1));
    outnumbered_line.tick(0.5);
    ft_vector<ft_ship_spatial_state> outnumbered_positions;
    FT_ASSERT(outnumbered_line.get_assault_defender_positions(PLANET_TERRA, outnumbered_positions));
    double outnumbered_avg = 0.0;
    size_t outnumbered_count = 0;
    for (size_t idx = 0; idx < outnumbered_positions.size(); ++idx)
    {
        if (outnumbered_positions[idx].ship_type == SHIP_SHIELD)
        {
            outnumbered_avg += outnumbered_positions[idx].z;
            outnumbered_count += 1;
        }
    }
    FT_ASSERT(outnumbered_count > 0);
    outnumbered_avg /= static_cast<double>(outnumbered_count);

    Game supported_line(ft_string("127.0.0.1:8080"), ft_string("/"));
    supported_line.set_ore(PLANET_TERRA, ORE_IRON, 200);
    supported_line.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    supported_line.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(supported_line.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    supported_line.tick(0.0);
    for (int fleet_id = 1; fleet_id <= 5; ++fleet_id)
    {
        supported_line.create_fleet(fleet_id);
        int guard_ship = supported_line.create_ship(fleet_id, SHIP_SHIELD);
        supported_line.set_ship_hp(fleet_id, guard_ship, 130);
        supported_line.set_ship_shield(fleet_id, guard_ship, 70);
    }
    FT_ASSERT(supported_line.start_raider_assault(PLANET_TERRA, 1.0));
    for (int fleet_id = 1; fleet_id <= 5; ++fleet_id)
        FT_ASSERT(supported_line.assign_fleet_to_assault(PLANET_TERRA, fleet_id));
    supported_line.tick(0.5);
    ft_vector<ft_ship_spatial_state> supported_positions;
    FT_ASSERT(supported_line.get_assault_defender_positions(PLANET_TERRA, supported_positions));
    double supported_avg = 0.0;
    size_t supported_count = 0;
    for (size_t idx = 0; idx < supported_positions.size(); ++idx)
    {
        if (supported_positions[idx].ship_type == SHIP_SHIELD)
        {
            supported_avg += supported_positions[idx].z;
            supported_count += 1;
        }
    }
    FT_ASSERT(supported_count > 0);
    supported_avg /= static_cast<double>(supported_count);
    FT_ASSERT(outnumbered_avg + 2.0 < supported_avg);

    Game healthy_capital(ft_string("127.0.0.1:8080"), ft_string("/"));
    healthy_capital.set_ore(PLANET_TERRA, ORE_IRON, 200);
    healthy_capital.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    healthy_capital.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(healthy_capital.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    healthy_capital.tick(0.0);
    healthy_capital.create_fleet(1);
    int healthy_warship = healthy_capital.create_ship(1, SHIP_CAPITAL);
    healthy_capital.set_ship_hp(1, healthy_warship, 220);
    healthy_capital.set_ship_shield(1, healthy_warship, 100);
    FT_ASSERT(healthy_capital.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(healthy_capital.assign_fleet_to_assault(PLANET_TERRA, 1));
    healthy_capital.tick(0.5);
    ft_vector<ft_ship_spatial_state> healthy_positions;
    FT_ASSERT(healthy_capital.get_assault_defender_positions(PLANET_TERRA, healthy_positions));
    double healthy_capital_z = -1000.0;
    for (size_t idx = 0; idx < healthy_positions.size(); ++idx)
    {
        if (healthy_positions[idx].ship_type == SHIP_CAPITAL)
        {
            healthy_capital_z = healthy_positions[idx].z;
            break;
        }
    }
    FT_ASSERT(healthy_capital_z < -5.0);

    Game low_hp_capital(ft_string("127.0.0.1:8080"), ft_string("/"));
    low_hp_capital.set_ore(PLANET_TERRA, ORE_IRON, 200);
    low_hp_capital.set_ore(PLANET_TERRA, ORE_COPPER, 200);
    low_hp_capital.set_ore(PLANET_TERRA, ORE_COAL, 200);
    FT_ASSERT(low_hp_capital.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    low_hp_capital.tick(0.0);
    low_hp_capital.create_fleet(1);
    int desperate_warship = low_hp_capital.create_ship(1, SHIP_CAPITAL);
    low_hp_capital.set_ship_hp(1, desperate_warship, 220);
    low_hp_capital.set_ship_shield(1, desperate_warship, 100);
    low_hp_capital.set_ship_hp(1, desperate_warship, 44);
    FT_ASSERT(low_hp_capital.start_raider_assault(PLANET_TERRA, 1.0));
    FT_ASSERT(low_hp_capital.assign_fleet_to_assault(PLANET_TERRA, 1));
    low_hp_capital.tick(0.5);
    ft_vector<ft_ship_spatial_state> low_hp_positions;
    FT_ASSERT(low_hp_capital.get_assault_defender_positions(PLANET_TERRA, low_hp_positions));
    double desperate_capital_z = -1000.0;
    for (size_t idx = 0; idx < low_hp_positions.size(); ++idx)
    {
        if (low_hp_positions[idx].ship_type == SHIP_CAPITAL)
        {
            desperate_capital_z = low_hp_positions[idx].z;
            break;
        }
    }
    FT_ASSERT(desperate_capital_z > healthy_capital_z + 1.5);

    Game hard_game(ft_string("127.0.0.1:8080"), ft_string("/"), GAME_DIFFICULTY_HARD);
    FT_ASSERT_EQ(GAME_DIFFICULTY_HARD, hard_game.get_difficulty());
    hard_game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    hard_game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    hard_game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    hard_game.tick(0.0);
    double hard_defense_timer = hard_game.get_quest_time_remaining(QUEST_DEFENSE_OF_TERRA);
    FT_ASSERT(hard_defense_timer > 134.9 && hard_defense_timer < 135.1);
    hard_game.set_ore(PLANET_TERRA, ORE_IRON, 0);
    hard_game.produce(10.0);
    int hard_iron_yield = hard_game.get_ore(PLANET_TERRA, ORE_IRON);
    FT_ASSERT(hard_iron_yield <= 4);
    hard_game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    hard_game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    hard_game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    FT_ASSERT(hard_game.start_research(RESEARCH_UNLOCK_MARS));
    double hard_research_time = hard_game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(hard_research_time > 35.9 && hard_research_time < 36.1);
    FT_ASSERT(hard_game.start_raider_assault(PLANET_TERRA, 1.0));
    double hard_shield = hard_game.get_assault_raider_shield(PLANET_TERRA);
    FT_ASSERT(hard_shield > 100.5 && hard_shield < 101.5);

    server_thread.join();
    return 0;
}
