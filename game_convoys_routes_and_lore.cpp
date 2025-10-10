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

static double compute_campaign_raider_focus(int convoys_delivered_total,
    int order_branch_victories, int rebellion_branch_victories)
{
    double focus = 0.0;
    if (convoys_delivered_total > 12)
    {
        double deliveries = static_cast<double>(convoys_delivered_total - 12);
        if (deliveries > 70.0)
            deliveries = 70.0;
        focus += deliveries / 70.0;
    }
    int combined_victories = order_branch_victories + rebellion_branch_victories;
    if (combined_victories > 0)
    {
        double victory_progress = static_cast<double>(combined_victories) * 0.15;
        if (victory_progress > 0.6)
            victory_progress = 0.6;
        focus += victory_progress;
    }
    if (focus > 1.0)
        focus = 1.0;
    if (focus < 0.0)
        focus = 0.0;
    return focus;
}

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
