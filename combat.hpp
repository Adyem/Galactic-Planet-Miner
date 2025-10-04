#ifndef COMBAT_HPP
#define COMBAT_HPP

#include "fleets.hpp"
#include "libft/Template/map.hpp"
#include "libft/Template/vector.hpp"
#include "libft/Template/pair.hpp"
#include "libft/Template/shared_ptr.hpp"
#include "libft/CPP_class/class_nullptr.hpp"

#define ASSAULT_CONTROL_AUTO 1
#define ASSAULT_CONTROL_ACTIVE 2

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

struct ft_ship_spatial_state
{
    int    ship_uid;
    int    ship_type;
    double x;
    double y;
    double z;
    ft_ship_spatial_state()
        : ship_uid(0), ship_type(0), x(0.0), y(0.0), z(0.0)
    {}
};

class CombatManager
{
private:
    struct ft_ship_tracker
    {
        ft_ship_spatial_state spatial;
        double                preferred_radius;
        double                lane_offset;
        double                vertical_layer;
        double                advance_bias;
        double                drift_origin;
        double                drift_speed;
        double                base_preferred_radius;
        double                base_advance_bias;
        double                max_speed;
        double                acceleration;
        double                deceleration;
        double                turn_speed;
        double                current_speed;
        double                optimal_range;
        double                max_range;
        double                base_damage;
        bool                  flank;
        bool                  base_flank;
        bool                  requires_escort;
        int                   role;
        int                   max_hp;
        int                   max_shield;
        int                   normal_behavior;
        int                   outnumbered_behavior;
        int                   unescorted_behavior;
        int                   low_hp_behavior;
        double                hp_ratio;
        double                shield_ratio;
        double                heading_x;
        double                heading_y;
        double                heading_z;
        ft_ship_tracker()
            : spatial(), preferred_radius(30.0), lane_offset(0.0),
              vertical_layer(0.0), advance_bias(0.0), drift_origin(0.0),
              drift_speed(0.6), base_preferred_radius(30.0),
              base_advance_bias(0.0), max_speed(18.0), acceleration(4.0),
              deceleration(3.5), turn_speed(60.0), current_speed(0.0), optimal_range(180.0),
              max_range(240.0), base_damage(4.0), flank(false),
              base_flank(false), requires_escort(false), role(SHIP_ROLE_LINE),
              max_hp(0), max_shield(0), normal_behavior(SHIP_BEHAVIOR_LINE_HOLD),
              outnumbered_behavior(SHIP_BEHAVIOR_RETREAT),
              unescorted_behavior(SHIP_BEHAVIOR_WITHDRAW_SUPPORT),
              low_hp_behavior(SHIP_BEHAVIOR_RETREAT), hp_ratio(1.0),
              shield_ratio(1.0), heading_x(0.0), heading_y(0.0), heading_z(0.0)
        {}
    };

    struct ft_combat_encounter
    {
        int                     planet_id;
        ft_sharedptr<ft_vector<int> > fleet_ids;
        ft_sharedptr<ft_fleet>  raider_fleet;
        ft_combat_modifiers     modifiers;
        double                  attack_multiplier;
        double                  defense_multiplier;
        double                  energy_pressure;
        double                  narrative_pressure;
        double                  spike_timer;
        double                  spike_time_remaining;
        double                  pending_shield_support;
        double                  pending_hull_support;
        double                  manual_focus_remaining;
        double                  manual_focus_cooldown;
        double                  tactical_pause_remaining;
        double                  tactical_pause_cooldown;
        double                  elapsed;
        double                  raider_aggression;
        int                     control_mode;
        bool                    active;
        ft_map<int, ft_ship_tracker> raider_tracks;
        ft_map<int, ft_ship_tracker> defender_tracks;
        double                  raider_frontline;
        double                  defender_line;
        double                  formation_time;
        int                     raider_operational_ships;
        int                     defender_operational_ships;
        int                     raider_line_ships;
        int                     defender_line_ships;
        int                     raider_support_ships;
        int                     defender_support_ships;
        int                     sunflare_target_fleet_id;
        int                     sunflare_target_ship_uid;
        double                  sunflare_focus_pool;
        double                  auto_generator_stability;
        bool                    auto_repair_drones_active;
        ft_combat_encounter()
            : planet_id(0), fleet_ids(new ft_vector<int>()), raider_fleet(), modifiers(),
              attack_multiplier(1.0), defense_multiplier(1.0), energy_pressure(0.0),
              narrative_pressure(0.0), spike_timer(0.0), spike_time_remaining(0.0),
              pending_shield_support(0.0), pending_hull_support(0.0),
              manual_focus_remaining(0.0), manual_focus_cooldown(0.0),
              tactical_pause_remaining(0.0), tactical_pause_cooldown(0.0),
              elapsed(0.0), raider_aggression(1.0), control_mode(ASSAULT_CONTROL_AUTO), active(false),
              raider_tracks(), defender_tracks(), raider_frontline(100.0),
              defender_line(-40.0), formation_time(0.0),
              raider_operational_ships(0), defender_operational_ships(0),
              raider_line_ships(0), defender_line_ships(0),
              raider_support_ships(0), defender_support_ships(0),
              sunflare_target_fleet_id(0), sunflare_target_ship_uid(0),
              sunflare_focus_pool(0.0), auto_generator_stability(0.0),
              auto_repair_drones_active(false)
        {}
    };

    ft_map<int, ft_combat_encounter> _encounters;
    double                           _player_weapon_multiplier;
    double                           _player_shield_multiplier;
    double                           _player_hull_multiplier;

    void gather_defenders(const ft_combat_encounter &encounter,
        ft_map<int, ft_sharedptr<ft_fleet> > &fleets,
        ft_map<int, ft_sharedptr<ft_fleet> > &planet_fleets,
        ft_vector<ft_sharedptr<ft_fleet> > &out) const;

    double calculate_side_power(const ft_map<int, ft_ship_tracker> &tracks,
        double opposing_frontline, bool raider_side) const;
    double compute_tracker_contribution(const ft_ship_tracker &tracker,
        double distance) const;
    double calculate_player_power(const ft_combat_encounter &encounter) const;
    double calculate_raider_power(const ft_combat_encounter &encounter) const;

    int add_raider_ship(ft_fleet &fleet, int ship_type, int base_hp,
        int base_shield, int armor, double scale) const;

    void build_raider_fleet(ft_combat_encounter &encounter, double difficulty,
        double energy_pressure, double narrative_pressure);

    void apply_support(ft_combat_encounter &encounter,
        ft_vector<ft_sharedptr<ft_fleet> > &defenders,
        double seconds);

    void apply_raider_support(ft_combat_encounter &encounter, double seconds,
        bool spike_active);

    void sync_raider_tracks(ft_combat_encounter &encounter);
    void sync_defender_tracks(ft_combat_encounter &encounter,
        const ft_vector<ft_sharedptr<ft_fleet> > &defenders);
    void initialize_tracker(ft_ship_tracker &tracker, int ship_uid,
        const ft_ship &ship, bool raider_side,
        const ft_combat_encounter &encounter);
    void update_formations(ft_combat_encounter &encounter, double seconds,
        bool spike_active);
    void update_tracks(ft_map<int, ft_ship_tracker> &tracks,
        ft_combat_encounter &encounter, double seconds, bool raider_side,
        bool spike_active);
    void compute_target(const ft_combat_encounter &encounter,
        const ft_ship_tracker &tracker, bool raider_side, bool spike_active,
        double &out_x, double &out_y, double &out_z) const;
    void apply_behavior_adjustments(const ft_ship_tracker &tracker, int behavior,
        bool raider_side, bool spike_active, double aggression, double &radius,
        double &advance, bool &flank, double &desired_speed) const;

public:
    CombatManager();

    void set_player_weapon_multiplier(double value);
    void set_player_shield_multiplier(double value);
    void set_player_hull_multiplier(double value);

    bool start_raider_assault(int planet_id, double difficulty,
        double energy_pressure, double narrative_pressure, int control_mode);
    bool add_fleet(int planet_id, int fleet_id);
    bool set_support(int planet_id, bool sunflare_docked,
        bool repair_drones_active, bool shield_generator_online);
    bool set_sunflare_dock_target(int planet_id, int fleet_id, int ship_uid);
    bool set_control_mode(int planet_id, int control_mode);
    bool set_raider_aggression(int planet_id, double aggression);
    bool trigger_focus_fire(int planet_id);
    bool request_tactical_pause(int planet_id);
    bool is_assault_active(int planet_id) const;
    void get_active_planets(ft_vector<int> &out) const;
    void set_auto_shield_generator(int planet_id, double stability);
    double get_raider_shield(int planet_id) const;
    double get_raider_hull(int planet_id) const;
    double get_elapsed(int planet_id) const;
    bool get_raider_positions(int planet_id, ft_vector<ft_ship_spatial_state> &out) const;
    bool get_defender_positions(int planet_id, ft_vector<ft_ship_spatial_state> &out) const;
    void tick(double seconds, ft_map<int, ft_sharedptr<ft_fleet> > &fleets,
        ft_map<int, ft_sharedptr<ft_fleet> > &planet_fleets,
        ft_vector<int> &completed, ft_vector<int> &failed);
};

#endif
