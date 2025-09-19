#ifndef SAVE_SYSTEM_HPP
#define SAVE_SYSTEM_HPP

#include "planets.hpp"
#include "fleets.hpp"
#include "research.hpp"
#include "achievements.hpp"
#include "../libft/JSon/document.hpp"
#include "../libft/JSon/json.hpp"
#include "../libft/CPP_class/class_string_class.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Template/pair.hpp"

class SaveSystem
{
public:
    SaveSystem() noexcept;
    ~SaveSystem() noexcept;

    ft_string serialize_planets(const ft_map<int, ft_sharedptr<ft_planet> > &planets) const noexcept;
    bool deserialize_planets(const char *content,
        ft_map<int, ft_sharedptr<ft_planet> > &planets) const noexcept;

    ft_string serialize_fleets(const ft_map<int, ft_sharedptr<ft_fleet> > &fleets) const noexcept;
    bool deserialize_fleets(const char *content,
        ft_map<int, ft_sharedptr<ft_fleet> > &fleets) const noexcept;

    ft_string serialize_research(const ResearchManager &research) const noexcept;
    bool deserialize_research(const char *content, ResearchManager &research) const noexcept;

    ft_string serialize_achievements(const AchievementManager &achievements) const noexcept;
    bool deserialize_achievements(const char *content, AchievementManager &achievements) const noexcept;

private:
    ft_sharedptr<ft_planet> create_planet_instance(int planet_id) const noexcept;
    ft_sharedptr<ft_fleet> create_fleet_instance(int fleet_id) const noexcept;
    long scale_double_to_long(double value) const noexcept;
    double unscale_long_to_double(long value) const noexcept;
};

#endif
