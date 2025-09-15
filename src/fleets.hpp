#ifndef FLEETS_HPP
#define FLEETS_HPP

#include "planets.hpp"
#include "../libft/Game/game_character.hpp"
#include "../libft/Template/map.hpp"
#include "../libft/Template/pair.hpp"
#include "../libft/Template/shared_ptr.hpp"
#include "../libft/CPP_class/class_nullptr.hpp"

enum e_ship_id
{
    SHIP_SHIELD = 1,
    SHIP_RADAR,
    SHIP_SALVAGE,
    SHIP_CAPITAL
};

enum e_misc_location_id
{
    MISC_ASTEROID_HIDEOUT = 1,
    MISC_OUTPOST_NEBULA_X
};

enum e_location_type
{
    LOCATION_PLANET = 1,
    LOCATION_TRAVEL,
    LOCATION_MISC
};

struct ft_location
{
    int type;
    int from;
    int to;
    int misc;
    ft_location() : type(LOCATION_PLANET), from(PLANET_TERRA), to(PLANET_TERRA), misc(0) {}
};

struct ft_ship
{
    int id;
    int type;
    int armor;
    int hp;
    int shield;
    ft_ship() : id(0), type(0), armor(0), hp(0), shield(0) {}
    ft_ship(int i, int t) : id(i), type(t), armor(0), hp(0), shield(0) {}
};

class ft_fleet : public ft_character
{
private:
    int                    _id;
    ft_map<int, ft_ship>   _ships;
    ft_location            _loc;
    static int             _next_ship_id;

    ft_ship *find_ship(int ship_uid) noexcept;
    const ft_ship *find_ship(int ship_uid) const noexcept;

public:
    explicit ft_fleet(int id) noexcept;

    int get_id() const noexcept;

    int create_ship(int ship_type) noexcept;
    void remove_ship(int ship_uid) noexcept;

    void set_ship_armor(int ship_uid, int value) noexcept;
    int get_ship_armor(int ship_uid) const noexcept;
    int add_ship_armor(int ship_uid, int amount) noexcept;
    int sub_ship_armor(int ship_uid, int amount) noexcept;

    void set_ship_hp(int ship_uid, int value) noexcept;
    int get_ship_hp(int ship_uid) const noexcept;
    int add_ship_hp(int ship_uid, int amount) noexcept;
    int sub_ship_hp(int ship_uid, int amount) noexcept;

    void set_ship_shield(int ship_uid, int value) noexcept;
    int get_ship_shield(int ship_uid) const noexcept;
    int add_ship_shield(int ship_uid, int amount) noexcept;
    int sub_ship_shield(int ship_uid, int amount) noexcept;

    void set_location_planet(int planet_id) noexcept;
    void set_location_travel(int from, int to) noexcept;
    void set_location_misc(int misc_id) noexcept;
    ft_location get_location() const noexcept;
};

#endif
