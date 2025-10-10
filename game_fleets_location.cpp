void Game::set_fleet_location_planet(int fleet_id, int planet_id)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_location_planet(planet_id);
}

void Game::set_fleet_location_travel(int fleet_id, int from, int to, double time)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_location_travel(from, to, time);
}

void Game::set_fleet_location_misc(int fleet_id, int misc_id)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_location_misc(misc_id);
}

ft_location Game::get_fleet_location(int fleet_id) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ft_location();
    return fleet->get_location();
}

double Game::get_fleet_travel_time(int fleet_id) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0.0;
    return fleet->get_travel_time();
}

int Game::get_planet_fleet_ship_hp(int planet_id, int ship_uid) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_planet_fleet(planet_id);
    if (!fleet)
        return 0;
    return fleet->get_ship_hp(ship_uid);
}

ft_location Game::get_planet_fleet_location(int planet_id) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_planet_fleet(planet_id);
    if (!fleet)
        return ft_location();
    return fleet->get_location();
}

void Game::build_fleet_management_entry(const ft_fleet &fleet, ft_fleet_management_entry &out, bool is_garrison,
    int garrison_planet_id) const
{
    out.fleet_id = fleet.get_id();
    out.is_garrison = is_garrison;
    out.station_planet_id = is_garrison ? garrison_planet_id : 0;
    ft_location location = fleet.get_location();
    out.location_type = location.type;
    out.origin_planet_id = location.from;
    out.destination_planet_id = location.to;
    out.misc_location_id = location.misc;
    out.travel_time = fleet.get_travel_time();
    out.ship_count = fleet.get_ship_count();
    out.total_hp = fleet.get_total_ship_hp();
    out.total_shield = fleet.get_total_ship_shield();
    out.escort_veterancy = fleet.get_escort_veterancy();
    out.escort_veterancy_bonus = fleet.get_escort_veterancy_bonus();
    out.average_weapon_range = 0.0;
    out.average_attack_speed = 0.0;
    out.ships.clear();

    ft_vector<int> ship_ids;
    fleet.get_ship_ids(ship_ids);
    double total_range = 0.0;
    double total_attack_speed = 0.0;
    size_t ship_count = ship_ids.size();
    for (size_t i = 0; i < ship_count; ++i)
    {
        const ft_ship *ship = fleet.get_ship(ship_ids[i]);
        if (!ship)
            continue;
        ft_ship_management_entry entry;
        entry.ship_uid = ship->id;
        entry.ship_type = ship->type;
        entry.role = ship->role;
        entry.combat_behavior = ship->combat_behavior;
        entry.outnumbered_behavior = ship->outnumbered_behavior;
        entry.unescorted_behavior = ship->unescorted_behavior;
        entry.low_hp_behavior = ship->low_hp_behavior;
        entry.armor = ship->armor;
        entry.hp = ship->hp;
        entry.max_hp = ship->max_hp;
        entry.shield = ship->shield;
        entry.max_shield = ship->max_shield;
        entry.max_speed = ship->max_speed;
        entry.acceleration = ship->acceleration;
        entry.deceleration = ship->deceleration;
        entry.turn_speed = ship->turn_speed;
        entry.optimal_range = ship->optimal_range;
        entry.max_range = ship->max_range;
        entry.attack_speed = ship->attack_speed;
        entry.base_damage = ship->base_damage;
        out.ships.push_back(entry);
        total_range += entry.max_range;
        total_attack_speed += entry.attack_speed;
    }
    size_t summary_count = out.ships.size();
    if (summary_count > 0)
    {
        double denominator = static_cast<double>(summary_count);
        out.average_weapon_range = total_range / denominator;
        out.average_attack_speed = total_attack_speed / denominator;
    }
    else
    {
        out.average_weapon_range = 0.0;
        out.average_attack_speed = 0.0;
    }
}

void Game::get_fleet_management_snapshot(ft_fleet_management_snapshot &out) const
{
    out.player_fleets.clear();
    out.traveling_fleets.clear();
    out.planet_garrisons.clear();

    size_t player_count = this->_fleets.size();
    if (player_count > 0)
    {
        const Pair<int, ft_sharedptr<ft_fleet> > *entries = this->_fleets.end();
        entries -= player_count;
        for (size_t i = 0; i < player_count; ++i)
        {
            const ft_sharedptr<ft_fleet> &fleet = entries[i].value;
            if (!fleet)
                continue;
            ft_fleet_management_entry entry;
            this->build_fleet_management_entry(*fleet, entry, false, 0);
            if (entry.location_type == LOCATION_TRAVEL)
                out.traveling_fleets.push_back(ft_move(entry));
            else
                out.player_fleets.push_back(ft_move(entry));
        }
    }

    size_t garrison_count = this->_planet_fleets.size();
    if (garrison_count > 0)
    {
        const Pair<int, ft_sharedptr<ft_fleet> > *entries = this->_planet_fleets.end();
        entries -= garrison_count;
        for (size_t i = 0; i < garrison_count; ++i)
        {
            const ft_sharedptr<ft_fleet> &fleet = entries[i].value;
            if (!fleet)
                continue;
            ft_fleet_management_entry entry;
            this->build_fleet_management_entry(*fleet, entry, true, entries[i].key);
            out.planet_garrisons.push_back(ft_move(entry));
        }
    }
}
