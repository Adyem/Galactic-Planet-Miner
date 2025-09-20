#include "fleets.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Libft/limits.hpp"

namespace
{
    struct ship_profile
    {
        int     armor;
        int     hp;
        int     shield;
        double  max_speed;
        double  acceleration;
        double  turn_speed;
        double  optimal_range;
        double  max_range;
        double  base_damage;
        int     combat_behavior;
        int     outnumbered_behavior;
        int     unescorted_behavior;
        int     low_hp_behavior;
        int     role;

        ship_profile()
            : armor(18), hp(180), shield(60), max_speed(18.0), acceleration(4.0),
              turn_speed(60.0), optimal_range(180.0), max_range(240.0),
              base_damage(4.0), combat_behavior(SHIP_BEHAVIOR_LINE_HOLD),
              outnumbered_behavior(SHIP_BEHAVIOR_RETREAT),
              unescorted_behavior(SHIP_BEHAVIOR_WITHDRAW_SUPPORT),
              low_hp_behavior(SHIP_BEHAVIOR_RETREAT), role(SHIP_ROLE_LINE)
        {}
    };

    ship_profile get_ship_profile(int type)
    {
        ship_profile profile;
        switch (type)
        {
        case SHIP_SHIELD:
            profile.armor = 35;
            profile.hp = 180;
            profile.shield = 180;
            profile.max_speed = 21.0;
            profile.acceleration = 5.5;
            profile.turn_speed = 95.0;
            profile.optimal_range = 205.0;
            profile.max_range = 265.0;
            profile.base_damage = 5.5;
            profile.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            profile.unescorted_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            break;
        case SHIP_RADAR:
            profile.armor = 10;
            profile.hp = 120;
            profile.shield = 70;
            profile.max_speed = 26.0;
            profile.acceleration = 6.5;
            profile.turn_speed = 110.0;
            profile.optimal_range = 235.0;
            profile.max_range = 320.0;
            profile.base_damage = 4.5;
            profile.combat_behavior = SHIP_BEHAVIOR_FLANK_SWEEP;
            profile.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            profile.low_hp_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            profile.role = SHIP_ROLE_SUPPORT;
            break;
        case SHIP_SALVAGE:
            profile.armor = 22;
            profile.hp = 200;
            profile.shield = 50;
            profile.max_speed = 16.0;
            profile.acceleration = 3.5;
            profile.turn_speed = 70.0;
            profile.optimal_range = 185.0;
            profile.max_range = 225.0;
            profile.base_damage = 4.0;
            profile.combat_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            profile.outnumbered_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            profile.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            profile.role = SHIP_ROLE_TRANSPORT;
            break;
        case SHIP_CAPITAL:
            profile.armor = 65;
            profile.hp = 360;
            profile.shield = 200;
            profile.max_speed = 18.5;
            profile.acceleration = 3.2;
            profile.turn_speed = 48.0;
            profile.optimal_range = 225.0;
            profile.max_range = 290.0;
            profile.base_damage = 15.0;
            profile.combat_behavior = SHIP_BEHAVIOR_CHARGE;
            profile.outnumbered_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            profile.unescorted_behavior = SHIP_BEHAVIOR_CHARGE;
            profile.low_hp_behavior = SHIP_BEHAVIOR_LAST_STAND;
            break;
        case SHIP_TRANSPORT:
            profile.armor = 18;
            profile.hp = 190;
            profile.shield = 60;
            profile.max_speed = 19.0;
            profile.acceleration = 4.2;
            profile.turn_speed = 72.0;
            profile.optimal_range = 175.0;
            profile.max_range = 215.0;
            profile.base_damage = 3.0;
            profile.combat_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            profile.outnumbered_behavior = SHIP_BEHAVIOR_RETREAT;
            profile.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            profile.role = SHIP_ROLE_TRANSPORT;
            break;
        case SHIP_CORVETTE:
            profile.armor = 28;
            profile.hp = 220;
            profile.shield = 90;
            profile.max_speed = 23.0;
            profile.acceleration = 5.8;
            profile.turn_speed = 95.0;
            profile.optimal_range = 190.0;
            profile.max_range = 235.0;
            profile.base_damage = 8.0;
            profile.combat_behavior = SHIP_BEHAVIOR_FLANK_SWEEP;
            break;
        case SHIP_INTERCEPTOR:
            profile.armor = 20;
            profile.hp = 170;
            profile.shield = 80;
            profile.max_speed = 30.0;
            profile.acceleration = 7.2;
            profile.turn_speed = 130.0;
            profile.optimal_range = 165.0;
            profile.max_range = 210.0;
            profile.base_damage = 9.0;
            profile.combat_behavior = SHIP_BEHAVIOR_CHARGE;
            profile.outnumbered_behavior = SHIP_BEHAVIOR_FLANK_SWEEP;
            profile.unescorted_behavior = SHIP_BEHAVIOR_CHARGE;
            break;
        case SHIP_REPAIR_DRONE:
            profile.armor = 8;
            profile.hp = 110;
            profile.shield = 90;
            profile.max_speed = 22.0;
            profile.acceleration = 6.8;
            profile.turn_speed = 140.0;
            profile.optimal_range = 155.0;
            profile.max_range = 205.0;
            profile.base_damage = 2.5;
            profile.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            profile.outnumbered_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            profile.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            profile.role = SHIP_ROLE_SUPPORT;
            break;
        case SHIP_SUNFLARE_SLOOP:
            profile.armor = 32;
            profile.hp = 240;
            profile.shield = 200;
            profile.max_speed = 21.0;
            profile.acceleration = 5.2;
            profile.turn_speed = 110.0;
            profile.optimal_range = 215.0;
            profile.max_range = 275.0;
            profile.base_damage = 4.0;
            profile.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            profile.outnumbered_behavior = SHIP_BEHAVIOR_RETREAT;
            profile.unescorted_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            profile.low_hp_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            profile.role = SHIP_ROLE_SUPPORT;
            break;
        case SHIP_FRIGATE_ESCORT:
            profile.armor = 42;
            profile.hp = 280;
            profile.shield = 130;
            profile.max_speed = 20.0;
            profile.acceleration = 4.6;
            profile.turn_speed = 85.0;
            profile.optimal_range = 205.0;
            profile.max_range = 265.0;
            profile.base_damage = 11.0;
            profile.combat_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            break;
        case SHIP_FRIGATE_SUPPORT:
            profile.armor = 38;
            profile.hp = 260;
            profile.shield = 150;
            profile.max_speed = 19.0;
            profile.acceleration = 4.3;
            profile.turn_speed = 80.0;
            profile.optimal_range = 225.0;
            profile.max_range = 285.0;
            profile.base_damage = 7.0;
            profile.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            profile.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            profile.low_hp_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            profile.role = SHIP_ROLE_SUPPORT;
            break;
        case SHIP_CAPITAL_CARRIER:
            profile.armor = 60;
            profile.hp = 340;
            profile.shield = 240;
            profile.max_speed = 18.0;
            profile.acceleration = 3.0;
            profile.turn_speed = 46.0;
            profile.optimal_range = 240.0;
            profile.max_range = 320.0;
            profile.base_damage = 13.5;
            profile.combat_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            profile.outnumbered_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            profile.unescorted_behavior = SHIP_BEHAVIOR_CHARGE;
            profile.low_hp_behavior = SHIP_BEHAVIOR_LAST_STAND;
            break;
        case SHIP_CAPITAL_DREADNOUGHT:
            profile.armor = 75;
            profile.hp = 400;
            profile.shield = 180;
            profile.max_speed = 17.5;
            profile.acceleration = 2.6;
            profile.turn_speed = 42.0;
            profile.optimal_range = 245.0;
            profile.max_range = 330.0;
            profile.base_damage = 18.0;
            profile.combat_behavior = SHIP_BEHAVIOR_CHARGE;
            profile.outnumbered_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            profile.unescorted_behavior = SHIP_BEHAVIOR_CHARGE;
            profile.low_hp_behavior = SHIP_BEHAVIOR_LAST_STAND;
            break;
        default:
            break;
        }
        return profile;
    }

    void assign_ship_defaults(ft_ship &ship)
    {
        ship_profile profile = get_ship_profile(ship.type);
        ship.armor = profile.armor;
        ship.hp = profile.hp;
        ship.max_hp = profile.hp;
        ship.shield = profile.shield;
        ship.max_shield = profile.shield;
        ship.max_speed = profile.max_speed;
        ship.acceleration = profile.acceleration;
        ship.turn_speed = profile.turn_speed;
        ship.optimal_range = profile.optimal_range;
        ship.max_range = profile.max_range;
        ship.base_damage = profile.base_damage;
        ship.combat_behavior = profile.combat_behavior;
        ship.outnumbered_behavior = profile.outnumbered_behavior;
        ship.unescorted_behavior = profile.unescorted_behavior;
        ship.low_hp_behavior = profile.low_hp_behavior;
        ship.role = profile.role;
    }

    const double ESCORT_VETERANCY_PER_BONUS = 60.0;
    const int    ESCORT_VETERANCY_MAX_BONUS = 8;
    const double ESCORT_VETERANCY_MAX_XP =
        ESCORT_VETERANCY_PER_BONUS * static_cast<double>(ESCORT_VETERANCY_MAX_BONUS);
}

int ft_fleet::_next_ship_id = 1;

ft_fleet::ft_fleet() noexcept : _id(0), _travel_time(0), _escort_veterancy(0.0)
{
    return ;
}

ft_fleet::ft_fleet(int id) noexcept : _id(id), _travel_time(0), _escort_veterancy(0.0)
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

double ft_fleet::get_escort_veterancy() const noexcept
{
    if (this->_escort_veterancy < 0.0)
        return 0.0;
    return this->_escort_veterancy;
}

void ft_fleet::set_escort_veterancy(double value) noexcept
{
    if (value < 0.0)
        value = 0.0;
    if (value > ESCORT_VETERANCY_MAX_XP)
        value = ESCORT_VETERANCY_MAX_XP;
    this->_escort_veterancy = value;
}

int ft_fleet::get_escort_veterancy_bonus() const noexcept
{
    if (this->_escort_veterancy <= 0.0)
        return 0;
    double xp = this->_escort_veterancy;
    if (xp > ESCORT_VETERANCY_MAX_XP)
        xp = ESCORT_VETERANCY_MAX_XP;
    int bonus = static_cast<int>(xp / ESCORT_VETERANCY_PER_BONUS);
    if (bonus < 0)
        bonus = 0;
    if (bonus > ESCORT_VETERANCY_MAX_BONUS)
        bonus = ESCORT_VETERANCY_MAX_BONUS;
    return bonus;
}

bool ft_fleet::add_escort_veterancy(double amount) noexcept
{
    if (amount <= 0.0)
        return false;
    int before = this->get_escort_veterancy_bonus();
    this->_escort_veterancy += amount;
    if (this->_escort_veterancy > ESCORT_VETERANCY_MAX_XP)
        this->_escort_veterancy = ESCORT_VETERANCY_MAX_XP;
    int after = this->get_escort_veterancy_bonus();
    return after > before;
}

bool ft_fleet::decay_escort_veterancy(double amount) noexcept
{
    if (amount <= 0.0)
        return false;
    int before = this->get_escort_veterancy_bonus();
    this->_escort_veterancy -= amount;
    if (this->_escort_veterancy < 0.0)
        this->_escort_veterancy = 0.0;
    int after = this->get_escort_veterancy_bonus();
    return after < before;
}

int ft_fleet::create_ship(int ship_type) noexcept
{
    int uid = _next_ship_id++;
    ft_ship ship(uid, ship_type);
    assign_ship_defaults(ship);
    this->_ships.insert(uid, ship);
    return uid;
}

void ft_fleet::clear_ships() noexcept
{
    this->_ships.clear();
}

void ft_fleet::add_ship_snapshot(const ft_ship &ship) noexcept
{
    this->_ships.insert(ship.id, ship);
    if (ship.id >= _next_ship_id)
    {
        if (ship.id >= FT_INT_MAX)
            _next_ship_id = FT_INT_MAX;
        else
        {
            int next_id = ship.id + 1;
            if (next_id <= ship.id)
                _next_ship_id = FT_INT_MAX;
            else
                _next_ship_id = next_id;
        }
    }
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
    if (value > ship->max_hp)
        ship->max_hp = value;
    if (ship->max_hp < 0)
        ship->max_hp = 0;
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
    if (ship->hp > ship->max_hp)
        ship->max_hp = ship->hp;
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
    if (value > ship->max_shield)
        ship->max_shield = value;
    if (ship->max_shield < 0)
        ship->max_shield = 0;
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
    if (ship->shield > ship->max_shield)
        ship->max_shield = ship->shield;
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

void ft_fleet::get_ship_ids(ft_vector<int> &out) const noexcept
{
    out.clear();
    size_t count = this->_ships.size();
    if (count == 0)
        return ;
    const Pair<int, ft_ship> *entries = this->_ships.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
        out.push_back(entries[i].key);
}

int ft_fleet::get_ship_type(int ship_uid) const noexcept
{
    const ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    return ship->type;
}

const ft_ship *ft_fleet::get_ship(int ship_uid) const noexcept
{
    return this->find_ship(ship_uid);
}

double ft_fleet::absorb_damage(double damage, double shield_multiplier, double hull_multiplier) noexcept
{
    if (damage <= 0.0)
        return 0.0;
    if (shield_multiplier < 1.0)
        shield_multiplier = 1.0;
    if (hull_multiplier < 1.0)
        hull_multiplier = 1.0;
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
        double incoming = static_cast<double>(share);
        int available_shield = ship->shield;
        if (available_shield > 0)
        {
            double ratio = incoming / shield_multiplier;
            int reduction_units = static_cast<int>(ratio);
            if (static_cast<double>(reduction_units) < ratio)
                reduction_units += 1;
            if (reduction_units > available_shield)
                reduction_units = available_shield;
            ship->shield -= reduction_units;
            double absorbed = static_cast<double>(reduction_units) * shield_multiplier;
            if (absorbed > incoming)
                absorbed = incoming;
            incoming -= absorbed;
            if (incoming < 0.0)
                incoming = 0.0;
        }
        if (incoming <= 0.0)
            continue;
        double base_damage = incoming / hull_multiplier;
        double reduction = static_cast<double>(ship->armor) * 0.01;
        if (reduction > 0.6)
            reduction = 0.6;
        double hull_damage_d = base_damage * (1.0 - reduction);
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
        if (ship->max_shield > 0 && ship->shield >= ship->max_shield)
            continue;
        size_t remain = shield_targets.size() - i;
        int share = shield_remaining / static_cast<int>(remain);
        if (share <= 0)
            share = 1;
        if (share > shield_remaining)
            share = shield_remaining;
        if (ship->max_shield > 0)
        {
            int capacity = ship->max_shield - ship->shield;
            if (capacity <= 0)
                continue;
            if (share > capacity)
                share = capacity;
        }
        ship->shield += share;
        shield_remaining -= share;
    }
    int repair_remaining = repair_amount;
    for (size_t i = 0; i < repair_targets.size() && repair_remaining > 0; ++i)
    {
        ft_ship *ship = repair_targets[i];
        if (ship->max_hp > 0 && ship->hp >= ship->max_hp)
            continue;
        size_t remain = repair_targets.size() - i;
        int share = repair_remaining / static_cast<int>(remain);
        if (share <= 0)
            share = 1;
        if (share > repair_remaining)
            share = repair_remaining;
        if (ship->max_hp > 0)
        {
            int capacity = ship->max_hp - ship->hp;
            if (capacity <= 0)
                continue;
            if (share > capacity)
                share = capacity;
        }
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

double ft_fleet::get_ship_damage_baseline(int ship_type) noexcept
{
    switch (ship_type)
    {
    case SHIP_SHIELD:
        return 5.5;
    case SHIP_RADAR:
        return 4.5;
    case SHIP_SALVAGE:
        return 4.0;
    case SHIP_TRANSPORT:
        return 3.0;
    case SHIP_CORVETTE:
        return 8.0;
    case SHIP_INTERCEPTOR:
        return 9.0;
    case SHIP_REPAIR_DRONE:
        return 2.5;
    case SHIP_SUNFLARE_SLOOP:
        return 4.0;
    case SHIP_FRIGATE_ESCORT:
        return 11.0;
    case SHIP_FRIGATE_SUPPORT:
        return 7.0;
    case SHIP_CAPITAL_CARRIER:
        return 13.5;
    case SHIP_CAPITAL_DREADNOUGHT:
        return 18.0;
    case SHIP_CAPITAL:
        return 15.0;
    default:
        break;
    }
    return 4.0;
}

bool is_capital_ship_type(int ship_type) noexcept
{
    if (ship_type == SHIP_CAPITAL)
        return true;
    if (ship_type == SHIP_CAPITAL_CARRIER)
        return true;
    if (ship_type == SHIP_CAPITAL_DREADNOUGHT)
        return true;
    return false;
}

