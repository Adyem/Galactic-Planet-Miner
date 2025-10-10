{
    ft_supply_route *route = this->get_route_by_id(convoy.route_id);
    if (!route)
        return ;
    this->modify_route_threat(*route, 0.45, true);
    double severity = 0.45;
    int effective_origin = convoy.origin_escort + convoy.escort_rating;
    if (effective_origin > 48)
        effective_origin = 48;
    int effective_destination = convoy.destination_escort + convoy.escort_rating;
    if (effective_destination > 48)
        effective_destination = 48;
    if (effective_origin < route->escort_requirement)
        severity += 0.25;
    if (effective_destination < route->escort_requirement)
        severity += 0.1;
    if (origin_under_attack || destination_under_attack)
    {
        severity += 0.3;
        if (effective_origin < route->escort_requirement)
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
    if (lost > 0)
        this->decrease_contract_pending_delivery(convoy.contract_id, lost);
    convoy.amount -= lost;
    convoy.raided = true;
    convoy.raid_meter = 0.0;
    if (convoy.amount <= 0)
    {
        convoy.amount = 0;
        convoy.destroyed = true;
        convoy.remaining_time = 0.0;
        if (!convoy.loss_recorded)
        {
            this->record_convoy_loss(convoy, true);
            convoy.loss_recorded = true;
        }
    }
    ft_string entry("Raiders ambushed a convoy from ");
    entry.append(ft_to_string(convoy.origin_planet_id));
    entry.append(ft_string(" to "));
    entry.append(ft_to_string(convoy.destination_planet_id));
    entry.append(ft_string("; "));
    entry.append(ft_to_string(lost));
    entry.append(ft_string(" units were lost"));
    if (convoy.destroyed)
    {
        if (convoy.escort_rating > 0 && convoy.escort_fleet_id > 0)
        {
            entry.append(ft_string(", and escort fleet #"));
            entry.append(ft_to_string(convoy.escort_fleet_id));
            entry.append(ft_string(" was overwhelmed."));
        }
        else
            entry.append(ft_string(", and without escorts the freighters were wiped out."));
    }
    else if (convoy.escort_rating > 0 && convoy.escort_fleet_id > 0)
    {
        entry.append(ft_string(", but escort fleet #"));
        entry.append(ft_to_string(convoy.escort_fleet_id));
        entry.append(ft_string(" drove the raiders off."));
    }
    else
        entry.append(ft_string(", and the defenseless freighters limped onward."));
    this->append_lore_entry(entry);
    this->append_raider_lore_snippet(convoy.origin_planet_id, convoy.destination_planet_id);
    this->accelerate_contract(convoy.contract_id, 0.5);
}

void Game::finalize_convoy(ft_supply_convoy &convoy)
{
    if (convoy.amount > 0)
        this->decrease_contract_pending_delivery(convoy.contract_id, convoy.amount);
    if (!convoy.destroyed && convoy.amount > 0)
    {
        this->record_convoy_delivery(convoy);
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
        double origin_speed_bonus = this->_buildings.get_planet_convoy_speed_bonus(convoy.origin_planet_id);
        double destination_speed_bonus = this->_buildings.get_planet_convoy_speed_bonus(convoy.destination_planet_id);
        double origin_risk_modifier = this->_buildings.get_planet_convoy_raid_risk_modifier(convoy.origin_planet_id);
        double destination_risk_modifier = this->_buildings.get_planet_convoy_raid_risk_modifier(convoy.destination_planet_id);
        bool has_speed_bonus = (origin_speed_bonus > 0.0 || destination_speed_bonus > 0.0);
        bool has_risk_bonus = (origin_risk_modifier > 0.0 || destination_risk_modifier > 0.0);
        if (has_speed_bonus || has_risk_bonus)
        {
            entry.append(ft_string(" Trade relays "));
            if (has_speed_bonus && has_risk_bonus)
                entry.append(ft_string("shortened the journey and kept raiders at bay."));
            else if (has_speed_bonus)
                entry.append(ft_string("shortened the journey."));
            else
                entry.append(ft_string("kept raiders at bay."));
        }
        this->append_lore_entry(entry);
        this->append_resource_lore_snippet(convoy.resource_id, convoy.origin_planet_id, convoy.destination_planet_id);
        this->handle_celestial_barrens_salvage(convoy);
        this->handle_nebula_outpost_scan(convoy);
    }
    else
    {
        if (!convoy.loss_recorded)
        {
            this->record_convoy_loss(convoy, convoy.raided);
            convoy.loss_recorded = true;
        }
        ft_string entry("A convoy from ");
        entry.append(ft_to_string(convoy.origin_planet_id));
        entry.append(ft_string(" to "));
        entry.append(ft_to_string(convoy.destination_planet_id));
        entry.append(ft_string(" failed to arrive."));
        this->append_lore_entry(entry);
    }
    if (convoy.escort_fleet_id > 0)
    {
        ft_sharedptr<ft_fleet> escort = this->get_fleet(convoy.escort_fleet_id);
        if (escort)
        {
            if (convoy.destroyed)
                escort->set_location_planet(convoy.origin_planet_id);
            else
                escort->set_location_planet(convoy.destination_planet_id);
        }
    }
    bool restore_route_escort = false;
    if (convoy.route_escort_claimed && convoy.escort_fleet_id > 0 && !convoy.destroyed)
    {
        const ft_supply_route *route = this->get_route_by_id(convoy.route_id);
        if (route != ft_nullptr)
        {
            ft_sharedptr<ft_fleet> escort = this->get_fleet(convoy.escort_fleet_id);
            if (escort && escort->has_operational_ships())
            {
                ft_location escort_location = escort->get_location();
                if (escort_location.type == LOCATION_PLANET &&
                    (escort_location.from == convoy.origin_planet_id ||
                     escort_location.from == convoy.destination_planet_id))
                {
                    restore_route_escort = true;
                }
            }
        }
    }
    if (restore_route_escort)
    {
        Pair<int, int> *existing = this->_route_convoy_escorts.find(convoy.route_id);
        if (existing != ft_nullptr)
            existing->value = convoy.escort_fleet_id;
        else
            this->_route_convoy_escorts.insert(convoy.route_id, convoy.escort_fleet_id);
    }
    this->handle_contract_completion(convoy);
    convoy.escort_fleet_id = 0;
    convoy.escort_rating = 0;
}

void Game::handle_contract_completion(const ft_supply_convoy &convoy)
{
    if (convoy.contract_id <= 0)
        return ;
    if (convoy.destroyed)
    {
        this->accelerate_contract(convoy.contract_id, 1.0);
        return ;
    }
    if (convoy.raided)
        this->accelerate_contract(convoy.contract_id, 0.25);
}

void Game::process_supply_contracts(double seconds)
{
    if (seconds <= 0.0)
        return ;
    size_t contract_count = this->_supply_contracts.size();
    if (contract_count == 0)
        return ;
    Pair<int, ft_supply_contract> *entries = this->_supply_contracts.end();
    entries -= contract_count;
    for (size_t i = 0; i < contract_count; ++i)
    {
        ft_supply_contract &contract = entries[i].value;
        if (contract.interval_seconds <= 0.0)
            continue;
        if (contract.max_active_convoys < 1)
            contract.max_active_convoys = 1;
        contract.elapsed_seconds += seconds;
        if (contract.elapsed_seconds < 0.0)
            contract.elapsed_seconds = 0.0;
        bool dispatched = false;
        while (contract.elapsed_seconds >= contract.interval_seconds)
        {
            if (this->has_active_convoy_for_contract(contract.id))
            {
                contract.elapsed_seconds = preserve_contract_elapsed(contract.elapsed_seconds,
                                                                      contract.interval_seconds);
                break;
            }
            ft_sharedptr<ft_planet> origin = this->get_planet(contract.origin_planet_id);
            ft_sharedptr<ft_planet> destination = this->get_planet(contract.destination_planet_id);
            if (!origin || !destination)
            {
                contract.elapsed_seconds = contract.interval_seconds;
                break;
            }
            if (contract.has_minimum_stock)
            {
                int destination_stock = destination->get_resource(contract.resource_id);
                int projected_stock = destination_stock;
                if (contract.pending_delivery > 0)
                {
                    projected_stock += contract.pending_delivery;
                    if (projected_stock < destination_stock)
                        projected_stock = contract.minimum_stock;
                }
                if (projected_stock >= contract.minimum_stock)
                {
                    contract.elapsed_seconds = preserve_contract_elapsed(contract.elapsed_seconds,
                                                                          contract.interval_seconds);
                    break;
                }
            }
            int available = origin->get_resource(contract.resource_id);
            if (available <= 0)
            {
                contract.elapsed_seconds = contract.interval_seconds;
                break;
            }
            int shipment = contract.shipment_size;
            if (shipment <= 0)
            {
