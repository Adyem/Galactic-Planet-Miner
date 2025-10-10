        count_out = 1;
        if (cursor == token_end)
            return true;
        if (*cursor != 'x')
            return false;
        ++cursor;
        ft_string count_buffer;
        bool has_count_digit = false;
        while (cursor < token_end && ft_isdigit(*cursor))
        {
            count_buffer.append(*cursor);
            ++cursor;
            has_count_digit = true;
        }
        if (!has_count_digit)
            return false;
        if (cursor != token_end)
            return false;
        long parsed_count = ft_atol(count_buffer.c_str());
        if (parsed_count <= 0)
            return false;
        count_out = static_cast<size_t>(parsed_count);
        return true;
    }

    bool save_system_parse_csv_long(const char *&cursor, const char *limit, long &out) noexcept
    {
        if (!cursor || !limit)
            return false;
        if (cursor >= limit)
            return false;
        ft_string buffer;
        bool has_digit = false;
        if (*cursor == '-')
        {
            buffer.append('-');
            ++cursor;
        }
        while (cursor < limit && ft_isdigit(*cursor))
        {
            buffer.append(*cursor);
            ++cursor;
            has_digit = true;
        }
        if (!has_digit)
            return false;
        out = ft_atol(buffer.c_str());
        if (cursor < limit)
        {
            if (*cursor != ',')
                return false;
            ++cursor;
        }
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
