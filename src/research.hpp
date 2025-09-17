#ifndef RESEARCH_HPP
#define RESEARCH_HPP

#include "planets.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Template/map.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/pair.hpp"
#include "../libft/Template/shared_ptr.hpp"
#include "../libft/CPP_class/class_nullptr.hpp"

enum e_research_id
{
    RESEARCH_UNLOCK_MARS = 1,
    RESEARCH_UNLOCK_ZALTHOR,
    RESEARCH_UNLOCK_VULCAN,
    RESEARCH_UNLOCK_NOCTARIS,
    RESEARCH_URBAN_PLANNING_TERRA,
    RESEARCH_URBAN_PLANNING_MARS,
    RESEARCH_URBAN_PLANNING_ZALTHOR,
    RESEARCH_SOLAR_PANELS,
    RESEARCH_CRAFTING_MASTERY,
    RESEARCH_STRUCTURAL_REINFORCEMENT_I,
    RESEARCH_STRUCTURAL_REINFORCEMENT_II,
    RESEARCH_STRUCTURAL_REINFORCEMENT_III,
    RESEARCH_DEFENSIVE_FORTIFICATION_I,
    RESEARCH_DEFENSIVE_FORTIFICATION_II,
    RESEARCH_DEFENSIVE_FORTIFICATION_III,
    RESEARCH_ARMAMENT_ENHANCEMENT_I,
    RESEARCH_ARMAMENT_ENHANCEMENT_II,
    RESEARCH_ARMAMENT_ENHANCEMENT_III
};

enum e_research_status
{
    RESEARCH_STATUS_LOCKED = 0,
    RESEARCH_STATUS_AVAILABLE,
    RESEARCH_STATUS_IN_PROGRESS,
    RESEARCH_STATUS_COMPLETED
};

struct ft_research_definition
{
    int                         id;
    ft_string                   name;
    double                      duration;
    ft_vector<int>              prerequisites;
    ft_vector<Pair<int, int> >  costs;
    ft_vector<int>              unlock_planets;
};

struct ft_research_progress
{
    int     status;
    double  remaining_time;
    ft_research_progress() : status(RESEARCH_STATUS_LOCKED), remaining_time(0.0) {}
};

class ResearchManager
{
private:
    ft_map<int, ft_sharedptr<ft_research_definition> > _definitions;
    ft_map<int, ft_research_progress>   _progress;
    double                              _duration_scale;

    void register_research(const ft_sharedptr<ft_research_definition> &definition);
    void update_availability();

public:
    ResearchManager();

    void tick(double seconds, ft_vector<int> &completed);

    void set_duration_scale(double scale);
    double get_duration_scale() const { return this->_duration_scale; }

    bool can_start(int research_id) const;
    bool start(int research_id);
    bool is_completed(int research_id) const;
    int get_status(int research_id) const;
    double get_remaining_time(int research_id) const;
    const ft_research_definition *get_definition(int research_id) const;
    void mark_completed(int research_id);
};

#endif
