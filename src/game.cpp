#include "game.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Template/pair.hpp"

Game::Game(const ft_string &host, const ft_string &path, int difficulty)
    : _backend(host, path),
      _difficulty(GAME_DIFFICULTY_STANDARD),
      _resource_multiplier(1.0),
      _quest_time_scale(1.0),
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
      _supply_routes(),
      _route_lookup(),
      _active_convoys(),
      _next_route_id(1),
      _next_convoy_id(1)
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
}

Game::~Game()
{
    return ;
}

void Game::produce(double seconds)
{
    int planet_ids[] = {
        PLANET_TERRA,
        PLANET_MARS,
        PLANET_ZALTHOR,
        PLANET_VULCAN,
        PLANET_NOCTARIS_PRIME,
        PLANET_LUNA
    };
    size_t count = sizeof(planet_ids) / sizeof(planet_ids[0]);
    for (size_t i = 0; i < count; ++i)
    {
        int planet_id = planet_ids[i];
        ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
        if (!planet)
            continue;
        ft_vector<Pair<int, int> > produced = planet->produce(seconds);
        double mine_multiplier = this->_buildings.get_mine_multiplier(planet_id);
        for (size_t j = 0; j < produced.size(); ++j)
        {
            int ore_id = produced[j].key;
            int base_amount = produced[j].value;
            int final_amount = base_amount;
            if (base_amount > 0)
            {
                double multiplier_delta = this->_resource_multiplier - 1.0;
                if (multiplier_delta < 0.0)
                    multiplier_delta = -multiplier_delta;
                if (multiplier_delta > 0.000001)
                {
                    double scaled_amount = static_cast<double>(base_amount) * this->_resource_multiplier;
                    int target = static_cast<int>(scaled_amount);
                    if (this->_resource_multiplier > 1.0)
                    {
                        double fractional = scaled_amount - static_cast<double>(target);
                        if (fractional > 0.000001)
                            target += 1;
                    }
                    if (target < 0)
                        target = 0;
                    final_amount = target;
                    int diff = final_amount - base_amount;
                    if (diff > 0)
                        planet->add_resource(ore_id, diff);
                    else if (diff < 0)
                        planet->sub_resource(ore_id, -diff);
                }
            }
            this->send_state(planet_id, ore_id);
            if (mine_multiplier > 1.0 && final_amount > 0)
            {
                double bonus_amount = (mine_multiplier - 1.0) * static_cast<double>(final_amount);
                int bonus = static_cast<int>(bonus_amount);
                if (bonus > 0)
                {
                    planet->add_resource(ore_id, bonus);
                    this->send_state(planet_id, ore_id);
                }
            }
        }
    }
}

void Game::tick(double seconds)
{
    this->produce(seconds);
    this->advance_convoys(seconds);
    this->_buildings.tick(*this, seconds);
    size_t count = this->_fleets.size();
    Pair<int, ft_sharedptr<ft_fleet> > *entries = this->_fleets.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        ft_sharedptr<ft_fleet> fleet = entries[i].value;
        fleet->tick(seconds);
    }
    ft_vector<int> completed;
    this->_research.tick(seconds, completed);
    for (size_t i = 0; i < completed.size(); ++i)
        this->handle_research_completion(completed[i]);

    ft_quest_context quest_context;
    this->build_quest_context(quest_context);
    ft_vector<int> quest_completed;
    ft_vector<int> quest_failed;
    ft_vector<int> quest_choices;
    this->_quests.update(seconds, quest_context, quest_completed, quest_failed, quest_choices);
    for (size_t i = 0; i < quest_completed.size(); ++i)
        this->handle_quest_completion(quest_completed[i]);
    for (size_t i = 0; i < quest_failed.size(); ++i)
        this->handle_quest_failure(quest_failed[i]);
    for (size_t i = 0; i < quest_choices.size(); ++i)
        this->handle_quest_choice_prompt(quest_choices[i]);

    ft_vector<int> assault_completed;
    ft_vector<int> assault_failed;
    this->_combat.tick(seconds, this->_fleets, this->_planet_fleets, assault_completed, assault_failed);
    for (size_t i = 0; i < assault_completed.size(); ++i)
    {
        int planet_id = assault_completed[i];
        ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
        if (planet)
        {
            const ft_vector<Pair<int, double> > &resources = planet->get_resources();
            if (resources.size() > 0)
            {
                int reward_ore = resources[0].key;
                planet->add_resource(reward_ore, 3);
                this->send_state(planet_id, reward_ore);
            }
        }
        ft_string entry("Old Miner Joe records a victory at planet ");
        entry.append(ft_to_string(planet_id));
        entry.append(ft_string(": the raider fleet was repelled."));
        this->_lore_log.push_back(entry);
    }
    for (size_t i = 0; i < assault_failed.size(); ++i)
    {
        int planet_id = assault_failed[i];
        ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
        if (planet)
        {
            const ft_vector<Pair<int, double> > &resources = planet->get_resources();
            if (resources.size() > 0)
            {
                int penalty_ore = resources[0].key;
                planet->sub_resource(penalty_ore, 2);
                this->send_state(planet_id, penalty_ore);
            }
        }
        ft_string entry("Professor Lumen warns of losses on planet ");
        entry.append(ft_to_string(planet_id));
        entry.append(ft_string(": raiders breached the defenses."));
        this->_lore_log.push_back(entry);
    }
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

void Game::send_state(int planet_id, int ore_id)
{
    ft_sharedptr<const ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return ;
    ft_string body("{\"planet\":");
    body.append(ft_to_string(planet_id));
    body.append(",\"ore\":");
    body.append(ft_to_string(ore_id));
    body.append(",\"amount\":");
    body.append(ft_to_string(planet->get_resource(ore_id)));
    body.append("}");
    ft_string response;
    this->_backend.send_state(body, response);
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
}

bool Game::can_pay_research_cost(const ft_vector<Pair<int, int> > &costs) const
{
    for (size_t i = 0; i < costs.size(); ++i)
    {
        int ore_id = costs[i].key;
        int required = costs[i].value;
        int total = 0;
        size_t count = this->_planets.size();
        const Pair<int, ft_sharedptr<ft_planet> > *entries = this->_planets.end();
        entries -= count;
        for (size_t j = 0; j < count; ++j)
        {
            const ft_sharedptr<ft_planet> &planet = entries[j].value;
            total += planet->get_resource(ore_id);
            if (total >= required)
                break;
        }
        if (total < required)
            return false;
    }
    return true;
}

void Game::pay_research_cost(const ft_vector<Pair<int, int> > &costs)
{
    for (size_t i = 0; i < costs.size(); ++i)
    {
        int ore_id = costs[i].key;
        int remaining = costs[i].value;
        if (remaining <= 0)
            continue;
        size_t count = this->_planets.size();
        Pair<int, ft_sharedptr<ft_planet> > *entries = this->_planets.end();
        entries -= count;
        for (size_t j = 0; j < count && remaining > 0; ++j)
        {
            ft_sharedptr<ft_planet> planet = entries[j].value;
            int available = planet->get_resource(ore_id);
            if (available <= 0)
                continue;
            int take = remaining < available ? remaining : available;
            planet->sub_resource(ore_id, take);
            this->send_state(planet->get_id(), ore_id);
            remaining -= take;
        }
    }
}

void Game::handle_research_completion(int research_id)
{
    const ft_research_definition *definition = this->_research.get_definition(research_id);
    if (definition == ft_nullptr)
        return ;
    bool update_modifiers = false;
    for (size_t i = 0; i < definition->unlock_planets.size(); ++i)
        this->unlock_planet(definition->unlock_planets[i]);
    this->_buildings.apply_research_unlock(research_id);
    if (research_id == RESEARCH_URBAN_PLANNING_TERRA)
        this->_buildings.add_planet_logistic_bonus(PLANET_TERRA, 4);
    else if (research_id == RESEARCH_URBAN_PLANNING_MARS)
        this->_buildings.add_planet_logistic_bonus(PLANET_MARS, 4);
    else if (research_id == RESEARCH_URBAN_PLANNING_ZALTHOR)
        this->_buildings.add_planet_logistic_bonus(PLANET_ZALTHOR, 4);
    else if (research_id == RESEARCH_CRAFTING_MASTERY)
        this->_buildings.set_crafting_energy_multiplier(0.8);
    else if (research_id == RESEARCH_STRUCTURAL_REINFORCEMENT_I
        || research_id == RESEARCH_STRUCTURAL_REINFORCEMENT_II
        || research_id == RESEARCH_STRUCTURAL_REINFORCEMENT_III)
    {
        this->_ship_hull_multiplier += 0.1;
        update_modifiers = true;
    }
    else if (research_id == RESEARCH_DEFENSIVE_FORTIFICATION_I
        || research_id == RESEARCH_DEFENSIVE_FORTIFICATION_II
        || research_id == RESEARCH_DEFENSIVE_FORTIFICATION_III)
    {
        this->_ship_shield_multiplier += 0.1;
        update_modifiers = true;
    }
    else if (research_id == RESEARCH_ARMAMENT_ENHANCEMENT_I
        || research_id == RESEARCH_ARMAMENT_ENHANCEMENT_II
        || research_id == RESEARCH_ARMAMENT_ENHANCEMENT_III)
    {
        this->_ship_weapon_multiplier += 0.1;
        update_modifiers = true;
    }
    else if (research_id == RESEARCH_FASTER_CRAFTING)
        this->_buildings.set_crafting_speed_multiplier(1.25);
    else if (research_id == RESEARCH_EMERGENCY_ENERGY_CONSERVATION)
        this->_buildings.set_global_energy_multiplier(0.9);
    else if (research_id == RESEARCH_SHIELD_TECHNOLOGY)
        this->_shield_support_unlocked = true;
    else if (research_id == RESEARCH_REPAIR_DRONE_TECHNOLOGY)
        this->_repair_drones_unlocked = true;
    else if (research_id == RESEARCH_CAPITAL_SHIP_INITIATIVE)
        this->_capital_ship_limit = 1;
    else if (research_id == RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT)
        this->_capital_ship_limit = 2;
    else if (research_id == RESEARCH_ESCAPE_POD_LIFELINE)
    {
        this->_escape_pod_protocol = true;
        size_t saved = this->_escape_pod_rescued.size();
        if (saved > 0)
        {
            Pair<int, bool> *entries = this->_escape_pod_rescued.end();
            entries -= saved;
            for (size_t i = 0; i < saved; ++i)
                entries[i].value = false;
        }
    }
    if (update_modifiers)
        this->update_combat_modifiers();
}

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
    if (entry.size() > 0)
        this->_lore_log.push_back(entry);
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
        this->_quest_time_scale = 1.25;
        this->_research_duration_scale = 0.85;
        this->_assault_difficulty_multiplier = 0.85;
    }
    else if (selected == GAME_DIFFICULTY_HARD)
    {
        this->_resource_multiplier = 0.85;
        this->_quest_time_scale = 0.75;
        this->_research_duration_scale = 1.2;
        this->_assault_difficulty_multiplier = 1.25;
    }
    else
    {
        this->_resource_multiplier = 1.0;
        this->_quest_time_scale = 1.0;
        this->_research_duration_scale = 1.0;
        this->_assault_difficulty_multiplier = 1.0;
    }
    this->_research.set_duration_scale(this->_research_duration_scale);
    this->_quests.set_time_scale(this->_quest_time_scale);
    this->update_combat_modifiers();
}

void Game::update_combat_modifiers()
{
    this->_combat.set_player_weapon_multiplier(this->_ship_weapon_multiplier);
    this->_combat.set_player_shield_multiplier(this->_ship_shield_multiplier);
    this->_combat.set_player_hull_multiplier(this->_ship_hull_multiplier);
}

int Game::count_capital_ships_in_collection(const ft_map<int, ft_sharedptr<ft_fleet> > &collection) const
{
    size_t count = collection.size();
    if (count == 0)
        return 0;
    const Pair<int, ft_sharedptr<ft_fleet> > *entries = collection.end();
    entries -= count;
    int total = 0;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_sharedptr<ft_fleet> &fleet = entries[i].value;
        if (!fleet)
            continue;
        ft_vector<int> ship_ids;
        fleet->get_ship_ids(ship_ids);
        for (size_t j = 0; j < ship_ids.size(); ++j)
        {
            if (is_capital_ship_type(fleet->get_ship_type(ship_ids[j])))
                total += 1;
        }
    }
    return total;
}

int Game::count_capital_ships() const
{
    int total = this->count_capital_ships_in_collection(this->_fleets);
    total += this->count_capital_ships_in_collection(this->_planet_fleets);
    return total;
}

bool Game::is_ship_type_available(int ship_type) const
{
    switch (ship_type)
    {
    case SHIP_SHIELD:
    case SHIP_RADAR:
    case SHIP_SALVAGE:
        return true;
    case SHIP_TRANSPORT:
        if (!this->_research.is_completed(RESEARCH_CRAFTING_MASTERY))
            return false;
        return true;
    case SHIP_CORVETTE:
        if (!this->_research.is_completed(RESEARCH_ARMAMENT_ENHANCEMENT_I))
            return false;
        return true;
    case SHIP_INTERCEPTOR:
        if (!this->_research.is_completed(RESEARCH_ARMAMENT_ENHANCEMENT_II))
            return false;
        return true;
    case SHIP_REPAIR_DRONE:
        if (!this->_repair_drones_unlocked)
            return false;
        return true;
    case SHIP_SUNFLARE_SLOOP:
        if (!this->_shield_support_unlocked)
            return false;
        return true;
    case SHIP_FRIGATE_ESCORT:
    case SHIP_FRIGATE_SUPPORT:
        if (!this->_research.is_completed(RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT))
            return false;
        return true;
    case SHIP_CAPITAL_CARRIER:
    case SHIP_CAPITAL_DREADNOUGHT:
        if (!this->_research.is_completed(RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT))
            return false;
        if (this->_capital_ship_limit <= 0)
            return false;
        return true;
    case SHIP_CAPITAL:
        if (this->_capital_ship_limit <= 0)
            return false;
        return true;
    default:
        return true;
    }
}

void Game::clear_escape_pod_records(const ft_fleet &fleet)
{
    ft_vector<int> ship_ids;
    fleet.get_ship_ids(ship_ids);
    for (size_t i = 0; i < ship_ids.size(); ++i)
        this->_escape_pod_rescued.remove(ship_ids[i]);
}

int Game::compose_route_key(int origin, int destination) const
{
    return origin * 256 + destination;
}

Game::ft_supply_route *Game::ensure_supply_route(int origin, int destination)
{
    if (origin == destination)
        return ft_nullptr;
    int key = this->compose_route_key(origin, destination);
    Pair<int, ft_supply_route> *entry = this->_supply_routes.find(key);
    if (entry != ft_nullptr)
        return &entry->value;
    ft_supply_route route;
    route.id = this->_next_route_id++;
    route.origin_planet_id = origin;
    route.destination_planet_id = destination;
    route.base_travel_time = this->estimate_route_travel_time(origin, destination);
    route.escort_requirement = this->estimate_route_escort_requirement(origin, destination);
    route.base_raid_risk = this->estimate_route_raid_risk(origin, destination);
    this->_supply_routes.insert(key, route);
    entry = this->_supply_routes.find(key);
    if (entry == ft_nullptr)
        return ft_nullptr;
    this->_route_lookup.insert(route.id, key);
    return &entry->value;
}

const Game::ft_supply_route *Game::get_route_by_id(int route_id) const
{
    const Pair<int, int> *lookup = this->_route_lookup.find(route_id);
    if (lookup == ft_nullptr)
        return ft_nullptr;
    const Pair<int, ft_supply_route> *entry = this->_supply_routes.find(lookup->value);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
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
    double base = 28.0;
    if (involves_terra && involves_luna)
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
    if (involves_noctaris)
        return 6;
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
    if (involves_vulcan)
        risk += 0.006;
    if (involves_noctaris)
        risk += 0.012;
    if (involves_luna)
        risk -= 0.004;
    if (risk < 0.006)
        risk = 0.006;
    return risk;
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
        case SHIP_FRIGATE_ESCORT:
            rating += 6;
            break;
        case SHIP_FRIGATE_SUPPORT:
            rating += 4;
            break;
        case SHIP_CAPITAL:
        case SHIP_CAPITAL_CARRIER:
        case SHIP_CAPITAL_DREADNOUGHT:
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
    if (rating > 48)
        rating = 48;
    return rating;
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
            ft_location loc = fleet->get_location();
            if (loc.type == LOCATION_PLANET && loc.from == planet_id)
                rating += this->calculate_fleet_escort_rating(*fleet);
        }
    }
    if (rating > 48)
        rating = 48;
    return rating;
}

double Game::calculate_convoy_travel_time(const ft_supply_route &route, int origin_escort, int destination_escort) const
{
    double time = route.base_travel_time;
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
    if (route->escort_requirement > 0)
    {
        if (convoy.origin_escort < route->escort_requirement)
        {
            double deficit = static_cast<double>(route->escort_requirement - convoy.origin_escort);
            risk *= 1.6;
            risk += deficit * 0.01;
        }
        else
        {
            double surplus = static_cast<double>(convoy.origin_escort - route->escort_requirement);
            double divisor = 1.0 + surplus * 0.12;
            if (divisor > 1.0)
                risk /= divisor;
        }
        if (convoy.destination_escort > route->escort_requirement)
        {
            double dest_surplus = static_cast<double>(convoy.destination_escort - route->escort_requirement);
            risk -= dest_surplus * 0.006;
        }
    }
    if (origin_under_attack)
        risk *= 1.8;
    if (destination_under_attack)
        risk *= 1.6;
    if (convoy.raided)
        risk *= 0.35;
    if (risk < 0.002)
        risk = 0.002;
    return risk;
}

void Game::handle_convoy_raid(ft_supply_convoy &convoy, bool origin_under_attack, bool destination_under_attack)
{
    const ft_supply_route *route = this->get_route_by_id(convoy.route_id);
    if (!route)
        return ;
    double severity = 0.45;
    if (convoy.origin_escort < route->escort_requirement)
        severity += 0.25;
    if (convoy.destination_escort < route->escort_requirement)
        severity += 0.1;
    if (origin_under_attack || destination_under_attack)
    {
        severity += 0.3;
        if (convoy.origin_escort < route->escort_requirement)
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
    convoy.amount -= lost;
    convoy.raided = true;
    convoy.raid_meter = 0.0;
    if (convoy.amount <= 0)
    {
        convoy.amount = 0;
        convoy.destroyed = true;
        convoy.remaining_time = 0.0;
    }
    ft_string entry("Raiders ambushed a convoy from ");
    entry.append(ft_to_string(convoy.origin_planet_id));
    entry.append(ft_string(" to "));
    entry.append(ft_to_string(convoy.destination_planet_id));
    entry.append(ft_string("; "));
    entry.append(ft_to_string(lost));
    entry.append(ft_string(" units were lost"));
    if (convoy.destroyed)
        entry.append(ft_string(", and the shipment was wiped out."));
    else
        entry.append(ft_string(", but escorts salvaged part of the cargo."));
    this->_lore_log.push_back(entry);
}

void Game::finalize_convoy(const ft_supply_convoy &convoy)
{
    if (!convoy.destroyed && convoy.amount > 0)
    {
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
        this->_lore_log.push_back(entry);
    }
    else
    {
        ft_string entry("A convoy from ");
        entry.append(ft_to_string(convoy.origin_planet_id));
        entry.append(ft_string(" to "));
        entry.append(ft_to_string(convoy.destination_planet_id));
        entry.append(ft_string(" failed to arrive."));
        this->_lore_log.push_back(entry);
    }
}

void Game::advance_convoys(double seconds)
{
    if (seconds <= 0.0)
        return ;
    size_t count = this->_active_convoys.size();
    if (count == 0)
        return ;
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
                this->handle_convoy_raid(convoy, origin_under_attack, destination_under_attack);
                if (convoy.destroyed)
                {
                    completed.push_back(entries[i].key);
                    continue;
                }
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

bool Game::is_planet_unlocked(int planet_id) const
{
    const Pair<int, ft_sharedptr<ft_planet> > *entry = this->_planets.find(planet_id);
    return entry != ft_nullptr;
}

bool Game::can_place_building(int planet_id, int building_id, int x, int y) const
{
    if (!this->is_planet_unlocked(planet_id))
        return false;
    return this->_buildings.can_place_building(*this, planet_id, building_id, x, y);
}

int Game::place_building(int planet_id, int building_id, int x, int y)
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.place_building(*this, planet_id, building_id, x, y);
}

bool Game::remove_building(int planet_id, int instance_id)
{
    if (!this->is_planet_unlocked(planet_id))
        return false;
    return this->_buildings.remove_building(*this, planet_id, instance_id);
}

int Game::get_building_instance(int planet_id, int x, int y) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.get_building_instance(planet_id, x, y);
}

int Game::get_building_count(int planet_id, int building_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.get_building_count(planet_id, building_id);
}

int Game::get_planet_plot_capacity(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.get_planet_plot_capacity(planet_id);
}

int Game::get_planet_plot_usage(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.get_planet_plot_usage(planet_id);
}

int Game::get_planet_logistic_capacity(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.get_planet_logistic_capacity(planet_id);
}

int Game::get_planet_logistic_usage(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0;
    return this->_buildings.get_planet_logistic_usage(planet_id);
}

double Game::get_planet_energy_generation(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0.0;
    return this->_buildings.get_planet_energy_generation(planet_id);
}

double Game::get_planet_energy_consumption(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0.0;
    return this->_buildings.get_planet_energy_consumption(planet_id);
}

double Game::get_planet_mine_multiplier(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 1.0;
    return this->_buildings.get_mine_multiplier(planet_id);
}

double Game::get_planet_energy_pressure(int planet_id) const
{
    if (!this->is_planet_unlocked(planet_id))
        return 0.0;
    return this->_buildings.get_planet_energy_pressure(planet_id);
}

void Game::ensure_planet_item_slot(int planet_id, int resource_id)
{
    ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return ;
    planet->ensure_item_slot(resource_id);
}

bool Game::can_start_research(int research_id) const
{
    const ft_research_definition *definition = this->_research.get_definition(research_id);
    if (definition == ft_nullptr)
        return false;
    if (!this->_research.can_start(research_id))
        return false;
    return this->can_pay_research_cost(definition->costs);
}

bool Game::start_research(int research_id)
{
    const ft_research_definition *definition = this->_research.get_definition(research_id);
    if (definition == ft_nullptr)
        return false;
    if (!this->_research.can_start(research_id))
        return false;
    if (!this->can_pay_research_cost(definition->costs))
        return false;
    if (!this->_research.start(research_id))
        return false;
    this->pay_research_cost(definition->costs);
    return true;
}

int Game::get_research_status(int research_id) const
{
    return this->_research.get_status(research_id);
}

double Game::get_research_time_remaining(int research_id) const
{
    return this->_research.get_remaining_time(research_id);
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
    (void)quest_completed;
    (void)quest_failed;
    (void)quest_choices;
    this->handle_quest_choice_resolution(quest_id, choice_id);
    return true;
}

int Game::get_quest_choice(int quest_id) const
{
    return this->_quests.get_choice(quest_id);
}

int Game::add_ore(int planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return 0;
    int total = planet->add_resource(ore_id, amount);
    this->send_state(planet_id, ore_id);
    return total;
}

int Game::sub_ore(int planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return 0;
    int total = planet->sub_resource(ore_id, amount);
    this->send_state(planet_id, ore_id);
    return total;
}

int Game::get_ore(int planet_id, int ore_id) const
{
    ft_sharedptr<const ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return 0;
    return planet->get_resource(ore_id);
}

void Game::set_ore(int planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return ;
    planet->set_resource(ore_id, amount);
    this->send_state(planet_id, ore_id);
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
    ft_supply_convoy convoy;
    convoy.id = this->_next_convoy_id++;
    convoy.route_id = route->id;
    convoy.origin_planet_id = from_planet_id;
    convoy.destination_planet_id = to_planet_id;
    convoy.resource_id = ore_id;
    convoy.amount = amount;
    convoy.origin_escort = this->calculate_planet_escort_rating(from_planet_id);
    convoy.destination_escort = this->calculate_planet_escort_rating(to_planet_id);
    convoy.remaining_time = this->calculate_convoy_travel_time(*route, convoy.origin_escort, convoy.destination_escort);
    this->_active_convoys.insert(convoy.id, convoy);
    ft_string entry("Quartermaster Nia dispatches a convoy from ");
    entry.append(ft_to_string(from_planet_id));
    entry.append(ft_string(" to "));
    entry.append(ft_to_string(to_planet_id));
    entry.append(ft_string(" carrying "));
    entry.append(ft_to_string(amount));
    entry.append(ft_string(" units."));
    this->_lore_log.push_back(entry);
    return amount;
}

double Game::get_rate(int planet_id, int ore_id) const
{
    ft_sharedptr<const ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return 0;
    return planet->get_rate(ore_id);
}

const ft_vector<Pair<int, double> > &Game::get_planet_resources(int planet_id) const
{
    static ft_vector<Pair<int, double> > empty;
    ft_sharedptr<const ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return empty;
    return planet->get_resources();
}

int Game::get_active_convoy_count() const
{
    return static_cast<int>(this->_active_convoys.size());
}

void Game::create_fleet(int fleet_id)
{
    if (this->_fleets.find(fleet_id) != ft_nullptr)
        return ;
    ft_sharedptr<ft_fleet> fleet(new ft_fleet(fleet_id));
    fleet->set_location_planet(PLANET_TERRA);
    this->_state.add_character(fleet);
    this->_fleets.insert(fleet_id, fleet);
}

void Game::remove_fleet(int fleet_id, int target_fleet_id, int target_planet_id)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    bool transferred = false;
    if (target_fleet_id >= 0)
    {
        ft_sharedptr<ft_fleet> target = this->get_fleet(target_fleet_id);
        if (target)
        {
            fleet->move_ships_to(*target);
            transferred = true;
        }
    }
    else if (target_planet_id >= 0)
    {
        ft_sharedptr<ft_fleet> target = this->get_planet_fleet(target_planet_id);
        if (target)
        {
            fleet->move_ships_to(*target);
            transferred = true;
        }
    }
    if (!transferred)
        this->clear_escape_pod_records(*fleet);
    this->_fleets.remove(fleet_id);
}

int Game::create_ship(int fleet_id, int ship_type)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    if (!this->is_ship_type_available(ship_type))
        return 0;
    if (is_capital_ship_type(ship_type))
    {
        if (this->_capital_ship_limit <= 0)
            return 0;
        if (this->count_capital_ships() >= this->_capital_ship_limit)
            return 0;
    }
    int uid = fleet->create_ship(ship_type);
    if (uid != 0)
        this->_escape_pod_rescued.remove(uid);
    return uid;
}

void Game::remove_ship(int fleet_id, int ship_uid)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    this->_escape_pod_rescued.remove(ship_uid);
    fleet->remove_ship(ship_uid);
}

bool Game::transfer_ship(int from_fleet_id, int to_fleet_id, int ship_uid)
{
    ft_sharedptr<ft_fleet> from = this->get_fleet(from_fleet_id);
    if (!from)
        return false;
    ft_sharedptr<ft_fleet> to = this->get_fleet(to_fleet_id);
    if (!to)
        return false;
    return from->move_ship_to(*to, ship_uid);
}

void Game::set_ship_armor(int fleet_id, int ship_uid, int value)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_ship_armor(ship_uid, value);
}

int Game::get_ship_armor(int fleet_id, int ship_uid) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->get_ship_armor(ship_uid);
}

int Game::add_ship_armor(int fleet_id, int ship_uid, int amount)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->add_ship_armor(ship_uid, amount);
}

int Game::sub_ship_armor(int fleet_id, int ship_uid, int amount)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->sub_ship_armor(ship_uid, amount);
}

void Game::set_ship_hp(int fleet_id, int ship_uid, int value)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_ship_hp(ship_uid, value);
    if (value <= 0)
        this->_escape_pod_rescued.remove(ship_uid);
}

int Game::get_ship_hp(int fleet_id, int ship_uid) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->get_ship_hp(ship_uid);
}

int Game::add_ship_hp(int fleet_id, int ship_uid, int amount)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->add_ship_hp(ship_uid, amount);
}

int Game::sub_ship_hp(int fleet_id, int ship_uid, int amount)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    int hp = fleet->sub_ship_hp(ship_uid, amount);
    if (hp <= 0)
    {
        if (this->_escape_pod_protocol)
        {
            Pair<int, bool> *entry = this->_escape_pod_rescued.find(ship_uid);
            bool already_used = (entry != ft_nullptr && entry->value);
            if (!already_used)
            {
                int restored = 10;
                fleet->set_ship_hp(ship_uid, restored);
                if (entry == ft_nullptr)
                    this->_escape_pod_rescued.insert(ship_uid, true);
                else
                    entry->value = true;
                return restored;
            }
        }
        this->_escape_pod_rescued.remove(ship_uid);
    }
    return hp;
}

void Game::set_ship_shield(int fleet_id, int ship_uid, int value)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_ship_shield(ship_uid, value);
}

int Game::get_ship_shield(int fleet_id, int ship_uid) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->get_ship_shield(ship_uid);
}

int Game::add_ship_shield(int fleet_id, int ship_uid, int amount)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->add_ship_shield(ship_uid, amount);
}

int Game::sub_ship_shield(int fleet_id, int ship_uid, int amount)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->sub_ship_shield(ship_uid, amount);
}

void Game::set_fleet_location_planet(int fleet_id, int planet_id)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_location_planet(planet_id);
}

void Game::set_fleet_location_travel(int fleet_id, int from, int to, double time)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_location_travel(from, to, time);
}

void Game::set_fleet_location_misc(int fleet_id, int misc_id)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
    fleet->set_location_misc(misc_id);
}

ft_location Game::get_fleet_location(int fleet_id) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ft_location();
    return fleet->get_location();
}

double Game::get_fleet_travel_time(int fleet_id) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0.0;
    return fleet->get_travel_time();
}

int Game::get_planet_fleet_ship_hp(int planet_id, int ship_uid) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_planet_fleet(planet_id);
    if (!fleet)
        return 0;
    return fleet->get_ship_hp(ship_uid);
}

ft_location Game::get_planet_fleet_location(int planet_id) const
{
    ft_sharedptr<const ft_fleet> fleet = this->get_planet_fleet(planet_id);
    if (!fleet)
        return ft_location();
    return fleet->get_location();
}

bool Game::start_raider_assault(int planet_id, double difficulty, int control_mode)
{
    if (!this->is_planet_unlocked(planet_id))
        return false;
    this->_buildings.tick(*this, 0.0);
    double raw_energy_pressure = this->_buildings.get_planet_energy_pressure(planet_id);
    if (raw_energy_pressure < 0.0)
        raw_energy_pressure = 0.0;
    double normalized_energy = raw_energy_pressure / 4.0;
    double generation = this->_buildings.get_planet_energy_generation(planet_id);
    double consumption = this->_buildings.get_planet_energy_consumption(planet_id);
    if (generation > 0.0)
    {
        double ratio = consumption / generation;
        if (ratio > 0.95)
        {
            double extra = (ratio - 0.95) * 10.0;
            normalized_energy += extra;
        }
    }
    if (normalized_energy > 1.5)
        normalized_energy = 1.5;
    double narrative_pressure = 0.0;
    int active_quest = this->_quests.get_active_quest_id();
    if (active_quest == QUEST_CLIMACTIC_BATTLE || active_quest == QUEST_CRITICAL_DECISION)
        narrative_pressure = 0.35;
    else if (active_quest == QUEST_ORDER_UPRISING || active_quest == QUEST_REBELLION_FLEET)
        narrative_pressure = 0.4;
    else if (active_quest != 0)
        narrative_pressure = 0.2;
    double scaled = difficulty * this->_assault_difficulty_multiplier;
    if (scaled <= 0.0)
        scaled = difficulty;
    if (!this->_combat.start_raider_assault(planet_id, scaled, normalized_energy, narrative_pressure, control_mode))
        return false;
    this->get_planet_fleet(planet_id);
    ft_string entry("Navigator Zara signals a raider incursion on planet ");
    entry.append(ft_to_string(planet_id));
    entry.append(ft_string(": defenses are mobilizing."));
    this->_lore_log.push_back(entry);
    return true;
}

bool Game::assign_fleet_to_assault(int planet_id, int fleet_id)
{
    if (!this->_combat.is_assault_active(planet_id))
        return false;
    Pair<int, ft_sharedptr<ft_fleet> > *entry = this->_fleets.find(fleet_id);
    if (entry == ft_nullptr || !entry->value)
        return false;
    return this->_combat.add_fleet(planet_id, fleet_id);
}

bool Game::set_assault_support(int planet_id, bool sunflare_docked,
                               bool repair_drones_active, bool shield_generator_online)
{
    if (repair_drones_active && !this->_repair_drones_unlocked)
        return false;
    if (shield_generator_online && !this->_shield_support_unlocked)
        return false;
    return this->_combat.set_support(planet_id, sunflare_docked, repair_drones_active, shield_generator_online);
}

bool Game::set_assault_control_mode(int planet_id, int control_mode)
{
    return this->_combat.set_control_mode(planet_id, control_mode);
}

bool Game::set_assault_aggression(int planet_id, double aggression)
{
    return this->_combat.set_raider_aggression(planet_id, aggression);
}

bool Game::trigger_assault_focus_fire(int planet_id)
{
    return this->_combat.trigger_focus_fire(planet_id);
}

bool Game::request_assault_tactical_pause(int planet_id)
{
    return this->_combat.request_tactical_pause(planet_id);
}

bool Game::is_assault_active(int planet_id) const
{
    return this->_combat.is_assault_active(planet_id);
}

double Game::get_assault_raider_shield(int planet_id) const
{
    return this->_combat.get_raider_shield(planet_id);
}

double Game::get_assault_raider_hull(int planet_id) const
{
    return this->_combat.get_raider_hull(planet_id);
}

bool Game::get_assault_raider_positions(int planet_id, ft_vector<ft_ship_spatial_state> &out) const
{
    return this->_combat.get_raider_positions(planet_id, out);
}

bool Game::get_assault_defender_positions(int planet_id, ft_vector<ft_ship_spatial_state> &out) const
{
    return this->_combat.get_defender_positions(planet_id, out);
}

const ft_vector<ft_string> &Game::get_lore_log() const
{
    return this->_lore_log;
}

