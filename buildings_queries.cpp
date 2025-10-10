int BuildingManager::get_building_instance(int planet_id, int x, int y) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    if (x < 0 || y < 0 || x >= state->width || y >= state->height)
        return 0;
    int index = y * state->width + x;
    if (index < 0 || index >= static_cast<int>(state->grid.size()))
        return 0;
    return state->grid[index];
}

int BuildingManager::get_building_count(int planet_id, int building_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    int count = 0;
    ft_vector<Pair<int, ft_building_instance> > entries;
    ft_map_snapshot(state->instances, entries);
    for (size_t i = 0; i < entries.size(); ++i)
    {
        if (entries[i].value.definition_id == building_id)
            ++count;
    }
    return count;
}

int BuildingManager::get_planet_plot_capacity(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    return state->width * state->height;
}

int BuildingManager::get_planet_plot_usage(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    return state->used_plots;
}

int BuildingManager::get_planet_logistic_capacity(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    return state->logistic_capacity;
}

int BuildingManager::get_planet_logistic_usage(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    return state->logistic_usage;
}

double BuildingManager::get_planet_energy_generation(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0.0;
    return state->energy_generation;
}

double BuildingManager::get_planet_energy_consumption(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0.0;
    return state->energy_consumption;
}

double BuildingManager::get_planet_support_energy(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0.0;
    if (state->support_energy < 0.0)
        return 0.0;
    return state->support_energy;
}

double BuildingManager::get_mine_multiplier(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 1.0;
    return state->mine_multiplier;
}

double BuildingManager::get_planet_energy_pressure(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0.0;
    if (state->energy_deficit_pressure < 0.0)
        return 0.0;
    return state->energy_deficit_pressure;
}

double BuildingManager::get_planet_convoy_speed_bonus(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0.0;
    if (state->convoy_speed_bonus < 0.0)
        return 0.0;
    return state->convoy_speed_bonus;
}

double BuildingManager::get_planet_convoy_raid_risk_modifier(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0.0;
    if (state->convoy_raid_risk_modifier < 0.0)
        return 0.0;
    return state->convoy_raid_risk_modifier;
}

