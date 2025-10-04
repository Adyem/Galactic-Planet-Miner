#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Networking/socket_class.hpp"
#include "../libft/PThread/thread.hpp"
#include "../libft/PThread/pthread.hpp"
#include "../libft/RNG/rng_internal.hpp"
#include "../libft/Math/math.hpp"
#include "backend_client.hpp"
#include "buildings.hpp"
#include "game.hpp"
#include "fleets.hpp"
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

int verify_initial_journal_seed()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    const ft_vector<ft_string> &journal = game.get_journal_entries();
    FT_ASSERT(journal.size() >= 10);

    bool found_miner = false;
    bool found_lumen = false;
    bool found_daisy = false;
    bool found_blackthorne = false;
    bool found_zara = false;
    bool found_finn = false;
    bool found_terra = false;
    bool found_mars = false;
    bool found_zalthor = false;
    bool found_convoy_corps = false;

    for (size_t i = 0; i < journal.size(); ++i)
    {
        const ft_string &entry = journal[i];
        if (!found_miner && ft_strstr(entry.c_str(), "Character Profile – Old Miner Joe") != ft_nullptr)
            found_miner = true;
        if (!found_lumen && ft_strstr(entry.c_str(), "Character Profile – Professor Lumen") != ft_nullptr)
            found_lumen = true;
        if (!found_daisy && ft_strstr(entry.c_str(), "Character Profile – Farmer Daisy") != ft_nullptr)
            found_daisy = true;
        if (!found_blackthorne && ft_strstr(entry.c_str(), "Character Profile – Captain Blackthorne") != ft_nullptr)
            found_blackthorne = true;
        if (!found_zara && ft_strstr(entry.c_str(), "Character Profile – Navigator Zara") != ft_nullptr)
            found_zara = true;
        if (!found_finn && ft_strstr(entry.c_str(), "Character Profile – Old Scout Finn") != ft_nullptr)
            found_finn = true;
        if (!found_terra && ft_strstr(entry.c_str(), "Journal – Rebuilding Terra") != ft_nullptr)
            found_terra = true;
        if (!found_mars && ft_strstr(entry.c_str(), "Journal – Mars Garrison Outposts") != ft_nullptr)
            found_mars = true;
        if (!found_zalthor && ft_strstr(entry.c_str(), "Journal – The Zalthor Anomaly") != ft_nullptr)
            found_zalthor = true;
        if (!found_convoy_corps && ft_strstr(entry.c_str(), "Journal – Convoy Corps Charter") != ft_nullptr)
            found_convoy_corps = true;
    }

    FT_ASSERT(found_miner);
    FT_ASSERT(found_lumen);
    FT_ASSERT(found_daisy);
    FT_ASSERT(found_blackthorne);
    FT_ASSERT(found_zara);
    FT_ASSERT(found_finn);
    FT_ASSERT(found_terra);
    FT_ASSERT(found_mars);
    FT_ASSERT(found_zalthor);
    FT_ASSERT(found_convoy_corps);

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

int verify_supply_route_threat_decay()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    Game::ft_supply_route *route = game.ensure_supply_route(71000, 72000);
    FT_ASSERT(route != ft_nullptr);

    route->threat_level = 4.0;
    route->quiet_timer = 45.0;

    game.decay_all_route_threat(10.0);

    FT_ASSERT(route->threat_level < 4.0);
    FT_ASSERT(route->threat_level > 0.0);
    FT_ASSERT(route->quiet_timer > 45.0);

    return 1;
}

int verify_celestial_barrens_salvage_event()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    game.unlock_planet(PLANET_MARS);
    game.unlock_planet(PLANET_ZALTHOR);
    game.ensure_planet_item_slot(PLANET_ZALTHOR, ORE_CRYSTAL);
    game.ensure_planet_item_slot(PLANET_ZALTHOR, ORE_GOLD);
    game.ensure_planet_item_slot(PLANET_ZALTHOR, ORE_TRITIUM);
    game.set_ore(PLANET_ZALTHOR, ORE_CRYSTAL, 0);
    game.set_ore(PLANET_ZALTHOR, ORE_GOLD, 0);
    game.set_ore(PLANET_ZALTHOR, ORE_TRITIUM, 0);

    Game::ft_supply_route *route = game.ensure_supply_route(PLANET_MARS, PLANET_ZALTHOR);
    FT_ASSERT(route != ft_nullptr);
    route->threat_level = 5.0;
    route->quiet_timer = 120.0;

    Game::ft_supply_convoy convoy;
    convoy.route_id = route->id;
    convoy.origin_planet_id = PLANET_MARS;
    convoy.destination_planet_id = PLANET_ZALTHOR;
    convoy.escort_rating = 2;

    size_t lore_before = game.get_lore_log().size();
    int crystal_before = game.get_ore(PLANET_ZALTHOR, ORE_CRYSTAL);
    double threat_before = route->threat_level;

    ft_seed_random_engine(97531u);
    FT_ASSERT(game.handle_celestial_barrens_salvage(convoy));

    int crystal_after = game.get_ore(PLANET_ZALTHOR, ORE_CRYSTAL);
    FT_ASSERT_EQ(crystal_before + 4, crystal_after);
    double threat_after = route->threat_level;
    FT_ASSERT(threat_after > threat_before);
    FT_ASSERT(route->quiet_timer >= 0.0 && route->quiet_timer < 0.0001);

    const ft_vector<ft_string> &log = game.get_lore_log();
    FT_ASSERT(log.size() >= lore_before + 2);
    bool found_salvage_entry = false;
    bool found_resource_snippet = false;
    for (size_t i = lore_before; i < log.size(); ++i)
    {
        const ft_string &entry = log[i];
        if (!found_salvage_entry && ft_strstr(entry.c_str(), "Celestial Barrens") != ft_nullptr)
            found_salvage_entry = true;
        if (!found_resource_snippet && ft_strstr(entry.c_str(), "crystal") != ft_nullptr)
            found_resource_snippet = true;
    }
    FT_ASSERT(found_salvage_entry);
    FT_ASSERT(found_resource_snippet);
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_CELESTIAL_BARRENS));
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_IMPERIUM_PRESSURE));
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_RESOURCE_CRYSTAL_INTRIGUE));

    return 1;
}

int verify_imperium_pressure_threshold()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    game.unlock_planet(PLANET_MARS);
    game.unlock_planet(PLANET_ZALTHOR);

    Game::ft_supply_route *route = game.ensure_supply_route(PLANET_MARS, PLANET_ZALTHOR);
    FT_ASSERT(route != ft_nullptr);

    const ft_vector<ft_string> &initial_journal = game.get_journal_entries();
    size_t initial_count = initial_journal.size();

    route->threat_level = 2.5;
    game.maybe_unlock_imperium_pressure(*route);
    FT_ASSERT(!game.is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_IMPERIUM_PRESSURE));
    FT_ASSERT_EQ(initial_count, game.get_journal_entries().size());

    route->threat_level = 3.4;
    game.maybe_unlock_imperium_pressure(*route);
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_IMPERIUM_PRESSURE));

    const ft_vector<ft_string> &after_unlock = game.get_journal_entries();
    FT_ASSERT(after_unlock.size() == initial_count + 1);
    bool found_entry = false;
    for (size_t i = 0; i < after_unlock.size(); ++i)
    {
        if (ft_strstr(after_unlock[i].c_str(), "Imperium Pressure") != ft_nullptr)
        {
            found_entry = true;
            break;
        }
    }
    FT_ASSERT(found_entry);

    size_t post_unlock_count = after_unlock.size();
    route->threat_level = 9.0;
    game.maybe_unlock_imperium_pressure(*route);
    FT_ASSERT_EQ(post_unlock_count, game.get_journal_entries().size());

    return 1;
}

int verify_nebula_outpost_scan_event()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    game.unlock_planet(PLANET_NOCTARIS_PRIME);
    game.ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ITEM_IRON_BAR);

    Game::ft_supply_route *route = game.ensure_supply_route(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
    FT_ASSERT(route != ft_nullptr);
    route->threat_level = 4.2;
    route->quiet_timer = 90.0;

    int contract_id = game.create_supply_contract(PLANET_TERRA, PLANET_NOCTARIS_PRIME,
        ITEM_IRON_BAR, 30, 100.0, -1, 2);
    FT_ASSERT(contract_id > 0);
    Pair<int, Game::ft_supply_contract> *contract_entry = game._supply_contracts.find(contract_id);
    FT_ASSERT(contract_entry != ft_nullptr);
    contract_entry->value.elapsed_seconds = 0.0;

    Game::ft_supply_convoy convoy;
    convoy.route_id = route->id;
    convoy.contract_id = contract_id;
    convoy.origin_planet_id = PLANET_TERRA;
    convoy.destination_planet_id = PLANET_NOCTARIS_PRIME;
    convoy.escort_rating = 1;

    size_t lore_before = game.get_lore_log().size();
    double threat_before = route->threat_level;

    ft_seed_random_engine(97531u);
    FT_ASSERT(game.handle_nebula_outpost_scan(convoy));

    double threat_after = route->threat_level;
    FT_ASSERT(threat_after < threat_before);
    FT_ASSERT(route->quiet_timer >= 0.0 && route->quiet_timer < 0.0001);
    double elapsed = contract_entry->value.elapsed_seconds;
    FT_ASSERT(elapsed > 9.9 && elapsed < 10.1);

    const ft_vector<ft_string> &log = game.get_lore_log();
    FT_ASSERT(log.size() > lore_before);
    bool found_scan_entry = false;
    for (size_t i = lore_before; i < log.size(); ++i)
    {
        if (ft_strstr(log[i].c_str(), "Nebula-X") != ft_nullptr)
        {
            found_scan_entry = true;
            break;
        }
    }
    FT_ASSERT(found_scan_entry);
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_NEBULA_OUTPOST));

    return 1;
}

int verify_resource_lore_rotation()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    size_t lore_start = game.get_lore_log().size();
    FT_ASSERT(game.append_resource_lore_snippet(ORE_IRON, PLANET_TERRA, PLANET_MARS));
    const ft_vector<ft_string> &log_after_first = game.get_lore_log();
    FT_ASSERT(log_after_first.size() == lore_start + 1);
    const ft_string &first_entry = log_after_first[lore_start];
    FT_ASSERT(ft_strstr(first_entry.c_str(), "Old Miner Joe") != ft_nullptr);
    FT_ASSERT(ft_strstr(first_entry.c_str(), "Terra -> Mars") != ft_nullptr);
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_RESOURCE_IRON_FOUNDATION));
    const ft_vector<ft_string> &journal_after_first = game.get_journal_entries();
    FT_ASSERT(journal_after_first.size() >= 1);
    FT_ASSERT(ft_strstr(journal_after_first[journal_after_first.size() - 1].c_str(),
        "Iron Lifeblood") != ft_nullptr);
    size_t journal_count = journal_after_first.size();

    FT_ASSERT(game.append_resource_lore_snippet(ORE_IRON, PLANET_TERRA, PLANET_MARS));
    const ft_vector<ft_string> &log_after_second = game.get_lore_log();
    FT_ASSERT(log_after_second.size() == lore_start + 2);
    const ft_string &second_entry = log_after_second[lore_start + 1];
    FT_ASSERT(ft_strstr(second_entry.c_str(), "Quartermaster Nia") != ft_nullptr);
    FT_ASSERT(ft_strstr(second_entry.c_str(), "Terra -> Mars") != ft_nullptr);
    FT_ASSERT(game.get_journal_entries().size() == journal_count);

    FT_ASSERT(game.append_resource_lore_snippet(ORE_IRON, PLANET_TERRA, PLANET_MARS));
    const ft_vector<ft_string> &log_after_third = game.get_lore_log();
    FT_ASSERT(log_after_third.size() == lore_start + 3);
    const ft_string &third_entry = log_after_third[lore_start + 2];
    FT_ASSERT(ft_strstr(third_entry.c_str(), "Old Miner Joe") != ft_nullptr);
    FT_ASSERT(ft_strstr(third_entry.c_str(), "Terra -> Mars") != ft_nullptr);
    FT_ASSERT(game.get_journal_entries().size() == journal_count);

    return 1;
}

int verify_nanomaterial_resource_lore()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    size_t lore_start = game.get_lore_log().size();
    size_t journal_start = game.get_journal_entries().size();

    FT_ASSERT(game.append_resource_lore_snippet(ORE_NANOMATERIAL, PLANET_TERRA, PLANET_NOCTARIS_PRIME));
    const ft_vector<ft_string> &log_after_first = game.get_lore_log();
    FT_ASSERT_EQ(lore_start + 1, log_after_first.size());
    const ft_string &first_entry = log_after_first[lore_start];
    FT_ASSERT(ft_strstr(first_entry.c_str(), "nanomaterial") != ft_nullptr);
    FT_ASSERT(ft_strstr(first_entry.c_str(), "Terra -> Noctaris Prime") != ft_nullptr);

    const ft_vector<ft_string> &journal_after_first = game.get_journal_entries();
    FT_ASSERT_EQ(journal_start + 1, journal_after_first.size());
    const ft_string &journal_entry = journal_after_first[journal_after_first.size() - 1];
    FT_ASSERT(ft_strstr(journal_entry.c_str(), "Nanomaterial Renewal") != ft_nullptr);

    FT_ASSERT(game.append_resource_lore_snippet(ORE_NANOMATERIAL, PLANET_MARS, PLANET_LUNA));
    const ft_vector<ft_string> &log_after_second = game.get_lore_log();
    FT_ASSERT_EQ(lore_start + 2, log_after_second.size());
    const ft_string &second_entry = log_after_second[lore_start + 1];
    FT_ASSERT(ft_strstr(second_entry.c_str(), "nanomaterial") != ft_nullptr);
    FT_ASSERT(ft_strstr(second_entry.c_str(), "Mars -> Luna") != ft_nullptr);

    FT_ASSERT_EQ(journal_after_first.size(), game.get_journal_entries().size());

    return 1;
}

int verify_raider_lore_rotation()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    size_t lore_start = game.get_lore_log().size();
    FT_ASSERT(game.append_raider_lore_snippet(PLANET_TERRA, PLANET_MARS));
    const ft_vector<ft_string> &log_after_first = game.get_lore_log();
    FT_ASSERT(log_after_first.size() == lore_start + 1);
    const ft_string &first_entry = log_after_first[lore_start];
    FT_ASSERT(ft_strstr(first_entry.c_str(), "Old Scout Finn") != ft_nullptr);
    FT_ASSERT(ft_strstr(first_entry.c_str(), "Terra -> Mars") != ft_nullptr);
    FT_ASSERT(game.is_journal_entry_unlocked(JOURNAL_ENTRY_RAIDER_SIGNAL_WEB));
    const ft_vector<ft_string> &journal_after_first = game.get_journal_entries();
    FT_ASSERT(journal_after_first.size() >= 1);
    FT_ASSERT(ft_strstr(journal_after_first[journal_after_first.size() - 1].c_str(),
        "Raider Signal Web") != ft_nullptr);
    size_t journal_count = journal_after_first.size();

    FT_ASSERT(game.append_raider_lore_snippet(PLANET_MARS, PLANET_ZALTHOR));
    const ft_vector<ft_string> &log_after_second = game.get_lore_log();
    FT_ASSERT(log_after_second.size() == lore_start + 2);
    const ft_string &second_entry = log_after_second[lore_start + 1];
    FT_ASSERT(ft_strstr(second_entry.c_str(), "Captain Blackthorne") != ft_nullptr);
    FT_ASSERT(ft_strstr(second_entry.c_str(), "Mars -> Zalthor") != ft_nullptr);

    FT_ASSERT(game.append_raider_lore_snippet(PLANET_ZALTHOR, PLANET_VULCAN));
    const ft_vector<ft_string> &log_after_third = game.get_lore_log();
    FT_ASSERT(log_after_third.size() == lore_start + 3);
    const ft_string &third_entry = log_after_third[lore_start + 2];
    FT_ASSERT(ft_strstr(third_entry.c_str(), "Professor Lumen") != ft_nullptr);
    FT_ASSERT(ft_strstr(third_entry.c_str(), "Zalthor -> Vulcan") != ft_nullptr);

    FT_ASSERT(game.append_raider_lore_snippet(PLANET_VULCAN, PLANET_NOCTARIS_PRIME));
    const ft_vector<ft_string> &log_after_fourth = game.get_lore_log();
    FT_ASSERT(log_after_fourth.size() == lore_start + 4);
    const ft_string &fourth_entry = log_after_fourth[lore_start + 3];
    FT_ASSERT(ft_strstr(fourth_entry.c_str(), "Navigator Zara") != ft_nullptr);
    FT_ASSERT(ft_strstr(fourth_entry.c_str(), "Vulcan -> Noctaris Prime") != ft_nullptr);

    FT_ASSERT(game.append_raider_lore_snippet(PLANET_NOCTARIS_PRIME, PLANET_TERRA));
    const ft_vector<ft_string> &log_after_fifth = game.get_lore_log();
    FT_ASSERT(log_after_fifth.size() == lore_start + 5);
    const ft_string &fifth_entry = log_after_fifth[lore_start + 4];
    FT_ASSERT(ft_strstr(fifth_entry.c_str(), "Old Scout Finn") != ft_nullptr);
    FT_ASSERT(ft_strstr(fifth_entry.c_str(), "Noctaris Prime -> Terra") != ft_nullptr);

    const ft_vector<ft_string> &journal_after = game.get_journal_entries();
    FT_ASSERT(journal_after.size() == journal_count);

    return 1;
}

int verify_quest_log_snapshot()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    Game::ft_quest_log_snapshot snapshot;
    game.get_quest_log_snapshot(snapshot);

    FT_ASSERT_EQ(QUEST_INITIAL_SKIRMISHES, snapshot.active_main_quest_id);
    const Game::ft_quest_log_entry *initial_entry = ft_nullptr;
    for (size_t i = 0; i < snapshot.main_quests.size(); ++i)
    {
        if (snapshot.main_quests[i].quest_id == QUEST_INITIAL_SKIRMISHES)
        {
            initial_entry = &snapshot.main_quests[i];
            break;
        }
    }
    FT_ASSERT(initial_entry != ft_nullptr);
    FT_ASSERT_EQ(QUEST_STATUS_ACTIVE, initial_entry->status);
    FT_ASSERT(initial_entry->objectives.size() >= 1);
    bool found_iron_requirement = false;
    for (size_t i = 0; i < initial_entry->objectives.size(); ++i)
    {
        const Game::ft_quest_objective_snapshot &objective = initial_entry->objectives[i];
        if (objective.type == QUEST_OBJECTIVE_RESOURCE_TOTAL && objective.target_id == ORE_IRON)
        {
            found_iron_requirement = true;
            FT_ASSERT(objective.current_amount >= 0.0);
            break;
        }
    }
    FT_ASSERT(found_iron_requirement);

    FT_ASSERT(advance_to_order_final_verdict(game));

    game.get_quest_log_snapshot(snapshot);

    const Game::ft_quest_log_entry *verdict_entry = ft_nullptr;
    const Game::ft_quest_log_entry *dominion_entry = ft_nullptr;
    const Game::ft_quest_log_entry *rebellion_entry = ft_nullptr;
    for (size_t i = 0; i < snapshot.main_quests.size(); ++i)
    {
        const Game::ft_quest_log_entry &entry = snapshot.main_quests[i];
        if (entry.quest_id == QUEST_ORDER_FINAL_VERDICT)
            verdict_entry = &snapshot.main_quests[i];
        else if (entry.quest_id == QUEST_ORDER_DOMINION)
            dominion_entry = &snapshot.main_quests[i];
        else if (entry.quest_id == QUEST_REBELLION_FLEET)
            rebellion_entry = &snapshot.main_quests[i];
    }

    FT_ASSERT(verdict_entry != ft_nullptr);
    FT_ASSERT_EQ(QUEST_STATUS_AWAITING_CHOICE, verdict_entry->status);
    FT_ASSERT(verdict_entry->requires_choice);
    FT_ASSERT(verdict_entry->awaiting_choice);
    FT_ASSERT(verdict_entry->choices.size() == 2);
    bool has_execution_option = false;
    bool has_trial_option = false;
    for (size_t i = 0; i < verdict_entry->choices.size(); ++i)
    {
        const Game::ft_quest_choice_snapshot &choice = verdict_entry->choices[i];
        FT_ASSERT(choice.is_available);
        if (ft_strstr(choice.description.c_str(), "Purge the captured rebels") != ft_nullptr)
            has_execution_option = true;
        if (ft_strstr(choice.description.c_str(), "promise reform") != ft_nullptr)
            has_trial_option = true;
    }
    FT_ASSERT(has_execution_option);
    FT_ASSERT(has_trial_option);
    FT_ASSERT_EQ(QUEST_CHOICE_NONE, verdict_entry->selected_choice);

    bool listed_as_pending = false;
    for (size_t i = 0; i < snapshot.awaiting_choice_ids.size(); ++i)
    {
        if (snapshot.awaiting_choice_ids[i] == QUEST_ORDER_FINAL_VERDICT)
        {
            listed_as_pending = true;
            break;
        }
    }
    FT_ASSERT(listed_as_pending);

    FT_ASSERT(dominion_entry != ft_nullptr);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, dominion_entry->status);
    FT_ASSERT(dominion_entry->objectives_completed);

    FT_ASSERT(rebellion_entry != ft_nullptr);
    FT_ASSERT_EQ(QUEST_STATUS_LOCKED, rebellion_entry->status);
    FT_ASSERT(!rebellion_entry->branch_requirement_met);

    FT_ASSERT(snapshot.recent_journal_entries.size() >= 1);
    FT_ASSERT(snapshot.recent_journal_entries.size() <= 5);
    bool found_final_mandate = false;
    for (size_t i = 0; i < snapshot.recent_journal_entries.size(); ++i)
    {
        if (ft_strstr(snapshot.recent_journal_entries[i].c_str(), "Final Order Mandate") != ft_nullptr)
        {
            found_final_mandate = true;
            break;
        }
    }
    FT_ASSERT(found_final_mandate);

    for (size_t i = 0; i < snapshot.main_quests.size(); ++i)
    {
        const Game::ft_quest_log_entry &entry = snapshot.main_quests[i];
        if (entry.time_limit > 0.0)
        {
            FT_ASSERT(entry.time_remaining_ratio >= 0.0);
            FT_ASSERT(entry.time_remaining_ratio <= 1.0);
        }
    }
    for (size_t i = 0; i < snapshot.side_quests.size(); ++i)
    {
        const Game::ft_quest_log_entry &entry = snapshot.side_quests[i];
        if (entry.time_limit > 0.0)
        {
            FT_ASSERT(entry.time_remaining_ratio >= 0.0);
            FT_ASSERT(entry.time_remaining_ratio <= 1.0);
        }
    }

    FT_ASSERT(snapshot.recent_lore_entries.size() <= 5);
    if (snapshot.recent_lore_entries.size() > 0)
        FT_ASSERT(snapshot.recent_lore_entries.back().size() > 0);

    return 1;
}

int verify_quick_quest_completion_bonus()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    double initial_scale = game.get_effective_quest_time_scale();
    FT_ASSERT(initial_scale > 0.99 && initial_scale < 1.01);

    game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    game.tick(0.0);

    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_INITIAL_SKIRMISHES));
    FT_ASSERT_EQ(QUEST_DEFENSE_OF_TERRA, game.get_active_quest());

    int parts_before = game.get_ore(PLANET_TERRA, ITEM_ENGINE_PART);
    size_t lore_before = game.get_lore_log().size();

    game.create_fleet(1);
    int flagship = game.create_ship(1, SHIP_CAPITAL);
    FT_ASSERT(flagship != 0);
    game.set_ship_hp(1, flagship, 140);
    game.create_fleet(2);
    int shield = game.create_ship(2, SHIP_SHIELD);
    FT_ASSERT(shield != 0);
    game.set_ship_hp(2, shield, 90);

    game.tick(0.0);

    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_DEFENSE_OF_TERRA));

    int parts_after = game.get_ore(PLANET_TERRA, ITEM_ENGINE_PART);
    FT_ASSERT(parts_after >= parts_before + 1);

    const ft_vector<ft_string> &lore_after = game.get_lore_log();
    FT_ASSERT(lore_after.size() >= lore_before + 2);
    const ft_string &quick_entry = lore_after.back();
    FT_ASSERT(ft_strstr(quick_entry.c_str(), "praises the swift resolution") != ft_nullptr);

    double scaled = game.get_effective_quest_time_scale();
    FT_ASSERT(scaled < initial_scale);
    FT_ASSERT(scaled > 0.94);

    return 1;
}

int verify_resource_dashboard_overview()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    auto stock_resource = [&](int planet_id, int resource_id, int amount) {
        game.ensure_planet_item_slot(planet_id, resource_id);
        game.set_ore(planet_id, resource_id, amount);
    };

    stock_resource(PLANET_TERRA, ORE_IRON, 140);
    stock_resource(PLANET_TERRA, ORE_COPPER, 80);
    stock_resource(PLANET_TERRA, ORE_COAL, 40);

    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    game.tick(40.0);

    stock_resource(PLANET_MARS, ORE_IRON, 40);
    stock_resource(PLANET_MARS, ORE_COPPER, 20);
    stock_resource(PLANET_MARS, ORE_COAL, 20);

    int dispatched = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ORE_IRON, 20);
    FT_ASSERT_EQ(20, dispatched);

    Game::ft_resource_dashboard dashboard;
    game.get_resource_dashboard(dashboard);

    FT_ASSERT_EQ(game.get_active_convoy_count(), dashboard.total_active_convoys);
    FT_ASSERT(dashboard.total_active_convoys >= 1);
    FT_ASSERT(dashboard.routes.size() >= 1);

    const Game::ft_route_dashboard_entry *route_summary = ft_nullptr;
    for (size_t i = 0; i < dashboard.routes.size(); ++i)
    {
        const Game::ft_route_dashboard_entry &entry = dashboard.routes[i];
        if (entry.origin_planet_id == PLANET_TERRA && entry.destination_planet_id == PLANET_MARS)
        {
            route_summary = &entry;
            break;
        }
    }
    FT_ASSERT(route_summary != ft_nullptr);
    FT_ASSERT(route_summary->route_id != 0);
    FT_ASSERT(route_summary->active_convoys >= 1);
    FT_ASSERT(route_summary->threat_level >= 0.0);
    FT_ASSERT(route_summary->quiet_timer >= 0.0);

    bool found_iron = false;
    bool found_copper = false;
    for (size_t i = 0; i < dashboard.resources.size(); ++i)
    {
        const Game::ft_resource_dashboard_entry &summary = dashboard.resources[i];
        if (summary.resource_id == ORE_IRON)
        {
            found_iron = true;
            FT_ASSERT_EQ(160, summary.total_stock);
            double delta = summary.production_rate - 0.6;
            if (delta < 0.0)
                delta = -delta;
            FT_ASSERT(delta < 0.0001);
        }
        else if (summary.resource_id == ORE_COPPER)
        {
            found_copper = true;
            FT_ASSERT_EQ(100, summary.total_stock);
            double delta = summary.production_rate - 0.6;
            if (delta < 0.0)
                delta = -delta;
            FT_ASSERT(delta < 0.0001);
        }
    }
    FT_ASSERT(found_iron);
    FT_ASSERT(found_copper);

    double average_threat_delta = math_fabs(route_summary->threat_level - dashboard.average_route_threat);
    FT_ASSERT(average_threat_delta < 0.0001);
    double max_threat_delta = math_fabs(route_summary->threat_level - dashboard.maximum_route_threat);
    FT_ASSERT(max_threat_delta < 0.0005);

    return 1;
}

int verify_fleet_management_snapshot()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    game.create_fleet(1200);
    int shield_ship = game.create_ship(1200, SHIP_SHIELD);
    FT_ASSERT(shield_ship != 0);
    int radar_ship = game.create_ship(1200, SHIP_RADAR);
    FT_ASSERT(radar_ship != 0);
    game.set_ship_hp(1200, radar_ship, 90);
    game.set_ship_shield(1200, radar_ship, 55);
    game.set_fleet_location_travel(1200, PLANET_TERRA, PLANET_MARS, 14.5);

    game.create_fleet(1201);
    int salvage_ship = game.create_ship(1201, SHIP_SALVAGE);
    FT_ASSERT(salvage_ship != 0);
    game.set_fleet_location_planet(1201, PLANET_TERRA);

    Game::ft_fleet_management_snapshot snapshot;
    game.get_fleet_management_snapshot(snapshot);

    const Game::ft_fleet_management_entry *travel_entry = ft_nullptr;
    for (size_t i = 0; i < snapshot.traveling_fleets.size(); ++i)
    {
        if (snapshot.traveling_fleets[i].fleet_id == 1200)
        {
            travel_entry = &snapshot.traveling_fleets[i];
            break;
        }
    }
    FT_ASSERT(travel_entry != ft_nullptr);
    FT_ASSERT_EQ(LOCATION_TRAVEL, travel_entry->location_type);
    FT_ASSERT_EQ(PLANET_TERRA, travel_entry->origin_planet_id);
    FT_ASSERT_EQ(PLANET_MARS, travel_entry->destination_planet_id);
    FT_ASSERT(!travel_entry->is_garrison);
    FT_ASSERT_EQ(2, travel_entry->ship_count);
    FT_ASSERT_EQ(2, travel_entry->ships.size());
    double range_delta = travel_entry->average_weapon_range - 292.5;
    if (range_delta < 0.0)
        range_delta = -range_delta;
    FT_ASSERT(range_delta < 0.0001);
    double cadence_delta = travel_entry->average_attack_speed - 0.975;
    if (cadence_delta < 0.0)
        cadence_delta = -cadence_delta;
    FT_ASSERT(cadence_delta < 0.0001);

    bool saw_shield = false;
    bool saw_radar = false;
    for (size_t i = 0; i < travel_entry->ships.size(); ++i)
    {
        const Game::ft_ship_management_entry &ship = travel_entry->ships[i];
        if (ship.ship_type == SHIP_SHIELD)
        {
            saw_shield = true;
            FT_ASSERT_EQ(120, ship.hp);
            FT_ASSERT_EQ(150, ship.max_shield);
            double shield_speed_delta = ship.attack_speed - 0.85;
            if (shield_speed_delta < 0.0)
                shield_speed_delta = -shield_speed_delta;
            FT_ASSERT(shield_speed_delta < 0.0001);
        }
        else if (ship.ship_type == SHIP_RADAR)
        {
            saw_radar = true;
            FT_ASSERT_EQ(90, ship.hp);
            FT_ASSERT_EQ(55, ship.shield);
            double radar_speed_delta = ship.attack_speed - 1.1;
            if (radar_speed_delta < 0.0)
                radar_speed_delta = -radar_speed_delta;
            FT_ASSERT(radar_speed_delta < 0.0001);
            double radar_range_delta = ship.max_range - 320.0;
            if (radar_range_delta < 0.0)
                radar_range_delta = -radar_range_delta;
            FT_ASSERT(radar_range_delta < 0.0001);
        }
    }
    FT_ASSERT(saw_shield);
    FT_ASSERT(saw_radar);

    const Game::ft_fleet_management_entry *station_entry = ft_nullptr;
    for (size_t i = 0; i < snapshot.player_fleets.size(); ++i)
    {
        if (snapshot.player_fleets[i].fleet_id == 1201)
        {
            station_entry = &snapshot.player_fleets[i];
            break;
        }
    }
    FT_ASSERT(station_entry != ft_nullptr);
    FT_ASSERT_EQ(LOCATION_PLANET, station_entry->location_type);
    FT_ASSERT_EQ(1, station_entry->ship_count);
    FT_ASSERT_EQ(1, station_entry->ships.size());
    double salvage_speed_delta = station_entry->average_attack_speed - 0.65;
    if (salvage_speed_delta < 0.0)
        salvage_speed_delta = -salvage_speed_delta;
    FT_ASSERT(salvage_speed_delta < 0.0001);
    const Game::ft_ship_management_entry &salvage_summary = station_entry->ships[0];
    FT_ASSERT_EQ(SHIP_SALVAGE, salvage_summary.ship_type);
    FT_ASSERT_EQ(0, station_entry->station_planet_id);

    return 1;
}

int verify_building_layout_snapshot()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    FT_ASSERT(game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    FT_ASSERT(game.place_building(PLANET_TERRA, BUILDING_SMELTER, 0, 2) != 0);

    Game::ft_building_layout_snapshot snapshot;
    game.get_building_layout_snapshot(snapshot);

    FT_ASSERT(snapshot.planets.size() >= 1);
    const Game::ft_planet_building_snapshot *terra_snapshot = ft_nullptr;
    for (size_t i = 0; i < snapshot.planets.size(); ++i)
    {
        if (snapshot.planets[i].planet_id == PLANET_TERRA)
        {
            terra_snapshot = &snapshot.planets[i];
            break;
        }
    }
    FT_ASSERT(terra_snapshot != ft_nullptr);
    FT_ASSERT_EQ(PLANET_TERRA, terra_snapshot->planet_id);
    FT_ASSERT(ft_strcmp(terra_snapshot->planet_name.c_str(), "Terra") == 0);
    FT_ASSERT_EQ(4, terra_snapshot->width);
    FT_ASSERT_EQ(4, terra_snapshot->height);
    FT_ASSERT_EQ(9, terra_snapshot->used_plots);
    FT_ASSERT_EQ(1, terra_snapshot->base_logistic);
    FT_ASSERT_EQ(1, terra_snapshot->logistic_capacity);
    FT_ASSERT_EQ(0, terra_snapshot->logistic_usage);
    FT_ASSERT_EQ(1, terra_snapshot->logistic_available);
    FT_ASSERT_EQ(0, terra_snapshot->research_logistic_bonus);
    double generator_delta = terra_snapshot->energy_generation - 6.0;
    if (generator_delta < 0.0)
        generator_delta = -generator_delta;
    FT_ASSERT(generator_delta < 0.0001);
    FT_ASSERT(terra_snapshot->energy_consumption < 0.0001);
    FT_ASSERT(terra_snapshot->support_energy < 0.0001);
    FT_ASSERT(terra_snapshot->energy_deficit_pressure < 0.0001);
    FT_ASSERT(terra_snapshot->mine_multiplier >= 1.0);
    FT_ASSERT(terra_snapshot->convoy_speed_bonus < 0.0001);
    FT_ASSERT(terra_snapshot->convoy_raid_risk_modifier < 0.0001);
    FT_ASSERT(!terra_snapshot->emergency_energy_conservation);

    FT_ASSERT_EQ(static_cast<size_t>(terra_snapshot->width * terra_snapshot->height),
                 terra_snapshot->grid.size());

    const Game::ft_building_instance_snapshot *mine_instance = ft_nullptr;
    const Game::ft_building_instance_snapshot *generator_instance = ft_nullptr;
    const Game::ft_building_instance_snapshot *smelter_instance = ft_nullptr;
    for (size_t i = 0; i < terra_snapshot->instances.size(); ++i)
    {
        const Game::ft_building_instance_snapshot &instance = terra_snapshot->instances[i];
        if (instance.building_id == BUILDING_MINE_CORE)
            mine_instance = &terra_snapshot->instances[i];
        else if (instance.building_id == BUILDING_POWER_GENERATOR)
            generator_instance = &terra_snapshot->instances[i];
        else if (instance.building_id == BUILDING_SMELTER)
            smelter_instance = &terra_snapshot->instances[i];
    }
    FT_ASSERT(mine_instance != ft_nullptr);
    FT_ASSERT(generator_instance != ft_nullptr);
    FT_ASSERT(smelter_instance != ft_nullptr);
    FT_ASSERT(!mine_instance->removable);
    FT_ASSERT(mine_instance->occupies_grid);
    FT_ASSERT(generator_instance->occupies_grid);
    FT_ASSERT(smelter_instance->occupies_grid);
    FT_ASSERT_EQ(2, generator_instance->width);
    FT_ASSERT_EQ(2, generator_instance->height);
    double generator_energy_delta = generator_instance->energy_gain - 6.0;
    if (generator_energy_delta < 0.0)
        generator_energy_delta = -generator_energy_delta;
    FT_ASSERT(generator_energy_delta < 0.0001);
    FT_ASSERT_EQ(0, generator_instance->logistic_cost);
    FT_ASSERT_EQ(0, generator_instance->logistic_gain);
    double smelter_cost_delta = smelter_instance->energy_cost - 2.0;
    if (smelter_cost_delta < 0.0)
        smelter_cost_delta = -smelter_cost_delta;
    FT_ASSERT(smelter_cost_delta < 0.0001);
    FT_ASSERT_EQ(1, smelter_instance->logistic_cost);
    FT_ASSERT_EQ(0, smelter_instance->logistic_gain);
    FT_ASSERT_EQ(1U, smelter_instance->inputs.size());
    FT_ASSERT_EQ(ORE_IRON, smelter_instance->inputs[0].key);
    FT_ASSERT_EQ(3, smelter_instance->inputs[0].value);
    FT_ASSERT_EQ(1U, smelter_instance->outputs.size());
    FT_ASSERT_EQ(ITEM_IRON_BAR, smelter_instance->outputs[0].key);
    FT_ASSERT_EQ(2, smelter_instance->outputs[0].value);

    bool saw_mine_cell = false;
    bool saw_generator_cell = false;
    bool saw_smelter_cell = false;
    for (size_t i = 0; i < terra_snapshot->grid.size(); ++i)
    {
        const Game::ft_building_grid_cell_snapshot &cell = terra_snapshot->grid[i];
        if (cell.x == mine_instance->x && cell.y == mine_instance->y)
        {
            saw_mine_cell = true;
            FT_ASSERT_EQ(mine_instance->instance_id, cell.instance_id);
            FT_ASSERT_EQ(BUILDING_MINE_CORE, cell.building_id);
        }
        else if (cell.x == generator_instance->x && cell.y == generator_instance->y)
        {
            saw_generator_cell = true;
            FT_ASSERT_EQ(generator_instance->instance_id, cell.instance_id);
            FT_ASSERT_EQ(BUILDING_POWER_GENERATOR, cell.building_id);
        }
        else if (cell.x == smelter_instance->x && cell.y == smelter_instance->y)
        {
            saw_smelter_cell = true;
            FT_ASSERT_EQ(smelter_instance->instance_id, cell.instance_id);
            FT_ASSERT_EQ(BUILDING_SMELTER, cell.building_id);
        }
    }
    FT_ASSERT(saw_mine_cell);
    FT_ASSERT(saw_generator_cell);
    FT_ASSERT(saw_smelter_cell);

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

    FT_ASSERT_EQ(fleet_id, game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS));
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
    game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);

    size_t lore_start = game.get_lore_log().size();
    int mars_second_start = game.get_ore(PLANET_MARS, ITEM_IRON_BAR);
    int dispatched_second = game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 50);
    FT_ASSERT_EQ(50, dispatched_second);
    FT_ASSERT_EQ(0, game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS));
    const ft_vector<ft_string> &log = game.get_lore_log();
    FT_ASSERT(log.size() > lore_start);
    ft_string escort_fragment("Escort fleet #");
    escort_fragment.append(ft_to_string(fleet_id));
    bool found_escort_entry = false;
    for (size_t i = lore_start; i < log.size(); ++i)
    {
        if (ft_strstr(log[i].c_str(), escort_fragment.c_str()) != ft_nullptr)
        {
            found_escort_entry = true;
            break;
        }
    }
    FT_ASSERT(found_escort_entry);

    double elapsed_second = 0.0;
    bool escort_moved = false;
    while (game.get_ore(PLANET_MARS, ITEM_IRON_BAR) == mars_second_start && elapsed_second < 600.0)
    {
        game.tick(0.5);
        elapsed_second += 0.5;
        if (!escort_moved && elapsed_second >= 1.0)
        {
            game.set_fleet_location_travel(fleet_id, PLANET_MARS, PLANET_VULCAN, 10.0);
            escort_moved = true;
        }
    }
    FT_ASSERT(elapsed_second < 600.0);
    double cleanup_second = elapsed_second;
    while (game.get_active_convoy_count() > 0 && cleanup_second < 600.0)
    {
        game.tick(0.5);
        cleanup_second += 0.5;
    }
    FT_ASSERT_EQ(0, game.get_active_convoy_count());
    FT_ASSERT_EQ(0, game.get_assigned_convoy_escort(PLANET_TERRA, PLANET_MARS));

    return 1;
}

int verify_convoy_escort_rating_excludes_active_escort()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    int baseline_rating = game.calculate_planet_escort_rating(PLANET_TERRA);

    const int fleet_id = 9405;
    game.create_fleet(fleet_id);
    FT_ASSERT(game.create_ship(fleet_id, SHIP_SHIELD) != 0);
    game.set_fleet_location_planet(fleet_id, PLANET_TERRA);

    ft_sharedptr<ft_fleet> escort = game.get_fleet(fleet_id);
    FT_ASSERT(escort);

    int fleet_rating = game.calculate_fleet_escort_rating(*escort);
    FT_ASSERT(fleet_rating > 0);

    int combined_rating = game.calculate_planet_escort_rating(PLANET_TERRA);
    FT_ASSERT_EQ(baseline_rating + fleet_rating, combined_rating);

    Game::ft_supply_route *route = game.ensure_supply_route(PLANET_TERRA, PLANET_MARS);
    FT_ASSERT(route != ft_nullptr);

    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
    game.ensure_planet_item_slot(PLANET_MARS, ITEM_IRON_BAR);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 50);
    game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);

    int convoy_id = game.dispatch_convoy(*route, PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 20, 0, fleet_id);
    FT_ASSERT(convoy_id > 0);
    FT_ASSERT_EQ(1, game.get_active_convoy_count());

    ft_location traveling = game.get_fleet_location(fleet_id);
    FT_ASSERT_EQ(LOCATION_TRAVEL, traveling.type);
    FT_ASSERT_EQ(PLANET_TERRA, traveling.from);
    FT_ASSERT_EQ(PLANET_MARS, traveling.to);

    int rating_during_escort = game.calculate_planet_escort_rating(PLANET_TERRA);
    FT_ASSERT_EQ(baseline_rating, rating_during_escort);

    game.tick(600.0);

    FT_ASSERT_EQ(0, game.get_active_convoy_count());

    ft_location arrived = game.get_fleet_location(fleet_id);
    FT_ASSERT_EQ(LOCATION_PLANET, arrived.type);
    FT_ASSERT_EQ(PLANET_MARS, arrived.from);
    FT_ASSERT_EQ(PLANET_MARS, arrived.to);

    int rating_after = game.calculate_planet_escort_rating(PLANET_TERRA);
    FT_ASSERT_EQ(baseline_rating, rating_after);

    return 1;
}
