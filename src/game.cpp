#include "game.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Template/pair.hpp"

Game::Game(const ft_string &host, const ft_string &path)
    : _backend(host, path)
{
    ft_sharedptr<ft_planet> terra(new ft_planet_terra());
    ft_sharedptr<ft_planet> mars(new ft_planet_mars());
    ft_sharedptr<ft_planet> zalthor(new ft_planet_zalthor());
    ft_sharedptr<ft_planet> vulcan(new ft_planet_vulcan());
    ft_sharedptr<ft_planet> noctaris(new ft_planet_noctaris_prime());

    this->_state.add_character(terra);
    this->_planets.insert(PLANET_TERRA, terra);
    this->_buildings.initialize_planet(*this, PLANET_TERRA);

    this->_locked_planets.insert(PLANET_MARS, mars);
    this->_locked_planets.insert(PLANET_ZALTHOR, zalthor);
    this->_locked_planets.insert(PLANET_VULCAN, vulcan);
    this->_locked_planets.insert(PLANET_NOCTARIS_PRIME, noctaris);
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
        PLANET_NOCTARIS_PRIME
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
            this->send_state(planet_id, produced[j].key);
            if (mine_multiplier > 1.0)
            {
                double bonus_amount = (mine_multiplier - 1.0) * static_cast<double>(produced[j].value);
                int bonus = static_cast<int>(bonus_amount);
                if (bonus > 0)
                {
                    planet->add_resource(produced[j].key, bonus);
                    this->send_state(planet_id, produced[j].key);
                }
            }
        }
    }
}

void Game::tick(double seconds)
{
    this->produce(seconds);
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
    (void)quest_completed;
    (void)quest_failed;
    (void)quest_choices;

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
    for (size_t i = 0; i < definition->unlock_planets.size(); ++i)
        this->unlock_planet(definition->unlock_planets[i]);
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
    int available = from->get_resource(ore_id);
    if (amount > available)
        amount = available;
    if (amount <= 0)
        return 0;
    from->sub_resource(ore_id, amount);
    to->add_resource(ore_id, amount);
    this->send_state(from_planet_id, ore_id);
    this->send_state(to_planet_id, ore_id);
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
    if (target_fleet_id >= 0)
    {
        ft_sharedptr<ft_fleet> target = this->get_fleet(target_fleet_id);
        if (target)
            fleet->move_ships_to(*target);
    }
    else if (target_planet_id >= 0)
    {
        ft_sharedptr<ft_fleet> target = this->get_planet_fleet(target_planet_id);
        if (target)
            fleet->move_ships_to(*target);
    }
    this->_fleets.remove(fleet_id);
}

int Game::create_ship(int fleet_id, int ship_type)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return 0;
    return fleet->create_ship(ship_type);
}

void Game::remove_ship(int fleet_id, int ship_uid)
{
    ft_sharedptr<ft_fleet> fleet = this->get_fleet(fleet_id);
    if (!fleet)
        return ;
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
    return fleet->sub_ship_hp(ship_uid, amount);
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

bool Game::start_raider_assault(int planet_id, double difficulty)
{
    if (!this->is_planet_unlocked(planet_id))
        return false;
    if (!this->_combat.start_raider_assault(planet_id, difficulty))
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
    return this->_combat.set_support(planet_id, sunflare_docked, repair_drones_active, shield_generator_online);
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

const ft_vector<ft_string> &Game::get_lore_log() const
{
    return this->_lore_log;
}

