bool Game::load_campaign_from_save(const ft_string &planet_json, const ft_string &fleet_json,
    const ft_string &research_json, const ft_string &achievement_json,
    const ft_string &building_json, const ft_string &progress_json) noexcept
{
    ft_map<int, ft_sharedptr<ft_planet> > planet_snapshot;
    ft_map<int, ft_sharedptr<ft_fleet> > fleet_snapshot;
    bool planets_ok = true;
    bool fleets_ok = true;
    bool research_ok = true;
    bool achievements_ok = true;
    bool buildings_ok = true;
    bool progress_ok = true;
    ft_map<int, ft_research_progress> research_state;
    double research_duration = this->_research.get_duration_scale();
    bool research_snapshot_present = false;
    ft_map<int, ft_achievement_progress> achievement_state;
    bool achievement_snapshot_present = false;
    BuildingManager building_snapshot;
    bool building_snapshot_present = false;
    int convoys_delivered_total = this->_convoys_delivered_total;
    int convoy_raid_losses = this->_convoy_raid_losses;
    int current_delivery_streak = this->_current_delivery_streak;
    int longest_delivery_streak = this->_longest_delivery_streak;
    size_t next_streak_milestone_index = this->_next_streak_milestone_index;
    int order_branch_assault_victories = this->_order_branch_assault_victories;
    int rebellion_branch_assault_victories = this->_rebellion_branch_assault_victories;
    int order_branch_pending_assault = this->_order_branch_pending_assault;
    int rebellion_branch_pending_assault = this->_rebellion_branch_pending_assault;
    if (planet_json.size() > 0)
        planets_ok = this->_save_system.deserialize_planets(planet_json.c_str(), planet_snapshot);
    if (fleet_json.size() > 0)
        fleets_ok = this->_save_system.deserialize_fleets(fleet_json.c_str(), fleet_snapshot);
    if (research_json.size() > 0)
    {
        ResearchManager research_snapshot;
// Auto-generated split part 8 of game.cpp

        research_ok = this->_save_system.deserialize_research(research_json.c_str(), research_snapshot);
        if (research_ok)
        {
            research_snapshot.get_progress_state(research_state);
            research_duration = research_snapshot.get_duration_scale();
            research_snapshot_present = true;
        }
    }
    if (achievement_json.size() > 0)
    {
        AchievementManager achievement_snapshot;
        achievements_ok = this->_save_system.deserialize_achievements(achievement_json.c_str(), achievement_snapshot);
        if (achievements_ok)
        {
            achievement_snapshot.get_progress_state(achievement_state);
            achievement_snapshot_present = true;
        }
    }
    if (building_json.size() > 0)
    {
        buildings_ok = this->_save_system.deserialize_buildings(building_json.c_str(), building_snapshot);
        if (buildings_ok)
            building_snapshot_present = true;
    }
    if (progress_json.size() > 0)
    {
        progress_ok = this->_save_system.deserialize_campaign_progress(progress_json.c_str(),
            convoys_delivered_total, convoy_raid_losses, current_delivery_streak,
            longest_delivery_streak, next_streak_milestone_index,
            order_branch_assault_victories, rebellion_branch_assault_victories,
            order_branch_pending_assault, rebellion_branch_pending_assault);
    }
    if (!planets_ok || !fleets_ok || !research_ok || !achievements_ok || !buildings_ok || !progress_ok)
        return false;
    if (next_streak_milestone_index > this->_streak_milestones.size())
        next_streak_milestone_index = this->_streak_milestones.size();
    if (current_delivery_streak < 0)
        current_delivery_streak = 0;
    if (longest_delivery_streak < 0)
        longest_delivery_streak = 0;
    if (convoys_delivered_total < 0)
        convoys_delivered_total = 0;
    if (convoy_raid_losses < 0)
        convoy_raid_losses = 0;
    if (order_branch_assault_victories < 0)
        order_branch_assault_victories = 0;
    if (rebellion_branch_assault_victories < 0)
        rebellion_branch_assault_victories = 0;
    if (order_branch_pending_assault < 0)
        order_branch_pending_assault = 0;
    if (rebellion_branch_pending_assault < 0)
        rebellion_branch_pending_assault = 0;
    this->_convoys_delivered_total = convoys_delivered_total;
    this->_convoy_raid_losses = convoy_raid_losses;
    this->_current_delivery_streak = current_delivery_streak;
    if (this->_current_delivery_streak > longest_delivery_streak)
        longest_delivery_streak = this->_current_delivery_streak;
    this->_longest_delivery_streak = longest_delivery_streak;
    this->_next_streak_milestone_index = next_streak_milestone_index;
    this->_order_branch_assault_victories = order_branch_assault_victories;
    this->_rebellion_branch_assault_victories = rebellion_branch_assault_victories;
    this->_order_branch_pending_assault = order_branch_pending_assault;
    this->_rebellion_branch_pending_assault = rebellion_branch_pending_assault;
    if (research_snapshot_present)
    {
        this->_research.set_duration_scale(research_duration);
        this->_research.set_progress_state(research_state);
        this->_research_duration_scale = this->_research.get_duration_scale();
    }
    if (this->_research.get_status(RESEARCH_EMERGENCY_ENERGY_CONSERVATION) == RESEARCH_STATUS_COMPLETED)
        this->_emergency_energy_protocol = true;
    else
        this->_emergency_energy_protocol = false;
    this->_energy_conservation_active.clear();
    if (achievement_snapshot_present)
        this->_achievements.set_progress_state(achievement_state);
    if (building_snapshot_present)
        this->_buildings.clone_from(building_snapshot);
    this->apply_planet_snapshot(planet_snapshot);
    this->apply_fleet_snapshot(fleet_snapshot);
    return true;
}

