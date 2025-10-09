#ifndef BACKEND_CLIENT_HPP
#define BACKEND_CLIENT_HPP

#include "libft/Networking/http_client.hpp"

class BackendClient
{
private:
    ft_string _host;
    ft_string _path;
    ft_string _port;
    bool      _use_ssl;

public:
    BackendClient(const ft_string &host, const ft_string &path);
    ~BackendClient();

    int send_state(const ft_string &state, ft_string &response);

    const ft_string &get_host_for_testing() const;
    const ft_string &get_port_for_testing() const;
    bool get_use_ssl_for_testing() const;
};

bool backend_client_ping(const ft_string &host, const ft_string &path, int &out_status_code) noexcept;
bool backend_client_fetch_patch_notes(const ft_string &host, const ft_string &path, ft_string &out_notes,
    int &out_status_code) noexcept;
bool backend_client_clear_cloud_data(const ft_string &host, const ft_string &path, ft_string &out_body,
    int &out_status_code) noexcept;

#endif
