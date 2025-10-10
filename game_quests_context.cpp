void Game::build_quest_context(ft_quest_context &context) const
{
    ft_vector<Pair<int, ft_sharedptr<ft_planet> > > planet_entries;
    ft_map_snapshot(this->_planets, planet_entries);
    size_t planet_count = planet_entries.size();
    for (size_t i = 0; i < planet_count; ++i)
    {
        const ft_sharedptr<ft_planet> &planet = planet_entries[i].value;
        const ft_vector<Pair<int, double> > &resources = planet->get_resources();
        for (size_t j = 0; j < resources.size(); ++j)
        {
            int ore_id = resources[j].key;
            int amount = planet->get_resource(ore_id);
            Pair<int, int> *entry = context.resource_totals.find(ore_id);
            if (entry == ft_nullptr)
                context.resource_totals.insert(ore_id, amount);
            else
                entry->value += amount;
        }
    }

    context.research_status.insert(RESEARCH_UNLOCK_MARS, this->_research.is_completed(RESEARCH_UNLOCK_MARS) ? 1 : 0);
    context.research_status.insert(RESEARCH_UNLOCK_ZALTHOR, this->_research.is_completed(RESEARCH_UNLOCK_ZALTHOR) ? 1 : 0);
    context.research_status.insert(RESEARCH_UNLOCK_VULCAN, this->_research.is_completed(RESEARCH_UNLOCK_VULCAN) ? 1 : 0);
    context.research_status.insert(RESEARCH_UNLOCK_NOCTARIS, this->_research.is_completed(RESEARCH_UNLOCK_NOCTARIS) ? 1 : 0);
    context.research_status.insert(RESEARCH_SOLAR_PANELS, this->_research.is_completed(RESEARCH_SOLAR_PANELS) ? 1 : 0);
    context.research_status.insert(RESEARCH_CRAFTING_MASTERY, this->_research.is_completed(RESEARCH_CRAFTING_MASTERY) ? 1 : 0);
    context.research_status.insert(RESEARCH_SHIELD_TECHNOLOGY, this->_research.is_completed(RESEARCH_SHIELD_TECHNOLOGY) ? 1 : 0);
    context.research_status.insert(RESEARCH_REPAIR_DRONE_TECHNOLOGY, this->_research.is_completed(RESEARCH_REPAIR_DRONE_TECHNOLOGY) ? 1 : 0);
    context.research_status.insert(RESEARCH_CAPITAL_SHIP_INITIATIVE, this->_research.is_completed(RESEARCH_CAPITAL_SHIP_INITIATIVE) ? 1 : 0);

    ft_vector<Pair<int, ft_sharedptr<ft_fleet> > > fleet_entries;
    ft_map_snapshot(this->_fleets, fleet_entries);
    for (size_t i = 0; i < fleet_entries.size(); ++i)
    {
        const ft_sharedptr<ft_fleet> &fleet = fleet_entries[i].value;
        context.total_ship_count += fleet->get_ship_count();
        context.total_ship_hp += fleet->get_total_ship_hp();
    }
    ft_vector<Pair<int, ft_sharedptr<ft_fleet> > > garrison_entries;
    ft_map_snapshot(this->_planet_fleets, garrison_entries);
    for (size_t i = 0; i < garrison_entries.size(); ++i)
    {
        const ft_sharedptr<ft_fleet> &fleet = garrison_entries[i].value;
        context.total_ship_count += fleet->get_ship_count();
        context.total_ship_hp += fleet->get_total_ship_hp();
    }
    context.successful_deliveries = this->_convoys_delivered_total;
    context.convoy_raid_losses = this->_convoy_raid_losses;
    context.delivery_streak = this->_current_delivery_streak;
    double total_threat = 0.0;
    double max_threat = 0.0;
    ft_vector<Pair<RouteKey, ft_supply_route> > route_entries;
    ft_map_snapshot(this->_supply_routes, route_entries);
    size_t route_count = route_entries.size();
    if (route_count > 0)
    {
        for (size_t i = 0; i < route_count; ++i)
        {
            double threat = route_entries[i].value.threat_level;
            total_threat += threat;
            if (threat > max_threat)
                max_threat = threat;
        }
    }
    context.total_convoy_threat = total_threat;
    if (route_count > 0)
        context.average_convoy_threat = total_threat / static_cast<double>(route_count);
    else
        context.average_convoy_threat = 0.0;
    context.maximum_convoy_threat = max_threat;

    int tracked_buildings[] = {BUILDING_PROXIMITY_RADAR, BUILDING_TRADE_RELAY, BUILDING_HELIOS_BEACON};
    size_t tracked_count = sizeof(tracked_buildings) / sizeof(int);
    for (size_t i = 0; i < tracked_count; ++i)
    {
        int building_id = tracked_buildings[i];
        int total = 0;
        for (size_t j = 0; j < planet_count; ++j)
            total += this->_buildings.get_building_count(planet_entries[j].key, building_id);
        context.building_counts.insert(building_id, total);
    }

    context.assault_victories.insert(PLANET_MARS, this->_order_branch_assault_victories);
    context.assault_victories.insert(PLANET_ZALTHOR, this->_rebellion_branch_assault_victories);
}
