#ifndef BACKEND_CLIENT_HPP
#define BACKEND_CLIENT_HPP

#include "../libft/Networking/http_client.hpp"

class BackendClient
{
private:
    ft_string _host;
    ft_string _path;
    ft_string _port;

public:
    BackendClient(const ft_string &host, const ft_string &path);
    ~BackendClient();

    int send_state(const ft_string &state, ft_string &response);
};

#endif
