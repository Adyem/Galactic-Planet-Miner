bool Game::flush_pending_resource_updates()
{
    ft_vector<Pair<int, ft_sharedptr<ft_map<int, int> > > > planet_entries;
    ft_map_snapshot(this->_pending_resource_updates, planet_entries);
    size_t pending_planet_count = planet_entries.size();
    if (pending_planet_count == 0)
        return true;

    ft_vector<ft_pending_resource_update> updates;
    for (size_t i = 0; i < pending_planet_count; ++i)
    {
        int planet_id = planet_entries[i].key;
        ft_sharedptr<ft_map<int, int> > ore_map_ptr = planet_entries[i].value;
        if (!ore_map_ptr)
            continue;
        ft_vector<Pair<int, int> > ore_entries;
        ft_map_snapshot(*ore_map_ptr, ore_entries);
        size_t ore_count = ore_entries.size();
        if (ore_count == 0)
            continue;
        for (size_t j = 0; j < ore_count; ++j)
        {
            ft_pending_resource_update update(planet_id, ore_entries[j].key, ore_entries[j].value);
            updates.push_back(update);
        }
    }

    size_t update_count = updates.size();
    for (size_t index = 0; index < update_count; ++index)
    {
        const ft_pending_resource_update &update = updates[index];
        if (!this->dispatch_resource_update(update.planet_id, update.ore_id, update.amount))
            return false;
        this->clear_pending_resource_update(update.planet_id, update.ore_id);
    }
    return true;
}

void Game::send_state(int planet_id, int ore_id)
{
    ft_sharedptr<const ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return ;

    int amount = planet->get_resource(ore_id);
    if (this->_backend_online)
    {
        Pair<int, ft_sharedptr<ft_map<int, int> > > *planet_entry = this->_last_sent_resources.find(planet_id);
        if (planet_entry != ft_nullptr)
        {
            ft_sharedptr<ft_map<int, int> > &ore_map_ptr = planet_entry->value;
            if (ore_map_ptr)
            {
                Pair<int, int> *ore_entry = ore_map_ptr->find(ore_id);
                if (ore_entry != ft_nullptr && ore_entry->value == amount)
                    return ;
            }
        }
    }

    this->queue_pending_resource_update(planet_id, ore_id, amount);

    if (!this->_backend_online)
    {
        long current_time_ms = ft_time_ms();
        if (this->_backend_next_retry_ms != 0 && current_time_ms < this->_backend_next_retry_ms)
            return ;
    }

    this->flush_pending_resource_updates();
}

void Game::unlock_planet(int planet_id)
{
    if (this->_planets.find(planet_id) != ft_nullptr)
        return ;
    Pair<int, ft_sharedptr<ft_planet> > *entry = this->_locked_planets.find(planet_id);
    if (entry == ft_nullptr)
        return ;
    ft_sharedptr<ft_planet> planet = entry->value;
    this->_planets.insert(planet_id, planet);
    this->_locked_planets.remove(planet_id);
    this->_state.add_character(planet);
    this->_buildings.initialize_planet(*this, planet_id);
    const ft_vector<Pair<int, double> > &resources = planet->get_resources();
    for (size_t i = 0; i < resources.size(); ++i)
        this->send_state(planet_id, resources[i].key);
    this->record_achievement_event(ACHIEVEMENT_EVENT_PLANET_UNLOCKED, 1);
}

void Game::configure_difficulty(int difficulty)
{
    int selected = difficulty;
    if (selected != GAME_DIFFICULTY_EASY && selected != GAME_DIFFICULTY_HARD)
        selected = GAME_DIFFICULTY_STANDARD;
    this->_difficulty = selected;
    if (selected == GAME_DIFFICULTY_EASY)
    {
        this->_resource_multiplier = 1.25;
        this->_quest_time_scale_base = 1.25;
        this->_research_duration_scale = 0.85;
        this->_assault_difficulty_multiplier = 0.85;
    }
    else if (selected == GAME_DIFFICULTY_HARD)
    {
        this->_resource_multiplier = 0.85;
        this->_quest_time_scale_base = 0.75;
        this->_research_duration_scale = 1.2;
        this->_assault_difficulty_multiplier = 1.25;
    }
    else
    {
        this->_resource_multiplier = 1.0;
        this->_quest_time_scale_base = 1.0;
        this->_research_duration_scale = 1.0;
        this->_assault_difficulty_multiplier = 1.0;
    }
    this->_research.set_duration_scale(this->_research_duration_scale);
    this->update_dynamic_quest_pressure();
    this->update_combat_modifiers();
}

void Game::set_ui_scale(double scale)
{
    double clamped = scale;
    if (clamped < 0.5)
        clamped = 0.5;
    if (clamped > 2.0)
        clamped = 2.0;
    this->_ui_scale = clamped;
}

void Game::set_lore_panel_anchor(int anchor)
{
    int normalized = anchor;
    if (normalized != PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT
        && normalized != PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT)
        normalized = PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT;
    this->_lore_panel_anchor = normalized;
}

void Game::set_combat_speed_multiplier(double multiplier)
{
    double clamped = multiplier;
    if (clamped < 0.25)
        clamped = 0.25;
    if (clamped > 2.0)
        clamped = 2.0;
    this->_combat_speed_multiplier = clamped;
}

void Game::apply_preferences(const PlayerProfilePreferences &preferences)
{
    this->configure_from_preferences(preferences);
}

void Game::configure_from_preferences(const PlayerProfilePreferences &preferences)
{
    double ui_scale = static_cast<double>(preferences.ui_scale_percent);
    if (ui_scale <= 0.0)
        ui_scale = 100.0;
    this->set_ui_scale(ui_scale / 100.0);

    this->set_lore_panel_anchor(static_cast<int>(preferences.lore_panel_anchor));

    double combat_speed = static_cast<double>(preferences.combat_speed_percent);
    if (combat_speed <= 0.0)
        combat_speed = 100.0;
    this->set_combat_speed_multiplier(combat_speed / 100.0);
}

double Game::get_effective_quest_time_scale() const
{
    return this->_quest_time_scale_base * this->_quest_time_scale_dynamic;
}

void Game::apply_quest_time_scale()
{
    double effective = this->get_effective_quest_time_scale();
    if (effective <= 0.0)
        effective = 0.0001;
    this->_quests.set_time_scale(effective);
}

void Game::update_dynamic_quest_pressure()
{
    ft_vector<Pair<int, ft_sharedptr<ft_quest_definition> > > definitions;
    this->_quests.snapshot_definitions(definitions);
    size_t count = definitions.size();
    int completed_main = 0;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_sharedptr<ft_quest_definition> &definition_ptr = definitions[i].value;
