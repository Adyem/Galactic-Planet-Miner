void BuildingManager::tick_planet(Game &game, ft_planet_build_state &state, double seconds)
{
    bool conserve_energy = game.update_planet_energy_conservation(state.planet_id);
    state.emergency_conservation_active = conserve_energy;
    state.energy_deficit_pressure = 0.0;
    state.energy_consumption = state.support_energy;
    if (state.energy_consumption > state.energy_generation)
    {
        state.energy_deficit_pressure += state.energy_consumption - state.energy_generation;
        state.energy_consumption = state.energy_generation;
    }
    state.logistic_usage = 0;
    ft_vector<Pair<int, ft_building_instance> > instance_entries;
    ft_map_snapshot(state.instances, instance_entries);
    for (size_t i = 0; i < instance_entries.size(); ++i)
    {
        Pair<int, ft_building_instance> *entry = state.instances.find(instance_entries[i].key);
        if (entry != ft_nullptr)
            entry->value.active = false;
    }
    for (size_t i = 0; i < instance_entries.size(); ++i)
    {
        Pair<int, ft_building_instance> *entry = state.instances.find(instance_entries[i].key);
        if (entry == ft_nullptr)
            continue;
        ft_building_instance &instance = entry->value;
        const ft_building_definition *definition = this->get_definition(instance.definition_id);
        if (definition == ft_nullptr)
            continue;
        if (definition->cycle_time <= 0.0 || definition->outputs.size() == 0)
            continue;
        bool can_run = true;
        if (conserve_energy && definition->energy_cost > 0.0 && definition->outputs.size() > 0)
        {
            if (instance.progress > definition->cycle_time)
                instance.progress = definition->cycle_time;
            continue;
        }
        if (definition->logistic_cost > 0)
        {
            if (state.logistic_usage + definition->logistic_cost > state.logistic_capacity)
                can_run = false;
        }
        double energy_cost = definition->energy_cost * this->_global_energy_multiplier;
        if (definition->cycle_time > 0.0 && definition->outputs.size() > 0)
            energy_cost *= this->_crafting_energy_multiplier;
        if (energy_cost > 0.0)
        {
            double projected = state.energy_consumption + energy_cost;
            if (projected > state.energy_generation + 0.0001)
            {
                can_run = false;
                double available = state.energy_generation - state.energy_consumption;
                if (available < 0.0)
                    available = 0.0;
                double shortage = energy_cost - available;
                if (shortage < 0.0)
                    shortage = 0.0;
                state.energy_deficit_pressure += shortage;
            }
        }
        if (!can_run)
        {
            if (instance.progress > definition->cycle_time)
                instance.progress = definition->cycle_time;
            continue;
        }
        instance.active = true;
        state.logistic_usage += definition->logistic_cost;
        state.energy_consumption += energy_cost;
    }
    for (size_t i = 0; i < instance_entries.size(); ++i)
    {
        Pair<int, ft_building_instance> *entry = state.instances.find(instance_entries[i].key);
        if (entry == ft_nullptr)
            continue;
        ft_building_instance &instance = entry->value;
        const ft_building_definition *definition = this->get_definition(instance.definition_id);
        if (definition == ft_nullptr)
            continue;
        if (definition->cycle_time <= 0.0 || definition->outputs.size() == 0)
            continue;
        if (!instance.active)
            continue;
        double elapsed = seconds;
        if (definition->cycle_time > 0.0 && definition->outputs.size() > 0)
            elapsed *= this->_crafting_speed_multiplier;
        instance.progress += elapsed;
        while (instance.progress >= definition->cycle_time)
        {
            if (!this->consume_inputs(game, state.planet_id, definition->inputs))
            {
                instance.progress = definition->cycle_time;
                break;
            }
            instance.progress -= definition->cycle_time;
            this->produce_outputs(game, state.planet_id, definition->outputs);
        }
    }
}

void BuildingManager::tick(Game &game, double seconds)
{
    ft_vector<Pair<int, ft_planet_build_state> > entries;
    ft_map_snapshot(this->_planets, entries);
    for (size_t i = 0; i < entries.size(); ++i)
    {
        Pair<int, ft_planet_build_state> *entry = this->_planets.find(entries[i].key);
        if (entry == ft_nullptr)
            continue;
        this->tick_planet(game, entry->value, seconds);
    }
}
