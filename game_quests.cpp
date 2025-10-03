#include "game.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/Template/pair.hpp"
#include "ft_map_snapshot.hpp"

namespace
{
static double get_objective_current_amount(const ft_quest_objective &objective,
                                           const ft_quest_context &context)
{
    if (objective.type == QUEST_OBJECTIVE_RESOURCE_TOTAL)
    {
        const Pair<int, int> *entry = context.resource_totals.find(objective.target_id);
        if (entry == ft_nullptr)
            return 0.0;
        return static_cast<double>(entry->value);
    }
    if (objective.type == QUEST_OBJECTIVE_RESEARCH_COMPLETED)
    {
        const Pair<int, int> *entry = context.research_status.find(objective.target_id);
        if (entry == ft_nullptr)
            return 0.0;
        return static_cast<double>(entry->value);
    }
    if (objective.type == QUEST_OBJECTIVE_FLEET_COUNT)
        return static_cast<double>(context.total_ship_count);
    if (objective.type == QUEST_OBJECTIVE_TOTAL_SHIP_HP)
        return static_cast<double>(context.total_ship_hp);
    if (objective.type == QUEST_OBJECTIVE_CONVOYS_DELIVERED)
        return static_cast<double>(context.successful_deliveries);
    if (objective.type == QUEST_OBJECTIVE_CONVOY_STREAK)
        return static_cast<double>(context.delivery_streak);
    if (objective.type == QUEST_OBJECTIVE_CONVOY_RAID_LOSSES_AT_MOST)
        return static_cast<double>(context.convoy_raid_losses);
    if (objective.type == QUEST_OBJECTIVE_MAX_CONVOY_THREAT_AT_MOST)
        return context.maximum_convoy_threat * 100.0;
    if (objective.type == QUEST_OBJECTIVE_BUILDING_COUNT)
    {
        const Pair<int, int> *entry = context.building_counts.find(objective.target_id);
        if (entry == ft_nullptr)
            return 0.0;
        return static_cast<double>(entry->value);
    }
    if (objective.type == QUEST_OBJECTIVE_ASSAULT_VICTORIES)
    {
        const Pair<int, int> *entry = context.assault_victories.find(objective.target_id);
        if (entry == ft_nullptr)
            return 0.0;
        return static_cast<double>(entry->value);
    }
    return 0.0;
}

static bool is_objective_met_for_snapshot(const ft_quest_objective &objective,
                                          const ft_quest_context &context)
{
    if (objective.type == QUEST_OBJECTIVE_RESOURCE_TOTAL)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_RESEARCH_COMPLETED)
        return get_objective_current_amount(objective, context) >= 1.0;
    if (objective.type == QUEST_OBJECTIVE_FLEET_COUNT)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_TOTAL_SHIP_HP)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_CONVOYS_DELIVERED)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_CONVOY_STREAK)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_CONVOY_RAID_LOSSES_AT_MOST)
        return get_objective_current_amount(objective, context) <= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_MAX_CONVOY_THREAT_AT_MOST)
    {
        double threshold = static_cast<double>(objective.amount) / 100.0;
        return context.maximum_convoy_threat <= threshold;
    }
    if (objective.type == QUEST_OBJECTIVE_BUILDING_COUNT)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    if (objective.type == QUEST_OBJECTIVE_ASSAULT_VICTORIES)
        return get_objective_current_amount(objective, context) >= static_cast<double>(objective.amount);
    return false;
}
} // namespace

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

void Game::handle_quest_completion(int quest_id)
{
    ft_string entry;
    if (quest_id == QUEST_INITIAL_SKIRMISHES)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ITEM_ENGINE_PART);
        this->add_ore(PLANET_TERRA, ITEM_ENGINE_PART, 2);
        entry = ft_string("Old Miner Joe cheers as Terra's convoys return with salvaged engine parts.");
        this->unlock_journal_entry(JOURNAL_ENTRY_INITIAL_RAIDER_SKIRMISHES,
            ft_string("Journal – Initial Raider Skirmishes: Old Miner Joe and Farmer Daisy rally Terra's escorts to shield core shipments as raids ignite across the belt."));
    }
    else if (quest_id == QUEST_DEFENSE_OF_TERRA)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_COAL);
        this->add_ore(PLANET_TERRA, ORE_COAL, 6);
        entry = ft_string("Professor Lumen catalogs the victory over Terra: new coal shipments fuel the forges.");
        this->unlock_journal_entry(JOURNAL_ENTRY_DEFENSE_OF_TERRA,
            ft_string("Journal – Defense of Terra: Professor Lumen warns of strange cosmic surges even as Terra's defensive wings drive the raiders from the core."));
    }
    else if (quest_id == QUEST_INVESTIGATE_RAIDERS)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_MITHRIL);
        this->add_ore(PLANET_TERRA, ORE_MITHRIL, 4);
        entry = ft_string("Farmer Daisy archives decoded mithril caches that hint at raider supply routes.");
        this->unlock_journal_entry(JOURNAL_ENTRY_INVESTIGATE_RAIDERS,
            ft_string("Journal – Investigate Raider Motives: Intelligence sweeps expose how injustice on the fringe fuels Blackthorne's insurgency and fractures the colonies."));
    }
    else if (quest_id == QUEST_SECURE_SUPPLY_LINES)
    {
        entry = ft_string("Quartermaster Nia reports convoy routes are fortified and raid losses remain contained.");
        this->unlock_journal_entry(JOURNAL_ENTRY_SECURE_SUPPLY_LINES,
            ft_string("Journal – Secure Supply Lines: Logistics crews weave Trade Relays and escorts together so Terra, Mars, and Zalthor can breathe between assaults."));
    }
    else if (quest_id == QUEST_STEADY_SUPPLY_STREAK)
    {
        entry = ft_string("Supply masters toast a flawless convoy streak that reassures nervous settlers.");
        this->unlock_journal_entry(JOURNAL_ENTRY_STEADY_SUPPLY_STREAK,
            ft_string("Journal – Steady Supply Streak: Weeks of uninterrupted deliveries calm the frontier, proving the core still watches over the outlying farms."));
    }
    else if (quest_id == QUEST_HIGH_VALUE_ESCORT)
    {
        entry = ft_string("Escort captains celebrate steering high-value cargos through the gauntlet.");
        this->unlock_journal_entry(JOURNAL_ENTRY_HIGH_VALUE_ESCORT,
            ft_string("Journal – High-Value Escort: Shield cruisers, repair drones, and Sunflare sloops coordinate to keep priceless freight intact under relentless fire."));
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
        this->unlock_journal_entry(JOURNAL_ENTRY_ORDER_UPRISING,
            ft_string("Journal – Order's Last Stand: Marshal Rhea rallies loyalist militias to stamp out uprisings sparked by Blackthorne's execution."));
    }
    else if (quest_id == QUEST_REBELLION_FLEET)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_OBSIDIAN);
        this->add_ore(PLANET_TERRA, ORE_OBSIDIAN, 2);
        entry = ft_string("Professor Lumen preserves obsidian shards from allied rebels as evidence of hope.");
        this->unlock_journal_entry(JOURNAL_ENTRY_REBELLION_FLEET,
            ft_string("Journal – Rebellion Rising: Sparing Blackthorne awakens secret allies who pledge obsidian hulls and shelter to the growing resistance."));
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
        this->unlock_journal_entry(JOURNAL_ENTRY_ORDER_SUPPRESS_RAIDS,
            ft_string("Journal – Suppress the Raider Cells: Loyalist patrols choke off raid traffic while Proximity Radars knit a surveillance lattice over Mars."));
    }
    else if (quest_id == QUEST_ORDER_DOMINION)
    {
        this->_order_branch_pending_assault = 0;
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_TITANIUM);
        this->add_ore(PLANET_TERRA, ORE_TITANIUM, 4);
        entry = ft_string("Marshal Rhea proclaims Dominion law after Mars bows to loyalist control.");
        this->unlock_journal_entry(JOURNAL_ENTRY_ORDER_DOMINION,
            ft_string("Journal – Order Dominion: Dominion edicts lock down the colonies, trading personal freedoms for the promise of security under the core."));
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
        this->unlock_journal_entry(JOURNAL_ENTRY_REBELLION_NETWORK,
            ft_string("Journal – Shadow Network: Rebel couriers light hidden relays across the fringe, trading intel and hope while convoys dodge Order patrols."));
    }
    else if (quest_id == QUEST_REBELLION_LIBERATION)
    {
        this->_rebellion_branch_pending_assault = 0;
        this->ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ORE_OBSIDIAN);
        this->add_ore(PLANET_NOCTARIS_PRIME, ORE_OBSIDIAN, 5);
        entry = ft_string("Farmer Daisy chronicles rebel banners rising over Zalthor's liberated shipyards.");
        this->unlock_journal_entry(JOURNAL_ENTRY_REBELLION_LIBERATION,
            ft_string("Journal – Liberation of the Frontier: Allied cells secure Zalthor's shipyards, promising a new order forged by those who once lived in the shadows."));
    }
    else if (quest_id == QUEST_REBELLION_FINAL_PUSH)
    {
        this->ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ORE_NANOMATERIAL);
        this->add_ore(PLANET_NOCTARIS_PRIME, ORE_NANOMATERIAL, 2);
        entry = ft_string("Professor Lumen oversees nanomaterial forges primed for the freedom armada's final strike.");
        this->unlock_journal_entry(JOURNAL_ENTRY_REBELLION_FINAL_PUSH,
            ft_string("Journal – Battle for Freedom: Rebel flotillas stockpile nanothread hull patches and liberation pledges before challenging the Dominion's last wall."));
    }
    else if (quest_id == QUEST_SIDE_CONVOY_RESCUE)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_IRON);
        this->add_ore(PLANET_TERRA, ORE_IRON, 6);
        entry = ft_string("Old Miner Joe leads relief crews as convoy escorts haul stranded settlers back under Terra's shield.");
        this->unlock_journal_entry(JOURNAL_ENTRY_SIDE_CONVOY_RESCUE,
            ft_string("Journal – Convoy Rescue Effort: Emergency escorts reroute ore and med-pods to isolated habitats while Terra's beacons promise the belt it hasn't been forgotten."));
    }
    else if (quest_id == QUEST_SIDE_OUTPOST_REPAIR)
    {
        this->ensure_planet_item_slot(PLANET_MARS, ITEM_ENGINE_PART);
        this->add_ore(PLANET_MARS, ITEM_ENGINE_PART, 2);
        entry = ft_string("Professor Lumen certifies that rebuilt redline outposts are online and requesting fresh supply lanes.");
        this->unlock_journal_entry(JOURNAL_ENTRY_SIDE_OUTPOST_REPAIR,
            ft_string("Journal – Outpost Repair Run: Convoy crews deliver engine spares and hull plating to scarred border forts, weaving Mars' defense lattice back into the network."));
    }
    else if (quest_id == QUEST_SIDE_ORDER_PROPAGANDA)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_MARS);
        if (route != ft_nullptr)
        {
            double reduction = 0.75;
            if (route->threat_level < reduction)
                reduction = route->threat_level;
            if (reduction > 0.0)
                this->modify_route_threat(*route, -reduction, false);
            route->quiet_timer += 120.0;
            if (route->quiet_timer > 480.0)
                route->quiet_timer = 480.0;
        }
        entry = ft_string("Marshal Rhea's propaganda blitz steadies loyalist convoys while raider broadcasters fall silent.");
        this->unlock_journal_entry(JOURNAL_ENTRY_SIDE_ORDER_PROPAGANDA,
            ft_string("Journal – Order Broadcast Blitz: Dominion signals flood the relays, promising security as convoys tighten formations and the belt watches under wary eyes."));
    }
    else if (quest_id == QUEST_SIDE_REBELLION_BROADCAST)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
        if (route != ft_nullptr)
        {
            if (route->threat_level > 0.5)
                this->modify_route_threat(*route, -0.5, false);
            route->quiet_timer += 150.0;
            if (route->quiet_timer > 450.0)
                route->quiet_timer = 450.0;
        }
        this->ensure_planet_item_slot(PLANET_NOCTARIS_PRIME, ORE_CRYSTAL);
        this->add_ore(PLANET_NOCTARIS_PRIME, ORE_CRYSTAL, 2);
        entry = ft_string("Captain Blackthorne hails encoded rebel broadcasts linking hidden relays without tipping Dominion scouts.");
        this->unlock_journal_entry(JOURNAL_ENTRY_SIDE_REBELLION_BROADCAST,
            ft_string("Journal – Rebel Signal Uprising: Secret transmissions ignite uprisings from mine shafts to orbital farms, each message promising convoys will be the rebellion's lifeline."));
    }
    if (entry.size() > 0)
        this->append_lore_entry(entry);
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
    else if (quest_id == QUEST_SIDE_CONVOY_RESCUE)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_MARS);
        if (route != ft_nullptr)
            this->modify_route_threat(*route, 0.5, true);
        entry = ft_string("Old Miner Joe mourns that the relief convoy never reached the stranded habitats.");
    }
    else if (quest_id == QUEST_SIDE_OUTPOST_REPAIR)
    {
        this->sub_ore(PLANET_MARS, ITEM_ENGINE_PART, 1);
        entry = ft_string("Professor Lumen reports that redline outposts remain dark without the promised repairs.");
    }
    else if (quest_id == QUEST_SIDE_ORDER_PROPAGANDA)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_MARS);
        if (route != ft_nullptr)
            this->modify_route_threat(*route, 0.6, true);
        entry = ft_string("Marshal Rhea fumes as loyalist broadcasts fail to quiet the raider chatter.");
    }
    else if (quest_id == QUEST_SIDE_REBELLION_BROADCAST)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
        if (route != ft_nullptr)
            this->modify_route_threat(*route, 0.5, true);
        entry = ft_string("Captain Blackthorne laments that rebel relays went silent before the uprising could spark.");
    }
    if (entry.size() > 0)
        this->append_lore_entry(entry);
}

void Game::handle_quest_choice_prompt(int quest_id)
{
    if (quest_id == QUEST_CRITICAL_DECISION)
    {
        ft_string entry("Navigator Zara's sacrifice forces a reckoning over Captain Blackthorne's fate.");
        this->append_lore_entry(entry);
        this->unlock_journal_entry(JOURNAL_ENTRY_CLIMACTIC_BATTLE,
            ft_string("Journal – Climactic Battle: Navigator Zara gives everything to shield the fleet, leaving command to decide whether Blackthorne lives or dies."));
    }
    else if (quest_id == QUEST_ORDER_FINAL_VERDICT)
    {
        ft_string entry("Tribunal screens flicker as the Order convenes to determine how rebels will be judged.");
        this->append_lore_entry(entry);
        this->unlock_journal_entry(JOURNAL_ENTRY_ORDER_FINAL_MANDATE,
            ft_string("Journal – Final Order Mandate: Marshal Rhea demands a verdict on the captured rebels, forcing loyalists to choose between fear and reform."));
    }
}

void Game::handle_quest_choice_resolution(int quest_id, int choice_id)
{
    ft_string entry;
    if (quest_id == QUEST_CRITICAL_DECISION)
    {
        if (choice_id == QUEST_CHOICE_EXECUTE_BLACKTHORNE)
        {
            this->ensure_planet_item_slot(PLANET_TERRA, ORE_COAL);
            this->add_ore(PLANET_TERRA, ORE_COAL, 5);
            entry = ft_string("Captain Blackthorne's execution steels Terra's loyalists; coal stockpiles surge for the war effort.");
            this->unlock_journal_entry(JOURNAL_ENTRY_DECISION_EXECUTE_BLACKTHORNE,
                ft_string("Journal – The Critical Decision: Executing Blackthorne satisfies hardliners but risks martyring him for every colonist who ever felt abandoned."));
        }
        else if (choice_id == QUEST_CHOICE_SPARE_BLACKTHORNE)
        {
            this->ensure_planet_item_slot(PLANET_TERRA, ORE_CRYSTAL);
            this->add_ore(PLANET_TERRA, ORE_CRYSTAL, 3);
            entry = ft_string("Sparing Blackthorne yields encoded crystal data that Professor Lumen studies for hidden conspiracies.");
            this->unlock_journal_entry(JOURNAL_ENTRY_DECISION_SPARE_BLACKTHORNE,
                ft_string("Journal – The Critical Decision: Sparing Blackthorne opens the door to expose corruption, even as loyalists whisper that the commander has gone soft."));
        }
    }
    else if (quest_id == QUEST_ORDER_FINAL_VERDICT)
    {
        if (choice_id == QUEST_CHOICE_ORDER_EXECUTE_REBELS)
        {
            ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_MARS);
            if (route != ft_nullptr)
            {
                double reduction = route->threat_level;
                if (reduction > 0.0)
                    this->modify_route_threat(*route, -reduction, false);
                route->quiet_timer += 240.0;
                if (route->quiet_timer > 720.0)
                    route->quiet_timer = 720.0;
            }
            this->ensure_planet_item_slot(PLANET_TERRA, ORE_TITANIUM);
            this->add_ore(PLANET_TERRA, ORE_TITANIUM, 2);
            entry = ft_string("Executions broadcast across the belt; Order patrols lock down lanes as titanium armor is rushed to the garrisons.");
            this->unlock_journal_entry(JOURNAL_ENTRY_ORDER_VERDICT_EXECUTION,
                ft_string("Journal – Ironclad Verdict: The Order purges the rebels, tightening convoys with fear even as whispers of martyrdom ripple through the colonies."));
        }
        else if (choice_id == QUEST_CHOICE_ORDER_TRIAL_REBELS)
        {
            ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
            if (route != ft_nullptr)
                this->modify_route_threat(*route, 0.35, true);
            this->ensure_planet_item_slot(PLANET_TERRA, ORE_CRYSTAL);
            this->add_ore(PLANET_TERRA, ORE_CRYSTAL, 2);
            entry = ft_string("Tribunals promise reform; crystal data archives flow to the public while sympathizers rally cautiously.");
            this->unlock_journal_entry(JOURNAL_ENTRY_ORDER_VERDICT_REFORM,
                ft_string("Journal – Trials for Tomorrow: Public hearings spare the rebels, trading short-term unrest for a fragile hope that the Order can change."));
        }
    }
    if (entry.size() > 0)
        this->append_lore_entry(entry);
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
    if (!this->_lore_log_cache_dirty)
        return this->_lore_log_cache;

    this->_lore_log_cache.clear();
    if (this->_lore_log_count == 0)
    {
        this->_lore_log_cache_dirty = false;
        return this->_lore_log_cache;
    }

    size_t buffer_size = this->_lore_log.size();
    if (buffer_size == 0)
    {
        this->_lore_log_cache_dirty = false;
        return this->_lore_log_cache;
    }

    this->_lore_log_cache.reserve(this->_lore_log_count);
    for (size_t i = 0; i < this->_lore_log_count; ++i)
    {
        size_t index = (this->_lore_log_start + i) % buffer_size;
        this->_lore_log_cache.push_back(this->_lore_log[index]);
    }
    this->_lore_log_cache_dirty = false;
    return this->_lore_log_cache;
}

void Game::get_quest_log_snapshot(ft_quest_log_snapshot &out) const
{
    out.main_quests.clear();
    out.side_quests.clear();
    out.awaiting_choice_ids.clear();
    out.recent_journal_entries.clear();
    out.active_main_quest_id = this->get_active_quest();

    ft_quest_context context;
    this->build_quest_context(context);

    ft_vector<Pair<int, ft_sharedptr<ft_quest_definition> > > definitions;
    this->_quests.snapshot_definitions(definitions);
    size_t definition_count = definitions.size();
    for (size_t i = 0; i < definition_count; ++i)
    {
        const ft_sharedptr<ft_quest_definition> &definition_ptr = definitions[i].value;
        if (!definition_ptr)
            continue;
        const ft_quest_definition &definition = *definition_ptr;

        ft_quest_log_entry entry;
        entry.quest_id = definition.id;
        entry.name = definition.name;
        entry.description = definition.description;
        entry.status = this->_quests.get_status(definition.id);
        entry.time_remaining = this->_quests.get_time_remaining(definition.id);
        if (definition.time_limit > 0.0)
            entry.time_limit = definition.time_limit * this->_quest_time_scale;
        else
            entry.time_limit = 0.0;
        entry.is_side_quest = definition.is_side_quest;
        entry.requires_choice = definition.requires_choice;
        entry.awaiting_choice = (entry.status == QUEST_STATUS_AWAITING_CHOICE);
        entry.selected_choice = this->_quests.get_choice(definition.id);

        entry.prerequisites_met = true;
        for (size_t j = 0; j < definition.prerequisites.size(); ++j)
        {
            int prerequisite_id = definition.prerequisites[j];
            if (this->_quests.get_status(prerequisite_id) != QUEST_STATUS_COMPLETED)
            {
                entry.prerequisites_met = false;
                break;
            }
        }

        entry.branch_requirement_met = true;
        if (definition.required_choice_quest != 0)
        {
            int required_choice = this->_quests.get_choice(definition.required_choice_quest);
            entry.branch_requirement_met = (required_choice == definition.required_choice_value);
        }

        entry.objectives_completed = true;
        for (size_t j = 0; j < definition.objectives.size(); ++j)
        {
            const ft_quest_objective &objective = definition.objectives[j];
            ft_quest_objective_snapshot snapshot;
            snapshot.type = objective.type;
            snapshot.target_id = objective.target_id;
            snapshot.required_amount = objective.amount;
            snapshot.current_amount = get_objective_current_amount(objective, context);
            snapshot.is_met = is_objective_met_for_snapshot(objective, context);
            if (!snapshot.is_met)
                entry.objectives_completed = false;
            entry.objectives.push_back(snapshot);
        }

        for (size_t j = 0; j < definition.choices.size(); ++j)
        {
            const ft_quest_choice_definition &choice_definition = definition.choices[j];
            ft_quest_choice_snapshot choice_entry;
            choice_entry.choice_id = choice_definition.choice_id;
            choice_entry.description = choice_definition.description;
            choice_entry.is_selected = (entry.selected_choice == choice_definition.choice_id);
            choice_entry.is_available = entry.awaiting_choice;
            entry.choices.push_back(choice_entry);
        }

        if (entry.awaiting_choice)
            out.awaiting_choice_ids.push_back(entry.quest_id);

        if (definition.is_side_quest)
            out.side_quests.push_back(entry);
        else
            out.main_quests.push_back(entry);
    }

    const ft_vector<ft_string> &journal_entries = this->get_journal_entries();
    size_t journal_count = journal_entries.size();
    size_t max_recent = 5;
    size_t start_index = 0;
    if (journal_count > max_recent)
        start_index = journal_count - max_recent;
    for (size_t i = start_index; i < journal_count; ++i)
        out.recent_journal_entries.push_back(journal_entries[i]);
}

bool Game::is_backend_online() const
{
    return this->_backend_online;
}

long Game::get_backend_retry_delay_ms_for_testing() const
{
    return this->_backend_retry_delay_ms;
}

long Game::get_backend_next_retry_ms_for_testing() const
{
    return this->_backend_next_retry_ms;
}

