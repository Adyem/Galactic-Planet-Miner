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
        double  deceleration;
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
              deceleration(3.5), turn_speed(60.0), optimal_range(180.0), max_range(240.0),
              base_damage(4.0), combat_behavior(SHIP_BEHAVIOR_LINE_HOLD),
              outnumbered_behavior(SHIP_BEHAVIOR_RETREAT),
              unescorted_behavior(SHIP_BEHAVIOR_WITHDRAW_SUPPORT),
              low_hp_behavior(SHIP_BEHAVIOR_RETREAT), role(SHIP_ROLE_LINE)
        {}
    };

    struct ship_profile_table
    {
        ship_profile profiles[SHIP_RAIDER_BATTLESHIP + 1];

        ship_profile_table()
        {
            for (int i = 0; i <= SHIP_RAIDER_BATTLESHIP; ++i)
                profiles[i] = ship_profile();

            ship_profile &transport = profiles[SHIP_TRANSPORT];
            transport.armor = 18;
            transport.hp = 100;
            transport.shield = 50;
            transport.max_speed = 19.0;
            transport.acceleration = 4.2;
            transport.deceleration = 3.6;
            transport.turn_speed = 72.0;
            transport.optimal_range = 175.0;
            transport.max_range = 215.0;
            transport.base_damage = 10.0;
            transport.combat_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            transport.outnumbered_behavior = SHIP_BEHAVIOR_RETREAT;
            transport.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            transport.role = SHIP_ROLE_TRANSPORT;

            ship_profile &corvette = profiles[SHIP_CORVETTE];
            corvette.armor = 28;
            corvette.hp = 100;
            corvette.shield = 75;
            corvette.max_speed = 23.0;
            corvette.acceleration = 5.8;
            corvette.deceleration = 5.4;
            corvette.turn_speed = 95.0;
            corvette.optimal_range = 190.0;
            corvette.max_range = 235.0;
            corvette.base_damage = 30.0;
            corvette.combat_behavior = SHIP_BEHAVIOR_FLANK_SWEEP;

            ship_profile &shield = profiles[SHIP_SHIELD];
            shield.armor = 35;
            shield.hp = 120;
            shield.shield = 150;
            shield.max_speed = 21.0;
            shield.acceleration = 5.5;
            shield.deceleration = 5.0;
            shield.turn_speed = 95.0;
            shield.optimal_range = 205.0;
            shield.max_range = 265.0;
            shield.base_damage = 20.0;
            shield.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            shield.unescorted_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            shield.role = SHIP_ROLE_SUPPORT;

            ship_profile &radar = profiles[SHIP_RADAR];
            radar.armor = 10;
            radar.hp = 110;
            radar.shield = 80;
            radar.max_speed = 26.0;
            radar.acceleration = 6.5;
            radar.deceleration = 6.2;
            radar.turn_speed = 110.0;
            radar.optimal_range = 235.0;
            radar.max_range = 320.0;
            radar.base_damage = 25.0;
            radar.combat_behavior = SHIP_BEHAVIOR_FLANK_SWEEP;
            radar.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            radar.low_hp_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            radar.role = SHIP_ROLE_SUPPORT;

            ship_profile &salvage = profiles[SHIP_SALVAGE];
            salvage.armor = 22;
            salvage.hp = 110;
            salvage.shield = 70;
            salvage.max_speed = 16.0;
            salvage.acceleration = 3.5;
            salvage.deceleration = 3.0;
            salvage.turn_speed = 70.0;
            salvage.optimal_range = 185.0;
            salvage.max_range = 225.0;
            salvage.base_damage = 5.0;
            salvage.combat_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            salvage.outnumbered_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            salvage.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            salvage.role = SHIP_ROLE_TRANSPORT;

            ship_profile &interceptor = profiles[SHIP_INTERCEPTOR];
            interceptor.armor = 20;
            interceptor.hp = 90;
            interceptor.shield = 60;
            interceptor.max_speed = 30.0;
            interceptor.acceleration = 7.2;
            interceptor.deceleration = 7.0;
            interceptor.turn_speed = 130.0;
            interceptor.optimal_range = 165.0;
            interceptor.max_range = 210.0;
            interceptor.base_damage = 40.0;
            interceptor.combat_behavior = SHIP_BEHAVIOR_CHARGE;
            interceptor.outnumbered_behavior = SHIP_BEHAVIOR_FLANK_SWEEP;
            interceptor.unescorted_behavior = SHIP_BEHAVIOR_CHARGE;

            ship_profile &repair_drone = profiles[SHIP_REPAIR_DRONE];
            repair_drone.armor = 8;
            repair_drone.hp = 80;
            repair_drone.shield = 40;
            repair_drone.max_speed = 22.0;
            repair_drone.acceleration = 6.8;
            repair_drone.deceleration = 6.4;
            repair_drone.turn_speed = 140.0;
            repair_drone.optimal_range = 155.0;
            repair_drone.max_range = 205.0;
            repair_drone.base_damage = 5.0;
            repair_drone.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            repair_drone.outnumbered_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            repair_drone.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            repair_drone.role = SHIP_ROLE_SUPPORT;

            ship_profile &sunflare = profiles[SHIP_SUNFLARE_SLOOP];
            sunflare.armor = 32;
            sunflare.hp = 80;
            sunflare.shield = 60;
            sunflare.max_speed = 21.0;
            sunflare.acceleration = 5.2;
            sunflare.deceleration = 4.6;
            sunflare.turn_speed = 110.0;
            sunflare.optimal_range = 215.0;
            sunflare.max_range = 275.0;
            sunflare.base_damage = 10.0;
            sunflare.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            sunflare.outnumbered_behavior = SHIP_BEHAVIOR_RETREAT;
            sunflare.unescorted_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            sunflare.low_hp_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            sunflare.role = SHIP_ROLE_SUPPORT;

            ship_profile &capital_juggernaut = profiles[SHIP_CAPITAL_JUGGERNAUT];
            capital_juggernaut.armor = 70;
            capital_juggernaut.hp = 550;
            capital_juggernaut.shield = 100;
            capital_juggernaut.max_speed = 18.5;
            capital_juggernaut.acceleration = 3.0;
            capital_juggernaut.deceleration = 2.3;
            capital_juggernaut.turn_speed = 45.0;
            capital_juggernaut.optimal_range = 200.0;
            capital_juggernaut.max_range = 260.0;
            capital_juggernaut.base_damage = 80.0;
            capital_juggernaut.combat_behavior = SHIP_BEHAVIOR_CHARGE;
            capital_juggernaut.outnumbered_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            capital_juggernaut.unescorted_behavior = SHIP_BEHAVIOR_CHARGE;
            capital_juggernaut.low_hp_behavior = SHIP_BEHAVIOR_LAST_STAND;
            capital_juggernaut.role = SHIP_ROLE_TRANSPORT;

            ship_profile &capital_nova = profiles[SHIP_CAPITAL_NOVA];
            capital_nova.armor = 60;
            capital_nova.hp = 530;
            capital_nova.shield = 120;
            capital_nova.max_speed = 18.0;
            capital_nova.acceleration = 3.0;
            capital_nova.deceleration = 2.2;
            capital_nova.turn_speed = 46.0;
            capital_nova.optimal_range = 240.0;
            capital_nova.max_range = 320.0;
            capital_nova.base_damage = 35.0;
            capital_nova.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            capital_nova.outnumbered_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            capital_nova.unescorted_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            capital_nova.low_hp_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            capital_nova.role = SHIP_ROLE_SUPPORT;

            ship_profile &capital_obsidian = profiles[SHIP_CAPITAL_OBSIDIAN];
            capital_obsidian.armor = 80;
            capital_obsidian.hp = 600;
            capital_obsidian.shield = 80;
            capital_obsidian.max_speed = 17.5;
            capital_obsidian.acceleration = 2.6;
            capital_obsidian.deceleration = 1.8;
            capital_obsidian.turn_speed = 42.0;
            capital_obsidian.optimal_range = 230.0;
            capital_obsidian.max_range = 310.0;
            capital_obsidian.base_damage = 85.0;
            capital_obsidian.combat_behavior = SHIP_BEHAVIOR_CHARGE;
            capital_obsidian.outnumbered_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            capital_obsidian.unescorted_behavior = SHIP_BEHAVIOR_CHARGE;
            capital_obsidian.low_hp_behavior = SHIP_BEHAVIOR_LAST_STAND;

            ship_profile &capital_preemptor = profiles[SHIP_CAPITAL_PREEMPTOR];
            capital_preemptor.armor = 68;
            capital_preemptor.hp = 520;
            capital_preemptor.shield = 110;
            capital_preemptor.max_speed = 17.0;
            capital_preemptor.acceleration = 2.5;
            capital_preemptor.deceleration = 1.9;
            capital_preemptor.turn_speed = 40.0;
            capital_preemptor.optimal_range = 320.0;
            capital_preemptor.max_range = 420.0;
            capital_preemptor.base_damage = 250.0;
            capital_preemptor.combat_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            capital_preemptor.outnumbered_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            capital_preemptor.unescorted_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            capital_preemptor.low_hp_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;

            ship_profile &capital_protector = profiles[SHIP_CAPITAL_PROTECTOR];
            capital_protector.armor = 72;
            capital_protector.hp = 540;
            capital_protector.shield = 200;
            capital_protector.max_speed = 18.0;
            capital_protector.acceleration = 2.8;
            capital_protector.deceleration = 2.1;
            capital_protector.turn_speed = 44.0;
            capital_protector.optimal_range = 210.0;
            capital_protector.max_range = 280.0;
            capital_protector.base_damage = 82.0;
            capital_protector.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            capital_protector.outnumbered_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            capital_protector.unescorted_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            capital_protector.low_hp_behavior = SHIP_BEHAVIOR_LAST_STAND;
            capital_protector.role = SHIP_ROLE_SUPPORT;

            ship_profile &capital_eclipse = profiles[SHIP_CAPITAL_ECLIPSE];
            capital_eclipse.armor = 90;
            capital_eclipse.hp = 1000;
            capital_eclipse.shield = 0;
            capital_eclipse.max_speed = 16.0;
            capital_eclipse.acceleration = 2.2;
            capital_eclipse.deceleration = 1.6;
            capital_eclipse.turn_speed = 36.0;
            capital_eclipse.optimal_range = 160.0;
            capital_eclipse.max_range = 220.0;
            capital_eclipse.base_damage = 70.0;
            capital_eclipse.combat_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            capital_eclipse.outnumbered_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            capital_eclipse.unescorted_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            capital_eclipse.low_hp_behavior = SHIP_BEHAVIOR_LAST_STAND;
            capital_eclipse.role = SHIP_ROLE_SUPPORT;

            ship_profile &frigate_juggernaut = profiles[SHIP_FRIGATE_JUGGERNAUT];
            frigate_juggernaut.armor = 42;
            frigate_juggernaut.hp = 150;
            frigate_juggernaut.shield = 100;
            frigate_juggernaut.max_speed = 20.0;
            frigate_juggernaut.acceleration = 4.6;
            frigate_juggernaut.deceleration = 4.0;
            frigate_juggernaut.turn_speed = 85.0;
            frigate_juggernaut.optimal_range = 205.0;
            frigate_juggernaut.max_range = 265.0;
            frigate_juggernaut.base_damage = 50.0;
            frigate_juggernaut.combat_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            frigate_juggernaut.role = SHIP_ROLE_TRANSPORT;

            ship_profile &frigate_carrier = profiles[SHIP_FRIGATE_CARRIER];
            frigate_carrier.armor = 36;
            frigate_carrier.hp = 140;
            frigate_carrier.shield = 90;
            frigate_carrier.max_speed = 19.5;
            frigate_carrier.acceleration = 4.2;
            frigate_carrier.deceleration = 3.6;
            frigate_carrier.turn_speed = 82.0;
            frigate_carrier.optimal_range = 220.0;
            frigate_carrier.max_range = 280.0;
            frigate_carrier.base_damage = 45.0;
            frigate_carrier.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            frigate_carrier.outnumbered_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            frigate_carrier.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            frigate_carrier.low_hp_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            frigate_carrier.role = SHIP_ROLE_SUPPORT;

            ship_profile &frigate_sovereign = profiles[SHIP_FRIGATE_SOVEREIGN];
            frigate_sovereign.armor = 44;
            frigate_sovereign.hp = 160;
            frigate_sovereign.shield = 110;
            frigate_sovereign.max_speed = 20.5;
            frigate_sovereign.acceleration = 4.8;
            frigate_sovereign.deceleration = 4.2;
            frigate_sovereign.turn_speed = 88.0;
            frigate_sovereign.optimal_range = 210.0;
            frigate_sovereign.max_range = 270.0;
            frigate_sovereign.base_damage = 55.0;
            frigate_sovereign.combat_behavior = SHIP_BEHAVIOR_LINE_HOLD;

            ship_profile &frigate_preemptor = profiles[SHIP_FRIGATE_PREEMPTOR];
            frigate_preemptor.armor = 34;
            frigate_preemptor.hp = 130;
            frigate_preemptor.shield = 95;
            frigate_preemptor.max_speed = 21.0;
            frigate_preemptor.acceleration = 4.9;
            frigate_preemptor.deceleration = 4.3;
            frigate_preemptor.turn_speed = 90.0;
            frigate_preemptor.optimal_range = 260.0;
            frigate_preemptor.max_range = 330.0;
            frigate_preemptor.base_damage = 100.0;
            frigate_preemptor.combat_behavior = SHIP_BEHAVIOR_LINE_HOLD;

            ship_profile &frigate_protector = profiles[SHIP_FRIGATE_PROTECTOR];
            frigate_protector.armor = 38;
            frigate_protector.hp = 145;
            frigate_protector.shield = 100;
            frigate_protector.max_speed = 19.0;
            frigate_protector.acceleration = 4.3;
            frigate_protector.deceleration = 3.7;
            frigate_protector.turn_speed = 80.0;
            frigate_protector.optimal_range = 225.0;
            frigate_protector.max_range = 285.0;
            frigate_protector.base_damage = 48.0;
            frigate_protector.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            frigate_protector.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            frigate_protector.low_hp_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            frigate_protector.role = SHIP_ROLE_SUPPORT;

            ship_profile &frigate_eclipse = profiles[SHIP_FRIGATE_ECLIPSE];
            frigate_eclipse.armor = 40;
            frigate_eclipse.hp = 325;
            frigate_eclipse.shield = 0;
            frigate_eclipse.max_speed = 18.0;
            frigate_eclipse.acceleration = 3.8;
            frigate_eclipse.deceleration = 3.2;
            frigate_eclipse.turn_speed = 78.0;
            frigate_eclipse.optimal_range = 200.0;
            frigate_eclipse.max_range = 260.0;
            frigate_eclipse.base_damage = 40.0;
            frigate_eclipse.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
            frigate_eclipse.outnumbered_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            frigate_eclipse.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            frigate_eclipse.low_hp_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
            frigate_eclipse.role = SHIP_ROLE_SUPPORT;

            ship_profile &raider_corvette = profiles[SHIP_RAIDER_CORVETTE];
            raider_corvette.armor = 24;
            raider_corvette.hp = 190;
            raider_corvette.shield = 70;
            raider_corvette.max_speed = 27.0;
            raider_corvette.acceleration = 6.8;
            raider_corvette.deceleration = 6.3;
            raider_corvette.turn_speed = 118.0;
            raider_corvette.optimal_range = 185.0;
            raider_corvette.max_range = 245.0;
            raider_corvette.base_damage = 9.5;
            raider_corvette.combat_behavior = SHIP_BEHAVIOR_CHARGE;
            raider_corvette.outnumbered_behavior = SHIP_BEHAVIOR_FLANK_SWEEP;
            raider_corvette.unescorted_behavior = SHIP_BEHAVIOR_CHARGE;
            raider_corvette.low_hp_behavior = SHIP_BEHAVIOR_RETREAT;

            ship_profile &raider_destroyer = profiles[SHIP_RAIDER_DESTROYER];
            raider_destroyer.armor = 36;
            raider_destroyer.hp = 260;
            raider_destroyer.shield = 110;
            raider_destroyer.max_speed = 20.5;
            raider_destroyer.acceleration = 4.8;
            raider_destroyer.deceleration = 4.0;
            raider_destroyer.turn_speed = 92.0;
            raider_destroyer.optimal_range = 210.0;
            raider_destroyer.max_range = 280.0;
            raider_destroyer.base_damage = 12.5;
            raider_destroyer.combat_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            raider_destroyer.outnumbered_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            raider_destroyer.unescorted_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            raider_destroyer.low_hp_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;

            ship_profile &raider_battleship = profiles[SHIP_RAIDER_BATTLESHIP];
            raider_battleship.armor = 68;
            raider_battleship.hp = 430;
            raider_battleship.shield = 210;
            raider_battleship.max_speed = 18.0;
            raider_battleship.acceleration = 3.0;
            raider_battleship.deceleration = 2.1;
            raider_battleship.turn_speed = 50.0;
            raider_battleship.optimal_range = 240.0;
            raider_battleship.max_range = 320.0;
            raider_battleship.base_damage = 16.0;
            raider_battleship.combat_behavior = SHIP_BEHAVIOR_CHARGE;
            raider_battleship.outnumbered_behavior = SHIP_BEHAVIOR_LINE_HOLD;
            raider_battleship.unescorted_behavior = SHIP_BEHAVIOR_CHARGE;
            raider_battleship.low_hp_behavior = SHIP_BEHAVIOR_LAST_STAND;
        }
    };

    ship_profile get_ship_profile(int type)
    {
        static const ship_profile_table table;

        if (type < 0 || type > SHIP_RAIDER_BATTLESHIP)
            return table.profiles[0];
        return table.profiles[type];
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
        ship.deceleration = profile.deceleration;
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
    if (_next_ship_id >= FT_INT_MAX)
        return 0;
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
        if (ship.id >= FT_INT_MAX - 1)
            _next_ship_id = FT_INT_MAX;
        else
        {
            int next_id = ship.id + 1;
            if (next_id <= ship.id || next_id >= FT_INT_MAX)
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
    ship_profile profile = get_ship_profile(ship_type);
    return profile.base_damage;
}

bool is_capital_ship_type(int ship_type) noexcept
{
    switch (ship_type)
    {
    case SHIP_CAPITAL_JUGGERNAUT:
    case SHIP_CAPITAL_NOVA:
    case SHIP_CAPITAL_OBSIDIAN:
    case SHIP_CAPITAL_PREEMPTOR:
    case SHIP_CAPITAL_PROTECTOR:
    case SHIP_CAPITAL_ECLIPSE:
    case SHIP_RAIDER_BATTLESHIP:
        return true;
    default:
        break;
    }
    return false;
}

