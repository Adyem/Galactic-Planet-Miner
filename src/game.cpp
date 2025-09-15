#include "game.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Template/pair.hpp"

Game::Game(const ft_string &host, const ft_string &path)
    : _backend(host, path)
{
    ft_sharedptr<ft_planet> terra(new ft_planet_terra());
    ft_sharedptr<ft_planet> mars(new ft_planet_mars());
    ft_sharedptr<ft_planet> zalthor(new ft_planet_zalthor());
    ft_sharedptr<ft_planet> vulcan(new ft_planet_vulcan());
    ft_sharedptr<ft_planet> noctaris(new ft_planet_noctaris_prime());

    this->_state.add_character(terra);
    this->_state.add_character(mars);
    this->_state.add_character(zalthor);
    this->_state.add_character(vulcan);
    this->_state.add_character(noctaris);

    this->_planets.insert(PLANET_TERRA, terra);
    this->_planets.insert(PLANET_MARS, mars);
    this->_planets.insert(PLANET_ZALTHOR, zalthor);
    this->_planets.insert(PLANET_VULCAN, vulcan);
    this->_planets.insert(PLANET_NOCTARIS_PRIME, noctaris);
}

Game::~Game()
{
    return ;
}

void Game::produce(double seconds)
{
    int planet_ids[] = {
        PLANET_TERRA,
        PLANET_MARS,
        PLANET_ZALTHOR,
        PLANET_VULCAN,
        PLANET_NOCTARIS_PRIME
    };
    size_t count = sizeof(planet_ids) / sizeof(planet_ids[0]);
    for (size_t i = 0; i < count; ++i)
    {
        int planet_id = planet_ids[i];
        ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
        if (!planet)
            continue;
        ft_vector<Pair<int, int> > produced = planet->produce(seconds);
        for (size_t j = 0; j < produced.size(); ++j)
            this->send_state(planet_id, produced[j].key);
    }
}

ft_sharedptr<ft_planet> Game::get_planet(int id)
{
    Pair<int, ft_sharedptr<ft_planet> > *entry = this->_planets.find(id);
    if (entry == ft_nullptr)
        return ft_sharedptr<ft_planet>();
    return entry->value;
}

ft_sharedptr<const ft_planet> Game::get_planet(int id) const
{
    const Pair<int, ft_sharedptr<ft_planet> > *entry = this->_planets.find(id);
    if (entry == ft_nullptr)
        return ft_sharedptr<const ft_planet>();
    return entry->value;
}

ft_sharedptr<ft_fleet> Game::get_fleet(int id)
{
    Pair<int, ft_sharedptr<ft_fleet> > *entry = this->_fleets.find(id);
    if (entry == ft_nullptr)
        return ft_sharedptr<ft_fleet>();
    return entry->value;
}

ft_sharedptr<const ft_fleet> Game::get_fleet(int id) const
{
    const Pair<int, ft_sharedptr<ft_fleet> > *entry = this->_fleets.find(id);
    if (entry == ft_nullptr)
        return ft_sharedptr<const ft_fleet>();
    return entry->value;
}

void Game::send_state(int planet_id, int ore_id)
{
    ft_sharedptr<const ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return ;
    ft_string body("{\"planet\":");
    body.append(ft_to_string(planet_id));
    body.append(",\"ore\":");
    body.append(ft_to_string(ore_id));
    body.append(",\"amount\":");
    body.append(ft_to_string(planet->get_resource(ore_id)));
    body.append("}");
    ft_string response;
    this->_backend.send_state(body, response);
}

int Game::add_ore(int planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return 0;
    int total = planet->add_resource(ore_id, amount);
    this->send_state(planet_id, ore_id);
    return total;
}

int Game::sub_ore(int planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return 0;
    int total = planet->sub_resource(ore_id, amount);
    this->send_state(planet_id, ore_id);
    return total;
}

int Game::get_ore(int planet_id, int ore_id) const
{
    ft_sharedptr<const ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return 0;
    return planet->get_resource(ore_id);
}

void Game::set_ore(int planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return ;
    planet->set_resource(ore_id, amount);
    this->send_state(planet_id, ore_id);
}

double Game::get_rate(int planet_id, int ore_id) const
{
    ft_sharedptr<const ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return 0;
    return planet->get_rate(ore_id);
}

const ft_vector<Pair<int, double> > &Game::get_planet_resources(int planet_id) const
{
    static ft_vector<Pair<int, double> > empty;
    ft_sharedptr<const ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return empty;
    return planet->get_resources();
}

void Game::create_fleet(int fleet_id)
{
    if (this->_fleets.find(fleet_id) != ft_nullptr)
        return ;
    ft_sharedptr<ft_fleet> fleet(new ft_fleet(fleet_id));
    fleet->set_location_planet(PLANET_TERRA);
    this->_state.add_character(fleet);
    this->_fleets.insert(fleet_id, fleet);
}

int Game::create_ship(int fleet_id, int ship_type)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->create_ship(ship_type);
}

void Game::remove_ship(int fleet_id, int ship_uid)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->remove_ship(ship_uid);
}

void Game::set_ship_armor(int fleet_id, int ship_uid, int value)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_ship_armor(ship_uid, value);
}

int Game::get_ship_armor(int fleet_id, int ship_uid) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->get_ship_armor(ship_uid);
}

int Game::add_ship_armor(int fleet_id, int ship_uid, int amount)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->add_ship_armor(ship_uid, amount);
}

int Game::sub_ship_armor(int fleet_id, int ship_uid, int amount)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->sub_ship_armor(ship_uid, amount);
}

void Game::set_ship_hp(int fleet_id, int ship_uid, int value)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_ship_hp(ship_uid, value);
}

int Game::get_ship_hp(int fleet_id, int ship_uid) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->get_ship_hp(ship_uid);
}

int Game::add_ship_hp(int fleet_id, int ship_uid, int amount)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->add_ship_hp(ship_uid, amount);
}

int Game::sub_ship_hp(int fleet_id, int ship_uid, int amount)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->sub_ship_hp(ship_uid, amount);
}

void Game::set_ship_shield(int fleet_id, int ship_uid, int value)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_ship_shield(ship_uid, value);
}

int Game::get_ship_shield(int fleet_id, int ship_uid) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->get_ship_shield(ship_uid);
}

int Game::add_ship_shield(int fleet_id, int ship_uid, int amount)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->add_ship_shield(ship_uid, amount);
}

int Game::sub_ship_shield(int fleet_id, int ship_uid, int amount)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->sub_ship_shield(ship_uid, amount);
}

void Game::set_fleet_location_planet(int fleet_id, int planet_id)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_location_planet(planet_id);
}

void Game::set_fleet_location_travel(int fleet_id, int from, int to)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_location_travel(from, to);
}

void Game::set_fleet_location_misc(int fleet_id, int misc_id)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_location_misc(misc_id);
}

ft_location Game::get_fleet_location(int fleet_id) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ft_location();
    return fleet->get_location();
}

