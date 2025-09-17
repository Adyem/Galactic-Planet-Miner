#include "game.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Template/pair.hpp"

Game::RouteKey Game::compose_route_key(int origin, int destination) const
{
    return RouteKey(origin, destination);
}

Game::ft_supply_route *Game::ensure_supply_route(int origin, int destination)
{
    if (origin == destination)
        return ft_nullptr;
    RouteKey key = this->compose_route_key(origin, destination);
    Pair<RouteKey, ft_supply_route> *entry = this->_supply_routes.find(key);
    if (entry != ft_nullptr)
        return &entry->value;
    ft_supply_route route;
    route.id = this->_next_route_id++;
    route.origin_planet_id = origin;
    route.destination_planet_id = destination;
    route.base_travel_time = this->estimate_route_travel_time(origin, destination);
    route.escort_requirement = this->estimate_route_escort_requirement(origin, destination);
    route.base_raid_risk = this->estimate_route_raid_risk(origin, destination);
    this->_supply_routes.insert(key, route);
    entry = this->_supply_routes.find(key);
    if (entry == ft_nullptr)
        return ft_nullptr;
    this->_route_lookup.insert(route.id, key);
    return &entry->value;
}

const Game::ft_supply_route *Game::get_route_by_id(int route_id) const
{
    const Pair<int, RouteKey> *lookup = this->_route_lookup.find(route_id);
    if (lookup == ft_nullptr)
        return ft_nullptr;
    const Pair<RouteKey, ft_supply_route> *entry = this->_supply_routes.find(lookup->value);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
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
    double base = 28.0;
    if (involves_terra && involves_luna)
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
    if (involves_noctaris)
        return 6;
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
    if (involves_vulcan)
        risk += 0.006;
    if (involves_noctaris)
        risk += 0.012;
    if (involves_luna)
        risk -= 0.004;
    if (risk < 0.006)
        risk = 0.006;
    return risk;
}

int Game::calculate_planet_escort_rating(int planet_id) const
{
    int rating = 0;
    ft_sharedptr<const ft_fleet> garrison = this->get_planet_fleet(planet_id);
    if (garrison)
        rating += this->calculate_fleet_escort_rating(*garrison);
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
        case SHIP_FRIGATE_ESCORT:
            rating += 6;
            break;
        case SHIP_FRIGATE_SUPPORT:
            rating += 4;
            break;
        case SHIP_CAPITAL:
        case SHIP_CAPITAL_CARRIER:
        case SHIP_CAPITAL_DREADNOUGHT:
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
    if (rating > 48)
        rating = 48;
    return rating;
}

double Game::calculate_convoy_travel_time(const ft_supply_route &route, int origin_escort, int destination_escort) const
{
    double time = route.base_travel_time;
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
    if (route->escort_requirement > 0)
    {
        if (convoy.origin_escort < route->escort_requirement)
        {
            double deficit = static_cast<double>(route->escort_requirement - convoy.origin_escort);
            risk *= 1.6;
            risk += deficit * 0.01;
        }
        else
        {
            double surplus = static_cast<double>(convoy.origin_escort - route->escort_requirement);
            double divisor = 1.0 + surplus * 0.12;
            if (divisor > 1.0)
                risk /= divisor;
        }
        if (convoy.destination_escort > route->escort_requirement)
        {
            double dest_surplus = static_cast<double>(convoy.destination_escort - route->escort_requirement);
            risk -= dest_surplus * 0.006;
        }
    }
    if (origin_under_attack)
        risk *= 1.8;
    if (destination_under_attack)
        risk *= 1.6;
    if (convoy.raided)
        risk *= 0.35;
    if (risk < 0.002)
        risk = 0.002;
    return risk;
}

void Game::handle_convoy_raid(ft_supply_convoy &convoy, bool origin_under_attack, bool destination_under_attack)
{
    const ft_supply_route *route = this->get_route_by_id(convoy.route_id);
    if (!route)
        return ;
    double severity = 0.45;
    if (convoy.origin_escort < route->escort_requirement)
        severity += 0.25;
    if (convoy.destination_escort < route->escort_requirement)
        severity += 0.1;
    if (origin_under_attack || destination_under_attack)
    {
        severity += 0.3;
        if (convoy.origin_escort < route->escort_requirement)
            severity = 1.0;
    }
    if (severity > 1.0)
        severity = 1.0;
    if (severity < 0.15)
        severity = 0.15;
    int lost = static_cast<int>(static_cast<double>(convoy.amount) * severity);
    if (lost <= 0 && convoy.amount > 0)
        lost = 1;
    if (lost > convoy.amount)
        lost = convoy.amount;
    convoy.amount -= lost;
    convoy.raided = true;
    convoy.raid_meter = 0.0;
    if (convoy.amount <= 0)
    {
        convoy.amount = 0;
        convoy.destroyed = true;
        convoy.remaining_time = 0.0;
    }
    ft_string entry("Raiders ambushed a convoy from ");
    entry.append(ft_to_string(convoy.origin_planet_id));
    entry.append(ft_string(" to "));
    entry.append(ft_to_string(convoy.destination_planet_id));
    entry.append(ft_string("; "));
    entry.append(ft_to_string(lost));
    entry.append(ft_string(" units were lost"));
    if (convoy.destroyed)
        entry.append(ft_string(", and the shipment was wiped out."));
    else
        entry.append(ft_string(", but escorts salvaged part of the cargo."));
    this->_lore_log.push_back(entry);
}

void Game::finalize_convoy(const ft_supply_convoy &convoy)
{
    if (!convoy.destroyed && convoy.amount > 0)
    {
        ft_sharedptr<ft_planet> destination = this->get_planet(convoy.destination_planet_id);
        if (destination)
        {
            destination->add_resource(convoy.resource_id, convoy.amount);
            this->send_state(convoy.destination_planet_id, convoy.resource_id);
        }
        ft_string entry("A convoy from ");
        entry.append(ft_to_string(convoy.origin_planet_id));
        entry.append(ft_string(" delivered "));
        entry.append(ft_to_string(convoy.amount));
        entry.append(ft_string(" units to "));
        entry.append(ft_to_string(convoy.destination_planet_id));
        entry.append(ft_string("."));
        this->_lore_log.push_back(entry);
    }
    else
    {
        ft_string entry("A convoy from ");
        entry.append(ft_to_string(convoy.origin_planet_id));
        entry.append(ft_string(" to "));
        entry.append(ft_to_string(convoy.destination_planet_id));
        entry.append(ft_string(" failed to arrive."));
        this->_lore_log.push_back(entry);
    }
}

void Game::advance_convoys(double seconds)
{
    if (seconds <= 0.0)
        return ;
    size_t count = this->_active_convoys.size();
    if (count == 0)
        return ;
    Pair<int, ft_supply_convoy> *entries = this->_active_convoys.end();
    entries -= count;
    ft_vector<int> completed;
    for (size_t i = 0; i < count; ++i)
    {
        ft_supply_convoy &convoy = entries[i].value;
        if (convoy.remaining_time <= 0.0)
        {
            completed.push_back(entries[i].key);
            continue;
        }
        bool origin_under_attack = this->_combat.is_assault_active(convoy.origin_planet_id);
        bool destination_under_attack = this->_combat.is_assault_active(convoy.destination_planet_id);
        if (!convoy.destroyed)
        {
            double risk = this->calculate_convoy_raid_risk(convoy, origin_under_attack, destination_under_attack);
            convoy.raid_meter += risk * seconds;
            if (convoy.raid_meter >= 1.0)
            {
                while (convoy.raid_meter >= 1.0 && !convoy.destroyed)
                {
                    double remainder = convoy.raid_meter - 1.0;
                    if (remainder < 0.0)
                        remainder = 0.0;
                    this->handle_convoy_raid(convoy, origin_under_attack, destination_under_attack);
                    if (convoy.destroyed)
                    {
                        completed.push_back(entries[i].key);
                        break;
                    }
                    convoy.raid_meter = remainder;
                }
                if (convoy.destroyed)
                    continue;
            }
        }
        convoy.remaining_time -= seconds;
        if (convoy.remaining_time <= 0.0)
            completed.push_back(entries[i].key);
    }
    for (size_t i = 0; i < completed.size(); ++i)
    {
        Pair<int, ft_supply_convoy> *entry = this->_active_convoys.find(completed[i]);
        if (entry == ft_nullptr)
            continue;
        this->finalize_convoy(entry->value);
        this->_active_convoys.remove(completed[i]);
    }
}

int Game::get_active_convoy_count() const
{
    return static_cast<int>(this->_active_convoys.size());
}

int Game::transfer_ore(int from_planet_id, int to_planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> from = this->get_planet(from_planet_id);
    ft_sharedptr<ft_planet> to = this->get_planet(to_planet_id);
    if (!from || !to)
        return 0;
    if (from_planet_id == to_planet_id)
        return 0;
    int available = from->get_resource(ore_id);
    if (amount > available)
        amount = available;
    if (amount <= 0)
        return 0;
    ft_supply_route *route = this->ensure_supply_route(from_planet_id, to_planet_id);
    if (!route)
        return 0;
    from->sub_resource(ore_id, amount);
    this->send_state(from_planet_id, ore_id);
    this->ensure_planet_item_slot(to_planet_id, ore_id);
    ft_supply_convoy convoy;
    convoy.id = this->_next_convoy_id++;
    convoy.route_id = route->id;
    convoy.origin_planet_id = from_planet_id;
    convoy.destination_planet_id = to_planet_id;
    convoy.resource_id = ore_id;
    convoy.amount = amount;
    convoy.origin_escort = this->calculate_planet_escort_rating(from_planet_id);
    convoy.destination_escort = this->calculate_planet_escort_rating(to_planet_id);
    convoy.remaining_time = this->calculate_convoy_travel_time(*route, convoy.origin_escort, convoy.destination_escort);
    this->_active_convoys.insert(convoy.id, convoy);
    ft_string entry("Quartermaster Nia dispatches a convoy from ");
    entry.append(ft_to_string(from_planet_id));
    entry.append(ft_string(" to "));
    entry.append(ft_to_string(to_planet_id));
    entry.append(ft_string(" carrying "));
    entry.append(ft_to_string(amount));
    entry.append(ft_string(" units."));
    this->_lore_log.push_back(entry);
    return amount;
}

