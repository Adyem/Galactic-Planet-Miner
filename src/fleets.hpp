#ifndef FLEETS_HPP
#define FLEETS_HPP

#include "planets.hpp"
#include "../libft/Game/game_character.hpp"
#include "../libft/Template/map.hpp"
#include "../libft/Template/pair.hpp"
#include "../libft/Template/shared_ptr.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/CPP_class/class_nullptr.hpp"

enum e_ship_id
{
    SHIP_SHIELD = 1,
    SHIP_RADAR,
    SHIP_SALVAGE,
    SHIP_CAPITAL,
    SHIP_TRANSPORT,
    SHIP_CORVETTE,
    SHIP_INTERCEPTOR,
    SHIP_REPAIR_DRONE,
    SHIP_SUNFLARE_SLOOP,
    SHIP_FRIGATE_ESCORT,
    SHIP_FRIGATE_SUPPORT,
    SHIP_CAPITAL_CARRIER,
    SHIP_CAPITAL_DREADNOUGHT,
    SHIP_CAPITAL_BATTLESHIP = SHIP_CAPITAL
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

enum e_ship_role
{
    SHIP_ROLE_LINE = 1,
    SHIP_ROLE_SUPPORT,
    SHIP_ROLE_TRANSPORT
};

enum e_ship_behavior_mode
{
    SHIP_BEHAVIOR_LINE_HOLD = 1,
    SHIP_BEHAVIOR_FLANK_SWEEP,
    SHIP_BEHAVIOR_SCREEN_SUPPORT,
    SHIP_BEHAVIOR_CHARGE,
    SHIP_BEHAVIOR_RETREAT,
    SHIP_BEHAVIOR_WITHDRAW_SUPPORT,
    SHIP_BEHAVIOR_LAST_STAND
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
    int max_hp;
    int max_shield;
    double max_speed;
    double acceleration;
    double turn_speed;
    int combat_behavior;
    int outnumbered_behavior;
    int unescorted_behavior;
    int low_hp_behavior;
    int role;
    ft_ship()
        : id(0), type(0), armor(0), hp(0), shield(0), max_hp(0),
          max_shield(0), max_speed(18.0), acceleration(4.0),
          turn_speed(60.0), combat_behavior(SHIP_BEHAVIOR_LINE_HOLD),
          outnumbered_behavior(SHIP_BEHAVIOR_RETREAT),
          unescorted_behavior(SHIP_BEHAVIOR_WITHDRAW_SUPPORT),
          low_hp_behavior(SHIP_BEHAVIOR_RETREAT), role(SHIP_ROLE_LINE)
    {}
    ft_ship(int i, int t)
        : id(i), type(t), armor(0), hp(0), shield(0), max_hp(0),
          max_shield(0), max_speed(18.0), acceleration(4.0),
          turn_speed(60.0), combat_behavior(SHIP_BEHAVIOR_LINE_HOLD),
          outnumbered_behavior(SHIP_BEHAVIOR_RETREAT),
          unescorted_behavior(SHIP_BEHAVIOR_WITHDRAW_SUPPORT),
          low_hp_behavior(SHIP_BEHAVIOR_RETREAT), role(SHIP_ROLE_LINE)
    {}
};

class ft_fleet : public ft_character
{
private:
    int                    _id;
    ft_map<int, ft_ship>   _ships;
    ft_location            _loc;
    double                 _travel_time;
    double                 _escort_veterancy;
    static int             _next_ship_id;

    ft_ship *find_ship(int ship_uid) noexcept;
    const ft_ship *find_ship(int ship_uid) const noexcept;

public:
    ft_fleet() noexcept;
    explicit ft_fleet(int id) noexcept;

    int get_id() const noexcept;
    int get_ship_count() const noexcept;
    int get_total_ship_hp() const noexcept;
    int get_total_ship_shield() const noexcept;
    double get_attack_power() const noexcept;
    double get_escort_veterancy() const noexcept;
    int get_escort_veterancy_bonus() const noexcept;
    bool add_escort_veterancy(double amount) noexcept;
    bool decay_escort_veterancy(double amount) noexcept;

    int create_ship(int ship_type) noexcept;
    void remove_ship(int ship_uid) noexcept;
    bool move_ship_to(ft_fleet &target, int ship_uid) noexcept;
    void move_ships_to(ft_fleet &target) noexcept;

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

    void get_ship_ids(ft_vector<int> &out) const noexcept;
    int get_ship_type(int ship_uid) const noexcept;
    const ft_ship *get_ship(int ship_uid) const noexcept;

    double absorb_damage(double damage, double shield_multiplier, double hull_multiplier) noexcept;
    void apply_support(int shield_amount, int repair_amount) noexcept;
    bool has_operational_ships() const noexcept;

    void set_location_planet(int planet_id) noexcept;
    void set_location_travel(int from, int to, double time) noexcept;
    void set_location_misc(int misc_id) noexcept;
    ft_location get_location() const noexcept;
    double get_travel_time() const noexcept;
    void tick(double seconds) noexcept;
};

bool is_capital_ship_type(int ship_type) noexcept;


#endif
