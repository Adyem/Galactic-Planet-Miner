#ifndef COMBAT_HPP
#define COMBAT_HPP

#include "fleets.hpp"
#include "../libft/Template/map.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/pair.hpp"
#include "../libft/Template/shared_ptr.hpp"
#include "../libft/CPP_class/class_nullptr.hpp"

struct ft_combat_modifiers
{
    bool sunflare_docked;
    bool repair_drones_active;
    bool shield_generator_online;
    ft_combat_modifiers()
        : sunflare_docked(false),
          repair_drones_active(false),
          shield_generator_online(false)
    {}
};

class CombatManager
{
private:
    struct ft_combat_encounter
    {
        int                     planet_id;
        ft_sharedptr<ft_vector<int> > fleet_ids;
        ft_combat_modifiers     modifiers;
        double                  raider_shield;
        double                  raider_hull;
        double                  base_damage;
        double                  elapsed;
        bool                    active;
        ft_combat_encounter()
            : planet_id(0), fleet_ids(new ft_vector<int>()), modifiers(), raider_shield(0.0),
              raider_hull(0.0), base_damage(0.0), elapsed(0.0), active(false)
        {}
    };

    ft_map<int, ft_combat_encounter> _encounters;

    void gather_defenders(const ft_combat_encounter &encounter,
        ft_map<int, ft_sharedptr<ft_fleet> > &fleets,
        ft_map<int, ft_sharedptr<ft_fleet> > &planet_fleets,
        ft_vector<ft_sharedptr<ft_fleet> > &out) const;

    double calculate_player_power(const ft_vector<ft_sharedptr<ft_fleet> > &defenders) const;

    void apply_support(const ft_combat_encounter &encounter,
        ft_vector<ft_sharedptr<ft_fleet> > &defenders,
        double seconds);

public:
    CombatManager();

    bool start_raider_assault(int planet_id, double difficulty);
    bool add_fleet(int planet_id, int fleet_id);
    bool set_support(int planet_id, bool sunflare_docked,
        bool repair_drones_active, bool shield_generator_online);
    bool is_assault_active(int planet_id) const;
    double get_raider_shield(int planet_id) const;
    double get_raider_hull(int planet_id) const;
    double get_elapsed(int planet_id) const;
    void tick(double seconds, ft_map<int, ft_sharedptr<ft_fleet> > &fleets,
        ft_map<int, ft_sharedptr<ft_fleet> > &planet_fleets,
        ft_vector<int> &completed, ft_vector<int> &failed);
};

#endif
