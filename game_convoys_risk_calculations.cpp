    ft_sharedptr<const ft_fleet> garrison = this->get_planet_fleet(planet_id);
    if (garrison)
        rating += this->calculate_fleet_escort_rating(*garrison);
    int radar_bonus = 0;
    int proximity_count = this->_buildings.get_building_count(planet_id, BUILDING_PROXIMITY_RADAR);
    if (proximity_count > 0)
        radar_bonus += proximity_count * 2;
    int mobile_count = this->_buildings.get_building_count(planet_id, BUILDING_MOBILE_RADAR);
    if (mobile_count > 0)
        radar_bonus += mobile_count * 3;
    if (radar_bonus > 12)
        radar_bonus = 12;
    rating += radar_bonus;
    size_t fleet_count = this->_fleets.size();
    if (fleet_count > 0)
    {
        const Pair<int, ft_sharedptr<ft_fleet> > *entries = this->_fleets.end();
        entries -= fleet_count;
        for (size_t i = 0; i < fleet_count; ++i)
        {
            const ft_sharedptr<ft_fleet> &fleet = entries[i].value;
            if (!fleet)
                continue;
            if (this->is_fleet_escorting_convoy(fleet->get_id()))
                continue;
            ft_location loc = fleet->get_location();
            if (loc.type == LOCATION_PLANET && loc.from == planet_id)
                rating += this->calculate_fleet_escort_rating(*fleet);
        }
    }
    if (rating > 48)
        rating = 48;
    return rating;
}

int Game::calculate_fleet_escort_rating(const ft_fleet &fleet) const
{
    ft_vector<int> ship_ids;
    fleet.get_ship_ids(ship_ids);
    int rating = 0;
    for (size_t i = 0; i < ship_ids.size(); ++i)
    {
        int type = fleet.get_ship_type(ship_ids[i]);
        switch (type)
        {
        case SHIP_INTERCEPTOR:
            rating += 4;
            break;
        case SHIP_CORVETTE:
            rating += 3;
            break;
        case SHIP_FRIGATE_SOVEREIGN:
        case SHIP_FRIGATE_PREEMPTOR:
            rating += 6;
            break;
        case SHIP_FRIGATE_JUGGERNAUT:
            rating += 5;
            break;
        case SHIP_FRIGATE_CARRIER:
        case SHIP_FRIGATE_PROTECTOR:
        case SHIP_FRIGATE_ECLIPSE:
            rating += 4;
            break;
        case SHIP_CAPITAL_JUGGERNAUT:
        case SHIP_CAPITAL_NOVA:
        case SHIP_CAPITAL_OBSIDIAN:
        case SHIP_CAPITAL_PREEMPTOR:
        case SHIP_CAPITAL_PROTECTOR:
        case SHIP_CAPITAL_ECLIPSE:
            rating += 7;
            break;
        case SHIP_SUNFLARE_SLOOP:
            rating += 3;
            break;
        case SHIP_REPAIR_DRONE:
            rating += 2;
            break;
        case SHIP_SHIELD:
            rating += 2;
            break;
        case SHIP_RADAR:
            rating += 1;
            break;
        default:
            break;
        }
    }
    int veterancy_bonus = fleet.get_escort_veterancy_bonus();
    if (veterancy_bonus > 0)
        rating += veterancy_bonus;
    if (rating > 48)
        rating = 48;
    return rating;
}

bool Game::is_fleet_escorting_convoy(int fleet_id) const
{
    if (fleet_id <= 0)
        return false;
    size_t count = this->_active_convoys.size();
    if (count == 0)
        return false;
    const Pair<int, ft_supply_convoy> *entries = this->_active_convoys.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_supply_convoy &convoy = entries[i].value;
        if (convoy.escort_fleet_id == fleet_id)
            return true;
    }
    return false;
}

int Game::claim_route_escort(int route_id)
{
    Pair<int, int> *entry = this->_route_convoy_escorts.find(route_id);
    if (entry == ft_nullptr)
        return 0;
    int fleet_id = entry->value;
    this->_route_convoy_escorts.remove(route_id);
    return fleet_id;
}

double Game::calculate_convoy_travel_time(const ft_supply_route &route, int origin_escort, int destination_escort) const
{
    double time = route.base_travel_time;
    double base_floor = route.base_travel_time * 0.3;
    int strongest = origin_escort;
    if (destination_escort > strongest)
        strongest = destination_escort;
    if (route.escort_requirement > 0 && strongest > route.escort_requirement)
    {
        double surplus = static_cast<double>(strongest - route.escort_requirement);
        double reduction = surplus * 1.5;
        double minimum = route.base_travel_time * 0.5;
        time -= reduction;
        if (time < minimum)
            time = minimum;
    }
    double origin_bonus = this->_buildings.get_planet_convoy_speed_bonus(route.origin_planet_id);
    double destination_bonus = this->_buildings.get_planet_convoy_speed_bonus(route.destination_planet_id);
    double combined_bonus = origin_bonus + destination_bonus;
    if (combined_bonus > 0.6)
        combined_bonus = 0.6;
    if (combined_bonus > 0.0)
    {
        double multiplier = 1.0 - combined_bonus;
        if (multiplier < 0.3)
            multiplier = 0.3;
        time *= multiplier;
    }
    if (time < base_floor)
        time = base_floor;
    if (time < 10.0)
        time = 10.0;
    return time;
}

double Game::calculate_convoy_raid_risk(const ft_supply_convoy &convoy, bool origin_under_attack, bool destination_under_attack) const
{
    const ft_supply_route *route = this->get_route_by_id(convoy.route_id);
    if (!route)
        return 0.0;
    double risk = route->base_raid_risk;
    double campaign_focus = compute_campaign_raider_focus(
        this->_convoys_delivered_total,
        this->_order_branch_assault_victories,
        this->_rebellion_branch_assault_victories);
    if (route->threat_level > 0.0)
    {
        double threat_factor = route->threat_level;
        if (threat_factor > 6.0)
            threat_factor = 6.0;
        double threat_multiplier = 1.0 + threat_factor * 0.35;
        if (threat_multiplier > 3.5)
            threat_multiplier = 3.5;
        risk *= threat_multiplier;
    }
    int effective_origin = convoy.origin_escort + convoy.escort_rating;
    if (effective_origin > 48)
        effective_origin = 48;
    int effective_destination = convoy.destination_escort + convoy.escort_rating;
    if (effective_destination > 48)
        effective_destination = 48;
    if (route->escort_requirement > 0)
    {
        if (effective_origin < route->escort_requirement)
        {
            double deficit = static_cast<double>(route->escort_requirement - effective_origin);
            risk *= 1.6;
            risk += deficit * 0.01;
        }
        else
        {
            double surplus = static_cast<double>(effective_origin - route->escort_requirement);
            double divisor = 1.0 + surplus * 0.12;
            if (divisor > 1.0)
                risk /= divisor;
        }
        if (effective_destination > route->escort_requirement)
        {
            double dest_surplus = static_cast<double>(effective_destination - route->escort_requirement);
            risk -= dest_surplus * 0.006;
        }
        int weakest_escort = effective_origin;
        if (effective_destination < weakest_escort)
            weakest_escort = effective_destination;
        if (campaign_focus > 0.0)
        {
            int escort_gap = route->escort_requirement - weakest_escort;
            if (escort_gap < 0)
                escort_gap = 0;
            if (escort_gap > 0)
            {
                double focus_multiplier = 1.0 + campaign_focus * (0.9 + static_cast<double>(escort_gap) * 0.18);
                if (focus_multiplier > 4.0)
                    focus_multiplier = 4.0;
                risk *= focus_multiplier;
            }
            else if (campaign_focus > 0.75 && weakest_escort == route->escort_requirement)
            {
                double edge_bias = 1.0 + (campaign_focus - 0.75) * 0.4;
                risk *= edge_bias;
            }
        }
    }
    if (origin_under_attack)
        risk *= 1.8;
    if (destination_under_attack)
        risk *= 1.6;
    if (convoy.raided)
        risk *= 0.35;
    double origin_modifier = this->_buildings.get_planet_convoy_raid_risk_modifier(convoy.origin_planet_id);
    double destination_modifier = this->_buildings.get_planet_convoy_raid_risk_modifier(convoy.destination_planet_id);
    double combined_modifier = origin_modifier + destination_modifier;
    if (combined_modifier > 0.75)
        combined_modifier = 0.75;
    if (combined_modifier > 0.0)
    {
        double multiplier = 1.0 - combined_modifier;
        if (multiplier < 0.25)
            multiplier = 0.25;
        risk *= multiplier;
    }
    if (risk < 0.002)
        risk = 0.002;
    return risk;
}

void Game::handle_convoy_raid(ft_supply_convoy &convoy, bool origin_under_attack, bool destination_under_attack)
