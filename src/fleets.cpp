#include "fleets.hpp"

int ft_fleet::_next_ship_id = 1;

ft_fleet::ft_fleet(int id) noexcept : _id(id)
{
    return ;
}

ft_ship *ft_fleet::find_ship(int ship_uid) noexcept
{
    Pair<int, ft_ship> *entry = this->_ships.find(ship_uid);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
}

const ft_ship *ft_fleet::find_ship(int ship_uid) const noexcept
{
    const Pair<int, ft_ship> *entry = this->_ships.find(ship_uid);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
}

int ft_fleet::get_id() const noexcept
{
    return this->_id;
}

int ft_fleet::create_ship(int ship_type) noexcept
{
    int uid = _next_ship_id++;
    ft_ship ship(uid, ship_type);
    this->_ships.insert(uid, ship);
    return uid;
}

void ft_fleet::remove_ship(int ship_uid) noexcept
{
    this->_ships.remove(ship_uid);
}

void ft_fleet::set_ship_armor(int ship_uid, int value) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return ;
    ship->armor = value;
}

int ft_fleet::get_ship_armor(int ship_uid) const noexcept
{
    const ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    return ship->armor;
}

int ft_fleet::add_ship_armor(int ship_uid, int amount) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    ship->armor += amount;
    return ship->armor;
}

int ft_fleet::sub_ship_armor(int ship_uid, int amount) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    ship->armor -= amount;
    if (ship->armor < 0)
        ship->armor = 0;
    return ship->armor;
}

void ft_fleet::set_ship_hp(int ship_uid, int value) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return ;
    ship->hp = value;
}

int ft_fleet::get_ship_hp(int ship_uid) const noexcept
{
    const ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    return ship->hp;
}

int ft_fleet::add_ship_hp(int ship_uid, int amount) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    ship->hp += amount;
    return ship->hp;
}

int ft_fleet::sub_ship_hp(int ship_uid, int amount) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    ship->hp -= amount;
    if (ship->hp < 0)
        ship->hp = 0;
    return ship->hp;
}

void ft_fleet::set_ship_shield(int ship_uid, int value) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return ;
    ship->shield = value;
}

int ft_fleet::get_ship_shield(int ship_uid) const noexcept
{
    const ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    return ship->shield;
}

int ft_fleet::add_ship_shield(int ship_uid, int amount) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    ship->shield += amount;
    return ship->shield;
}

int ft_fleet::sub_ship_shield(int ship_uid, int amount) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    ship->shield -= amount;
    if (ship->shield < 0)
        ship->shield = 0;
    return ship->shield;
}

void ft_fleet::set_location_planet(int planet_id) noexcept
{
    this->_loc.type = LOCATION_PLANET;
    this->_loc.from = planet_id;
    this->_loc.to = planet_id;
    this->_loc.misc = 0;
}

void ft_fleet::set_location_travel(int from, int to) noexcept
{
    this->_loc.type = LOCATION_TRAVEL;
    this->_loc.from = from;
    this->_loc.to = to;
    this->_loc.misc = 0;
}

void ft_fleet::set_location_misc(int misc_id) noexcept
{
    this->_loc.type = LOCATION_MISC;
    this->_loc.from = 0;
    this->_loc.to = 0;
    this->_loc.misc = misc_id;
}

ft_location ft_fleet::get_location() const noexcept
{
    return this->_loc;
}

