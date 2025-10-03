#include "game.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/Math/math.hpp"
#include "libft/Template/map.hpp"
#include "libft/Template/pair.hpp"
#include "libft/RNG/rng.hpp"
#include "ft_map_snapshot.hpp"

namespace
{
    struct ResourceLoreDefinition
    {
        int                 resource_id;
        const char *const   *lines;
        size_t              count;
        int                 journal_entry_id;
        const char          *journal_text;
    };

    static bool route_passes_celestial_barrens(int origin_planet_id, int destination_planet_id)
    {
        return (origin_planet_id == PLANET_MARS && destination_planet_id == PLANET_ZALTHOR)
            || (origin_planet_id == PLANET_ZALTHOR && destination_planet_id == PLANET_MARS);
    }

    static bool route_touches_nebula_outpost(int origin_planet_id, int destination_planet_id)
    {
        return origin_planet_id == PLANET_NOCTARIS_PRIME
            || destination_planet_id == PLANET_NOCTARIS_PRIME;
    }

    static const char *resolve_resource_name(int resource_id)
    {
        switch (resource_id)
        {
        case ORE_GOLD:
            return "gold";
        case ORE_CRYSTAL:
            return "crystal";
        case ORE_TRITIUM:
            return "tritium";
        case ORE_OBSIDIAN:
            return "obsidian";
        default:
            return ft_nullptr;
        }
    }

    static const char *const kIronLore[] = {
        "Old Miner Joe notes Terra's iron shipments forge the plating that keeps the core alive.",
        "Quartermaster Nia diverts iron into emergency barricades around Terra's perimeter."
    };

    static const char *const kCopperLore[] = {
        "Farmer Daisy barters copper coils for hydroponic stabilizers to keep the belt fed.",
        "Technicians lace copper conduits through Trade Relays, widening the convoy grid."
    };

    static const char *const kMithrilLore[] = {
        "Professor Lumen studies shimmering mithril shards for clues to the anomaly pulsing beyond Zalthor.",
        "Rebel smiths temper mithril into stealth armor promised to Blackthorne's scouts."
    };

    static const char *const kCoalLore[] = {
        "Terra's furnaces gulp coal shipments to feed the Order's defensive batteries.",
        "Freighter crews stash coal reserves to warm frozen outposts waiting for relief."
    };

    static const char *const kTitaniumLore[] = {
        "Engineer cadres hammer titanium plating into the bulwarks shielding the capital fleets.",
        "Old Scout Finn catalogs titanium caches earmarked for a future juggernaut hull."
    };

    static const char *const kObsidianLore[] = {
        "Obsidian shards refract resistance codes that pass quietly between rebel captains.",
        "Navigator Zara once charted obsidian-rich asteroids now guarded by sympathetic miners."
    };

    static const char *const kCrystalLore[] = {
        "Professor Lumen refracts crystal data-keys to trace corruption running through the bureaucracy.",
        "Crystal arrays power clandestine transmitters guiding refugees to safe corridors."
    };

    static const char *const kNanomaterialLore[] = {
        "Helios labs weave nanomaterial threads into repair drones that mend hull fractures mid-flight.",
        "Experimental nanomaterial vats promise to double shield regeneration for the liberation push."
    };

    static const ResourceLoreDefinition kResourceLoreDefinitions[] = {
        {ORE_IRON, kIronLore, sizeof(kIronLore) / sizeof(kIronLore[0]),
            JOURNAL_ENTRY_RESOURCE_IRON_FOUNDATION,
            "Journal – Iron Lifeblood: Terra forges hull plating, stabilizer beams, and refuge domes from every ingot we deliver. Old Miner Joe reminds the crews that without iron the core would fall silent."},
        {ORE_COPPER, kCopperLore, sizeof(kCopperLore) / sizeof(kCopperLore[0]),
            JOURNAL_ENTRY_RESOURCE_COPPER_NETWORK,
            "Journal – Copper Signal Web: Copper coils braid together relay stations that let convoys whisper across the void. Farmer Daisy says each shipment keeps the hydroponics grid singing."},
        {ORE_MITHRIL, kMithrilLore, sizeof(kMithrilLore) / sizeof(kMithrilLore[0]),
            JOURNAL_ENTRY_RESOURCE_MITHRIL_MYSTERIES,
            "Journal – Mithril Mysteries: The shimmering alloy refracts the Zalthor anomaly, and Professor Lumen believes it hides coordinates to the Dominion's darkest experiments."},
        {ORE_COAL, kCoalLore, sizeof(kCoalLore) / sizeof(kCoalLore[0]),
            JOURNAL_ENTRY_RESOURCE_COAL_BARRICADES,
            "Journal – Coal-Fired Barricades: Loyalist batteries gulp coal to hold the defense grid, while miners stash extra to warm refugees fleeing the raids."},
        {ORE_TITANIUM, kTitaniumLore, sizeof(kTitaniumLore) / sizeof(kTitaniumLore[0]),
            JOURNAL_ENTRY_RESOURCE_TITANIUM_BULWARK,
            "Journal – Titanium Bulwark: Each crate of titanium becomes reinforced prows for capital cruisers and armor plates for the militias guarding Mars."},
        {ORE_OBSIDIAN, kObsidianLore, sizeof(kObsidianLore) / sizeof(kObsidianLore[0]),
            JOURNAL_ENTRY_RESOURCE_OBSIDIAN_ALLIANCE,
            "Journal – Obsidian Alliance: Rebel artisans etch promises into obsidian, sealing pacts between hidden enclaves and Blackthorne's captains."},
        {ORE_CRYSTAL, kCrystalLore, sizeof(kCrystalLore) / sizeof(kCrystalLore[0]),
            JOURNAL_ENTRY_RESOURCE_CRYSTAL_INTRIGUE,
            "Journal – Crystal Intrigue: Encoded crystal shards carry data caches exposing Dominion betrayal, and the Convoy Corps smuggles them behind enemy blockades."},
        {ORE_NANOMATERIAL, kNanomaterialLore, sizeof(kNanomaterialLore) / sizeof(kNanomaterialLore[0]),
            JOURNAL_ENTRY_RESOURCE_NANOMATERIAL_RENEWAL,
            "Journal – Nanomaterial Renewal: Repair drones weave nanothreads into cracked hulls mid-battle, giving battered fleets a second chance to fight."}
    };

    static const size_t kResourceLoreDefinitionCount = sizeof(kResourceLoreDefinitions) / sizeof(kResourceLoreDefinitions[0]);

    static const char *const kRaiderLoreSnippets[] = {
        "Old Scout Finn spots raider beacons flickering between dust-choked asteroids.",
        "Captain Blackthorne's lieutenants broadcast promises of liberation to every oppressed miner.",
        "Professor Lumen detects ion trails that match the raiders' latest ambush signatures.",
        "Navigator Zara's archived flight paths warn of slingshot maneuvers raiders exploit around the moons."
    };

    static const size_t kRaiderLoreSnippetCount = sizeof(kRaiderLoreSnippets) / sizeof(kRaiderLoreSnippets[0]);

    static const char *const kRaiderJournalEntryText =
        "Journal – Raider Signal Web: Scout Finn maps the beacons that stitch Blackthorne's fleet together, revealing how the rebellion rides hidden currents between the colonies.";

    static const char *resolve_planet_name(int planet_id)
    {
        switch (planet_id)
        {
        case PLANET_TERRA:
            return "Terra";
        case PLANET_MARS:
            return "Mars";
        case PLANET_ZALTHOR:
            return "Zalthor";
        case PLANET_VULCAN:
            return "Vulcan";
        case PLANET_NOCTARIS_PRIME:
            return "Noctaris Prime";
        case PLANET_LUNA:
            return "Luna";
        default:
            return ft_nullptr;
        }
    }

    static const ResourceLoreDefinition *find_resource_lore_definition(int resource_id)
    {
        for (size_t i = 0; i < kResourceLoreDefinitionCount; ++i)
        {
            if (kResourceLoreDefinitions[i].resource_id == resource_id)
                return &kResourceLoreDefinitions[i];
        }
        return ft_nullptr;
    }
}

static double preserve_contract_elapsed(double elapsed, double interval)
{
    if (interval <= 0.0)
        return 0.0;
    double remainder = elapsed - interval;
    if (remainder <= 0.0)
        return interval;
    double overflow = math_fmod(remainder, interval);
    if (overflow < 0.0)
        overflow += interval;
    return interval + overflow;
}

static const double ROUTE_ESCALATION_THRESHOLD = 5.0;
static const double ROUTE_ESCALATION_TRIGGER_TIME = 60.0;
static const double ROUTE_ESCALATION_DECAY_RATE = 1.0;

Game::RouteKey Game::compose_route_key(int origin, int destination) const
{
    return RouteKey(origin, destination);
}

Game::ft_supply_route *Game::ensure_supply_route(int origin, int destination)
{
    if (origin == destination)
        return ft_nullptr;
    RouteKey key = this->compose_route_key(origin, destination);
    Pair<RouteKey, ft_supply_route> *entry = this->_supply_routes.find(key);
    if (entry != ft_nullptr)
        return &entry->value;
    ft_supply_route route;
    route.id = this->_next_route_id++;
    route.origin_planet_id = origin;
    route.destination_planet_id = destination;
    route.base_travel_time = this->estimate_route_travel_time(origin, destination);
    route.escort_requirement = this->estimate_route_escort_requirement(origin, destination);
    route.base_raid_risk = this->estimate_route_raid_risk(origin, destination);
    route.threat_level = 0.0;
    route.quiet_timer = 0.0;
    route.escalation_timer = 0.0;
    route.escalation_pending = false;
    route.escalation_planet_id = 0;
    this->_supply_routes.insert(key, route);
    entry = this->_supply_routes.find(key);
    if (entry == ft_nullptr)
        return ft_nullptr;
    this->_route_lookup.insert(route.id, key);
    return &entry->value;
}

Game::ft_supply_route *Game::find_supply_route(int origin, int destination)
{
    if (origin == destination)
        return ft_nullptr;
    RouteKey key = this->compose_route_key(origin, destination);
    Pair<RouteKey, ft_supply_route> *entry = this->_supply_routes.find(key);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
}

const Game::ft_supply_route *Game::find_supply_route(int origin, int destination) const
{
    if (origin == destination)
        return ft_nullptr;
    RouteKey key = this->compose_route_key(origin, destination);
    const Pair<RouteKey, ft_supply_route> *entry = this->_supply_routes.find(key);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
}

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
    entry.append(ft_string(" before fresh meteor storms close the corridor."));
    this->append_lore_entry(entry);
    this->append_resource_lore_snippet(salvage_resource, convoy.origin_planet_id, convoy.destination_planet_id);
    this->modify_route_threat(*route, 0.35, true);
    this->maybe_unlock_imperium_pressure(*route);
    if (!this->is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_CELESTIAL_BARRENS))
    {
        ft_string journal_text("Journal – Celestial Barrens: Between Mars and Zalthor, shattered meteors hide rare isotopes. Finn's charts keep convoys alive while raiders and Imperium agents chase the same treasure through lethal debris fields.");
        this->unlock_journal_entry(JOURNAL_ENTRY_LORE_CELESTIAL_BARRENS, journal_text);
    }
    return true;
}

bool Game::handle_nebula_outpost_scan(const ft_supply_convoy &convoy)
{
    if (!route_touches_nebula_outpost(convoy.origin_planet_id, convoy.destination_planet_id))
        return false;
    ft_supply_route *route = this->get_route_by_id(convoy.route_id);
    if (!route)
        return false;
    double chance = 0.16;
    if (route->threat_level > 2.0)
    {
        double bonus = (route->threat_level - 2.0) * 0.04;
        if (bonus > 0.12)
            bonus = 0.12;
        chance += bonus;
    }
    if (convoy.escort_rating > 0)
        chance += 0.04;
    if (chance > 0.5)
        chance = 0.5;
    double roll = static_cast<double>(ft_random_float());
    if (roll >= chance)
        return false;
    double threat_reduction = 0.7 + route->threat_level * 0.15;
    if (threat_reduction > 2.0)
        threat_reduction = 2.0;
    this->modify_route_threat(*route, -threat_reduction, true);
    this->accelerate_contract(convoy.contract_id, 0.1);
    ft_string entry("Outpost Nebula-X sweeps the convoy lane with aurora-lit sensors. Professor Lumen decodes the telemetry, peeling back raider ambushes while liberation cells whisper across the anomaly.");
    this->append_lore_entry(entry);
    if (!this->is_journal_entry_unlocked(JOURNAL_ENTRY_LORE_NEBULA_OUTPOST))
    {
        ft_string journal_text("Journal – Outpost Nebula-X: The frontier research bastion watches over Noctaris Prime, sharing anomaly scans that warn of raids and Imperium probes alike. Its signals are the lifeline for every convoy brave enough to cross the nebula.");
        this->unlock_journal_entry(JOURNAL_ENTRY_LORE_NEBULA_OUTPOST, journal_text);
    }
    return true;
}

void Game::update_route_escalation(ft_supply_route &route, double seconds)
{
    if (seconds <= 0.0)
        return ;
    bool origin_assault = this->_combat.is_assault_active(route.origin_planet_id);
    bool destination_assault = this->_combat.is_assault_active(route.destination_planet_id);
    if (origin_assault && destination_assault)
    {
        route.escalation_timer = 0.0;
        route.escalation_pending = false;
        route.escalation_planet_id = 0;
        return ;
    }
    bool origin_available = this->is_planet_unlocked(route.origin_planet_id) && !origin_assault;
    bool destination_available = this->is_planet_unlocked(route.destination_planet_id) && !destination_assault;
    if (destination_available)
        route.escalation_planet_id = route.destination_planet_id;
    else if (origin_available)
        route.escalation_planet_id = route.origin_planet_id;
    else if (!route.escalation_pending)
        route.escalation_planet_id = 0;
    bool threat_high = (route.threat_level >= ROUTE_ESCALATION_THRESHOLD);
    if (threat_high && route.escalation_planet_id != 0)
    {
        route.escalation_timer += seconds;
        if (route.escalation_timer > ROUTE_ESCALATION_TRIGGER_TIME)
            route.escalation_timer = ROUTE_ESCALATION_TRIGGER_TIME;
        if (!route.escalation_pending && route.escalation_timer >= ROUTE_ESCALATION_TRIGGER_TIME)
            route.escalation_pending = true;
    }
    else
    {
        if (route.escalation_timer > 0.0)
        {
            route.escalation_timer -= seconds * ROUTE_ESCALATION_DECAY_RATE;
            if (route.escalation_timer < 0.0)
                route.escalation_timer = 0.0;
        }
        if (route.escalation_timer <= 0.0)
        {
            route.escalation_pending = false;
            if (!threat_high)
                route.escalation_planet_id = 0;
        }
    }
    if (route.escalation_pending)
        this->trigger_route_assault(route);
}

void Game::trigger_route_assault(ft_supply_route &route)
{
    if (!route.escalation_pending)
        return ;
    int planet_id = route.escalation_planet_id;
    if (planet_id == 0)
        return ;
    if (!this->is_planet_unlocked(planet_id))
        return ;
    if (this->_combat.is_assault_active(planet_id))
        return ;
    double difficulty = 1.0 + (route.threat_level * 0.12);
    if (difficulty < 0.8)
        difficulty = 0.8;
    if (!this->start_raider_assault(planet_id, difficulty))
        return ;
    ft_string entry("Sustained raids along convoy route ");
    entry.append(ft_to_string(route.origin_planet_id));
    entry.append(ft_string(" -> "));
    entry.append(ft_to_string(route.destination_planet_id));
    entry.append(ft_string(" escalate into a direct assault on planet "));
    entry.append(ft_to_string(planet_id));
    entry.append(ft_string("."));
    this->append_lore_entry(entry);
    double relief = route.threat_level - (ROUTE_ESCALATION_THRESHOLD - 1.0);
    if (relief < 2.5)
        relief = 2.5;
    if (relief > route.threat_level)
        relief = route.threat_level;
    this->modify_route_threat(route, -relief, true);
    route.escalation_timer = 0.0;
    route.escalation_pending = false;
    route.escalation_planet_id = 0;
}

void Game::trigger_branch_assault(int planet_id, double difficulty, bool order_branch)
{
    if (planet_id == 0)
        return ;
    if (!this->is_planet_unlocked(planet_id))
        return ;
    if (this->_combat.is_assault_active(planet_id))
        return ;
    if (!this->start_raider_assault(planet_id, difficulty))
        return ;
    if (order_branch)
        this->_order_branch_pending_assault = planet_id;
    else
        this->_rebellion_branch_pending_assault = planet_id;
    ft_string entry;
    if (order_branch)
        entry = ft_string("Marshal Rhea orders a purge strike on planet ");
    else
        entry = ft_string("Captain Blackthorne rallies the liberation strike on planet ");
    entry.append(ft_to_string(planet_id));
    entry.append(ft_string("."));
    this->append_lore_entry(entry);
}

int Game::count_active_convoys_for_contract(int contract_id) const
{
    if (contract_id <= 0)
        return 0;
    size_t count = this->_active_convoys.size();
    if (count == 0)
        return 0;
    const Pair<int, ft_supply_convoy> *entries = this->_active_convoys.end();
    entries -= count;
    int active = 0;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_supply_convoy &convoy = entries[i].value;
        if (convoy.contract_id == contract_id)
            active += 1;
    }
    return active;
}

bool Game::has_active_convoy_for_contract(int contract_id) const
{
    if (contract_id <= 0)
        return false;
    const Pair<int, ft_supply_contract> *contract_entry = this->_supply_contracts.find(contract_id);
    if (contract_entry == ft_nullptr)
        return false;
    int limit = contract_entry->value.max_active_convoys;
    if (limit < 1)
        limit = 1;
    int active = this->count_active_convoys_for_contract(contract_id);
    return active >= limit;
}

void Game::increase_contract_pending_delivery(int contract_id, int amount)
{
    if (contract_id <= 0 || amount <= 0)
        return ;
    Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract_id);
    if (entry == ft_nullptr)
        return ;
    ft_supply_contract &contract = entry->value;
    if (contract.pending_delivery < 0)
        contract.pending_delivery = 0;
    contract.pending_delivery += amount;
}

void Game::decrease_contract_pending_delivery(int contract_id, int amount)
{
    if (contract_id <= 0 || amount <= 0)
        return ;
    Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract_id);
    if (entry == ft_nullptr)
        return ;
    ft_supply_contract &contract = entry->value;
    contract.pending_delivery -= amount;
    if (contract.pending_delivery < 0)
        contract.pending_delivery = 0;
}

void Game::accelerate_contract(int contract_id, double fraction)
{
    if (contract_id <= 0 || fraction <= 0.0)
        return ;
    Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract_id);
    if (entry == ft_nullptr)
        return ;
    ft_supply_contract &contract = entry->value;
    if (contract.interval_seconds <= 0.0)
        return ;
    double bonus = contract.interval_seconds * fraction;
    if (bonus < 0.0)
        bonus = 0.0;
    contract.elapsed_seconds += bonus;
    if (contract.elapsed_seconds > contract.interval_seconds)
        contract.elapsed_seconds = contract.interval_seconds;
    if (contract.elapsed_seconds < 0.0)
        contract.elapsed_seconds = 0.0;
}

bool Game::assign_convoy_escort(int origin_planet_id, int destination_planet_id, int fleet_id)
{
    if (origin_planet_id == destination_planet_id)
        return false;
    if (fleet_id <= 0)
        return false;
    ft_supply_route *route = this->ensure_supply_route(origin_planet_id, destination_planet_id);
    if (!route)
        return false;
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return false;
    if (this->is_fleet_escorting_convoy(fleet_id))
        return false;
    ft_location location = fleet->get_location();
    if (location.type != LOCATION_PLANET || location.from != origin_planet_id)
        return false;
    size_t assignment_count = this->_route_convoy_escorts.size();
    if (assignment_count > 0)
    {
        Pair<int, int> *assignments = this->_route_convoy_escorts.end();
        assignments -= assignment_count;
        for (size_t i = 0; i < assignment_count; ++i)
        {
            if (assignments[i].value == fleet_id && assignments[i].key != route->id)
                return false;
        }
    }
    Pair<int, int> *existing = this->_route_convoy_escorts.find(route->id);
    if (existing != ft_nullptr)
        existing->value = fleet_id;
    else
        this->_route_convoy_escorts.insert(route->id, fleet_id);
    return true;
}

bool Game::clear_convoy_escort(int origin_planet_id, int destination_planet_id)
{
    if (origin_planet_id == destination_planet_id)
        return false;
    const ft_supply_route *route = this->find_supply_route(origin_planet_id, destination_planet_id);
    if (!route)
        return false;
    Pair<int, int> *entry = this->_route_convoy_escorts.find(route->id);
    if (entry == ft_nullptr)
        return false;
    this->_route_convoy_escorts.remove(route->id);
    return true;
}

int Game::get_assigned_convoy_escort(int origin_planet_id, int destination_planet_id) const
{
    if (origin_planet_id == destination_planet_id)
        return 0;
    const ft_supply_route *route = this->find_supply_route(origin_planet_id, destination_planet_id);
    if (!route)
        return 0;
    const Pair<int, int> *entry = this->_route_convoy_escorts.find(route->id);
    if (entry == ft_nullptr)
        return 0;
    return entry->value;
}

int Game::dispatch_convoy(const ft_supply_route &route, int origin_planet_id,
                          int destination_planet_id, int resource_id,
                          int amount, int contract_id, int escort_fleet_id)
{
    if (amount <= 0)
        return 0;
    ft_supply_convoy convoy;
    convoy.id = this->_next_convoy_id++;
    convoy.route_id = route.id;
    convoy.contract_id = contract_id;
    convoy.origin_planet_id = origin_planet_id;
    convoy.destination_planet_id = destination_planet_id;
    convoy.resource_id = resource_id;
    convoy.amount = amount;
    convoy.origin_escort = this->calculate_planet_escort_rating(origin_planet_id);
    convoy.destination_escort = this->calculate_planet_escort_rating(destination_planet_id);
    int claimed_escort_id = 0;
    bool escort_was_claimed = false;
    convoy.route_escort_claimed = false;
    if (escort_fleet_id <= 0)
    {
        claimed_escort_id = this->claim_route_escort(route.id);
        if (claimed_escort_id > 0)
        {
            escort_fleet_id = claimed_escort_id;
            escort_was_claimed = true;
        }
    }
    if (escort_fleet_id > 0)
    {
        ft_sharedptr<ft_fleet> escort = this->get_fleet(escort_fleet_id);
        bool valid = true;
        if (!escort)
            valid = false;
        if (valid)
        {
            if (this->is_fleet_escorting_convoy(escort_fleet_id))
                valid = false;
        }
        if (valid)
        {
            ft_location escort_location = escort->get_location();
            if (escort_location.type != LOCATION_PLANET || escort_location.from != origin_planet_id)
                valid = false;
        }
        if (valid)
        {
            convoy.escort_fleet_id = escort_fleet_id;
            convoy.escort_rating = this->calculate_fleet_escort_rating(*escort);
            if (escort_was_claimed)
                convoy.route_escort_claimed = true;
        }
        else if (escort_was_claimed)
        {
            this->_route_convoy_escorts.remove(route.id);
        }
    }
    int effective_origin = convoy.origin_escort + convoy.escort_rating;
    if (effective_origin > 48)
        effective_origin = 48;
    int effective_destination = convoy.destination_escort + convoy.escort_rating;
    if (effective_destination > 48)
        effective_destination = 48;
    convoy.remaining_time = this->calculate_convoy_travel_time(route, effective_origin, effective_destination);
    if (convoy.escort_fleet_id > 0)
        this->set_fleet_location_travel(convoy.escort_fleet_id, origin_planet_id,
                                        destination_planet_id, convoy.remaining_time);
    this->_active_convoys.insert(convoy.id, convoy);
    ft_string entry("Quartermaster Nia dispatches a convoy from ");
    entry.append(ft_to_string(origin_planet_id));
    entry.append(ft_string(" to "));
    entry.append(ft_to_string(destination_planet_id));
    entry.append(ft_string(" carrying "));
    entry.append(ft_to_string(amount));
    entry.append(ft_string(" units"));
    if (contract_id > 0)
    {
        entry.append(ft_string(" (Contract #"));
        entry.append(ft_to_string(contract_id));
        entry.append(ft_string(")"));
    }
    entry.append(ft_string("."));
    if (convoy.escort_fleet_id > 0)
    {
        entry.append(ft_string(" Escort fleet #"));
        entry.append(ft_to_string(convoy.escort_fleet_id));
        entry.append(ft_string(" forms up for protection"));
        int veterancy_bonus = this->get_fleet_escort_veterancy_bonus(convoy.escort_fleet_id);
        if (veterancy_bonus > 0)
        {
            entry.append(ft_string(", bringing a +"));
            entry.append(ft_to_string(veterancy_bonus));
            entry.append(ft_string(" escort bonus from prior runs"));
        }
        entry.append(ft_string("."));
    }
    double origin_speed_bonus = this->_buildings.get_planet_convoy_speed_bonus(origin_planet_id);
    double destination_speed_bonus = this->_buildings.get_planet_convoy_speed_bonus(destination_planet_id);
    double origin_risk_modifier = this->_buildings.get_planet_convoy_raid_risk_modifier(origin_planet_id);
    double destination_risk_modifier = this->_buildings.get_planet_convoy_raid_risk_modifier(destination_planet_id);
    bool has_speed_bonus = (origin_speed_bonus > 0.0 || destination_speed_bonus > 0.0);
    bool has_risk_bonus = (origin_risk_modifier > 0.0 || destination_risk_modifier > 0.0);
    if (has_speed_bonus || has_risk_bonus)
    {
        entry.append(ft_string(" Trade relays "));
        if (has_speed_bonus && has_risk_bonus)
            entry.append(ft_string("accelerate this route and blunt raider odds"));
        else if (has_speed_bonus)
            entry.append(ft_string("accelerate this route"));
        else
            entry.append(ft_string("blunt raider odds"));
        entry.append(ft_string("."));
    }
    this->append_lore_entry(entry);
    return amount;
}

double Game::estimate_route_travel_time(int origin, int destination) const
{
    if (origin == destination)
        return 0.0;
    bool involves_terra = (origin == PLANET_TERRA || destination == PLANET_TERRA);
    bool involves_mars = (origin == PLANET_MARS || destination == PLANET_MARS);
    bool involves_luna = (origin == PLANET_LUNA || destination == PLANET_LUNA);
    bool involves_zalthor = (origin == PLANET_ZALTHOR || destination == PLANET_ZALTHOR);
    bool involves_vulcan = (origin == PLANET_VULCAN || destination == PLANET_VULCAN);
    bool involves_noctaris = (origin == PLANET_NOCTARIS_PRIME || destination == PLANET_NOCTARIS_PRIME);
    bool passes_barrens = route_passes_celestial_barrens(origin, destination);
    double base = 28.0;
    if (passes_barrens)
        base = 36.0;
    else if (involves_terra && involves_luna)
        base = 18.0;
    else if (involves_terra && involves_mars)
        base = 24.0;
    else if ((involves_mars && involves_luna) || (involves_terra && involves_zalthor))
        base = 32.0;
    else if (involves_terra && involves_vulcan)
        base = 40.0;
    else if (involves_vulcan && involves_noctaris)
        base = 44.0;
    else if (involves_terra && involves_noctaris)
        base = 52.0;
    else if (involves_noctaris)
        base = 48.0;
    else if (involves_vulcan)
        base = 38.0;
    else
        base = 34.0;
    if (involves_luna && involves_noctaris)
        base += 8.0;
    if (base < 16.0)
        base = 16.0;
    return base;
}

int Game::estimate_route_escort_requirement(int origin, int destination) const
{
    if (origin == destination)
        return 0;
    bool involves_luna = (origin == PLANET_LUNA || destination == PLANET_LUNA);
    bool involves_vulcan = (origin == PLANET_VULCAN || destination == PLANET_VULCAN);
    bool involves_noctaris = (origin == PLANET_NOCTARIS_PRIME || destination == PLANET_NOCTARIS_PRIME);
    bool passes_barrens = route_passes_celestial_barrens(origin, destination);
    if (involves_noctaris)
        return 6;
    if (passes_barrens)
        return 5;
    if (involves_vulcan)
        return 4;
    if (involves_luna)
        return 1;
    return 3;
}

double Game::estimate_route_raid_risk(int origin, int destination) const
{
    if (origin == destination)
        return 0.0;
    double risk = 0.014;
    bool involves_vulcan = (origin == PLANET_VULCAN || destination == PLANET_VULCAN);
    bool involves_noctaris = (origin == PLANET_NOCTARIS_PRIME || destination == PLANET_NOCTARIS_PRIME);
    bool involves_luna = (origin == PLANET_LUNA || destination == PLANET_LUNA);
    bool passes_barrens = route_passes_celestial_barrens(origin, destination);
    if (involves_vulcan)
        risk += 0.006;
    if (involves_noctaris)
        risk += 0.012;
    if (passes_barrens)
        risk += 0.018;
    if (involves_luna)
        risk -= 0.004;
    if (risk < 0.006)
        risk = 0.006;
    return risk;
}

int Game::calculate_planet_escort_rating(int planet_id) const
{
    int rating = 0;
    ft_sharedptr<const ft_fleet> garrison = this->get_planet_fleet(planet_id);
    if (garrison)
        rating += this->calculate_fleet_escort_rating(*garrison);
    size_t fleet_count = this->_fleets.size();
    if (fleet_count > 0)
    {
        const Pair<int, ft_sharedptr<ft_fleet> > *entries = this->_fleets.end();
        entries -= fleet_count;
        for (size_t i = 0; i < fleet_count; ++i)
        {
            const ft_sharedptr<ft_fleet> &fleet = entries[i].value;
            if (!fleet)
                continue;
            if (this->is_fleet_escorting_convoy(fleet->get_id()))
                continue;
            ft_location loc = fleet->get_location();
            if (loc.type == LOCATION_PLANET && loc.from == planet_id)
                rating += this->calculate_fleet_escort_rating(*fleet);
        }
    }
    if (rating > 48)
        rating = 48;
    return rating;
}

int Game::calculate_fleet_escort_rating(const ft_fleet &fleet) const
{
    ft_vector<int> ship_ids;
    fleet.get_ship_ids(ship_ids);
    int rating = 0;
    for (size_t i = 0; i < ship_ids.size(); ++i)
    {
        int type = fleet.get_ship_type(ship_ids[i]);
        switch (type)
        {
        case SHIP_INTERCEPTOR:
            rating += 4;
            break;
        case SHIP_CORVETTE:
            rating += 3;
            break;
        case SHIP_FRIGATE_SOVEREIGN:
        case SHIP_FRIGATE_PREEMPTOR:
            rating += 6;
            break;
        case SHIP_FRIGATE_JUGGERNAUT:
            rating += 5;
            break;
        case SHIP_FRIGATE_CARRIER:
        case SHIP_FRIGATE_PROTECTOR:
        case SHIP_FRIGATE_ECLIPSE:
            rating += 4;
            break;
        case SHIP_CAPITAL_JUGGERNAUT:
        case SHIP_CAPITAL_NOVA:
        case SHIP_CAPITAL_OBSIDIAN:
        case SHIP_CAPITAL_PREEMPTOR:
        case SHIP_CAPITAL_PROTECTOR:
        case SHIP_CAPITAL_ECLIPSE:
            rating += 7;
            break;
        case SHIP_SUNFLARE_SLOOP:
            rating += 3;
            break;
        case SHIP_REPAIR_DRONE:
            rating += 2;
            break;
        case SHIP_SHIELD:
            rating += 2;
            break;
        case SHIP_RADAR:
            rating += 1;
            break;
        default:
            break;
        }
    }
    int veterancy_bonus = fleet.get_escort_veterancy_bonus();
    if (veterancy_bonus > 0)
        rating += veterancy_bonus;
    if (rating > 48)
        rating = 48;
    return rating;
}

bool Game::is_fleet_escorting_convoy(int fleet_id) const
{
    if (fleet_id <= 0)
        return false;
    size_t count = this->_active_convoys.size();
    if (count == 0)
        return false;
    const Pair<int, ft_supply_convoy> *entries = this->_active_convoys.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_supply_convoy &convoy = entries[i].value;
        if (convoy.escort_fleet_id == fleet_id)
            return true;
    }
    return false;
}

int Game::claim_route_escort(int route_id)
{
    Pair<int, int> *entry = this->_route_convoy_escorts.find(route_id);
    if (entry == ft_nullptr)
        return 0;
    int fleet_id = entry->value;
    this->_route_convoy_escorts.remove(route_id);
    return fleet_id;
}

double Game::calculate_convoy_travel_time(const ft_supply_route &route, int origin_escort, int destination_escort) const
{
    double time = route.base_travel_time;
    double base_floor = route.base_travel_time * 0.3;
    int strongest = origin_escort;
    if (destination_escort > strongest)
        strongest = destination_escort;
    if (route.escort_requirement > 0 && strongest > route.escort_requirement)
    {
        double surplus = static_cast<double>(strongest - route.escort_requirement);
        double reduction = surplus * 1.5;
        double minimum = route.base_travel_time * 0.5;
        time -= reduction;
        if (time < minimum)
            time = minimum;
    }
    double origin_bonus = this->_buildings.get_planet_convoy_speed_bonus(route.origin_planet_id);
    double destination_bonus = this->_buildings.get_planet_convoy_speed_bonus(route.destination_planet_id);
    double combined_bonus = origin_bonus + destination_bonus;
    if (combined_bonus > 0.6)
        combined_bonus = 0.6;
    if (combined_bonus > 0.0)
    {
        double multiplier = 1.0 - combined_bonus;
        if (multiplier < 0.3)
            multiplier = 0.3;
        time *= multiplier;
    }
    if (time < base_floor)
        time = base_floor;
    if (time < 10.0)
        time = 10.0;
    return time;
}

double Game::calculate_convoy_raid_risk(const ft_supply_convoy &convoy, bool origin_under_attack, bool destination_under_attack) const
{
    const ft_supply_route *route = this->get_route_by_id(convoy.route_id);
    if (!route)
        return 0.0;
    double risk = route->base_raid_risk;
    if (route->threat_level > 0.0)
    {
        double threat_factor = route->threat_level;
        if (threat_factor > 6.0)
            threat_factor = 6.0;
        double threat_multiplier = 1.0 + threat_factor * 0.35;
        if (threat_multiplier > 3.5)
            threat_multiplier = 3.5;
        risk *= threat_multiplier;
    }
    int effective_origin = convoy.origin_escort + convoy.escort_rating;
    if (effective_origin > 48)
        effective_origin = 48;
    int effective_destination = convoy.destination_escort + convoy.escort_rating;
    if (effective_destination > 48)
        effective_destination = 48;
    if (route->escort_requirement > 0)
    {
        if (effective_origin < route->escort_requirement)
        {
            double deficit = static_cast<double>(route->escort_requirement - effective_origin);
            risk *= 1.6;
            risk += deficit * 0.01;
        }
        else
        {
            double surplus = static_cast<double>(effective_origin - route->escort_requirement);
            double divisor = 1.0 + surplus * 0.12;
            if (divisor > 1.0)
                risk /= divisor;
        }
        if (effective_destination > route->escort_requirement)
        {
            double dest_surplus = static_cast<double>(effective_destination - route->escort_requirement);
            risk -= dest_surplus * 0.006;
        }
    }
    if (origin_under_attack)
        risk *= 1.8;
    if (destination_under_attack)
        risk *= 1.6;
    if (convoy.raided)
        risk *= 0.35;
    double origin_modifier = this->_buildings.get_planet_convoy_raid_risk_modifier(convoy.origin_planet_id);
    double destination_modifier = this->_buildings.get_planet_convoy_raid_risk_modifier(convoy.destination_planet_id);
    double combined_modifier = origin_modifier + destination_modifier;
    if (combined_modifier > 0.75)
        combined_modifier = 0.75;
    if (combined_modifier > 0.0)
    {
        double multiplier = 1.0 - combined_modifier;
        if (multiplier < 0.25)
            multiplier = 0.25;
        risk *= multiplier;
    }
    if (risk < 0.002)
        risk = 0.002;
    return risk;
}

void Game::handle_convoy_raid(ft_supply_convoy &convoy, bool origin_under_attack, bool destination_under_attack)
{
    ft_supply_route *route = this->get_route_by_id(convoy.route_id);
    if (!route)
        return ;
    this->modify_route_threat(*route, 0.45, true);
    double severity = 0.45;
    int effective_origin = convoy.origin_escort + convoy.escort_rating;
    if (effective_origin > 48)
        effective_origin = 48;
    int effective_destination = convoy.destination_escort + convoy.escort_rating;
    if (effective_destination > 48)
        effective_destination = 48;
    if (effective_origin < route->escort_requirement)
        severity += 0.25;
    if (effective_destination < route->escort_requirement)
        severity += 0.1;
    if (origin_under_attack || destination_under_attack)
    {
        severity += 0.3;
        if (effective_origin < route->escort_requirement)
            severity = 1.0;
    }
    if (severity > 1.0)
        severity = 1.0;
    if (severity < 0.15)
        severity = 0.15;
    int lost = static_cast<int>(static_cast<double>(convoy.amount) * severity);
    if (lost <= 0 && convoy.amount > 0)
        lost = 1;
    if (lost > convoy.amount)
        lost = convoy.amount;
    if (lost > 0)
        this->decrease_contract_pending_delivery(convoy.contract_id, lost);
    convoy.amount -= lost;
    convoy.raided = true;
    convoy.raid_meter = 0.0;
    if (convoy.amount <= 0)
    {
        convoy.amount = 0;
        convoy.destroyed = true;
        convoy.remaining_time = 0.0;
        if (!convoy.loss_recorded)
        {
            this->record_convoy_loss(convoy, true);
            convoy.loss_recorded = true;
        }
    }
    ft_string entry("Raiders ambushed a convoy from ");
    entry.append(ft_to_string(convoy.origin_planet_id));
    entry.append(ft_string(" to "));
    entry.append(ft_to_string(convoy.destination_planet_id));
    entry.append(ft_string("; "));
    entry.append(ft_to_string(lost));
    entry.append(ft_string(" units were lost"));
    if (convoy.destroyed)
    {
        if (convoy.escort_rating > 0 && convoy.escort_fleet_id > 0)
        {
            entry.append(ft_string(", and escort fleet #"));
            entry.append(ft_to_string(convoy.escort_fleet_id));
            entry.append(ft_string(" was overwhelmed."));
        }
        else
            entry.append(ft_string(", and without escorts the freighters were wiped out."));
    }
    else if (convoy.escort_rating > 0 && convoy.escort_fleet_id > 0)
    {
        entry.append(ft_string(", but escort fleet #"));
        entry.append(ft_to_string(convoy.escort_fleet_id));
        entry.append(ft_string(" drove the raiders off."));
    }
    else
        entry.append(ft_string(", and the defenseless freighters limped onward."));
    this->append_lore_entry(entry);
    this->append_raider_lore_snippet(convoy.origin_planet_id, convoy.destination_planet_id);
    this->accelerate_contract(convoy.contract_id, 0.5);
}

void Game::finalize_convoy(ft_supply_convoy &convoy)
{
    if (convoy.amount > 0)
        this->decrease_contract_pending_delivery(convoy.contract_id, convoy.amount);
    if (!convoy.destroyed && convoy.amount > 0)
    {
        this->record_convoy_delivery(convoy);
        ft_sharedptr<ft_planet> destination = this->get_planet(convoy.destination_planet_id);
        if (destination)
        {
            destination->add_resource(convoy.resource_id, convoy.amount);
            this->send_state(convoy.destination_planet_id, convoy.resource_id);
        }
        ft_string entry("A convoy from ");
        entry.append(ft_to_string(convoy.origin_planet_id));
        entry.append(ft_string(" delivered "));
        entry.append(ft_to_string(convoy.amount));
        entry.append(ft_string(" units to "));
        entry.append(ft_to_string(convoy.destination_planet_id));
        entry.append(ft_string("."));
        double origin_speed_bonus = this->_buildings.get_planet_convoy_speed_bonus(convoy.origin_planet_id);
        double destination_speed_bonus = this->_buildings.get_planet_convoy_speed_bonus(convoy.destination_planet_id);
        double origin_risk_modifier = this->_buildings.get_planet_convoy_raid_risk_modifier(convoy.origin_planet_id);
        double destination_risk_modifier = this->_buildings.get_planet_convoy_raid_risk_modifier(convoy.destination_planet_id);
        bool has_speed_bonus = (origin_speed_bonus > 0.0 || destination_speed_bonus > 0.0);
        bool has_risk_bonus = (origin_risk_modifier > 0.0 || destination_risk_modifier > 0.0);
        if (has_speed_bonus || has_risk_bonus)
        {
            entry.append(ft_string(" Trade relays "));
            if (has_speed_bonus && has_risk_bonus)
                entry.append(ft_string("shortened the journey and kept raiders at bay."));
            else if (has_speed_bonus)
                entry.append(ft_string("shortened the journey."));
            else
                entry.append(ft_string("kept raiders at bay."));
        }
        this->append_lore_entry(entry);
        this->append_resource_lore_snippet(convoy.resource_id, convoy.origin_planet_id, convoy.destination_planet_id);
        this->handle_celestial_barrens_salvage(convoy);
        this->handle_nebula_outpost_scan(convoy);
    }
    else
    {
        if (!convoy.loss_recorded)
        {
            this->record_convoy_loss(convoy, convoy.raided);
            convoy.loss_recorded = true;
        }
        ft_string entry("A convoy from ");
        entry.append(ft_to_string(convoy.origin_planet_id));
        entry.append(ft_string(" to "));
        entry.append(ft_to_string(convoy.destination_planet_id));
        entry.append(ft_string(" failed to arrive."));
        this->append_lore_entry(entry);
    }
    if (convoy.escort_fleet_id > 0)
    {
        ft_sharedptr<ft_fleet> escort = this->get_fleet(convoy.escort_fleet_id);
        if (escort)
        {
            if (convoy.destroyed)
                escort->set_location_planet(convoy.origin_planet_id);
            else
                escort->set_location_planet(convoy.destination_planet_id);
        }
    }
    bool restore_route_escort = false;
    if (convoy.route_escort_claimed && convoy.escort_fleet_id > 0 && !convoy.destroyed)
    {
        const ft_supply_route *route = this->get_route_by_id(convoy.route_id);
        if (route != ft_nullptr)
        {
            ft_sharedptr<ft_fleet> escort = this->get_fleet(convoy.escort_fleet_id);
            if (escort && escort->has_operational_ships())
            {
                ft_location escort_location = escort->get_location();
                if (escort_location.type == LOCATION_PLANET &&
                    (escort_location.from == convoy.origin_planet_id ||
                     escort_location.from == convoy.destination_planet_id))
                {
                    restore_route_escort = true;
                }
            }
        }
    }
    if (restore_route_escort)
    {
        Pair<int, int> *existing = this->_route_convoy_escorts.find(convoy.route_id);
        if (existing != ft_nullptr)
            existing->value = convoy.escort_fleet_id;
        else
            this->_route_convoy_escorts.insert(convoy.route_id, convoy.escort_fleet_id);
    }
    this->handle_contract_completion(convoy);
    convoy.escort_fleet_id = 0;
    convoy.escort_rating = 0;
}

void Game::handle_contract_completion(const ft_supply_convoy &convoy)
{
    if (convoy.contract_id <= 0)
        return ;
    if (convoy.destroyed)
    {
        this->accelerate_contract(convoy.contract_id, 1.0);
        return ;
    }
    if (convoy.raided)
        this->accelerate_contract(convoy.contract_id, 0.25);
}

void Game::process_supply_contracts(double seconds)
{
    if (seconds <= 0.0)
        return ;
    size_t contract_count = this->_supply_contracts.size();
    if (contract_count == 0)
        return ;
    Pair<int, ft_supply_contract> *entries = this->_supply_contracts.end();
    entries -= contract_count;
    for (size_t i = 0; i < contract_count; ++i)
    {
        ft_supply_contract &contract = entries[i].value;
        if (contract.interval_seconds <= 0.0)
            continue;
        if (contract.max_active_convoys < 1)
            contract.max_active_convoys = 1;
        contract.elapsed_seconds += seconds;
        if (contract.elapsed_seconds < 0.0)
            contract.elapsed_seconds = 0.0;
        bool dispatched = false;
        while (contract.elapsed_seconds >= contract.interval_seconds)
        {
            if (this->has_active_convoy_for_contract(contract.id))
            {
                contract.elapsed_seconds = preserve_contract_elapsed(contract.elapsed_seconds,
                                                                      contract.interval_seconds);
                break;
            }
            ft_sharedptr<ft_planet> origin = this->get_planet(contract.origin_planet_id);
            ft_sharedptr<ft_planet> destination = this->get_planet(contract.destination_planet_id);
            if (!origin || !destination)
            {
                contract.elapsed_seconds = contract.interval_seconds;
                break;
            }
            if (contract.has_minimum_stock)
            {
                int destination_stock = destination->get_resource(contract.resource_id);
                int projected_stock = destination_stock;
                if (contract.pending_delivery > 0)
                {
                    projected_stock += contract.pending_delivery;
                    if (projected_stock < destination_stock)
                        projected_stock = contract.minimum_stock;
                }
                if (projected_stock >= contract.minimum_stock)
                {
                    contract.elapsed_seconds = preserve_contract_elapsed(contract.elapsed_seconds,
                                                                          contract.interval_seconds);
                    break;
                }
            }
            int available = origin->get_resource(contract.resource_id);
            if (available <= 0)
            {
                contract.elapsed_seconds = contract.interval_seconds;
                break;
            }
            int shipment = contract.shipment_size;
            if (shipment <= 0)
            {
                contract.elapsed_seconds = 0.0;
                break;
            }
            if (shipment > available)
                shipment = available;
            ft_supply_route *route = this->ensure_supply_route(contract.origin_planet_id, contract.destination_planet_id);
            if (!route)
            {
                contract.elapsed_seconds = contract.interval_seconds;
                break;
            }
            origin->sub_resource(contract.resource_id, shipment);
            this->send_state(contract.origin_planet_id, contract.resource_id);
            this->ensure_planet_item_slot(contract.destination_planet_id, contract.resource_id);
            int dispatched_amount = this->dispatch_convoy(*route, contract.origin_planet_id,
                                                          contract.destination_planet_id,
                                                          contract.resource_id, shipment,
                                                          contract.id);
            if (dispatched_amount <= 0)
            {
                origin->add_resource(contract.resource_id, shipment);
                this->send_state(contract.origin_planet_id, contract.resource_id);
                contract.elapsed_seconds = contract.interval_seconds;
                break;
            }
            dispatched = true;
            this->increase_contract_pending_delivery(contract.id, dispatched_amount);
            contract.elapsed_seconds -= contract.interval_seconds;
            if (contract.elapsed_seconds < 0.0)
                contract.elapsed_seconds = 0.0;
            if (contract.elapsed_seconds > contract.interval_seconds)
                contract.elapsed_seconds = contract.interval_seconds;
        }
        if (!dispatched && contract.elapsed_seconds > contract.interval_seconds)
            contract.elapsed_seconds = contract.interval_seconds;
    }
}

void Game::advance_convoys(double seconds)
{
    if (seconds <= 0.0)
        return ;
    this->decay_all_route_threat(seconds);
    size_t count = this->_active_convoys.size();
    if (count > 0)
    {
        Pair<int, ft_supply_convoy> *entries = this->_active_convoys.end();
        entries -= count;
        ft_vector<int> completed;
        for (size_t i = 0; i < count; ++i)
        {
            ft_supply_convoy &convoy = entries[i].value;
            if (convoy.remaining_time <= 0.0)
            {
                completed.push_back(entries[i].key);
                continue;
            }
            bool origin_under_attack = this->_combat.is_assault_active(convoy.origin_planet_id);
            bool destination_under_attack = this->_combat.is_assault_active(convoy.destination_planet_id);
            if (!convoy.destroyed)
            {
                double risk = this->calculate_convoy_raid_risk(convoy, origin_under_attack, destination_under_attack);
                convoy.raid_meter += risk * seconds;
                if (convoy.raid_meter >= 1.0)
                {
                    while (convoy.raid_meter >= 1.0 && !convoy.destroyed)
                    {
                        double remainder = convoy.raid_meter - 1.0;
                        if (remainder < 0.0)
                            remainder = 0.0;
                        this->handle_convoy_raid(convoy, origin_under_attack, destination_under_attack);
                        if (convoy.destroyed)
                        {
                            completed.push_back(entries[i].key);
                            break;
                        }
                        convoy.raid_meter = remainder;
                    }
                    if (convoy.destroyed)
                        continue;
                }
            }
            convoy.remaining_time -= seconds;
            if (convoy.remaining_time <= 0.0)
                completed.push_back(entries[i].key);
        }
        for (size_t i = 0; i < completed.size(); ++i)
        {
            Pair<int, ft_supply_convoy> *entry = this->_active_convoys.find(completed[i]);
            if (entry == ft_nullptr)
                continue;
            this->finalize_convoy(entry->value);
            this->_active_convoys.remove(completed[i]);
        }
    }
    size_t route_count = this->_supply_routes.size();
    if (route_count > 0)
    {
        Pair<RouteKey, ft_supply_route> *routes = this->_supply_routes.end();
        routes -= route_count;
        for (size_t i = 0; i < route_count; ++i)
            this->update_route_escalation(routes[i].value, seconds);
    }
}

void Game::reset_delivery_streak()
{
    this->_current_delivery_streak = 0;
    this->_next_streak_milestone_index = 0;
}

void Game::record_convoy_delivery(const ft_supply_convoy &convoy)
{
    ft_supply_route *route = this->get_route_by_id(convoy.route_id);
    if (route)
    {
        double reduction = convoy.raided ? -0.6 : -0.9;
        this->modify_route_threat(*route, reduction, true);
    }
    this->_convoys_delivered_total += 1;
    this->_current_delivery_streak += 1;
    if (this->_current_delivery_streak > this->_longest_delivery_streak)
    {
        this->_longest_delivery_streak = this->_current_delivery_streak;
        ft_string record_entry("Quartermaster Nia records a new convoy streak of ");
        record_entry.append(ft_to_string(this->_current_delivery_streak));
        record_entry.append(ft_string(" successful deliveries."));
        this->append_lore_entry(record_entry);
    }
    while (this->_next_streak_milestone_index < this->_streak_milestones.size() &&
           this->_current_delivery_streak >= this->_streak_milestones[this->_next_streak_milestone_index])
    {
        int milestone = this->_streak_milestones[this->_next_streak_milestone_index];
        ft_string entry("Logistics crews celebrate ");
        entry.append(ft_to_string(milestone));
        entry.append(ft_string(" convoys arriving uninterrupted."));
        this->append_lore_entry(entry);
        this->_next_streak_milestone_index += 1;
    }
    this->record_achievement_event(ACHIEVEMENT_EVENT_CONVOY_DELIVERED, 1);
    this->record_achievement_event(ACHIEVEMENT_EVENT_CONVOY_STREAK_BEST, this->_longest_delivery_streak);
    if (convoy.escort_fleet_id > 0 && convoy.escort_rating > 0)
    {
        ft_sharedptr<ft_fleet> escort = this->get_fleet(convoy.escort_fleet_id);
        if (escort)
        {
            double xp_gain = 16.0;
            if (route)
                xp_gain += static_cast<double>(route->escort_requirement) * 3.0;
            if (route && route->threat_level > 0.0)
                xp_gain += route->threat_level * 1.5;
            if (convoy.raided)
                xp_gain += 12.0;
            if (xp_gain < 0.0)
                xp_gain = 0.0;
            if (escort->add_escort_veterancy(xp_gain))
            {
                int bonus = escort->get_escort_veterancy_bonus();
                ft_string veterancy_entry("Escort fleet #");
                veterancy_entry.append(ft_to_string(convoy.escort_fleet_id));
                veterancy_entry.append(ft_string(" hones its convoy tactics guarding route "));
                veterancy_entry.append(ft_to_string(convoy.origin_planet_id));
                veterancy_entry.append(ft_string(" -> "));
                veterancy_entry.append(ft_to_string(convoy.destination_planet_id));
                veterancy_entry.append(ft_string(", raising its escort rating bonus to +"));
                veterancy_entry.append(ft_to_string(bonus));
                if (convoy.raided)
                    veterancy_entry.append(ft_string(" after weathering a raid."));
                else
                    veterancy_entry.append(ft_string("."));
                this->append_lore_entry(veterancy_entry);
            }
        }
    }
}

void Game::record_convoy_loss(const ft_supply_convoy &convoy, bool destroyed_by_raid)
{
    ft_supply_route *route = this->get_route_by_id(convoy.route_id);
    if (route)
    {
        double delta = destroyed_by_raid ? 1.2 : 0.6;
        this->modify_route_threat(*route, delta, true);
    }
    bool had_escort = (convoy.escort_fleet_id > 0 && convoy.escort_rating > 0);
    if (destroyed_by_raid)
        this->_convoy_raid_losses += 1;
    if (this->_current_delivery_streak > 0)
    {
        ft_string streak_entry("Quartermaster Nia laments the end of a ");
        streak_entry.append(ft_to_string(this->_current_delivery_streak));
        streak_entry.append(ft_string(" convoy streak."));
        this->append_lore_entry(streak_entry);
    }
    this->reset_delivery_streak();
    if (destroyed_by_raid)
    {
        ft_string raid_entry("Professor Lumen tallies raid losses now at ");
        raid_entry.append(ft_to_string(this->_convoy_raid_losses));
        raid_entry.append(ft_string(" convoys."));
        if (had_escort)
        {
            raid_entry.append(ft_string(" Escort fleet #"));
            raid_entry.append(ft_to_string(convoy.escort_fleet_id));
            raid_entry.append(ft_string(" could not turn the tide."));
        }
        this->append_lore_entry(raid_entry);
    }
    else if (had_escort)
    {
        ft_string escort_entry("Escort fleet #");
        escort_entry.append(ft_to_string(convoy.escort_fleet_id));
        escort_entry.append(ft_string(" returns without its charge."));
        this->append_lore_entry(escort_entry);
    }
    if (had_escort)
    {
        ft_sharedptr<ft_fleet> escort = this->get_fleet(convoy.escort_fleet_id);
        if (escort)
        {
            double penalty = destroyed_by_raid ? 36.0 : 18.0;
            if (!destroyed_by_raid && convoy.raided)
                penalty += 8.0;
            if (penalty < 0.0)
                penalty = 0.0;
            if (escort->decay_escort_veterancy(penalty))
            {
                int bonus = escort->get_escort_veterancy_bonus();
                ft_string veterancy_entry("Escort fleet #");
                veterancy_entry.append(ft_to_string(convoy.escort_fleet_id));
                if (destroyed_by_raid)
                    veterancy_entry.append(ft_string(" loses convoy veterancy after its freighters were wiped out"));
                else
                    veterancy_entry.append(ft_string(" sheds convoy veterancy while regrouping from heavy losses"));
                if (bonus > 0)
                {
                    veterancy_entry.append(ft_string(", retaining a +"));
                    veterancy_entry.append(ft_to_string(bonus));
                    veterancy_entry.append(ft_string(" escort bonus."));
                }
                else
                    veterancy_entry.append(ft_string(", leaving no remaining escort bonus."));
                this->append_lore_entry(veterancy_entry);
            }
        }
    }
}

int Game::create_supply_contract(int origin_planet_id, int destination_planet_id,
                                 int resource_id, int shipment_size,
                                 double interval_seconds,
                                 int minimum_destination_stock,
                                 int max_active_convoys)
{
    if (origin_planet_id == destination_planet_id)
        return 0;
    if (shipment_size <= 0)
        return 0;
    if (interval_seconds <= 0.0)
        return 0;
    ft_sharedptr<ft_planet> origin = this->get_planet(origin_planet_id);
    ft_sharedptr<ft_planet> destination = this->get_planet(destination_planet_id);
    if (!origin || !destination)
        return 0;
    ft_supply_route *route = this->ensure_supply_route(origin_planet_id, destination_planet_id);
    if (!route)
        return 0;
    this->ensure_planet_item_slot(destination_planet_id, resource_id);
    ft_supply_contract contract;
    contract.id = this->_next_contract_id++;
    contract.origin_planet_id = origin_planet_id;
    contract.destination_planet_id = destination_planet_id;
    contract.resource_id = resource_id;
    contract.shipment_size = shipment_size;
    contract.interval_seconds = interval_seconds;
    contract.elapsed_seconds = 0.0;
    if (max_active_convoys < 1)
        max_active_convoys = 1;
    contract.max_active_convoys = max_active_convoys;
    contract.pending_delivery = 0;
    if (minimum_destination_stock >= 0)
    {
        contract.has_minimum_stock = true;
        contract.minimum_stock = minimum_destination_stock;
    }
    else
    {
        contract.has_minimum_stock = false;
        contract.minimum_stock = 0;
    }
    this->_supply_contracts.insert(contract.id, contract);
    Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract.id);
    if (entry == ft_nullptr)
        return 0;
    return contract.id;
}

bool Game::cancel_supply_contract(int contract_id)
{
    Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract_id);
    if (entry == ft_nullptr)
        return false;
    this->_supply_contracts.remove(contract_id);
    return true;
}

bool Game::update_supply_contract(int contract_id, int shipment_size,
                                  double interval_seconds,
                                  int minimum_destination_stock,
                                  int max_active_convoys)
{
    if (shipment_size <= 0)
        return false;
    if (interval_seconds <= 0.0)
        return false;
    Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract_id);
    if (entry == ft_nullptr)
        return false;
    ft_supply_contract &contract = entry->value;
    contract.shipment_size = shipment_size;
    contract.interval_seconds = interval_seconds;
    if (minimum_destination_stock >= 0)
    {
        contract.has_minimum_stock = true;
        contract.minimum_stock = minimum_destination_stock;
    }
    else
    {
        contract.has_minimum_stock = false;
        contract.minimum_stock = 0;
    }
    if (max_active_convoys >= 0)
    {
        if (max_active_convoys < 1)
            max_active_convoys = 1;
        contract.max_active_convoys = max_active_convoys;
    }
    if (contract.max_active_convoys < 1)
        contract.max_active_convoys = 1;
    if (contract.elapsed_seconds > contract.interval_seconds)
        contract.elapsed_seconds = contract.interval_seconds;
    if (contract.elapsed_seconds < 0.0)
        contract.elapsed_seconds = 0.0;
    return true;
}

void Game::get_supply_contract_ids(ft_vector<int> &out) const
{
    out.clear();
    size_t count = this->_supply_contracts.size();
    if (count == 0)
        return ;
    const Pair<int, ft_supply_contract> *entries = this->_supply_contracts.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
        out.push_back(entries[i].key);
}

bool Game::get_supply_contract(int contract_id, ft_supply_contract &out) const
{
    const Pair<int, ft_supply_contract> *entry = this->_supply_contracts.find(contract_id);
    if (entry == ft_nullptr)
        return false;
    out = entry->value;
    return true;
}

void Game::get_resource_dashboard(ft_resource_dashboard &out) const
{
    out.resources.clear();
    out.routes.clear();
    out.total_active_convoys = static_cast<int>(this->_active_convoys.size());
    out.average_route_threat = 0.0;
    out.maximum_route_threat = 0.0;

    struct ResourceAggregate
    {
        int amount;
        double rate;
        ResourceAggregate() : amount(0), rate(0.0) {}
    };

    ft_map<int, ResourceAggregate> aggregates;
    ft_vector<Pair<int, ft_sharedptr<ft_planet> > > planet_entries;
    ft_map_snapshot(this->_planets, planet_entries);
    for (size_t i = 0; i < planet_entries.size(); ++i)
    {
        int planet_id = planet_entries[i].key;
        const ft_sharedptr<ft_planet> &planet = planet_entries[i].value;
        if (!planet)
            continue;
        double mine_multiplier = this->_buildings.get_mine_multiplier(planet_id);
        if (mine_multiplier < 0.0)
            mine_multiplier = 0.0;
        const ft_vector<Pair<int, double> > &rates = planet->get_resources();
        for (size_t j = 0; j < rates.size(); ++j)
        {
            int resource_id = rates[j].key;
            double adjusted_rate = rates[j].value * this->_resource_multiplier * mine_multiplier;
            int amount = planet->get_resource(resource_id);
            Pair<int, ResourceAggregate> *entry = aggregates.find(resource_id);
            if (entry == ft_nullptr)
            {
                ResourceAggregate aggregate;
                aggregate.amount = amount;
                aggregate.rate = adjusted_rate;
                aggregates.insert(resource_id, aggregate);
            }
            else
            {
                entry->value.amount += amount;
                entry->value.rate += adjusted_rate;
            }
        }
    }

    size_t aggregate_count = aggregates.size();
    if (aggregate_count > 0)
    {
        Pair<int, ResourceAggregate> *entries = aggregates.end();
        entries -= aggregate_count;
        out.resources.reserve(aggregate_count);
        for (size_t i = 0; i < aggregate_count; ++i)
        {
            ft_resource_dashboard_entry summary;
            summary.resource_id = entries[i].key;
            summary.total_stock = entries[i].value.amount;
            summary.production_rate = entries[i].value.rate;
            out.resources.push_back(summary);
        }
    }

    ft_map<int, int> active_convoys_per_route;
    size_t active_count = this->_active_convoys.size();
    if (active_count > 0)
    {
        Pair<int, ft_supply_convoy> *convoys = this->_active_convoys.end();
        convoys -= active_count;
        for (size_t i = 0; i < active_count; ++i)
        {
            int route_id = convoys[i].value.route_id;
            if (route_id == 0)
                continue;
            Pair<int, int> *entry = active_convoys_per_route.find(route_id);
            if (entry == ft_nullptr)
                active_convoys_per_route.insert(route_id, 1);
            else
                entry->value += 1;
        }
    }

    double total_threat = 0.0;
    double max_threat = 0.0;
    ft_vector<Pair<RouteKey, ft_supply_route> > route_entries;
    ft_map_snapshot(this->_supply_routes, route_entries);
    size_t route_count = route_entries.size();
    if (route_count > 0)
    {
        out.routes.reserve(route_count);
        for (size_t i = 0; i < route_count; ++i)
        {
            const ft_supply_route &route = route_entries[i].value;
            ft_route_dashboard_entry entry;
            entry.route_id = route.id;
            entry.origin_planet_id = route.origin_planet_id;
            entry.destination_planet_id = route.destination_planet_id;
            entry.threat_level = route.threat_level;
            entry.quiet_timer = route.quiet_timer;
            entry.escalation_pending = route.escalation_pending;
            Pair<int, int> *active_entry = active_convoys_per_route.find(route.id);
            if (active_entry != ft_nullptr)
                entry.active_convoys = active_entry->value;
            else
                entry.active_convoys = 0;
            total_threat += route.threat_level;
            if (route.threat_level > max_threat)
                max_threat = route.threat_level;
            out.routes.push_back(entry);
        }
    }

    if (route_count > 0)
        out.average_route_threat = total_threat / static_cast<double>(route_count);
    else
        out.average_route_threat = 0.0;
    out.maximum_route_threat = max_threat;
}

int Game::get_active_convoy_count() const
{
    return static_cast<int>(this->_active_convoys.size());
}

double Game::get_fleet_escort_veterancy(int fleet_id) const
{
    if (fleet_id <= 0)
        return 0.0;
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0.0;
    return fleet->get_escort_veterancy();
}

int Game::get_fleet_escort_veterancy_bonus(int fleet_id) const
{
    if (fleet_id <= 0)
        return 0;
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->get_escort_veterancy_bonus();
}

int Game::transfer_ore(int from_planet_id, int to_planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> from = this->get_planet(from_planet_id);
    ft_sharedptr<ft_planet> to = this->get_planet(to_planet_id);
    if (!from || !to)
        return 0;
    if (from_planet_id == to_planet_id)
        return 0;
    int available = from->get_resource(ore_id);
    if (amount > available)
        amount = available;
    if (amount <= 0)
        return 0;
    ft_supply_route *route = this->ensure_supply_route(from_planet_id, to_planet_id);
    if (!route)
        return 0;
    from->sub_resource(ore_id, amount);
    this->send_state(from_planet_id, ore_id);
    this->ensure_planet_item_slot(to_planet_id, ore_id);
    int dispatched = this->dispatch_convoy(*route, from_planet_id, to_planet_id,
                                          ore_id, amount, 0);
    if (dispatched <= 0)
    {
        from->add_resource(ore_id, amount);
        this->send_state(from_planet_id, ore_id);
        return 0;
    }
    return dispatched;
}

