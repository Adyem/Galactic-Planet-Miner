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
    const int  SAVE_PLANET_STACK_LIMIT = 1000000;

    bool save_system_parse_int_exact(const char *value, int &out) noexcept
    {
        if (!value)
            return false;
        char *end = ft_nullptr;
        long parsed = ft_strtol(value, &end, 10);
        if (end == value)
            return false;
        while (end && *end != '\0')
        {
            if (!ft_isspace(*end))
                return false;
            ++end;
        }
        if (parsed < FT_INT_MIN)
            parsed = FT_INT_MIN;
        if (parsed > FT_INT_MAX)
            parsed = FT_INT_MAX;
        out = static_cast<int>(parsed);
        return true;
    }

    bool save_system_parse_non_negative_int(const char *value, int &out) noexcept
    {
        if (!save_system_parse_int_exact(value, out))
            return false;
        if (out < 0)
            out = 0;
        return true;
    }

    bool save_system_parse_stack_amount(const char *value, int &out) noexcept
    {
        if (!save_system_parse_non_negative_int(value, out))
            return false;
        if (out > SAVE_PLANET_STACK_LIMIT)
            out = SAVE_PLANET_STACK_LIMIT;
        return true;
    }

    bool save_system_is_known_ship_type(int ship_type) noexcept
    {
        switch (ship_type)
        {
        case SHIP_SHIELD:
        case SHIP_RADAR:
        case SHIP_SALVAGE:
        case SHIP_TRANSPORT:
        case SHIP_CORVETTE:
        case SHIP_INTERCEPTOR:
        case SHIP_REPAIR_DRONE:
        case SHIP_SUNFLARE_SLOOP:
        case SHIP_FRIGATE_JUGGERNAUT:
        case SHIP_FRIGATE_CARRIER:
        case SHIP_FRIGATE_SOVEREIGN:
        case SHIP_FRIGATE_PREEMPTOR:
        case SHIP_FRIGATE_PROTECTOR:
        case SHIP_FRIGATE_ECLIPSE:
        case SHIP_CAPITAL_JUGGERNAUT:
        case SHIP_CAPITAL_NOVA:
        case SHIP_CAPITAL_OBSIDIAN:
        case SHIP_CAPITAL_PREEMPTOR:
        case SHIP_CAPITAL_PROTECTOR:
        case SHIP_CAPITAL_ECLIPSE:
        case SHIP_RAIDER_CORVETTE:
        case SHIP_RAIDER_DESTROYER:
        case SHIP_RAIDER_BATTLESHIP:
            return true;
        default:
            return false;
        }
    }

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

    void save_system_append_grid_run(ft_string &target, int value, size_t count) noexcept
    {
        if (!target.empty())
            target.append(" ");
        target.append(ft_to_string(value));
        if (count > 1)
        {
            target.append("x");
            target.append(ft_to_string(static_cast<long>(count)));
        }
    }

    bool save_system_parse_grid_run(const char *token_begin, const char *token_end,
        long &value_out, size_t &count_out) noexcept
    {
        if (!token_begin || !token_end || token_begin == token_end)
            return false;
        const char *cursor = token_begin;
        ft_string value_buffer;
        bool has_digit = false;

        if (*cursor == '-')
        {
            value_buffer.append('-');
            ++cursor;
        }
        while (cursor < token_end && ft_isdigit(*cursor))
        {
            value_buffer.append(*cursor);
            ++cursor;
            has_digit = true;
        }
        if (!has_digit)
            return false;
        value_out = ft_atol(value_buffer.c_str());
