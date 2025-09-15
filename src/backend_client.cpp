#include "backend_client.hpp"

BackendClient::BackendClient(const ft_string &host, const ft_string &path)
    : _host(host), _path(path)
{
    return ;
}

BackendClient::~BackendClient()
{
    return ;
}

int BackendClient::send_state(const ft_string &state, ft_string &response)
{
    return (http_post(this->_host.c_str(), this->_path.c_str(), state, response, false));
}
