    return fleet->get_escort_veterancy();
}

int Game::get_fleet_escort_veterancy_bonus(int fleet_id) const
{
    if (fleet_id <= 0)
        return 0;
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->get_escort_veterancy_bonus();
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
    int dispatched = this->dispatch_convoy(*route, from_planet_id, to_planet_id,
                                          ore_id, amount, 0);
    if (dispatched <= 0)
    {
        from->add_resource(ore_id, amount);
        this->send_state(from_planet_id, ore_id);
        return 0;
    }
    return dispatched;
}

