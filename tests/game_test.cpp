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

static void run_server()
{
    ft_http_server server;
    server.start("127.0.0.1", 8080, AF_INET, false);
    server.run_once();
    server.run_once();
    server.run_once();
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
    FT_ASSERT(game.is_planet_unlocked(PLANET_TERRA));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_MARS));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_ZALTHOR));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_VULCAN));
    FT_ASSERT(!game.is_planet_unlocked(PLANET_NOCTARIS_PRIME));

    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_UNLOCK_MARS));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_UNLOCK_ZALTHOR));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_UNLOCK_VULCAN));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_UNLOCK_NOCTARIS));
    FT_ASSERT(!game.can_start_research(RESEARCH_UNLOCK_MARS));
    FT_ASSERT(!game.can_start_research(RESEARCH_UNLOCK_ZALTHOR));

    FT_ASSERT_EQ(QUEST_INITIAL_SKIRMISHES, game.get_active_quest());
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, game.get_quest_status(QUEST_INITIAL_SKIRMISHES));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_DEFENSE_OF_TERRA));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_INVESTIGATE_RAIDERS));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_CLIMACTIC_BATTLE));
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, game.get_quest_status(QUEST_CRITICAL_DECISION));
    FT_ASSERT_EQ(QUEST_CHOICE_NONE, game.get_quest_choice(QUEST_CRITICAL_DECISION));

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
    double mithril_rate = game.get_rate(PLANET_MARS, ORE_MITHRIL);
    FT_ASSERT(mithril_rate > 0.049 && mithril_rate < 0.051);

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

    game.set_ore(PLANET_TERRA, ORE_COPPER, 10);
    game.set_ore(PLANET_MARS, ORE_COPPER, 0);
    int moved = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ORE_COPPER, 4);
    FT_ASSERT_EQ(4, moved);
    FT_ASSERT_EQ(6, game.get_ore(PLANET_TERRA, ORE_COPPER));
    FT_ASSERT_EQ(4, game.get_ore(PLANET_MARS, ORE_COPPER));

    game.set_ore(PLANET_TERRA, ORE_IRON, 3);
    game.set_ore(PLANET_MARS, ORE_IRON, 0);
    int over = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ORE_IRON, 10);
    FT_ASSERT_EQ(3, over);
    FT_ASSERT_EQ(0, game.get_ore(PLANET_TERRA, ORE_IRON));
    FT_ASSERT_EQ(3, game.get_ore(PLANET_MARS, ORE_IRON));

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
    FT_ASSERT(game.start_raider_assault(PLANET_TERRA, 1.0));
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

    double vulcan_energy = game.get_planet_energy_generation(PLANET_VULCAN);
    FT_ASSERT(vulcan_energy > 5.9 && vulcan_energy < 6.1);
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

    server_thread.join();
    return 0;
}
