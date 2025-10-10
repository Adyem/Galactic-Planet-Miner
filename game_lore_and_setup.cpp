namespace
{
struct CombatVictoryNarrative
{
    int         journal_entry_id;
    const char  *journal_text;
    const char  *lore_text;
};

static const CombatVictoryNarrative kCombatVictoryNarratives[] = {
    {
        JOURNAL_ENTRY_COMBAT_VICTORY_RAIDER_BROADCAST,
        "Journal – Raider Broadcast Intercept: Raider captains curse Terra's shield wall as Sunflare sloops and repair drones bleed their formations dry.",
        "Intercepted raider gunner: \"They're cycling Sunflare charges again—shields are blinding us! Pull back before they cut the flank to ribbons.\""
    },
    {
        JOURNAL_ENTRY_COMBAT_VICTORY_DEFENSE_DEBRIEF,
        "Journal – Defense Debrief: Professor Lumen and Old Miner Joe document how frontline escorts layered shield generators and repair drones to stabilize the battle.",
        "Old Miner Joe to Professor Lumen: \"Repair drones sealed the hull breach—keep those shield generators humming and we'll hold the line.\""
    },
    {
        JOURNAL_ENTRY_COMBAT_VICTORY_LIBERATION_SIGNAL,
        "Journal – Liberation Signal: Rebel couriers whisper that Zara's sacrifice still echoes as loyal convoys repel the raid and the belt dares to hope.",
        "Encrypted rebel courier: \"Zara's sacrifice wasn't wasted—the convoy escorts fought like ghosts. The belt is listening.\""
    }
};

static const char *get_planet_story_name(int planet_id)
{
    if (planet_id == PLANET_TERRA)
        return "Terra";
    if (planet_id == PLANET_MARS)
        return "Mars";
    if (planet_id == PLANET_ZALTHOR)
        return "Zalthor";
    if (planet_id == PLANET_VULCAN)
        return "Vulcan";
    if (planet_id == PLANET_NOCTARIS_PRIME)
        return "Noctaris Prime";
    if (planet_id == PLANET_LUNA)
        return "Luna";
    return "the frontier";
}
} // namespace

void Game::append_lore_entry(const ft_string &entry)
{
    if (this->_lore_log_count < LORE_LOG_MAX_ENTRIES)
    {
        this->_lore_log.push_back(entry);
        this->_lore_log_count += 1;
    }
    else if (this->_lore_log.size() > 0)
    {
        size_t overwrite_index = this->_lore_log_start;
        this->_lore_log[overwrite_index] = entry;
        this->_lore_log_start = (this->_lore_log_start + 1) % this->_lore_log.size();
    }
    else
    {
        this->_lore_log.push_back(entry);
        this->_lore_log_count = 1;
        this->_lore_log_start = 0;
    }
    this->_lore_log_cache_dirty = true;
}

void Game::unlock_journal_entry(int entry_id, const ft_string &text)
{
    if (entry_id == 0 || text.size() == 0)
        return ;
    if (this->_journal_entries.find(entry_id) != ft_nullptr)
        return ;
    this->_journal_entries.insert(entry_id, text);
    this->_journal_unlock_order.push_back(entry_id);
    this->_journal_cache_dirty = true;
}

const ft_vector<ft_string> &Game::get_journal_entries() const
{
    if (!this->_journal_cache_dirty)
        return this->_journal_cache;

    this->_journal_cache.clear();
    for (size_t i = 0; i < this->_journal_unlock_order.size(); ++i)
    {
        int entry_id = this->_journal_unlock_order[i];
        const Pair<int, ft_string> *entry = this->_journal_entries.find(entry_id);
        if (entry != ft_nullptr)
            this->_journal_cache.push_back(entry->value);
    }
    this->_journal_cache_dirty = false;
    return this->_journal_cache;
}

bool Game::is_journal_entry_unlocked(int entry_id) const
{
    if (entry_id == 0)
        return false;
    if (this->_journal_entries.find(entry_id) != ft_nullptr)
        return true;
    return false;
}

void Game::record_combat_victory_narrative(int planet_id)
{
    size_t count = sizeof(kCombatVictoryNarratives) / sizeof(kCombatVictoryNarratives[0]);
    for (size_t i = 0; i < count; ++i)
    {
        const CombatVictoryNarrative &narrative = kCombatVictoryNarratives[i];
        if (this->is_journal_entry_unlocked(narrative.journal_entry_id))
            continue;
        if (narrative.journal_text != ft_nullptr)
        {
            ft_string journal_entry(narrative.journal_text);
            this->unlock_journal_entry(narrative.journal_entry_id, journal_entry);
        }
        if (narrative.lore_text != ft_nullptr)
        {
            ft_string lore_entry(narrative.lore_text);
            const char *planet_name = get_planet_story_name(planet_id);
            lore_entry.append(ft_string(" (Defense logged at "));
            lore_entry.append(ft_string(planet_name));
            lore_entry.append(ft_string(".)"));
            this->append_lore_entry(lore_entry);
        }
        return;
    }
}

void Game::handle_quick_quest_completion(const ft_quest_definition &definition, double completion_ratio)
{
    static const char *kQuickVoices[] = {
        "Old Miner Joe",
        "Professor Lumen",
        "Farmer Daisy",
        "Captain Blackthorne"
    };
    size_t voice_count = sizeof(kQuickVoices) / sizeof(kQuickVoices[0]);
    const char *speaker = "Old Miner Joe";
    if (voice_count > 0)
    {
        size_t index = static_cast<size_t>(this->_quick_completion_cursor % static_cast<int>(voice_count));
        speaker = kQuickVoices[index];
        this->_quick_completion_cursor += 1;
        if (this->_quick_completion_cursor >= static_cast<int>(voice_count))
            this->_quick_completion_cursor = 0;
    }
    int percent = static_cast<int>(completion_ratio * 100.0 + 0.5);
    if (percent < 0)
        percent = 0;
    if (percent > 100)
        percent = 100;
    ft_string lore_entry(speaker);
    lore_entry.append(ft_string(" praises the swift resolution of \""));
    lore_entry.append(definition.name);
    lore_entry.append(ft_string("\" with "));
    lore_entry.append(ft_to_string(percent));
    lore_entry.append(ft_string("% of the timer remaining."));
    this->append_lore_entry(lore_entry);
    if (!definition.is_side_quest)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ITEM_ENGINE_PART);
        this->add_ore(PLANET_TERRA, ITEM_ENGINE_PART, 1);
    }
    else
        this->add_ore(PLANET_TERRA, ORE_COPPER, 2);
}

Game::Game(const ft_string &host, const ft_string &path, int difficulty)
    : _backend(host, path),
      _save_system(),
      _lore_log(),
      _lore_log_start(0),
      _lore_log_count(0),
      _lore_log_cache(),
      _lore_log_cache_dirty(false),
      _journal_entries(),
      _journal_unlock_order(),
      _journal_cache(),
      _journal_cache_dirty(false),
      _resource_lore_cursors(),
      _raider_lore_cursor(0),
      _quick_completion_cursor(0),
      _difficulty(GAME_DIFFICULTY_STANDARD),
      _resource_multiplier(1.0),
      _quest_time_scale_base(1.0),
      _quest_time_scale_dynamic(1.0),
      _ui_scale(1.0),
      _lore_panel_anchor(PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT),
      _combat_speed_multiplier(1.0),
      _research_duration_scale(1.0),
      _assault_difficulty_multiplier(1.0),
      _ship_weapon_multiplier(1.0),
      _ship_shield_multiplier(1.0),
      _ship_hull_multiplier(1.0),
      _capital_ship_limit(0),
      _repair_drones_unlocked(false),
      _shield_support_unlocked(false),
      _escape_pod_protocol(false),
      _escape_pod_rescued(),
      _emergency_energy_protocol(false),
      _energy_conservation_active(),
      _supply_routes(),
      _route_lookup(),
      _active_convoys(),
      _route_convoy_escorts(),
      _supply_contracts(),
      _resource_deficits(),
      _next_route_id(1),
      _next_convoy_id(1),
      _next_contract_id(1),
      _convoys_delivered_total(0),
      _convoy_raid_losses(0),
      _current_delivery_streak(0),
      _longest_delivery_streak(0),
      _streak_milestones(),
      _next_streak_milestone_index(0),
      _order_branch_assault_victories(0),
      _rebellion_branch_assault_victories(0),
      _order_branch_pending_assault(0),
      _rebellion_branch_pending_assault(0),
      _last_planet_checkpoint(),
      _last_fleet_checkpoint(),
      _last_research_checkpoint(),
      _last_achievement_checkpoint(),
      _last_building_checkpoint(),
      _last_progress_checkpoint(),
      _last_checkpoint_tag(),
      _has_checkpoint(false),
      _failed_checkpoint_tags(),
      _force_checkpoint_failure(false),
      _backend_online(true),
      _backend_retry_delay_ms(0),
      _backend_next_retry_ms(0)
{
    ft_sharedptr<ft_planet> terra(new ft_planet_terra());
    ft_sharedptr<ft_planet> mars(new ft_planet_mars());
    ft_sharedptr<ft_planet> zalthor(new ft_planet_zalthor());
    ft_sharedptr<ft_planet> vulcan(new ft_planet_vulcan());
    ft_sharedptr<ft_planet> noctaris(new ft_planet_noctaris_prime());
    ft_sharedptr<ft_planet> luna(new ft_planet_luna());

    this->_state.add_character(terra);
    this->_planets.insert(PLANET_TERRA, terra);
    this->_buildings.initialize_planet(*this, PLANET_TERRA);

    this->_locked_planets.insert(PLANET_MARS, mars);
    this->_locked_planets.insert(PLANET_ZALTHOR, zalthor);
    this->_locked_planets.insert(PLANET_VULCAN, vulcan);
    this->_locked_planets.insert(PLANET_NOCTARIS_PRIME, noctaris);
    this->_locked_planets.insert(PLANET_LUNA, luna);

    this->configure_difficulty(difficulty);
    this->_streak_milestones.push_back(3);
    this->_streak_milestones.push_back(5);
    this->_streak_milestones.push_back(8);
    this->unlock_journal_entry(JOURNAL_ENTRY_PROFILE_MINER_JOE,
        ft_string("Character Profile – Old Miner Joe: A weathered Terra veteran who rebuilt the mines after tragedy and now mentors crews with relentless patience. He keeps ledgers of every sacrifice made during the core collapse and fights to ensure no crew is ever forgotten again."));
    this->unlock_journal_entry(JOURNAL_ENTRY_PROFILE_PROFESSOR_LUMEN,
        ft_string("Character Profile – Professor Lumen: An astrophysicist tracking strange energy waves whose research ties raider strikes to corruption in the core. Lumen's private notes describe how the anomaly threads through bureaucracy, warping minds that once swore to protect the frontier."));
    this->unlock_journal_entry(JOURNAL_ENTRY_PROFILE_FARMER_DAISY,
        ft_string("Character Profile – Farmer Daisy: The heart of the agricultural colonies, balancing hydroponic harvests with convoy negotiations to keep everyone fed. Daisy remembers when Zalthor's soil ran red with famine and refuses to let another generation go hungry on her watch."));
    this->unlock_journal_entry(JOURNAL_ENTRY_PROFILE_BLACKTHORNE,
        ft_string("Character Profile – Captain Blackthorne: Charismatic raider leader shaped by betrayal, convinced the system must be shattered to heal the frontier. His manifesto mourns miners abandoned to die on Vulcan Station and promises a rebellion that answers to the forgotten."));
    this->unlock_journal_entry(JOURNAL_ENTRY_PROFILE_NAVIGATOR_ZARA,
        ft_string("Character Profile – Navigator Zara: A prodigy pilot whose daring maneuvers keep resistance fleets alive even when sacrifice is the only option. Her map room is covered in flight lines marking every convoy saved, each string woven into a constellation of hope."));
    this->unlock_journal_entry(JOURNAL_ENTRY_PROFILE_SCOUT_FINN,
        ft_string("Character Profile – Old Scout Finn: Quiet frontier scout documenting how neglect and desperation turn ordinary settlers into hardened raiders. Finn's journals chart abandoned settlements and list the names of colonists praying for supply ships that never came."));
    this->unlock_journal_entry(JOURNAL_ENTRY_LORE_TERRA_REBUILD,
        ft_string("Journal – Rebuilding Terra: The capital's shattered docks now hum with modular refineries, rebuilt by surviving families and freshly minted cadets. Terra's leadership pledges every convoy assignment to weave new trust between the core and the outer belt."));
    this->unlock_journal_entry(JOURNAL_ENTRY_LORE_MARS_OUTPOSTS,
        ft_string("Journal – Mars Garrison Outposts: Fortified redline settlements oversee the lifelines to the agricultural moons, watching for the first flare of raider engines. Veterans stationed there trade stories of frozen nights endured to keep the convoys running."));
    this->unlock_journal_entry(JOURNAL_ENTRY_LORE_ZALTHOR_ANOMALY,
        ft_string("Journal – The Zalthor Anomaly: Professor Lumen's arrays pinpoint energy rifts above Zalthor's shipyards, echoes of experiments abandoned by the Dominion. The anomaly has begun to sing, and every scientist fears the chorus might wake something older than the rebellion."));
    this->unlock_journal_entry(JOURNAL_ENTRY_LORE_CONVOY_CORPS,
        ft_string("Journal – Convoy Corps Charter: Logistics commanders formalize the Convoy Corps, pairing engineers with pilots who swear to deliver supplies no matter the cost. Their oath binds Terra, Mars, Noctaris, and Luna into a single lattice of mutual survival."));
    this->save_campaign_checkpoint(ft_string("initial_setup"));
}

Game::~Game()
{
    return ;
}
