int Game::count_capital_ships_in_collection(const ft_map<int, ft_sharedptr<ft_fleet> > &collection) const
{
    size_t count = collection.size();
    if (count == 0)
        return 0;
    const Pair<int, ft_sharedptr<ft_fleet> > *entries = collection.end();
    entries -= count;
    int total = 0;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_sharedptr<ft_fleet> &fleet = entries[i].value;
        if (!fleet)
            continue;
        ft_vector<int> ship_ids;
        fleet->get_ship_ids(ship_ids);
        for (size_t j = 0; j < ship_ids.size(); ++j)
        {
            if (is_capital_ship_type(fleet->get_ship_type(ship_ids[j])))
                total += 1;
        }
    }
    return total;
}

int Game::count_capital_ships() const
{
    int total = this->count_capital_ships_in_collection(this->_fleets);
    total += this->count_capital_ships_in_collection(this->_planet_fleets);
    return total;
}

bool Game::is_ship_type_available(int ship_type) const
{
    switch (ship_type)
    {
    case SHIP_SHIELD:
    case SHIP_RADAR:
    case SHIP_SALVAGE:
        return true;
    case SHIP_TRANSPORT:
        if (!this->_research.is_completed(RESEARCH_CRAFTING_MASTERY))
            return false;
        return true;
    case SHIP_CORVETTE:
        if (!this->_research.is_completed(RESEARCH_ARMAMENT_ENHANCEMENT_I))
            return false;
        return true;
    case SHIP_INTERCEPTOR:
        if (!this->_research.is_completed(RESEARCH_ARMAMENT_ENHANCEMENT_II))
            return false;
        return true;
    case SHIP_REPAIR_DRONE:
        if (!this->_repair_drones_unlocked)
            return false;
        return true;
    case SHIP_SUNFLARE_SLOOP:
        if (!this->_shield_support_unlocked)
            return false;
        return true;
    case SHIP_FRIGATE_JUGGERNAUT:
    case SHIP_FRIGATE_CARRIER:
    case SHIP_FRIGATE_SOVEREIGN:
    case SHIP_FRIGATE_PREEMPTOR:
    case SHIP_FRIGATE_PROTECTOR:
    case SHIP_FRIGATE_ECLIPSE:
        if (!this->_research.is_completed(RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT))
            return false;
        return true;
    case SHIP_CAPITAL_NOVA:
    case SHIP_CAPITAL_OBSIDIAN:
    case SHIP_CAPITAL_PREEMPTOR:
    case SHIP_CAPITAL_PROTECTOR:
    case SHIP_CAPITAL_ECLIPSE:
        if (!this->_research.is_completed(RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT))
            return false;
        if (this->_capital_ship_limit <= 0)
            return false;
        return true;
    case SHIP_CAPITAL_JUGGERNAUT:
        if (this->_capital_ship_limit <= 0)
            return false;
        return true;
    case SHIP_RAIDER_CORVETTE:
    case SHIP_RAIDER_DESTROYER:
    case SHIP_RAIDER_BATTLESHIP:
        return false;
    default:
        return true;
    }
}

void Game::clear_escape_pod_records(const ft_fleet &fleet)
{
    ft_vector<int> ship_ids;
    fleet.get_ship_ids(ship_ids);
    for (size_t i = 0; i < ship_ids.size(); ++i)
        this->_escape_pod_rescued.remove(ship_ids[i]);
}

void Game::create_fleet(int fleet_id)
{
    if (this->_fleets.find(fleet_id) != ft_nullptr)
        return ;
    ft_sharedptr<ft_fleet> fleet(new ft_fleet(fleet_id));
    fleet->set_location_planet(PLANET_TERRA);
    this->_state.add_character(fleet);
    this->_fleets.insert(fleet_id, fleet);
}

void Game::remove_fleet(int fleet_id, int target_fleet_id, int target_planet_id)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    bool transferred = false;
    if (target_fleet_id >= 0)
    {
        ft_sharedptr<ft_fleet> target = this->get_fleet(target_fleet_id);
        if (target)
        {
            fleet->move_ships_to(*target);
            transferred = true;
        }
    }
    else if (target_planet_id >= 0)
    {
        ft_sharedptr<ft_fleet> target = this->get_planet_fleet(target_planet_id);
        if (target)
        {
            fleet->move_ships_to(*target);
            transferred = true;
        }
    }
    if (!transferred)
        this->clear_escape_pod_records(*fleet);
    this->_fleets.remove(fleet_id);
}

int Game::create_ship(int fleet_id, int ship_type)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    if (!this->is_ship_type_available(ship_type))
        return 0;
    if (is_capital_ship_type(ship_type))
    {
        if (this->_capital_ship_limit <= 0)
            return 0;
        if (this->count_capital_ships() >= this->_capital_ship_limit)
            return 0;
    }
    int uid = fleet->create_ship(ship_type);
    if (uid != 0)
        this->_escape_pod_rescued.remove(uid);
    return uid;
}

void Game::remove_ship(int fleet_id, int ship_uid)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    this->_escape_pod_rescued.remove(ship_uid);
    fleet->remove_ship(ship_uid);
}

bool Game::transfer_ship(int from_fleet_id, int to_fleet_id, int ship_uid)
{
    ft_sharedptr<ft_fleet> from = this->get_fleet(from_fleet_id);
    if (!from)
        return false;
    ft_sharedptr<ft_fleet> to = this->get_fleet(to_fleet_id);
    if (!to)
        return false;
    return from->move_ship_to(*to, ship_uid);
}
