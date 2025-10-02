#ifndef GAME_BOOTSTRAP_HPP
#define GAME_BOOTSTRAP_HPP

#include "planets.hpp"
#include "libft/Template/vector.hpp"
#include "libft/Libft/libft.hpp"

struct GameBootstrapResource
{
    int resource_id;
    int amount;

    GameBootstrapResource() noexcept : resource_id(0), amount(0) {}
    GameBootstrapResource(int id, int value) noexcept : resource_id(id), amount(value) {}
};

struct GameBootstrapPlanetResource
{
    int resource_id;
    int amount;
    double rate;

    GameBootstrapPlanetResource() noexcept : resource_id(0), amount(0), rate(0.0) {}
    GameBootstrapPlanetResource(int id, int value, double production_rate) noexcept
        : resource_id(id), amount(value), rate(production_rate)
    {}
};

struct GameBootstrapPlanet
{
    int                                           id;
    ft_vector<GameBootstrapPlanetResource>        resources;

    GameBootstrapPlanet() noexcept : id(0), resources() {}
};

struct GameBootstrapData
{
    ft_string                    commander_name;
    GameBootstrapPlanet          starting_planet;
    ft_vector<GameBootstrapResource> player_resources;

    GameBootstrapData() noexcept : commander_name(), starting_planet(), player_resources() {}
};

bool      game_bootstrap_initialize_default(GameBootstrapData &out_data) noexcept;
bool      game_bootstrap_initialize_with_commander(GameBootstrapData &out_data, const ft_string &commander_name) noexcept;
ft_string game_bootstrap_serialize(const GameBootstrapData &data) noexcept;
bool      game_bootstrap_write_quicksave(const GameBootstrapData &data, const char *file_path) noexcept;
bool      game_bootstrap_create_default_quicksave(const char *file_path) noexcept;
bool      game_bootstrap_create_quicksave_with_commander(const char *file_path, const ft_string &commander_name) noexcept;

#endif
