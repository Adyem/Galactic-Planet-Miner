#include "game_bootstrap.hpp"

#include "libft/CMA/CMA.hpp"
#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/CPP_class/class_ofstream.hpp"
#include "libft/JSon/document.hpp"
#include "libft/JSon/json.hpp"
#include "libft/Libft/limits.hpp"
#include "libft/Libft/libft.hpp"

namespace
{
    const double kBootstrapRateScale = 1000000.0;

    long bootstrap_scale_double(double value) noexcept
    {
        double scaled = value * kBootstrapRateScale;
        if (scaled >= 0.0)
            scaled += 0.5;
        else
            scaled -= 0.5;
        if (scaled > static_cast<double>(FT_LONG_MAX))
            scaled = static_cast<double>(FT_LONG_MAX);
        if (scaled < static_cast<double>(FT_LONG_MIN))
            scaled = static_cast<double>(FT_LONG_MIN);
        return static_cast<long>(scaled);
    }

    ft_string bootstrap_abort(json_document &document) noexcept
    {
        document.clear();
        return ft_string();
    }

    bool bootstrap_append_group(json_document &document, json_group *group) noexcept
    {
        if (group == ft_nullptr)
            return false;
        document.append_group(group);
        return true;
    }

    bool bootstrap_add_item(json_document &document, json_group *group, const char *key, int value) noexcept
    {
        if (group == ft_nullptr || key == ft_nullptr)
            return false;
        json_item *item = document.create_item(key, value);
        if (!item)
            return false;
        document.add_item(group, item);
        return true;
    }

    bool bootstrap_add_item(json_document &document, json_group *group, const char *key, const ft_string &value) noexcept
    {
        if (group == ft_nullptr || key == ft_nullptr)
            return false;
        json_item *item = document.create_item(key, value.c_str());
        if (!item)
            return false;
        document.add_item(group, item);
        return true;
    }

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
    bootstrap_populate_planet_defaults(out_data);
    bootstrap_populate_player_inventory(out_data);
    return true;
}

bool game_bootstrap_initialize_default(GameBootstrapData &out_data) noexcept
{
    const ft_string default_name("Commander");
    return game_bootstrap_initialize_with_commander(out_data, default_name);
}

ft_string game_bootstrap_serialize(const GameBootstrapData &data) noexcept
{
    json_document document;

    json_group *metadata_group = document.create_group("metadata");
    if (!bootstrap_append_group(document, metadata_group))
        return bootstrap_abort(document);
    if (!bootstrap_add_item(document, metadata_group, "version", 1))
        return bootstrap_abort(document);
    const ft_string save_type("quicksave");
    if (!bootstrap_add_item(document, metadata_group, "save_type", save_type))
        return bootstrap_abort(document);

    json_group *player_group = document.create_group("player");
    if (!bootstrap_append_group(document, player_group))
        return bootstrap_abort(document);
    if (!data.commander_name.empty())
    {
        if (!bootstrap_add_item(document, player_group, "commander_name", data.commander_name))
            return bootstrap_abort(document);
    }
    if (!bootstrap_add_item(document, player_group, "starting_planet_id", data.starting_planet.id))
        return bootstrap_abort(document);
    for (size_t index = 0; index < data.player_resources.size(); ++index)
    {
        const GameBootstrapResource &resource = data.player_resources[index];
        ft_string key("resource_");
        key.append(ft_to_string(resource.resource_id));
        if (!bootstrap_add_item(document, player_group, key.c_str(), resource.amount))
            return bootstrap_abort(document);
    }

    ft_string planet_group_name("planet_");
    planet_group_name.append(ft_to_string(data.starting_planet.id));
    json_group *planet_group = document.create_group(planet_group_name.c_str());
    if (!bootstrap_append_group(document, planet_group))
        return bootstrap_abort(document);
    if (!bootstrap_add_item(document, planet_group, "id", data.starting_planet.id))
        return bootstrap_abort(document);
    for (size_t index = 0; index < data.starting_planet.resources.size(); ++index)
    {
        const GameBootstrapPlanetResource &resource = data.starting_planet.resources[index];
        ft_string amount_key("resource_");
        amount_key.append(ft_to_string(resource.resource_id));
        if (!bootstrap_add_item(document, planet_group, amount_key.c_str(), resource.amount))
            return bootstrap_abort(document);
        ft_string rate_key("rate_");
        rate_key.append(ft_to_string(resource.resource_id));
        const long scaled_rate = bootstrap_scale_double(resource.rate);
        ft_string rate_value = ft_to_string(scaled_rate);
        if (!bootstrap_add_item(document, planet_group, rate_key.c_str(), rate_value))
            return bootstrap_abort(document);
    }

    char *serialized = document.write_to_string();
    if (serialized == ft_nullptr)
        return bootstrap_abort(document);
    ft_string result(serialized);
    cma_free(serialized);
    return result;
}

bool game_bootstrap_write_quicksave(const GameBootstrapData &data, const char *file_path) noexcept
{
    if (file_path == ft_nullptr)
        return false;
    const ft_string serialized = game_bootstrap_serialize(data);
    if (serialized.empty())
        return false;

    ft_ofstream stream;
    if (stream.open(file_path) != 0)
        return false;
    if (stream.write(serialized.c_str()) < 0)
    {
        stream.close();
        return false;
    }
    stream.write("\n");
    stream.close();
    return true;
}

bool game_bootstrap_create_quicksave_with_commander(const char *file_path, const ft_string &commander_name) noexcept
{
    GameBootstrapData data;
    if (!game_bootstrap_initialize_with_commander(data, commander_name))
        return false;
    return game_bootstrap_write_quicksave(data, file_path);
}

bool game_bootstrap_create_default_quicksave(const char *file_path) noexcept
{
    const ft_string default_name("Commander");
    return game_bootstrap_create_quicksave_with_commander(file_path, default_name);
}
