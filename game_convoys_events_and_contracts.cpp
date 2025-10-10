    entry.append(ft_string(" before fresh meteor storms close the corridor."));
    this->append_lore_entry(entry);
    this->append_resource_lore_snippet(salvage_resource, convoy.origin_planet_id, convoy.destination_planet_id);
    this->modify_route_threat(*route, 0.35, true);
    this->maybe_unlock_imperium_pressure(*route);
    if (!this->is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_CELESTIAL_BARRENS))
    {
        ft_string journal_text("Journal – Celestial Barrens: Between Mars and Zalthor, shattered meteors hide rare isotopes. Finn's charts keep convoys alive while raiders and Imperium agents chase the same treasure through lethal debris fields.");
        this->unlock_journal_entry(JOURNAL_ENTRY_LORE_CELESTIAL_BARRENS, journal_text);
    }
    return true;
}

bool Game::handle_nebula_outpost_scan(const ft_supply_convoy &convoy)
{
    if (!route_touches_nebula_outpost(convoy.origin_planet_id, convoy.destination_planet_id))
        return false;
    ft_supply_route *route = this->get_route_by_id(convoy.route_id);
    if (!route)
        return false;
    double chance = 0.16;
    if (route->threat_level > 2.0)
    {
        double bonus = (route->threat_level - 2.0) * 0.04;
        if (bonus > 0.12)
            bonus = 0.12;
        chance += bonus;
    }
    if (convoy.escort_rating > 0)
        chance += 0.04;
    if (chance > 0.5)
        chance = 0.5;
    double roll = static_cast<double>(ft_random_float());
    if (roll >= chance)
        return false;
    double threat_reduction = 0.7 + route->threat_level * 0.15;
    if (threat_reduction > 2.0)
        threat_reduction = 2.0;
    this->modify_route_threat(*route, -threat_reduction, true);
    this->accelerate_contract(convoy.contract_id, 0.1);
    ft_string entry("Outpost Nebula-X sweeps the convoy lane with aurora-lit sensors. Professor Lumen decodes the telemetry, peeling back raider ambushes while liberation cells whisper across the anomaly.");
    this->append_lore_entry(entry);
    if (!this->is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_NEBULA_OUTPOST))
    {
        ft_string journal_text("Journal – Outpost Nebula-X: The frontier research bastion watches over Noctaris Prime, sharing anomaly scans that warn of raids and Imperium probes alike. Its signals are the lifeline for every convoy brave enough to cross the nebula.");
        this->unlock_journal_entry(JOURNAL_ENTRY_LORE_NEBULA_OUTPOST, journal_text);
    }
    return true;
}

void Game::update_route_escalation(ft_supply_route &route, double seconds)
{
    if (seconds <= 0.0)
        return ;
    bool origin_assault = this->_combat.is_assault_active(route.origin_planet_id);
    bool destination_assault = this->_combat.is_assault_active(route.destination_planet_id);
    if (origin_assault && destination_assault)
    {
        route.escalation_timer = 0.0;
        route.escalation_pending = false;
        route.escalation_planet_id = 0;
        return ;
    }
    bool origin_available = this->is_planet_unlocked(route.origin_planet_id) && !origin_assault;
    bool destination_available = this->is_planet_unlocked(route.destination_planet_id) && !destination_assault;
    if (destination_available)
        route.escalation_planet_id = route.destination_planet_id;
    else if (origin_available)
        route.escalation_planet_id = route.origin_planet_id;
    else if (!route.escalation_pending)
        route.escalation_planet_id = 0;
    bool threat_high = (route.threat_level >= ROUTE_ESCALATION_THRESHOLD);
    if (threat_high && route.escalation_planet_id != 0)
    {
        route.escalation_timer += seconds;
        if (route.escalation_timer > ROUTE_ESCALATION_TRIGGER_TIME)
            route.escalation_timer = ROUTE_ESCALATION_TRIGGER_TIME;
        if (!route.escalation_pending && route.escalation_timer >= ROUTE_ESCALATION_TRIGGER_TIME)
            route.escalation_pending = true;
    }
    else
    {
        if (route.escalation_timer > 0.0)
        {
            route.escalation_timer -= seconds * ROUTE_ESCALATION_DECAY_RATE;
            if (route.escalation_timer < 0.0)
                route.escalation_timer = 0.0;
        }
        if (route.escalation_timer <= 0.0)
        {
            route.escalation_pending = false;
            if (!threat_high)
                route.escalation_planet_id = 0;
        }
    }
    if (route.escalation_pending)
        this->trigger_route_assault(route);
}

void Game::trigger_route_assault(ft_supply_route &route)
{
    if (!route.escalation_pending)
        return ;
    int planet_id = route.escalation_planet_id;
    if (planet_id == 0)
        return ;
    if (!this->is_planet_unlocked(planet_id))
        return ;
    if (this->_combat.is_assault_active(planet_id))
        return ;
    double difficulty = 1.0 + (route.threat_level * 0.12);
    if (difficulty < 0.8)
        difficulty = 0.8;
    if (!this->start_raider_assault(planet_id, difficulty))
        return ;
    ft_string entry("Sustained raids along convoy route ");
    entry.append(ft_to_string(route.origin_planet_id));
    entry.append(ft_string(" -> "));
    entry.append(ft_to_string(route.destination_planet_id));
    entry.append(ft_string(" escalate into a direct assault on planet "));
    entry.append(ft_to_string(planet_id));
    entry.append(ft_string("."));
    this->append_lore_entry(entry);
    double relief = route.threat_level - (ROUTE_ESCALATION_THRESHOLD - 1.0);
    if (relief < 2.5)
        relief = 2.5;
    if (relief > route.threat_level)
        relief = route.threat_level;
    this->modify_route_threat(route, -relief, true);
    route.escalation_timer = 0.0;
    route.escalation_pending = false;
    route.escalation_planet_id = 0;
}

void Game::trigger_branch_assault(int planet_id, double difficulty, bool order_branch)
{
    if (planet_id == 0)
        return ;
    if (!this->is_planet_unlocked(planet_id))
        return ;
    if (this->_combat.is_assault_active(planet_id))
        return ;
    if (!this->start_raider_assault(planet_id, difficulty))
        return ;
    if (order_branch)
        this->_order_branch_pending_assault = planet_id;
    else
        this->_rebellion_branch_pending_assault = planet_id;
    ft_string entry;
    if (order_branch)
        entry = ft_string("Marshal Rhea orders a purge strike on planet ");
    else
        entry = ft_string("Captain Blackthorne rallies the liberation strike on planet ");
    entry.append(ft_to_string(planet_id));
    entry.append(ft_string("."));
    this->append_lore_entry(entry);
}

int Game::count_active_convoys_for_contract(int contract_id) const
{
    if (contract_id <= 0)
        return 0;
    size_t count = this->_active_convoys.size();
    if (count == 0)
        return 0;
    const Pair<int, ft_supply_convoy> *entries = this->_active_convoys.end();
    entries -= count;
    int active = 0;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_supply_convoy &convoy = entries[i].value;
        if (convoy.contract_id == contract_id)
            active += 1;
    }
    return active;
}

bool Game::has_active_convoy_for_contract(int contract_id) const
{
    if (contract_id <= 0)
        return false;
    const Pair<int, ft_supply_contract> *contract_entry = this->_supply_contracts.find(contract_id);
    if (contract_entry == ft_nullptr)
        return false;
    int limit = contract_entry->value.max_active_convoys;
    if (limit < 1)
        limit = 1;
    int active = this->count_active_convoys_for_contract(contract_id);
    return active >= limit;
}

void Game::increase_contract_pending_delivery(int contract_id, int amount)
{
    if (contract_id <= 0 || amount <= 0)
        return ;
    Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract_id);
    if (entry == ft_nullptr)
        return ;
    ft_supply_contract &contract = entry->value;
    if (contract.pending_delivery < 0)
        contract.pending_delivery = 0;
    contract.pending_delivery += amount;
}

void Game::decrease_contract_pending_delivery(int contract_id, int amount)
{
    if (contract_id <= 0 || amount <= 0)
        return ;
    Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract_id);
    if (entry == ft_nullptr)
        return ;
    ft_supply_contract &contract = entry->value;
    contract.pending_delivery -= amount;
    if (contract.pending_delivery < 0)
        contract.pending_delivery = 0;
}

void Game::accelerate_contract(int contract_id, double fraction)
{
    if (contract_id <= 0 || fraction <= 0.0)
        return ;
    Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract_id);
    if (entry == ft_nullptr)
        return ;
    ft_supply_contract &contract = entry->value;
    if (contract.interval_seconds <= 0.0)
        return ;
    double bonus = contract.interval_seconds * fraction;
    if (bonus < 0.0)
        bonus = 0.0;
    contract.elapsed_seconds += bonus;
    if (contract.elapsed_seconds > contract.interval_seconds)
        contract.elapsed_seconds = contract.interval_seconds;
    if (contract.elapsed_seconds < 0.0)
        contract.elapsed_seconds = 0.0;
}

bool Game::assign_convoy_escort(int origin_planet_id, int destination_planet_id, int fleet_id)
{
    if (origin_planet_id == destination_planet_id)
        return false;
    if (fleet_id <= 0)
        return false;
    ft_supply_route *route = this->ensure_supply_route(origin_planet_id, destination_planet_id);
    if (!route)
        return false;
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return false;
    if (this->is_fleet_escorting_convoy(fleet_id))
