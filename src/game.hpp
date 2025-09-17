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

#define GAME_DIFFICULTY_EASY 1
#define GAME_DIFFICULTY_STANDARD 2
#define GAME_DIFFICULTY_HARD 3

class Game
{
private:
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

    bool start_raider_assault(int planet_id, double difficulty);
    bool assign_fleet_to_assault(int planet_id, int fleet_id);
    bool set_assault_support(int planet_id, bool sunflare_docked,
                             bool repair_drones_active, bool shield_generator_online);
    bool is_assault_active(int planet_id) const;
    double get_assault_raider_shield(int planet_id) const;
    double get_assault_raider_hull(int planet_id) const;
    const ft_vector<ft_string> &get_lore_log() const;

    int add_ore(int planet_id, int ore_id, int amount);
    int sub_ore(int planet_id, int ore_id, int amount);
    int get_ore(int planet_id, int ore_id) const;
    void set_ore(int planet_id, int ore_id, int amount);
    int transfer_ore(int from_planet_id, int to_planet_id, int ore_id, int amount);
    double get_rate(int planet_id, int ore_id) const;
    const ft_vector<Pair<int, double> > &get_planet_resources(int planet_id) const;

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
