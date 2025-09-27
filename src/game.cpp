#include "game.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Template/pair.hpp"
#include "../libft/Template/set.hpp"
#include "../libft/Math/math.hpp"
#include "ft_map_snapshot.hpp"

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

Game::Game(const ft_string &host, const ft_string &path, int difficulty)
    : _backend(host, path),
      _save_system(),
      _lore_log(),
      _lore_log_start(0),
      _lore_log_count(0),
      _lore_log_cache(),
      _lore_log_cache_dirty(false),
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
    this->save_campaign_checkpoint(ft_string("initial_setup"));
}

Game::~Game()
{
    return ;
}

void Game::produce(double seconds)
{
    ft_vector<Pair<int, ft_sharedptr<ft_planet> > > planet_entries;
    ft_map_snapshot(this->_planets, planet_entries);
    size_t planet_count = planet_entries.size();
    for (size_t i = 0; i < planet_count; ++i)
    {
        int planet_id = planet_entries[i].key;
        ft_sharedptr<ft_planet> planet = planet_entries[i].value;
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
                        Pair<int, ft_resource_accumulator> *ore_accumulator = this->get_resource_accumulator(planet_id, ore_id, true);
                        double carryover = 0.0;
                        if (ore_accumulator != ft_nullptr)
                            carryover = ore_accumulator->value.multiplier_deficit;
                        double total = scaled_amount + carryover;
                        target = static_cast<int>(total + epsilon);
                        double remainder = total - static_cast<double>(target);
                        if (remainder < 0.0)
                            remainder = 0.0;
                        if (ore_accumulator != ft_nullptr)
                            ore_accumulator->value.multiplier_deficit = remainder;
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
            if (mine_multiplier > 1.0 && final_amount > 0)
            {
                Pair<int, ft_resource_accumulator> *ore_accumulator = this->get_resource_accumulator(planet_id, ore_id, false);
                double bonus_amount = (mine_multiplier - 1.0) * static_cast<double>(final_amount);
                if (ore_accumulator != ft_nullptr)
                    bonus_amount += ore_accumulator->value.mine_bonus_remainder;
                int bonus = static_cast<int>(bonus_amount);
                double remainder = bonus_amount - static_cast<double>(bonus);
                if (remainder < 0.0)
                    remainder = 0.0;
                if (ore_accumulator == ft_nullptr)
                {
                    if (bonus > 0 || remainder > 0.0)
                        ore_accumulator = this->get_resource_accumulator(planet_id, ore_id, true);
                }
                if (bonus > 0)
                {
                    planet->add_resource(ore_id, bonus);
                }
                if (ore_accumulator != ft_nullptr)
                    ore_accumulator->value.mine_bonus_remainder = remainder;
            }
            this->send_state(planet_id, ore_id);
        }
    }
}

void Game::update_gameplay(double milliseconds)
{
    double seconds = milliseconds * 0.001;
    if (seconds < 0.0)
        seconds = 0.0;
    this->produce(seconds);
    this->process_supply_contracts(seconds);
    this->advance_convoys(seconds);
    this->_buildings.tick(*this, seconds);
    ft_vector<Pair<int, ft_sharedptr<ft_fleet> > > fleet_entries;
    ft_map_snapshot(this->_fleets, fleet_entries);
    size_t count = fleet_entries.size();
    for (size_t i = 0; i < count; ++i)
    {
        ft_sharedptr<ft_fleet> fleet = fleet_entries[i].value;
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
}

void Game::update_combat(double milliseconds)
{
    double seconds = milliseconds * 0.001;
    if (seconds < 0.0)
        seconds = 0.0;
    ft_vector<int> assault_completed;
    ft_vector<int> assault_failed;
    this->_combat.tick(seconds, this->_fleets, this->_planet_fleets, assault_completed, assault_failed);
    for (size_t i = 0; i < assault_completed.size(); ++i)
    {
        int planet_id = assault_completed[i];
        ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
        if (planet)
        {
            int reward_ore = this->select_planet_resource_for_assault(planet, 0, true);
            if (reward_ore != 0)
            {
                planet->add_resource(reward_ore, 3);
                this->send_state(planet_id, reward_ore);
            }
        }
        ft_string entry("Old Miner Joe records a victory at planet ");
        entry.append(ft_to_string(planet_id));
        entry.append(ft_string(": the raider fleet was repelled."));
        this->append_lore_entry(entry);
        if (planet_id == this->_order_branch_pending_assault)
        {
            this->_order_branch_assault_victories += 1;
            this->_order_branch_pending_assault = 0;
            ft_string branch_entry("Marshal Rhea honors the Dominion strike force triumphant at planet ");
            branch_entry.append(ft_to_string(planet_id));
            branch_entry.append(ft_string("."));
            this->append_lore_entry(branch_entry);
        }
        if (planet_id == this->_rebellion_branch_pending_assault)
        {
            this->_rebellion_branch_assault_victories += 1;
            this->_rebellion_branch_pending_assault = 0;
            ft_string branch_entry("Captain Blackthorne salutes the liberated crews holding planet ");
            branch_entry.append(ft_to_string(planet_id));
            branch_entry.append(ft_string("."));
            this->append_lore_entry(branch_entry);
        }
    }
    for (size_t i = 0; i < assault_failed.size(); ++i)
    {
        int planet_id = assault_failed[i];
        ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
        if (planet)
        {
            int penalty_ore = this->select_planet_resource_for_assault(planet, 2, false);
            if (penalty_ore != 0)
            {
                planet->sub_resource(penalty_ore, 2);
                this->send_state(planet_id, penalty_ore);
            }
        }
        ft_string entry("Professor Lumen warns of losses on planet ");
        entry.append(ft_to_string(planet_id));
        entry.append(ft_string(": raiders breached the defenses."));
        this->append_lore_entry(entry);
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

void Game::tick_milliseconds(double milliseconds)
{
    if (milliseconds < 0.0)
        milliseconds = 0.0;
    this->update_gameplay(milliseconds);
    this->update_combat(milliseconds);
}

void Game::tick(double seconds)
{
    double milliseconds = seconds * 1000.0;
    this->tick_milliseconds(milliseconds);
}

int Game::select_planet_resource_for_assault(const ft_sharedptr<ft_planet> &planet, int minimum_stock, bool allow_stock_fallback) const noexcept
{
    if (!planet)
        return 0;
    const ft_vector<Pair<int, double> > &resources = planet->get_resources();
    int selected_ore = 0;
    double selected_rate = 0.0;
    int selected_stock = 0;
    bool has_rate_candidate = false;
    const double epsilon = 0.0000001;
    for (size_t i = 0; i < resources.size(); ++i)
    {
        int ore_id = resources[i].key;
        double rate = resources[i].value;
        if (rate <= 0.0)
            continue;
        int stock = planet->get_resource(ore_id);
        if (stock < minimum_stock)
            continue;
        if (!has_rate_candidate || rate > selected_rate + epsilon)
        {
            selected_ore = ore_id;
            selected_rate = rate;
            selected_stock = stock;
            has_rate_candidate = true;
        }
        else if (math_fabs(rate - selected_rate) <= epsilon && stock > selected_stock)
        {
            selected_ore = ore_id;
            selected_stock = stock;
        }
    }
    if (has_rate_candidate)
        return selected_ore;
    if (!allow_stock_fallback)
        return 0;
    int fallback_ore = 0;
    int fallback_stock = 0;
    for (size_t i = 0; i < resources.size(); ++i)
    {
        int ore_id = resources[i].key;
        int stock = planet->get_resource(ore_id);
        if (stock < minimum_stock)
            continue;
        if (stock > fallback_stock)
        {
            fallback_ore = ore_id;
            fallback_stock = stock;
        }
    }
    return fallback_ore;
}

Pair<int, Game::ft_resource_accumulator> *Game::get_resource_accumulator(int planet_id, int ore_id, bool create)
{
    Pair<int, ft_sharedptr<ft_vector<Pair<int, ft_resource_accumulator> > > > *planet_entry = this->_resource_deficits.find(planet_id);
    if (planet_entry == ft_nullptr)
    {
        if (!create)
            return ft_nullptr;
        ft_sharedptr<ft_vector<Pair<int, ft_resource_accumulator> > > new_accumulators(new ft_vector<Pair<int, ft_resource_accumulator> >());
        this->_resource_deficits.insert(planet_id, new_accumulators);
        planet_entry = this->_resource_deficits.find(planet_id);
        if (planet_entry == ft_nullptr)
            return ft_nullptr;
    }
    ft_sharedptr<ft_vector<Pair<int, ft_resource_accumulator> > > &vector_ptr = planet_entry->value;
    if (!vector_ptr)
    {
        if (!create)
            return ft_nullptr;
        ft_sharedptr<ft_vector<Pair<int, ft_resource_accumulator> > > replacement(new ft_vector<Pair<int, ft_resource_accumulator> >());
        planet_entry->value = replacement;
        vector_ptr = planet_entry->value;
    }
    if (!vector_ptr)
        return ft_nullptr;
    ft_vector<Pair<int, ft_resource_accumulator> > &entries = *vector_ptr;
    for (size_t i = 0; i < entries.size(); ++i)
    {
        if (entries[i].key == ore_id)
            return &entries[i];
    }
    if (!create)
        return ft_nullptr;
    Pair<int, ft_resource_accumulator> new_entry;
    new_entry.key = ore_id;
    new_entry.value = ft_resource_accumulator();
    entries.push_back(new_entry);
    return &entries[entries.size() - 1];
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

ft_sharedptr<ft_planet> Game::get_planet_storage_target(int id)
{
    Pair<int, ft_sharedptr<ft_planet> > *entry = this->_planets.find(id);
    if (entry != ft_nullptr)
        return entry->value;
    Pair<int, ft_sharedptr<ft_planet> > *locked = this->_locked_planets.find(id);
    if (locked != ft_nullptr)
        return locked->value;
    return ft_sharedptr<ft_planet>();
}

ft_sharedptr<const ft_planet> Game::get_planet_storage_target(int id) const
{
    const Pair<int, ft_sharedptr<ft_planet> > *entry = this->_planets.find(id);
    if (entry != ft_nullptr)
        return entry->value;
    const Pair<int, ft_sharedptr<ft_planet> > *locked = this->_locked_planets.find(id);
    if (locked != ft_nullptr)
        return locked->value;
    return ft_sharedptr<const ft_planet>();
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

void Game::queue_pending_resource_update(int planet_id, int ore_id, int amount)
{
    Pair<int, ft_sharedptr<ft_map<int, int> > > *planet_entry = this->_pending_resource_updates.find(planet_id);
    if (planet_entry == ft_nullptr)
    {
        ft_sharedptr<ft_map<int, int> > ore_map(new ft_map<int, int>());
        this->_pending_resource_updates.insert(planet_id, ore_map);
        planet_entry = this->_pending_resource_updates.find(planet_id);
    }
    if (planet_entry == ft_nullptr)
        return ;
    ft_sharedptr<ft_map<int, int> > &ore_map_ptr = planet_entry->value;
    if (!ore_map_ptr)
    {
        ft_sharedptr<ft_map<int, int> > replacement(new ft_map<int, int>());
        planet_entry->value = replacement;
    }
    ft_sharedptr<ft_map<int, int> > &ore_map_ref = planet_entry->value;
    Pair<int, int> *ore_entry = ore_map_ref->find(ore_id);
    if (ore_entry == ft_nullptr)
        ore_map_ref->insert(ore_id, amount);
    else
        ore_entry->value = amount;
}

void Game::clear_pending_resource_update(int planet_id, int ore_id)
{
    Pair<int, ft_sharedptr<ft_map<int, int> > > *planet_entry = this->_pending_resource_updates.find(planet_id);
    if (planet_entry == ft_nullptr)
        return ;
    ft_sharedptr<ft_map<int, int> > ore_map_ptr = planet_entry->value;
    if (!ore_map_ptr)
    {
        this->_pending_resource_updates.remove(planet_id);
        return ;
    }
    ore_map_ptr->remove(ore_id);
    if (ore_map_ptr->size() == 0)
        this->_pending_resource_updates.remove(planet_id);
}

bool Game::dispatch_resource_update(int planet_id, int ore_id, int amount)
{
    ft_string body("{\"planet\":");
    body.append(ft_to_string(planet_id));
    body.append(",\"ore\":");
    body.append(ft_to_string(ore_id));
    body.append(",\"amount\":");
    body.append(ft_to_string(amount));
    body.append("}");
    ft_string response;
    int status = this->_backend.send_state(body, response);
    bool offline = (status < 200 || status >= 400);
    const ft_string fallback_prefix("[offline] echo=");
    size_t prefix_size = fallback_prefix.size();
    if (!offline && response.size() >= prefix_size)
    {
        const char *resp_cstr = response.c_str();
        if (ft_strncmp(resp_cstr, fallback_prefix.c_str(), static_cast<size_t>(prefix_size)) == 0)
            offline = true;
    }
    if (offline)
    {
        long retry_now_ms = ft_time_ms();
        if (this->_backend_retry_delay_ms <= 0)
            this->_backend_retry_delay_ms = BACKEND_RETRY_INITIAL_DELAY_MS;
        else
        {
            long doubled_delay = this->_backend_retry_delay_ms * 2;
            if (doubled_delay > BACKEND_RETRY_MAX_DELAY_MS)
                doubled_delay = BACKEND_RETRY_MAX_DELAY_MS;
            this->_backend_retry_delay_ms = doubled_delay;
        }
        this->_backend_next_retry_ms = retry_now_ms + this->_backend_retry_delay_ms;
        bool was_online = this->_backend_online;
        this->_backend_online = false;
        if (was_online)
        {
            ft_string entry("Operations report: backend connection lost");
            entry.append(" (status ");
            entry.append(ft_to_string(status));
            entry.append("). Switching to offline mode.");
            this->append_lore_entry(entry);
        }
        return false;
    }

    this->_backend_retry_delay_ms = 0;
    this->_backend_next_retry_ms = 0;
    bool was_offline = !this->_backend_online;
    this->_backend_online = true;
    if (was_offline)
    {
        ft_string entry("Operations report: backend connection restored.");
        this->append_lore_entry(entry);
    }

    Pair<int, ft_sharedptr<ft_map<int, int> > > *planet_entry = this->_last_sent_resources.find(planet_id);
    if (planet_entry == ft_nullptr)
    {
        ft_sharedptr<ft_map<int, int> > ore_map(new ft_map<int, int>());
        this->_last_sent_resources.insert(planet_id, ore_map);
        planet_entry = this->_last_sent_resources.find(planet_id);
    }
    if (planet_entry != ft_nullptr)
    {
        ft_sharedptr<ft_map<int, int> > &ore_map_ptr = planet_entry->value;
        if (!ore_map_ptr)
        {
            ft_sharedptr<ft_map<int, int> > replacement(new ft_map<int, int>());
            planet_entry->value = replacement;
        }
        ft_sharedptr<ft_map<int, int> > &ore_map_ref = planet_entry->value;
        Pair<int, int> *ore_entry = ore_map_ref->find(ore_id);
        if (ore_entry == ft_nullptr)
            ore_map_ref->insert(ore_id, amount);
        else
            ore_entry->value = amount;
    }
    return true;
}

bool Game::flush_pending_resource_updates()
{
    ft_vector<Pair<int, ft_sharedptr<ft_map<int, int> > > > planet_entries;
    ft_map_snapshot(this->_pending_resource_updates, planet_entries);
    size_t pending_planet_count = planet_entries.size();
    if (pending_planet_count == 0)
        return true;

    ft_vector<ft_pending_resource_update> updates;
    for (size_t i = 0; i < pending_planet_count; ++i)
    {
        int planet_id = planet_entries[i].key;
        ft_sharedptr<ft_map<int, int> > ore_map_ptr = planet_entries[i].value;
        if (!ore_map_ptr)
            continue;
        ft_vector<Pair<int, int> > ore_entries;
        ft_map_snapshot(*ore_map_ptr, ore_entries);
        size_t ore_count = ore_entries.size();
        if (ore_count == 0)
            continue;
        for (size_t j = 0; j < ore_count; ++j)
        {
            ft_pending_resource_update update(planet_id, ore_entries[j].key, ore_entries[j].value);
            updates.push_back(update);
        }
    }

    size_t update_count = updates.size();
    for (size_t index = 0; index < update_count; ++index)
    {
        const ft_pending_resource_update &update = updates[index];
        if (!this->dispatch_resource_update(update.planet_id, update.ore_id, update.amount))
            return false;
        this->clear_pending_resource_update(update.planet_id, update.ore_id);
    }
    return true;
}

void Game::send_state(int planet_id, int ore_id)
{
    ft_sharedptr<const ft_planet> planet = this->get_planet(planet_id);
    if (!planet)
        return ;

    int amount = planet->get_resource(ore_id);
    if (this->_backend_online)
    {
        Pair<int, ft_sharedptr<ft_map<int, int> > > *planet_entry = this->_last_sent_resources.find(planet_id);
        if (planet_entry != ft_nullptr)
        {
            ft_sharedptr<ft_map<int, int> > &ore_map_ptr = planet_entry->value;
            if (ore_map_ptr)
            {
                Pair<int, int> *ore_entry = ore_map_ptr->find(ore_id);
                if (ore_entry != ft_nullptr && ore_entry->value == amount)
                    return ;
            }
        }
    }

    this->queue_pending_resource_update(planet_id, ore_id, amount);

    if (!this->_backend_online)
    {
        long current_time_ms = ft_time_ms();
        if (this->_backend_next_retry_ms != 0 && current_time_ms < this->_backend_next_retry_ms)
            return ;
    }

    this->flush_pending_resource_updates();
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
        this->append_lore_entry(entry);
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
    ft_sharedptr<ft_planet> planet = this->get_planet_storage_target(planet_id);
    if (!planet)
        return ;
    planet->ensure_item_slot(resource_id);
}

int Game::add_ore(int planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> planet = this->get_planet_storage_target(planet_id);
    if (!planet)
        return 0;
    int total = planet->add_resource(ore_id, amount);
    this->send_state(planet_id, ore_id);
    return total;
}

int Game::sub_ore(int planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> planet = this->get_planet_storage_target(planet_id);
    if (!planet)
        return 0;
    int total = planet->sub_resource(ore_id, amount);
    this->send_state(planet_id, ore_id);
    return total;
}

int Game::get_ore(int planet_id, int ore_id) const
{
    ft_sharedptr<const ft_planet> planet = this->get_planet_storage_target(planet_id);
    if (!planet)
        return 0;
    return planet->get_resource(ore_id);
}

void Game::set_ore(int planet_id, int ore_id, int amount)
{
    ft_sharedptr<ft_planet> planet = this->get_planet_storage_target(planet_id);
    if (!planet)
        return ;
    this->ensure_planet_item_slot(planet_id, ore_id);
    planet->set_resource(ore_id, amount);
    this->send_state(planet_id, ore_id);
}

double Game::get_rate(int planet_id, int ore_id) const
{
    ft_sharedptr<const ft_planet> planet = this->get_planet_storage_target(planet_id);
    if (!planet)
        return 0;
    return planet->get_rate(ore_id);
}

const ft_vector<Pair<int, double> > &Game::get_planet_resources(int planet_id) const
{
    static ft_vector<Pair<int, double> > empty;
    ft_sharedptr<const ft_planet> planet = this->get_planet_storage_target(planet_id);
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
    ft_vector<Pair<int, ft_sharedptr<ft_planet> > > entries;
    ft_map_snapshot(snapshot, entries);
    size_t count = entries.size();
    if (count == 0)
        return ;
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
        {
            int ore_id = saved_rates[j].key;
            if (ore_id <= 0)
                continue;
            planet->register_resource(ore_id, saved_rates[j].value);
        }
        const ft_vector<Pair<int, double> > &saved_carryover = saved_planet->get_carryover();
        for (size_t j = 0; j < saved_carryover.size(); ++j)
        {
            int ore_id = saved_carryover[j].key;
            if (ore_id <= 0)
                continue;
            planet->set_carryover(ore_id, saved_carryover[j].value);
        }
        ft_vector<Pair<int, int> > inventory_snapshot = saved_planet->get_items_snapshot();
        ft_set<int> saved_item_ids(inventory_snapshot.size());
        for (size_t j = 0; j < inventory_snapshot.size(); ++j)
            saved_item_ids.insert(inventory_snapshot[j].key);
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
    if (achievement_snapshot_present)
        this->_achievements.set_progress_state(achievement_state);
    if (building_snapshot_present)
        this->_buildings.clone_from(building_snapshot);
    this->apply_planet_snapshot(planet_snapshot);
    this->apply_fleet_snapshot(fleet_snapshot);
    return true;
}

