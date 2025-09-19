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
    int status = http_post(this->_host.c_str(), this->_path.c_str(), state, response, false);

    if (status != 0 || response.size() == 0)
    {
        ft_string fallback_prefix("[offline] echo=");
        fallback_prefix.append(state);
        response = fallback_prefix;
    }
    return (status);
}
