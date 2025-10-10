                                 double interval_seconds,
                                 int minimum_destination_stock,
                                 int max_active_convoys)
{
    if (origin_planet_id == destination_planet_id)
        return 0;
    if (shipment_size <= 0)
        return 0;
    if (interval_seconds <= 0.0)
        return 0;
    ft_sharedptr<ft_planet> origin = this->get_planet(origin_planet_id);
    ft_sharedptr<ft_planet> destination = this->get_planet(destination_planet_id);
    if (!origin || !destination)
        return 0;
    ft_supply_route *route = this->ensure_supply_route(origin_planet_id, destination_planet_id);
    if (!route)
        return 0;
    this->ensure_planet_item_slot(destination_planet_id, resource_id);
    ft_supply_contract contract;
    contract.id = this->_next_contract_id++;
    contract.origin_planet_id = origin_planet_id;
    contract.destination_planet_id = destination_planet_id;
    contract.resource_id = resource_id;
    contract.shipment_size = shipment_size;
    contract.interval_seconds = interval_seconds;
    contract.elapsed_seconds = 0.0;
    if (max_active_convoys < 1)
        max_active_convoys = 1;
    contract.max_active_convoys = max_active_convoys;
    contract.pending_delivery = 0;
    if (minimum_destination_stock >= 0)
    {
        contract.has_minimum_stock = true;
        contract.minimum_stock = minimum_destination_stock;
    }
    else
    {
        contract.has_minimum_stock = false;
        contract.minimum_stock = 0;
    }
    this->_supply_contracts.insert(contract.id, contract);
    Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract.id);
    if (entry == ft_nullptr)
        return 0;
    return contract.id;
}

bool Game::cancel_supply_contract(int contract_id)
{
    Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract_id);
    if (entry == ft_nullptr)
        return false;
    this->_supply_contracts.remove(contract_id);
    return true;
}

bool Game::update_supply_contract(int contract_id, int shipment_size,
                                  double interval_seconds,
                                  int minimum_destination_stock,
                                  int max_active_convoys)
{
    if (shipment_size <= 0)
        return false;
    if (interval_seconds <= 0.0)
        return false;
    Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract_id);
    if (entry == ft_nullptr)
        return false;
    ft_supply_contract &contract = entry->value;
    contract.shipment_size = shipment_size;
    contract.interval_seconds = interval_seconds;
    if (minimum_destination_stock >= 0)
    {
        contract.has_minimum_stock = true;
        contract.minimum_stock = minimum_destination_stock;
    }
    else
    {
        contract.has_minimum_stock = false;
        contract.minimum_stock = 0;
    }
    if (max_active_convoys >= 0)
    {
        if (max_active_convoys < 1)
            max_active_convoys = 1;
        contract.max_active_convoys = max_active_convoys;
    }
    if (contract.max_active_convoys < 1)
        contract.max_active_convoys = 1;
    if (contract.elapsed_seconds > contract.interval_seconds)
        contract.elapsed_seconds = contract.interval_seconds;
    if (contract.elapsed_seconds < 0.0)
        contract.elapsed_seconds = 0.0;
    return true;
}

void Game::get_supply_contract_ids(ft_vector<int> &out) const
{
    out.clear();
    size_t count = this->_supply_contracts.size();
    if (count == 0)
        return ;
    const Pair<int, ft_supply_contract> *entries = this->_supply_contracts.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
        out.push_back(entries[i].key);
}

bool Game::get_supply_contract(int contract_id, ft_supply_contract &out) const
{
    const Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract_id);
    if (entry == ft_nullptr)
        return false;
    out = entry->value;
    return true;
}

void Game::get_resource_dashboard(ft_resource_dashboard &out) const
{
    out.resources.clear();
    out.routes.clear();
    out.total_active_convoys = static_cast<int>(this->_active_convoys.size());
    out.average_route_threat = 0.0;
    out.maximum_route_threat = 0.0;

    struct ResourceAggregate
    {
        int amount;
        double rate;
        ResourceAggregate() : amount(0), rate(0.0) {}
    };

    ft_map<int, ResourceAggregate> aggregates;
    ft_vector<Pair<int, ft_sharedptr<ft_planet> > > planet_entries;
    ft_map_snapshot(this->_planets, planet_entries);
    for (size_t i = 0; i < planet_entries.size(); ++i)
    {
        int planet_id = planet_entries[i].key;
        const ft_sharedptr<ft_planet> &planet = planet_entries[i].value;
        if (!planet)
            continue;
        double mine_multiplier = this->_buildings.get_mine_multiplier(planet_id);
        if (mine_multiplier < 0.0)
            mine_multiplier = 0.0;
        const ft_vector<Pair<int, double> > &rates = planet->get_resources();
        for (size_t j = 0; j < rates.size(); ++j)
        {
            int resource_id = rates[j].key;
            double adjusted_rate = rates[j].value * this->_resource_multiplier * mine_multiplier;
            int amount = planet->get_resource(resource_id);
            Pair<int, ResourceAggregate> *entry = aggregates.find(resource_id);
            if (entry == ft_nullptr)
            {
                ResourceAggregate aggregate;
                aggregate.amount = amount;
                aggregate.rate = adjusted_rate;
                aggregates.insert(resource_id, aggregate);
            }
            else
            {
                entry->value.amount += amount;
                entry->value.rate += adjusted_rate;
            }
        }
    }

    size_t aggregate_count = aggregates.size();
    if (aggregate_count > 0)
    {
        Pair<int, ResourceAggregate> *entries = aggregates.end();
        entries -= aggregate_count;
        out.resources.reserve(aggregate_count);
        for (size_t i = 0; i < aggregate_count; ++i)
        {
            ft_resource_dashboard_entry summary;
            summary.resource_id = entries[i].key;
            summary.total_stock = entries[i].value.amount;
            summary.production_rate = entries[i].value.rate;
            out.resources.push_back(summary);
        }
    }

    ft_map<int, int> active_convoys_per_route;
    size_t active_count = this->_active_convoys.size();
    if (active_count > 0)
    {
        const Pair<int, ft_supply_convoy> *convoys = this->_active_convoys.end();
        convoys -= active_count;
        for (size_t i = 0; i < active_count; ++i)
        {
            int route_id = convoys[i].value.route_id;
            if (route_id == 0)
                continue;
            Pair<int, int> *entry = active_convoys_per_route.find(route_id);
            if (entry == ft_nullptr)
                active_convoys_per_route.insert(route_id, 1);
            else
                entry->value += 1;
        }
    }

    double total_threat = 0.0;
    double max_threat = 0.0;
    ft_vector<Pair<RouteKey, ft_supply_route> > route_entries;
    ft_map_snapshot(this->_supply_routes, route_entries);
    size_t route_count = route_entries.size();
    if (route_count > 0)
    {
        out.routes.reserve(route_count);
        for (size_t i = 0; i < route_count; ++i)
        {
            const ft_supply_route &route = route_entries[i].value;
            ft_route_dashboard_entry entry;
            entry.route_id = route.id;
            entry.origin_planet_id = route.origin_planet_id;
            entry.destination_planet_id = route.destination_planet_id;
            entry.threat_level = route.threat_level;
            entry.quiet_timer = route.quiet_timer;
            entry.escalation_pending = route.escalation_pending;
            Pair<int, int> *active_entry = active_convoys_per_route.find(route.id);
            if (active_entry != ft_nullptr)
                entry.active_convoys = active_entry->value;
            else
                entry.active_convoys = 0;
            total_threat += route.threat_level;
            if (route.threat_level > max_threat)
                max_threat = route.threat_level;
            out.routes.push_back(entry);
        }
    }

    if (route_count > 0)
        out.average_route_threat = total_threat / static_cast<double>(route_count);
    else
        out.average_route_threat = 0.0;
    out.maximum_route_threat = max_threat;
}

int Game::get_active_convoy_count() const
{
    return static_cast<int>(this->_active_convoys.size());
}

double Game::get_fleet_escort_veterancy(int fleet_id) const
{
    if (fleet_id <= 0)
        return 0.0;
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0.0;
