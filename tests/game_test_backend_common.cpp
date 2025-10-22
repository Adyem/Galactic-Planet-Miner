#include "../libft/Networking/networking.hpp"

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
        nw_close(client_fd);
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

int verify_backend_patch_notes_fetch()
{
    ft_string notes;
    int       status_code = 0;

    FT_ASSERT(!backend_client_fetch_patch_notes(ft_string("127.0.0.1:65535"), ft_string("/patch-notes/latest"), notes,
        status_code));
    FT_ASSERT(status_code != 0);
    FT_ASSERT(!notes.empty());

    return 1;
}

int verify_backend_clear_cloud_data()
{
    ft_string response_body;
    int       status_code = 0;

    FT_ASSERT(!backend_client_clear_cloud_data(
        ft_string("127.0.0.1:65535"), ft_string("/cloud-data/clear"), response_body, status_code));
    FT_ASSERT(status_code != 0);
    FT_ASSERT(!response_body.empty());

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

