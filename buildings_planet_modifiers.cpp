void BuildingManager::add_planet_logistic_bonus(int planet_id, int amount)
{
    if (amount <= 0)
        return ;
    ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return ;
    state->research_logistic_bonus += amount;
    if (state->research_logistic_bonus < 0)
        state->research_logistic_bonus = 0;
    recalculate_planet_statistics(*state);
    this->refresh_helios_network();
}

void BuildingManager::apply_research_unlock(int research_id)
{
    if (research_id == RESEARCH_SOLAR_PANELS)
        this->set_building_unlocked(BUILDING_SOLAR_ARRAY, true);
    else if (research_id == RESEARCH_CRAFTING_MASTERY)
    {
        this->set_building_unlocked(BUILDING_FACILITY_WORKSHOP, true);
        this->set_building_unlocked(BUILDING_SHIPYARD, true);
    }
    else if (research_id == RESEARCH_DEFENSIVE_FORTIFICATION_I)
    {
        this->set_building_unlocked(BUILDING_PROXIMITY_RADAR, true);
        this->set_building_unlocked(BUILDING_DEFENSE_TURRET, true);
    }
    else if (research_id == RESEARCH_DEFENSIVE_FORTIFICATION_II)
        this->set_building_unlocked(BUILDING_MOBILE_RADAR, true);
    else if (research_id == RESEARCH_DEFENSIVE_FORTIFICATION_III)
        this->set_building_unlocked(BUILDING_HELIOS_BEACON, true);
    else if (research_id == RESEARCH_ARMAMENT_ENHANCEMENT_I)
        this->set_building_unlocked(BUILDING_PLASMA_TURRET, true);
    else if (research_id == RESEARCH_ARMAMENT_ENHANCEMENT_III)
        this->set_building_unlocked(BUILDING_RAILGUN_TURRET, true);
    else if (research_id == RESEARCH_SHIELD_TECHNOLOGY)
        this->set_building_unlocked(BUILDING_SHIELD_GENERATOR, true);
    else if (research_id == RESEARCH_TRITIUM_EXTRACTION)
        this->set_building_unlocked(BUILDING_TRITIUM_EXTRACTOR, true);
    else if (research_id == RESEARCH_INTERSTELLAR_TRADE)
        this->set_building_unlocked(BUILDING_TRADE_RELAY, true);
    else if (research_id == RESEARCH_CAPITAL_SHIP_INITIATIVE)
        this->set_building_unlocked(BUILDING_FLAGSHIP_DOCK, true);
}

void BuildingManager::set_crafting_energy_multiplier(double multiplier)
{
    if (multiplier <= 0.0)
        multiplier = 1.0;
    this->_crafting_energy_multiplier = multiplier;
}

void BuildingManager::set_crafting_speed_multiplier(double multiplier)
{
    if (multiplier <= 0.0)
        multiplier = 1.0;
    this->_crafting_speed_multiplier = multiplier;
}

void BuildingManager::set_global_energy_multiplier(double multiplier)
{
    if (multiplier <= 0.0)
        multiplier = 1.0;
    this->_global_energy_multiplier = multiplier;
}

