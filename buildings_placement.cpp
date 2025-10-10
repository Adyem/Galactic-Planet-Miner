int BuildingManager::place_building(Game &game, int planet_id, int building_id, int x, int y)
{
    ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    const ft_building_definition *definition = this->get_definition(building_id);
    if (definition == ft_nullptr)
        return 0;
    if (!this->is_building_unlocked(building_id))
        return 0;
    if (state->next_instance_id >= FT_BUILDING_INSTANCE_ID_MAX)
        return 0;
    if (definition->unique && this->get_building_count(planet_id, building_id) > 0)
        return 0;
    if (definition->occupies_grid && !this->is_area_free(*state, x, y, definition->width, definition->height))
        return 0;
    if (!this->check_build_costs(game, planet_id, definition->build_costs))
        return 0;
    this->pay_build_costs(game, planet_id, definition->build_costs);
    if (definition->occupies_grid)
        this->occupy_area(*state, state->next_instance_id, x, y, definition->width, definition->height);
    ft_building_instance instance;
    instance.uid = state->next_instance_id++;
    instance.definition_id = building_id;
    instance.x = x;
    instance.y = y;
    instance.progress = 0.0;
    instance.active = false;
    state->instances.insert(instance.uid, instance);
    this->ensure_outputs_registered(game, planet_id, definition->outputs);
    recalculate_planet_statistics(*state);
    this->refresh_helios_network();
    return instance.uid;
}

bool BuildingManager::remove_building(Game &game, int planet_id, int instance_id)
{
    (void)game;
    ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return false;
    Pair<int, ft_building_instance> *entry = state->instances.find(instance_id);
    if (entry == ft_nullptr)
        return false;
    const ft_building_definition *definition = this->get_definition(entry->value.definition_id);
    if (definition == ft_nullptr)
        return false;
    if (!definition->removable)
        return false;
    this->clear_area(*state, instance_id);
    state->instances.remove(instance_id);
    recalculate_planet_statistics(*state);
    this->refresh_helios_network();
    return true;
}

bool BuildingManager::can_place_building(const Game &game, int planet_id, int building_id, int x, int y) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return false;
    const ft_building_definition *definition = this->get_definition(building_id);
    if (definition == ft_nullptr)
        return false;
    if (!this->is_building_unlocked(building_id))
        return false;
    if (definition->unique && this->get_building_count(planet_id, building_id) > 0)
        return false;
    if (definition->occupies_grid && !this->is_area_free(*state, x, y, definition->width, definition->height))
        return false;
    if (!this->check_build_costs(game, planet_id, definition->build_costs))
        return false;
    return true;
}

