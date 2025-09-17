#ifndef GAME_HPP
#define GAME_HPP

#include "backend_client.hpp"
#include "planets.hpp"
#include "fleets.hpp"
#include "research.hpp"
#include "quests.hpp"
#include "combat.hpp"
#include "buildings.hpp"
#include "../libft/Game/game_state.hpp"
#include "../libft/Template/map.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/pair.hpp"

#define GAME_DIFFICULTY_EASY 1
#define GAME_DIFFICULTY_STANDARD 2
#define GAME_DIFFICULTY_HARD 3

class Game
{
private:
    struct RouteKey
    {
        int origin;
        int destination;

        RouteKey() : origin(0), destination(0) {}
        RouteKey(int o, int d) : origin(o), destination(d) {}

        bool operator==(const RouteKey &other) const
        {
            return this->origin == other.origin && this->destination == other.destination;
        }
    };

    friend int verify_supply_route_key_collisions();

    ft_game_state                                 _state;
    ft_map<int, ft_sharedptr<ft_planet> >         _planets;
    ft_map<int, ft_sharedptr<ft_planet> >         _locked_planets;
    ft_map<int, ft_sharedptr<ft_fleet> >          _fleets;
    ft_map<int, ft_sharedptr<ft_fleet> >          _planet_fleets;
    BackendClient                                _backend;
    ResearchManager                              _research;
    QuestManager                                 _quests;
    CombatManager                                _combat;
    BuildingManager                              _buildings;
    ft_vector<ft_string>                         _lore_log;
    int                                          _difficulty;
    double                                       _resource_multiplier;
    double                                       _quest_time_scale;
    double                                       _research_duration_scale;
    double                                       _assault_difficulty_multiplier;
    double                                       _ship_weapon_multiplier;
    double                                       _ship_shield_multiplier;
    double                                       _ship_hull_multiplier;
    int                                          _capital_ship_limit;
    bool                                         _repair_drones_unlocked;
    bool                                         _shield_support_unlocked;
    bool                                         _escape_pod_protocol;
    ft_map<int, bool>                            _escape_pod_rescued;
    struct ft_supply_route
    {
        int     id;
        int     origin_planet_id;
        int     destination_planet_id;
        double  base_travel_time;
        int     escort_requirement;
        double  base_raid_risk;
        ft_supply_route()
            : id(0), origin_planet_id(0), destination_planet_id(0),
              base_travel_time(30.0), escort_requirement(0), base_raid_risk(0.02)
        {}
    };
    struct ft_supply_convoy
    {
        int     id;
        int     route_id;
        int     origin_planet_id;
        int     destination_planet_id;
        int     resource_id;
        int     amount;
        double  remaining_time;
        double  raid_meter;
        int     origin_escort;
        int     destination_escort;
        bool    raided;
        bool    destroyed;
        ft_supply_convoy()
            : id(0), route_id(0), origin_planet_id(0), destination_planet_id(0),
              resource_id(0), amount(0), remaining_time(0.0), raid_meter(0.0),
              origin_escort(0), destination_escort(0), raided(false),
              destroyed(false)
        {}
    };
    ft_map<RouteKey, ft_supply_route>            _supply_routes;
    ft_map<int, RouteKey>                        _route_lookup;
    ft_map<int, ft_supply_convoy>                _active_convoys;
    ft_map<int, ft_map<int, double> >            _resource_deficits;
    int                                          _next_route_id;
    int                                          _next_convoy_id;

    ft_sharedptr<ft_planet> get_planet(int id);
    ft_sharedptr<const ft_planet> get_planet(int id) const;
    ft_sharedptr<ft_fleet> get_fleet(int id);
    ft_sharedptr<const ft_fleet> get_fleet(int id) const;
    ft_sharedptr<ft_fleet> get_planet_fleet(int id);
    ft_sharedptr<const ft_fleet> get_planet_fleet(int id) const;
    void send_state(int planet_id, int ore_id);
    void unlock_planet(int planet_id);
    bool can_pay_research_cost(const ft_vector<Pair<int, int> > &costs) const;
    void pay_research_cost(const ft_vector<Pair<int, int> > &costs);
    void handle_research_completion(int research_id);
    void build_quest_context(ft_quest_context &context) const;
    void handle_quest_completion(int quest_id);
    void handle_quest_failure(int quest_id);
    void handle_quest_choice_prompt(int quest_id);
    void handle_quest_choice_resolution(int quest_id, int choice_id);
    void configure_difficulty(int difficulty);
    void update_combat_modifiers();
    int count_capital_ships_in_collection(const ft_map<int, ft_sharedptr<ft_fleet> > &collection) const;
    int count_capital_ships() const;
    void clear_escape_pod_records(const ft_fleet &fleet);
    bool is_ship_type_available(int ship_type) const;
    RouteKey compose_route_key(int origin, int destination) const;
    ft_supply_route *ensure_supply_route(int origin, int destination);
    const ft_supply_route *get_route_by_id(int route_id) const;
    double estimate_route_travel_time(int origin, int destination) const;
    int estimate_route_escort_requirement(int origin, int destination) const;
    double estimate_route_raid_risk(int origin, int destination) const;
    int calculate_planet_escort_rating(int planet_id) const;
    int calculate_fleet_escort_rating(const ft_fleet &fleet) const;
    double calculate_convoy_travel_time(const ft_supply_route &route, int origin_escort, int destination_escort) const;
    double calculate_convoy_raid_risk(const ft_supply_convoy &convoy, bool origin_under_attack, bool destination_under_attack) const;
    void handle_convoy_raid(ft_supply_convoy &convoy, bool origin_under_attack, bool destination_under_attack);
    void finalize_convoy(const ft_supply_convoy &convoy);
    void advance_convoys(double seconds);

public:
    Game(const ft_string &host, const ft_string &path, int difficulty = GAME_DIFFICULTY_STANDARD);
    ~Game();

    void produce(double seconds);
    void tick(double seconds);

    int get_difficulty() const { return this->_difficulty; }

    bool is_planet_unlocked(int planet_id) const;

    bool can_place_building(int planet_id, int building_id, int x, int y) const;
    int place_building(int planet_id, int building_id, int x, int y);
    bool remove_building(int planet_id, int instance_id);
    int get_building_instance(int planet_id, int x, int y) const;
    int get_building_count(int planet_id, int building_id) const;
    int get_planet_plot_capacity(int planet_id) const;
    int get_planet_plot_usage(int planet_id) const;
    int get_planet_logistic_capacity(int planet_id) const;
    int get_planet_logistic_usage(int planet_id) const;
    double get_planet_energy_generation(int planet_id) const;
    double get_planet_energy_consumption(int planet_id) const;
    double get_planet_mine_multiplier(int planet_id) const;
    double get_planet_energy_pressure(int planet_id) const;
    void ensure_planet_item_slot(int planet_id, int resource_id);

    bool can_start_research(int research_id) const;
    bool start_research(int research_id);
    int get_research_status(int research_id) const;
    double get_research_time_remaining(int research_id) const;

    int get_active_quest() const;
    int get_quest_status(int quest_id) const;
    double get_quest_time_remaining(int quest_id) const;
    bool resolve_quest_choice(int quest_id, int choice_id);
    int get_quest_choice(int quest_id) const;

    bool start_raider_assault(int planet_id, double difficulty, int control_mode = ASSAULT_CONTROL_AUTO);
    bool assign_fleet_to_assault(int planet_id, int fleet_id);
    bool set_assault_support(int planet_id, bool sunflare_docked,
                             bool repair_drones_active, bool shield_generator_online);
    bool set_assault_control_mode(int planet_id, int control_mode);
    bool set_assault_aggression(int planet_id, double aggression);
    bool trigger_assault_focus_fire(int planet_id);
    bool request_assault_tactical_pause(int planet_id);
    bool is_assault_active(int planet_id) const;
    double get_assault_raider_shield(int planet_id) const;
    double get_assault_raider_hull(int planet_id) const;
    bool get_assault_raider_positions(int planet_id, ft_vector<ft_ship_spatial_state> &out) const;
    bool get_assault_defender_positions(int planet_id, ft_vector<ft_ship_spatial_state> &out) const;
    const ft_vector<ft_string> &get_lore_log() const;

    int add_ore(int planet_id, int ore_id, int amount);
    int sub_ore(int planet_id, int ore_id, int amount);
    int get_ore(int planet_id, int ore_id) const;
    void set_ore(int planet_id, int ore_id, int amount);
    int transfer_ore(int from_planet_id, int to_planet_id, int ore_id, int amount);
    double get_rate(int planet_id, int ore_id) const;
    const ft_vector<Pair<int, double> > &get_planet_resources(int planet_id) const;
    int get_active_convoy_count() const;

    double get_ship_weapon_multiplier() const { return this->_ship_weapon_multiplier; }
    double get_ship_shield_multiplier() const { return this->_ship_shield_multiplier; }
    double get_ship_hull_multiplier() const { return this->_ship_hull_multiplier; }

    void create_fleet(int fleet_id);
    void remove_fleet(int fleet_id, int target_fleet_id = -1, int target_planet_id = -1);
    int create_ship(int fleet_id, int ship_type);
    void remove_ship(int fleet_id, int ship_uid);
    bool transfer_ship(int from_fleet_id, int to_fleet_id, int ship_uid);

    void set_ship_armor(int fleet_id, int ship_uid, int value);
    int get_ship_armor(int fleet_id, int ship_uid) const;
    int add_ship_armor(int fleet_id, int ship_uid, int amount);
    int sub_ship_armor(int fleet_id, int ship_uid, int amount);

    void set_ship_hp(int fleet_id, int ship_uid, int value);
    int get_ship_hp(int fleet_id, int ship_uid) const;
    int add_ship_hp(int fleet_id, int ship_uid, int amount);
    int sub_ship_hp(int fleet_id, int ship_uid, int amount);

    void set_ship_shield(int fleet_id, int ship_uid, int value);
    int get_ship_shield(int fleet_id, int ship_uid) const;
    int add_ship_shield(int fleet_id, int ship_uid, int amount);
    int sub_ship_shield(int fleet_id, int ship_uid, int amount);

    void set_fleet_location_planet(int fleet_id, int planet_id);
    void set_fleet_location_travel(int fleet_id, int from, int to, double time);
    void set_fleet_location_misc(int fleet_id, int misc_id);
    ft_location get_fleet_location(int fleet_id) const;
    double get_fleet_travel_time(int fleet_id) const;
    int get_planet_fleet_ship_hp(int planet_id, int ship_uid) const;
    ft_location get_planet_fleet_location(int planet_id) const;
};

#endif
