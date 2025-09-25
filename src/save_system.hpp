#ifndef SAVE_SYSTEM_HPP
#define SAVE_SYSTEM_HPP

#include "planets.hpp"
#include "fleets.hpp"
#include "research.hpp"
#include "achievements.hpp"
#include "buildings.hpp"
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

    typedef bool (*json_allocation_hook_t)(const char *type, const char *identifier);

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

    ft_string serialize_buildings(const BuildingManager &buildings) const noexcept;
    bool deserialize_buildings(const char *content, BuildingManager &buildings) const noexcept;

    ft_string serialize_campaign_progress(int convoys_delivered_total,
        int convoy_raid_losses, int current_delivery_streak,
        int longest_delivery_streak, size_t next_streak_milestone_index,
        int order_branch_assault_victories,
        int rebellion_branch_assault_victories,
        int order_branch_pending_assault,
        int rebellion_branch_pending_assault) const noexcept;
    bool deserialize_campaign_progress(const char *content,
        int &convoys_delivered_total, int &convoy_raid_losses,
        int &current_delivery_streak, int &longest_delivery_streak,
        size_t &next_streak_milestone_index,
        int &order_branch_assault_victories,
        int &rebellion_branch_assault_victories,
        int &order_branch_pending_assault,
        int &rebellion_branch_pending_assault) const noexcept;

    static void set_json_allocation_hook(json_allocation_hook_t hook) noexcept;

private:
    ft_sharedptr<ft_planet> create_planet_instance(int planet_id) const noexcept;
    ft_sharedptr<ft_fleet> create_fleet_instance(int fleet_id) const noexcept;
    long scale_double_to_long(double value) const noexcept;
    double unscale_long_to_double(long value) const noexcept;
    ft_string encode_building_grid(const ft_vector<int> &grid) const noexcept;
    bool decode_building_grid(const char *encoded, size_t expected_cells,
        ft_vector<int> &grid) const noexcept;
    ft_string encode_building_instances(const ft_map<int, ft_building_instance> &instances)
        const noexcept;
    bool decode_building_instances(const char *encoded,
        ft_map<int, ft_building_instance> &instances) const noexcept;
};

#endif
