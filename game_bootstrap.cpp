#include "game_bootstrap.hpp"

#include "game.hpp"

#include "libft/Libft/libft.hpp"

namespace
{
    void bootstrap_populate_planet_defaults(GameBootstrapData &out_data) noexcept
    {
        ft_planet_terra terra;

        out_data.starting_planet.id = PLANET_TERRA;
        out_data.starting_planet.resources.clear();

        GameBootstrapPlanetResource iron(ORE_IRON, 200, terra.get_rate(ORE_IRON));
        GameBootstrapPlanetResource copper(ORE_COPPER, 120, terra.get_rate(ORE_COPPER));
        GameBootstrapPlanetResource coal(ORE_COAL, 80, terra.get_rate(ORE_COAL));

        out_data.starting_planet.resources.push_back(iron);
        out_data.starting_planet.resources.push_back(copper);
        out_data.starting_planet.resources.push_back(coal);
    }

    void bootstrap_populate_player_inventory(GameBootstrapData &out_data) noexcept
    {
        out_data.player_resources.clear();

        GameBootstrapResource iron(ORE_IRON, 60);
        GameBootstrapResource copper(ORE_COPPER, 40);
        GameBootstrapResource coal(ORE_COAL, 20);

        out_data.player_resources.push_back(iron);
        out_data.player_resources.push_back(copper);
        out_data.player_resources.push_back(coal);
    }
}

bool game_bootstrap_initialize_with_commander(GameBootstrapData &out_data, const ft_string &commander_name) noexcept
{
    out_data.commander_name = commander_name;
    if (out_data.commander_name.empty())
        out_data.commander_name = ft_string("Commander");
    out_data.campaign_day = 1;
    out_data.commander_level = 1;
    out_data.difficulty_setting = GAME_DIFFICULTY_STANDARD;
    out_data.save_type = ft_string("quicksave");
    bootstrap_populate_planet_defaults(out_data);
    bootstrap_populate_player_inventory(out_data);
    return true;
}

bool game_bootstrap_initialize_default(GameBootstrapData &out_data) noexcept
{
    const ft_string default_name("Commander");
    return game_bootstrap_initialize_with_commander(out_data, default_name);
}

bool game_bootstrap_create_quicksave_with_commander(const char *file_path, const ft_string &commander_name) noexcept
{
    GameBootstrapData data;
    if (!game_bootstrap_initialize_with_commander(data, commander_name))
        return false;
    return game_bootstrap_write_quicksave(data, file_path);
}

bool game_bootstrap_create_tutorial_quicksave(const char *file_path, const ft_string &commander_name) noexcept
{
    GameBootstrapData data;
    if (!game_bootstrap_initialize_with_commander(data, commander_name))
        return false;

    data.save_type = ft_string("tutorial");
    data.campaign_day = 1;
    data.commander_level = 1;
    data.difficulty_setting = GAME_DIFFICULTY_EASY;
    data.player_resources.clear();

    GameBootstrapResource iron(ORE_IRON, 25);
    GameBootstrapResource copper(ORE_COPPER, 12);
    GameBootstrapResource coal(ORE_COAL, 6);
    data.player_resources.push_back(iron);
    data.player_resources.push_back(copper);
    data.player_resources.push_back(coal);

    return game_bootstrap_write_quicksave(data, file_path);
}

bool game_bootstrap_create_default_quicksave(const char *file_path) noexcept
{
    const ft_string default_name("Commander");
    return game_bootstrap_create_quicksave_with_commander(file_path, default_name);
}
