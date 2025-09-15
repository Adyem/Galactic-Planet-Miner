#ifndef GAME_HPP
#define GAME_HPP

#include "backend_client.hpp"
#include "planets.hpp"
#include "fleets.hpp"
#include "../libft/Game/game_state.hpp"
#include "../libft/Template/map.hpp"

class Game
{
private:
    ft_game_state                                 _state;
    ft_map<int, ft_sharedptr<ft_planet> >         _planets;
    ft_map<int, ft_sharedptr<ft_fleet> >          _fleets;
    BackendClient                                _backend;

    ft_sharedptr<ft_planet> get_planet(int id);
    ft_sharedptr<const ft_planet> get_planet(int id) const;
    ft_sharedptr<ft_fleet> get_fleet(int id);
    ft_sharedptr<const ft_fleet> get_fleet(int id) const;
    void send_state(int planet_id, int ore_id);

public:
    Game(const ft_string &host, const ft_string &path);
    ~Game();

    int add_ore(int planet_id, int ore_id, int amount);
    int sub_ore(int planet_id, int ore_id, int amount);
    int get_ore(int planet_id, int ore_id) const;
    void set_ore(int planet_id, int ore_id, int amount);
    double get_rate(int planet_id, int ore_id) const;
    const ft_vector<Pair<int, double> > &get_planet_resources(int planet_id) const;

    void create_fleet(int fleet_id);
    int create_ship(int fleet_id, int ship_type);
    void remove_ship(int fleet_id, int ship_uid);

    void set_ship_armor(int fleet_id, int ship_uid, int value);
    int get_ship_armor(int fleet_id, int ship_uid) const;
    int add_ship_armor(int fleet_id, int ship_uid, int amount);
    int sub_ship_armor(int fleet_id, int ship_uid, int amount);

    void set_ship_hp(int fleet_id, int ship_uid, int value);
    int get_ship_hp(int fleet_id, int ship_uid) const;
    int add_ship_hp(int fleet_id, int ship_uid, int amount);
    int sub_ship_hp(int fleet_id, int ship_uid, int amount);

    void set_ship_shield(int fleet_id, int ship_uid, int value);
    int get_ship_shield(int fleet_id, int ship_uid) const;
    int add_ship_shield(int fleet_id, int ship_uid, int amount);
    int sub_ship_shield(int fleet_id, int ship_uid, int amount);

    void set_fleet_location_planet(int fleet_id, int planet_id);
    void set_fleet_location_travel(int fleet_id, int from, int to);
    void set_fleet_location_misc(int fleet_id, int misc_id);
    ft_location get_fleet_location(int fleet_id) const;
};

#endif
