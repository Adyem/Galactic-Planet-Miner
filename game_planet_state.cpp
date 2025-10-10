bool Game::update_planet_energy_conservation(int planet_id)
{
    if (planet_id <= 0)
        return this->update_energy_conservation_state(planet_id, false, false, false, false, false);
    if (!this->_emergency_energy_protocol || !this->is_planet_unlocked(planet_id))
        return this->update_energy_conservation_state(planet_id, false, false, false, false, false);

    bool assault_active = this->_combat.is_assault_active(planet_id);
    bool escalation_target = false;
    bool convoy_danger = false;
    bool threat_triggered = false;
    bool active = assault_active;
    double highest_threat = 0.0;

    const double escalation_timer_threshold = 40.0;
    const double threat_activation_threshold = 4.0;

    ft_vector<Pair<RouteKey, ft_supply_route> > route_entries;
    ft_map_snapshot(this->_supply_routes, route_entries);
    for (size_t i = 0; i < route_entries.size(); ++i)
    {
        const ft_supply_route &route = route_entries[i].value;
        if (route.origin_planet_id != planet_id && route.destination_planet_id != planet_id)
            continue;
        if (route.threat_level > highest_threat)
            highest_threat = route.threat_level;
        if (!escalation_target && route.escalation_planet_id == planet_id)
        {
            if (route.escalation_pending)
                escalation_target = true;
            else if (route.escalation_timer >= escalation_timer_threshold)
                escalation_target = true;
        }
    }

    if (!active)
    {
        ft_vector<Pair<int, ft_supply_convoy> > convoy_entries;
        ft_map_snapshot(this->_active_convoys, convoy_entries);
        for (size_t i = 0; i < convoy_entries.size(); ++i)
        {
            const ft_supply_convoy &convoy = convoy_entries[i].value;
            if (convoy.origin_planet_id != planet_id && convoy.destination_planet_id != planet_id)
                continue;
            if (convoy.raided || convoy.destroyed || convoy.raid_meter >= 0.6)
            {
                convoy_danger = true;
                break;
            }
        }
    }

    if (!active)
    {
        if (highest_threat >= threat_activation_threshold)
        {
            active = true;
            threat_triggered = true;
        }
        else if (escalation_target)
            active = true;
        else if (convoy_danger && highest_threat >= 2.5)
            active = true;
    }

    return this->update_energy_conservation_state(planet_id, active,
        assault_active, escalation_target, convoy_danger, threat_triggered);
}

bool Game::update_energy_conservation_state(int planet_id, bool active,
    bool assault_active, bool escalation_target, bool convoy_danger, bool threat_triggered)
{
    Pair<int, bool> *entry = this->_energy_conservation_active.find(planet_id);
    bool previous = (entry != ft_nullptr && entry->value);
    if (active)
    {
        if (entry == ft_nullptr)
            this->_energy_conservation_active.insert(planet_id, true);
        else
            entry->value = true;
    }
    else if (entry != ft_nullptr)
        this->_energy_conservation_active.remove(planet_id);

    if (active != previous)
        this->log_energy_conservation_transition(planet_id, active,
            assault_active, escalation_target, convoy_danger, threat_triggered);
    return active;
}

void Game::log_energy_conservation_transition(int planet_id, bool active,
    bool assault_active, bool escalation_target, bool convoy_danger, bool threat_triggered)
{
    const char *planet_name = get_planet_story_name(planet_id);
    if (planet_name == ft_nullptr)
        planet_name = "Unknown Colony";
    ft_string entry;
    if (active)
    {
        entry = ft_string("Engineer cadres on ");
        entry.append(ft_string(planet_name));
        if (assault_active)
            entry.append(ft_string(" divert factory power into shield grids as raider assault sirens wail."));
        else if (escalation_target)
            entry.append(ft_string(" follow Professor Lumen's warning, spinning down fabrication lines before the strike hits."));
        else if (convoy_danger)
            entry.append(ft_string(" freeze smelters while convoy controllers coordinate with escort wings to break the ambush."));
        else if (threat_triggered)
            entry.append(ft_string(" pivot into emergency conservation after proximity radars spike with raider chatter."));
        else
            entry.append(ft_string(" slip into emergency conservation drills to preserve defensive reserves."));
    }
    else
    {
        entry = ft_string("Operations crews on ");
        entry.append(ft_string(planet_name));
        entry.append(ft_string(" restore full production as emergency drills stand down."));
    }
    this->append_lore_entry(entry);
}

bool Game::is_planet_energy_conservation_active(int planet_id) const noexcept
{
    const Pair<int, bool> *entry = this->_energy_conservation_active.find(planet_id);
    if (entry == ft_nullptr)
        return false;
    return entry->value;
}

int Game::get_planet_escort_rating(int planet_id) const
{
    return this->calculate_planet_escort_rating(planet_id);
}

double Game::get_planet_convoy_raid_risk_modifier(int planet_id) const
{
    return this->_buildings.get_planet_convoy_raid_risk_modifier(planet_id);
}

void Game::ensure_planet_item_slot(int planet_id, int resource_id)
{
    ft_sharedptr<ft_planet> planet = this->get_planet_storage_target(planet_id);
    if (!planet)
        return ;
    planet->ensure_item_slot(resource_id);
}

int Game::add_ore(int planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> planet = this->get_planet_storage_target(planet_id);
    if (!planet)
        return 0;
    int total = planet->add_resource(ore_id, amount);
    this->send_state(planet_id, ore_id);
    return total;
}

int Game::sub_ore(int planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> planet = this->get_planet_storage_target(planet_id);
    if (!planet)
        return 0;
    int total = planet->sub_resource(ore_id, amount);
    this->send_state(planet_id, ore_id);
    return total;
}

int Game::get_ore(int planet_id, int ore_id) const
{
    ft_sharedptr<const ft_planet> planet = this->get_planet_storage_target(planet_id);
    if (!planet)
        return 0;
    return planet->get_resource(ore_id);
}

void Game::set_ore(int planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> planet = this->get_planet_storage_target(planet_id);
    if (!planet)
        return ;
    this->ensure_planet_item_slot(planet_id, ore_id);
    planet->set_resource(ore_id, amount);
    this->send_state(planet_id, ore_id);
}

double Game::get_rate(int planet_id, int ore_id) const
{
    ft_sharedptr<const ft_planet> planet = this->get_planet_storage_target(planet_id);
    if (!planet)
        return 0;
    return planet->get_rate(ore_id);
}

const ft_vector<Pair<int, double> > &Game::get_planet_resources(int planet_id) const
{
    static ft_vector<Pair<int, double> > empty;
    ft_sharedptr<const ft_planet> planet = this->get_planet_storage_target(planet_id);
    if (!planet)
        return empty;
    return planet->get_resources();
}

void Game::get_achievement_ids(ft_vector<int> &out) const
{
    this->_achievements.get_achievement_ids(out);
}

int Game::get_achievement_status(int achievement_id) const
{
    return this->_achievements.get_status(achievement_id);
}

int Game::get_achievement_progress(int achievement_id) const
{
    return this->_achievements.get_progress(achievement_id);
}

int Game::get_achievement_target(int achievement_id) const
{
    return this->_achievements.get_target(achievement_id);
}

bool Game::get_achievement_info(int achievement_id, ft_achievement_info &out) const
{
    return this->_achievements.get_info(achievement_id, out);
}

void Game::apply_planet_snapshot(const ft_map<int, ft_sharedptr<ft_planet> > &snapshot)
{
    ft_vector<Pair<int, ft_sharedptr<ft_planet> > > entries;
    ft_map_snapshot(snapshot, entries);
    size_t count = entries.size();
    if (count == 0)
        return ;
    for (size_t i = 0; i < count; ++i)
    {
        int planet_id = entries[i].key;
        const ft_sharedptr<ft_planet> &saved_planet = entries[i].value;
        if (!saved_planet)
            continue;
        ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
        if (!planet)
        {
            this->unlock_planet(planet_id);
            planet = this->get_planet(planet_id);
        }
        if (!planet)
            continue;
        const ft_vector<Pair<int, double> > &saved_rates = saved_planet->get_resources();
        for (size_t j = 0; j < saved_rates.size(); ++j)
        {
            int ore_id = saved_rates[j].key;
            if (ore_id <= 0)
                continue;
            planet->register_resource(ore_id, saved_rates[j].value);
        }
        const ft_vector<Pair<int, double> > &saved_carryover = saved_planet->get_carryover();
        for (size_t j = 0; j < saved_carryover.size(); ++j)
        {
            int ore_id = saved_carryover[j].key;
            if (ore_id <= 0)
                continue;
            planet->set_carryover(ore_id, saved_carryover[j].value);
        }
        ft_vector<Pair<int, int> > inventory_snapshot = saved_planet->get_items_snapshot();
        ft_set<int> saved_item_ids(inventory_snapshot.size());
        for (size_t j = 0; j < inventory_snapshot.size(); ++j)
            saved_item_ids.insert(inventory_snapshot[j].key);
