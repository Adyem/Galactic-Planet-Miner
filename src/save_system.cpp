#include "save_system.hpp"
#include "../libft/CMA/CMA.hpp"
#include "../libft/CPP_class/class_nullptr.hpp"
#include "../libft/Libft/limits.hpp"

namespace
{
    const long SAVE_DOUBLE_SCALE = 1000000;
    const long SAVE_DOUBLE_SENTINEL_NAN = FT_LONG_MIN;
    const long SAVE_DOUBLE_SENTINEL_NEG_INF = FT_LONG_MIN + 1;
    const long SAVE_DOUBLE_SENTINEL_POS_INF = FT_LONG_MAX;
    const long SAVE_DOUBLE_MIN_FINITE = FT_LONG_MIN + 2;
    const long SAVE_DOUBLE_MAX_FINITE = FT_LONG_MAX - 1;
    const int  SAVE_SHIP_ID_MIN = 1;
    const int  SAVE_SHIP_ID_MAX = FT_INT_MAX - 1;
    const int  SAVE_MAX_SHIPS_PER_FLEET = 4096;
    const long BUILDING_GRID_MAX_CELLS = 1048576;

    union save_system_double_converter
    {
        double double_value;
        unsigned long long bit_pattern;
    };

    bool save_system_is_nan(double value)
    {
        save_system_double_converter converter;
        unsigned long long exponent_bits;
        unsigned long long mantissa_bits;

        converter.double_value = value;
        exponent_bits = converter.bit_pattern & 0x7ff0000000000000ULL;
        mantissa_bits = converter.bit_pattern & 0x000fffffffffffffULL;
        if (exponent_bits == 0x7ff0000000000000ULL && mantissa_bits != 0ULL)
            return true;
        return false;
    }

    int save_system_infinity_direction(double value)
    {
        save_system_double_converter converter;
        unsigned long long exponent_bits;
        unsigned long long mantissa_bits;

        converter.double_value = value;
        exponent_bits = converter.bit_pattern & 0x7ff0000000000000ULL;
        mantissa_bits = converter.bit_pattern & 0x000fffffffffffffULL;
        if (exponent_bits == 0x7ff0000000000000ULL && mantissa_bits == 0ULL)
        {
            if ((converter.bit_pattern & 0x8000000000000000ULL) != 0ULL)
                return -1;
            return 1;
        }
        return 0;
    }

    bool save_system_is_infinite(double value)
    {
        if (save_system_infinity_direction(value) != 0)
            return true;
        return false;
    }

    bool save_system_is_finite(double value)
    {
        if (save_system_is_nan(value))
            return false;
        if (save_system_is_infinite(value))
            return false;
        return true;
    }

    double save_system_positive_infinity()
    {
        save_system_double_converter converter;

        converter.bit_pattern = 0x7ff0000000000000ULL;
        return converter.double_value;
    }

    double save_system_negative_infinity()
    {
        save_system_double_converter converter;

        converter.bit_pattern = 0xfff0000000000000ULL;
        return converter.double_value;
    }

    double save_system_quiet_nan()
    {
        save_system_double_converter converter;

        converter.bit_pattern = 0x7ff8000000000000ULL;
        return converter.double_value;
    }

    SaveSystem::json_allocation_hook_t g_json_allocation_hook = ft_nullptr;

    bool save_system_allocation_blocked(const char *type, const char *identifier)
    {
        if (!g_json_allocation_hook)
            return false;
        if (!g_json_allocation_hook(type, identifier))
            return true;
        return false;
    }

    ft_string save_system_abort_serialization(json_document &document)
    {
        document.clear();
        return ft_string();
    }

    json_group *save_system_create_group(json_document &document, const char *name)
    {
        if (save_system_allocation_blocked("group", name))
            return ft_nullptr;
        json_group *group = document.create_group(name);
        if (!group)
            return ft_nullptr;
        document.append_group(group);
        return group;
    }

    bool save_system_add_item(json_document &document, json_group *group, const char *key, int value)
    {
        if (save_system_allocation_blocked("item", key))
            return false;
        json_item *item = document.create_item(key, value);
        if (!item)
            return false;
        document.add_item(group, item);
        return true;
    }

    bool save_system_add_item(json_document &document, json_group *group, const char *key, const char *value)
    {
        if (save_system_allocation_blocked("item", key))
            return false;
        json_item *item = document.create_item(key, value);
        if (!item)
            return false;
        document.add_item(group, item);
        return true;
    }
}

SaveSystem::SaveSystem() noexcept
{
    return ;
}

SaveSystem::~SaveSystem() noexcept
{
    return ;
}

void SaveSystem::set_json_allocation_hook(json_allocation_hook_t hook) noexcept
{
    g_json_allocation_hook = hook;
    return ;
}

long SaveSystem::scale_double_to_long(double value) const noexcept
{
    int infinity_direction;

    if (save_system_is_nan(value))
        return SAVE_DOUBLE_SENTINEL_NAN;
    infinity_direction = save_system_infinity_direction(value);
    if (infinity_direction > 0)
        return SAVE_DOUBLE_SENTINEL_POS_INF;
    if (infinity_direction < 0)
        return SAVE_DOUBLE_SENTINEL_NEG_INF;
    if (SAVE_DOUBLE_SCALE == 0)
        return 0;
    double scaled = value * static_cast<double>(SAVE_DOUBLE_SCALE);
    if (!save_system_is_finite(scaled))
        return value >= 0.0 ? SAVE_DOUBLE_MAX_FINITE : SAVE_DOUBLE_MIN_FINITE;
    if (scaled >= 0.0)
        scaled += 0.5;
    else
        scaled -= 0.5;
    if (scaled > static_cast<double>(SAVE_DOUBLE_MAX_FINITE))
        return SAVE_DOUBLE_MAX_FINITE;
    if (scaled < static_cast<double>(SAVE_DOUBLE_MIN_FINITE))
        return SAVE_DOUBLE_MIN_FINITE;
    long result = static_cast<long>(scaled);
    return result;
}

double SaveSystem::unscale_long_to_double(long value) const noexcept
{
    if (value == SAVE_DOUBLE_SENTINEL_NAN)
        return save_system_quiet_nan();
    if (value == SAVE_DOUBLE_SENTINEL_POS_INF)
        return save_system_positive_infinity();
    if (value == SAVE_DOUBLE_SENTINEL_NEG_INF)
        return save_system_negative_infinity();
    if (value > SAVE_DOUBLE_MAX_FINITE)
        value = SAVE_DOUBLE_MAX_FINITE;
    else if (value < SAVE_DOUBLE_MIN_FINITE)
        value = SAVE_DOUBLE_MIN_FINITE;
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
            json_group *group = save_system_create_group(document, group_name.c_str());
            if (!group)
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "id", entries[i].key))
                return save_system_abort_serialization(document);
            const ft_vector<Pair<int, double> > &resources = planet->get_resources();
            for (size_t j = 0; j < resources.size(); ++j)
            {
                int ore_id = resources[j].key;
                ft_string ore_string = ft_to_string(ore_id);
                ft_string amount_key = "resource_";
                amount_key.append(ore_string);
                int amount = planet->get_resource(ore_id);
                if (!save_system_add_item(document, group, amount_key.c_str(), amount))
                    return save_system_abort_serialization(document);
                ft_string rate_key = "rate_";
                rate_key.append(ore_string);
                long scaled_rate = this->scale_double_to_long(resources[j].value);
                ft_string rate_value = ft_to_string(scaled_rate);
                if (!save_system_add_item(document, group, rate_key.c_str(), rate_value.c_str()))
                    return save_system_abort_serialization(document);
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
                if (!save_system_add_item(document, group, carry_key.c_str(), carry_value.c_str()))
                    return save_system_abort_serialization(document);
            }
            ft_vector<Pair<int, int> > inventory_snapshot = planet->get_items_snapshot();
            for (size_t j = 0; j < inventory_snapshot.size(); ++j)
            {
                ft_string item_key = "item_";
                ft_string item_id_string = ft_to_string(inventory_snapshot[j].key);
                item_key.append(item_id_string);
                if (!save_system_add_item(document, group, item_key.c_str(), inventory_snapshot[j].value))
                    return save_system_abort_serialization(document);
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
        ft_vector<Pair<int, int> > inventory_items;
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
            else if (item->key && ft_strncmp(item->key, "item_", 5) == 0)
            {
                int item_id = ft_atoi(item->key + 5);
                Pair<int, int> entry;
                entry.key = item_id;
                entry.value = ft_atoi(item->value);
                inventory_items.push_back(entry);
            }
            item = item->next;
        }
        for (size_t i = 0; i < resource_rates.size(); ++i)
        {
            int ore_id = resource_rates[i].key;
            double rate = this->unscale_long_to_double(resource_rates[i].value);
            if (!save_system_is_finite(rate))
                rate = 0.0;
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
            if (!save_system_is_finite(carry_value))
                carry_value = 0.0;
            planet->set_carryover(ore_id, carry_value);
        }
        for (size_t i = 0; i < inventory_items.size(); ++i)
        {
            int item_id = inventory_items[i].key;
            planet->ensure_item_slot(item_id);
            planet->set_resource(item_id, inventory_items[i].value);
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
            json_group *group = save_system_create_group(document, group_name.c_str());
            if (!group)
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "id", entries[i].key))
                return save_system_abort_serialization(document);
            ft_location location = fleet->get_location();
            if (!save_system_add_item(document, group, "location_type", location.type))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "location_from", location.from))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "location_to", location.to))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "location_misc", location.misc))
                return save_system_abort_serialization(document);
            long travel_scaled = this->scale_double_to_long(fleet->get_travel_time());
            ft_string travel_value = ft_to_string(travel_scaled);
            if (!save_system_add_item(document, group, "travel_time", travel_value.c_str()))
                return save_system_abort_serialization(document);
            long veterancy_scaled = this->scale_double_to_long(fleet->get_escort_veterancy());
            ft_string veterancy_value = ft_to_string(veterancy_scaled);
            if (!save_system_add_item(document, group, "escort_veterancy", veterancy_value.c_str()))
                return save_system_abort_serialization(document);
            int ship_total = fleet->get_ship_count();
            if (!save_system_add_item(document, group, "ship_count", ship_total))
                return save_system_abort_serialization(document);
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
                if (!save_system_add_item(document, group, key.c_str(), ship->id))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_type");
                if (!save_system_add_item(document, group, key.c_str(), ship->type))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_armor");
                if (!save_system_add_item(document, group, key.c_str(), ship->armor))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_hp");
                if (!save_system_add_item(document, group, key.c_str(), ship->hp))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_shield");
                if (!save_system_add_item(document, group, key.c_str(), ship->shield))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_max_hp");
                if (!save_system_add_item(document, group, key.c_str(), ship->max_hp))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_max_shield");
                if (!save_system_add_item(document, group, key.c_str(), ship->max_shield))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_max_speed");
                long max_speed_scaled = this->scale_double_to_long(ship->max_speed);
                ft_string max_speed_value = ft_to_string(max_speed_scaled);
                if (!save_system_add_item(document, group, key.c_str(), max_speed_value.c_str()))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_acceleration");
                long acceleration_scaled = this->scale_double_to_long(ship->acceleration);
                ft_string acceleration_value = ft_to_string(acceleration_scaled);
                if (!save_system_add_item(document, group, key.c_str(), acceleration_value.c_str()))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_turn_speed");
                long turn_speed_scaled = this->scale_double_to_long(ship->turn_speed);
                ft_string turn_speed_value = ft_to_string(turn_speed_scaled);
                if (!save_system_add_item(document, group, key.c_str(), turn_speed_value.c_str()))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_behavior");
                if (!save_system_add_item(document, group, key.c_str(), ship->combat_behavior))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_outnumbered");
                if (!save_system_add_item(document, group, key.c_str(), ship->outnumbered_behavior))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_unescorted");
                if (!save_system_add_item(document, group, key.c_str(), ship->unescorted_behavior))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_low_hp");
                if (!save_system_add_item(document, group, key.c_str(), ship->low_hp_behavior))
                    return save_system_abort_serialization(document);
                key = base_key;
                key.append("_role");
                if (!save_system_add_item(document, group, key.c_str(), ship->role))
                    return save_system_abort_serialization(document);
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
        if (ship_count < 0)
            ship_count = 0;
        int highest_ship_slot = -1;
        json_item *ship_entry = current->items;
        while (ship_entry)
        {
            if (ship_entry->key && ft_strncmp(ship_entry->key, "ship_", 5) == 0)
            {
                const char *index_start = ship_entry->key + 5;
                const char *cursor = index_start;
                while (*cursor && *cursor >= '0' && *cursor <= '9')
                    ++cursor;
                if (cursor != index_start && ft_strncmp(cursor, "_id", 3) == 0)
                {
                    int slot = ft_atoi(index_start);
                    if (slot < 0)
                    {
                        json_free_groups(groups);
                        return false;
                    }
                    if (slot > highest_ship_slot)
                        highest_ship_slot = slot;
                    if (slot >= SAVE_MAX_SHIPS_PER_FLEET)
                    {
                        json_free_groups(groups);
                        return false;
                    }
                }
            }
            ship_entry = ship_entry->next;
        }
        if (ship_count > SAVE_MAX_SHIPS_PER_FLEET)
            ship_count = SAVE_MAX_SHIPS_PER_FLEET;
        if (highest_ship_slot < 0)
            ship_count = 0;
        else if (ship_count > highest_ship_slot + 1)
            ship_count = highest_ship_slot + 1;
        int missing_streak = 0;
        bool saw_ship = false;
        for (int i = 0; i < ship_count; ++i)
        {
            ft_string index_string = ft_to_string(static_cast<long>(i));
            ft_string base_key = "ship_";
            base_key.append(index_string);
            ft_string key = base_key;
            key.append("_id");
            json_item *ship_id_item = json_find_item(current, key.c_str());
            if (!ship_id_item)
            {
                ++missing_streak;
                if (saw_ship && missing_streak >= 2)
                    break;
                continue;
            }
            missing_streak = 0;
            saw_ship = true;
            ft_ship ship_snapshot;
            ship_snapshot.id = ft_atoi(ship_id_item->value);
            if (ship_snapshot.id < SAVE_SHIP_ID_MIN
                || ship_snapshot.id > SAVE_SHIP_ID_MAX)
            {
                json_free_groups(groups);
                return false;
            }
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
    json_group *settings = save_system_create_group(document, "research_settings");
    if (!settings)
        return save_system_abort_serialization(document);
    long scaled = this->scale_double_to_long(research.get_duration_scale());
    ft_string value = ft_to_string(scaled);
    if (!save_system_add_item(document, settings, "duration_scale", value.c_str()))
        return save_system_abort_serialization(document);
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
            json_group *group = save_system_create_group(document, group_name.c_str());
            if (!group)
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "id", entries[i].key))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "status", entries[i].value.status))
                return save_system_abort_serialization(document);
            long remaining_scaled = this->scale_double_to_long(entries[i].value.remaining_time);
            ft_string remaining_value = ft_to_string(remaining_scaled);
            if (!save_system_add_item(document, group, "remaining_time", remaining_value.c_str()))
                return save_system_abort_serialization(document);
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
            json_group *group = save_system_create_group(document, group_name.c_str());
            if (!group)
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "id", entries[i].key))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "value", entries[i].value.value))
                return save_system_abort_serialization(document);
            int completed_flag = entries[i].value.completed ? 1 : 0;
            if (!save_system_add_item(document, group, "completed", completed_flag))
                return save_system_abort_serialization(document);
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

ft_string SaveSystem::serialize_buildings(const BuildingManager &buildings) const noexcept
{
    json_document document;
    json_group *manager_group = save_system_create_group(document, "buildings_manager");
    if (!manager_group)
        return save_system_abort_serialization(document);
    if (!save_system_add_item(document, manager_group, "type", "manager"))
        return save_system_abort_serialization(document);

    long crafting_energy_scaled = this->scale_double_to_long(buildings._crafting_energy_multiplier);
    ft_string crafting_energy_value = ft_to_string(crafting_energy_scaled);
    if (!save_system_add_item(document, manager_group, "crafting_energy_multiplier",
        crafting_energy_value.c_str()))
    {
        return save_system_abort_serialization(document);
    }
    long crafting_speed_scaled = this->scale_double_to_long(buildings._crafting_speed_multiplier);
    ft_string crafting_speed_value = ft_to_string(crafting_speed_scaled);
    if (!save_system_add_item(document, manager_group, "crafting_speed_multiplier",
        crafting_speed_value.c_str()))
    {
        return save_system_abort_serialization(document);
    }
    long global_energy_scaled = this->scale_double_to_long(buildings._global_energy_multiplier);
    ft_string global_energy_value = ft_to_string(global_energy_scaled);
    if (!save_system_add_item(document, manager_group, "global_energy_multiplier",
        global_energy_value.c_str()))
    {
        return save_system_abort_serialization(document);
    }

    size_t unlock_count = buildings._building_unlocks.size();
    if (unlock_count > 0)
    {
        const Pair<int, bool> *unlock_entries = buildings._building_unlocks.end();
        unlock_entries -= unlock_count;
        for (size_t i = 0; i < unlock_count; ++i)
        {
            ft_string key("unlock_");
            key.append(ft_to_string(unlock_entries[i].key));
            int unlocked = unlock_entries[i].value ? 1 : 0;
            if (!save_system_add_item(document, manager_group, key.c_str(), unlocked))
                return save_system_abort_serialization(document);
        }
    }

    size_t planet_count = buildings._planets.size();
    if (planet_count > 0)
    {
        const Pair<int, ft_planet_build_state> *entries = buildings._planets.end();
        entries -= planet_count;
        for (size_t i = 0; i < planet_count; ++i)
        {
            const ft_planet_build_state &state = entries[i].value;
            ft_string group_name = "building_planet_";
            group_name.append(ft_to_string(entries[i].key));
            json_group *group = save_system_create_group(document, group_name.c_str());
            if (!group)
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "type", "planet"))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "id", entries[i].key))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "width", state.width))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "height", state.height))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "base_logistic", state.base_logistic))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "research_logistic_bonus",
                state.research_logistic_bonus))
            {
                return save_system_abort_serialization(document);
            }
            if (!save_system_add_item(document, group, "used_plots", state.used_plots))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "logistic_capacity", state.logistic_capacity))
                return save_system_abort_serialization(document);
            if (!save_system_add_item(document, group, "logistic_usage", state.logistic_usage))
                return save_system_abort_serialization(document);
            long base_energy_scaled = this->scale_double_to_long(state.base_energy_generation);
            ft_string base_energy_value = ft_to_string(base_energy_scaled);
            if (!save_system_add_item(document, group, "base_energy_generation",
                base_energy_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            long energy_generation_scaled = this->scale_double_to_long(state.energy_generation);
            ft_string energy_generation_value = ft_to_string(energy_generation_scaled);
            if (!save_system_add_item(document, group, "energy_generation",
                energy_generation_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            long energy_consumption_scaled = this->scale_double_to_long(state.energy_consumption);
            ft_string energy_consumption_value = ft_to_string(energy_consumption_scaled);
            if (!save_system_add_item(document, group, "energy_consumption",
                energy_consumption_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            long support_energy_scaled = this->scale_double_to_long(state.support_energy);
            ft_string support_energy_value = ft_to_string(support_energy_scaled);
            if (!save_system_add_item(document, group, "support_energy", support_energy_value.c_str()))
                return save_system_abort_serialization(document);
            long mine_multiplier_scaled = this->scale_double_to_long(state.mine_multiplier);
            ft_string mine_multiplier_value = ft_to_string(mine_multiplier_scaled);
            if (!save_system_add_item(document, group, "mine_multiplier",
                mine_multiplier_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            long convoy_speed_scaled = this->scale_double_to_long(state.convoy_speed_bonus);
            ft_string convoy_speed_value = ft_to_string(convoy_speed_scaled);
            if (!save_system_add_item(document, group, "convoy_speed_bonus",
                convoy_speed_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            long convoy_risk_scaled = this->scale_double_to_long(state.convoy_raid_risk_modifier);
            ft_string convoy_risk_value = ft_to_string(convoy_risk_scaled);
            if (!save_system_add_item(document, group, "convoy_raid_risk_modifier",
                convoy_risk_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            long deficit_scaled = this->scale_double_to_long(state.energy_deficit_pressure);
            ft_string deficit_value = ft_to_string(deficit_scaled);
            if (!save_system_add_item(document, group, "energy_deficit_pressure",
                deficit_value.c_str()))
            {
                return save_system_abort_serialization(document);
            }
            if (!save_system_add_item(document, group, "next_instance_id", state.next_instance_id))
                return save_system_abort_serialization(document);

            size_t grid_size = state.grid.size();
            for (size_t j = 0; j < grid_size; ++j)
            {
                ft_string cell_key("cell_");
                cell_key.append(ft_to_string(static_cast<long>(j)));
                if (!save_system_add_item(document, group, cell_key.c_str(), state.grid[j]))
                    return save_system_abort_serialization(document);
            }

            size_t instance_count = state.instances.size();
            if (instance_count > 0)
            {
                const Pair<int, ft_building_instance> *inst_entries = state.instances.end();
                inst_entries -= instance_count;
                for (size_t j = 0; j < instance_count; ++j)
                {
                    const ft_building_instance &instance = inst_entries[j].value;
                    ft_string base_key("instance_");
                    base_key.append(ft_to_string(instance.uid));
                    ft_string key = base_key;
                    key.append("_uid");
                    if (!save_system_add_item(document, group, key.c_str(), instance.uid))
                        return save_system_abort_serialization(document);
                    key = base_key;
                    key.append("_definition");
                    if (!save_system_add_item(document, group, key.c_str(), instance.definition_id))
                        return save_system_abort_serialization(document);
                    key = base_key;
                    key.append("_x");
                    if (!save_system_add_item(document, group, key.c_str(), instance.x))
                        return save_system_abort_serialization(document);
                    key = base_key;
                    key.append("_y");
                    if (!save_system_add_item(document, group, key.c_str(), instance.y))
                        return save_system_abort_serialization(document);
                    key = base_key;
                    key.append("_progress");
                    long progress_scaled = this->scale_double_to_long(instance.progress);
                    ft_string progress_value = ft_to_string(progress_scaled);
                    if (!save_system_add_item(document, group, key.c_str(), progress_value.c_str()))
                        return save_system_abort_serialization(document);
                    key = base_key;
                    key.append("_active");
                    int active_flag = instance.active ? 1 : 0;
                    if (!save_system_add_item(document, group, key.c_str(), active_flag))
                        return save_system_abort_serialization(document);
                }
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

bool SaveSystem::deserialize_buildings(const char *content, BuildingManager &buildings) const noexcept
{
    if (!content)
        return false;
    json_group *groups = json_read_from_string(content);
    if (!groups)
        return false;

    ft_map<int, ft_planet_build_state> parsed_planets;
    ft_map<int, bool> parsed_unlocks;
    double parsed_crafting_energy = 1.0;
    double parsed_crafting_speed = 1.0;
    double parsed_global_energy = 1.0;

    json_group *current = groups;
    while (current)
    {
        json_item *type_item = json_find_item(current, "type");
        const char *type_value = ft_nullptr;
        if (type_item)
            type_value = type_item->value;
        if (type_value && ft_strcmp(type_value, "manager") == 0)
        {
            json_item *item = current->items;
            while (item)
            {
                if (!item->key)
                {
                    item = item->next;
                    continue;
                }
                if (ft_strcmp(item->key, "crafting_energy_multiplier") == 0)
                {
                    parsed_crafting_energy = this->unscale_long_to_double(ft_atol(item->value));
                    if (parsed_crafting_energy <= 0.0)
                        parsed_crafting_energy = 1.0;
                }
                else if (ft_strcmp(item->key, "crafting_speed_multiplier") == 0)
                {
                    parsed_crafting_speed = this->unscale_long_to_double(ft_atol(item->value));
                    if (parsed_crafting_speed <= 0.0)
                        parsed_crafting_speed = 1.0;
                }
                else if (ft_strcmp(item->key, "global_energy_multiplier") == 0)
                {
                    parsed_global_energy = this->unscale_long_to_double(ft_atol(item->value));
                    if (parsed_global_energy <= 0.0)
                        parsed_global_energy = 1.0;
                }
                else if (ft_strncmp(item->key, "unlock_", 7) == 0)
                {
                    int building_id = ft_atoi(item->key + 7);
                    bool unlocked = ft_atoi(item->value) != 0;
                    Pair<int, bool> *entry = parsed_unlocks.find(building_id);
                    if (entry == ft_nullptr)
                        parsed_unlocks.insert(building_id, unlocked);
                    else
                        entry->value = unlocked;
                }
                item = item->next;
            }
        }
        else
        {
            json_item *id_item = json_find_item(current, "id");
            if (!id_item)
            {
                current = current->next;
                continue;
            }
            int planet_id = ft_atoi(id_item->value);
            ft_planet_build_state state;
            state.planet_id = planet_id;
            ft_vector<Pair<int, int> > grid_entries;
            ft_map<int, ft_building_instance> instance_lookup;
            json_item *item = current->items;
            while (item)
            {
                if (!item->key)
                {
                    item = item->next;
                    continue;
                }
                if (ft_strcmp(item->key, "type") == 0)
                {
                    item = item->next;
                    continue;
                }
                if (ft_strcmp(item->key, "width") == 0)
                    state.width = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "height") == 0)
                    state.height = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "base_logistic") == 0)
                    state.base_logistic = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "research_logistic_bonus") == 0)
                    state.research_logistic_bonus = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "used_plots") == 0)
                    state.used_plots = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "logistic_capacity") == 0)
                    state.logistic_capacity = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "logistic_usage") == 0)
                    state.logistic_usage = ft_atoi(item->value);
                else if (ft_strcmp(item->key, "base_energy_generation") == 0)
                    state.base_energy_generation = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "energy_generation") == 0)
                    state.energy_generation = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "energy_consumption") == 0)
                    state.energy_consumption = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "support_energy") == 0)
                    state.support_energy = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "mine_multiplier") == 0)
                    state.mine_multiplier = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "convoy_speed_bonus") == 0)
                    state.convoy_speed_bonus = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "convoy_raid_risk_modifier") == 0)
                    state.convoy_raid_risk_modifier = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "energy_deficit_pressure") == 0)
                    state.energy_deficit_pressure = this->unscale_long_to_double(ft_atol(item->value));
                else if (ft_strcmp(item->key, "next_instance_id") == 0)
                {
                    state.next_instance_id = ft_atoi(item->value);
                    if (state.next_instance_id >= FT_BUILDING_INSTANCE_ID_MAX)
                        state.next_instance_id = FT_BUILDING_INSTANCE_ID_MAX;
                }
                else if (ft_strncmp(item->key, "cell_", 5) == 0)
                {
                    Pair<int, int> entry;
                    entry.key = ft_atoi(item->key + 5);
                    entry.value = ft_atoi(item->value);
                    grid_entries.push_back(entry);
                }
                else if (ft_strncmp(item->key, "instance_", 9) == 0)
                {
                    int uid = ft_atoi(item->key + 9);
                    const char *suffix = ft_strchr(item->key + 9, '_');
                    if (!suffix)
                    {
                        item = item->next;
                        continue;
                    }
                    suffix += 1;
                    Pair<int, ft_building_instance> *entry = instance_lookup.find(uid);
                    if (entry == ft_nullptr)
                    {
                        ft_building_instance instance;
                        instance.uid = uid;
                        instance_lookup.insert(uid, instance);
                        entry = instance_lookup.find(uid);
                    }
                    if (entry == ft_nullptr)
                    {
                        item = item->next;
                        continue;
                    }
                    ft_building_instance &instance = entry->value;
                    if (ft_strcmp(suffix, "uid") == 0)
                        instance.uid = ft_atoi(item->value);
                    else if (ft_strcmp(suffix, "definition") == 0)
                        instance.definition_id = ft_atoi(item->value);
                    else if (ft_strcmp(suffix, "x") == 0)
                        instance.x = ft_atoi(item->value);
                    else if (ft_strcmp(suffix, "y") == 0)
                        instance.y = ft_atoi(item->value);
                    else if (ft_strcmp(suffix, "progress") == 0)
                        instance.progress = this->unscale_long_to_double(ft_atol(item->value));
                    else if (ft_strcmp(suffix, "active") == 0)
                        instance.active = ft_atoi(item->value) != 0;
                }
                item = item->next;
            }

            if (state.width < 0)
                state.width = 0;
            if (state.height < 0)
                state.height = 0;

            size_t sanitized_width = static_cast<size_t>(state.width);
            size_t sanitized_height = static_cast<size_t>(state.height);
            size_t max_cells_limit = static_cast<size_t>(BUILDING_GRID_MAX_CELLS);
            if (sanitized_width > max_cells_limit || sanitized_height > max_cells_limit)
            {
                json_free_groups(groups);
                return false;
            }
            if (sanitized_width > 0 && sanitized_height > max_cells_limit / sanitized_width)
            {
                json_free_groups(groups);
                return false;
            }
            if (state.base_logistic < 0)
                state.base_logistic = 0;
            if (state.research_logistic_bonus < 0)
                state.research_logistic_bonus = 0;
            if (state.logistic_capacity < 0)
                state.logistic_capacity = 0;
            if (state.logistic_usage < 0)
                state.logistic_usage = 0;
            if (state.logistic_usage > state.logistic_capacity)
                state.logistic_usage = state.logistic_capacity;
            if (state.next_instance_id <= 0)
                state.next_instance_id = 1;
            if (state.mine_multiplier < 1.0)
                state.mine_multiplier = 1.0;
            if (state.convoy_speed_bonus < 0.0)
                state.convoy_speed_bonus = 0.0;
            if (state.convoy_raid_risk_modifier < 0.0)
                state.convoy_raid_risk_modifier = 0.0;
            if (state.support_energy < 0.0)
                state.support_energy = 0.0;
            if (state.energy_deficit_pressure < 0.0)
                state.energy_deficit_pressure = 0.0;

            long total_cells_long = static_cast<long>(state.width) * static_cast<long>(state.height);
            if (total_cells_long < 0)
                total_cells_long = 0;
            size_t total_cells = static_cast<size_t>(total_cells_long);
            state.grid.clear();
            if (total_cells > 0)
            {
                state.grid.resize(total_cells, 0);
                for (size_t j = 0; j < grid_entries.size(); ++j)
                {
                    int index = grid_entries[j].key;
                    if (index < 0)
                        continue;
                    size_t cell_index = static_cast<size_t>(index);
                    if (cell_index >= total_cells)
                        continue;
                    state.grid[cell_index] = grid_entries[j].value;
                }
            }

            int max_plots = state.width * state.height;
            if (max_plots < 0)
                max_plots = 0;
            if (state.used_plots < 0)
                state.used_plots = 0;
            if (state.used_plots > max_plots)
                state.used_plots = max_plots;

            state.instances.clear();
            size_t instance_count = instance_lookup.size();
            if (instance_count > 0)
            {
                const Pair<int, ft_building_instance> *inst_entries = instance_lookup.end();
                inst_entries -= instance_count;
                for (size_t j = 0; j < instance_count; ++j)
                {
                    ft_building_instance instance = inst_entries[j].value;
                    if (instance.uid == 0)
                        instance.uid = inst_entries[j].key;
                    state.instances.insert(inst_entries[j].key, instance);
                }
            }

            parsed_planets.remove(planet_id);
            parsed_planets.insert(planet_id, ft_move(state));
        }
        current = current->next;
    }

    json_free_groups(groups);
    buildings._planets.clear();
    buildings._building_unlocks.clear();
    buildings._crafting_energy_multiplier = parsed_crafting_energy;
    buildings._crafting_speed_multiplier = parsed_crafting_speed;
    buildings._global_energy_multiplier = parsed_global_energy;

    size_t unlock_count = parsed_unlocks.size();
    if (unlock_count > 0)
    {
        const Pair<int, bool> *unlock_entries = parsed_unlocks.end();
        unlock_entries -= unlock_count;
        for (size_t i = 0; i < unlock_count; ++i)
            buildings._building_unlocks.insert(unlock_entries[i].key, unlock_entries[i].value);
    }

    size_t planet_count = parsed_planets.size();
    if (planet_count > 0)
    {
        Pair<int, ft_planet_build_state> *entries = parsed_planets.end();
        entries -= planet_count;
        for (size_t i = 0; i < planet_count; ++i)
            buildings._planets.insert(entries[i].key, ft_move(entries[i].value));
    }
    return true;
}
