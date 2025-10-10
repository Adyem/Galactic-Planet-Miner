                contract.elapsed_seconds = 0.0;
                break;
            }
            if (shipment > available)
                shipment = available;
            ft_supply_route *route = this->ensure_supply_route(contract.origin_planet_id, contract.destination_planet_id);
            if (!route)
            {
                contract.elapsed_seconds = contract.interval_seconds;
                break;
            }
            origin->sub_resource(contract.resource_id, shipment);
            this->send_state(contract.origin_planet_id, contract.resource_id);
            this->ensure_planet_item_slot(contract.destination_planet_id, contract.resource_id);
            int dispatched_amount = this->dispatch_convoy(*route, contract.origin_planet_id,
                                                          contract.destination_planet_id,
                                                          contract.resource_id, shipment,
                                                          contract.id);
            if (dispatched_amount <= 0)
            {
                origin->add_resource(contract.resource_id, shipment);
                this->send_state(contract.origin_planet_id, contract.resource_id);
                contract.elapsed_seconds = contract.interval_seconds;
                break;
            }
            dispatched = true;
            this->increase_contract_pending_delivery(contract.id, dispatched_amount);
            contract.elapsed_seconds -= contract.interval_seconds;
            if (contract.elapsed_seconds < 0.0)
                contract.elapsed_seconds = 0.0;
            if (contract.elapsed_seconds > contract.interval_seconds)
                contract.elapsed_seconds = contract.interval_seconds;
        }
        if (!dispatched && contract.elapsed_seconds > contract.interval_seconds)
            contract.elapsed_seconds = contract.interval_seconds;
    }
}

void Game::advance_convoys(double seconds)
{
    if (seconds <= 0.0)
        return ;
    this->decay_all_route_threat(seconds);
    size_t count = this->_active_convoys.size();
    if (count > 0)
    {
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
    size_t route_count = this->_supply_routes.size();
    if (route_count > 0)
    {
        Pair<RouteKey, ft_supply_route> *routes = this->_supply_routes.end();
        routes -= route_count;
        for (size_t i = 0; i < route_count; ++i)
            this->update_route_escalation(routes[i].value, seconds);
    }
}

void Game::reset_delivery_streak()
{
    this->_current_delivery_streak = 0;
    this->_next_streak_milestone_index = 0;
}

void Game::record_convoy_delivery(const ft_supply_convoy &convoy)
{
    ft_supply_route *route = this->get_route_by_id(convoy.route_id);
    if (route)
    {
        double reduction = convoy.raided ? -0.6 : -0.9;
        this->modify_route_threat(*route, reduction, true);
    }
    this->_convoys_delivered_total += 1;
    this->_current_delivery_streak += 1;
    if (this->_current_delivery_streak > this->_longest_delivery_streak)
    {
        this->_longest_delivery_streak = this->_current_delivery_streak;
        ft_string record_entry("Quartermaster Nia records a new convoy streak of ");
        record_entry.append(ft_to_string(this->_current_delivery_streak));
        record_entry.append(ft_string(" successful deliveries."));
        this->append_lore_entry(record_entry);
    }
    while (this->_next_streak_milestone_index < this->_streak_milestones.size() &&
           this->_current_delivery_streak >= this->_streak_milestones[this->_next_streak_milestone_index])
    {
        int milestone = this->_streak_milestones[this->_next_streak_milestone_index];
        ft_string entry("Logistics crews celebrate ");
        entry.append(ft_to_string(milestone));
        entry.append(ft_string(" convoys arriving uninterrupted."));
        this->append_lore_entry(entry);
        this->_next_streak_milestone_index += 1;
    }
    this->record_achievement_event(ACHIEVEMENT_EVENT_CONVOY_DELIVERED, 1);
    this->record_achievement_event(ACHIEVEMENT_EVENT_CONVOY_STREAK_BEST, this->_longest_delivery_streak);
    if (convoy.escort_fleet_id > 0 && convoy.escort_rating > 0)
    {
        ft_sharedptr<ft_fleet> escort = this->get_fleet(convoy.escort_fleet_id);
        if (escort)
        {
            double xp_gain = 16.0;
            if (route)
                xp_gain += static_cast<double>(route->escort_requirement) * 3.0;
            if (route && route->threat_level > 0.0)
                xp_gain += route->threat_level * 1.5;
            if (convoy.raided)
                xp_gain += 12.0;
            if (xp_gain < 0.0)
                xp_gain = 0.0;
            if (escort->add_escort_veterancy(xp_gain))
            {
                int bonus = escort->get_escort_veterancy_bonus();
                ft_string veterancy_entry("Escort fleet #");
                veterancy_entry.append(ft_to_string(convoy.escort_fleet_id));
                veterancy_entry.append(ft_string(" hones its convoy tactics guarding route "));
                veterancy_entry.append(ft_to_string(convoy.origin_planet_id));
                veterancy_entry.append(ft_string(" -> "));
                veterancy_entry.append(ft_to_string(convoy.destination_planet_id));
                veterancy_entry.append(ft_string(", raising its escort rating bonus to +"));
                veterancy_entry.append(ft_to_string(bonus));
                if (convoy.raided)
                    veterancy_entry.append(ft_string(" after weathering a raid."));
                else
                    veterancy_entry.append(ft_string("."));
                this->append_lore_entry(veterancy_entry);
            }
        }
    }
}

void Game::record_convoy_loss(const ft_supply_convoy &convoy, bool destroyed_by_raid)
{
    ft_supply_route *route = this->get_route_by_id(convoy.route_id);
    if (route)
    {
        double delta = destroyed_by_raid ? 1.2 : 0.6;
        this->modify_route_threat(*route, delta, true);
    }
    bool had_escort = (convoy.escort_fleet_id > 0 && convoy.escort_rating > 0);
    if (destroyed_by_raid)
        this->_convoy_raid_losses += 1;
    if (this->_current_delivery_streak > 0)
    {
        ft_string streak_entry("Quartermaster Nia laments the end of a ");
        streak_entry.append(ft_to_string(this->_current_delivery_streak));
        streak_entry.append(ft_string(" convoy streak."));
        this->append_lore_entry(streak_entry);
    }
    this->reset_delivery_streak();
    if (destroyed_by_raid)
    {
        ft_string raid_entry("Professor Lumen tallies raid losses now at ");
        raid_entry.append(ft_to_string(this->_convoy_raid_losses));
        raid_entry.append(ft_string(" convoys."));
        if (had_escort)
        {
            raid_entry.append(ft_string(" Escort fleet #"));
            raid_entry.append(ft_to_string(convoy.escort_fleet_id));
            raid_entry.append(ft_string(" could not turn the tide."));
        }
        this->append_lore_entry(raid_entry);
    }
    else if (had_escort)
    {
        ft_string escort_entry("Escort fleet #");
        escort_entry.append(ft_to_string(convoy.escort_fleet_id));
        escort_entry.append(ft_string(" returns without its charge."));
        this->append_lore_entry(escort_entry);
    }
    if (had_escort)
    {
        ft_sharedptr<ft_fleet> escort = this->get_fleet(convoy.escort_fleet_id);
        if (escort)
        {
            double penalty = destroyed_by_raid ? 36.0 : 18.0;
            if (!destroyed_by_raid && convoy.raided)
                penalty += 8.0;
            if (penalty < 0.0)
                penalty = 0.0;
            if (escort->decay_escort_veterancy(penalty))
            {
                int bonus = escort->get_escort_veterancy_bonus();
                ft_string veterancy_entry("Escort fleet #");
                veterancy_entry.append(ft_to_string(convoy.escort_fleet_id));
                if (destroyed_by_raid)
                    veterancy_entry.append(ft_string(" loses convoy veterancy after its freighters were wiped out"));
                else
                    veterancy_entry.append(ft_string(" sheds convoy veterancy while regrouping from heavy losses"));
                if (bonus > 0)
                {
                    veterancy_entry.append(ft_string(", retaining a +"));
                    veterancy_entry.append(ft_to_string(bonus));
                    veterancy_entry.append(ft_string(" escort bonus."));
                }
                else
                    veterancy_entry.append(ft_string(", leaving no remaining escort bonus."));
                this->append_lore_entry(veterancy_entry);
            }
        }
    }
}

int Game::create_supply_contract(int origin_planet_id, int destination_planet_id,
                                 int resource_id, int shipment_size,
