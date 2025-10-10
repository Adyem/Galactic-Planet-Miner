bool BuildingManager::check_build_costs(const Game &game, int planet_id, const ft_vector<Pair<int, int> > &costs) const
{
    for (size_t i = 0; i < costs.size(); ++i)
    {
        int resource_id = costs[i].key;
        int required = costs[i].value;
        if (required <= 0)
            continue;
        int available = game.get_ore(planet_id, resource_id);
        if (available < required)
            return false;
    }
    return true;
}

void BuildingManager::pay_build_costs(Game &game, int planet_id, const ft_vector<Pair<int, int> > &costs)
{
    for (size_t i = 0; i < costs.size(); ++i)
    {
        int resource_id = costs[i].key;
        int amount = costs[i].value;
        if (amount <= 0)
            continue;
        game.sub_ore(planet_id, resource_id, amount);
    }
}

bool BuildingManager::consume_inputs(Game &game, int planet_id, const ft_vector<Pair<int, int> > &inputs)
{
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        int resource_id = inputs[i].key;
        int amount = inputs[i].value;
        if (amount <= 0)
            continue;
        if (game.get_ore(planet_id, resource_id) < amount)
            return false;
    }
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        int resource_id = inputs[i].key;
        int amount = inputs[i].value;
        if (amount > 0)
            game.sub_ore(planet_id, resource_id, amount);
    }
    return true;
}

void BuildingManager::produce_outputs(Game &game, int planet_id, const ft_vector<Pair<int, int> > &outputs)
{
    for (size_t i = 0; i < outputs.size(); ++i)
    {
        int resource_id = outputs[i].key;
        int amount = outputs[i].value;
        if (amount > 0)
            game.add_ore(planet_id, resource_id, amount);
    }
}

void BuildingManager::ensure_outputs_registered(Game &game, int planet_id, const ft_vector<Pair<int, int> > &outputs)
{
    for (size_t i = 0; i < outputs.size(); ++i)
    {
        int resource_id = outputs[i].key;
        game.ensure_planet_item_slot(planet_id, resource_id);
    }
}

void BuildingManager::initialize_planet(Game &game, int planet_id)
{
    if (this->_planets.find(planet_id) != ft_nullptr)
        return ;
    this->_planets.insert(planet_id, ft_planet_build_state());
    Pair<int, ft_planet_build_state> *entry = this->_planets.find(planet_id);
    if (entry == ft_nullptr)
        return ;
    ft_planet_build_state &stored = entry->value;
    stored.planet_id = planet_id;
    stored.base_logistic = 1;
    stored.research_logistic_bonus = 0;
    stored.base_energy_generation = 0.0;
    if (planet_id == PLANET_TERRA)
    {
        stored.width = 4;
        stored.height = 4;
    }
    else if (planet_id == PLANET_MARS)
    {
        stored.width = 3;
        stored.height = 3;
    }
    else if (planet_id == PLANET_ZALTHOR)
    {
        stored.width = 3;
        stored.height = 4;
    }
    else if (planet_id == PLANET_VULCAN)
    {
        stored.width = 4;
        stored.height = 4;
    }
    else
    {
        stored.width = 4;
        stored.height = 5;
    }
    stored.grid.clear();
    stored.grid.resize(static_cast<size_t>(stored.width * stored.height), 0);
    stored.used_plots = 0;
    stored.logistic_capacity = 0;
    stored.logistic_usage = 0;
    stored.energy_generation = 0.0;
    stored.energy_consumption = 0.0;
    stored.support_energy = 0.0;
    stored.mine_multiplier = 1.0;
    stored.helios_projection = 0.0;
    stored.helios_incoming = 0.0;
    stored.next_instance_id = 1;
    stored.instances.clear();
    const ft_building_definition *mine = this->get_definition(BUILDING_MINE_CORE);
    if (mine != ft_nullptr)
    {
        if (mine->occupies_grid)
            this->occupy_area(stored, stored.next_instance_id, 0, 0, mine->width, mine->height);
        ft_building_instance instance;
        instance.uid = stored.next_instance_id++;
        instance.definition_id = BUILDING_MINE_CORE;
        instance.x = 0;
        instance.y = 0;
        instance.progress = 0.0;
        instance.active = false;
        stored.instances.insert(instance.uid, instance);
    }
    recalculate_planet_statistics(stored);
    this->refresh_helios_network();
    game.ensure_planet_item_slot(planet_id, ITEM_IRON_BAR);
    game.ensure_planet_item_slot(planet_id, ITEM_COPPER_BAR);
    game.ensure_planet_item_slot(planet_id, ITEM_MITHRIL_BAR);
    game.ensure_planet_item_slot(planet_id, ITEM_ENGINE_PART);
    game.ensure_planet_item_slot(planet_id, ITEM_TITANIUM_BAR);
    game.ensure_planet_item_slot(planet_id, ITEM_ADVANCED_ENGINE_PART);
    game.ensure_planet_item_slot(planet_id, ITEM_FUSION_REACTOR);
    game.ensure_planet_item_slot(planet_id, ITEM_ACCUMULATOR);
    game.ensure_planet_item_slot(planet_id, ORE_TRITIUM);
}

