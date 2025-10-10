#include "../libft/Libft/libft.hpp"
#include "../libft/Libft/limits.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/map.hpp"
#include "../libft/Template/pair.hpp"
#include "../libft/JSon/document.hpp"
#include "../libft/JSon/json.hpp"
#include "../libft/CMA/CMA.hpp"
#include "../libft/File/file_utils.hpp"
#include "save_system.hpp"
#include "player_profile.hpp"
#include "main_menu_system.hpp"
#include "game_bootstrap.hpp"

#define private public
#define protected public
#include "game.hpp"
#include "combat.hpp"
#undef private
#undef protected

#include "game_test_scenarios.hpp"

static bool g_fail_next_json_allocation = false;
static const char *g_fail_allocation_type = ft_nullptr;
static const char *g_fail_allocation_identifier = ft_nullptr;

static bool save_system_test_allocation_hook(const char *type, const char *identifier)
{
    if (!g_fail_next_json_allocation)
        return true;
    if (g_fail_allocation_type && (!type || ft_strcmp(type, g_fail_allocation_type) != 0))
        return true;
    if (g_fail_allocation_identifier && (!identifier || ft_strcmp(identifier, g_fail_allocation_identifier) != 0))
        return true;
    g_fail_next_json_allocation = false;
    g_fail_allocation_type = ft_nullptr;
    g_fail_allocation_identifier = ft_nullptr;
    return false;
}

static void save_system_trigger_allocation_failure(const char *type, const char *identifier)
{
    g_fail_next_json_allocation = true;
    g_fail_allocation_type = type;
    g_fail_allocation_identifier = identifier;
}

static void save_system_reset_allocation_hook()
{
    SaveSystem::set_json_allocation_hook(ft_nullptr);
    g_fail_next_json_allocation = false;
    g_fail_allocation_type = ft_nullptr;
    g_fail_allocation_identifier = ft_nullptr;
}

static double save_system_test_positive_infinity()
{
    union
    {
        double double_value;
        unsigned long long bit_pattern;
    } converter;

    converter.bit_pattern = 0x7ff0000000000000ULL;
    return converter.double_value;
}

static double save_system_test_negative_infinity()
{
    union
    {
        double double_value;
        unsigned long long bit_pattern;
    } converter;

    converter.bit_pattern = 0xfff0000000000000ULL;
    return converter.double_value;
}

static double save_system_test_quiet_nan()
{
    union
    {
        double double_value;
        unsigned long long bit_pattern;
    } converter;

    converter.bit_pattern = 0x7ff8000000000000ULL;
    return converter.double_value;
}

static double save_system_test_double_max()
{
    union
    {
        double double_value;
        unsigned long long bit_pattern;
    } converter;

    converter.bit_pattern = 0x7fefffffffffffffULL;
    return converter.double_value;
}

[[maybe_unused]] static bool save_system_test_is_infinite(double value)
{
    union
    {
        double double_value;
        unsigned long long bit_pattern;
    } converter;
    unsigned long long exponent_bits;
    unsigned long long mantissa_bits;

    converter.double_value = value;
    exponent_bits = converter.bit_pattern & 0x7ff0000000000000ULL;
    mantissa_bits = converter.bit_pattern & 0x000fffffffffffffULL;
    if (exponent_bits == 0x7ff0000000000000ULL && mantissa_bits == 0ULL)
        return true;
    return false;
}

[[maybe_unused]] static bool save_system_test_is_nan(double value)
{
    union
    {
        double double_value;
        unsigned long long bit_pattern;
    } converter;
    unsigned long long exponent_bits;
    unsigned long long mantissa_bits;

    converter.double_value = value;
    exponent_bits = converter.bit_pattern & 0x7ff0000000000000ULL;
    mantissa_bits = converter.bit_pattern & 0x000fffffffffffffULL;
    if (exponent_bits == 0x7ff0000000000000ULL && mantissa_bits != 0ULL)
        return true;
    return false;
}

static double ft_absolute(double value)
{
    if (value < 0.0)
        return -value;
    return value;
}

static long reference_scale_double(double value)
{
    double scaled = value * 1000000.0;
    if (scaled >= 0.0)
        scaled += 0.5;
    else
        scaled -= 0.5;
    return static_cast<long>(scaled);
}

static ft_string save_system_building_payload(int width, int height,
    int used_plots = 0, int logistic_capacity = 0, int logistic_usage = 0,
    int next_instance_id = 1, int planet_id = 99,
    const ft_vector<Pair<int, int> > *grid_entries = ft_nullptr,
    const ft_vector<ft_building_instance> *instances = ft_nullptr)
{
    json_document document;
    json_group *manager_group = document.create_group("buildings_manager");
    if (manager_group == ft_nullptr)
        return ft_string();
    document.append_group(manager_group);
    json_item *manager_type = document.create_item("type", "manager");
    if (manager_type == ft_nullptr)
        return ft_string();
    document.add_item(manager_group, manager_type);

    json_group *planet_group = document.create_group("building_planet_test");
    if (planet_group == ft_nullptr)
        return ft_string();
    document.append_group(planet_group);

    json_item *planet_type = document.create_item("type", "planet");
    if (planet_type == ft_nullptr)
        return ft_string();
    document.add_item(planet_group, planet_type);

    json_item *planet_id_item = document.create_item("id", planet_id);
    if (planet_id_item == ft_nullptr)
        return ft_string();
    document.add_item(planet_group, planet_id_item);

    json_item *width_item = document.create_item("width", width);
    if (width_item == ft_nullptr)
        return ft_string();
    document.add_item(planet_group, width_item);

    json_item *height_item = document.create_item("height", height);
    if (height_item == ft_nullptr)
        return ft_string();
    document.add_item(planet_group, height_item);

    json_item *used_item = document.create_item("used_plots", used_plots);
    if (used_item == ft_nullptr)
        return ft_string();
    document.add_item(planet_group, used_item);

    json_item *capacity_item = document.create_item("logistic_capacity", logistic_capacity);
    if (capacity_item == ft_nullptr)
        return ft_string();
    document.add_item(planet_group, capacity_item);

    json_item *usage_item = document.create_item("logistic_usage", logistic_usage);
    if (usage_item == ft_nullptr)
        return ft_string();
    document.add_item(planet_group, usage_item);

    json_item *next_instance = document.create_item("next_instance_id", next_instance_id);
    if (next_instance == ft_nullptr)
        return ft_string();
    document.add_item(planet_group, next_instance);

    if (grid_entries)
    {
        for (size_t i = 0; i < grid_entries->size(); ++i)
        {
            const Pair<int, int> &entry = (*grid_entries)[i];
            ft_string cell_key("cell_");
            cell_key.append(ft_to_string(static_cast<long>(entry.key)));
            json_item *cell_item = document.create_item(cell_key.c_str(), entry.value);
            if (cell_item == ft_nullptr)
                return ft_string();
            document.add_item(planet_group, cell_item);
        }
    }

    if (instances)
    {
        for (size_t i = 0; i < instances->size(); ++i)
        {
            const ft_building_instance &instance = (*instances)[i];
            ft_string base_key("instance_");
            base_key.append(ft_to_string(static_cast<long>(instance.uid)));

            ft_string key = base_key;
            key.append("_uid");
            json_item *uid_item = document.create_item(key.c_str(), instance.uid);
            if (uid_item == ft_nullptr)
                return ft_string();
            document.add_item(planet_group, uid_item);

            key = base_key;
            key.append("_definition");
            json_item *definition_item = document.create_item(key.c_str(), instance.definition_id);
            if (definition_item == ft_nullptr)
                return ft_string();
            document.add_item(planet_group, definition_item);

            key = base_key;
            key.append("_x");
            json_item *x_item = document.create_item(key.c_str(), instance.x);
            if (x_item == ft_nullptr)
                return ft_string();
            document.add_item(planet_group, x_item);

            key = base_key;
            key.append("_y");
            json_item *y_item = document.create_item(key.c_str(), instance.y);
            if (y_item == ft_nullptr)
                return ft_string();
            document.add_item(planet_group, y_item);

            key = base_key;
            key.append("_progress");
            long progress_scaled = reference_scale_double(instance.progress);
            ft_string progress_value = ft_to_string(progress_scaled);
            json_item *progress_item = document.create_item(key.c_str(), progress_value.c_str());
            if (progress_item == ft_nullptr)
                return ft_string();
            document.add_item(planet_group, progress_item);

            key = base_key;
            key.append("_active");
            int active_flag = instance.active ? 1 : 0;
            json_item *active_item = document.create_item(key.c_str(), active_flag);
            if (active_item == ft_nullptr)
                return ft_string();
            document.add_item(planet_group, active_item);
        }
    }

    char *raw = document.write_to_string();
    if (raw == ft_nullptr)
        return ft_string();
    ft_string payload(raw);
    cma_free(raw);
    return payload;
}

