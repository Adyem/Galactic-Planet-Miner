#include "../libft/Networking/http_server.hpp"
#include "../libft/PThread/thread.hpp"
#include "../libft/Time/time.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/pair.hpp"
#include "game_test_scenarios.hpp"

#include "game_test_main_pre_checks.cpp"
#include "game_test_main_campaign.cpp"
#include "game_test_main_post_checks.cpp"

static void run_server()
{
    ft_http_server server;
    server.start("127.0.0.1", 8080, AF_INET, false);
    for (int i = 0; i < 64; ++i)
        server.run_once();
}

int main()
{
    ft_thread server_thread(run_server);
    time_sleep_ms(100);

    if (!run_main_prelaunch_verifications())
        return 0;

    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    if (!run_main_campaign_verifications(game))
        return 0;

    if (!run_main_post_campaign_verifications())
        return 0;

    server_thread.join();
    return 0;
}
