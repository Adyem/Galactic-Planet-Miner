#ifndef GAME_HPP
#define GAME_HPP

#include "backend_client.hpp"
#include "planets.hpp"
#include "fleets.hpp"
#include "research.hpp"
#include "quests.hpp"
#include "combat.hpp"
#include "buildings.hpp"
#include "achievements.hpp"
#include "save_system.hpp"
#include "player_profile.hpp"
#include "libft/Game/game_state.hpp"
#include "libft/Template/map.hpp"
#include "libft/Template/vector.hpp"
#include "libft/Template/pair.hpp"

#define GAME_DIFFICULTY_EASY 1
#define GAME_DIFFICULTY_STANDARD 2
#define GAME_DIFFICULTY_HARD 3

enum e_journal_entry_id
{
    JOURNAL_ENTRY_PROFILE_MINER_JOE = 1,
    JOURNAL_ENTRY_PROFILE_PROFESSOR_LUMEN,
    JOURNAL_ENTRY_PROFILE_FARMER_DAISY,
    JOURNAL_ENTRY_PROFILE_BLACKTHORNE,
    JOURNAL_ENTRY_PROFILE_NAVIGATOR_ZARA,
    JOURNAL_ENTRY_PROFILE_SCOUT_FINN,
    JOURNAL_ENTRY_INITIAL_RAIDER_SKIRMISHES,
    JOURNAL_ENTRY_DEFENSE_OF_TERRA,
    JOURNAL_ENTRY_INVESTIGATE_RAIDERS,
    JOURNAL_ENTRY_SECURE_SUPPLY_LINES,
    JOURNAL_ENTRY_STEADY_SUPPLY_STREAK,
    JOURNAL_ENTRY_HIGH_VALUE_ESCORT,
    JOURNAL_ENTRY_CLIMACTIC_BATTLE,
    JOURNAL_ENTRY_DECISION_EXECUTE_BLACKTHORNE,
    JOURNAL_ENTRY_DECISION_SPARE_BLACKTHORNE,
    JOURNAL_ENTRY_ORDER_UPRISING,
    JOURNAL_ENTRY_ORDER_SUPPRESS_RAIDS,
    JOURNAL_ENTRY_ORDER_DOMINION,
    JOURNAL_ENTRY_REBELLION_FLEET,
    JOURNAL_ENTRY_REBELLION_NETWORK,
    JOURNAL_ENTRY_REBELLION_LIBERATION,
    JOURNAL_ENTRY_ORDER_FINAL_MANDATE,
    JOURNAL_ENTRY_ORDER_VERDICT_EXECUTION,
    JOURNAL_ENTRY_ORDER_VERDICT_REFORM,
    JOURNAL_ENTRY_REBELLION_FINAL_PUSH,
    JOURNAL_ENTRY_LORE_TERRA_REBUILD,
    JOURNAL_ENTRY_LORE_MARS_OUTPOSTS,
    JOURNAL_ENTRY_LORE_ZALTHOR_ANOMALY,
    JOURNAL_ENTRY_LORE_CONVOY_CORPS,
    JOURNAL_ENTRY_LORE_EMERGENCY_GRID,
    JOURNAL_ENTRY_RESOURCE_IRON_FOUNDATION,
    JOURNAL_ENTRY_RESOURCE_COPPER_NETWORK,
    JOURNAL_ENTRY_RESOURCE_MITHRIL_MYSTERIES,
    JOURNAL_ENTRY_RESOURCE_COAL_BARRICADES,
    JOURNAL_ENTRY_RESOURCE_TITANIUM_BULWARK,
    JOURNAL_ENTRY_RESOURCE_OBSIDIAN_ALLIANCE,
    JOURNAL_ENTRY_RESOURCE_CRYSTAL_INTRIGUE,
    JOURNAL_ENTRY_RESOURCE_NANOMATERIAL_RENEWAL,
    JOURNAL_ENTRY_RAIDER_SIGNAL_WEB,
    JOURNAL_ENTRY_SIDE_CONVOY_RESCUE,
    JOURNAL_ENTRY_SIDE_OUTPOST_REPAIR,
    JOURNAL_ENTRY_SIDE_ORDER_PROPAGANDA,
    JOURNAL_ENTRY_SIDE_REBELLION_BROADCAST,
    JOURNAL_ENTRY_LORE_CELESTIAL_BARRENS,
    JOURNAL_ENTRY_LORE_NEBULA_OUTPOST,
    JOURNAL_ENTRY_LORE_IMPERIUM_PRESSURE,
    JOURNAL_ENTRY_COMBAT_VICTORY_RAIDER_BROADCAST,
    JOURNAL_ENTRY_COMBAT_VICTORY_DEFENSE_DEBRIEF,
    JOURNAL_ENTRY_COMBAT_VICTORY_LIBERATION_SIGNAL
};

class Game
{
public:
    struct ft_supply_contract
    {
        int     id;
        int     origin_planet_id;
        int     destination_planet_id;
        int     resource_id;
        int     shipment_size;
        double  interval_seconds;
        double  elapsed_seconds;
        bool    has_minimum_stock;
        int     minimum_stock;
        int     max_active_convoys;
        int     pending_delivery;

        ft_supply_contract()
            : id(0), origin_planet_id(0), destination_planet_id(0),
              resource_id(0), shipment_size(0), interval_seconds(0.0),
              elapsed_seconds(0.0), has_minimum_stock(false),
              minimum_stock(0), max_active_convoys(1),
              pending_delivery(0)
        {}
    };

    struct ft_resource_dashboard_entry
    {
        int     resource_id;
        int     total_stock;
        double  production_rate;

        ft_resource_dashboard_entry()
            : resource_id(0), total_stock(0), production_rate(0.0)
        {}
    };

    struct ft_route_dashboard_entry
    {
        int     route_id;
        int     origin_planet_id;
        int     destination_planet_id;
        double  threat_level;
        double  quiet_timer;
        int     active_convoys;
        bool    escalation_pending;

        ft_route_dashboard_entry()
            : route_id(0), origin_planet_id(0), destination_planet_id(0),
              threat_level(0.0), quiet_timer(0.0), active_convoys(0),
              escalation_pending(false)
        {}
    };

    struct ft_resource_dashboard
    {
        ft_vector<ft_resource_dashboard_entry> resources;
        ft_vector<ft_route_dashboard_entry>    routes;
        int                                    total_active_convoys;
        double                                 average_route_threat;
        double                                 maximum_route_threat;

        ft_resource_dashboard()
            : resources(), routes(), total_active_convoys(0),
              average_route_threat(0.0), maximum_route_threat(0.0)
        {}
    };

    struct ft_building_grid_cell_snapshot
    {
        int x;
        int y;
        int instance_id;
        int building_id;

        ft_building_grid_cell_snapshot()
            : x(0), y(0), instance_id(0), building_id(0)
        {}
    };

    struct ft_building_instance_snapshot
    {
        int                         instance_id;
        int                         building_id;
        ft_string                   name;
        int                         x;
        int                         y;
        int                         width;
        int                         height;
        double                      progress;
        bool                        active;
        int                         logistic_cost;
        int                         logistic_gain;
        double                      energy_cost;
        double                      energy_gain;
        double                      cycle_time;
        double                      mine_bonus;
        double                      convoy_speed_bonus;
        double                      convoy_raid_risk_modifier;
        bool                        unique;
        bool                        occupies_grid;
        bool                        removable;
        ft_vector<Pair<int, int> >  inputs;
        ft_vector<Pair<int, int> >  outputs;
        ft_vector<Pair<int, int> >  build_costs;

        ft_building_instance_snapshot()
            : instance_id(0), building_id(0), name(), x(0), y(0), width(0),
              height(0), progress(0.0), active(false), logistic_cost(0),
              logistic_gain(0), energy_cost(0.0), energy_gain(0.0),
              cycle_time(0.0), mine_bonus(0.0), convoy_speed_bonus(0.0),
              convoy_raid_risk_modifier(0.0), unique(false),
              occupies_grid(false), removable(false), inputs(), outputs(),
              build_costs()
        {}
    };

    struct ft_planet_building_snapshot
    {
        int                                 planet_id;
        ft_string                           planet_name;
        int                                 width;
        int                                 height;
        int                                 base_logistic;
        int                                 research_logistic_bonus;
        int                                 used_plots;
        int                                 logistic_capacity;
        int                                 logistic_usage;
        int                                 logistic_available;
        double                              base_energy_generation;
        double                              energy_generation;
        double                              energy_consumption;
        double                              support_energy;
        double                              energy_deficit_pressure;
        double                              mine_multiplier;
        double                              convoy_speed_bonus;
        double                              convoy_raid_risk_modifier;
        bool                                emergency_energy_conservation;
        ft_vector<ft_building_grid_cell_snapshot> grid;
        ft_vector<ft_building_instance_snapshot>  instances;

        ft_planet_building_snapshot()
            : planet_id(0), planet_name(), width(0), height(0),
              base_logistic(0), research_logistic_bonus(0), used_plots(0),
              logistic_capacity(0), logistic_usage(0), logistic_available(0),
              base_energy_generation(0.0), energy_generation(0.0),
              energy_consumption(0.0), support_energy(0.0),
              energy_deficit_pressure(0.0), mine_multiplier(1.0),
              convoy_speed_bonus(0.0), convoy_raid_risk_modifier(0.0),
              emergency_energy_conservation(false),
              grid(), instances()
        {}
    };

    struct ft_building_layout_snapshot
    {
        ft_vector<ft_planet_building_snapshot> planets;

        ft_building_layout_snapshot() : planets() {}
    };

    struct ft_quest_objective_snapshot
    {
        int     type;
        int     target_id;
        int     required_amount;
        double  current_amount;
        bool    is_met;

        ft_quest_objective_snapshot()
            : type(0), target_id(0), required_amount(0),
              current_amount(0.0), is_met(false)
        {}
    };

    enum e_story_act_id
    {
        STORY_ACT_RISING_THREAT = 1,
        STORY_ACT_UNRAVELING_TRUTH = 2,
        STORY_ACT_CROSSROADS = 3
    };

    enum e_story_branch_id
    {
        STORY_BRANCH_NONE = 0,
        STORY_BRANCH_ORDER_DOMINION = 1,
        STORY_BRANCH_REBELLION_LIBERATION = 2
    };

    struct ft_quest_choice_snapshot
    {
        int         choice_id;
        ft_string   description;
        bool        is_selected;
        bool        is_available;

        ft_quest_choice_snapshot()
            : choice_id(0), description(), is_selected(false),
              is_available(false)
        {}
    };

    struct ft_quest_log_entry
    {
        int                                      quest_id;
        ft_string                                name;
        ft_string                                description;
        int                                      status;
        double                                   time_remaining;
        double                                   time_limit;
        double                                   time_remaining_ratio;
        bool                                     is_side_quest;
        bool                                     requires_choice;
        bool                                     awaiting_choice;
        bool                                     objectives_completed;
        bool                                     prerequisites_met;
        bool                                     branch_requirement_met;
        int                                      selected_choice;
        ft_vector<ft_quest_objective_snapshot>   objectives;
        ft_vector<ft_quest_choice_snapshot>      choices;

        ft_quest_log_entry()
            : quest_id(0), name(), description(), status(QUEST_STATUS_LOCKED),
              time_remaining(0.0), time_limit(0.0), time_remaining_ratio(0.0), is_side_quest(false),
              requires_choice(false), awaiting_choice(false),
              objectives_completed(false), prerequisites_met(false),
              branch_requirement_met(true), selected_choice(QUEST_CHOICE_NONE),
              objectives(), choices()
        {}
    };

    struct ft_story_branch_snapshot
    {
        int         branch_id;
        ft_string   name;
        ft_string   summary;
        bool        is_active;
        bool        is_available;
        int         completed_quests;
        int         total_quests;

        ft_story_branch_snapshot()
            : branch_id(STORY_BRANCH_NONE), name(), summary(), is_active(false),
              is_available(false), completed_quests(0), total_quests(0)
        {}
    };

    struct ft_story_choice_option_snapshot
    {
        int       choice_id;
        ft_string title;
        ft_string summary;
        bool      is_available;
        bool      is_selected;

        ft_story_choice_option_snapshot()
            : choice_id(QUEST_CHOICE_NONE), title(), summary(), is_available(false),
              is_selected(false)
        {}
    };

    struct ft_story_choice_snapshot
    {
        int                                      quest_id;
        ft_string                                title;
        bool                                     awaiting_selection;
        bool                                     has_been_made;
        ft_vector<ft_story_choice_option_snapshot> options;

        ft_story_choice_snapshot()
            : quest_id(0), title(), awaiting_selection(false), has_been_made(false),
              options()
        {}
    };

    struct ft_story_act_snapshot
    {
        int                                 act_id;
        ft_string                           name;
        ft_string                           theme;
        bool                                is_active;
        bool                                is_completed;
        bool                                awaiting_branch_choice;
        int                                 completed_main_quests;
        int                                 total_main_quests;
        ft_vector<ft_story_branch_snapshot> branches;

        ft_story_act_snapshot()
            : act_id(0), name(), theme(), is_active(false),
              is_completed(false), awaiting_branch_choice(false),
              completed_main_quests(0), total_main_quests(0), branches()
        {}
    };

    struct ft_story_epilogue_snapshot
    {
        bool                    is_available;
        ft_string               title;
        ft_vector<ft_string>    paragraphs;

        ft_story_epilogue_snapshot()
            : is_available(false), title(), paragraphs()
        {}
    };

    struct ft_quest_log_snapshot
    {
        ft_vector<ft_quest_log_entry>    main_quests;
        ft_vector<ft_quest_log_entry>    side_quests;
        ft_vector<int>                   awaiting_choice_ids;
        ft_vector<ft_string>             recent_journal_entries;
        ft_vector<ft_string>             recent_lore_entries;
        ft_vector<ft_story_choice_snapshot> critical_choices;
        ft_vector<ft_story_act_snapshot>  story_acts;
        ft_story_epilogue_snapshot        epilogue;
        int                              active_main_quest_id;
        int                              current_act_id;
        double                           ui_scale;
        int                              lore_panel_anchor;

        ft_quest_log_snapshot()
            : main_quests(), side_quests(), awaiting_choice_ids(),
              recent_journal_entries(), recent_lore_entries(), critical_choices(), story_acts(), epilogue(),
              active_main_quest_id(0), current_act_id(STORY_ACT_RISING_THREAT),
              ui_scale(1.0), lore_panel_anchor(PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT)
        {}
    };

    struct ft_ship_management_entry
    {
        int     ship_uid;
        int     ship_type;
        int     role;
        int     combat_behavior;
        int     outnumbered_behavior;
        int     unescorted_behavior;
        int     low_hp_behavior;
        int     armor;
        int     hp;
        int     max_hp;
        int     shield;
        int     max_shield;
        double  max_speed;
        double  acceleration;
        double  deceleration;
        double  turn_speed;
        double  optimal_range;
        double  max_range;
        double  attack_speed;
        double  base_damage;

        ft_ship_management_entry()
            : ship_uid(0), ship_type(0), role(0), combat_behavior(0),
              outnumbered_behavior(0), unescorted_behavior(0),
              low_hp_behavior(0), armor(0), hp(0), max_hp(0),
              shield(0), max_shield(0), max_speed(0.0),
              acceleration(0.0), deceleration(0.0), turn_speed(0.0),
              optimal_range(0.0), max_range(0.0), attack_speed(0.0),
              base_damage(0.0)
        {}
    };

    struct ft_fleet_management_entry
    {
        int                                 fleet_id;
        bool                                is_garrison;
        int                                 station_planet_id;
        int                                 location_type;
        int                                 origin_planet_id;
        int                                 destination_planet_id;
        int                                 misc_location_id;
        double                              travel_time;
        int                                 ship_count;
        int                                 total_hp;
        int                                 total_shield;
        double                              escort_veterancy;
        int                                 escort_veterancy_bonus;
        double                              average_weapon_range;
        double                              average_attack_speed;
        ft_vector<ft_ship_management_entry> ships;

        ft_fleet_management_entry()
            : fleet_id(0), is_garrison(false), station_planet_id(0),
              location_type(LOCATION_PLANET), origin_planet_id(PLANET_TERRA),
              destination_planet_id(PLANET_TERRA), misc_location_id(0),
              travel_time(0.0), ship_count(0), total_hp(0), total_shield(0),
              escort_veterancy(0.0), escort_veterancy_bonus(0),
              average_weapon_range(0.0), average_attack_speed(0.0), ships()
        {}
    };

    struct ft_fleet_management_snapshot
    {
        ft_vector<ft_fleet_management_entry> player_fleets;
        ft_vector<ft_fleet_management_entry> traveling_fleets;
        ft_vector<ft_fleet_management_entry> planet_garrisons;

        ft_fleet_management_snapshot()
            : player_fleets(), traveling_fleets(), planet_garrisons()
        {}
    };

private:
    struct RouteKey;
    struct ft_supply_route;
    struct ft_supply_convoy;

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
    friend int verify_supply_route_threat_decay();
    friend int verify_lore_log_retention();
    friend int verify_convoy_escort_rating_excludes_active_escort();
    friend int verify_celestial_barrens_salvage_event();
    friend int verify_nebula_outpost_scan_event();
    friend int verify_resource_lore_rotation();
    friend int verify_raider_lore_rotation();
    friend int verify_imperium_pressure_threshold();
    friend int verify_nanomaterial_resource_lore();
    friend int verify_fleet_management_snapshot();
    friend int verify_building_layout_snapshot();
    friend int verify_late_campaign_raider_focus();

    ft_game_state                                 _state;
    ft_map<int, ft_sharedptr<ft_planet> >         _planets;
    ft_map<int, ft_sharedptr<ft_planet> >         _locked_planets;
    ft_map<int, ft_sharedptr<ft_fleet> >          _fleets;
    ft_map<int, ft_sharedptr<ft_fleet> >          _planet_fleets;
    BackendClient                                _backend;
    SaveSystem                                   _save_system;
    ResearchManager                              _research;
    QuestManager                                 _quests;
    CombatManager                                _combat;
    BuildingManager                              _buildings;
    AchievementManager                           _achievements;
    static const size_t                          LORE_LOG_MAX_ENTRIES = 512;
    ft_vector<ft_string>                         _lore_log;
    size_t                                       _lore_log_start;
    size_t                                       _lore_log_count;
    mutable ft_vector<ft_string>                 _lore_log_cache;
    mutable bool                                 _lore_log_cache_dirty;
    ft_map<int, ft_string>                       _journal_entries;
    ft_vector<int>                               _journal_unlock_order;
    mutable ft_vector<ft_string>                 _journal_cache;
    mutable bool                                 _journal_cache_dirty;
    ft_map<int, int>                             _resource_lore_cursors;
    int                                          _raider_lore_cursor;
    int                                          _quick_completion_cursor;
    void                                        append_lore_entry(const ft_string &entry);
    void                                         unlock_journal_entry(int entry_id, const ft_string &text);
    bool                                         append_resource_lore_snippet(int resource_id, int origin_planet_id, int destination_planet_id);
    bool                                         append_raider_lore_snippet(int origin_planet_id, int destination_planet_id);
    void                                         record_combat_victory_narrative(int planet_id);
    bool                                         handle_celestial_barrens_salvage(const ft_supply_convoy &convoy);
    bool                                         handle_nebula_outpost_scan(const ft_supply_convoy &convoy);
    void                                         maybe_unlock_imperium_pressure(const ft_supply_route &route);
    bool                                         update_energy_conservation_state(int planet_id, bool active,
        bool assault_active, bool escalation_target, bool convoy_danger, bool threat_triggered);
    void                                         log_energy_conservation_transition(int planet_id, bool active,
        bool assault_active, bool escalation_target, bool convoy_danger, bool threat_triggered);
    int                                          _difficulty;
    double                                       _resource_multiplier;
    double                                       _quest_time_scale_base;
    double                                       _quest_time_scale_dynamic;
    double                                       _ui_scale;
    int                                          _lore_panel_anchor;
    double                                       _combat_speed_multiplier;
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
    bool                                         _emergency_energy_protocol;
    ft_map<int, bool>                            _energy_conservation_active;
    struct ft_supply_route
    {
        int     id;
        int     origin_planet_id;
        int     destination_planet_id;
        double  base_travel_time;
        int     escort_requirement;
        double  base_raid_risk;
        double  threat_level;
        double  quiet_timer;
        double  escalation_timer;
        bool    escalation_pending;
        int     escalation_planet_id;
        ft_supply_route()
            : id(0), origin_planet_id(0), destination_planet_id(0),
              base_travel_time(30.0), escort_requirement(0), base_raid_risk(0.02),
              threat_level(0.0), quiet_timer(0.0), escalation_timer(0.0),
              escalation_pending(false), escalation_planet_id(0)
        {}
    };
    struct ft_supply_convoy
    {
        int     id;
        int     route_id;
        int     contract_id;
        int     origin_planet_id;
        int     destination_planet_id;
        int     resource_id;
        int     amount;
        double  remaining_time;
        double  raid_meter;
        int     origin_escort;
        int     destination_escort;
        int     escort_fleet_id;
        int     escort_rating;
        bool    raided;
        bool    destroyed;
        bool    loss_recorded;
        bool    route_escort_claimed;
        ft_supply_convoy()
            : id(0), route_id(0), contract_id(0), origin_planet_id(0),
              destination_planet_id(0), resource_id(0), amount(0),
              remaining_time(0.0), raid_meter(0.0), origin_escort(0),
              destination_escort(0), escort_fleet_id(0), escort_rating(0),
              raided(false), destroyed(false), loss_recorded(false),
              route_escort_claimed(false)
        {}
    };
    ft_map<RouteKey, ft_supply_route>            _supply_routes;
    ft_map<int, RouteKey>                        _route_lookup;
    ft_map<int, ft_supply_convoy>                _active_convoys;
    ft_map<int, int>                             _route_convoy_escorts;
    ft_map<int, ft_supply_contract>              _supply_contracts;
    struct ft_resource_accumulator
    {
        double multiplier_deficit;
        double mine_bonus_remainder;

        ft_resource_accumulator()
            : multiplier_deficit(0.0), mine_bonus_remainder(0.0)
        {}
    };

    struct ft_pending_resource_update
    {
        int planet_id;
        int ore_id;
        int amount;

        ft_pending_resource_update()
            : planet_id(0), ore_id(0), amount(0)
        {}

        ft_pending_resource_update(int planet, int ore, int value)
            : planet_id(planet), ore_id(ore), amount(value)
        {}
    };

    ft_map<int, ft_sharedptr<ft_vector<Pair<int, ft_resource_accumulator> > > > _resource_deficits;
    ft_map<int, ft_sharedptr<ft_map<int, int> > > _last_sent_resources;
    ft_map<int, ft_sharedptr<ft_map<int, int> > > _pending_resource_updates;
    int                                          _next_route_id;
    int                                          _next_convoy_id;
    int                                          _next_contract_id;
    int                                          _convoys_delivered_total;
    int                                          _convoy_raid_losses;
    int                                          _current_delivery_streak;
    int                                          _longest_delivery_streak;
    ft_vector<int>                               _streak_milestones;
    size_t                                       _next_streak_milestone_index;
    int                                          _order_branch_assault_victories;
    int                                          _rebellion_branch_assault_victories;
    int                                          _order_branch_pending_assault;
    int                                          _rebellion_branch_pending_assault;
    ft_string                                    _last_planet_checkpoint;
    ft_string                                    _last_fleet_checkpoint;
    ft_string                                    _last_research_checkpoint;
    ft_string                                    _last_achievement_checkpoint;
    ft_string                                    _last_building_checkpoint;
    ft_string                                    _last_progress_checkpoint;
    ft_string                                    _last_checkpoint_tag;
    bool                                         _has_checkpoint;
    ft_vector<ft_string>                         _failed_checkpoint_tags;
    bool                                         _force_checkpoint_failure;
    bool                                         _backend_online;
    static const long                            BACKEND_RETRY_INITIAL_DELAY_MS = 1000;
    static const long                            BACKEND_RETRY_MAX_DELAY_MS = 60000;
    long                                         _backend_retry_delay_ms;
    long                                         _backend_next_retry_ms;

    ft_sharedptr<ft_planet> get_planet(int id);
    ft_sharedptr<const ft_planet> get_planet(int id) const;
    ft_sharedptr<ft_planet> get_planet_storage_target(int id);
    ft_sharedptr<const ft_planet> get_planet_storage_target(int id) const;
    ft_sharedptr<ft_fleet> get_fleet(int id);
    ft_sharedptr<const ft_fleet> get_fleet(int id) const;
    ft_sharedptr<ft_fleet> get_planet_fleet(int id);
    ft_sharedptr<const ft_fleet> get_planet_fleet(int id) const;
    void send_state(int planet_id, int ore_id);
    void queue_pending_resource_update(int planet_id, int ore_id, int amount);
    void clear_pending_resource_update(int planet_id, int ore_id);
    bool flush_pending_resource_updates();
    bool dispatch_resource_update(int planet_id, int ore_id, int amount);
    int select_planet_resource_for_assault(const ft_sharedptr<ft_planet> &planet, int minimum_stock, bool allow_stock_fallback) const noexcept;
    Pair<int, ft_resource_accumulator> *get_resource_accumulator(int planet_id, int ore_id, bool create);
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
    void configure_from_preferences(const PlayerProfilePreferences &preferences);
    void update_combat_modifiers();
    void apply_quest_time_scale();
    void update_dynamic_quest_pressure();
    void handle_quick_quest_completion(const ft_quest_definition &definition, double completion_ratio);
    double compute_auto_shield_generator_stability(int planet_id) const;
    int count_capital_ships_in_collection(const ft_map<int, ft_sharedptr<ft_fleet> > &collection) const;
    int count_capital_ships() const;
    void clear_escape_pod_records(const ft_fleet &fleet);
    bool is_ship_type_available(int ship_type) const;
    void record_achievement_event(int event_id, int value);
    int get_quest_achievement_event(int quest_id) const;
    void record_quest_achievement(int quest_id);
    void announce_achievements(const ft_vector<int> &achievement_ids);
    RouteKey compose_route_key(int origin, int destination) const;
    ft_supply_route *ensure_supply_route(int origin, int destination);
    ft_supply_route *find_supply_route(int origin, int destination);
    const ft_supply_route *find_supply_route(int origin, int destination) const;
    const ft_supply_route *get_route_by_id(int route_id) const;
    ft_supply_route *get_route_by_id(int route_id);
    double estimate_route_travel_time(int origin, int destination) const;
    int estimate_route_escort_requirement(int origin, int destination) const;
    double estimate_route_raid_risk(int origin, int destination) const;
    int calculate_planet_escort_rating(int planet_id) const;
    int calculate_fleet_escort_rating(const ft_fleet &fleet) const;
    bool is_fleet_escorting_convoy(int fleet_id) const;
    int claim_route_escort(int route_id);
    double calculate_convoy_travel_time(const ft_supply_route &route, int origin_escort, int destination_escort) const;
    double calculate_convoy_raid_risk(const ft_supply_convoy &convoy, bool origin_under_attack, bool destination_under_attack) const;
    void handle_convoy_raid(ft_supply_convoy &convoy, bool origin_under_attack, bool destination_under_attack);
    void finalize_convoy(ft_supply_convoy &convoy);
    void handle_contract_completion(const ft_supply_convoy &convoy);
    void accelerate_contract(int contract_id, double fraction);
    int count_active_convoys_for_contract(int contract_id) const;
    bool has_active_convoy_for_contract(int contract_id) const;
    void increase_contract_pending_delivery(int contract_id, int amount);
    void decrease_contract_pending_delivery(int contract_id, int amount);
    int dispatch_convoy(const ft_supply_route &route, int origin_planet_id,
                        int destination_planet_id, int resource_id, int amount,
                        int contract_id, int escort_fleet_id = 0);
    void process_supply_contracts(double seconds);
    void advance_convoys(double seconds);
    void record_convoy_delivery(const ft_supply_convoy &convoy);
    void record_convoy_loss(const ft_supply_convoy &convoy, bool destroyed_by_raid);
    void reset_delivery_streak();
    void modify_route_threat(ft_supply_route &route, double delta, bool reset_quiet_timer);
    void decay_route_threat(ft_supply_route &route, double seconds);
    void decay_all_route_threat(double seconds);
    void update_route_escalation(ft_supply_route &route, double seconds);
    void trigger_route_assault(ft_supply_route &route);
    void trigger_branch_assault(int planet_id, double difficulty, bool order_branch);
    void apply_planet_snapshot(const ft_map<int, ft_sharedptr<ft_planet> > &snapshot);
    void apply_fleet_snapshot(const ft_map<int, ft_sharedptr<ft_fleet> > &snapshot);
    bool checkpoint_campaign_state_internal(const ft_string &tag);
    void record_checkpoint_failure(const ft_string &tag) noexcept;
    void build_fleet_management_entry(const ft_fleet &fleet,
                                      ft_fleet_management_entry &out,
                                      bool is_garrison,
                                      int garrison_planet_id) const;

public:
    Game(const ft_string &host, const ft_string &path, int difficulty = GAME_DIFFICULTY_STANDARD);
    ~Game();

    void produce(double seconds);
    void update_gameplay(double milliseconds);
    void update_combat(double milliseconds);
    void tick_milliseconds(double milliseconds);
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
    double get_planet_support_energy(int planet_id) const;
    double get_planet_mine_multiplier(int planet_id) const;
    double get_planet_energy_pressure(int planet_id) const;
    bool update_planet_energy_conservation(int planet_id);
    bool is_planet_energy_conservation_active(int planet_id) const noexcept;
    int get_planet_escort_rating(int planet_id) const;
    double get_planet_convoy_raid_risk_modifier(int planet_id) const;
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
    void get_resource_dashboard(ft_resource_dashboard &out) const;
    void get_building_layout_snapshot(ft_building_layout_snapshot &out) const;
    void get_quest_log_snapshot(ft_quest_log_snapshot &out) const;
    void get_fleet_management_snapshot(ft_fleet_management_snapshot &out) const;

    bool assign_convoy_escort(int origin_planet_id, int destination_planet_id, int fleet_id);
    bool clear_convoy_escort(int origin_planet_id, int destination_planet_id);
    int get_assigned_convoy_escort(int origin_planet_id, int destination_planet_id) const;

    bool start_raider_assault(int planet_id, double difficulty, int control_mode = ASSAULT_CONTROL_AUTO);
    bool assign_fleet_to_assault(int planet_id, int fleet_id);
    bool set_assault_support(int planet_id, bool sunflare_docked,
                             bool repair_drones_active, bool shield_generator_online);
    bool set_assault_sunflare_target(int planet_id, int fleet_id, int ship_uid);
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
    bool is_backend_online() const;
    long get_backend_retry_delay_ms_for_testing() const;
    long get_backend_next_retry_ms_for_testing() const;
    double get_effective_quest_time_scale() const;
    void set_ui_scale(double scale);
    double get_ui_scale() const { return this->_ui_scale; }
    void set_lore_panel_anchor(int anchor);
    int get_lore_panel_anchor() const { return this->_lore_panel_anchor; }
    void set_combat_speed_multiplier(double multiplier);
    double get_combat_speed_multiplier() const { return this->_combat_speed_multiplier; }

    const ft_vector<ft_string> &get_journal_entries() const;
    bool is_journal_entry_unlocked(int entry_id) const;

    int add_ore(int planet_id, int ore_id, int amount);
    int sub_ore(int planet_id, int ore_id, int amount);
    int get_ore(int planet_id, int ore_id) const;
    void set_ore(int planet_id, int ore_id, int amount);
    int transfer_ore(int from_planet_id, int to_planet_id, int ore_id, int amount);
    int get_total_convoys_delivered() const { return this->_convoys_delivered_total; }
    int get_convoy_raid_losses() const { return this->_convoy_raid_losses; }
    int get_convoy_delivery_streak() const { return this->_current_delivery_streak; }
    int get_longest_convoy_delivery_streak() const { return this->_longest_delivery_streak; }
    double get_supply_route_threat_level(int origin_planet_id, int destination_planet_id) const;
    double get_rate(int planet_id, int ore_id) const;
    const ft_vector<Pair<int, double> > &get_planet_resources(int planet_id) const;
    int get_active_convoy_count() const;
    double get_fleet_escort_veterancy(int fleet_id) const;
    int get_fleet_escort_veterancy_bonus(int fleet_id) const;

    void get_achievement_ids(ft_vector<int> &out) const;
    int get_achievement_status(int achievement_id) const;
    int get_achievement_progress(int achievement_id) const;
    int get_achievement_target(int achievement_id) const;
    bool get_achievement_info(int achievement_id, ft_achievement_info &out) const;

    int create_supply_contract(int origin_planet_id, int destination_planet_id,
                               int resource_id, int shipment_size,
                               double interval_seconds,
                               int minimum_destination_stock = -1,
                               int max_active_convoys = 1);
    bool cancel_supply_contract(int contract_id);
    bool update_supply_contract(int contract_id, int shipment_size,
                                double interval_seconds,
                                int minimum_destination_stock = -1,
                                int max_active_convoys = -1);
    void get_supply_contract_ids(ft_vector<int> &out) const;
    bool get_supply_contract(int contract_id, ft_supply_contract &out) const;

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

    bool save_campaign_checkpoint(const ft_string &tag) noexcept;
    const ft_vector<ft_string> &get_failed_checkpoint_tags() const noexcept;
    void set_force_checkpoint_failure(bool enabled) noexcept;
    bool has_campaign_checkpoint() const noexcept;
    const ft_string &get_campaign_planet_checkpoint() const noexcept;
    const ft_string &get_campaign_fleet_checkpoint() const noexcept;
    const ft_string &get_campaign_checkpoint_tag() const noexcept;
    const ft_string &get_campaign_research_checkpoint() const noexcept;
    const ft_string &get_campaign_achievement_checkpoint() const noexcept;
    const ft_string &get_campaign_building_checkpoint() const noexcept;
    const ft_string &get_campaign_progress_checkpoint() const noexcept;
    bool reload_campaign_checkpoint() noexcept;
    bool load_campaign_from_save(const ft_string &planet_json, const ft_string &fleet_json,
        const ft_string &research_json, const ft_string &achievement_json,
        const ft_string &building_json, const ft_string &progress_json) noexcept;
};

#endif
