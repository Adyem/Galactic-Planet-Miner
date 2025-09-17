#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "backend_client.hpp"
#include "game_test_scenarios.hpp"

int verify_backend_roundtrip()
{
    BackendClient client(ft_string("127.0.0.1:8080"), ft_string("/"));
    ft_string response;
    client.send_state(ft_string("test"), response);
    const char *resp = response.c_str();
    FT_ASSERT(response.size() >= 4);
    FT_ASSERT_EQ(0, ft_strcmp(resp + response.size() - 4, "test"));
    return 1;
}
