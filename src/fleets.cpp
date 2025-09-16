#include "fleets.hpp"
#include "../libft/Template/vector.hpp"

int ft_fleet::_next_ship_id = 1;

ft_fleet::ft_fleet() noexcept : _id(0), _travel_time(0)
{
    return ;
}

ft_fleet::ft_fleet(int id) noexcept : _id(id), _travel_time(0)
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

int ft_fleet::get_ship_count() const noexcept
{
    return static_cast<int>(this->_ships.size());
}

int ft_fleet::get_total_ship_hp() const noexcept
{
    size_t count = this->_ships.size();
    if (count == 0)
        return 0;
    const Pair<int, ft_ship> *entries = this->_ships.end();
    entries -= count;
    int total = 0;
    for (size_t i = 0; i < count; ++i)
        total += entries[i].value.hp;
    return total;
}

int ft_fleet::get_total_ship_shield() const noexcept
{
    size_t count = this->_ships.size();
    if (count == 0)
        return 0;
    const Pair<int, ft_ship> *entries = this->_ships.end();
    entries -= count;
    int total = 0;
    for (size_t i = 0; i < count; ++i)
        total += entries[i].value.shield;
    return total;
}

double ft_fleet::get_attack_power() const noexcept
{
    size_t count = this->_ships.size();
    if (count == 0)
        return 0.0;
    const Pair<int, ft_ship> *entries = this->_ships.end();
    entries -= count;
    double total = 0.0;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_ship &ship = entries[i].value;
        if (ship.hp <= 0)
            continue;
        double base = 4.0;
        if (ship.type == SHIP_SHIELD)
            base = 5.0;
        else if (ship.type == SHIP_RADAR)
            base = 6.0;
        else if (ship.type == SHIP_SALVAGE)
            base = 9.0;
        else if (ship.type == SHIP_CAPITAL)
            base = 15.0;
        double hp_value = static_cast<double>(ship.hp);
        if (hp_value > 100.0)
            hp_value = 100.0;
        double efficiency = hp_value / 100.0;
        if (efficiency < 0.1)
            efficiency = 0.1;
        total += base * efficiency;
    }
    return total;
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

bool ft_fleet::move_ship_to(ft_fleet &target, int ship_uid) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return false;
    target._ships.insert(ship_uid, *ship);
    this->_ships.remove(ship_uid);
    return true;
}

void ft_fleet::move_ships_to(ft_fleet &target) noexcept
{
    while (this->_ships.size() > 0)
    {
        Pair<int, ft_ship> *entry = this->_ships.end();
        entry -= 1;
        target._ships.insert(entry->key, entry->value);
        this->_ships.remove(entry->key);
    }
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

double ft_fleet::absorb_damage(double damage) noexcept
{
    if (damage <= 0.0)
        return 0.0;
    int total_damage = static_cast<int>(damage + 0.5);
    if (total_damage <= 0)
        return 0.0;
    ft_vector<ft_ship*> active;
    size_t count = this->_ships.size();
    Pair<int, ft_ship> *entries = this->_ships.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        ft_ship &ship = entries[i].value;
        if (ship.hp > 0 || ship.shield > 0)
            active.push_back(&ship);
    }
    if (active.size() == 0)
        return static_cast<double>(total_damage);
    for (size_t i = 0; i < active.size() && total_damage > 0; ++i)
    {
        ft_ship *ship = active[i];
        size_t defenders_left = active.size() - i;
        int share = total_damage / static_cast<int>(defenders_left);
        if (share <= 0)
            share = 1;
        if (share > total_damage)
            share = total_damage;
        total_damage -= share;
        int remaining = share;
        if (ship->shield >= remaining)
        {
            ship->shield -= remaining;
            remaining = 0;
        }
        else
        {
            remaining -= ship->shield;
            ship->shield = 0;
        }
        if (remaining <= 0)
            continue;
        double reduction = static_cast<double>(ship->armor) * 0.01;
        if (reduction > 0.6)
            reduction = 0.6;
        double hull_damage_d = static_cast<double>(remaining) * (1.0 - reduction);
        int hull_damage = static_cast<int>(hull_damage_d + 0.5);
        if (hull_damage <= 0 && ship->hp > 0)
            hull_damage = 1;
        if (hull_damage > ship->hp)
            hull_damage = ship->hp;
        ship->hp -= hull_damage;
    }
    return static_cast<double>(total_damage);
}

void ft_fleet::apply_support(int shield_amount, int repair_amount) noexcept
{
    if (shield_amount <= 0 && repair_amount <= 0)
        return ;
    ft_vector<ft_ship*> shield_targets;
    ft_vector<ft_ship*> repair_targets;
    size_t count = this->_ships.size();
    Pair<int, ft_ship> *entries = this->_ships.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        ft_ship &ship = entries[i].value;
        if (ship.hp > 0 || ship.shield > 0)
            shield_targets.push_back(&ship);
        if (ship.hp > 0)
            repair_targets.push_back(&ship);
    }
    int shield_remaining = shield_amount;
    for (size_t i = 0; i < shield_targets.size() && shield_remaining > 0; ++i)
    {
        ft_ship *ship = shield_targets[i];
        size_t remain = shield_targets.size() - i;
        int share = shield_remaining / static_cast<int>(remain);
        if (share <= 0)
            share = 1;
        if (share > shield_remaining)
            share = shield_remaining;
        ship->shield += share;
        shield_remaining -= share;
    }
    int repair_remaining = repair_amount;
    for (size_t i = 0; i < repair_targets.size() && repair_remaining > 0; ++i)
    {
        ft_ship *ship = repair_targets[i];
        size_t remain = repair_targets.size() - i;
        int share = repair_remaining / static_cast<int>(remain);
        if (share <= 0)
            share = 1;
        if (share > repair_remaining)
            share = repair_remaining;
        ship->hp += share;
        repair_remaining -= share;
    }
}

bool ft_fleet::has_operational_ships() const noexcept
{
    size_t count = this->_ships.size();
    if (count == 0)
        return false;
    const Pair<int, ft_ship> *entries = this->_ships.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_ship &ship = entries[i].value;
        if (ship.hp > 0 || ship.shield > 0)
            return true;
    }
    return false;
}

void ft_fleet::set_location_planet(int planet_id) noexcept
{
    this->_loc.type = LOCATION_PLANET;
    this->_loc.from = planet_id;
    this->_loc.to = planet_id;
    this->_loc.misc = 0;
    this->_travel_time = 0;
}

void ft_fleet::set_location_travel(int from, int to, double time) noexcept
{
    this->_loc.type = LOCATION_TRAVEL;
    this->_loc.from = from;
    this->_loc.to = to;
    this->_loc.misc = 0;
    this->_travel_time = time;
}

void ft_fleet::set_location_misc(int misc_id) noexcept
{
    this->_loc.type = LOCATION_MISC;
    this->_loc.from = 0;
    this->_loc.to = 0;
    this->_loc.misc = misc_id;
    this->_travel_time = 0;
}

ft_location ft_fleet::get_location() const noexcept
{
    return this->_loc;
}

double ft_fleet::get_travel_time() const noexcept
{
    return this->_travel_time;
}

void ft_fleet::tick(double seconds) noexcept
{
    if (this->_loc.type == LOCATION_TRAVEL)
    {
        if (this->_travel_time > seconds)
            this->_travel_time -= seconds;
        else
        {
            this->set_location_planet(this->_loc.to);
        }
    }
}

