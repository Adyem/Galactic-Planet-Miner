#include "game.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Template/pair.hpp"

Game::Game(const ft_string &host, const ft_string &path, int difficulty)
    : _backend(host, path),
      _save_system(),
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
      _last_checkpoint_tag(),
      _has_checkpoint(false)
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
    this->save_campaign_checkpoint(ft_string("initial_setup"));
}

Game::~Game()
{
    return ;
}

void Game::produce(double seconds)
{
    size_t planet_count = this->_planets.size();
    Pair<int, ft_sharedptr<ft_planet> > *entries = this->_planets.end();
    entries -= planet_count;
    for (size_t i = 0; i < planet_count; ++i)
    {
        int planet_id = entries[i].key;
        ft_sharedptr<ft_planet> planet = entries[i].value;
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
                    int target = base_amount;
                    if (this->_resource_multiplier < 1.0)
                    {
                        const double epsilon = 0.0000001;
                        Pair<int, ft_map<int, double> > *planet_deficits = this->_resource_deficits.find(planet_id);
                        if (planet_deficits == ft_nullptr)
                        {
                            ft_map<int, double> new_deficits;
                            this->_resource_deficits.insert(planet_id, new_deficits);
                            planet_deficits = this->_resource_deficits.find(planet_id);
                        }
                        Pair<int, double> *ore_deficit = ft_nullptr;
                        if (planet_deficits != ft_nullptr)
                        {
                            ore_deficit = planet_deficits->value.find(ore_id);
                            if (ore_deficit == ft_nullptr)
                            {
                                planet_deficits->value.insert(ore_id, 0.0);
                                ore_deficit = planet_deficits->value.find(ore_id);
                            }
                        }
                        double carryover = 0.0;
                        if (ore_deficit != ft_nullptr)
                            carryover = ore_deficit->value;
                        double total = scaled_amount + carryover;
                        target = static_cast<int>(total + epsilon);
                        double remainder = total - static_cast<double>(target);
                        if (remainder < 0.0)
                            remainder = 0.0;
                        if (ore_deficit != ft_nullptr)
                            ore_deficit->value = remainder;
                        if (target > base_amount)
                            target = base_amount;
                        if (target < 0)
                            target = 0;
                    }
                    else
                    {
                        int scaled_target = static_cast<int>(scaled_amount);
                        target = scaled_target;
                        if (this->_resource_multiplier > 1.0)
                        {
                            double fractional = scaled_amount - static_cast<double>(scaled_target);
                            if (fractional > 0.000001)
                                target += 1;
                        }
                        if (target < 0)
                            target = 0;
                    }
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
    this->process_supply_contracts(seconds);
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
        if (planet_id == this->_order_branch_pending_assault)
        {
            this->_order_branch_assault_victories += 1;
            this->_order_branch_pending_assault = 0;
            ft_string branch_entry("Marshal Rhea honors the Dominion strike force triumphant at planet ");
            branch_entry.append(ft_to_string(planet_id));
            branch_entry.append(ft_string("."));
            this->_lore_log.push_back(branch_entry);
        }
        if (planet_id == this->_rebellion_branch_pending_assault)
        {
            this->_rebellion_branch_assault_victories += 1;
            this->_rebellion_branch_pending_assault = 0;
            ft_string branch_entry("Captain Blackthorne salutes the liberated crews holding planet ");
            branch_entry.append(ft_to_string(planet_id));
            branch_entry.append(ft_string("."));
            this->_lore_log.push_back(branch_entry);
        }
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
        if (planet_id == this->_order_branch_pending_assault)
        {
            if (this->get_quest_status(QUEST_ORDER_DOMINION) == QUEST_STATUS_ACTIVE)
                this->trigger_branch_assault(planet_id, 1.2, true);
            else
                this->_order_branch_pending_assault = 0;
        }
        if (planet_id == this->_rebellion_branch_pending_assault)
        {
            if (this->get_quest_status(QUEST_REBELLION_LIBERATION) == QUEST_STATUS_ACTIVE)
                this->trigger_branch_assault(planet_id, 1.1, false);
            else
                this->_rebellion_branch_pending_assault = 0;
        }
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

void Game::record_achievement_event(int event_id, int value)
{
    ft_vector<int> completed;
    this->_achievements.record_event(event_id, value, &completed);
    if (completed.size() == 0)
        return ;
    this->announce_achievements(completed);
}

int Game::get_quest_achievement_event(int quest_id) const
{
    if (quest_id == QUEST_INITIAL_SKIRMISHES)
        return ACHIEVEMENT_EVENT_QUEST_INITIAL_SKIRMISHES;
    if (quest_id == QUEST_DEFENSE_OF_TERRA)
        return ACHIEVEMENT_EVENT_QUEST_DEFENSE_OF_TERRA;
    if (quest_id == QUEST_INVESTIGATE_RAIDERS)
        return ACHIEVEMENT_EVENT_QUEST_INVESTIGATE_RAIDERS;
    if (quest_id == QUEST_SECURE_SUPPLY_LINES)
        return ACHIEVEMENT_EVENT_QUEST_SECURE_SUPPLY_LINES;
    if (quest_id == QUEST_STEADY_SUPPLY_STREAK)
        return ACHIEVEMENT_EVENT_QUEST_STEADY_SUPPLY_STREAK;
    if (quest_id == QUEST_HIGH_VALUE_ESCORT)
        return ACHIEVEMENT_EVENT_QUEST_HIGH_VALUE_ESCORT;
    if (quest_id == QUEST_CLIMACTIC_BATTLE)
        return ACHIEVEMENT_EVENT_QUEST_CLIMACTIC_BATTLE;
    if (quest_id == QUEST_CRITICAL_DECISION)
        return ACHIEVEMENT_EVENT_QUEST_CRITICAL_DECISION;
    if (quest_id == QUEST_ORDER_UPRISING)
        return ACHIEVEMENT_EVENT_QUEST_ORDER_UPRISING;
    if (quest_id == QUEST_REBELLION_FLEET)
        return ACHIEVEMENT_EVENT_QUEST_REBELLION_FLEET;
    return 0;
}

void Game::record_quest_achievement(int quest_id)
{
    int event_id = this->get_quest_achievement_event(quest_id);
    if (event_id != 0)
        this->record_achievement_event(event_id, 1);
}

void Game::announce_achievements(const ft_vector<int> &achievement_ids)
{
    for (size_t i = 0; i < achievement_ids.size(); ++i)
    {
        ft_achievement_info info;
        if (!this->_achievements.get_info(achievement_ids[i], info))
            continue;
        ft_string entry("Archivist Lyra records achievement ");
        entry.append(info.name);
        entry.append(ft_string(": "));
        entry.append(info.description);
        this->_lore_log.push_back(entry);
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

void Game::get_achievement_ids(ft_vector<int> &out) const
{
    this->_achievements.get_achievement_ids(out);
}

int Game::get_achievement_status(int achievement_id) const
{
    return this->_achievements.get_status(achievement_id);
}

int Game::get_achievement_progress(int achievement_id) const
{
    return this->_achievements.get_progress(achievement_id);
}

int Game::get_achievement_target(int achievement_id) const
{
    return this->_achievements.get_target(achievement_id);
}

bool Game::get_achievement_info(int achievement_id, ft_achievement_info &out) const
{
    return this->_achievements.get_info(achievement_id, out);
}

void Game::apply_planet_snapshot(const ft_map<int, ft_sharedptr<ft_planet> > &snapshot)
{
    size_t count = snapshot.size();
    if (count == 0)
        return ;
    const Pair<int, ft_sharedptr<ft_planet> > *entries = snapshot.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        int planet_id = entries[i].key;
        const ft_sharedptr<ft_planet> &saved_planet = entries[i].value;
        if (!saved_planet)
            continue;
        ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
        if (!planet)
        {
            this->unlock_planet(planet_id);
            planet = this->get_planet(planet_id);
        }
        if (!planet)
            continue;
        const ft_vector<Pair<int, double> > &saved_rates = saved_planet->get_resources();
        for (size_t j = 0; j < saved_rates.size(); ++j)
            planet->register_resource(saved_rates[j].key, saved_rates[j].value);
        const ft_vector<Pair<int, double> > &saved_carryover = saved_planet->get_carryover();
        for (size_t j = 0; j < saved_carryover.size(); ++j)
            planet->set_carryover(saved_carryover[j].key, saved_carryover[j].value);
        for (size_t j = 0; j < saved_rates.size(); ++j)
        {
            int ore_id = saved_rates[j].key;
            planet->set_resource(ore_id, saved_planet->get_resource(ore_id));
            this->send_state(planet_id, ore_id);
        }
        this->_resource_deficits.remove(planet_id);
    }
}

void Game::apply_fleet_snapshot(const ft_map<int, ft_sharedptr<ft_fleet> > &snapshot)
{
    ft_vector<int> to_remove;
    size_t existing_count = this->_fleets.size();
    if (existing_count > 0)
    {
        const Pair<int, ft_sharedptr<ft_fleet> > *existing_entries = this->_fleets.end();
        existing_entries -= existing_count;
        for (size_t i = 0; i < existing_count; ++i)
        {
            int fleet_id = existing_entries[i].key;
            if (snapshot.find(fleet_id) == ft_nullptr)
                to_remove.push_back(fleet_id);
        }
    }
    for (size_t i = 0; i < to_remove.size(); ++i)
        this->remove_fleet(to_remove[i], -1, -1);

    size_t count = snapshot.size();
    if (count == 0)
        return ;
    const Pair<int, ft_sharedptr<ft_fleet> > *entries = snapshot.end();
    entries -= count;
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
            if (ship)
                fleet->add_ship_snapshot(*ship);
        }
    }
}

void Game::checkpoint_campaign_state_internal(const ft_string &tag)
{
    ft_string planet_json = this->_save_system.serialize_planets(this->_planets);
    ft_string fleet_json = this->_save_system.serialize_fleets(this->_fleets);
    bool planets_valid = (planet_json.size() > 0) || (this->_planets.size() == 0);
    bool fleets_valid = (fleet_json.size() > 0) || (this->_fleets.size() == 0);
    if (!planets_valid || !fleets_valid)
        return ;
    this->_last_planet_checkpoint = planet_json;
    this->_last_fleet_checkpoint = fleet_json;
    this->_last_checkpoint_tag = tag;
    this->_has_checkpoint = true;
}

void Game::save_campaign_checkpoint(const ft_string &tag) noexcept
{
    this->checkpoint_campaign_state_internal(tag);
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

bool Game::reload_campaign_checkpoint() noexcept
{
    if (!this->_has_checkpoint)
        return false;
    return this->load_campaign_from_save(this->_last_planet_checkpoint, this->_last_fleet_checkpoint);
}

bool Game::load_campaign_from_save(const ft_string &planet_json, const ft_string &fleet_json) noexcept
{
    ft_map<int, ft_sharedptr<ft_planet> > planet_snapshot;
    ft_map<int, ft_sharedptr<ft_fleet> > fleet_snapshot;
    bool planets_ok = true;
    bool fleets_ok = true;
    if (planet_json.size() > 0)
        planets_ok = this->_save_system.deserialize_planets(planet_json.c_str(), planet_snapshot);
    if (fleet_json.size() > 0)
        fleets_ok = this->_save_system.deserialize_fleets(fleet_json.c_str(), fleet_snapshot);
    if (!planets_ok || !fleets_ok)
        return false;
    this->apply_planet_snapshot(planet_snapshot);
    this->apply_fleet_snapshot(fleet_snapshot);
    return true;
}

