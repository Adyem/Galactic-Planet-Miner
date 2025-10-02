#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Networking/socket_class.hpp"
#include "../libft/PThread/thread.hpp"
#include "../libft/PThread/pthread.hpp"
#include "backend_client.hpp"
#include "buildings.hpp"
#include "game.hpp"
#include "game_test_scenarios.hpp"
#include "planets.hpp"
#include "research.hpp"
#include <cstdint>
#include "../libft/Time/time.hpp"

struct MalformedResponseServerConfig
{
    uint16_t port;
    ft_string payload;

    MalformedResponseServerConfig(uint16_t port_value, const ft_string &body)
        : port(port_value), payload(body)
    {}
};

static void run_malformed_response_server(MalformedResponseServerConfig *config)
{
    if (config == ft_nullptr)
        return ;

    SocketConfig server_config;
    server_config._type = SocketType::SERVER;
    server_config._ip = ft_string("127.0.0.1");
    server_config._port = config->port;
    server_config._backlog = 1;
    server_config._recv_timeout = 0;
    server_config._send_timeout = 0;

    ft_socket server_socket(server_config);
    if (server_socket.get_error() != ER_SUCCESS)
        return ;

    int listen_fd = server_socket.get_fd();
    if (listen_fd < 0)
        return ;

    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = nw_accept(listen_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_len);
    if (client_fd >= 0)
    {
        ssize_t sent = nw_send(client_fd, config->payload.c_str(), config->payload.size(), 0);
        (void)sent;
        FT_CLOSE_SOCKET(client_fd);
    }
    server_socket.close_socket();
}

static ft_string build_loopback_host(uint16_t port)
{
    ft_string host("127.0.0.1:");
    host.append(ft_to_string(port));
    return host;
}

static ft_thread start_malformed_response_server(MalformedResponseServerConfig &config)
{
    return ft_thread(run_malformed_response_server, &config);
}

static void wait_for_server_start()
{
    ft_this_thread_sleep_for(time_duration_ms_create(50));
}

int verify_backend_roundtrip()
{
    const ft_string payload("test");
    const ft_string fallback_prefix("[offline] echo=");
    const size_t payload_size = payload.size();
    const size_t fallback_size = fallback_prefix.size();

    BackendClient offline_client(ft_string("127.0.0.1:65535"), ft_string("/"));
    ft_string offline_response;
    int offline_status = offline_client.send_state(payload, offline_response);
    const char *offline_cstr = offline_response.c_str();
    FT_ASSERT(offline_status != 0);
    FT_ASSERT(offline_response.size() >= fallback_size + payload_size);
    FT_ASSERT_EQ(0, ft_strncmp(offline_cstr, fallback_prefix.c_str(), static_cast<size_t>(fallback_size)));
    FT_ASSERT_EQ(0, ft_strcmp(offline_cstr + offline_response.size() - payload_size, payload.c_str()));

    Game offline_game(ft_string("127.0.0.1:65535"), ft_string("/"));
    FT_ASSERT(offline_game.is_backend_online());
    size_t offline_lore_before = offline_game.get_lore_log().size();
    offline_game.add_ore(PLANET_TERRA, ORE_IRON, 1);
    FT_ASSERT(!offline_game.is_backend_online());
    const ft_vector<ft_string> &offline_log = offline_game.get_lore_log();
    FT_ASSERT_EQ(offline_lore_before + 1, offline_log.size());
    const ft_string &offline_entry = offline_log[offline_lore_before];
    const ft_string offline_prefix("Operations report: backend connection lost");
    FT_ASSERT_EQ(0, ft_strncmp(offline_entry.c_str(), offline_prefix.c_str(), offline_prefix.size()));
    const ft_string offline_status_fragment("(status 503)");
    FT_ASSERT(ft_strstr(offline_entry.c_str(), offline_status_fragment.c_str()) != ft_nullptr);
    long offline_retry_delay = offline_game.get_backend_retry_delay_ms_for_testing();
    FT_ASSERT(offline_retry_delay >= 1000);
    long offline_next_retry = offline_game.get_backend_next_retry_ms_for_testing();
    long offline_failure_time = ft_time_ms();
    FT_ASSERT(offline_next_retry > offline_failure_time);
    offline_game.add_ore(PLANET_TERRA, ORE_COPPER, 1);
    FT_ASSERT_EQ(offline_retry_delay, offline_game.get_backend_retry_delay_ms_for_testing());
    FT_ASSERT_EQ(offline_next_retry, offline_game.get_backend_next_retry_ms_for_testing());
    int wait_milliseconds = static_cast<int>(offline_retry_delay + 50);
    ft_this_thread_sleep_for(time_duration_ms_create(wait_milliseconds));
    offline_game.add_ore(PLANET_TERRA, ORE_GOLD, 1);
    long second_retry_delay = offline_game.get_backend_retry_delay_ms_for_testing();
    FT_ASSERT(second_retry_delay >= offline_retry_delay);
    long second_next_retry = offline_game.get_backend_next_retry_ms_for_testing();
    long second_failure_time = ft_time_ms();
    FT_ASSERT(second_next_retry > second_failure_time);

    const uint16_t malformed_port = 18080;
    MalformedResponseServerConfig malformed_backend_config(malformed_port, ft_string("garbled-response"));
    ft_thread malformed_backend_thread = start_malformed_response_server(malformed_backend_config);
    wait_for_server_start();
    BackendClient malformed_client(build_loopback_host(malformed_port), ft_string("/"));
    ft_string malformed_response;
    int malformed_status = malformed_client.send_state(payload, malformed_response);
    malformed_backend_thread.join();
    FT_ASSERT(malformed_status < 200 || malformed_status >= 400);
    FT_ASSERT(malformed_status != 0);
    FT_ASSERT(malformed_response.size() >= fallback_size + payload_size);
    FT_ASSERT_EQ(0, ft_strncmp(malformed_response.c_str(), fallback_prefix.c_str(), static_cast<size_t>(fallback_size)));
    FT_ASSERT_EQ(0, ft_strcmp(malformed_response.c_str() + malformed_response.size() - payload_size, payload.c_str()));

    const uint16_t malformed_game_port = 18081;
    MalformedResponseServerConfig malformed_game_config(malformed_game_port, ft_string("garbled-response"));
    ft_thread malformed_game_thread = start_malformed_response_server(malformed_game_config);
    wait_for_server_start();
    Game malformed_game(build_loopback_host(malformed_game_port), ft_string("/"));
    FT_ASSERT(malformed_game.is_backend_online());
    size_t malformed_lore_before = malformed_game.get_lore_log().size();
    malformed_game.add_ore(PLANET_TERRA, ORE_IRON, 1);
    malformed_game_thread.join();
    FT_ASSERT(!malformed_game.is_backend_online());
    const ft_vector<ft_string> &malformed_log = malformed_game.get_lore_log();
    FT_ASSERT_EQ(malformed_lore_before + 1, malformed_log.size());
    const ft_string &malformed_entry = malformed_log[malformed_lore_before];
    FT_ASSERT_EQ(0, ft_strncmp(malformed_entry.c_str(), offline_prefix.c_str(), offline_prefix.size()));
    FT_ASSERT(ft_strstr(malformed_entry.c_str(), offline_status_fragment.c_str()) != ft_nullptr);

    const uint16_t redirect_port = 18082;
    const ft_string redirect_payload("HTTP/1.1 302 Found\r\nLocation: /redirect\r\nContent-Length: 0\r\n\r\n");
    MalformedResponseServerConfig redirect_backend_config(redirect_port, redirect_payload);
    ft_thread redirect_backend_thread = start_malformed_response_server(redirect_backend_config);
    wait_for_server_start();
    BackendClient redirect_client(build_loopback_host(redirect_port), ft_string("/"));
    ft_string redirect_response;
    int redirect_status = redirect_client.send_state(payload, redirect_response);
    redirect_backend_thread.join();
    FT_ASSERT_EQ(302, redirect_status);
    FT_ASSERT_EQ(redirect_payload, redirect_response);

    MalformedResponseServerConfig redirect_game_config(redirect_port, redirect_payload);
    ft_thread redirect_game_thread = start_malformed_response_server(redirect_game_config);
    wait_for_server_start();
    Game redirect_game(build_loopback_host(redirect_port), ft_string("/"));
    FT_ASSERT(redirect_game.is_backend_online());
    size_t redirect_lore_before = redirect_game.get_lore_log().size();
    redirect_game.add_ore(PLANET_TERRA, ORE_IRON, 1);
    redirect_game_thread.join();
    FT_ASSERT(redirect_game.is_backend_online());
    const ft_vector<ft_string> &redirect_log = redirect_game.get_lore_log();
    FT_ASSERT_EQ(redirect_lore_before, redirect_log.size());

    BackendClient client(ft_string("127.0.0.1:8080"), ft_string("/"));
    ft_string response;
    int status = client.send_state(payload, response);
    const char *resp = response.c_str();
    FT_ASSERT(response.size() >= payload_size);
    FT_ASSERT_EQ(0, ft_strcmp(resp + response.size() - payload_size, payload.c_str()));
    bool has_fallback_prefix = (response.size() >= fallback_size
        && ft_strncmp(resp, fallback_prefix.c_str(), static_cast<size_t>(fallback_size)) == 0);
    if (has_fallback_prefix)
    {
        FT_ASSERT(status != 0);
        FT_ASSERT(status < 200 || status >= 400);
    }
    else
    {
        FT_ASSERT(status >= 200);
    }

    Game online_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(online_game.is_backend_online());
    size_t online_lore_before = online_game.get_lore_log().size();
    online_game.add_ore(PLANET_TERRA, ORE_IRON, 1);
    FT_ASSERT(online_game.is_backend_online());
    FT_ASSERT_EQ(online_lore_before, online_game.get_lore_log().size());
    return 1;
}

int verify_backend_host_parsing()
{
    BackendClient numeric(ft_string("http://127.0.0.1:18090"), ft_string("/"));
    ft_string expected_numeric_host("127.0.0.1");
    FT_ASSERT_EQ(expected_numeric_host, numeric.get_host_for_testing());
    ft_string expected_numeric_port("18090");
    FT_ASSERT_EQ(expected_numeric_port, numeric.get_port_for_testing());
    FT_ASSERT(!numeric.get_use_ssl_for_testing());

    BackendClient bracketed_ipv6(ft_string("[::1]:65535"), ft_string("/"));
    ft_string expected_bracketed_host("[::1]");
    FT_ASSERT_EQ(expected_bracketed_host, bracketed_ipv6.get_host_for_testing());
    ft_string expected_bracketed_port("65535");
    FT_ASSERT_EQ(expected_bracketed_port, bracketed_ipv6.get_port_for_testing());
    FT_ASSERT(!bracketed_ipv6.get_use_ssl_for_testing());

    BackendClient scheme_without_port(ft_string("https://example.com"), ft_string("/"));
    ft_string expected_scheme_host("example.com");
    FT_ASSERT_EQ(expected_scheme_host, scheme_without_port.get_host_for_testing());
    FT_ASSERT(scheme_without_port.get_port_for_testing().empty());
    FT_ASSERT(scheme_without_port.get_use_ssl_for_testing());

    BackendClient uppercase_scheme(ft_string("HTTPS://example.com"), ft_string("/"));
    FT_ASSERT_EQ(expected_scheme_host, uppercase_scheme.get_host_for_testing());
    FT_ASSERT(uppercase_scheme.get_port_for_testing().empty());
    FT_ASSERT(uppercase_scheme.get_use_ssl_for_testing());

    BackendClient mixed_scheme(ft_string("Http://example.com:8080"), ft_string("/"));
    FT_ASSERT_EQ(expected_scheme_host, mixed_scheme.get_host_for_testing());
    ft_string expected_mixed_port("8080");
    FT_ASSERT_EQ(expected_mixed_port, mixed_scheme.get_port_for_testing());
    FT_ASSERT(!mixed_scheme.get_use_ssl_for_testing());

    BackendClient service_named_port(ft_string("localhost:http"), ft_string("/"));
    ft_string expected_service_host("localhost:http");
    FT_ASSERT_EQ(expected_service_host, service_named_port.get_host_for_testing());
    FT_ASSERT(service_named_port.get_port_for_testing().empty());
    FT_ASSERT(!service_named_port.get_use_ssl_for_testing());

    BackendClient unbracketed_ipv6(ft_string("fc00::1"), ft_string("/"));
    ft_string expected_unbracketed_host("fc00::1");
    FT_ASSERT_EQ(expected_unbracketed_host, unbracketed_ipv6.get_host_for_testing());
    FT_ASSERT(unbracketed_ipv6.get_port_for_testing().empty());
    FT_ASSERT(!unbracketed_ipv6.get_use_ssl_for_testing());

    BackendClient scheme_with_path(ft_string("https://example.org:4443/api"), ft_string("/"));
    ft_string expected_path_host("example.org");
    FT_ASSERT_EQ(expected_path_host, scheme_with_path.get_host_for_testing());
    ft_string expected_path_port("4443");
    FT_ASSERT_EQ(expected_path_port, scheme_with_path.get_port_for_testing());
    FT_ASSERT(scheme_with_path.get_use_ssl_for_testing());

    BackendClient whitespace_client(ft_string("  https://example.net:9090  "), ft_string("/"));
    ft_string expected_whitespace_host("example.net");
    FT_ASSERT_EQ(expected_whitespace_host, whitespace_client.get_host_for_testing());
    ft_string expected_whitespace_port("9090");
    FT_ASSERT_EQ(expected_whitespace_port, whitespace_client.get_port_for_testing());
    FT_ASSERT(whitespace_client.get_use_ssl_for_testing());

    return 1;
}

int verify_locked_planet_reward_delivery()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    const int iron_reward = 75;
    const int copper_reward = 30;

    game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    int iron_total = game.add_ore(PLANET_MARS, ITEM_IRON_BAR, iron_reward);
    FT_ASSERT_EQ(iron_reward, iron_total);

    int after_sub = game.sub_ore(PLANET_MARS, ITEM_IRON_BAR, 15);
    FT_ASSERT_EQ(iron_reward - 15, after_sub);

    game.set_ore(PLANET_MARS, ITEM_COPPER_BAR, copper_reward);

    FT_ASSERT_EQ(iron_reward - 15, game.get_ore(PLANET_MARS, ITEM_IRON_BAR));
    FT_ASSERT_EQ(copper_reward, game.get_ore(PLANET_MARS, ITEM_COPPER_BAR));

    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    double elapsed = 0.0;
    while (!game.is_planet_unlocked(PLANET_MARS) && elapsed < 1200.0)
    {
        game.tick(1.0);
        elapsed += 1.0;
    }
    FT_ASSERT(game.is_planet_unlocked(PLANET_MARS));

    FT_ASSERT_EQ(iron_reward - 15, game.get_ore(PLANET_MARS, ITEM_IRON_BAR));
    FT_ASSERT_EQ(copper_reward, game.get_ore(PLANET_MARS, ITEM_COPPER_BAR));

    const ft_vector<Pair<int, double> > &resources = game.get_planet_resources(PLANET_MARS);
    bool iron_slot_found = false;
    bool copper_slot_found = false;
    for (size_t i = 0; i < resources.size(); ++i)
    {
        if (resources[i].key == ITEM_IRON_BAR)
            iron_slot_found = true;
        else if (resources[i].key == ITEM_COPPER_BAR)
            copper_slot_found = true;
    }

    FT_ASSERT(iron_slot_found);
    FT_ASSERT(copper_slot_found);

    return 1;
}

int verify_lore_log_retention()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    const size_t limit = Game::LORE_LOG_MAX_ENTRIES;
    const size_t total_entries = limit + 10;

    for (size_t i = 0; i < total_entries; ++i)
    {
        ft_string entry("Lore entry ");
        entry.append(ft_to_string(i));
        game.append_lore_entry(entry);
    }

    const ft_vector<ft_string> &log = game.get_lore_log();
    FT_ASSERT_EQ(limit, log.size());

    ft_string expected_first("Lore entry ");
    expected_first.append(ft_to_string(total_entries - limit));
    FT_ASSERT_EQ(expected_first, log[0]);

    ft_string expected_last("Lore entry ");
    expected_last.append(ft_to_string(total_entries - 1));
    FT_ASSERT_EQ(expected_last, log[log.size() - 1]);

    return 1;
}

int verify_fractional_resource_accumulation()
{
    ft_planet_mars planet;
    planet.set_resource(ORE_IRON, 0);
    planet.set_resource(ORE_COPPER, 0);

    for (int i = 0; i < 9; ++i)
        planet.produce(1.0);
    FT_ASSERT_EQ(0, planet.get_resource(ORE_IRON));
    FT_ASSERT_EQ(0, planet.get_resource(ORE_COPPER));

    ft_vector<Pair<int, int> > final_tick = planet.produce(1.0);
    bool found_iron = false;
    for (size_t i = 0; i < final_tick.size(); ++i)
    {
        if (final_tick[i].key == ORE_IRON)
        {
            FT_ASSERT_EQ(1, final_tick[i].value);
            found_iron = true;
        }
    }
    FT_ASSERT(found_iron);
    FT_ASSERT_EQ(1, planet.get_resource(ORE_IRON));
    FT_ASSERT_EQ(1, planet.get_resource(ORE_COPPER));

    for (int i = 0; i < 10; ++i)
        planet.produce(1.0);
    FT_ASSERT_EQ(2, planet.get_resource(ORE_IRON));
    FT_ASSERT_EQ(2, planet.get_resource(ORE_COPPER));

    return 1;
}

int verify_hard_difficulty_fractional_output()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"), GAME_DIFFICULTY_HARD);
    const int planet_id = PLANET_TERRA;
    const int ore_id = ORE_COAL;

    game.set_ore(planet_id, ORE_IRON, 0);
    game.set_ore(planet_id, ORE_COPPER, 0);
    game.set_ore(planet_id, ore_id, 0);

    const int tick_count = 100;
    for (int i = 0; i < tick_count; ++i)
        game.produce(1.0);

    int hard_amount = game.get_ore(planet_id, ore_id);
    FT_ASSERT(hard_amount > 0);

    ft_planet_terra baseline;
    baseline.set_resource(ore_id, 0);
    for (int i = 0; i < tick_count; ++i)
        baseline.produce(1.0);
    int base_amount = baseline.get_resource(ore_id);
    FT_ASSERT(base_amount > 0);

    const double hard_multiplier = 0.85;
    int expected = static_cast<int>(static_cast<double>(base_amount) * hard_multiplier + 0.0000001);
    FT_ASSERT_EQ(expected, hard_amount);

    return 1;
}

int verify_mine_upgrade_station_bonus()
{
    const int planet_id = PLANET_TERRA;
    const int ore_id = ORE_IRON;

    Game baseline(ft_string("127.0.0.1:8080"), ft_string("/"));
    baseline.set_ore(planet_id, ore_id, 0);
    baseline.set_ore(planet_id, ORE_COPPER, 0);
    baseline.set_ore(planet_id, ORE_COAL, 0);

    Game upgraded(ft_string("127.0.0.1:8080"), ft_string("/"));
    upgraded.ensure_planet_item_slot(planet_id, ORE_MITHRIL);
    upgraded.set_ore(planet_id, ore_id, 20);
    upgraded.set_ore(planet_id, ORE_MITHRIL, 4);
    FT_ASSERT(upgraded.place_building(planet_id, BUILDING_UPGRADE_STATION, 1, 0) != 0);
    upgraded.tick(0.0);
    upgraded.set_ore(planet_id, ore_id, 0);
    upgraded.set_ore(planet_id, ORE_COPPER, 0);
    upgraded.set_ore(planet_id, ORE_COAL, 0);
    upgraded.set_ore(planet_id, ORE_MITHRIL, 0);

    const int tick_count = 240;
    for (int i = 0; i < tick_count; ++i)
        baseline.produce(1.0);
    for (int i = 0; i < tick_count; ++i)
        upgraded.produce(1.0);

    int baseline_output = baseline.get_ore(planet_id, ore_id);
    int upgraded_output = upgraded.get_ore(planet_id, ore_id);
    FT_ASSERT(baseline_output > 0);
    FT_ASSERT(upgraded_output > baseline_output);

    double ratio = static_cast<double>(upgraded_output) / static_cast<double>(baseline_output);
    FT_ASSERT(ratio > 1.13);
    FT_ASSERT(ratio < 1.17);

    return 1;
}

int verify_set_ore_creates_missing_resource()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    const int planet_id = PLANET_TERRA;
    const int resource_id = ORE_TRITIUM;

    FT_ASSERT_EQ(0, game.get_ore(planet_id, resource_id));

    const int first_amount = 37;
    game.set_ore(planet_id, resource_id, first_amount);
    FT_ASSERT_EQ(first_amount, game.get_ore(planet_id, resource_id));

    const int second_amount = 12;
    game.set_ore(planet_id, resource_id, second_amount);
    FT_ASSERT_EQ(second_amount, game.get_ore(planet_id, resource_id));

    return 1;
}

int verify_supply_route_key_collisions()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    Game::ft_supply_route *first = game.ensure_supply_route(65000, 70000);
    FT_ASSERT(first != ft_nullptr);
    int first_id = first->id;

    Game::ft_supply_route *second = game.ensure_supply_route(65000, 70001);
    FT_ASSERT(second != ft_nullptr);
    FT_ASSERT(second != first);
    FT_ASSERT(first_id != second->id);

    Game::ft_supply_route *swapped = game.ensure_supply_route(70000, 65000);
    FT_ASSERT(swapped != ft_nullptr);
    FT_ASSERT(swapped != first);
    FT_ASSERT(swapped != second);

    Game::ft_supply_route *first_again = game.ensure_supply_route(65000, 70000);
    FT_ASSERT(first_again == first);
    FT_ASSERT_EQ(first_id, first_again->id);

    const Game::ft_supply_route *lookup_first = game.get_route_by_id(first_id);
    FT_ASSERT(lookup_first != ft_nullptr);
    FT_ASSERT_EQ(65000, lookup_first->origin_planet_id);
    FT_ASSERT_EQ(70000, lookup_first->destination_planet_id);

    const Game::ft_supply_route *lookup_second = game.get_route_by_id(second->id);
    FT_ASSERT(lookup_second != ft_nullptr);
    FT_ASSERT_EQ(65000, lookup_second->origin_planet_id);
    FT_ASSERT_EQ(70001, lookup_second->destination_planet_id);

    const Game::ft_supply_route *lookup_swapped = game.get_route_by_id(swapped->id);
    FT_ASSERT(lookup_swapped != ft_nullptr);
    FT_ASSERT_EQ(70000, lookup_swapped->origin_planet_id);
    FT_ASSERT_EQ(65000, lookup_swapped->destination_planet_id);

    Game::ft_supply_route *third = game.ensure_supply_route(131072, 196608);
    FT_ASSERT(third != ft_nullptr);
    FT_ASSERT(third != first);
    FT_ASSERT(third != second);
    FT_ASSERT(third != swapped);

    const Game::ft_supply_route *lookup_third = game.get_route_by_id(third->id);
    FT_ASSERT(lookup_third != ft_nullptr);
    FT_ASSERT_EQ(131072, lookup_third->origin_planet_id);
    FT_ASSERT_EQ(196608, lookup_third->destination_planet_id);

    return 1;
}

int verify_trade_relay_convoy_modifiers()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    auto stock_resource = [&](int planet_id, int resource_id, int amount) {
        game.ensure_planet_item_slot(planet_id, resource_id);
        game.set_ore(planet_id, resource_id, amount);
    };

    stock_resource(PLANET_TERRA, ORE_IRON, 400);
    stock_resource(PLANET_TERRA, ORE_COPPER, 400);
    stock_resource(PLANET_TERRA, ORE_COAL, 400);
    stock_resource(PLANET_TERRA, ORE_MITHRIL, 200);
    stock_resource(PLANET_TERRA, ORE_GOLD, 120);
    stock_resource(PLANET_TERRA, ORE_TITANIUM, 160);
    stock_resource(PLANET_TERRA, ORE_TIN, 160);
    stock_resource(PLANET_TERRA, ORE_SILVER, 160);
    stock_resource(PLANET_TERRA, ITEM_IRON_BAR, 240);
    stock_resource(PLANET_TERRA, ITEM_COPPER_BAR, 240);
    stock_resource(PLANET_TERRA, ITEM_ENGINE_PART, 180);
    stock_resource(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 140);
    stock_resource(PLANET_TERRA, ITEM_TITANIUM_BAR, 140);
    stock_resource(PLANET_TERRA, ITEM_FUSION_REACTOR, 12);
    stock_resource(PLANET_TERRA, ITEM_ACCUMULATOR, 40);

    auto complete_research = [&](int research_id) -> int {
        FT_ASSERT(game.start_research(research_id));
        game.tick(200.0);
        FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(research_id));
        return 1;
    };

    complete_research(RESEARCH_UNLOCK_MARS);
    complete_research(RESEARCH_UNLOCK_ZALTHOR);
    complete_research(RESEARCH_UNLOCK_VULCAN);
    complete_research(RESEARCH_UNLOCK_NOCTARIS);
    complete_research(RESEARCH_SOLAR_PANELS);
    complete_research(RESEARCH_CRAFTING_MASTERY);
    complete_research(RESEARCH_INTERSTELLAR_TRADE);

    stock_resource(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 0);
    stock_resource(PLANET_NOCTARIS_PRIME, ITEM_COPPER_BAR, 60);
    stock_resource(PLANET_NOCTARIS_PRIME, ITEM_ADVANCED_ENGINE_PART, 30);
    stock_resource(PLANET_NOCTARIS_PRIME, ITEM_ACCUMULATOR, 12);

    int noctaris_start = game.get_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);
    stock_resource(PLANET_TERRA, ITEM_IRON_BAR, 220);
    int dispatched = game.transfer_ore(PLANET_TERRA, PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 100);
    FT_ASSERT_EQ(100, dispatched);
    double baseline_elapsed = 0.0;
    while (game.get_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR) == noctaris_start && baseline_elapsed < 600.0)
    {
        game.tick(1.0);
        baseline_elapsed += 1.0;
    }
    FT_ASSERT(baseline_elapsed < 600.0);
    int noctaris_after_baseline = game.get_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);
    int baseline_delivered = noctaris_after_baseline - noctaris_start;
    FT_ASSERT(baseline_delivered > 0);
    FT_ASSERT(baseline_delivered < 100);
    FT_ASSERT_EQ(0, game.get_active_convoy_count());

    stock_resource(PLANET_TERRA, ITEM_COPPER_BAR, 120);
    stock_resource(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 120);
    stock_resource(PLANET_TERRA, ITEM_ACCUMULATOR, 40);

    int terra_relay = game.place_building(PLANET_TERRA, BUILDING_TRADE_RELAY, 2, 0);
    FT_ASSERT(terra_relay != 0);
    int noctaris_relay = game.place_building(PLANET_NOCTARIS_PRIME, BUILDING_TRADE_RELAY, 1, 0);
    FT_ASSERT(noctaris_relay != 0);

    int noctaris_relay_start = game.get_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);
    stock_resource(PLANET_TERRA, ITEM_IRON_BAR, 220);
    int relay_dispatched = game.transfer_ore(PLANET_TERRA, PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR, 100);
    FT_ASSERT_EQ(100, relay_dispatched);
    double relay_elapsed = 0.0;
    while (game.get_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR) == noctaris_relay_start && relay_elapsed < 600.0)
    {
        game.tick(1.0);
        relay_elapsed += 1.0;
    }
    FT_ASSERT(relay_elapsed < 600.0);
    int noctaris_final = game.get_ore(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);
    int relay_delivered = noctaris_final - noctaris_relay_start;
    FT_ASSERT_EQ(100, relay_delivered);
    FT_ASSERT(relay_elapsed + 0.001 < baseline_elapsed);
    FT_ASSERT_EQ(0, game.get_active_convoy_count());

    return 1;
}

int verify_convoy_escort_travel_speed()
{
    Game baseline(ft_string("127.0.0.1:8080"), ft_string("/"));
    Game escorted(ft_string("127.0.0.1:8080"), ft_string("/"));

    auto prepare = [&](Game &game) -> int {
        FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
        game.tick(200.0);
        FT_ASSERT(game.is_planet_unlocked(PLANET_MARS));
        game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
        game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
        game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
        game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
        return 1;
    };

    auto dispatch_and_measure = [&](Game &game, bool with_escort) -> double {
        game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
        game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
        if (with_escort)
        {
            const int fleet_id = 7801;
            game.create_fleet(fleet_id);
            FT_ASSERT(game.create_ship(fleet_id, SHIP_SHIELD) != 0);
            FT_ASSERT(game.create_ship(fleet_id, SHIP_SHIELD) != 0);
            FT_ASSERT(game.create_ship(fleet_id, SHIP_SHIELD) != 0);
            FT_ASSERT(game.assign_convoy_escort(PLANET_TERRA, PLANET_MARS, fleet_id));
        }
        else
        {
            int assigned = game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS);
            if (assigned != 0)
                FT_ASSERT(game.clear_convoy_escort(PLANET_TERRA, PLANET_MARS));
        }
        int destination_start = game.get_ore(PLANET_MARS, ITEM_IRON_BAR);
        int dispatched = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 100);
        FT_ASSERT_EQ(100, dispatched);
        if (with_escort)
            FT_ASSERT_EQ(0, game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS));
        double elapsed = 0.0;
        while (game.get_ore(PLANET_MARS, ITEM_IRON_BAR) == destination_start && elapsed < 600.0)
        {
            game.tick(0.5);
            elapsed += 0.5;
        }
        FT_ASSERT(elapsed < 600.0);
        int delivered = game.get_ore(PLANET_MARS, ITEM_IRON_BAR) - destination_start;
        FT_ASSERT_EQ(100, delivered);
        double cleanup = elapsed;
        while (game.get_active_convoy_count() > 0 && cleanup < 600.0)
        {
            game.tick(0.5);
            cleanup += 0.5;
        }
        FT_ASSERT_EQ(0, game.get_active_convoy_count());
        return elapsed;
    };

    prepare(baseline);
    double baseline_time = dispatch_and_measure(baseline, false);

    prepare(escorted);
    double escorted_time = dispatch_and_measure(escorted, true);

    FT_ASSERT(escorted_time + 0.5 < baseline_time);

    return 1;
}

int verify_convoy_escort_assignment_persistence()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    game.tick(200.0);
    FT_ASSERT(game.is_planet_unlocked(PLANET_MARS));
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
    game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);

    const int fleet_id = 9101;
    game.create_fleet(fleet_id);
    FT_ASSERT(game.create_ship(fleet_id, SHIP_SHIELD) != 0);
    FT_ASSERT(game.assign_convoy_escort(PLANET_TERRA, PLANET_MARS, fleet_id));

    game.set_fleet_location_travel(fleet_id, PLANET_TERRA, PLANET_MARS, 10.0);

    int mars_start = game.get_ore(PLANET_MARS, ITEM_IRON_BAR);
    int dispatched = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 50);
    FT_ASSERT_EQ(50, dispatched);
    FT_ASSERT_EQ(0, game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS));

    double elapsed = 0.0;
    while (game.get_ore(PLANET_MARS, ITEM_IRON_BAR) == mars_start && elapsed < 600.0)
    {
        game.tick(1.0);
        elapsed += 1.0;
    }
    FT_ASSERT(elapsed < 600.0);
    double cleanup = elapsed;
    while (game.get_active_convoy_count() > 0 && cleanup < 600.0)
    {
        game.tick(1.0);
        cleanup += 1.0;
    }
    FT_ASSERT_EQ(0, game.get_active_convoy_count());

    game.set_fleet_location_planet(fleet_id, PLANET_TERRA);
    FT_ASSERT(game.assign_convoy_escort(PLANET_TERRA, PLANET_MARS, fleet_id));
    FT_ASSERT_EQ(fleet_id, game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS));
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);

    int mars_second_start = game.get_ore(PLANET_MARS, ITEM_IRON_BAR);
    int dispatched_second = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 50);
    FT_ASSERT_EQ(50, dispatched_second);
    FT_ASSERT_EQ(0, game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS));

    double elapsed_second = 0.0;
    while (game.get_ore(PLANET_MARS, ITEM_IRON_BAR) == mars_second_start && elapsed_second < 600.0)
    {
        game.tick(0.5);
        elapsed_second += 0.5;
    }
    FT_ASSERT(elapsed_second < 600.0);
    double cleanup_second = elapsed_second;
    while (game.get_active_convoy_count() > 0 && cleanup_second < 600.0)
    {
        game.tick(0.5);
        cleanup_second += 0.5;
    }
    FT_ASSERT_EQ(0, game.get_active_convoy_count());

    return 1;
}
