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

Game::ft_supply_route *Game::get_route_by_id(int route_id)
{
    Pair<int, RouteKey> *lookup = this->_route_lookup.find(route_id);
    if (lookup == ft_nullptr)
        return ft_nullptr;
    Pair<RouteKey, ft_supply_route> *entry = this->_supply_routes.find(lookup->value);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
}

double Game::get_supply_route_threat_level(int origin_planet_id, int destination_planet_id) const
{
    const ft_supply_route *route = this->find_supply_route(origin_planet_id, destination_planet_id);
    if (!route)
        return 0.0;
    return route->threat_level;
}

void Game::modify_route_threat(ft_supply_route &route, double delta, bool reset_quiet_timer)
{
    route.threat_level += delta;
    if (route.threat_level < 0.0)
        route.threat_level = 0.0;
    else if (route.threat_level > 10.0)
        route.threat_level = 10.0;
    if (reset_quiet_timer)
    {
        route.quiet_timer = 0.0;
    }
}

void Game::decay_route_threat(ft_supply_route &route, double seconds)
{
    if (seconds <= 0.0)
        return ;
    if (route.quiet_timer < 0.0)
        route.quiet_timer = 0.0;
    route.quiet_timer += seconds;
    if (route.threat_level <= 0.0)
    {
        route.threat_level = 0.0;
        double calm_cap = 300.0;
        if (route.quiet_timer > calm_cap)
            route.quiet_timer = calm_cap;
        return ;
    }
    double decay_delay = 30.0;
    if (route.quiet_timer < decay_delay)
        return ;
    double time_past_delay = route.quiet_timer - decay_delay;
    double effective_seconds = seconds;
    if (time_past_delay < seconds)
        effective_seconds = time_past_delay;
    if (effective_seconds < 0.0)
        effective_seconds = 0.0;
    double decay_rate = 0.02;
    double reduction = effective_seconds * decay_rate;
    if (reduction > route.threat_level)
        reduction = route.threat_level;
    route.threat_level -= reduction;
    if (route.threat_level < 0.0)
        route.threat_level = 0.0;
    double calm_cap = decay_delay + 300.0;
    if (route.quiet_timer > calm_cap)
        route.quiet_timer = calm_cap;
}

void Game::decay_all_route_threat(double seconds)
{
    if (seconds <= 0.0)
        return ;
    ft_vector<Pair<RouteKey, ft_supply_route> > entries;
    ft_map_snapshot(this->_supply_routes, entries);
    for (size_t i = 0; i < entries.size(); ++i)
    {
        Pair<RouteKey, ft_supply_route> *entry = this->_supply_routes.find(entries[i].key);
        if (entry != ft_nullptr)
            this->decay_route_threat(entry->value, seconds);
    }
}

bool Game::append_resource_lore_snippet(int resource_id, int origin_planet_id, int destination_planet_id)
{
    const ResourceLoreDefinition *definition = find_resource_lore_definition(resource_id);
    if (!definition || definition->count == 0)
        return false;
    int cursor = 0;
    Pair<int, int> *tracker = this->_resource_lore_cursors.find(resource_id);
    if (tracker != ft_nullptr)
        cursor = tracker->value;
    if (cursor < 0 || static_cast<size_t>(cursor) >= definition->count)
        cursor = 0;
    const char *line = definition->lines[cursor];
    if (!line)
        return false;
    ft_string snippet(line);
    const char *origin_name = resolve_planet_name(origin_planet_id);
    const char *destination_name = resolve_planet_name(destination_planet_id);
    if (origin_name && destination_name)
    {
        snippet.append(ft_string(" ("));
        snippet.append(origin_name);
        snippet.append(ft_string(" -> "));
        snippet.append(destination_name);
        snippet.append(ft_string(")"));
    }
    else if (origin_planet_id != 0 || destination_planet_id != 0)
    {
        snippet.append(ft_string(" (Route "));
        snippet.append(ft_to_string(origin_planet_id));
        snippet.append(ft_string(" -> "));
        snippet.append(ft_to_string(destination_planet_id));
        snippet.append(ft_string(")"));
    }
    this->append_lore_entry(snippet);
    int next_cursor = cursor + 1;
    if (next_cursor < 0 || static_cast<size_t>(next_cursor) >= definition->count)
        next_cursor = 0;
    if (tracker != ft_nullptr)
        tracker->value = next_cursor;
    else
        this->_resource_lore_cursors.insert(resource_id, next_cursor);
    if (definition->journal_entry_id != 0 && definition->journal_text != ft_nullptr)
    {
        if (!this->is_journal_entry_unlocked(definition->journal_entry_id))
        {
            ft_string journal_text(definition->journal_text);
            this->unlock_journal_entry(definition->journal_entry_id, journal_text);
        }
    }
    return true;
}

bool Game::append_raider_lore_snippet(int origin_planet_id, int destination_planet_id)
{
    if (kRaiderLoreSnippetCount == 0)
        return false;
    if (this->_raider_lore_cursor < 0 || static_cast<size_t>(this->_raider_lore_cursor) >= kRaiderLoreSnippetCount)
        this->_raider_lore_cursor = 0;
    const char *line = kRaiderLoreSnippets[this->_raider_lore_cursor];
    if (!line)
        return false;
    ft_string snippet(line);
    const char *origin_name = resolve_planet_name(origin_planet_id);
    const char *destination_name = resolve_planet_name(destination_planet_id);
    if (origin_name && destination_name)
    {
        snippet.append(ft_string(" (Ambush corridor: "));
        snippet.append(origin_name);
        snippet.append(ft_string(" -> "));
        snippet.append(destination_name);
        snippet.append(ft_string(")"));
    }
    else if (origin_planet_id != 0 || destination_planet_id != 0)
    {
        snippet.append(ft_string(" (Ambush corridor: "));
        snippet.append(ft_to_string(origin_planet_id));
        snippet.append(ft_string(" -> "));
        snippet.append(ft_to_string(destination_planet_id));
        snippet.append(ft_string(")"));
    }
    this->append_lore_entry(snippet);
    if (kRaiderJournalEntryText != ft_nullptr &&
        !this->is_journal_entry_unlocked(JOURNAL_ENTRY_RAIDER_SIGNAL_WEB))
    {
        ft_string journal_text(kRaiderJournalEntryText);
        this->unlock_journal_entry(JOURNAL_ENTRY_RAIDER_SIGNAL_WEB, journal_text);
    }
    this->_raider_lore_cursor += 1;
    if (this->_raider_lore_cursor < 0 || static_cast<size_t>(this->_raider_lore_cursor) >= kRaiderLoreSnippetCount)
        this->_raider_lore_cursor = 0;
    return true;
}

void Game::maybe_unlock_imperium_pressure(const ft_supply_route &route)
{
    if (this->is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_IMPERIUM_PRESSURE))
        return ;
    if (route.threat_level < 3.0)
        return ;
    ft_string journal_text("Journal – Imperium Pressure: Outpost analysts intercept Imperium Solarex couriers urging corrupt governors to tighten blockades. Each spike in convoy threat reminds the Convoy Corps how the distant regime still squeezes the frontier.");
    this->unlock_journal_entry(JOURNAL_ENTRY_LORE_IMPERIUM_PRESSURE, journal_text);
}

bool Game::handle_celestial_barrens_salvage(const ft_supply_convoy &convoy)
{
    if (!route_passes_celestial_barrens(convoy.origin_planet_id, convoy.destination_planet_id))
        return false;
    ft_supply_route *route = this->get_route_by_id(convoy.route_id);
    if (!route)
        return false;
    double chance = 0.18;
    if (route->threat_level > 0.0)
    {
        double bonus = route->threat_level * 0.035;
        if (bonus > 0.18)
            bonus = 0.18;
        chance += bonus;
    }
    if (convoy.escort_rating > 0)
        chance += 0.05;
    if (chance > 0.65)
        chance = 0.65;
    double roll = static_cast<double>(ft_random_float());
    if (roll >= chance)
        return false;
    double selection = static_cast<double>(ft_random_float());
    int salvage_resource = ORE_GOLD;
    if (selection < 0.45)
        salvage_resource = ORE_GOLD;
    else if (selection < 0.8)
        salvage_resource = ORE_CRYSTAL;
    else
        salvage_resource = ORE_TRITIUM;
    int salvage_amount = 1 + static_cast<int>(route->threat_level * 0.5);
    if (convoy.escort_rating > 0)
        salvage_amount += 1;
    if (salvage_amount < 1)
        salvage_amount = 1;
    if (salvage_amount > 6)
        salvage_amount = 6;
    ft_sharedptr<ft_planet> destination = this->get_planet(convoy.destination_planet_id);
    if (destination)
    {
        destination->add_resource(salvage_resource, salvage_amount);
        this->send_state(convoy.destination_planet_id, salvage_resource);
    }
    ft_string resource_label;
    const char *resource_name = resolve_resource_name(salvage_resource);
    if (resource_name != ft_nullptr)
        resource_label = ft_string(resource_name);
    else
        resource_label = ft_to_string(salvage_resource);
    ft_string entry("Scout Finn threads the Celestial Barrens and flags drifting ore veins. Salvage crews haul ");
    entry.append(ft_to_string(salvage_amount));
    entry.append(ft_string(" units of "));
    entry.append(resource_label);
