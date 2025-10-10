#include "buildings.hpp"
#include "game.hpp"
#include "research.hpp"
#include "libft/Libft/libft.hpp"
#include "ft_map_snapshot.hpp"

ft_planet_build_state::ft_planet_build_state()
    : planet_id(0), width(0), height(0), base_logistic(0), research_logistic_bonus(0), used_plots(0),
      logistic_capacity(0), logistic_usage(0), base_energy_generation(0.0),
      energy_generation(0.0), energy_consumption(0.0), support_energy(0.0),
      mine_multiplier(1.0), convoy_speed_bonus(0.0), convoy_raid_risk_modifier(0.0),
      energy_deficit_pressure(0.0), emergency_conservation_active(false),
      helios_projection(0.0), helios_incoming(0.0),
      next_instance_id(1), grid(), instances()
{
}

ft_planet_build_state::ft_planet_build_state(const ft_planet_build_state &other)
    : planet_id(0), width(0), height(0), base_logistic(0), research_logistic_bonus(0), used_plots(0),
      logistic_capacity(0), logistic_usage(0), base_energy_generation(0.0),
      energy_generation(0.0), energy_consumption(0.0), support_energy(0.0),
      mine_multiplier(1.0), convoy_speed_bonus(0.0), convoy_raid_risk_modifier(0.0),
      energy_deficit_pressure(0.0), emergency_conservation_active(false),
      helios_projection(0.0), helios_incoming(0.0),
      next_instance_id(1), grid(), instances()
{
    *this = other;
}

ft_planet_build_state &ft_planet_build_state::operator=(const ft_planet_build_state &other)
{
    if (this == &other)
        return *this;
    this->planet_id = other.planet_id;
    this->width = other.width;
    this->height = other.height;
    this->base_logistic = other.base_logistic;
    this->research_logistic_bonus = other.research_logistic_bonus;
    this->used_plots = other.used_plots;
    this->logistic_capacity = other.logistic_capacity;
    this->logistic_usage = other.logistic_usage;
    this->base_energy_generation = other.base_energy_generation;
    this->energy_generation = other.energy_generation;
    this->energy_consumption = other.energy_consumption;
    this->support_energy = other.support_energy;
    this->mine_multiplier = other.mine_multiplier;
    this->convoy_speed_bonus = other.convoy_speed_bonus;
    this->convoy_raid_risk_modifier = other.convoy_raid_risk_modifier;
    this->energy_deficit_pressure = other.energy_deficit_pressure;
    this->emergency_conservation_active = other.emergency_conservation_active;
    this->helios_projection = other.helios_projection;
    this->helios_incoming = other.helios_incoming;
    this->next_instance_id = other.next_instance_id;

    this->grid.clear();
    size_t grid_size = other.grid.size();
    if (grid_size > 0)
    {
        this->grid.reserve(grid_size);
        for (size_t index = 0; index < grid_size; ++index)
            this->grid.push_back(other.grid[index]);
    }

    this->instances.clear();
    ft_vector<Pair<int, ft_building_instance> > instance_entries;
    ft_map_snapshot(other.instances, instance_entries);
    for (size_t index = 0; index < instance_entries.size(); ++index)
        this->instances.insert(instance_entries[index].key, instance_entries[index].value);

    return *this;
}

ft_planet_build_state::ft_planet_build_state(ft_planet_build_state &&other) noexcept
    : planet_id(other.planet_id), width(other.width), height(other.height),
      base_logistic(other.base_logistic), research_logistic_bonus(other.research_logistic_bonus),
      used_plots(other.used_plots), logistic_capacity(other.logistic_capacity),
      logistic_usage(other.logistic_usage), base_energy_generation(other.base_energy_generation),
      energy_generation(other.energy_generation), energy_consumption(other.energy_consumption),
      support_energy(other.support_energy), mine_multiplier(other.mine_multiplier),
      convoy_speed_bonus(other.convoy_speed_bonus),
      convoy_raid_risk_modifier(other.convoy_raid_risk_modifier),
      energy_deficit_pressure(other.energy_deficit_pressure),
      emergency_conservation_active(other.emergency_conservation_active),
      helios_projection(other.helios_projection), helios_incoming(other.helios_incoming),
      next_instance_id(other.next_instance_id),
      grid(ft_move(other.grid)), instances(ft_move(other.instances))
{
    other.planet_id = 0;
    other.width = 0;
    other.height = 0;
    other.base_logistic = 0;
    other.research_logistic_bonus = 0;
    other.used_plots = 0;
    other.logistic_capacity = 0;
    other.logistic_usage = 0;
    other.base_energy_generation = 0.0;
    other.energy_generation = 0.0;
    other.energy_consumption = 0.0;
    other.support_energy = 0.0;
    other.mine_multiplier = 1.0;
    other.convoy_speed_bonus = 0.0;
    other.convoy_raid_risk_modifier = 0.0;
    other.energy_deficit_pressure = 0.0;
    other.emergency_conservation_active = false;
    other.helios_projection = 0.0;
    other.helios_incoming = 0.0;
    other.next_instance_id = 1;
}

ft_planet_build_state &ft_planet_build_state::operator=(ft_planet_build_state &&other) noexcept
{
    if (this == &other)
        return *this;

    this->planet_id = other.planet_id;
    this->width = other.width;
    this->height = other.height;
    this->base_logistic = other.base_logistic;
    this->research_logistic_bonus = other.research_logistic_bonus;
    this->used_plots = other.used_plots;
    this->logistic_capacity = other.logistic_capacity;
    this->logistic_usage = other.logistic_usage;
    this->base_energy_generation = other.base_energy_generation;
    this->energy_generation = other.energy_generation;
    this->energy_consumption = other.energy_consumption;
    this->support_energy = other.support_energy;
    this->mine_multiplier = other.mine_multiplier;
    this->convoy_speed_bonus = other.convoy_speed_bonus;
    this->convoy_raid_risk_modifier = other.convoy_raid_risk_modifier;
    this->energy_deficit_pressure = other.energy_deficit_pressure;
    this->emergency_conservation_active = other.emergency_conservation_active;
    this->helios_projection = other.helios_projection;
    this->helios_incoming = other.helios_incoming;
    this->next_instance_id = other.next_instance_id;

    this->grid = ft_move(other.grid);
    this->instances = ft_move(other.instances);

    other.planet_id = 0;
    other.width = 0;
    other.height = 0;
    other.base_logistic = 0;
    other.research_logistic_bonus = 0;
    other.used_plots = 0;
    other.logistic_capacity = 0;
    other.logistic_usage = 0;
    other.base_energy_generation = 0.0;
    other.energy_generation = 0.0;
    other.energy_consumption = 0.0;
    other.support_energy = 0.0;
    other.mine_multiplier = 1.0;
    other.convoy_speed_bonus = 0.0;
    other.convoy_raid_risk_modifier = 0.0;
    other.energy_deficit_pressure = 0.0;
    other.emergency_conservation_active = false;
    other.next_instance_id = 1;

    return *this;
}
