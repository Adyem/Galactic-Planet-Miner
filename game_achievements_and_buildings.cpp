// Auto-generated split part 5 of game.cpp

        if (!definition_ptr)
            continue;
        const ft_quest_definition &definition = *definition_ptr;
        if (definition.is_side_quest)
            continue;
        if (this->_quests.get_status(definition.id) == QUEST_STATUS_COMPLETED)
            completed_main += 1;
    }
    int stage = completed_main - 1;
    if (stage < 0)
        stage = 0;
    double reduction = 0.03 * static_cast<double>(stage);
    if (reduction > 0.25)
        reduction = 0.25;
    double dynamic_scale = 1.0 - reduction;
    if (dynamic_scale < 0.75)
        dynamic_scale = 0.75;
    if (dynamic_scale > 1.0)
        dynamic_scale = 1.0;
    if (math_fabs(dynamic_scale - this->_quest_time_scale_dynamic) > 0.0001)
        this->_quest_time_scale_dynamic = dynamic_scale;
    this->apply_quest_time_scale();
}

void Game::update_combat_modifiers()
{
    this->_combat.set_player_weapon_multiplier(this->_ship_weapon_multiplier);
    this->_combat.set_player_shield_multiplier(this->_ship_shield_multiplier);
    this->_combat.set_player_hull_multiplier(this->_ship_hull_multiplier);
}

double Game::compute_auto_shield_generator_stability(int planet_id) const
{
    if (!this->_shield_support_unlocked)
        return 0.0;
    const ft_building_definition *definition = this->_buildings.get_definition(BUILDING_SHIELD_GENERATOR);
    if (definition == ft_nullptr)
        return 0.0;
    int generator_count = this->_buildings.get_building_count(planet_id, BUILDING_SHIELD_GENERATOR);
    if (generator_count <= 0)
        return 0.0;
    double generator_cost = definition->energy_cost * static_cast<double>(generator_count);
    if (generator_cost <= 0.0)
        return 1.0;
    double generation = this->_buildings.get_planet_energy_generation(planet_id);
    double consumption = this->_buildings.get_planet_energy_consumption(planet_id);
    double non_generator_consumption = consumption - generator_cost;
    if (non_generator_consumption < 0.0)
        non_generator_consumption = 0.0;
    double available = generation - non_generator_consumption;
    if (available <= 0.0)
        return 0.0;
    double stability = available / generator_cost;
    if (stability < 0.0)
        stability = 0.0;
    if (stability > 1.0)
        stability = 1.0;
    return stability;
}

void Game::record_achievement_event(int event_id, int value)
{
    ft_vector<int> completed;
    this->_achievements.record_event(event_id, value, &completed);
    if (completed.size() == 0)
        return ;
    this->announce_achievements(completed);
}

int Game::get_quest_achievement_event(int quest_id) const
{
    if (quest_id == QUEST_INITIAL_SKIRMISHES)
        return ACHIEVEMENT_EVENT_QUEST_INITIAL_SKIRMISHES;
    if (quest_id == QUEST_DEFENSE_OF_TERRA)
        return ACHIEVEMENT_EVENT_QUEST_DEFENSE_OF_TERRA;
    if (quest_id == QUEST_INVESTIGATE_RAIDERS)
        return ACHIEVEMENT_EVENT_QUEST_INVESTIGATE_RAIDERS;
    if (quest_id == QUEST_SECURE_SUPPLY_LINES)
        return ACHIEVEMENT_EVENT_QUEST_SECURE_SUPPLY_LINES;
    if (quest_id == QUEST_STEADY_SUPPLY_STREAK)
        return ACHIEVEMENT_EVENT_QUEST_STEADY_SUPPLY_STREAK;
    if (quest_id == QUEST_HIGH_VALUE_ESCORT)
        return ACHIEVEMENT_EVENT_QUEST_HIGH_VALUE_ESCORT;
    if (quest_id == QUEST_CLIMACTIC_BATTLE)
        return ACHIEVEMENT_EVENT_QUEST_CLIMACTIC_BATTLE;
    if (quest_id == QUEST_CRITICAL_DECISION)
        return ACHIEVEMENT_EVENT_QUEST_CRITICAL_DECISION;
    if (quest_id == QUEST_ORDER_UPRISING)
        return ACHIEVEMENT_EVENT_QUEST_ORDER_UPRISING;
    if (quest_id == QUEST_REBELLION_FLEET)
        return ACHIEVEMENT_EVENT_QUEST_REBELLION_FLEET;
    if (quest_id == QUEST_ORDER_SUPPRESS_RAIDS)
        return ACHIEVEMENT_EVENT_QUEST_ORDER_SUPPRESS_RAIDS;
    if (quest_id == QUEST_ORDER_DOMINION)
        return ACHIEVEMENT_EVENT_QUEST_ORDER_DOMINION;
    if (quest_id == QUEST_ORDER_FINAL_VERDICT)
        return ACHIEVEMENT_EVENT_QUEST_ORDER_FINAL_VERDICT;
    if (quest_id == QUEST_REBELLION_NETWORK)
        return ACHIEVEMENT_EVENT_QUEST_REBELLION_NETWORK;
    if (quest_id == QUEST_REBELLION_LIBERATION)
        return ACHIEVEMENT_EVENT_QUEST_REBELLION_LIBERATION;
    if (quest_id == QUEST_REBELLION_FINAL_PUSH)
        return ACHIEVEMENT_EVENT_QUEST_REBELLION_FINAL_PUSH;
    return 0;
}

void Game::record_quest_achievement(int quest_id)
{
    int event_id = this->get_quest_achievement_event(quest_id);
    if (event_id != 0)
        this->record_achievement_event(event_id, 1);
}

void Game::announce_achievements(const ft_vector<int> &achievement_ids)
{
    for (size_t i = 0; i < achievement_ids.size(); ++i)
    {
        ft_achievement_info info;
        if (!this->_achievements.get_info(achievement_ids[i], info))
            continue;
        ft_string entry("Archivist Lyra records achievement ");
        entry.append(info.name);
        entry.append(ft_string(": "));
        entry.append(info.description);
        this->append_lore_entry(entry);
    }
}

bool Game::is_planet_unlocked(int planet_id) const
{
    const Pair<int, ft_sharedptr<ft_planet> > *entry = this->_planets.find(planet_id);
    return entry != ft_nullptr;
}

bool Game::can_place_building(int planet_id, int building_id, int x, int y) const
{
    if (!this->is_planet_unlocked(planet_id))
        return false;
    return this->_buildings.can_place_building(*this, planet_id, building_id, x, y);
}

int Game::place_building(int planet_id, int building_id, int x, int y)
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.place_building(*this, planet_id, building_id, x, y);
}

bool Game::remove_building(int planet_id, int instance_id)
{
    if (!this->is_planet_unlocked(planet_id))
        return false;
    return this->_buildings.remove_building(*this, planet_id, instance_id);
}

int Game::get_building_instance(int planet_id, int x, int y) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.get_building_instance(planet_id, x, y);
}

int Game::get_building_count(int planet_id, int building_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.get_building_count(planet_id, building_id);
}

int Game::get_planet_plot_capacity(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.get_planet_plot_capacity(planet_id);
}

int Game::get_planet_plot_usage(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.get_planet_plot_usage(planet_id);
}

int Game::get_planet_logistic_capacity(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.get_planet_logistic_capacity(planet_id);
}

int Game::get_planet_logistic_usage(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.get_planet_logistic_usage(planet_id);
}

double Game::get_planet_energy_generation(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0.0;
    return this->_buildings.get_planet_energy_generation(planet_id);
}

double Game::get_planet_energy_consumption(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0.0;
    return this->_buildings.get_planet_energy_consumption(planet_id);
}

double Game::get_planet_support_energy(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0.0;
    return this->_buildings.get_planet_support_energy(planet_id);
}

double Game::get_planet_mine_multiplier(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 1.0;
    return this->_buildings.get_mine_multiplier(planet_id);
}

double Game::get_planet_energy_pressure(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0.0;
    return this->_buildings.get_planet_energy_pressure(planet_id);
}

