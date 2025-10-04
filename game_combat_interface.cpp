#include "game.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/Template/pair.hpp"

bool Game::start_raider_assault(int planet_id, double difficulty, int control_mode)
{
    if (!this->is_planet_unlocked(planet_id))
        return false;
    this->_buildings.tick(*this, 0.0);
    double raw_energy_pressure = this->_buildings.get_planet_energy_pressure(planet_id);
    if (raw_energy_pressure < 0.0)
        raw_energy_pressure = 0.0;
    double normalized_energy = raw_energy_pressure / 4.0;
    double generation = this->_buildings.get_planet_energy_generation(planet_id);
    double consumption = this->_buildings.get_planet_energy_consumption(planet_id);
    if (generation > 0.0)
    {
        double ratio = consumption / generation;
        if (ratio > 0.95)
        {
            double extra = (ratio - 0.95) * 10.0;
            normalized_energy += extra;
        }
    }
    if (normalized_energy > 1.5)
        normalized_energy = 1.5;
    double narrative_pressure = 0.0;
    int active_quest = this->_quests.get_active_quest_id();
    if (active_quest == QUEST_CLIMACTIC_BATTLE || active_quest == QUEST_CRITICAL_DECISION)
        narrative_pressure = 0.35;
    else if (active_quest == QUEST_ORDER_UPRISING || active_quest == QUEST_REBELLION_FLEET)
        narrative_pressure = 0.4;
    else if (active_quest != 0)
        narrative_pressure = 0.2;
    double scaled = difficulty * this->_assault_difficulty_multiplier;
    if (scaled <= 0.0)
        scaled = difficulty;
    if (!this->_combat.start_raider_assault(planet_id, scaled, normalized_energy, narrative_pressure, control_mode))
        return false;
    this->get_planet_fleet(planet_id);
    ft_string entry("Navigator Zara signals a raider incursion on planet ");
    entry.append(ft_to_string(planet_id));
    entry.append(ft_string(": defenses are mobilizing."));
    this->append_lore_entry(entry);
    return true;
}

bool Game::assign_fleet_to_assault(int planet_id, int fleet_id)
{
    if (!this->_combat.is_assault_active(planet_id))
        return false;
    Pair<int, ft_sharedptr<ft_fleet> > *entry = this->_fleets.find(fleet_id);
    if (entry == ft_nullptr || !entry->value)
        return false;
    return this->_combat.add_fleet(planet_id, fleet_id);
}

bool Game::set_assault_support(int planet_id, bool sunflare_docked,
                               bool repair_drones_active, bool shield_generator_online)
{
    if (repair_drones_active && !this->_repair_drones_unlocked)
        return false;
    if (shield_generator_online && !this->_shield_support_unlocked)
        return false;
    return this->_combat.set_support(planet_id, sunflare_docked, repair_drones_active, shield_generator_online);
}

bool Game::set_assault_sunflare_target(int planet_id, int fleet_id, int ship_uid)
{
    if (!this->_shield_support_unlocked)
        return false;
    if (!this->_combat.is_assault_active(planet_id))
        return false;
    if (fleet_id == 0 || ship_uid == 0)
        return this->_combat.set_sunflare_dock_target(planet_id, 0, 0);
    ft_sharedptr<ft_fleet> fleet;
    Pair<int, ft_sharedptr<ft_fleet> > *entry = this->_fleets.find(fleet_id);
    if (entry != ft_nullptr)
        fleet = entry->value;
    if (!fleet)
    {
        ft_sharedptr<ft_fleet> garrison = this->get_planet_fleet(planet_id);
        if (garrison && garrison->get_id() == fleet_id)
            fleet = garrison;
    }
    if (!fleet)
        return false;
    if (fleet->get_ship(ship_uid) == ft_nullptr)
        return false;
    return this->_combat.set_sunflare_dock_target(planet_id, fleet_id, ship_uid);
}

bool Game::set_assault_control_mode(int planet_id, int control_mode)
{
    return this->_combat.set_control_mode(planet_id, control_mode);
}

bool Game::set_assault_aggression(int planet_id, double aggression)
{
    return this->_combat.set_raider_aggression(planet_id, aggression);
}

bool Game::trigger_assault_focus_fire(int planet_id)
{
    return this->_combat.trigger_focus_fire(planet_id);
}

bool Game::request_assault_tactical_pause(int planet_id)
{
    return this->_combat.request_tactical_pause(planet_id);
}

bool Game::is_assault_active(int planet_id) const
{
    return this->_combat.is_assault_active(planet_id);
}

double Game::get_assault_raider_shield(int planet_id) const
{
    return this->_combat.get_raider_shield(planet_id);
}

double Game::get_assault_raider_hull(int planet_id) const
{
    return this->_combat.get_raider_hull(planet_id);
}

bool Game::get_assault_raider_positions(int planet_id, ft_vector<ft_ship_spatial_state> &out) const
{
    return this->_combat.get_raider_positions(planet_id, out);
}

bool Game::get_assault_defender_positions(int planet_id, ft_vector<ft_ship_spatial_state> &out) const
{
    return this->_combat.get_defender_positions(planet_id, out);
}

