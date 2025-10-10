        return false;
    ft_location location = fleet->get_location();
    if (location.type != LOCATION_PLANET || location.from != origin_planet_id)
        return false;
    size_t assignment_count = this->_route_convoy_escorts.size();
    if (assignment_count > 0)
    {
        Pair<int, int> *assignments = this->_route_convoy_escorts.end();
        assignments -= assignment_count;
        for (size_t i = 0; i < assignment_count; ++i)
        {
            if (assignments[i].value == fleet_id && assignments[i].key != route->id)
                return false;
        }
    }
    Pair<int, int> *existing = this->_route_convoy_escorts.find(route->id);
    if (existing != ft_nullptr)
        existing->value = fleet_id;
    else
        this->_route_convoy_escorts.insert(route->id, fleet_id);
    return true;
}

bool Game::clear_convoy_escort(int origin_planet_id, int destination_planet_id)
{
    if (origin_planet_id == destination_planet_id)
        return false;
    const ft_supply_route *route = this->find_supply_route(origin_planet_id, destination_planet_id);
    if (!route)
        return false;
    Pair<int, int> *entry = this->_route_convoy_escorts.find(route->id);
    if (entry == ft_nullptr)
        return false;
    this->_route_convoy_escorts.remove(route->id);
    return true;
}

int Game::get_assigned_convoy_escort(int origin_planet_id, int destination_planet_id) const
{
    if (origin_planet_id == destination_planet_id)
        return 0;
    const ft_supply_route *route = this->find_supply_route(origin_planet_id, destination_planet_id);
    if (!route)
        return 0;
    const Pair<int, int> *entry = this->_route_convoy_escorts.find(route->id);
    if (entry == ft_nullptr)
        return 0;
    return entry->value;
}

int Game::dispatch_convoy(const ft_supply_route &route, int origin_planet_id,
                          int destination_planet_id, int resource_id,
                          int amount, int contract_id, int escort_fleet_id)
{
    if (amount <= 0)
        return 0;
    ft_supply_convoy convoy;
    convoy.id = this->_next_convoy_id++;
    convoy.route_id = route.id;
    convoy.contract_id = contract_id;
    convoy.origin_planet_id = origin_planet_id;
    convoy.destination_planet_id = destination_planet_id;
    convoy.resource_id = resource_id;
    convoy.amount = amount;
    convoy.origin_escort = this->calculate_planet_escort_rating(origin_planet_id);
    convoy.destination_escort = this->calculate_planet_escort_rating(destination_planet_id);
    int claimed_escort_id = 0;
    bool escort_was_claimed = false;
    convoy.route_escort_claimed = false;
    if (escort_fleet_id <= 0)
    {
        claimed_escort_id = this->claim_route_escort(route.id);
        if (claimed_escort_id > 0)
        {
            escort_fleet_id = claimed_escort_id;
            escort_was_claimed = true;
        }
    }
    if (escort_fleet_id > 0)
    {
        ft_sharedptr<ft_fleet> escort = this->get_fleet(escort_fleet_id);
        bool valid = true;
        if (!escort)
            valid = false;
        if (valid)
        {
            if (this->is_fleet_escorting_convoy(escort_fleet_id))
                valid = false;
        }
        if (valid)
        {
            ft_location escort_location = escort->get_location();
            if (escort_location.type != LOCATION_PLANET || escort_location.from != origin_planet_id)
                valid = false;
        }
        if (valid)
        {
            convoy.escort_fleet_id = escort_fleet_id;
            convoy.escort_rating = this->calculate_fleet_escort_rating(*escort);
            if (escort_was_claimed)
                convoy.route_escort_claimed = true;
        }
        else if (escort_was_claimed)
        {
            this->_route_convoy_escorts.remove(route.id);
        }
    }
    int effective_origin = convoy.origin_escort + convoy.escort_rating;
    if (effective_origin > 48)
        effective_origin = 48;
    int effective_destination = convoy.destination_escort + convoy.escort_rating;
    if (effective_destination > 48)
        effective_destination = 48;
    convoy.remaining_time = this->calculate_convoy_travel_time(route, effective_origin, effective_destination);
    if (convoy.escort_fleet_id > 0)
        this->set_fleet_location_travel(convoy.escort_fleet_id, origin_planet_id,
                                        destination_planet_id, convoy.remaining_time);
    this->_active_convoys.insert(convoy.id, convoy);
    ft_string entry("Quartermaster Nia dispatches a convoy from ");
    entry.append(ft_to_string(origin_planet_id));
    entry.append(ft_string(" to "));
    entry.append(ft_to_string(destination_planet_id));
    entry.append(ft_string(" carrying "));
    entry.append(ft_to_string(amount));
    entry.append(ft_string(" units"));
    if (contract_id > 0)
    {
        entry.append(ft_string(" (Contract #"));
        entry.append(ft_to_string(contract_id));
        entry.append(ft_string(")"));
    }
    entry.append(ft_string("."));
    if (convoy.escort_fleet_id > 0)
    {
        entry.append(ft_string(" Escort fleet #"));
        entry.append(ft_to_string(convoy.escort_fleet_id));
        entry.append(ft_string(" forms up for protection"));
        int veterancy_bonus = this->get_fleet_escort_veterancy_bonus(convoy.escort_fleet_id);
        if (veterancy_bonus > 0)
        {
            entry.append(ft_string(", bringing a +"));
            entry.append(ft_to_string(veterancy_bonus));
            entry.append(ft_string(" escort bonus from prior runs"));
        }
        entry.append(ft_string("."));
    }
    double origin_speed_bonus = this->_buildings.get_planet_convoy_speed_bonus(origin_planet_id);
    double destination_speed_bonus = this->_buildings.get_planet_convoy_speed_bonus(destination_planet_id);
    double origin_risk_modifier = this->_buildings.get_planet_convoy_raid_risk_modifier(origin_planet_id);
    double destination_risk_modifier = this->_buildings.get_planet_convoy_raid_risk_modifier(destination_planet_id);
    bool has_speed_bonus = (origin_speed_bonus > 0.0 || destination_speed_bonus > 0.0);
    bool has_risk_bonus = (origin_risk_modifier > 0.0 || destination_risk_modifier > 0.0);
    if (has_speed_bonus || has_risk_bonus)
    {
        entry.append(ft_string(" Trade relays "));
        if (has_speed_bonus && has_risk_bonus)
            entry.append(ft_string("accelerate this route and blunt raider odds"));
        else if (has_speed_bonus)
            entry.append(ft_string("accelerate this route"));
        else
            entry.append(ft_string("blunt raider odds"));
        entry.append(ft_string("."));
    }
    this->append_lore_entry(entry);
    return amount;
}

double Game::estimate_route_travel_time(int origin, int destination) const
{
    if (origin == destination)
        return 0.0;
    bool involves_terra = (origin == PLANET_TERRA || destination == PLANET_TERRA);
    bool involves_mars = (origin == PLANET_MARS || destination == PLANET_MARS);
    bool involves_luna = (origin == PLANET_LUNA || destination == PLANET_LUNA);
    bool involves_zalthor = (origin == PLANET_ZALTHOR || destination == PLANET_ZALTHOR);
    bool involves_vulcan = (origin == PLANET_VULCAN || destination == PLANET_VULCAN);
    bool involves_noctaris = (origin == PLANET_NOCTARIS_PRIME || destination == PLANET_NOCTARIS_PRIME);
    bool passes_barrens = route_passes_celestial_barrens(origin, destination);
    double base = 28.0;
    if (passes_barrens)
        base = 36.0;
    else if (involves_terra && involves_luna)
        base = 18.0;
    else if (involves_terra && involves_mars)
        base = 24.0;
    else if ((involves_mars && involves_luna) || (involves_terra && involves_zalthor))
        base = 32.0;
    else if (involves_terra && involves_vulcan)
        base = 40.0;
    else if (involves_vulcan && involves_noctaris)
        base = 44.0;
    else if (involves_terra && involves_noctaris)
        base = 52.0;
    else if (involves_noctaris)
        base = 48.0;
    else if (involves_vulcan)
        base = 38.0;
    else
        base = 34.0;
    if (involves_luna && involves_noctaris)
        base += 8.0;
    if (base < 16.0)
        base = 16.0;
    return base;
}

int Game::estimate_route_escort_requirement(int origin, int destination) const
{
    if (origin == destination)
        return 0;
    bool involves_luna = (origin == PLANET_LUNA || destination == PLANET_LUNA);
    bool involves_vulcan = (origin == PLANET_VULCAN || destination == PLANET_VULCAN);
    bool involves_noctaris = (origin == PLANET_NOCTARIS_PRIME || destination == PLANET_NOCTARIS_PRIME);
    bool passes_barrens = route_passes_celestial_barrens(origin, destination);
    if (involves_noctaris)
        return 6;
    if (passes_barrens)
        return 5;
    if (involves_vulcan)
        return 4;
    if (involves_luna)
        return 1;
    return 3;
}

double Game::estimate_route_raid_risk(int origin, int destination) const
{
    if (origin == destination)
        return 0.0;
    double risk = 0.014;
    bool involves_vulcan = (origin == PLANET_VULCAN || destination == PLANET_VULCAN);
    bool involves_noctaris = (origin == PLANET_NOCTARIS_PRIME || destination == PLANET_NOCTARIS_PRIME);
    bool involves_luna = (origin == PLANET_LUNA || destination == PLANET_LUNA);
    bool passes_barrens = route_passes_celestial_barrens(origin, destination);
    if (involves_vulcan)
        risk += 0.006;
    if (involves_noctaris)
        risk += 0.012;
    if (passes_barrens)
        risk += 0.018;
    if (involves_luna)
        risk -= 0.004;
    if (risk < 0.006)
        risk = 0.006;
    return risk;
}

int Game::calculate_planet_escort_rating(int planet_id) const
{
    int rating = 0;
