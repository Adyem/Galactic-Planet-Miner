namespace
{
    void gather_helios_neighbors(int planet_id, ft_vector<int> &neighbors)
    {
        neighbors.clear();
        switch (planet_id)
        {
        case PLANET_TERRA:
            neighbors.push_back(PLANET_MARS);
            neighbors.push_back(PLANET_LUNA);
            break;
        case PLANET_MARS:
            neighbors.push_back(PLANET_TERRA);
            neighbors.push_back(PLANET_ZALTHOR);
            break;
        case PLANET_ZALTHOR:
            neighbors.push_back(PLANET_MARS);
            neighbors.push_back(PLANET_VULCAN);
            break;
        case PLANET_VULCAN:
            neighbors.push_back(PLANET_ZALTHOR);
            neighbors.push_back(PLANET_NOCTARIS_PRIME);
            break;
        case PLANET_NOCTARIS_PRIME:
            neighbors.push_back(PLANET_VULCAN);
            break;
        case PLANET_LUNA:
            neighbors.push_back(PLANET_TERRA);
            break;
        default:
            break;
        }
    }
}

void BuildingManager::clone_from(const BuildingManager &other)
{
    if (this == &other)
        return ;
    this->_crafting_energy_multiplier = other._crafting_energy_multiplier;
    this->_crafting_speed_multiplier = other._crafting_speed_multiplier;
    this->_global_energy_multiplier = other._global_energy_multiplier;
    this->_building_unlocks.clear();
    ft_vector<Pair<int, bool> > unlock_entries;
    ft_map_snapshot(other._building_unlocks, unlock_entries);
    for (size_t i = 0; i < unlock_entries.size(); ++i)
        this->_building_unlocks.insert(unlock_entries[i].key, unlock_entries[i].value);
    this->_planets.clear();
    ft_vector<Pair<int, ft_planet_build_state> > entries;
    ft_map_snapshot(other._planets, entries);
    if (entries.size() == 0)
        return ;
    for (size_t i = 0; i < entries.size(); ++i)
    {
        const ft_planet_build_state &source = entries[i].value;
        this->_planets.insert(entries[i].key, ft_planet_build_state());
        Pair<int, ft_planet_build_state> *destination = this->_planets.find(entries[i].key);
        if (destination == ft_nullptr)
            continue;
        ft_planet_build_state &state = destination->value;
        state.planet_id = source.planet_id;
        state.width = source.width;
        state.height = source.height;
        state.base_logistic = source.base_logistic;
        state.research_logistic_bonus = source.research_logistic_bonus;
        state.used_plots = source.used_plots;
        state.logistic_capacity = source.logistic_capacity;
        state.logistic_usage = source.logistic_usage;
        state.base_energy_generation = source.base_energy_generation;
        state.energy_generation = source.energy_generation;
        state.energy_consumption = source.energy_consumption;
        state.support_energy = source.support_energy;
        state.mine_multiplier = source.mine_multiplier;
        state.convoy_speed_bonus = source.convoy_speed_bonus;
        state.convoy_raid_risk_modifier = source.convoy_raid_risk_modifier;
        state.energy_deficit_pressure = source.energy_deficit_pressure;
        state.emergency_conservation_active = source.emergency_conservation_active;
        state.next_instance_id = source.next_instance_id;
        state.grid.clear();
        size_t grid_size = source.grid.size();
        if (grid_size > 0)
        {
            state.grid.reserve(grid_size);
            for (size_t j = 0; j < grid_size; ++j)
                state.grid.push_back(source.grid[j]);
        }
        state.instances.clear();
        ft_vector<Pair<int, ft_building_instance> > inst_entries;
        ft_map_snapshot(source.instances, inst_entries);
        for (size_t j = 0; j < inst_entries.size(); ++j)
            state.instances.insert(inst_entries[j].key, inst_entries[j].value);
        recalculate_planet_statistics(state);
    }
    this->refresh_helios_network();
}

void BuildingManager::register_definition(const ft_sharedptr<ft_building_definition> &definition)
{
    this->_definitions.insert(definition->id, definition);
}

void BuildingManager::set_building_unlocked(int building_id, bool unlocked)
{
    Pair<int, bool> *entry = this->_building_unlocks.find(building_id);
    if (entry == ft_nullptr)
        this->_building_unlocks.insert(building_id, unlocked);
    else
        entry->value = unlocked;
}

bool BuildingManager::is_building_unlocked(int building_id) const
{
    const Pair<int, bool> *entry = this->_building_unlocks.find(building_id);
    if (entry == ft_nullptr)
        return true;
    return entry->value;
}

const ft_building_definition *BuildingManager::get_definition(int building_id) const
{
    const Pair<int, ft_sharedptr<ft_building_definition> > *entry = this->_definitions.find(building_id);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return entry->value.get();
}

ft_planet_build_state *BuildingManager::get_state(int planet_id)
{
    Pair<int, ft_planet_build_state> *entry = this->_planets.find(planet_id);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
}

const ft_planet_build_state *BuildingManager::get_state(int planet_id) const
{
    const Pair<int, ft_planet_build_state> *entry = this->_planets.find(planet_id);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
}

bool BuildingManager::is_area_free(const ft_planet_build_state &state, int x, int y, int width, int height) const
{
    if (x < 0 || y < 0 || width <= 0 || height <= 0)
        return false;
    if (x + width > state.width || y + height > state.height)
        return false;
    for (int dy = 0; dy < height; ++dy)
    {
        for (int dx = 0; dx < width; ++dx)
        {
            int index = (y + dy) * state.width + (x + dx);
            if (index < 0 || index >= static_cast<int>(state.grid.size()))
                return false;
            if (state.grid[index] != 0)
                return false;
        }
    }
    return true;
}

void BuildingManager::occupy_area(ft_planet_build_state &state, int instance_id, int x, int y, int width, int height)
{
    for (int dy = 0; dy < height; ++dy)
    {
        for (int dx = 0; dx < width; ++dx)
        {
            int index = (y + dy) * state.width + (x + dx);
            if (index >= 0 && index < static_cast<int>(state.grid.size()))
                state.grid[index] = instance_id;
        }
    }
    state.used_plots += width * height;
}

void BuildingManager::clear_area(ft_planet_build_state &state, int instance_id)
{
    int cleared = 0;
    for (size_t i = 0; i < state.grid.size(); ++i)
    {
        if (state.grid[i] == instance_id)
        {
            state.grid[i] = 0;
            ++cleared;
        }
    }
    if (cleared > 0 && state.used_plots >= cleared)
        state.used_plots -= cleared;
}

void BuildingManager::recalculate_planet_statistics(ft_planet_build_state &state)
{
    state.logistic_capacity = state.base_logistic + state.research_logistic_bonus;
    state.energy_generation = state.base_energy_generation;
    state.support_energy = 0.0;
    state.mine_multiplier = 1.0;
    state.convoy_speed_bonus = 0.0;
    state.convoy_raid_risk_modifier = 0.0;
    state.energy_deficit_pressure = 0.0;
    state.helios_projection = 0.0;
    state.helios_incoming = 0.0;
    ft_vector<Pair<int, ft_building_instance> > entries;
    ft_map_snapshot(state.instances, entries);
    for (size_t i = 0; i < entries.size(); ++i)
    {
        ft_building_instance &instance = entries[i].value;
        const ft_building_definition *definition = this->get_definition(instance.definition_id);
        if (definition == ft_nullptr)
            continue;
        state.logistic_capacity += definition->logistic_gain;
        if (definition->id == BUILDING_HELIOS_BEACON)
            state.helios_projection += definition->energy_gain;
        else
            state.energy_generation += definition->energy_gain;
        state.mine_multiplier += definition->mine_bonus;
        state.convoy_speed_bonus += definition->convoy_speed_bonus;
        state.convoy_raid_risk_modifier += definition->convoy_raid_risk_modifier;
        if (definition->energy_cost > 0.0 && (definition->cycle_time <= 0.0 || definition->outputs.size() == 0))
            state.support_energy += definition->energy_cost;
    }
    if (state.mine_multiplier < 1.0)
        state.mine_multiplier = 1.0;
    if (state.convoy_speed_bonus < 0.0)
        state.convoy_speed_bonus = 0.0;
    if (state.convoy_raid_risk_modifier < 0.0)
        state.convoy_raid_risk_modifier = 0.0;
    state.energy_consumption = state.support_energy;
    state.logistic_usage = 0;
}

void BuildingManager::refresh_helios_network()
{
    ft_vector<Pair<int, ft_planet_build_state> > entries;
    ft_map_snapshot(this->_planets, entries);
    for (size_t i = 0; i < entries.size(); ++i)
    {
        Pair<int, ft_planet_build_state> *entry = this->_planets.find(entries[i].key);
        if (entry == ft_nullptr)
            continue;
        if (entry->value.helios_incoming > 0.0)
        {
            entry->value.energy_generation -= entry->value.helios_incoming;
            if (entry->value.energy_generation < 0.0)
                entry->value.energy_generation = 0.0;
        }
        entry->value.helios_incoming = 0.0;
    }
    ft_vector<int> neighbors;
    neighbors.reserve(4);
    for (size_t i = 0; i < entries.size(); ++i)
    {
        Pair<int, ft_planet_build_state> *entry = this->_planets.find(entries[i].key);
        if (entry == ft_nullptr)
            continue;
        ft_planet_build_state &state = entry->value;
        if (state.helios_projection <= 0.0)
            continue;
        gather_helios_neighbors(state.planet_id, neighbors);
        if (neighbors.size() == 0)
            continue;
        double stability = 1.0;
        if (state.support_energy > 0.0 && state.energy_generation < state.support_energy)
        {
            double deficit = state.support_energy - state.energy_generation;
            double ratio = deficit / state.support_energy;
            if (ratio >= 1.0)
                stability = 0.15;
            else
            {
                stability = 1.0 - ratio * 0.7;
                if (stability < 0.15)
                    stability = 0.15;
            }
        }
        if (stability <= 0.0)
            continue;
        double per_neighbor = state.helios_projection * 0.9 * stability;
        if (per_neighbor <= 0.0)
            continue;
        for (size_t j = 0; j < neighbors.size(); ++j)
        {
            Pair<int, ft_planet_build_state> *target = this->_planets.find(neighbors[j]);
            if (target == ft_nullptr)
                continue;
            target->value.helios_incoming += per_neighbor;
        }
    }
    for (size_t i = 0; i < entries.size(); ++i)
    {
        Pair<int, ft_planet_build_state> *entry = this->_planets.find(entries[i].key);
        if (entry == ft_nullptr)
            continue;
        if (entry->value.helios_incoming > 0.0)
            entry->value.energy_generation += entry->value.helios_incoming;
    }
}

