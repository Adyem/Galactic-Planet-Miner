#ifndef QUESTS_HPP
#define QUESTS_HPP

#include "planets.hpp"
#include "research.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Template/map.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/pair.hpp"
#include "../libft/Template/shared_ptr.hpp"
#include "../libft/CPP_class/class_nullptr.hpp"

enum e_quest_id
{
    QUEST_INITIAL_SKIRMISHES = 1,
    QUEST_DEFENSE_OF_TERRA,
    QUEST_INVESTIGATE_RAIDERS,
    QUEST_CLIMACTIC_BATTLE,
    QUEST_CRITICAL_DECISION,
    QUEST_ORDER_UPRISING,
    QUEST_REBELLION_FLEET
};

enum e_quest_status
{
    QUEST_STATUS_LOCKED = 0,
    QUEST_STATUS_AVAILABLE,
    QUEST_STATUS_ACTIVE,
    QUEST_STATUS_AWAITING_CHOICE,
    QUEST_STATUS_COMPLETED,
    QUEST_STATUS_FAILED
};

enum e_quest_objective_type
{
    QUEST_OBJECTIVE_RESOURCE_TOTAL = 1,
    QUEST_OBJECTIVE_RESEARCH_COMPLETED,
    QUEST_OBJECTIVE_FLEET_COUNT,
    QUEST_OBJECTIVE_TOTAL_SHIP_HP
};

enum e_quest_choice_value
{
    QUEST_CHOICE_NONE = 0,
    QUEST_CHOICE_EXECUTE_BLACKTHORNE = 1,
    QUEST_CHOICE_SPARE_BLACKTHORNE = 2
};

struct ft_quest_objective
{
    int type;
    int target_id;
    int amount;
    ft_quest_objective() : type(0), target_id(0), amount(0) {}
};

struct ft_quest_choice_definition
{
    int choice_id;
    ft_string description;
    ft_quest_choice_definition() : choice_id(0), description() {}
};

struct ft_quest_definition
{
    int                                 id;
    ft_string                           name;
    ft_string                           description;
    double                              time_limit;
    ft_vector<ft_quest_objective>       objectives;
    ft_vector<int>                      prerequisites;
    bool                                requires_choice;
    ft_vector<ft_quest_choice_definition> choices;
    int                                 required_choice_quest;
    int                                 required_choice_value;
    ft_quest_definition()
        : id(0), name(), description(), time_limit(0.0), objectives(), prerequisites(),
          requires_choice(false), choices(), required_choice_quest(0), required_choice_value(0)
    {}
};

struct ft_quest_progress
{
    int     status;
    double  time_remaining;
    ft_quest_progress() : status(QUEST_STATUS_LOCKED), time_remaining(0.0) {}
};

struct ft_quest_context
{
    ft_map<int, int> resource_totals;
    ft_map<int, int> research_status;
    int total_ship_count;
    int total_ship_hp;
    ft_quest_context() : resource_totals(), research_status(), total_ship_count(0), total_ship_hp(0) {}
};

class QuestManager
{
private:
    ft_map<int, ft_sharedptr<ft_quest_definition> > _definitions;
    ft_map<int, ft_quest_progress>                  _progress;
    ft_map<int, int>                                _quest_choices;

    void register_quest(const ft_sharedptr<ft_quest_definition> &definition);
    void update_availability();
    void activate_next();
    bool are_objectives_met(const ft_quest_definition &definition, const ft_quest_context &context) const;

public:
    QuestManager();

    void update(double seconds, const ft_quest_context &context,
                ft_vector<int> &completed, ft_vector<int> &failed,
                ft_vector<int> &awaiting_choice);

    int get_active_quest_id() const;
    int get_status(int quest_id) const;
    double get_time_remaining(int quest_id) const;
    const ft_quest_definition *get_definition(int quest_id) const;
    bool make_choice(int quest_id, int choice_id);
    int get_choice(int quest_id) const;
};

#endif
