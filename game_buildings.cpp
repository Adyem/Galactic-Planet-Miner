#include "game.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/Template/move.hpp"
#include "ft_map_snapshot.hpp"

namespace
{
    static ft_string resolve_planet_label(int planet_id)
    {
        switch (planet_id)
        {
        case PLANET_TERRA:
            return ft_string("Terra");
        case PLANET_MARS:
            return ft_string("Mars");
        case PLANET_ZALTHOR:
            return ft_string("Zalthor");
        case PLANET_VULCAN:
            return ft_string("Vulcan");
        case PLANET_NOCTARIS_PRIME:
            return ft_string("Noctaris Prime");
        case PLANET_LUNA:
            return ft_string("Luna");
        default:
            return ft_string();
        }
    }

    static void copy_pair_vector(const ft_vector<Pair<int, int> > &source,
                                 ft_vector<Pair<int, int> > &dest)
    {
        dest.clear();
        size_t count = source.size();
        if (count == 0)
            return ;
        dest.reserve(count);
        for (size_t i = 0; i < count; ++i)
            dest.push_back(source[i]);
    }
}

void Game::get_building_layout_snapshot(ft_building_layout_snapshot &out) const
{
    out.planets.clear();

    ft_vector<Pair<int, ft_planet_build_state> > planet_entries;
    ft_map_snapshot(this->_buildings._planets, planet_entries);
    size_t planet_count = planet_entries.size();
    if (planet_count == 0)
        return ;

    out.planets.reserve(planet_count);
    for (size_t i = 0; i < planet_count; ++i)
    {
        const Pair<int, ft_planet_build_state> &entry = planet_entries[i];
        const ft_planet_build_state &state = entry.value;

        ft_planet_building_snapshot planet_snapshot;
        planet_snapshot.planet_id = entry.key;
        planet_snapshot.planet_name = resolve_planet_label(entry.key);
        planet_snapshot.width = state.width;
        planet_snapshot.height = state.height;
        planet_snapshot.base_logistic = state.base_logistic;
        planet_snapshot.research_logistic_bonus = state.research_logistic_bonus;
        planet_snapshot.used_plots = state.used_plots;
        planet_snapshot.logistic_capacity = state.logistic_capacity;
        planet_snapshot.logistic_usage = state.logistic_usage;
        planet_snapshot.logistic_available = state.logistic_capacity - state.logistic_usage;
        planet_snapshot.base_energy_generation = state.base_energy_generation;
        planet_snapshot.energy_generation = state.energy_generation;
        planet_snapshot.energy_consumption = state.energy_consumption;
        planet_snapshot.support_energy = state.support_energy;
        planet_snapshot.energy_deficit_pressure = state.energy_deficit_pressure;
        planet_snapshot.mine_multiplier = state.mine_multiplier;
        planet_snapshot.convoy_speed_bonus = state.convoy_speed_bonus;
        planet_snapshot.convoy_raid_risk_modifier = state.convoy_raid_risk_modifier;
        planet_snapshot.emergency_energy_conservation = state.emergency_conservation_active;

        size_t grid_size = state.grid.size();
        if (grid_size > 0 && state.width > 0)
        {
            planet_snapshot.grid.clear();
            planet_snapshot.grid.reserve(grid_size);
            for (size_t cell_index = 0; cell_index < grid_size; ++cell_index)
            {
                ft_building_grid_cell_snapshot cell;
                cell.instance_id = state.grid[cell_index];
                cell.x = static_cast<int>(cell_index % static_cast<size_t>(state.width));
                cell.y = static_cast<int>(cell_index / static_cast<size_t>(state.width));
                cell.building_id = 0;
                if (cell.instance_id != 0)
                {
                    const Pair<int, ft_building_instance> *instance_entry = state.instances.find(cell.instance_id);
                    if (instance_entry != ft_nullptr)
                        cell.building_id = instance_entry->value.definition_id;
                }
                planet_snapshot.grid.push_back(cell);
            }
        }

        ft_vector<Pair<int, ft_building_instance> > instance_entries;
        ft_map_snapshot(state.instances, instance_entries);
        size_t instance_count = instance_entries.size();
        if (instance_count > 0)
        {
            planet_snapshot.instances.clear();
            planet_snapshot.instances.reserve(instance_count);
            for (size_t j = 0; j < instance_count; ++j)
            {
                const ft_building_instance &instance = instance_entries[j].value;
                ft_building_instance_snapshot summary;
                summary.instance_id = instance.uid;
                summary.building_id = instance.definition_id;
                summary.name.clear();
                summary.x = instance.x;
                summary.y = instance.y;
                summary.progress = instance.progress;
                summary.active = instance.active;

                const Pair<int, ft_sharedptr<ft_building_definition> > *definition_entry =
                    this->_buildings._definitions.find(instance.definition_id);
                const ft_building_definition *definition = ft_nullptr;
                if (definition_entry != ft_nullptr && definition_entry->value)
                    definition = definition_entry->value.get();
                if (definition != ft_nullptr)
                {
                    summary.name = definition->name;
                    summary.width = definition->width;
                    summary.height = definition->height;
                    summary.logistic_cost = definition->logistic_cost;
                    summary.logistic_gain = definition->logistic_gain;
                    summary.energy_cost = definition->energy_cost;
                    summary.energy_gain = definition->energy_gain;
                    summary.cycle_time = definition->cycle_time;
                    summary.mine_bonus = definition->mine_bonus;
                    summary.convoy_speed_bonus = definition->convoy_speed_bonus;
                    summary.convoy_raid_risk_modifier = definition->convoy_raid_risk_modifier;
                    summary.unique = definition->unique;
                    summary.occupies_grid = definition->occupies_grid;
                    summary.removable = definition->removable;
                    copy_pair_vector(definition->inputs, summary.inputs);
                    copy_pair_vector(definition->outputs, summary.outputs);
                    copy_pair_vector(definition->build_costs, summary.build_costs);
                }
                else
                {
                    summary.width = 0;
                    summary.height = 0;
                    summary.logistic_cost = 0;
                    summary.logistic_gain = 0;
                    summary.energy_cost = 0.0;
                    summary.energy_gain = 0.0;
                    summary.cycle_time = 0.0;
                    summary.mine_bonus = 0.0;
                    summary.convoy_speed_bonus = 0.0;
                    summary.convoy_raid_risk_modifier = 0.0;
                    summary.unique = false;
                    summary.occupies_grid = false;
                    summary.removable = false;
                    summary.inputs.clear();
                    summary.outputs.clear();
                    summary.build_costs.clear();
                }
                planet_snapshot.instances.push_back(ft_move(summary));
            }
        }

        out.planets.push_back(ft_move(planet_snapshot));
    }
}
