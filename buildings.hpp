#ifndef BUILDINGS_HPP
#define BUILDINGS_HPP

#include "planets.hpp"
#include "libft/Template/map.hpp"
#include "libft/Template/vector.hpp"
#include "libft/Template/pair.hpp"
#include "libft/Template/shared_ptr.hpp"
#include "libft/CPP_class/class_string_class.hpp"
#include "libft/Libft/limits.hpp"

static const int FT_BUILDING_INSTANCE_ID_MAX = FT_INT_MAX - 1;

class Game;
class SaveSystem;

enum e_building_id
{
    BUILDING_MINE_CORE = 1,
    BUILDING_SMELTER,
    BUILDING_PROCESSOR,
    BUILDING_CRAFTING_BAY,
    BUILDING_CONVEYOR,
    BUILDING_TRANSFER_NODE,
    BUILDING_POWER_GENERATOR,
    BUILDING_SOLAR_ARRAY,
    BUILDING_UPGRADE_STATION,
    BUILDING_FACILITY_WORKSHOP,
    BUILDING_SHIPYARD,
    BUILDING_PROXIMITY_RADAR,
    BUILDING_MOBILE_RADAR,
    BUILDING_SHIELD_GENERATOR,
    BUILDING_TRITIUM_EXTRACTOR,
    BUILDING_DEFENSE_TURRET,
    BUILDING_PLASMA_TURRET,
    BUILDING_RAILGUN_TURRET,
    BUILDING_FLAGSHIP_DOCK,
    BUILDING_HELIOS_BEACON,
    BUILDING_TRADE_RELAY
};

struct ft_building_definition
{
    int                         id;
    ft_string                   name;
    int                         width;
    int                         height;
    int                         logistic_cost;
    int                         logistic_gain;
    double                      energy_cost;
    double                      energy_gain;
    double                      cycle_time;
    ft_vector<Pair<int, int> >  inputs;
    ft_vector<Pair<int, int> >  outputs;
    ft_vector<Pair<int, int> >  build_costs;
    double                      mine_bonus;
    double                      convoy_speed_bonus;
    double                      convoy_raid_risk_modifier;
    bool                        unique;
    bool                        occupies_grid;
    bool                        removable;

    ft_building_definition()
        : id(0), name(), width(0), height(0), logistic_cost(0), logistic_gain(0),
          energy_cost(0.0), energy_gain(0.0), cycle_time(0.0), inputs(), outputs(),
          build_costs(), mine_bonus(0.0), convoy_speed_bonus(0.0),
          convoy_raid_risk_modifier(0.0), unique(false), occupies_grid(false),
          removable(false)
    {}
};

struct ft_building_instance
{
    int     uid;
    int     definition_id;
    int     x;
    int     y;
    double  progress;
    bool    active;
    ft_building_instance() : uid(0), definition_id(0), x(0), y(0), progress(0.0), active(false) {}
};

struct ft_planet_build_state
{
    int                         planet_id;
    int                         width;
    int                         height;
    int                         base_logistic;
    int                         research_logistic_bonus;
    int                         used_plots;
    int                         logistic_capacity;
    int                         logistic_usage;
    double                      base_energy_generation;
    double                      energy_generation;
    double                      energy_consumption;
    double                      support_energy;
    double                      mine_multiplier;
    double                      convoy_speed_bonus;
    double                      convoy_raid_risk_modifier;
    double                      energy_deficit_pressure;
    int                         next_instance_id;
    ft_vector<int>              grid;
    ft_map<int, ft_building_instance> instances;

    ft_planet_build_state();
    ft_planet_build_state(const ft_planet_build_state &other);
    ft_planet_build_state &operator=(const ft_planet_build_state &other);
    ft_planet_build_state(ft_planet_build_state &&other) noexcept;
    ft_planet_build_state &operator=(ft_planet_build_state &&other) noexcept;
};

class BuildingManager
{
private:
    friend class SaveSystem;
    friend class Game;

    ft_map<int, ft_sharedptr<ft_building_definition> > _definitions;
    ft_map<int, ft_planet_build_state>                  _planets;
    ft_map<int, bool>                                   _building_unlocks;
    double                                             _crafting_energy_multiplier;
    double                                             _crafting_speed_multiplier;
    double                                             _global_energy_multiplier;

    void register_definition(const ft_sharedptr<ft_building_definition> &definition);
    const ft_building_definition *get_definition(int building_id) const;
    void set_building_unlocked(int building_id, bool unlocked);
    bool is_building_unlocked(int building_id) const;
    bool is_area_free(const ft_planet_build_state &state, int x, int y, int width, int height) const;
    void occupy_area(ft_planet_build_state &state, int instance_id, int x, int y, int width, int height);
    void clear_area(ft_planet_build_state &state, int instance_id);
    void recalculate_planet_statistics(ft_planet_build_state &state);
    bool check_build_costs(const Game &game, int planet_id, const ft_vector<Pair<int, int> > &costs) const;
    void pay_build_costs(Game &game, int planet_id, const ft_vector<Pair<int, int> > &costs);
    bool consume_inputs(Game &game, int planet_id, const ft_vector<Pair<int, int> > &inputs);
    void produce_outputs(Game &game, int planet_id, const ft_vector<Pair<int, int> > &outputs);
    void ensure_outputs_registered(Game &game, int planet_id, const ft_vector<Pair<int, int> > &outputs);
    void tick_planet(Game &game, ft_planet_build_state &state, double seconds);
    ft_planet_build_state *get_state(int planet_id);
    const ft_planet_build_state *get_state(int planet_id) const;

public:
    BuildingManager();

    void clone_from(const BuildingManager &other);

    void initialize_planet(Game &game, int planet_id);
    void add_planet_logistic_bonus(int planet_id, int amount);
    void apply_research_unlock(int research_id);
    void set_crafting_energy_multiplier(double multiplier);
    void set_crafting_speed_multiplier(double multiplier);
    void set_global_energy_multiplier(double multiplier);

    int place_building(Game &game, int planet_id, int building_id, int x, int y);
    bool remove_building(Game &game, int planet_id, int instance_id);
    bool can_place_building(const Game &game, int planet_id, int building_id, int x, int y) const;

    int get_building_instance(int planet_id, int x, int y) const;
    int get_building_count(int planet_id, int building_id) const;

    int get_planet_plot_capacity(int planet_id) const;
    int get_planet_plot_usage(int planet_id) const;
    int get_planet_logistic_capacity(int planet_id) const;
    int get_planet_logistic_usage(int planet_id) const;
    double get_planet_energy_generation(int planet_id) const;
    double get_planet_energy_consumption(int planet_id) const;
    double get_mine_multiplier(int planet_id) const;
    double get_planet_energy_pressure(int planet_id) const;
    double get_planet_convoy_speed_bonus(int planet_id) const;
    double get_planet_convoy_raid_risk_modifier(int planet_id) const;

    void tick(Game &game, double seconds);
};

#endif
