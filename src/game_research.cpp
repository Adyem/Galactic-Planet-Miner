#include "game.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Template/pair.hpp"
#include "ft_map_snapshot.hpp"

bool Game::can_pay_research_cost(const ft_vector<Pair<int, int> > &costs) const
{
    for (size_t i = 0; i < costs.size(); ++i)
    {
        int ore_id = costs[i].key;
        int required = costs[i].value;
        int total = 0;
        ft_vector<Pair<int, ft_sharedptr<ft_planet> > > entries;
        ft_map_snapshot(this->_planets, entries);
        for (size_t j = 0; j < entries.size(); ++j)
        {
            const ft_sharedptr<ft_planet> &planet = entries[j].value;
            total += planet->get_resource(ore_id);
            if (total >= required)
                break;
        }
        if (total < required)
            return false;
    }
    return true;
}

void Game::pay_research_cost(const ft_vector<Pair<int, int> > &costs)
{
    for (size_t i = 0; i < costs.size(); ++i)
    {
        int ore_id = costs[i].key;
        int remaining = costs[i].value;
        if (remaining <= 0)
            continue;
        ft_vector<Pair<int, ft_sharedptr<ft_planet> > > entries;
        ft_map_snapshot(this->_planets, entries);
        for (size_t j = 0; j < entries.size() && remaining > 0; ++j)
        {
            ft_sharedptr<ft_planet> planet = entries[j].value;
            int available = planet->get_resource(ore_id);
            if (available <= 0)
                continue;
            int take = remaining < available ? remaining : available;
            planet->sub_resource(ore_id, take);
            this->send_state(planet->get_id(), ore_id);
            remaining -= take;
        }
    }
}

void Game::handle_research_completion(int research_id)
{
    const ft_research_definition *definition = this->_research.get_definition(research_id);
    if (definition == ft_nullptr)
        return ;
    bool update_modifiers = false;
    for (size_t i = 0; i < definition->unlock_planets.size(); ++i)
        this->unlock_planet(definition->unlock_planets[i]);
    this->_buildings.apply_research_unlock(research_id);
    if (research_id == RESEARCH_URBAN_PLANNING_TERRA)
        this->_buildings.add_planet_logistic_bonus(PLANET_TERRA, 4);
    else if (research_id == RESEARCH_URBAN_PLANNING_MARS)
        this->_buildings.add_planet_logistic_bonus(PLANET_MARS, 4);
    else if (research_id == RESEARCH_URBAN_PLANNING_ZALTHOR)
        this->_buildings.add_planet_logistic_bonus(PLANET_ZALTHOR, 4);
    else if (research_id == RESEARCH_CRAFTING_MASTERY)
        this->_buildings.set_crafting_energy_multiplier(0.8);
    else if (research_id == RESEARCH_STRUCTURAL_REINFORCEMENT_I
        || research_id == RESEARCH_STRUCTURAL_REINFORCEMENT_II
        || research_id == RESEARCH_STRUCTURAL_REINFORCEMENT_III)
    {
        this->_ship_hull_multiplier += 0.1;
        update_modifiers = true;
    }
    else if (research_id == RESEARCH_DEFENSIVE_FORTIFICATION_I
        || research_id == RESEARCH_DEFENSIVE_FORTIFICATION_II
        || research_id == RESEARCH_DEFENSIVE_FORTIFICATION_III)
    {
        this->_ship_shield_multiplier += 0.1;
        update_modifiers = true;
    }
    else if (research_id == RESEARCH_ARMAMENT_ENHANCEMENT_I
        || research_id == RESEARCH_ARMAMENT_ENHANCEMENT_II
        || research_id == RESEARCH_ARMAMENT_ENHANCEMENT_III)
    {
        this->_ship_weapon_multiplier += 0.1;
        update_modifiers = true;
    }
    else if (research_id == RESEARCH_FASTER_CRAFTING)
        this->_buildings.set_crafting_speed_multiplier(1.25);
    else if (research_id == RESEARCH_EMERGENCY_ENERGY_CONSERVATION)
        this->_buildings.set_global_energy_multiplier(0.9);
    else if (research_id == RESEARCH_SHIELD_TECHNOLOGY)
        this->_shield_support_unlocked = true;
    else if (research_id == RESEARCH_REPAIR_DRONE_TECHNOLOGY)
        this->_repair_drones_unlocked = true;
    else if (research_id == RESEARCH_CAPITAL_SHIP_INITIATIVE)
        this->_capital_ship_limit = 1;
    else if (research_id == RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT)
        this->_capital_ship_limit = 2;
    else if (research_id == RESEARCH_ESCAPE_POD_LIFELINE)
    {
        this->_escape_pod_protocol = true;
        ft_vector<Pair<int, bool> > entries;
        ft_map_snapshot(this->_escape_pod_rescued, entries);
        for (size_t i = 0; i < entries.size(); ++i)
        {
            Pair<int, bool> *entry = this->_escape_pod_rescued.find(entries[i].key);
            if (entry != ft_nullptr)
                entry->value = false;
        }
    }
    this->record_achievement_event(ACHIEVEMENT_EVENT_RESEARCH_COMPLETED, 1);
    if (update_modifiers)
        this->update_combat_modifiers();
    ft_string tag("research_completed_");
    tag.append(ft_to_string(research_id));
    if (!this->save_campaign_checkpoint(tag))
        return ;
}

bool Game::can_start_research(int research_id) const
{
    const ft_research_definition *definition = this->_research.get_definition(research_id);
    if (definition == ft_nullptr)
        return false;
    if (!this->_research.can_start(research_id))
        return false;
    return this->can_pay_research_cost(definition->costs);
}

bool Game::start_research(int research_id)
{
    const ft_research_definition *definition = this->_research.get_definition(research_id);
    if (definition == ft_nullptr)
        return false;
    if (!this->_research.can_start(research_id))
        return false;
    if (!this->can_pay_research_cost(definition->costs))
        return false;
    if (!this->_research.start(research_id))
        return false;
    this->pay_research_cost(definition->costs);
    return true;
}

int Game::get_research_status(int research_id) const
{
    return this->_research.get_status(research_id);
}

double Game::get_research_time_remaining(int research_id) const
{
    return this->_research.get_remaining_time(research_id);
}

