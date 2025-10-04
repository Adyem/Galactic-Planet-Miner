#include "game.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/Template/move.hpp"
#include "libft/Template/pair.hpp"

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
    case SHIP_FRIGATE_JUGGERNAUT:
    case SHIP_FRIGATE_CARRIER:
    case SHIP_FRIGATE_SOVEREIGN:
    case SHIP_FRIGATE_PREEMPTOR:
    case SHIP_FRIGATE_PROTECTOR:
    case SHIP_FRIGATE_ECLIPSE:
        if (!this->_research.is_completed(RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT))
            return false;
        return true;
    case SHIP_CAPITAL_NOVA:
    case SHIP_CAPITAL_OBSIDIAN:
    case SHIP_CAPITAL_PREEMPTOR:
    case SHIP_CAPITAL_PROTECTOR:
    case SHIP_CAPITAL_ECLIPSE:
        if (!this->_research.is_completed(RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT))
            return false;
        if (this->_capital_ship_limit <= 0)
            return false;
        return true;
    case SHIP_CAPITAL_JUGGERNAUT:
        if (this->_capital_ship_limit <= 0)
            return false;
        return true;
    case SHIP_RAIDER_CORVETTE:
    case SHIP_RAIDER_DESTROYER:
    case SHIP_RAIDER_BATTLESHIP:
        return false;
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

void Game::build_fleet_management_entry(const ft_fleet &fleet,
    ft_fleet_management_entry &out, bool is_garrison,
    int garrison_planet_id) const
{
    out.fleet_id = fleet.get_id();
    out.is_garrison = is_garrison;
    out.station_planet_id = is_garrison ? garrison_planet_id : 0;
    ft_location location = fleet.get_location();
    out.location_type = location.type;
    out.origin_planet_id = location.from;
    out.destination_planet_id = location.to;
    out.misc_location_id = location.misc;
    out.travel_time = fleet.get_travel_time();
    out.ship_count = fleet.get_ship_count();
    out.total_hp = fleet.get_total_ship_hp();
    out.total_shield = fleet.get_total_ship_shield();
    out.escort_veterancy = fleet.get_escort_veterancy();
    out.escort_veterancy_bonus = fleet.get_escort_veterancy_bonus();
    out.average_weapon_range = 0.0;
    out.average_attack_speed = 0.0;
    out.ships.clear();

    ft_vector<int> ship_ids;
    fleet.get_ship_ids(ship_ids);
    double total_range = 0.0;
    double total_attack_speed = 0.0;
    size_t ship_count = ship_ids.size();
    for (size_t i = 0; i < ship_count; ++i)
    {
        const ft_ship *ship = fleet.get_ship(ship_ids[i]);
        if (!ship)
            continue;
        ft_ship_management_entry entry;
        entry.ship_uid = ship->id;
        entry.ship_type = ship->type;
        entry.role = ship->role;
        entry.combat_behavior = ship->combat_behavior;
        entry.outnumbered_behavior = ship->outnumbered_behavior;
        entry.unescorted_behavior = ship->unescorted_behavior;
        entry.low_hp_behavior = ship->low_hp_behavior;
        entry.armor = ship->armor;
        entry.hp = ship->hp;
        entry.max_hp = ship->max_hp;
        entry.shield = ship->shield;
        entry.max_shield = ship->max_shield;
        entry.max_speed = ship->max_speed;
        entry.acceleration = ship->acceleration;
        entry.deceleration = ship->deceleration;
        entry.turn_speed = ship->turn_speed;
        entry.optimal_range = ship->optimal_range;
        entry.max_range = ship->max_range;
        entry.attack_speed = ship->attack_speed;
        entry.base_damage = ship->base_damage;
        out.ships.push_back(entry);
        total_range += entry.max_range;
        total_attack_speed += entry.attack_speed;
    }
    size_t summary_count = out.ships.size();
    if (summary_count > 0)
    {
        double denominator = static_cast<double>(summary_count);
        out.average_weapon_range = total_range / denominator;
        out.average_attack_speed = total_attack_speed / denominator;
    }
    else
    {
        out.average_weapon_range = 0.0;
        out.average_attack_speed = 0.0;
    }
}

void Game::get_fleet_management_snapshot(ft_fleet_management_snapshot &out) const
{
    out.player_fleets.clear();
    out.traveling_fleets.clear();
    out.planet_garrisons.clear();

    size_t player_count = this->_fleets.size();
    if (player_count > 0)
    {
        const Pair<int, ft_sharedptr<ft_fleet> > *entries = this->_fleets.end();
        entries -= player_count;
        for (size_t i = 0; i < player_count; ++i)
        {
            const ft_sharedptr<ft_fleet> &fleet = entries[i].value;
            if (!fleet)
                continue;
            ft_fleet_management_entry entry;
            this->build_fleet_management_entry(*fleet, entry, false, 0);
            if (entry.location_type == LOCATION_TRAVEL)
                out.traveling_fleets.push_back(ft_move(entry));
            else
                out.player_fleets.push_back(ft_move(entry));
        }
    }

    size_t garrison_count = this->_planet_fleets.size();
    if (garrison_count > 0)
    {
        const Pair<int, ft_sharedptr<ft_fleet> > *entries = this->_planet_fleets.end();
        entries -= garrison_count;
        for (size_t i = 0; i < garrison_count; ++i)
        {
            const ft_sharedptr<ft_fleet> &fleet = entries[i].value;
            if (!fleet)
                continue;
            ft_fleet_management_entry entry;
            this->build_fleet_management_entry(*fleet, entry, true, entries[i].key);
            out.planet_garrisons.push_back(ft_move(entry));
        }
    }
}

