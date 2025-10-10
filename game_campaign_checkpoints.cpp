        ft_vector<Pair<int, int> > current_inventory = planet->get_items_snapshot();
        for (size_t j = 0; j < current_inventory.size(); ++j)
        {
            int existing_item_id = current_inventory[j].key;
            if (saved_item_ids.find(existing_item_id) != ft_nullptr)
                continue;
            planet->set_resource(existing_item_id, 0);
            this->send_state(planet_id, existing_item_id);
        }
        for (size_t j = 0; j < inventory_snapshot.size(); ++j)
        {
            int item_id = inventory_snapshot[j].key;
            if (item_id <= 0)
                continue;
            this->ensure_planet_item_slot(planet_id, item_id);
            int sanitized = planet->clamp_resource_amount(item_id, inventory_snapshot[j].value);
            planet->set_resource(item_id, sanitized);
            this->send_state(planet_id, item_id);
        }
        this->_resource_deficits.remove(planet_id);
    }
}

void Game::apply_fleet_snapshot(const ft_map<int, ft_sharedptr<ft_fleet> > &snapshot)
{
    ft_vector<int> to_remove;
    ft_vector<Pair<int, ft_sharedptr<ft_fleet> > > existing_entries;
    ft_map_snapshot(this->_fleets, existing_entries);
    for (size_t i = 0; i < existing_entries.size(); ++i)
    {
        int fleet_id = existing_entries[i].key;
        if (snapshot.find(fleet_id) == ft_nullptr)
            to_remove.push_back(fleet_id);
    }
    for (size_t i = 0; i < to_remove.size(); ++i)
        this->remove_fleet(to_remove[i], -1, -1);

    ft_vector<Pair<int, ft_sharedptr<ft_fleet> > > entries;
    ft_map_snapshot(snapshot, entries);
    size_t count = entries.size();
    if (count == 0)
        return ;
    int capital_ship_total = this->count_capital_ships_in_collection(this->_planet_fleets);
    if (capital_ship_total < 0)
        capital_ship_total = 0;
    for (size_t i = 0; i < count; ++i)
    {
        int fleet_id = entries[i].key;
        const ft_sharedptr<ft_fleet> &saved_fleet = entries[i].value;
        if (!saved_fleet)
            continue;
        if (this->_fleets.find(fleet_id) == ft_nullptr)
            this->create_fleet(fleet_id);
        ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
        if (!fleet)
            continue;
        this->clear_escape_pod_records(*fleet);
        fleet->clear_ships();
        ft_location location = saved_fleet->get_location();
        if (location.type == LOCATION_TRAVEL)
            fleet->set_location_travel(location.from, location.to, saved_fleet->get_travel_time());
        else if (location.type == LOCATION_MISC)
            fleet->set_location_misc(location.misc);
        else
            fleet->set_location_planet(location.from);
        fleet->set_escort_veterancy(saved_fleet->get_escort_veterancy());
        ft_vector<int> ship_ids;
        saved_fleet->get_ship_ids(ship_ids);
        for (size_t j = 0; j < ship_ids.size(); ++j)
        {
            const ft_ship *ship = saved_fleet->get_ship(ship_ids[j]);
            if (!ship)
                continue;
            bool type_allowed = this->is_ship_type_available(ship->type);
            bool capital_allowed = true;
            if (type_allowed && is_capital_ship_type(ship->type))
            {
                if (this->_capital_ship_limit <= 0)
                    capital_allowed = false;
                else if (capital_ship_total >= this->_capital_ship_limit)
                    capital_allowed = false;
            }
            if (!type_allowed || !capital_allowed)
            {
                ft_string message("Removed ship ");
                message.append(ft_to_string(static_cast<long>(ship->id)));
                message.append(" from fleet ");
                message.append(ft_to_string(static_cast<long>(fleet_id)));
                if (!type_allowed)
                    message.append(" (type unavailable)");
                else
                    message.append(" (capital limit reached)");
                this->append_lore_entry(message);
                continue;
            }
            fleet->add_ship_snapshot(*ship);
            if (is_capital_ship_type(ship->type))
                capital_ship_total += 1;
        }
    }
}

bool Game::checkpoint_campaign_state_internal(const ft_string &tag)
{
    if (this->_force_checkpoint_failure)
        return false;
    ft_string planet_json = this->_save_system.serialize_planets(this->_planets);
    ft_string fleet_json = this->_save_system.serialize_fleets(this->_fleets);
    ft_string research_json = this->_save_system.serialize_research(this->_research);
    ft_string achievement_json = this->_save_system.serialize_achievements(this->_achievements);
    ft_string building_json = this->_save_system.serialize_buildings(this->_buildings);
    ft_string progress_json = this->_save_system.serialize_campaign_progress(
        this->_convoys_delivered_total, this->_convoy_raid_losses,
        this->_current_delivery_streak, this->_longest_delivery_streak,
        this->_next_streak_milestone_index,
        this->_order_branch_assault_victories,
        this->_rebellion_branch_assault_victories,
        this->_order_branch_pending_assault,
        this->_rebellion_branch_pending_assault);
    bool planets_valid = (planet_json.size() > 0) || (this->_planets.size() == 0);
    bool fleets_valid = (fleet_json.size() > 0) || (this->_fleets.size() == 0);
    bool research_valid = (research_json.size() > 0);
    bool achievements_valid = (achievement_json.size() > 0);
    bool buildings_valid = (building_json.size() > 0);
    bool progress_valid = (progress_json.size() > 0);
    if (!planets_valid || !fleets_valid || !research_valid || !achievements_valid
        || !buildings_valid || !progress_valid)
        return false;
    this->_last_planet_checkpoint = planet_json;
    this->_last_fleet_checkpoint = fleet_json;
    this->_last_research_checkpoint = research_json;
    this->_last_achievement_checkpoint = achievement_json;
    this->_last_building_checkpoint = building_json;
    this->_last_progress_checkpoint = progress_json;
    this->_last_checkpoint_tag = tag;
    this->_has_checkpoint = true;
    return true;
}

void Game::record_checkpoint_failure(const ft_string &tag) noexcept
{
    this->_failed_checkpoint_tags.push_back(tag);
    ft_string failure_message("Checkpoint save failed: ");
    failure_message.append(tag);
    this->append_lore_entry(failure_message);
}

bool Game::save_campaign_checkpoint(const ft_string &tag) noexcept
{
    bool saved = this->checkpoint_campaign_state_internal(tag);
    if (!saved)
        this->record_checkpoint_failure(tag);
    return saved;
}

const ft_vector<ft_string> &Game::get_failed_checkpoint_tags() const noexcept
{
    return this->_failed_checkpoint_tags;
}

void Game::set_force_checkpoint_failure(bool enabled) noexcept
{
    this->_force_checkpoint_failure = enabled;
}

bool Game::has_campaign_checkpoint() const noexcept
{
    return this->_has_checkpoint;
}

const ft_string &Game::get_campaign_planet_checkpoint() const noexcept
{
    return this->_last_planet_checkpoint;
}

const ft_string &Game::get_campaign_fleet_checkpoint() const noexcept
{
    return this->_last_fleet_checkpoint;
}

const ft_string &Game::get_campaign_checkpoint_tag() const noexcept
{
    return this->_last_checkpoint_tag;
}

const ft_string &Game::get_campaign_research_checkpoint() const noexcept
{
    return this->_last_research_checkpoint;
}

const ft_string &Game::get_campaign_achievement_checkpoint() const noexcept
{
    return this->_last_achievement_checkpoint;
}

const ft_string &Game::get_campaign_building_checkpoint() const noexcept
{
    return this->_last_building_checkpoint;
}

const ft_string &Game::get_campaign_progress_checkpoint() const noexcept
{
    return this->_last_progress_checkpoint;
}

bool Game::reload_campaign_checkpoint() noexcept
{
    if (!this->_has_checkpoint)
        return false;
    return this->load_campaign_from_save(this->_last_planet_checkpoint,
        this->_last_fleet_checkpoint, this->_last_research_checkpoint,
        this->_last_achievement_checkpoint, this->_last_building_checkpoint,
        this->_last_progress_checkpoint);
}

