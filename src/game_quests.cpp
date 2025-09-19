#include "game.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Template/pair.hpp"

void Game::build_quest_context(ft_quest_context &context) const
{
    size_t planet_count = this->_planets.size();
    const Pair<int, ft_sharedptr<ft_planet> > *planet_entries = this->_planets.end();
    planet_entries -= planet_count;
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

    size_t fleet_count = this->_fleets.size();
    const Pair<int, ft_sharedptr<ft_fleet> > *fleet_entries = this->_fleets.end();
    fleet_entries -= fleet_count;
    for (size_t i = 0; i < fleet_count; ++i)
    {
        const ft_sharedptr<ft_fleet> &fleet = fleet_entries[i].value;
        context.total_ship_count += fleet->get_ship_count();
        context.total_ship_hp += fleet->get_total_ship_hp();
    }
    size_t garrison_count = this->_planet_fleets.size();
    const Pair<int, ft_sharedptr<ft_fleet> > *garrison_entries = this->_planet_fleets.end();
    garrison_entries -= garrison_count;
    for (size_t i = 0; i < garrison_count; ++i)
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
    size_t route_count = this->_supply_routes.size();
    if (route_count > 0)
    {
        const Pair<RouteKey, ft_supply_route> *route_entries = this->_supply_routes.end();
        route_entries -= route_count;
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

void Game::handle_quest_completion(int quest_id)
{
    ft_string entry;
    if (quest_id == QUEST_INITIAL_SKIRMISHES)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ITEM_ENGINE_PART);
        this->add_ore(PLANET_TERRA, ITEM_ENGINE_PART, 2);
        entry = ft_string("Old Miner Joe cheers as Terra's convoys return with salvaged engine parts.");
    }
    else if (quest_id == QUEST_DEFENSE_OF_TERRA)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_COAL);
        this->add_ore(PLANET_TERRA, ORE_COAL, 6);
        entry = ft_string("Professor Lumen catalogs the victory over Terra: new coal shipments fuel the forges.");
    }
    else if (quest_id == QUEST_INVESTIGATE_RAIDERS)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_MITHRIL);
        this->add_ore(PLANET_TERRA, ORE_MITHRIL, 4);
        entry = ft_string("Farmer Daisy archives decoded mithril caches that hint at raider supply routes.");
    }
    else if (quest_id == QUEST_CLIMACTIC_BATTLE)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_TITANIUM);
        this->add_ore(PLANET_TERRA, ORE_TITANIUM, 3);
        entry = ft_string("Old Scout Finn records the climactic stand: captured titanium plating is repurposed.");
    }
    else if (quest_id == QUEST_ORDER_UPRISING)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ITEM_ENGINE_PART);
        this->add_ore(PLANET_TERRA, ITEM_ENGINE_PART, 3);
        entry = ft_string("Farmer Daisy distributes engine parts to keep loyalist transports operational.");
    }
    else if (quest_id == QUEST_REBELLION_FLEET)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_OBSIDIAN);
        this->add_ore(PLANET_TERRA, ORE_OBSIDIAN, 2);
        entry = ft_string("Professor Lumen preserves obsidian shards from allied rebels as evidence of hope.");
    }
    else if (quest_id == QUEST_ORDER_SUPPRESS_RAIDS)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_MARS);
        if (route != ft_nullptr)
        {
            double reduction = route->threat_level;
            if (reduction > 0.0)
                this->modify_route_threat(*route, -reduction, false);
            route->quiet_timer += 180.0;
            if (route->quiet_timer > 480.0)
                route->quiet_timer = 480.0;
        }
        this->_order_branch_assault_victories = 0;
        this->_order_branch_pending_assault = 0;
        entry = ft_string("Marshal Rhea files triumphant reports as Order sentries hush the raider signal web.");
        this->trigger_branch_assault(PLANET_MARS, 1.15, true);
    }
    else if (quest_id == QUEST_ORDER_DOMINION)
    {
        this->_order_branch_pending_assault = 0;
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_TITANIUM);
        this->add_ore(PLANET_TERRA, ORE_TITANIUM, 4);
        entry = ft_string("Marshal Rhea proclaims Dominion law after Mars bows to loyalist control.");
    }
    else if (quest_id == QUEST_REBELLION_NETWORK)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
        if (route != ft_nullptr)
        {
            if (route->threat_level > 1.5)
                this->modify_route_threat(*route, -1.5, false);
            route->quiet_timer += 120.0;
            if (route->quiet_timer > 420.0)
                route->quiet_timer = 420.0;
        }
        this->_rebellion_branch_assault_victories = 0;
        this->_rebellion_branch_pending_assault = 0;
        entry = ft_string("Captain Blackthorne celebrates the shadow network relays lighting up across the belt.");
        this->trigger_branch_assault(PLANET_ZALTHOR, 1.05, false);
    }
    else if (quest_id == QUEST_REBELLION_LIBERATION)
    {
        this->_rebellion_branch_pending_assault = 0;
        this->ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ORE_OBSIDIAN);
        this->add_ore(PLANET_NOCTARIS_PRIME, ORE_OBSIDIAN, 5);
        entry = ft_string("Farmer Daisy chronicles rebel banners rising over Zalthor's liberated shipyards.");
    }
    if (entry.size() > 0)
        this->_lore_log.push_back(entry);
    this->record_quest_achievement(quest_id);
    ft_string tag("quest_completed_");
    tag.append(ft_to_string(quest_id));
    if (!this->save_campaign_checkpoint(tag))
        return ;
}

void Game::handle_quest_failure(int quest_id)
{
    ft_string entry;
    if (quest_id == QUEST_DEFENSE_OF_TERRA)
    {
        this->sub_ore(PLANET_TERRA, ITEM_ENGINE_PART, 1);
        entry = ft_string("Professor Lumen warns that Terra's defenses falter and precious engine parts are lost.");
    }
    else if (quest_id == QUEST_ORDER_UPRISING)
    {
        this->sub_ore(PLANET_TERRA, ORE_COAL, 4);
        entry = ft_string("Old Miner Joe laments that unrest drains coal reserves meant for the foundries.");
    }
    else if (quest_id == QUEST_REBELLION_FLEET)
    {
        this->sub_ore(PLANET_TERRA, ORE_MITHRIL, 2);
        entry = ft_string("Farmer Daisy notes that promised mithril reinforcements never arrive for the rebellion.");
    }
    else if (quest_id == QUEST_ORDER_SUPPRESS_RAIDS)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_MARS);
        if (route != ft_nullptr)
            this->modify_route_threat(*route, 1.5, true);
        entry = ft_string("Marshal Rhea fumes as raider cells resurge and convoys report renewed ambushes.");
    }
    else if (quest_id == QUEST_ORDER_DOMINION)
    {
        this->_order_branch_pending_assault = 0;
        this->sub_ore(PLANET_TERRA, ORE_TITANIUM, 2);
        entry = ft_string("Professor Lumen laments that Dominion plans falter and titanium stockpiles burn away.");
    }
    else if (quest_id == QUEST_REBELLION_NETWORK)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
        if (route != ft_nullptr)
            this->modify_route_threat(*route, 1.0, true);
        entry = ft_string("Navigator Zara whispers that rebel relays fall dark as raids intensify.");
    }
    else if (quest_id == QUEST_REBELLION_LIBERATION)
    {
        this->_rebellion_branch_pending_assault = 0;
        this->sub_ore(PLANET_NOCTARIS_PRIME, ORE_OBSIDIAN, 3);
        entry = ft_string("Old Miner Joe recounts how the liberation bid falters and obsidian caches are seized.");
    }
    if (entry.size() > 0)
        this->_lore_log.push_back(entry);
}

void Game::handle_quest_choice_prompt(int quest_id)
{
    if (quest_id != QUEST_CRITICAL_DECISION)
        return ;
    ft_string entry("Navigator Zara's sacrifice forces a reckoning over Captain Blackthorne's fate.");
    this->_lore_log.push_back(entry);
}

void Game::handle_quest_choice_resolution(int quest_id, int choice_id)
{
    if (quest_id != QUEST_CRITICAL_DECISION)
        return ;
    ft_string entry;
    if (choice_id == QUEST_CHOICE_EXECUTE_BLACKTHORNE)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_COAL);
        this->add_ore(PLANET_TERRA, ORE_COAL, 5);
        entry = ft_string("Captain Blackthorne's execution steels Terra's loyalists; coal stockpiles surge for the war effort.");
    }
    else if (choice_id == QUEST_CHOICE_SPARE_BLACKTHORNE)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_CRYSTAL);
        this->add_ore(PLANET_TERRA, ORE_CRYSTAL, 3);
        entry = ft_string("Sparing Blackthorne yields encoded crystal data that Professor Lumen studies for hidden conspiracies.");
    }
    if (entry.size() > 0)
        this->_lore_log.push_back(entry);
    this->record_quest_achievement(quest_id);
}

bool Game::resolve_quest_choice(int quest_id, int choice_id)
{
    if (!this->_quests.make_choice(quest_id, choice_id))
        return false;
    ft_quest_context context;
    this->build_quest_context(context);
    ft_vector<int> quest_completed;
    ft_vector<int> quest_failed;
    ft_vector<int> quest_choices;
    this->_quests.update(0.0, context, quest_completed, quest_failed, quest_choices);
    for (size_t i = 0; i < quest_completed.size(); ++i)
        this->handle_quest_completion(quest_completed[i]);
    for (size_t i = 0; i < quest_failed.size(); ++i)
        this->handle_quest_failure(quest_failed[i]);
    for (size_t i = 0; i < quest_choices.size(); ++i)
        this->handle_quest_choice_prompt(quest_choices[i]);
    this->handle_quest_choice_resolution(quest_id, choice_id);
    return true;
}

int Game::get_active_quest() const
{
    return this->_quests.get_active_quest_id();
}

int Game::get_quest_status(int quest_id) const
{
    return this->_quests.get_status(quest_id);
}

double Game::get_quest_time_remaining(int quest_id) const
{
    return this->_quests.get_time_remaining(quest_id);
}

int Game::get_quest_choice(int quest_id) const
{
    return this->_quests.get_choice(quest_id);
}

const ft_vector<ft_string> &Game::get_lore_log() const
{
    return this->_lore_log;
}

bool Game::is_backend_online() const
{
    return this->_backend_online;
}

