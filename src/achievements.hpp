#ifndef ACHIEVEMENTS_HPP
#define ACHIEVEMENTS_HPP

#include "../libft/Libft/libft.hpp"
#include "../libft/Template/map.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/shared_ptr.hpp"
#include "../libft/Template/pair.hpp"
#include "../libft/CPP_class/class_nullptr.hpp"

enum e_achievement_id
{
    ACHIEVEMENT_SECOND_HOME = 1,
    ACHIEVEMENT_RESEARCH_PIONEER,
    ACHIEVEMENT_LOGISTICS_PILOT,
    ACHIEVEMENT_CONVOY_STREAK_GUARDIAN,
    ACHIEVEMENT_QUEST_INITIAL_SKIRMISHES,
    ACHIEVEMENT_QUEST_DEFENSE_OF_TERRA,
    ACHIEVEMENT_QUEST_INVESTIGATE_RAIDERS,
    ACHIEVEMENT_QUEST_SECURE_SUPPLY_LINES,
    ACHIEVEMENT_QUEST_STEADY_SUPPLY_STREAK,
    ACHIEVEMENT_QUEST_HIGH_VALUE_ESCORT,
    ACHIEVEMENT_QUEST_CLIMACTIC_BATTLE,
    ACHIEVEMENT_QUEST_CRITICAL_DECISION,
    ACHIEVEMENT_QUEST_ORDER_UPRISING,
    ACHIEVEMENT_QUEST_REBELLION_FLEET
};

enum e_achievement_status
{
    ACHIEVEMENT_STATUS_LOCKED = 0,
    ACHIEVEMENT_STATUS_IN_PROGRESS,
    ACHIEVEMENT_STATUS_COMPLETED
};

enum e_achievement_event
{
    ACHIEVEMENT_EVENT_PLANET_UNLOCKED = 1,
    ACHIEVEMENT_EVENT_RESEARCH_COMPLETED,
    ACHIEVEMENT_EVENT_CONVOY_DELIVERED,
    ACHIEVEMENT_EVENT_CONVOY_STREAK_BEST,
    ACHIEVEMENT_EVENT_QUEST_INITIAL_SKIRMISHES,
    ACHIEVEMENT_EVENT_QUEST_DEFENSE_OF_TERRA,
    ACHIEVEMENT_EVENT_QUEST_INVESTIGATE_RAIDERS,
    ACHIEVEMENT_EVENT_QUEST_SECURE_SUPPLY_LINES,
    ACHIEVEMENT_EVENT_QUEST_STEADY_SUPPLY_STREAK,
    ACHIEVEMENT_EVENT_QUEST_HIGH_VALUE_ESCORT,
    ACHIEVEMENT_EVENT_QUEST_CLIMACTIC_BATTLE,
    ACHIEVEMENT_EVENT_QUEST_CRITICAL_DECISION,
    ACHIEVEMENT_EVENT_QUEST_ORDER_UPRISING,
    ACHIEVEMENT_EVENT_QUEST_REBELLION_FLEET
};

enum e_achievement_progress_mode
{
    ACHIEVEMENT_PROGRESS_INCREMENTAL = 1,
    ACHIEVEMENT_PROGRESS_BEST_VALUE
};

struct ft_achievement_definition
{
    int         id;
    ft_string   name;
    ft_string   description;
    int         event_id;
    int         target_value;
    int         progress_mode;
};

struct ft_achievement_progress
{
    int     value;
    bool    completed;
    ft_achievement_progress() : value(0), completed(false) {}
};

struct ft_achievement_info
{
    int         id;
    ft_string   name;
    ft_string   description;
    int         status;
    int         progress;
    int         target;
};

class AchievementManager
{
private:
    ft_map<int, ft_sharedptr<ft_achievement_definition> > _definitions;
    ft_map<int, ft_achievement_progress>                  _progress;
    ft_map<int, ft_vector<int> >                          _event_index;

    void register_achievement(const ft_sharedptr<ft_achievement_definition> &definition);
    ft_achievement_progress &ensure_progress(int achievement_id);
    const ft_achievement_progress *find_progress(int achievement_id) const;

public:
    AchievementManager();

    void record_event(int event_id, int value, ft_vector<int> *completed);
    void record_event(int event_id, int value);

    bool is_completed(int achievement_id) const;
    int get_status(int achievement_id) const;
    int get_progress(int achievement_id) const;
    int get_target(int achievement_id) const;
    bool get_info(int achievement_id, ft_achievement_info &out) const;
    void get_achievement_ids(ft_vector<int> &out) const;
};

#endif
