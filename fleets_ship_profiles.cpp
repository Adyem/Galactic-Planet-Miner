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
        double  attack_speed;
        double  base_damage;
        int     combat_behavior;
        int     outnumbered_behavior;
        int     unescorted_behavior;
        int     low_hp_behavior;
        int     role;

        ship_profile()
            : armor(18), hp(180), shield(60), max_speed(18.0), acceleration(4.0),
              deceleration(3.5), turn_speed(60.0), optimal_range(180.0), max_range(240.0),
              attack_speed(1.0), base_damage(4.0), combat_behavior(SHIP_BEHAVIOR_LINE_HOLD),
              outnumbered_behavior(SHIP_BEHAVIOR_RETREAT),
              unescorted_behavior(SHIP_BEHAVIOR_WITHDRAW_SUPPORT),
              low_hp_behavior(SHIP_BEHAVIOR_RETREAT), role(SHIP_ROLE_LINE)
        {}
    };

    struct ship_profile_table;

    void init_federation_profiles(ship_profile_table &table);
    void init_capital_profiles(ship_profile_table &table);
    void init_frigate_profiles(ship_profile_table &table);
    void init_raider_profiles(ship_profile_table &table);

    struct ship_profile_table
    {
        ship_profile profiles[SHIP_RAIDER_BATTLESHIP + 1];

        ship_profile_table()
        {
            for (int i = 0; i <= SHIP_RAIDER_BATTLESHIP; ++i)
                profiles[i] = ship_profile();

            init_federation_profiles(*this);
            init_capital_profiles(*this);
            init_frigate_profiles(*this);
            init_raider_profiles(*this);
        }
    };

#include "fleets_ship_profiles_federation.cpp"
#include "fleets_ship_profiles_capital.cpp"
#include "fleets_ship_profiles_frigate.cpp"
#include "fleets_ship_profiles_raider.cpp"

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
        ship.attack_speed = profile.attack_speed;
        ship.base_damage = profile.base_damage;
        ship.combat_behavior = profile.combat_behavior;
        ship.outnumbered_behavior = profile.outnumbered_behavior;
        ship.unescorted_behavior = profile.unescorted_behavior;
        ship.low_hp_behavior = profile.low_hp_behavior;
        ship.role = profile.role;
    }
}
