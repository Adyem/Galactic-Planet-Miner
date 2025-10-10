void Game::tick_milliseconds(double milliseconds)
{
    if (milliseconds < 0.0)
        milliseconds = 0.0;
    this->update_gameplay(milliseconds);
    this->update_combat(milliseconds);
}

void Game::tick(double seconds)
{
    double milliseconds = seconds * 1000.0;
    this->tick_milliseconds(milliseconds);
}

int Game::select_planet_resource_for_assault(const ft_sharedptr<ft_planet> &planet, int minimum_stock, bool allow_stock_fallback) const noexcept
{
    if (!planet)
        return 0;
    const ft_vector<Pair<int, double> > &resources = planet->get_resources();
    int selected_ore = 0;
    double selected_rate = 0.0;
    int selected_stock = 0;
    bool has_rate_candidate = false;
    const double epsilon = 0.0000001;
    for (size_t i = 0; i < resources.size(); ++i)
    {
        int ore_id = resources[i].key;
        double rate = resources[i].value;
        if (rate <= 0.0)
            continue;
        int stock = planet->get_resource(ore_id);
        if (stock < minimum_stock)
            continue;
        if (!has_rate_candidate || rate > selected_rate + epsilon)
        {
            selected_ore = ore_id;
            selected_rate = rate;
            selected_stock = stock;
            has_rate_candidate = true;
        }
        else if (math_fabs(rate - selected_rate) <= epsilon && stock > selected_stock)
        {
            selected_ore = ore_id;
            selected_stock = stock;
        }
    }
    if (has_rate_candidate)
        return selected_ore;
    if (!allow_stock_fallback)
        return 0;
    int fallback_ore = 0;
    int fallback_stock = 0;
    for (size_t i = 0; i < resources.size(); ++i)
    {
        int ore_id = resources[i].key;
        int stock = planet->get_resource(ore_id);
        if (stock < minimum_stock)
            continue;
        if (stock > fallback_stock)
        {
            fallback_ore = ore_id;
            fallback_stock = stock;
        }
    }
    return fallback_ore;
}

Pair<int, Game::ft_resource_accumulator> *Game::get_resource_accumulator(int planet_id, int ore_id, bool create)
{
    Pair<int, ft_sharedptr<ft_vector<Pair<int, ft_resource_accumulator> > > > *planet_entry = this->_resource_deficits.find(planet_id);
    if (planet_entry == ft_nullptr)
    {
        if (!create)
            return ft_nullptr;
        ft_sharedptr<ft_vector<Pair<int, ft_resource_accumulator> > > new_accumulators(new ft_vector<Pair<int, ft_resource_accumulator> >());
        this->_resource_deficits.insert(planet_id, new_accumulators);
        planet_entry = this->_resource_deficits.find(planet_id);
        if (planet_entry == ft_nullptr)
            return ft_nullptr;
    }
    ft_sharedptr<ft_vector<Pair<int, ft_resource_accumulator> > > &vector_ptr = planet_entry->value;
    if (!vector_ptr)
    {
        if (!create)
            return ft_nullptr;
        ft_sharedptr<ft_vector<Pair<int, ft_resource_accumulator> > > replacement(new ft_vector<Pair<int, ft_resource_accumulator> >());
        planet_entry->value = replacement;
        vector_ptr = planet_entry->value;
    }
    if (!vector_ptr)
        return ft_nullptr;
    ft_vector<Pair<int, ft_resource_accumulator> > &entries = *vector_ptr;
    for (size_t i = 0; i < entries.size(); ++i)
    {
        if (entries[i].key == ore_id)
            return &entries[i];
    }
    if (!create)
        return ft_nullptr;
    Pair<int, ft_resource_accumulator> new_entry;
    new_entry.key = ore_id;
    new_entry.value = ft_resource_accumulator();
    entries.push_back(new_entry);
    return &entries[entries.size() - 1];
}

ft_sharedptr<ft_planet> Game::get_planet(int id)
{
    Pair<int, ft_sharedptr<ft_planet> > *entry = this->_planets.find(id);
    if (entry == ft_nullptr)
        return ft_sharedptr<ft_planet>();
    return entry->value;
}

ft_sharedptr<const ft_planet> Game::get_planet(int id) const
{
    const Pair<int, ft_sharedptr<ft_planet> > *entry = this->_planets.find(id);
    if (entry == ft_nullptr)
        return ft_sharedptr<const ft_planet>();
    return entry->value;
}

ft_sharedptr<ft_planet> Game::get_planet_storage_target(int id)
{
    Pair<int, ft_sharedptr<ft_planet> > *entry = this->_planets.find(id);
    if (entry != ft_nullptr)
        return entry->value;
    Pair<int, ft_sharedptr<ft_planet> > *locked = this->_locked_planets.find(id);
    if (locked != ft_nullptr)
        return locked->value;
    return ft_sharedptr<ft_planet>();
}

ft_sharedptr<const ft_planet> Game::get_planet_storage_target(int id) const
{
    const Pair<int, ft_sharedptr<ft_planet> > *entry = this->_planets.find(id);
    if (entry != ft_nullptr)
        return entry->value;
    const Pair<int, ft_sharedptr<ft_planet> > *locked = this->_locked_planets.find(id);
    if (locked != ft_nullptr)
        return locked->value;
    return ft_sharedptr<const ft_planet>();
}

ft_sharedptr<ft_fleet> Game::get_fleet(int id)
{
    Pair<int, ft_sharedptr<ft_fleet> > *entry = this->_fleets.find(id);
    if (entry == ft_nullptr)
        return ft_sharedptr<ft_fleet>();
    return entry->value;
}

ft_sharedptr<const ft_fleet> Game::get_fleet(int id) const
{
    const Pair<int, ft_sharedptr<ft_fleet> > *entry = this->_fleets.find(id);
    if (entry == ft_nullptr)
        return ft_sharedptr<const ft_fleet>();
    return entry->value;
}

ft_sharedptr<ft_fleet> Game::get_planet_fleet(int id)
{
    Pair<int, ft_sharedptr<ft_fleet> > *entry = this->_planet_fleets.find(id);
    if (entry != ft_nullptr)
        return entry->value;
    ft_sharedptr<ft_planet> planet = this->get_planet(id);
    if (!planet)
        return ft_sharedptr<ft_fleet>();
    ft_sharedptr<ft_fleet> garrison(new ft_fleet(-id));
    garrison->set_location_planet(id);
    this->_state.add_character(garrison);
    this->_planet_fleets.insert(id, garrison);
    return garrison;
}

ft_sharedptr<const ft_fleet> Game::get_planet_fleet(int id) const
{
    const Pair<int, ft_sharedptr<ft_fleet> > *entry = this->_planet_fleets.find(id);
    if (entry == ft_nullptr)
        return ft_sharedptr<const ft_fleet>();
    return entry->value;
}

void Game::queue_pending_resource_update(int planet_id, int ore_id, int amount)
{
    Pair<int, ft_sharedptr<ft_map<int, int> > > *planet_entry = this->_pending_resource_updates.find(planet_id);
    if (planet_entry == ft_nullptr)
    {
        ft_sharedptr<ft_map<int, int> > ore_map(new ft_map<int, int>());
        this->_pending_resource_updates.insert(planet_id, ore_map);
        planet_entry = this->_pending_resource_updates.find(planet_id);
    }
    if (planet_entry == ft_nullptr)
        return ;
    ft_sharedptr<ft_map<int, int> > &ore_map_ptr = planet_entry->value;
    if (!ore_map_ptr)
    {
        ft_sharedptr<ft_map<int, int> > replacement(new ft_map<int, int>());
        planet_entry->value = replacement;
    }
    ft_sharedptr<ft_map<int, int> > &ore_map_ref = planet_entry->value;
    Pair<int, int> *ore_entry = ore_map_ref->find(ore_id);
    if (ore_entry == ft_nullptr)
        ore_map_ref->insert(ore_id, amount);
    else
        ore_entry->value = amount;
}

void Game::clear_pending_resource_update(int planet_id, int ore_id)
{
    Pair<int, ft_sharedptr<ft_map<int, int> > > *planet_entry = this->_pending_resource_updates.find(planet_id);
    if (planet_entry == ft_nullptr)
        return ;
    ft_sharedptr<ft_map<int, int> > ore_map_ptr = planet_entry->value;
    if (!ore_map_ptr)
    {
        this->_pending_resource_updates.remove(planet_id);
        return ;
    }
    ore_map_ptr->remove(ore_id);
    if (ore_map_ptr->size() == 0)
        this->_pending_resource_updates.remove(planet_id);
}

bool Game::dispatch_resource_update(int planet_id, int ore_id, int amount)
{
    ft_string body("{"planet":");
    body.append(ft_to_string(planet_id));
    body.append(","ore":");
    body.append(ft_to_string(ore_id));
    body.append(","amount":");
    body.append(ft_to_string(amount));
    body.append("}");
    ft_string response;
    int status = this->_backend.send_state(body, response);
    bool offline = (status < 200 || status >= 400);
    const ft_string fallback_prefix("[offline] echo=");
    size_t prefix_size = fallback_prefix.size();
    if (!offline && response.size() >= prefix_size)
    {
        const char *resp_cstr = response.c_str();
        if (ft_strncmp(resp_cstr, fallback_prefix.c_str(), static_cast<size_t>(prefix_size)) == 0)
            offline = true;
    }
    if (offline)
    {
        long retry_now_ms = ft_time_ms();
        if (this->_backend_retry_delay_ms <= 0)
            this->_backend_retry_delay_ms = BACKEND_RETRY_INITIAL_DELAY_MS;
        else
        {
            long doubled_delay = this->_backend_retry_delay_ms * 2;
            if (doubled_delay > BACKEND_RETRY_MAX_DELAY_MS)
                doubled_delay = BACKEND_RETRY_MAX_DELAY_MS;
            this->_backend_retry_delay_ms = doubled_delay;
        }
        this->_backend_next_retry_ms = retry_now_ms + this->_backend_retry_delay_ms;
        bool was_online = this->_backend_online;
        this->_backend_online = false;
        if (was_online)
        {
            ft_string entry("Operations report: backend connection lost");
            entry.append(" (status ");
            entry.append(ft_to_string(status));
            entry.append("). Switching to offline mode.");
            this->append_lore_entry(entry);
        }
        return false;
    }

    this->_backend_retry_delay_ms = 0;
    this->_backend_next_retry_ms = 0;
    bool was_offline = !this->_backend_online;
    this->_backend_online = true;
    if (was_offline)
    {
        ft_string entry("Operations report: backend connection restored.");
        this->append_lore_entry(entry);
    }

    Pair<int, ft_sharedptr<ft_map<int, int> > > *planet_entry = this->_last_sent_resources.find(planet_id);
    if (planet_entry == ft_nullptr)
    {
        ft_sharedptr<ft_map<int, int> > ore_map(new ft_map<int, int>());
        this->_last_sent_resources.insert(planet_id, ore_map);
        planet_entry = this->_last_sent_resources.find(planet_id);
    }
    if (planet_entry != ft_nullptr)
    {
        ft_sharedptr<ft_map<int, int> > &ore_map_ptr = planet_entry->value;
        if (!ore_map_ptr)
        {
            ft_sharedptr<ft_map<int, int> > replacement(new ft_map<int, int>());
            planet_entry->value = replacement;
        }
        ft_sharedptr<ft_map<int, int> > &ore_map_ref = planet_entry->value;
        Pair<int, int> *ore_entry = ore_map_ref->find(ore_id);
        if (ore_entry == ft_nullptr)
            ore_map_ref->insert(ore_id, amount);
        else
            ore_entry->value = amount;
    }
    return true;
}
