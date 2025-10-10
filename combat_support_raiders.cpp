#include "combat.hpp"

int CombatManager::add_raider_ship(ft_fleet &fleet, int ship_type, int base_hp,
    int base_shield, int armor, double scale) const
{
    double normalized = scale;
    if (normalized < 0.5)
        normalized = 0.5;
    if (normalized > 3.0)
        normalized = 3.0;
    int ship_uid = fleet.create_ship(ship_type);
    double hp_value = static_cast<double>(base_hp) * normalized;
    int scaled_hp = static_cast<int>(hp_value);
    if (static_cast<double>(scaled_hp) < hp_value)
        scaled_hp += 1;
    if (scaled_hp < 1)
        scaled_hp = 1;
    double shield_value = static_cast<double>(base_shield) * normalized;
    int scaled_shield = static_cast<int>(shield_value);
    if (static_cast<double>(scaled_shield) < shield_value)
        scaled_shield += 1;
    if (scaled_shield < 0)
        scaled_shield = 0;
    fleet.set_ship_hp(ship_uid, scaled_hp);
    fleet.set_ship_shield(ship_uid, scaled_shield);
    fleet.set_ship_armor(ship_uid, armor);
    return ship_uid;
}

void CombatManager::build_raider_fleet(ft_combat_encounter &encounter,
    double difficulty, double energy_pressure, double narrative_pressure)
{
    double scale = difficulty;
    if (scale < 0.5)
        scale = 0.5;
    if (scale > 2.5)
        scale = 2.5;
    double energy_scale = 1.0 + energy_pressure * 0.2;
    double narrative_scale = 1.0 + narrative_pressure * 0.12;
    scale *= energy_scale;
    scale *= narrative_scale;
    encounter.raider_fleet = ft_sharedptr<ft_fleet>(new ft_fleet(-(encounter.planet_id + 1000)));
    ft_fleet &fleet = *encounter.raider_fleet;
    fleet.set_location_planet(encounter.planet_id);
    this->add_raider_ship(fleet, SHIP_RAIDER_CORVETTE, 88, 28, 16, scale);
    this->add_raider_ship(fleet, SHIP_RAIDER_CORVETTE, 88, 28, 16, scale);
    this->add_raider_ship(fleet, SHIP_RAIDER_DESTROYER, 132, 46, 22, scale);
    this->add_raider_ship(fleet, SHIP_SHIELD, 52, 32, 10, scale);
    this->add_raider_ship(fleet, SHIP_SALVAGE, 50, 10, 18, scale);

    double energy_remaining = energy_pressure;
    while (energy_remaining >= 0.8)
    {
        this->add_raider_ship(fleet, SHIP_RAIDER_DESTROYER, 126, 42, 20, scale);
        energy_remaining -= 0.8;
    }
    if (energy_remaining >= 0.4)
        this->add_raider_ship(fleet, SHIP_RAIDER_CORVETTE, 84, 26, 16, scale);
    if (energy_pressure >= 1.0)
        this->add_raider_ship(fleet, SHIP_RAIDER_DESTROYER, 140, 52, 24, scale);

    int heavy = 0;
    if (narrative_pressure >= 0.3)
        heavy = 1;
    if (narrative_pressure >= 0.9)
        heavy = 2;
    for (int i = 0; i < heavy; ++i)
        this->add_raider_ship(fleet, SHIP_RAIDER_BATTLESHIP, 220, 90, 40, scale);

    double attack_multiplier = 1.25;
    if (difficulty > 0.0)
        attack_multiplier *= difficulty;
    attack_multiplier *= (1.0 + energy_pressure * 0.25);
    attack_multiplier *= (1.0 + narrative_pressure * 0.3);
    if (attack_multiplier < 0.75)
        attack_multiplier = 0.75;
    if (attack_multiplier > 4.0)
        attack_multiplier = 4.0;
    encounter.attack_multiplier = attack_multiplier;

    double defense_multiplier = 1.0;
    if (difficulty > 1.0)
        defense_multiplier += (difficulty - 1.0) * 0.2;
    else if (difficulty < 1.0)
        defense_multiplier -= (1.0 - difficulty) * 0.15;
    if (defense_multiplier < 0.7)
        defense_multiplier = 0.7;
    defense_multiplier += energy_pressure * 0.1;
    defense_multiplier += narrative_pressure * 0.05;
    if (defense_multiplier < 0.7)
        defense_multiplier = 0.7;
    if (defense_multiplier > 2.5)
        defense_multiplier = 2.5;
    encounter.defense_multiplier = defense_multiplier;
}

void CombatManager::apply_raider_support(ft_combat_encounter &encounter, double seconds,
    bool spike_active)
{
    if (!encounter.raider_fleet || seconds <= 0.0)
        return ;
    double base_factor = encounter.energy_pressure * 0.15;
    if (spike_active)
        base_factor += 0.35 + encounter.narrative_pressure * 0.15;
    else
        base_factor += encounter.narrative_pressure * 0.08;
    if (base_factor <= 0.0)
        return ;
    double shield_rate = (4.0 + encounter.energy_pressure * 3.0) * base_factor;
    double hull_rate = (2.0 + encounter.narrative_pressure * 2.0) * base_factor;
    if (shield_rate <= 0.0 && hull_rate <= 0.0)
        return ;
    encounter.pending_shield_support += shield_rate * seconds;
    encounter.pending_hull_support += hull_rate * seconds;
    int shield_amount = static_cast<int>(encounter.pending_shield_support);
    int hull_amount = static_cast<int>(encounter.pending_hull_support);
    if (shield_amount <= 0 && hull_amount <= 0)
        return ;
    encounter.pending_shield_support -= static_cast<double>(shield_amount);
    encounter.pending_hull_support -= static_cast<double>(hull_amount);
    if (shield_amount < 0)
        shield_amount = 0;
    if (hull_amount < 0)
        hull_amount = 0;
    if (shield_amount == 0 && hull_amount == 0)
        return ;
    encounter.raider_fleet->apply_support(shield_amount, hull_amount);
}
