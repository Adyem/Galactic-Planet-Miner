#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "backend_client.hpp"
#include "game_test_scenarios.hpp"
#include "planets.hpp"

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
