#include "backend_client.hpp"
#include "../libft/CPP_class/class_nullptr.hpp"
#include "../libft/Libft/libft.hpp"

namespace
{
    void set_offline_echo_response(ft_string &response, const ft_string &state)
    {
        ft_string fallback_prefix("[offline] echo=");
        fallback_prefix.append(state);
        response = fallback_prefix;
    }

    int extract_http_status_code(const ft_string &response)
    {
        const char *cstr = response.c_str();
        const char *http_prefix = "HTTP/";
        const size_t prefix_size = 5;

        if (response.size() < prefix_size || ft_strncmp(cstr, http_prefix, prefix_size) != 0)
            return 0;
        const char *first_space = ft_strchr(cstr, ' ');
        if (first_space == ft_nullptr)
            return 0;
        const char *status_start = first_space + 1;
        const char *status_end = ft_strchr(status_start, ' ');
        size_t token_length;
        if (status_end == ft_nullptr)
            token_length = static_cast<size_t>(ft_strlen(status_start));
        else
            token_length = static_cast<size_t>(status_end - status_start);
        char status_buffer[16];
        if (token_length == 0 || token_length >= sizeof(status_buffer))
            return 0;
        ft_memcpy(status_buffer, status_start, token_length);
        status_buffer[token_length] = '\0';
        return ft_atoi(status_buffer);
    }
}

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
    if (status != 0)
    {
        set_offline_echo_response(response, state);
        return (status);
    }

    int http_status = extract_http_status_code(response);
    const int fallback_status = 503;
    if (http_status >= 200 && http_status < 300)
        return (http_status);

    if (http_status == 0)
    {
        set_offline_echo_response(response, state);
        return (fallback_status);
    }
    if (http_status >= 100)
        return (http_status);
    return (fallback_status);
}
