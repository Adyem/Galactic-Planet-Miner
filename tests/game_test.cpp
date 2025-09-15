#include "../libft/Networking/http_server.hpp"
#include "../libft/PThread/thread.hpp"
#include "../libft/Time/time.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/pair.hpp"
#include "game.hpp"
#include "fleets.hpp"

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
    double mithril_rate = game.get_rate(PLANET_MARS, ORE_MITHRIL);
    FT_ASSERT(mithril_rate > 0.049 && mithril_rate < 0.051);

    game.produce(10.0);
    FT_ASSERT_EQ(12, game.get_ore(PLANET_TERRA, ORE_IRON));

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
    game.set_fleet_location_planet(1, PLANET_TERRA);
    ft_location loc1 = game.get_fleet_location(1);
    FT_ASSERT_EQ(LOCATION_PLANET, loc1.type);
    FT_ASSERT_EQ(PLANET_TERRA, loc1.from);

    game.create_fleet(2);
    int ship_c = game.create_ship(2, SHIP_SHIELD);
    FT_ASSERT(ship_c != ship_a);
    FT_ASSERT(ship_c != ship_b);
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

    server_thread.join();
    return 0;
}
