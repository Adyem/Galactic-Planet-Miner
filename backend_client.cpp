#include "backend_client.hpp"
#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/Libft/libft.hpp"

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

    void assign_substring(ft_string &target, const char *begin, const char *end)
    {
        target.clear();
        const char *cursor = begin;
        while (cursor < end)
        {
            target.append(*cursor);
            cursor += 1;
        }
    }

    void extract_http_body(const ft_string &response, ft_string &out_body)
    {
        out_body.clear();
        const char *cstr = response.c_str();
        if (cstr == ft_nullptr)
            return;
        const char *body_start = ft_strstr(cstr, "\r\n\r\n");
        if (body_start != ft_nullptr)
        {
            body_start += 4;
            assign_substring(out_body, body_start, cstr + response.size());
        }
        else
            out_body = response;
    }

    ft_string sanitize_host_input(const ft_string &raw)
    {
        const char *begin = raw.c_str();
        if (begin == ft_nullptr)
            return ft_string();
        const char *trim_start = begin;
        while (*trim_start != '\0' && ft_isspace(*trim_start))
            trim_start += 1;
        const char *trim_end = trim_start + ft_strlen(trim_start);
        while (trim_end > trim_start && ft_isspace(*(trim_end - 1)))
            trim_end -= 1;
        ft_string sanitized;
        assign_substring(sanitized, trim_start, trim_end);
        return sanitized;
    }

    bool is_numeric_range(const char *begin, const char *end)
    {
        if (begin == end)
            return false;
        const char *cursor = begin;
        while (cursor < end)
        {
            if (!ft_isdigit(*cursor))
                return false;
            cursor += 1;
        }
        return true;
    }
}

BackendClient::BackendClient(const ft_string &host, const ft_string &path)
    : _host(host), _path(path), _port(), _use_ssl(false)
{
    ft_string sanitized_input = sanitize_host_input(host);
    const char *input = sanitized_input.c_str();
    const char *scheme_separator = ft_strstr(input, "://");
    const char *authority_start;
    if (scheme_separator != ft_nullptr)
    {
        ft_string scheme_value;
        assign_substring(scheme_value, input, scheme_separator);
        ft_string normalized_scheme(scheme_value);
        ft_to_lower(normalized_scheme.print());
        if (normalized_scheme.size() == 5 && ft_strncmp(normalized_scheme.c_str(), "https", 5) == 0)
            this->_use_ssl = true;
        else if (normalized_scheme.size() == 4 && ft_strncmp(normalized_scheme.c_str(), "http", 4) == 0)
            this->_use_ssl = false;
        authority_start = scheme_separator + 3;
    }
    else
        authority_start = input;

    const char *authority_end = authority_start;
    while (*authority_end != '\0' && *authority_end != '/' && *authority_end != '?' && *authority_end != '#')
        authority_end += 1;

    const char *credentials_separator = ft_strrchr(authority_start, '@');
    if (credentials_separator != ft_nullptr && credentials_separator < authority_end)
        authority_start = credentials_separator + 1;

    if (authority_start >= authority_end)
    {
        assign_substring(this->_host, authority_start, authority_end);
        return ;
    }

    if (*authority_start == '[')
    {
        const char *closing_bracket = ft_strchr(authority_start, ']');
        if (closing_bracket != ft_nullptr && closing_bracket < authority_end)
        {
            assign_substring(this->_host, authority_start, closing_bracket + 1);
            const char *after_bracket = closing_bracket + 1;
            if (after_bracket < authority_end && *after_bracket == ':')
            {
                const char *port_candidate = after_bracket + 1;
                if (is_numeric_range(port_candidate, authority_end))
                    assign_substring(this->_port, port_candidate, authority_end);
            }
            return ;
        }
    }

    int colon_count = 0;
    const char *cursor = authority_start;
    while (cursor < authority_end)
    {
        if (*cursor == ':')
            colon_count += 1;
        cursor += 1;
    }

    if (colon_count == 1)
    {
        const char *last_colon = ft_strrchr(authority_start, ':');
        if (last_colon != ft_nullptr && last_colon < authority_end)
        {
            const char *port_candidate = last_colon + 1;
            if (is_numeric_range(port_candidate, authority_end))
            {
                assign_substring(this->_host, authority_start, last_colon);
                assign_substring(this->_port, port_candidate, authority_end);
                return ;
            }
        }
    }

    assign_substring(this->_host, authority_start, authority_end);
    return ;
}

BackendClient::~BackendClient()
{
    return ;
}

int BackendClient::send_state(const ft_string &state, ft_string &response)
{
    const char *port_cstr = ft_nullptr;
    if (!this->_port.empty())
        port_cstr = this->_port.c_str();

    const int fallback_status = 503;
    int       status = http_post(this->_host.c_str(), this->_path.c_str(), state, response, this->_use_ssl, port_cstr);
    if (status != 0)
    {
        set_offline_echo_response(response, state);
        return (fallback_status);
    }

    int http_status = extract_http_status_code(response);
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

const ft_string &BackendClient::get_host_for_testing() const
{
    return (this->_host);
}

const ft_string &BackendClient::get_port_for_testing() const
{
    return (this->_port);
}

bool BackendClient::get_use_ssl_for_testing() const
{
    return (this->_use_ssl);
}

bool backend_client_ping(const ft_string &host, const ft_string &path, int &out_status_code) noexcept
{
    BackendClient client(host, path);
    ft_string     response;
    ft_string     payload("ping");

    int status = client.send_state(payload, response);
    out_status_code = status;

    if (status >= 200 && status < 400)
        return true;
    return false;
}

bool backend_client_fetch_patch_notes(const ft_string &host, const ft_string &path, ft_string &out_notes,
    int &out_status_code) noexcept
{
    BackendClient client(host, path);
    ft_string     response;
    ft_string     payload("patch_notes");

    int status = client.send_state(payload, response);
    out_status_code = status;

    extract_http_body(response, out_notes);
    if (status >= 200 && status < 300)
        return true;
    return false;
}

bool backend_client_clear_cloud_data(const ft_string &host, const ft_string &path, ft_string &out_body,
    int &out_status_code) noexcept
{
    BackendClient client(host, path);
    ft_string     response;
    ft_string     payload("clear_cloud_data");

    int status = client.send_state(payload, response);
    out_status_code = status;

    extract_http_body(response, out_body);
    if (status >= 200 && status < 300)
        return true;
    return false;
}

bool backend_client_submit_crash_report(const ft_string &host, const ft_string &path, const ft_string &payload,
    ft_string &out_body, int &out_status_code) noexcept
{
    BackendClient client(host, path);
    ft_string     response;
    ft_string     request_payload("crash_report:");
    request_payload.append(payload);

    int status = client.send_state(request_payload, response);
    out_status_code = status;

    extract_http_body(response, out_body);
    if (status >= 200 && status < 300)
        return true;
    return false;
}
