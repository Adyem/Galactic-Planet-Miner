#include "save_system.hpp"
#include "../libft/CMA/CMA.hpp"

namespace
{
    const long SAVE_DOUBLE_SCALE = 1000000;
}

SaveSystem::SaveSystem() noexcept
{
    return ;
}

SaveSystem::~SaveSystem() noexcept
{
    return ;
}

long SaveSystem::scale_double_to_long(double value) const noexcept
{
    double scaled = value * static_cast<double>(SAVE_DOUBLE_SCALE);
    if (scaled >= 0.0)
        scaled += 0.5;
    else
        scaled -= 0.5;
    long result = static_cast<long>(scaled);
    return result;
}

double SaveSystem::unscale_long_to_double(long value) const noexcept
{
    if (SAVE_DOUBLE_SCALE == 0)
        return 0.0;
    double numerator = static_cast<double>(value);
    double denominator = static_cast<double>(SAVE_DOUBLE_SCALE);
    return numerator / denominator;
}

ft_sharedptr<ft_planet> SaveSystem::create_planet_instance(int planet_id) const noexcept
{
    switch (planet_id)
    {
    case PLANET_TERRA:
        return ft_sharedptr<ft_planet>(new ft_planet_terra());
    case PLANET_MARS:
        return ft_sharedptr<ft_planet>(new ft_planet_mars());
    case PLANET_ZALTHOR:
        return ft_sharedptr<ft_planet>(new ft_planet_zalthor());
    case PLANET_VULCAN:
        return ft_sharedptr<ft_planet>(new ft_planet_vulcan());
    case PLANET_NOCTARIS_PRIME:
        return ft_sharedptr<ft_planet>(new ft_planet_noctaris_prime());
    case PLANET_LUNA:
        return ft_sharedptr<ft_planet>(new ft_planet_luna());
    default:
        return ft_sharedptr<ft_planet>(new ft_planet(planet_id));
    }
}

ft_sharedptr<ft_fleet> SaveSystem::create_fleet_instance(int fleet_id) const noexcept
{
    ft_sharedptr<ft_fleet> fleet(new ft_fleet(fleet_id));
    return fleet;
}

ft_string SaveSystem::serialize_planets(const ft_map<int, ft_sharedptr<ft_planet> > &planets) const noexcept
{
    json_document document;
    size_t count = planets.size();
    if (count > 0)
    {
        const Pair<int, ft_sharedptr<ft_planet> > *entries = planets.end();
        entries -= count;
        for (size_t i = 0; i < count; ++i)
        {
            ft_sharedptr<ft_planet> planet = entries[i].value;
            if (!planet)
                continue;
            ft_string group_name = "planet_";
            ft_string id_string = ft_to_string(entries[i].key);
            group_name.append(id_string);
            json_group *group = document.create_group(group_name.c_str());
            if (!group)
                continue;
            document.append_group(group);
            json_item *id_item = document.create_item("id", entries[i].key);
            if (id_item)
                document.add_item(group, id_item);
            const ft_vector<Pair<int, double> > &resources = planet->get_resources();
            for (size_t j = 0; j < resources.size(); ++j)
            {
                int ore_id = resources[j].key;
                ft_string ore_string = ft_to_string(ore_id);
                ft_string amount_key = "resource_";
                amount_key.append(ore_string);
                int amount = planet->get_resource(ore_id);
                json_item *amount_item = document.create_item(amount_key.c_str(), amount);
                if (amount_item)
                    document.add_item(group, amount_item);
                ft_string rate_key = "rate_";
                rate_key.append(ore_string);
                long scaled_rate = this->scale_double_to_long(resources[j].value);
                ft_string rate_value = ft_to_string(scaled_rate);
                json_item *rate_item = document.create_item(rate_key.c_str(), rate_value.c_str());
                if (rate_item)
                    document.add_item(group, rate_item);
            }
            const ft_vector<Pair<int, double> > &carryover = planet->get_carryover();
            for (size_t j = 0; j < carryover.size(); ++j)
            {
                int ore_id = carryover[j].key;
                ft_string ore_string = ft_to_string(ore_id);
                ft_string carry_key = "carryover_";
                carry_key.append(ore_string);
                long scaled_carry = this->scale_double_to_long(carryover[j].value);
                ft_string carry_value = ft_to_string(scaled_carry);
                json_item *carry_item = document.create_item(carry_key.c_str(), carry_value.c_str());
                if (carry_item)
                    document.add_item(group, carry_item);
            }
        }
    }
    char *serialized = document.write_to_string();
    if (!serialized)
        return ft_string();
    ft_string result(serialized);
    cma_free(serialized);
    return result;
}

bool SaveSystem::deserialize_planets(const char *content,
    ft_map<int, ft_sharedptr<ft_planet> > &planets) const noexcept
{
    if (!content)
        return false;
    json_group *groups = json_read_from_string(content);
    if (!groups)
        return false;
    planets.clear();
    json_group *current = groups;
    while (current)
    {
        json_item *id_item = json_find_item(current, "id");
        if (!id_item)
        {
            current = current->next;
            continue;
        }
        int planet_id = ft_atoi(id_item->value);
        ft_sharedptr<ft_planet> planet = this->create_planet_instance(planet_id);
        if (!planet)
        {
            current = current->next;
            continue;
        }
        ft_vector<Pair<int, int> > resource_amounts;
        ft_vector<Pair<int, long> > resource_rates;
        ft_vector<Pair<int, long> > resource_carryover;
        json_item *item = current->items;
        while (item)
        {
            if (item->key && ft_strncmp(item->key, "resource_", 9) == 0)
            {
                int ore_id = ft_atoi(item->key + 9);
                Pair<int, int> entry;
                entry.key = ore_id;
                entry.value = ft_atoi(item->value);
                resource_amounts.push_back(entry);
            }
            else if (item->key && ft_strncmp(item->key, "rate_", 5) == 0)
            {
                int ore_id = ft_atoi(item->key + 5);
                Pair<int, long> entry;
                entry.key = ore_id;
                entry.value = ft_atol(item->value);
                resource_rates.push_back(entry);
            }
            else if (item->key && ft_strncmp(item->key, "carryover_", 10) == 0)
            {
                int ore_id = ft_atoi(item->key + 10);
                Pair<int, long> entry;
                entry.key = ore_id;
                entry.value = ft_atol(item->value);
                resource_carryover.push_back(entry);
            }
            item = item->next;
        }
        for (size_t i = 0; i < resource_rates.size(); ++i)
        {
            int ore_id = resource_rates[i].key;
            double rate = this->unscale_long_to_double(resource_rates[i].value);
            planet->register_resource(ore_id, rate);
        }
        for (size_t i = 0; i < resource_amounts.size(); ++i)
        {
            int ore_id = resource_amounts[i].key;
            planet->register_resource(ore_id, planet->get_rate(ore_id));
            planet->set_resource(ore_id, resource_amounts[i].value);
        }
        for (size_t i = 0; i < resource_carryover.size(); ++i)
        {
            int ore_id = resource_carryover[i].key;
            double carry_value = this->unscale_long_to_double(resource_carryover[i].value);
            planet->set_carryover(ore_id, carry_value);
        }
        planets.insert(planet_id, planet);
        current = current->next;
    }
    json_free_groups(groups);
    return true;
}

ft_string SaveSystem::serialize_fleets(const ft_map<int, ft_sharedptr<ft_fleet> > &fleets) const noexcept
{
    json_document document;
    size_t count = fleets.size();
    if (count > 0)
    {
        const Pair<int, ft_sharedptr<ft_fleet> > *entries = fleets.end();
        entries -= count;
        for (size_t i = 0; i < count; ++i)
        {
            ft_sharedptr<ft_fleet> fleet = entries[i].value;
            if (!fleet)
                continue;
            ft_string group_name = "fleet_";
            ft_string id_string = ft_to_string(entries[i].key);
            group_name.append(id_string);
            json_group *group = document.create_group(group_name.c_str());
            if (!group)
                continue;
            document.append_group(group);
            json_item *id_item = document.create_item("id", entries[i].key);
            if (id_item)
                document.add_item(group, id_item);
            ft_location location = fleet->get_location();
            json_item *type_item = document.create_item("location_type", location.type);
            if (type_item)
                document.add_item(group, type_item);
            json_item *from_item = document.create_item("location_from", location.from);
            if (from_item)
                document.add_item(group, from_item);
            json_item *to_item = document.create_item("location_to", location.to);
            if (to_item)
                document.add_item(group, to_item);
            json_item *misc_item = document.create_item("location_misc", location.misc);
            if (misc_item)
                document.add_item(group, misc_item);
            long travel_scaled = this->scale_double_to_long(fleet->get_travel_time());
            ft_string travel_value = ft_to_string(travel_scaled);
            json_item *travel_item = document.create_item("travel_time", travel_value.c_str());
            if (travel_item)
                document.add_item(group, travel_item);
            long veterancy_scaled = this->scale_double_to_long(fleet->get_escort_veterancy());
            ft_string veterancy_value = ft_to_string(veterancy_scaled);
            json_item *veterancy_item = document.create_item("escort_veterancy", veterancy_value.c_str());
            if (veterancy_item)
                document.add_item(group, veterancy_item);
            int ship_total = fleet->get_ship_count();
            json_item *ship_count_item = document.create_item("ship_count", ship_total);
            if (ship_count_item)
                document.add_item(group, ship_count_item);
            ft_vector<int> ship_ids;
            fleet->get_ship_ids(ship_ids);
            for (size_t j = 0; j < ship_ids.size(); ++j)
            {
                int ship_id = ship_ids[j];
                const ft_ship *ship = fleet->get_ship(ship_id);
                if (!ship)
                    continue;
                ft_string index_string = ft_to_string(static_cast<long>(j));
                ft_string base_key = "ship_";
                base_key.append(index_string);
                ft_string key = base_key;
                key.append("_id");
                json_item *ship_id_item = document.create_item(key.c_str(), ship->id);
                if (ship_id_item)
                    document.add_item(group, ship_id_item);
                key = base_key;
                key.append("_type");
                json_item *ship_type_item = document.create_item(key.c_str(), ship->type);
                if (ship_type_item)
                    document.add_item(group, ship_type_item);
                key = base_key;
                key.append("_armor");
                json_item *armor_item = document.create_item(key.c_str(), ship->armor);
                if (armor_item)
                    document.add_item(group, armor_item);
                key = base_key;
                key.append("_hp");
                json_item *hp_item = document.create_item(key.c_str(), ship->hp);
                if (hp_item)
                    document.add_item(group, hp_item);
                key = base_key;
                key.append("_shield");
                json_item *shield_item = document.create_item(key.c_str(), ship->shield);
                if (shield_item)
                    document.add_item(group, shield_item);
                key = base_key;
                key.append("_max_hp");
                json_item *max_hp_item = document.create_item(key.c_str(), ship->max_hp);
                if (max_hp_item)
                    document.add_item(group, max_hp_item);
                key = base_key;
                key.append("_max_shield");
                json_item *max_shield_item = document.create_item(key.c_str(), ship->max_shield);
                if (max_shield_item)
                    document.add_item(group, max_shield_item);
                key = base_key;
                key.append("_max_speed");
                long max_speed_scaled = this->scale_double_to_long(ship->max_speed);
                ft_string max_speed_value = ft_to_string(max_speed_scaled);
                json_item *max_speed_item = document.create_item(key.c_str(), max_speed_value.c_str());
                if (max_speed_item)
                    document.add_item(group, max_speed_item);
                key = base_key;
                key.append("_acceleration");
                long acceleration_scaled = this->scale_double_to_long(ship->acceleration);
                ft_string acceleration_value = ft_to_string(acceleration_scaled);
                json_item *acceleration_item = document.create_item(key.c_str(), acceleration_value.c_str());
                if (acceleration_item)
                    document.add_item(group, acceleration_item);
                key = base_key;
                key.append("_turn_speed");
                long turn_speed_scaled = this->scale_double_to_long(ship->turn_speed);
                ft_string turn_speed_value = ft_to_string(turn_speed_scaled);
                json_item *turn_speed_item = document.create_item(key.c_str(), turn_speed_value.c_str());
                if (turn_speed_item)
                    document.add_item(group, turn_speed_item);
                key = base_key;
                key.append("_behavior");
                json_item *behavior_item = document.create_item(key.c_str(), ship->combat_behavior);
                if (behavior_item)
                    document.add_item(group, behavior_item);
                key = base_key;
                key.append("_outnumbered");
                json_item *outnumbered_item = document.create_item(key.c_str(), ship->outnumbered_behavior);
                if (outnumbered_item)
                    document.add_item(group, outnumbered_item);
                key = base_key;
                key.append("_unescorted");
                json_item *unescorted_item = document.create_item(key.c_str(), ship->unescorted_behavior);
                if (unescorted_item)
                    document.add_item(group, unescorted_item);
                key = base_key;
                key.append("_low_hp");
                json_item *low_hp_item = document.create_item(key.c_str(), ship->low_hp_behavior);
                if (low_hp_item)
                    document.add_item(group, low_hp_item);
                key = base_key;
                key.append("_role");
                json_item *role_item = document.create_item(key.c_str(), ship->role);
                if (role_item)
                    document.add_item(group, role_item);
            }
        }
    }
    char *serialized = document.write_to_string();
    if (!serialized)
        return ft_string();
    ft_string result(serialized);
    cma_free(serialized);
    return result;
}

bool SaveSystem::deserialize_fleets(const char *content,
    ft_map<int, ft_sharedptr<ft_fleet> > &fleets) const noexcept
{
    if (!content)
        return false;
    json_group *groups = json_read_from_string(content);
    if (!groups)
        return false;
    fleets.clear();
    json_group *current = groups;
    while (current)
    {
        json_item *id_item = json_find_item(current, "id");
        if (!id_item)
        {
            current = current->next;
            continue;
        }
        int fleet_id = ft_atoi(id_item->value);
        ft_sharedptr<ft_fleet> fleet = this->create_fleet_instance(fleet_id);
        if (!fleet)
        {
            current = current->next;
            continue;
        }
        json_item *type_item = json_find_item(current, "location_type");
        int location_type = type_item ? ft_atoi(type_item->value) : LOCATION_PLANET;
        json_item *from_item = json_find_item(current, "location_from");
        int location_from = from_item ? ft_atoi(from_item->value) : PLANET_TERRA;
        json_item *to_item = json_find_item(current, "location_to");
        int location_to = to_item ? ft_atoi(to_item->value) : PLANET_TERRA;
        json_item *misc_item = json_find_item(current, "location_misc");
        int location_misc = misc_item ? ft_atoi(misc_item->value) : 0;
        json_item *travel_item = json_find_item(current, "travel_time");
        long travel_scaled = travel_item ? ft_atol(travel_item->value) : 0;
        json_item *veterancy_item = json_find_item(current, "escort_veterancy");
        long veterancy_scaled = veterancy_item ? ft_atol(veterancy_item->value) : 0;
        if (location_type == LOCATION_TRAVEL)
            fleet->set_location_travel(location_from, location_to,
                this->unscale_long_to_double(travel_scaled));
        else if (location_type == LOCATION_MISC)
            fleet->set_location_misc(location_misc);
        else
            fleet->set_location_planet(location_from);
        fleet->set_escort_veterancy(this->unscale_long_to_double(veterancy_scaled));
        json_item *ship_count_item = json_find_item(current, "ship_count");
        int ship_count = ship_count_item ? ft_atoi(ship_count_item->value) : 0;
        for (int i = 0; i < ship_count; ++i)
        {
            ft_string index_string = ft_to_string(static_cast<long>(i));
            ft_string base_key = "ship_";
            base_key.append(index_string);
            ft_string key = base_key;
            key.append("_id");
            json_item *ship_id_item = json_find_item(current, key.c_str());
            if (!ship_id_item)
                continue;
            ft_ship ship_snapshot;
            ship_snapshot.id = ft_atoi(ship_id_item->value);
            key = base_key;
            key.append("_type");
            json_item *ship_type_item = json_find_item(current, key.c_str());
            if (ship_type_item)
                ship_snapshot.type = ft_atoi(ship_type_item->value);
            key = base_key;
            key.append("_armor");
            json_item *armor_item = json_find_item(current, key.c_str());
            if (armor_item)
                ship_snapshot.armor = ft_atoi(armor_item->value);
            key = base_key;
            key.append("_hp");
            json_item *hp_item = json_find_item(current, key.c_str());
            if (hp_item)
                ship_snapshot.hp = ft_atoi(hp_item->value);
            key = base_key;
            key.append("_shield");
            json_item *shield_item = json_find_item(current, key.c_str());
            if (shield_item)
                ship_snapshot.shield = ft_atoi(shield_item->value);
            key = base_key;
            key.append("_max_hp");
            json_item *max_hp_item = json_find_item(current, key.c_str());
            if (max_hp_item)
                ship_snapshot.max_hp = ft_atoi(max_hp_item->value);
            key = base_key;
            key.append("_max_shield");
            json_item *max_shield_item = json_find_item(current, key.c_str());
            if (max_shield_item)
                ship_snapshot.max_shield = ft_atoi(max_shield_item->value);
            key = base_key;
            key.append("_max_speed");
            json_item *max_speed_item = json_find_item(current, key.c_str());
            if (max_speed_item)
                ship_snapshot.max_speed = this->unscale_long_to_double(ft_atol(max_speed_item->value));
            key = base_key;
            key.append("_acceleration");
            json_item *acceleration_item = json_find_item(current, key.c_str());
            if (acceleration_item)
                ship_snapshot.acceleration = this->unscale_long_to_double(ft_atol(acceleration_item->value));
            key = base_key;
            key.append("_turn_speed");
            json_item *turn_speed_item = json_find_item(current, key.c_str());
            if (turn_speed_item)
                ship_snapshot.turn_speed = this->unscale_long_to_double(ft_atol(turn_speed_item->value));
            key = base_key;
            key.append("_behavior");
            json_item *behavior_item = json_find_item(current, key.c_str());
            if (behavior_item)
                ship_snapshot.combat_behavior = ft_atoi(behavior_item->value);
            key = base_key;
            key.append("_outnumbered");
            json_item *outnumbered_item = json_find_item(current, key.c_str());
            if (outnumbered_item)
                ship_snapshot.outnumbered_behavior = ft_atoi(outnumbered_item->value);
            key = base_key;
            key.append("_unescorted");
            json_item *unescorted_item = json_find_item(current, key.c_str());
            if (unescorted_item)
                ship_snapshot.unescorted_behavior = ft_atoi(unescorted_item->value);
            key = base_key;
            key.append("_low_hp");
            json_item *low_hp_item = json_find_item(current, key.c_str());
            if (low_hp_item)
                ship_snapshot.low_hp_behavior = ft_atoi(low_hp_item->value);
            key = base_key;
            key.append("_role");
            json_item *role_item = json_find_item(current, key.c_str());
            if (role_item)
                ship_snapshot.role = ft_atoi(role_item->value);
            fleet->add_ship_snapshot(ship_snapshot);
        }
        fleets.insert(fleet_id, fleet);
        current = current->next;
    }
    json_free_groups(groups);
    return true;
}

ft_string SaveSystem::serialize_research(const ResearchManager &research) const noexcept
{
    json_document document;
    json_group *settings = document.create_group("research_settings");
    if (settings)
    {
        document.append_group(settings);
        long scaled = this->scale_double_to_long(research.get_duration_scale());
        ft_string value = ft_to_string(scaled);
        json_item *scale_item = document.create_item("duration_scale", value.c_str());
        if (scale_item)
            document.add_item(settings, scale_item);
    }
    ft_map<int, ft_research_progress> progress;
    research.get_progress_state(progress);
    size_t count = progress.size();
    if (count > 0)
    {
        const Pair<int, ft_research_progress> *entries = progress.end();
        entries -= count;
        for (size_t i = 0; i < count; ++i)
        {
            ft_string group_name = "research_";
            group_name.append(ft_to_string(entries[i].key));
            json_group *group = document.create_group(group_name.c_str());
            if (!group)
                continue;
            document.append_group(group);
            json_item *id_item = document.create_item("id", entries[i].key);
            if (id_item)
                document.add_item(group, id_item);
            json_item *status_item = document.create_item("status", entries[i].value.status);
            if (status_item)
                document.add_item(group, status_item);
            long remaining_scaled = this->scale_double_to_long(entries[i].value.remaining_time);
            ft_string remaining_value = ft_to_string(remaining_scaled);
            json_item *remaining_item = document.create_item("remaining_time", remaining_value.c_str());
            if (remaining_item)
                document.add_item(group, remaining_item);
        }
    }
    char *serialized = document.write_to_string();
    if (!serialized)
        return ft_string();
    ft_string result(serialized);
    cma_free(serialized);
    return result;
}

bool SaveSystem::deserialize_research(const char *content, ResearchManager &research) const noexcept
{
    if (!content)
        return false;
    json_group *groups = json_read_from_string(content);
    if (!groups)
        return false;
    ft_map<int, ft_research_progress> snapshot;
    double duration_scale = research.get_duration_scale();
    json_group *current = groups;
    while (current)
    {
        json_item *id_item = json_find_item(current, "id");
        if (id_item)
        {
            int research_id = ft_atoi(id_item->value);
            ft_research_progress progress;
            progress.status = RESEARCH_STATUS_LOCKED;
            progress.remaining_time = 0.0;
            json_item *status_item = json_find_item(current, "status");
            if (status_item)
                progress.status = ft_atoi(status_item->value);
            json_item *remaining_item = json_find_item(current, "remaining_time");
            if (remaining_item)
                progress.remaining_time = this->unscale_long_to_double(ft_atol(remaining_item->value));
            snapshot.insert(research_id, progress);
        }
        else
        {
            json_item *scale_item = json_find_item(current, "duration_scale");
            if (scale_item)
                duration_scale = this->unscale_long_to_double(ft_atol(scale_item->value));
        }
        current = current->next;
    }
    research.set_duration_scale(duration_scale);
    research.set_progress_state(snapshot);
    json_free_groups(groups);
    return true;
}

ft_string SaveSystem::serialize_achievements(const AchievementManager &achievements) const noexcept
{
    json_document document;
    ft_map<int, ft_achievement_progress> progress;
    achievements.get_progress_state(progress);
    size_t count = progress.size();
    if (count > 0)
    {
        const Pair<int, ft_achievement_progress> *entries = progress.end();
        entries -= count;
        for (size_t i = 0; i < count; ++i)
        {
            ft_string group_name = "achievement_";
            group_name.append(ft_to_string(entries[i].key));
            json_group *group = document.create_group(group_name.c_str());
            if (!group)
                continue;
            document.append_group(group);
            json_item *id_item = document.create_item("id", entries[i].key);
            if (id_item)
                document.add_item(group, id_item);
            json_item *value_item = document.create_item("value", entries[i].value.value);
            if (value_item)
                document.add_item(group, value_item);
            json_item *completed_item = document.create_item("completed", entries[i].value.completed ? 1 : 0);
            if (completed_item)
                document.add_item(group, completed_item);
        }
    }
    char *serialized = document.write_to_string();
    if (!serialized)
        return ft_string();
    ft_string result(serialized);
    cma_free(serialized);
    return result;
}

bool SaveSystem::deserialize_achievements(const char *content, AchievementManager &achievements) const noexcept
{
    if (!content)
        return false;
    json_group *groups = json_read_from_string(content);
    if (!groups)
        return false;
    ft_map<int, ft_achievement_progress> snapshot;
    json_group *current = groups;
    while (current)
    {
        json_item *id_item = json_find_item(current, "id");
        if (!id_item)
        {
            current = current->next;
            continue;
        }
        int achievement_id = ft_atoi(id_item->value);
        ft_achievement_progress progress;
        progress.value = 0;
        progress.completed = false;
        json_item *value_item = json_find_item(current, "value");
        if (value_item)
            progress.value = ft_atoi(value_item->value);
        json_item *completed_item = json_find_item(current, "completed");
        if (completed_item)
            progress.completed = ft_atoi(completed_item->value) != 0;
        snapshot.insert(achievement_id, progress);
        current = current->next;
    }
    achievements.set_progress_state(snapshot);
    json_free_groups(groups);
    return true;
}
