#include "../libft/Libft/libft.hpp"
#include "../libft/Libft/limits.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/map.hpp"
#include "../libft/Template/pair.hpp"
#include "../libft/JSon/document.hpp"
#include "../libft/JSon/json.hpp"
#include "../libft/CMA/CMA.hpp"
#include "save_system.hpp"
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
    int used_plots = 0, int logistic_capacity = 0, int logistic_usage = 0)
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

    json_item *planet_id = document.create_item("id", 99);
    if (planet_id == ft_nullptr)
        return ft_string();
    document.add_item(planet_group, planet_id);

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

    json_item *next_instance = document.create_item("next_instance_id", 1);
    if (next_instance == ft_nullptr)
        return ft_string();
    document.add_item(planet_group, next_instance);

    char *raw = document.write_to_string();
    if (raw == ft_nullptr)
        return ft_string();
    ft_string payload(raw);
    cma_free(raw);
    return payload;
}

int verify_save_system_round_trip()
{
    SaveSystem saves;
    ft_map<int, ft_sharedptr<ft_planet> > planets;
    ft_sharedptr<ft_planet> terra(new ft_planet_terra());
    terra->register_resource(ORE_IRON, 1234567.890123);
    terra->set_resource(ORE_IRON, 4200);
    terra->set_carryover(ORE_IRON, 0.765432);
    terra->register_resource(ORE_COPPER, 345.678901);
    terra->set_resource(ORE_COPPER, 9001);
    terra->set_carryover(ORE_COPPER, 0.123456);
    terra->ensure_item_slot(ITEM_ENGINE_PART);
    terra->set_resource(ITEM_ENGINE_PART, 37);
    terra->ensure_item_slot(ITEM_FUSION_REACTOR);
    terra->set_resource(ITEM_FUSION_REACTOR, 2);
    planets.insert(PLANET_TERRA, terra);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    ft_sharedptr<ft_fleet> fleet(new ft_fleet(88));
    fleet->set_location_travel(PLANET_TERRA, PLANET_MARS, 512.204);
    fleet->set_escort_veterancy(3.141592);
    ft_ship cruiser;
    cruiser.id = 501;
    cruiser.type = SHIP_CAPITAL;
    cruiser.hp = 275;
    cruiser.max_hp = 420;
    cruiser.shield = 180;
    cruiser.max_shield = 220;
    cruiser.max_speed = 987.654321;
    cruiser.acceleration = 12.345678;
    cruiser.turn_speed = 210.987654;
    cruiser.combat_behavior = SHIP_BEHAVIOR_CHARGE;
    cruiser.role = SHIP_ROLE_LINE;
    fleet->add_ship_snapshot(cruiser);
    ft_ship escort;
    escort.id = 777;
    escort.type = SHIP_SHIELD;
    escort.hp = 120;
    escort.max_hp = 160;
    escort.shield = 240;
    escort.max_shield = 260;
    escort.max_speed = 654.321098;
    escort.acceleration = 9.876543;
    escort.turn_speed = 198.765432;
    escort.combat_behavior = SHIP_BEHAVIOR_SCREEN_SUPPORT;
    escort.role = SHIP_ROLE_SUPPORT;
    fleet->add_ship_snapshot(escort);
    fleets.insert(88, fleet);

    ft_string planet_json = saves.serialize_planets(planets);
    ft_string fleet_json = saves.serialize_fleets(fleets);
    FT_ASSERT(planet_json.size() > 0);
    FT_ASSERT(fleet_json.size() > 0);

    ft_map<int, ft_sharedptr<ft_planet> > restored_planets;
    ft_map<int, ft_sharedptr<ft_fleet> > restored_fleets;
    FT_ASSERT(saves.deserialize_planets(planet_json.c_str(), restored_planets));
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), restored_fleets));

    Pair<int, ft_sharedptr<ft_planet> > *terra_entry = restored_planets.find(PLANET_TERRA);
    FT_ASSERT(terra_entry != ft_nullptr);
    ft_sharedptr<ft_planet> restored_terra = terra_entry->value;
    FT_ASSERT(restored_terra);
    double iron_rate = restored_terra->get_rate(ORE_IRON);
    FT_ASSERT(ft_absolute(iron_rate - 1234567.890123) < 0.000001);
    FT_ASSERT_EQ(4200, restored_terra->get_resource(ORE_IRON));
    const ft_vector<Pair<int, double> > &terra_carry = restored_terra->get_carryover();
    double iron_carry = 0.0;
    for (size_t i = 0; i < terra_carry.size(); ++i)
    {
        if (terra_carry[i].key == ORE_IRON)
            iron_carry = terra_carry[i].value;
    }
    FT_ASSERT(ft_absolute(iron_carry - 0.765432) < 0.000001);
    FT_ASSERT_EQ(37, restored_terra->get_resource(ITEM_ENGINE_PART));
    FT_ASSERT_EQ(2, restored_terra->get_resource(ITEM_FUSION_REACTOR));

    Pair<int, ft_sharedptr<ft_fleet> > *fleet_entry = restored_fleets.find(88);
    FT_ASSERT(fleet_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_fleet = fleet_entry->value;
    FT_ASSERT(restored_fleet);
    ft_location restored_loc = restored_fleet->get_location();
    FT_ASSERT_EQ(LOCATION_TRAVEL, restored_loc.type);
    FT_ASSERT_EQ(PLANET_TERRA, restored_loc.from);
    FT_ASSERT_EQ(PLANET_MARS, restored_loc.to);
    FT_ASSERT(ft_absolute(restored_fleet->get_travel_time() - 512.204) < 0.000001);
    FT_ASSERT(ft_absolute(restored_fleet->get_escort_veterancy() - 3.141592) < 0.000001);
    const ft_ship *restored_cruiser = restored_fleet->get_ship(501);
    FT_ASSERT(restored_cruiser != ft_nullptr);
    FT_ASSERT(ft_absolute(restored_cruiser->max_speed - 987.654321) < 0.000001);
    FT_ASSERT_EQ(420, restored_cruiser->max_hp);
    const ft_ship *restored_escort = restored_fleet->get_ship(777);
    FT_ASSERT(restored_escort != ft_nullptr);
    FT_ASSERT(ft_absolute(restored_escort->acceleration - 9.876543) < 0.000001);
    FT_ASSERT_EQ(SHIP_ROLE_SUPPORT, restored_escort->role);

    return 1;
}

int verify_save_system_edge_cases()
{
    SaveSystem saves;

    ft_map<int, ft_sharedptr<ft_planet> > empty_planets;
    ft_string empty_planet_json = saves.serialize_planets(empty_planets);
    if (empty_planet_json.size() > 0)
    {
        ft_map<int, ft_sharedptr<ft_planet> > roundtrip_empty_planets;
        FT_ASSERT(saves.deserialize_planets(empty_planet_json.c_str(), roundtrip_empty_planets));
        FT_ASSERT_EQ(0u, roundtrip_empty_planets.size());
    }

    ft_map<int, ft_sharedptr<ft_fleet> > empty_fleets;
    ft_string empty_fleet_json = saves.serialize_fleets(empty_fleets);
    if (empty_fleet_json.size() > 0)
    {
        ft_map<int, ft_sharedptr<ft_fleet> > roundtrip_empty_fleets;
        FT_ASSERT(saves.deserialize_fleets(empty_fleet_json.c_str(), roundtrip_empty_fleets));
        FT_ASSERT_EQ(0u, roundtrip_empty_fleets.size());
    }

    json_document planet_doc;
    json_group *planet_group = planet_doc.create_group("planet_sparse");
    FT_ASSERT(planet_group != ft_nullptr);
    planet_doc.append_group(planet_group);
    json_item *planet_id_item = planet_doc.create_item("id", PLANET_MARS);
    FT_ASSERT(planet_id_item != ft_nullptr);
    planet_doc.add_item(planet_group, planet_id_item);
    json_item *amount_item = planet_doc.create_item("resource_1", 12);
    FT_ASSERT(amount_item != ft_nullptr);
    planet_doc.add_item(planet_group, amount_item);
    char *sparse_planet_raw = planet_doc.write_to_string();
    FT_ASSERT(sparse_planet_raw != ft_nullptr);
    ft_string sparse_planet_json(sparse_planet_raw);
    cma_free(sparse_planet_raw);
    ft_map<int, ft_sharedptr<ft_planet> > sparse_planets;
    FT_ASSERT(saves.deserialize_planets(sparse_planet_json.c_str(), sparse_planets));
    Pair<int, ft_sharedptr<ft_planet> > *sparse_entry = sparse_planets.find(PLANET_MARS);
    FT_ASSERT(sparse_entry != ft_nullptr);
    FT_ASSERT_EQ(12, sparse_entry->value->get_resource(ORE_IRON));
    FT_ASSERT(ft_absolute(sparse_entry->value->get_rate(ORE_IRON)) < 0.000001);

    json_document fleet_doc;
    json_group *fleet_group = fleet_doc.create_group("fleet_sparse");
    FT_ASSERT(fleet_group != ft_nullptr);
    fleet_doc.append_group(fleet_group);
    json_item *fleet_id_item = fleet_doc.create_item("id", 55);
    FT_ASSERT(fleet_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_id_item);
    json_item *fleet_ship_count = fleet_doc.create_item("ship_count", 1);
    FT_ASSERT(fleet_ship_count != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_ship_count);
    json_item *fleet_ship_id = fleet_doc.create_item("ship_0_id", 333);
    FT_ASSERT(fleet_ship_id != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_ship_id);
    char *sparse_fleet_raw = fleet_doc.write_to_string();
    FT_ASSERT(sparse_fleet_raw != ft_nullptr);
    ft_string sparse_fleet_json(sparse_fleet_raw);
    cma_free(sparse_fleet_raw);
    ft_map<int, ft_sharedptr<ft_fleet> > sparse_fleets;
    FT_ASSERT(saves.deserialize_fleets(sparse_fleet_json.c_str(), sparse_fleets));
    Pair<int, ft_sharedptr<ft_fleet> > *sparse_fleet_entry = sparse_fleets.find(55);
    FT_ASSERT(sparse_fleet_entry != ft_nullptr);
    FT_ASSERT_EQ(1, sparse_fleet_entry->value->get_ship_count());
    const ft_ship *sparse_ship = sparse_fleet_entry->value->get_ship(333);
    FT_ASSERT(sparse_ship != ft_nullptr);
    FT_ASSERT_EQ(0, sparse_ship->hp);
    FT_ASSERT_EQ(0, sparse_ship->shield);
    ft_location sparse_location = sparse_fleet_entry->value->get_location();
    FT_ASSERT_EQ(LOCATION_PLANET, sparse_location.type);
    FT_ASSERT_EQ(PLANET_TERRA, sparse_location.from);
    FT_ASSERT_EQ(PLANET_TERRA, sparse_location.to);
    FT_ASSERT(ft_absolute(sparse_fleet_entry->value->get_escort_veterancy()) < 0.000001);

    return 1;
}

int verify_save_system_invalid_inputs()
{
    SaveSystem saves;

    ft_map<int, ft_sharedptr<ft_planet> > planets;
    ft_sharedptr<ft_planet> terra(new ft_planet_terra());
    terra->register_resource(ORE_IRON, 12.5);
    terra->set_resource(ORE_IRON, 24);
    planets.insert(PLANET_TERRA, terra);
    FT_ASSERT(!saves.deserialize_planets(ft_nullptr, planets));
    Pair<int, ft_sharedptr<ft_planet> > *terra_entry = planets.find(PLANET_TERRA);
    FT_ASSERT(terra_entry != ft_nullptr);
    FT_ASSERT_EQ(24, terra_entry->value->get_resource(ORE_IRON));
    ft_string invalid_payload("not json");
    FT_ASSERT(!saves.deserialize_planets(invalid_payload.c_str(), planets));
    terra_entry = planets.find(PLANET_TERRA);
    FT_ASSERT(terra_entry != ft_nullptr);
    FT_ASSERT_EQ(24, terra_entry->value->get_resource(ORE_IRON));

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    ft_sharedptr<ft_fleet> escort(new ft_fleet(31));
    escort->set_location_planet(PLANET_MARS);
    fleets.insert(31, escort);
    FT_ASSERT(!saves.deserialize_fleets(ft_nullptr, fleets));
    Pair<int, ft_sharedptr<ft_fleet> > *escort_entry = fleets.find(31);
    FT_ASSERT(escort_entry != ft_nullptr);
    FT_ASSERT_EQ(PLANET_MARS, escort_entry->value->get_location().from);
    FT_ASSERT(!saves.deserialize_fleets(invalid_payload.c_str(), fleets));
    escort_entry = fleets.find(31);
    FT_ASSERT(escort_entry != ft_nullptr);
    FT_ASSERT_EQ(PLANET_MARS, escort_entry->value->get_location().from);

    ResearchManager research;
    research.set_duration_scale(2.0);
    FT_ASSERT(research.start(RESEARCH_UNLOCK_MARS));
    ft_vector<int> completed;
    research.tick(3.0, completed);
    double remaining_before = research.get_remaining_time(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(remaining_before > 0.0);
    FT_ASSERT(!saves.deserialize_research(ft_nullptr, research));
    FT_ASSERT(ft_absolute(research.get_remaining_time(RESEARCH_UNLOCK_MARS)
        - remaining_before) < 0.000001);
    FT_ASSERT(ft_absolute(research.get_duration_scale() - 2.0) < 0.000001);
    FT_ASSERT(!saves.deserialize_research(invalid_payload.c_str(), research));
    FT_ASSERT(ft_absolute(research.get_remaining_time(RESEARCH_UNLOCK_MARS)
        - remaining_before) < 0.000001);

    AchievementManager achievements;
    achievements.record_event(ACHIEVEMENT_EVENT_PLANET_UNLOCKED, 1);
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED,
        achievements.get_status(ACHIEVEMENT_SECOND_HOME));
    FT_ASSERT(!saves.deserialize_achievements(ft_nullptr, achievements));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED,
        achievements.get_status(ACHIEVEMENT_SECOND_HOME));
    FT_ASSERT(!saves.deserialize_achievements(invalid_payload.c_str(), achievements));
    FT_ASSERT_EQ(1, achievements.get_progress(ACHIEVEMENT_SECOND_HOME));

    return 1;
}

int verify_save_system_rejects_oversized_building_grids()
{
    SaveSystem saves;
    BuildingManager buildings;

    ft_string excessive_width = save_system_building_payload(2000000, 1);
    FT_ASSERT(excessive_width.size() > 0);
    FT_ASSERT(!saves.deserialize_buildings(excessive_width.c_str(), buildings));
    FT_ASSERT_EQ(0, buildings.get_planet_plot_capacity(99));

    ft_string excessive_height = save_system_building_payload(1, 2000000);
    FT_ASSERT(excessive_height.size() > 0);
    FT_ASSERT(!saves.deserialize_buildings(excessive_height.c_str(), buildings));
    FT_ASSERT_EQ(0, buildings.get_planet_plot_capacity(99));

    ft_string excessive_area = save_system_building_payload(2048, 2048);
    FT_ASSERT(excessive_area.size() > 0);
    FT_ASSERT(!saves.deserialize_buildings(excessive_area.c_str(), buildings));
    FT_ASSERT_EQ(0, buildings.get_planet_plot_capacity(99));

    ft_string valid_payload = save_system_building_payload(4, 4, 37, 5, 12);
    FT_ASSERT(valid_payload.size() > 0);
    FT_ASSERT(saves.deserialize_buildings(valid_payload.c_str(), buildings));
    FT_ASSERT_EQ(16, buildings.get_planet_plot_capacity(99));
    FT_ASSERT_EQ(16, buildings.get_planet_plot_usage(99));
    FT_ASSERT_EQ(5, buildings.get_planet_logistic_capacity(99));
    FT_ASSERT_EQ(5, buildings.get_planet_logistic_usage(99));

    return 1;
}

int verify_save_system_rejects_overlarge_ship_ids()
{
    SaveSystem saves;

    ft_fleet baseline;
    int baseline_id = baseline.create_ship(SHIP_SHIELD);
    FT_ASSERT(baseline_id > 0);
    FT_ASSERT(baseline_id < FT_INT_MAX);

    json_document fleet_doc;
    json_group *fleet_group = fleet_doc.create_group("fleet_overlarge_ship");
    FT_ASSERT(fleet_group != ft_nullptr);
    fleet_doc.append_group(fleet_group);

    json_item *fleet_id_item = fleet_doc.create_item("id", 5100);
    FT_ASSERT(fleet_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_id_item);
    json_item *ship_count_item = fleet_doc.create_item("ship_count", 1);
    FT_ASSERT(ship_count_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_count_item);
    json_item *ship_id_item = fleet_doc.create_item("ship_0_id", FT_INT_MAX);
    FT_ASSERT(ship_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_id_item);

    char *fleet_raw = fleet_doc.write_to_string();
    FT_ASSERT(fleet_raw != ft_nullptr);
    ft_string fleet_json(fleet_raw);
    cma_free(fleet_raw);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    FT_ASSERT(!saves.deserialize_fleets(fleet_json.c_str(), fleets));

    ft_fleet followup;
    int next_id = followup.create_ship(SHIP_SHIELD);
    FT_ASSERT_EQ(baseline_id + 1, next_id);

    return 1;
}

int verify_save_system_limits_inflated_ship_counts()
{
    SaveSystem saves;

    json_document fleet_doc;
    json_group *fleet_group = fleet_doc.create_group("fleet_inflated");
    FT_ASSERT(fleet_group != ft_nullptr);
    fleet_doc.append_group(fleet_group);

    json_item *fleet_id_item = fleet_doc.create_item("id", 612);
    FT_ASSERT(fleet_id_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, fleet_id_item);
    json_item *ship_count_item = fleet_doc.create_item("ship_count", 512);
    FT_ASSERT(ship_count_item != ft_nullptr);
    fleet_doc.add_item(fleet_group, ship_count_item);

    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_id", 3100));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_type", SHIP_CAPITAL));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_0_hp", 300));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_1_id", 3200));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_1_type", SHIP_SHIELD));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_1_role", SHIP_ROLE_SUPPORT));
    fleet_doc.add_item(fleet_group, fleet_doc.create_item("ship_90_id", FT_INT_MAX));

    char *fleet_raw = fleet_doc.write_to_string();
    FT_ASSERT(fleet_raw != ft_nullptr);
    ft_string fleet_json(fleet_raw);
    cma_free(fleet_raw);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), fleets));
    FT_ASSERT_EQ(1u, fleets.size());

    Pair<int, ft_sharedptr<ft_fleet> > *fleet_entry = fleets.find(612);
    FT_ASSERT(fleet_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_fleet = fleet_entry->value;
    FT_ASSERT(restored_fleet);
    FT_ASSERT_EQ(2, restored_fleet->get_ship_count());
    const ft_ship *ship_zero = restored_fleet->get_ship(3100);
    FT_ASSERT(ship_zero != ft_nullptr);
    FT_ASSERT_EQ(SHIP_CAPITAL, ship_zero->type);
    FT_ASSERT_EQ(300, ship_zero->hp);
    const ft_ship *ship_one = restored_fleet->get_ship(3200);
    FT_ASSERT(ship_one != ft_nullptr);
    FT_ASSERT_EQ(SHIP_SHIELD, ship_one->type);
    FT_ASSERT_EQ(SHIP_ROLE_SUPPORT, ship_one->role);
    const ft_ship *missing_ship = restored_fleet->get_ship(FT_INT_MAX);
    FT_ASSERT(missing_ship == ft_nullptr);

    return 1;
}

int validate_save_system_serialized_samples()
{
    SaveSystem saves;

    ft_map<int, ft_sharedptr<ft_planet> > planets;
    ft_sharedptr<ft_planet> vulcan(new ft_planet_vulcan());
    double large_rate = 123456789012.345678;
    double large_carryover = 6543210.987654;
    int large_stock = 2147480000;
    vulcan->register_resource(ORE_GOLD, large_rate);
    vulcan->set_resource(ORE_GOLD, large_stock);
    vulcan->set_carryover(ORE_GOLD, large_carryover);
    planets.insert(PLANET_VULCAN, vulcan);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    ft_sharedptr<ft_fleet> armada(new ft_fleet(404));
    double travel_time = 654321.987654;
    double veterancy = 87654.321987;
    armada->set_location_travel(PLANET_VULCAN, PLANET_MARS, travel_time);
    armada->set_escort_veterancy(veterancy);
    ft_ship dreadnought;
    dreadnought.id = 9001;
    dreadnought.type = SHIP_CAPITAL_DREADNOUGHT;
    dreadnought.armor = 987654321;
    dreadnought.hp = 1900000000;
    dreadnought.shield = 1850000000;
    dreadnought.max_hp = 2000000000;
    dreadnought.max_shield = 2100000000;
    dreadnought.max_speed = 43210.987654;
    dreadnought.acceleration = 321.987654;
    dreadnought.turn_speed = 210.123456;
    dreadnought.combat_behavior = SHIP_BEHAVIOR_LAST_STAND;
    dreadnought.outnumbered_behavior = SHIP_BEHAVIOR_CHARGE;
    dreadnought.unescorted_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;
    dreadnought.low_hp_behavior = SHIP_BEHAVIOR_FLANK_SWEEP;
    dreadnought.role = SHIP_ROLE_SUPPORT;
    armada->add_ship_snapshot(dreadnought);
    fleets.insert(404, armada);

    ft_string planet_json = saves.serialize_planets(planets);
    FT_ASSERT(planet_json.size() > 0);
    ft_string fleet_json = saves.serialize_fleets(fleets);
    FT_ASSERT(fleet_json.size() > 0);

    json_group *planet_groups = json_read_from_string(planet_json.c_str());
    FT_ASSERT(planet_groups != ft_nullptr);
    json_group *planet_group = planet_groups;
    while (planet_group)
    {
        json_item *id_item = json_find_item(planet_group, "id");
        if (id_item && ft_atoi(id_item->value) == PLANET_VULCAN)
            break;
        planet_group = planet_group->next;
    }
    FT_ASSERT(planet_group != ft_nullptr);
    ft_string rate_key = "rate_";
    rate_key.append(ft_to_string(ORE_GOLD));
    json_item *rate_item = json_find_item(planet_group, rate_key.c_str());
    FT_ASSERT(rate_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(large_rate), ft_atol(rate_item->value));
    ft_string carry_key = "carryover_";
    carry_key.append(ft_to_string(ORE_GOLD));
    json_item *carry_item = json_find_item(planet_group, carry_key.c_str());
    FT_ASSERT(carry_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(large_carryover), ft_atol(carry_item->value));
    ft_string amount_key = "resource_";
    amount_key.append(ft_to_string(ORE_GOLD));
    json_item *amount_item = json_find_item(planet_group, amount_key.c_str());
    FT_ASSERT(amount_item != ft_nullptr);
    FT_ASSERT_EQ(large_stock, ft_atoi(amount_item->value));
    json_free_groups(planet_groups);

    json_group *fleet_groups = json_read_from_string(fleet_json.c_str());
    FT_ASSERT(fleet_groups != ft_nullptr);
    json_group *fleet_group = fleet_groups;
    while (fleet_group)
    {
        json_item *fleet_id = json_find_item(fleet_group, "id");
        if (fleet_id && ft_atoi(fleet_id->value) == 404)
            break;
        fleet_group = fleet_group->next;
    }
    FT_ASSERT(fleet_group != ft_nullptr);
    json_item *travel_item = json_find_item(fleet_group, "travel_time");
    FT_ASSERT(travel_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(travel_time), ft_atol(travel_item->value));
    json_item *veterancy_item = json_find_item(fleet_group, "escort_veterancy");
    FT_ASSERT(veterancy_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(veterancy), ft_atol(veterancy_item->value));
    json_item *ship_count_item = json_find_item(fleet_group, "ship_count");
    FT_ASSERT(ship_count_item != ft_nullptr);
    FT_ASSERT_EQ(1, ft_atoi(ship_count_item->value));
    ft_string ship_base = "ship_";
    ship_base.append(ft_to_string(0));
    ft_string ship_speed_key = ship_base;
    ship_speed_key.append("_max_speed");
    json_item *ship_speed_item = json_find_item(fleet_group, ship_speed_key.c_str());
    FT_ASSERT(ship_speed_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(dreadnought.max_speed), ft_atol(ship_speed_item->value));
    ft_string ship_accel_key = ship_base;
    ship_accel_key.append("_acceleration");
    json_item *ship_accel_item = json_find_item(fleet_group, ship_accel_key.c_str());
    FT_ASSERT(ship_accel_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(dreadnought.acceleration), ft_atol(ship_accel_item->value));
    ft_string ship_turn_key = ship_base;
    ship_turn_key.append("_turn_speed");
    json_item *ship_turn_item = json_find_item(fleet_group, ship_turn_key.c_str());
    FT_ASSERT(ship_turn_item != ft_nullptr);
    FT_ASSERT_EQ(reference_scale_double(dreadnought.turn_speed), ft_atol(ship_turn_item->value));
    json_free_groups(fleet_groups);

    ft_map<int, ft_sharedptr<ft_planet> > restored_planets;
    FT_ASSERT(saves.deserialize_planets(planet_json.c_str(), restored_planets));
    Pair<int, ft_sharedptr<ft_planet> > *vulcan_entry = restored_planets.find(PLANET_VULCAN);
    FT_ASSERT(vulcan_entry != ft_nullptr);
    double restored_rate = vulcan_entry->value->get_rate(ORE_GOLD);
    FT_ASSERT(ft_absolute(restored_rate - large_rate) < 0.000001);
    FT_ASSERT_EQ(large_stock, vulcan_entry->value->get_resource(ORE_GOLD));
    const ft_vector<Pair<int, double> > &restored_carry = vulcan_entry->value->get_carryover();
    double carry_value = 0.0;
    for (size_t i = 0; i < restored_carry.size(); ++i)
    {
        if (restored_carry[i].key == ORE_GOLD)
            carry_value = restored_carry[i].value;
    }
    FT_ASSERT(ft_absolute(carry_value - large_carryover) < 0.000001);

    ft_map<int, ft_sharedptr<ft_fleet> > restored_fleets;
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), restored_fleets));
    Pair<int, ft_sharedptr<ft_fleet> > *armada_entry = restored_fleets.find(404);
    FT_ASSERT(armada_entry != ft_nullptr);
    FT_ASSERT(ft_absolute(armada_entry->value->get_travel_time() - travel_time) < 0.000001);
    FT_ASSERT(ft_absolute(armada_entry->value->get_escort_veterancy() - veterancy) < 0.000001);
    const ft_ship *restored_dreadnought = armada_entry->value->get_ship(9001);
    FT_ASSERT(restored_dreadnought != ft_nullptr);
    FT_ASSERT_EQ(dreadnought.armor, restored_dreadnought->armor);
    FT_ASSERT_EQ(dreadnought.hp, restored_dreadnought->hp);
    FT_ASSERT_EQ(dreadnought.shield, restored_dreadnought->shield);
    FT_ASSERT_EQ(dreadnought.max_hp, restored_dreadnought->max_hp);
    FT_ASSERT_EQ(dreadnought.max_shield, restored_dreadnought->max_shield);
    FT_ASSERT(ft_absolute(restored_dreadnought->max_speed - dreadnought.max_speed) < 0.000001);
    FT_ASSERT(ft_absolute(restored_dreadnought->acceleration - dreadnought.acceleration) < 0.000001);
    FT_ASSERT(ft_absolute(restored_dreadnought->turn_speed - dreadnought.turn_speed) < 0.000001);
    FT_ASSERT_EQ(dreadnought.combat_behavior, restored_dreadnought->combat_behavior);
    FT_ASSERT_EQ(dreadnought.outnumbered_behavior, restored_dreadnought->outnumbered_behavior);
    FT_ASSERT_EQ(dreadnought.unescorted_behavior, restored_dreadnought->unescorted_behavior);
    FT_ASSERT_EQ(dreadnought.low_hp_behavior, restored_dreadnought->low_hp_behavior);
    FT_ASSERT_EQ(dreadnought.role, restored_dreadnought->role);

    return 1;
}

int verify_save_system_allocation_failures()
{
    SaveSystem saves;
    SaveSystem::set_json_allocation_hook(save_system_test_allocation_hook);

    ft_map<int, ft_sharedptr<ft_planet> > planets;
    ft_sharedptr<ft_planet> terra(new ft_planet_terra());
    terra->register_resource(ORE_IRON, 1.25);
    terra->set_resource(ORE_IRON, 10);
    planets.insert(PLANET_TERRA, terra);

    save_system_trigger_allocation_failure("group", "planet_1");
    ft_string planet_json = saves.serialize_planets(planets);
    FT_ASSERT_EQ(0u, planet_json.size());

    ft_string recovered_planet = saves.serialize_planets(planets);
    FT_ASSERT(recovered_planet.size() > 0);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    ft_sharedptr<ft_fleet> escort(new ft_fleet(31));
    escort->set_location_planet(PLANET_TERRA);
    ft_ship snapshot;
    snapshot.id = 512;
    snapshot.type = SHIP_SHIELD;
    escort->add_ship_snapshot(snapshot);
    fleets.insert(31, escort);

    save_system_trigger_allocation_failure("item", "ship_count");
    ft_string fleet_json = saves.serialize_fleets(fleets);
    FT_ASSERT_EQ(0u, fleet_json.size());

    ft_string recovered_fleet = saves.serialize_fleets(fleets);
    FT_ASSERT(recovered_fleet.size() > 0);

    save_system_reset_allocation_hook();

    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    SaveSystem::set_json_allocation_hook(save_system_test_allocation_hook);
    save_system_trigger_allocation_failure("group", "planet_1");
    FT_ASSERT(!game.save_campaign_checkpoint(ft_string("allocation_failure")));
    save_system_reset_allocation_hook();

    return 1;
}

int verify_save_system_extreme_scaling()
{
    SaveSystem saves;

    ft_map<int, ft_sharedptr<ft_planet> > planets;
    ft_sharedptr<ft_planet> terra(new ft_planet_terra());
    double huge_positive = save_system_test_double_max() / 4.0;
    double huge_negative = -save_system_test_double_max() / 4.0;
    double nan_value = save_system_test_quiet_nan();
    double positive_infinity = save_system_test_positive_infinity();
    double negative_infinity = save_system_test_negative_infinity();

    terra->register_resource(ORE_IRON, huge_positive);
    terra->set_carryover(ORE_IRON, nan_value);
    terra->register_resource(ORE_COPPER, huge_negative);
    terra->set_carryover(ORE_COPPER, positive_infinity);
    terra->register_resource(ORE_GOLD, positive_infinity);
    terra->set_carryover(ORE_GOLD, negative_infinity);
    terra->register_resource(ORE_COAL, negative_infinity);
    planets.insert(PLANET_TERRA, terra);

    ft_string planet_json = saves.serialize_planets(planets);
    FT_ASSERT(planet_json.size() > 0);

    ft_map<int, ft_sharedptr<ft_planet> > restored_planets;
    FT_ASSERT(saves.deserialize_planets(planet_json.c_str(), restored_planets));

    Pair<int, ft_sharedptr<ft_planet> > *terra_entry = restored_planets.find(PLANET_TERRA);
    FT_ASSERT(terra_entry != ft_nullptr);
    ft_sharedptr<ft_planet> restored_terra = terra_entry->value;
    FT_ASSERT(restored_terra);

    const long scale = 1000000;
    const double scale_double = static_cast<double>(scale);
    double expected_max = static_cast<double>(FT_LONG_MAX - 1L)
        / scale_double;
    double expected_min = static_cast<double>(FT_LONG_MIN + 2L)
        / scale_double;

    double restored_large_positive = restored_terra->get_rate(ORE_IRON);
    FT_ASSERT(ft_absolute(restored_large_positive - expected_max) < 0.000001);

    double restored_large_negative = restored_terra->get_rate(ORE_COPPER);
    FT_ASSERT(ft_absolute(restored_large_negative - expected_min) < 0.000001);

    double restored_positive_inf = restored_terra->get_rate(ORE_GOLD);
    FT_ASSERT(ft_absolute(restored_positive_inf) < 0.000001);

    double restored_negative_inf = restored_terra->get_rate(ORE_COAL);
    FT_ASSERT(ft_absolute(restored_negative_inf) < 0.000001);

    const ft_vector<Pair<int, double> > &carry = restored_terra->get_carryover();
    bool found_iron = false;
    bool found_copper = false;
    bool found_gold = false;
    double carry_iron = 0.0;
    double carry_copper = 0.0;
    double carry_gold = 0.0;
    for (size_t i = 0; i < carry.size(); ++i)
    {
        if (carry[i].key == ORE_IRON)
        {
            carry_iron = carry[i].value;
            found_iron = true;
        }
        else if (carry[i].key == ORE_COPPER)
        {
            carry_copper = carry[i].value;
            found_copper = true;
        }
        else if (carry[i].key == ORE_GOLD)
        {
            carry_gold = carry[i].value;
            found_gold = true;
        }
    }

    FT_ASSERT(found_iron);
    FT_ASSERT(ft_absolute(carry_iron) < 0.000001);

    FT_ASSERT(found_copper);
    FT_ASSERT(ft_absolute(carry_copper) < 0.000001);

    FT_ASSERT(found_gold);
    FT_ASSERT(ft_absolute(carry_gold) < 0.000001);

    return 1;
}

int verify_save_system_normalizes_non_finite_planet_values()
{
    SaveSystem saves;

    json_document document;
    json_group *planet_group = document.create_group("planet_non_finite");
    FT_ASSERT(planet_group != ft_nullptr);
    document.append_group(planet_group);

    json_item *id_item = document.create_item("id", 7777);
    FT_ASSERT(id_item != ft_nullptr);
    document.add_item(planet_group, id_item);

    json_item *resource_nan_item = document.create_item("resource_500", 10);
    FT_ASSERT(resource_nan_item != ft_nullptr);
    document.add_item(planet_group, resource_nan_item);

    json_item *resource_inf_item = document.create_item("resource_501", 5);
    FT_ASSERT(resource_inf_item != ft_nullptr);
    document.add_item(planet_group, resource_inf_item);

    ft_string rate_nan_string = ft_to_string(FT_LONG_MIN);
    json_item *rate_nan_item = document.create_item("rate_500",
        rate_nan_string.c_str());
    FT_ASSERT(rate_nan_item != ft_nullptr);
    document.add_item(planet_group, rate_nan_item);

    ft_string rate_infinite_string = ft_to_string(FT_LONG_MAX);
    json_item *rate_infinite_item = document.create_item("rate_501",
        rate_infinite_string.c_str());
    FT_ASSERT(rate_infinite_item != ft_nullptr);
    document.add_item(planet_group, rate_infinite_item);

    long negative_infinity_sentinel = FT_LONG_MIN + 1L;
    ft_string carry_nan_string = ft_to_string(FT_LONG_MIN);
    ft_string carry_negative_inf_string = ft_to_string(negative_infinity_sentinel);

    json_item *carry_nan_item = document.create_item("carryover_500",
        carry_nan_string.c_str());
    FT_ASSERT(carry_nan_item != ft_nullptr);
    document.add_item(planet_group, carry_nan_item);

    json_item *carry_negative_inf_item = document.create_item("carryover_501",
        carry_negative_inf_string.c_str());
    FT_ASSERT(carry_negative_inf_item != ft_nullptr);
    document.add_item(planet_group, carry_negative_inf_item);

    char *serialized_planet = document.write_to_string();
    FT_ASSERT(serialized_planet != ft_nullptr);
    ft_string planet_json(serialized_planet);
    cma_free(serialized_planet);

    ft_map<int, ft_sharedptr<ft_planet> > planets;
    FT_ASSERT(saves.deserialize_planets(planet_json.c_str(), planets));

    Pair<int, ft_sharedptr<ft_planet> > *planet_entry = planets.find(7777);
    FT_ASSERT(planet_entry != ft_nullptr);
    ft_sharedptr<ft_planet> planet = planet_entry->value;
    FT_ASSERT(planet);

    FT_ASSERT(ft_absolute(planet->get_rate(500)) < 0.000001);
    FT_ASSERT(ft_absolute(planet->get_rate(501)) < 0.000001);

    const ft_vector<Pair<int, double> > &carry = planet->get_carryover();
    bool found_nan_resource = false;
    bool found_inf_resource = false;
    double carry_nan_resource = 0.0;
    double carry_inf_resource = 0.0;
    for (size_t idx = 0; idx < carry.size(); ++idx)
    {
        if (carry[idx].key == 500)
        {
            carry_nan_resource = carry[idx].value;
            found_nan_resource = true;
        }
        else if (carry[idx].key == 501)
        {
            carry_inf_resource = carry[idx].value;
            found_inf_resource = true;
        }
    }

    FT_ASSERT(found_nan_resource);
    FT_ASSERT(ft_absolute(carry_nan_resource) < 0.000001);
    FT_ASSERT(found_inf_resource);
    FT_ASSERT(ft_absolute(carry_inf_resource) < 0.000001);

    ft_vector<Pair<int, int> > produced = planet->produce(5.0);
    FT_ASSERT_EQ(0u, produced.size());

    return 1;
}

int verify_save_system_massive_payload()
{
    SaveSystem saves;

    const int planet_base_id = 1000;
    const int planet_count = 18;
    const int resource_count = 16;
    const int item_count = 12;
    ft_map<int, ft_sharedptr<ft_planet> > planets;
    for (int planet_index = 0; planet_index < planet_count; ++planet_index)
    {
        int planet_id = planet_base_id + planet_index;
        ft_sharedptr<ft_planet> planet(new ft_planet(planet_id));
        FT_ASSERT(planet);
        for (int resource_index = 0; resource_index < resource_count; ++resource_index)
        {
            int resource_id = 2000 + resource_index;
            double base_rate = static_cast<double>((planet_index + 1)
                * (resource_index + 1));
            double rate;
            if ((resource_index % 3) == 0)
                rate = -base_rate * 0.5;
            else
                rate = base_rate * 0.125 + static_cast<double>(planet_index);
            planet->register_resource(resource_id, rate);
            int amount = (planet_index + 1) * (resource_index + 1) * 7;
            planet->set_resource(resource_id, amount);
            double carry_base = static_cast<double>((planet_index + 1)
                * (resource_index + 1));
            double carry;
            if ((resource_index % 2) == 0)
                carry = 0.25 * carry_base;
            else
                carry = -0.125 * carry_base;
            planet->set_carryover(resource_id, carry);
        }
        for (int item_index = 0; item_index < item_count; ++item_index)
        {
            int item_id = 4000 + item_index;
            planet->ensure_item_slot(item_id);
            int amount = (planet_index + 1) * (item_index + 1) * 11;
            planet->set_resource(item_id, amount);
        }
        planets.insert(planet_id, planet);
    }

    const int fleet_base_id = 900;
    const int fleet_count = 24;
    const int ships_per_fleet = 10;
    const int ship_types[] = {
        SHIP_SHIELD,
        SHIP_RADAR,
        SHIP_SALVAGE,
        SHIP_CAPITAL,
        SHIP_TRANSPORT,
        SHIP_CORVETTE,
        SHIP_INTERCEPTOR,
        SHIP_FRIGATE_SUPPORT
    };
    const int ship_type_count = static_cast<int>(sizeof(ship_types)
        / sizeof(ship_types[0]));
    const int ship_roles[] = {
        SHIP_ROLE_LINE,
        SHIP_ROLE_SUPPORT,
        SHIP_ROLE_TRANSPORT
    };
    const int ship_role_count = static_cast<int>(sizeof(ship_roles)
        / sizeof(ship_roles[0]));
    const int behavior_cycle[] = {
        SHIP_BEHAVIOR_LINE_HOLD,
        SHIP_BEHAVIOR_FLANK_SWEEP,
        SHIP_BEHAVIOR_SCREEN_SUPPORT,
        SHIP_BEHAVIOR_CHARGE,
        SHIP_BEHAVIOR_RETREAT,
        SHIP_BEHAVIOR_WITHDRAW_SUPPORT,
        SHIP_BEHAVIOR_LAST_STAND
    };
    const int behavior_count = static_cast<int>(sizeof(behavior_cycle)
        / sizeof(behavior_cycle[0]));
    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    for (int fleet_index = 0; fleet_index < fleet_count; ++fleet_index)
    {
        int fleet_id = fleet_base_id + fleet_index;
        ft_sharedptr<ft_fleet> fleet(new ft_fleet(fleet_id));
        FT_ASSERT(fleet);
        if ((fleet_index % 3) == 0)
            fleet->set_location_planet(PLANET_TERRA);
        else if ((fleet_index % 3) == 1)
            fleet->set_location_travel(PLANET_TERRA, PLANET_MARS,
                60.0 + static_cast<double>(fleet_index) * 3.5);
        else
            fleet->set_location_misc(MISC_OUTPOST_NEBULA_X);
        double veterancy = static_cast<double>(fleet_index) * 0.75;
        if ((fleet_index % 2) == 1)
            veterancy += 1.0;
        fleet->set_escort_veterancy(veterancy);
        for (int ship_index = 0; ship_index < ships_per_fleet; ++ship_index)
        {
            ft_ship snapshot;
            snapshot.id = 5000 + fleet_index * 100 + ship_index;
            snapshot.type = ship_types[ship_index % ship_type_count];
            snapshot.armor = (fleet_index + 1) * (ship_index + 1) * 3;
            snapshot.hp = (fleet_index + 2) * (ship_index + 5);
            snapshot.shield = (fleet_index + 3) * (ship_index + 2);
            snapshot.max_hp = snapshot.hp + 50;
            snapshot.max_shield = snapshot.shield + 25;
            snapshot.max_speed = 10.0 + static_cast<double>(ship_index) * 2.5
                + static_cast<double>(fleet_index) * 0.25;
            snapshot.acceleration = 1.0
                + static_cast<double>(ship_index) * 0.5
                + static_cast<double>(fleet_index) * 0.1;
            snapshot.turn_speed = 30.0
                + static_cast<double>(ship_index) * 1.75
                + static_cast<double>(fleet_index) * 0.5;
            snapshot.combat_behavior = behavior_cycle[(ship_index + fleet_index)
                % behavior_count];
            snapshot.outnumbered_behavior = behavior_cycle[(ship_index
                + fleet_index + 1) % behavior_count];
            snapshot.unescorted_behavior = behavior_cycle[(ship_index
                + fleet_index + 2) % behavior_count];
            snapshot.low_hp_behavior = behavior_cycle[(ship_index + fleet_index + 3)
                % behavior_count];
            snapshot.role = ship_roles[(ship_index + fleet_index) % ship_role_count];
            fleet->add_ship_snapshot(snapshot);
        }
        fleets.insert(fleet_id, fleet);
    }

    ft_string planet_json = saves.serialize_planets(planets);
    ft_string fleet_json = saves.serialize_fleets(fleets);
    FT_ASSERT(planet_json.size() > 0);
    FT_ASSERT(fleet_json.size() > 0);

    ft_map<int, ft_sharedptr<ft_planet> > restored_planets;
    ft_map<int, ft_sharedptr<ft_fleet> > restored_fleets;
    FT_ASSERT(saves.deserialize_planets(planet_json.c_str(), restored_planets));
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), restored_fleets));
    FT_ASSERT_EQ(planets.size(), restored_planets.size());
    FT_ASSERT_EQ(fleets.size(), restored_fleets.size());

    size_t restored_planet_count = restored_planets.size();
    const Pair<int, ft_sharedptr<ft_planet> > *planet_entries = restored_planets.end();
    planet_entries -= restored_planet_count;
    for (size_t idx = 0; idx < restored_planet_count; ++idx)
    {
        int planet_id = planet_entries[idx].key;
        int planet_index = planet_id - planet_base_id;
        FT_ASSERT(planet_index >= 0 && planet_index < planet_count);
        ft_sharedptr<ft_planet> restored_planet = planet_entries[idx].value;
        FT_ASSERT(restored_planet);
        for (int resource_index = 0; resource_index < resource_count; ++resource_index)
        {
            int resource_id = 2000 + resource_index;
            double base_rate = static_cast<double>((planet_index + 1)
                * (resource_index + 1));
            double expected_rate;
            if ((resource_index % 3) == 0)
                expected_rate = -base_rate * 0.5;
            else
                expected_rate = base_rate * 0.125
                    + static_cast<double>(planet_index);
            double restored_rate = restored_planet->get_rate(resource_id);
            FT_ASSERT(ft_absolute(restored_rate - expected_rate) < 0.000001);
            int expected_amount = (planet_index + 1) * (resource_index + 1) * 7;
            FT_ASSERT_EQ(expected_amount, restored_planet->get_resource(resource_id));
            double carry_base = static_cast<double>((planet_index + 1)
                * (resource_index + 1));
            double expected_carry;
            if ((resource_index % 2) == 0)
                expected_carry = 0.25 * carry_base;
            else
                expected_carry = -0.125 * carry_base;
            double restored_carry = 0.0;
            bool found_carry = false;
            const ft_vector<Pair<int, double> > &carry_entries = restored_planet->get_carryover();
            for (size_t carry_idx = 0; carry_idx < carry_entries.size(); ++carry_idx)
            {
                if (carry_entries[carry_idx].key == resource_id)
                {
                    restored_carry = carry_entries[carry_idx].value;
                    found_carry = true;
                    break;
                }
            }
            FT_ASSERT(found_carry);
            FT_ASSERT(ft_absolute(restored_carry - expected_carry) < 0.000001);
        }
        for (int item_index = 0; item_index < item_count; ++item_index)
        {
            int item_id = 4000 + item_index;
            int expected_amount = (planet_index + 1) * (item_index + 1) * 11;
            FT_ASSERT_EQ(expected_amount, restored_planet->get_resource(item_id));
        }
    }

    size_t restored_fleet_count = restored_fleets.size();
    const Pair<int, ft_sharedptr<ft_fleet> > *fleet_entries = restored_fleets.end();
    fleet_entries -= restored_fleet_count;
    for (size_t idx = 0; idx < restored_fleet_count; ++idx)
    {
        int fleet_id = fleet_entries[idx].key;
        int fleet_index = fleet_id - fleet_base_id;
        FT_ASSERT(fleet_index >= 0 && fleet_index < fleet_count);
        ft_sharedptr<ft_fleet> restored_fleet = fleet_entries[idx].value;
        FT_ASSERT(restored_fleet);
        ft_location location = restored_fleet->get_location();
        if ((fleet_index % 3) == 0)
        {
            FT_ASSERT_EQ(LOCATION_PLANET, location.type);
            FT_ASSERT_EQ(PLANET_TERRA, location.from);
        }
        else if ((fleet_index % 3) == 1)
        {
            FT_ASSERT_EQ(LOCATION_TRAVEL, location.type);
            FT_ASSERT_EQ(PLANET_TERRA, location.from);
            FT_ASSERT_EQ(PLANET_MARS, location.to);
            double expected_travel = 60.0 + static_cast<double>(fleet_index) * 3.5;
            FT_ASSERT(ft_absolute(restored_fleet->get_travel_time() - expected_travel) < 0.000001);
        }
        else
        {
            FT_ASSERT_EQ(LOCATION_MISC, location.type);
            FT_ASSERT_EQ(MISC_OUTPOST_NEBULA_X, location.misc);
        }
        double expected_veterancy = static_cast<double>(fleet_index) * 0.75;
        if ((fleet_index % 2) == 1)
            expected_veterancy += 1.0;
        FT_ASSERT(ft_absolute(restored_fleet->get_escort_veterancy()
            - expected_veterancy) < 0.000001);
        FT_ASSERT_EQ(ships_per_fleet, restored_fleet->get_ship_count());
        for (int ship_index = 0; ship_index < ships_per_fleet; ++ship_index)
        {
            int ship_id = 5000 + fleet_index * 100 + ship_index;
            const ft_ship *restored_ship = restored_fleet->get_ship(ship_id);
            FT_ASSERT(restored_ship != ft_nullptr);
            int expected_type = ship_types[ship_index % ship_type_count];
            FT_ASSERT_EQ(expected_type, restored_ship->type);
            int expected_armor = (fleet_index + 1) * (ship_index + 1) * 3;
            FT_ASSERT_EQ(expected_armor, restored_ship->armor);
            int expected_hp = (fleet_index + 2) * (ship_index + 5);
            FT_ASSERT_EQ(expected_hp, restored_ship->hp);
            int expected_shield = (fleet_index + 3) * (ship_index + 2);
            FT_ASSERT_EQ(expected_shield, restored_ship->shield);
            FT_ASSERT_EQ(expected_hp + 50, restored_ship->max_hp);
            FT_ASSERT_EQ(expected_shield + 25, restored_ship->max_shield);
            double expected_speed = 10.0 + static_cast<double>(ship_index) * 2.5
                + static_cast<double>(fleet_index) * 0.25;
            FT_ASSERT(ft_absolute(restored_ship->max_speed - expected_speed) < 0.000001);
            double expected_acceleration = 1.0
                + static_cast<double>(ship_index) * 0.5
                + static_cast<double>(fleet_index) * 0.1;
            FT_ASSERT(ft_absolute(restored_ship->acceleration - expected_acceleration)
                < 0.000001);
            double expected_turn_speed = 30.0
                + static_cast<double>(ship_index) * 1.75
                + static_cast<double>(fleet_index) * 0.5;
            FT_ASSERT(ft_absolute(restored_ship->turn_speed - expected_turn_speed)
                < 0.000001);
            int expected_behavior = behavior_cycle[(ship_index + fleet_index) % behavior_count];
            FT_ASSERT_EQ(expected_behavior, restored_ship->combat_behavior);
            int expected_outnumbered = behavior_cycle[(ship_index + fleet_index + 1)
                % behavior_count];
            FT_ASSERT_EQ(expected_outnumbered, restored_ship->outnumbered_behavior);
            int expected_unescorted = behavior_cycle[(ship_index + fleet_index + 2)
                % behavior_count];
            FT_ASSERT_EQ(expected_unescorted, restored_ship->unescorted_behavior);
            int expected_low_hp = behavior_cycle[(ship_index + fleet_index + 3)
                % behavior_count];
            FT_ASSERT_EQ(expected_low_hp, restored_ship->low_hp_behavior);
            int expected_role = ship_roles[(ship_index + fleet_index) % ship_role_count];
            FT_ASSERT_EQ(expected_role, restored_ship->role);
        }
    }

    return 1;
}

int verify_save_system_sparse_entries()
{
    SaveSystem saves;

    json_document planet_doc;
    json_group *valid_planet = planet_doc.create_group("planet_valid");
    FT_ASSERT(valid_planet != ft_nullptr);
    planet_doc.append_group(valid_planet);
    json_item *planet_id_item = planet_doc.create_item("id", 1234);
    FT_ASSERT(planet_id_item != ft_nullptr);
    planet_doc.add_item(valid_planet, planet_id_item);
    json_item *planet_amount = planet_doc.create_item("resource_2000", 9001);
    FT_ASSERT(planet_amount != ft_nullptr);
    planet_doc.add_item(valid_planet, planet_amount);
    json_item *planet_rate = planet_doc.create_item("rate_2000", 2500000);
    FT_ASSERT(planet_rate != ft_nullptr);
    planet_doc.add_item(valid_planet, planet_rate);
    json_item *planet_carry = planet_doc.create_item("carryover_2000", -1250000);
    FT_ASSERT(planet_carry != ft_nullptr);
    planet_doc.add_item(valid_planet, planet_carry);
    json_item *planet_item = planet_doc.create_item("item_4000", 33);
    FT_ASSERT(planet_item != ft_nullptr);
    planet_doc.add_item(valid_planet, planet_item);

    json_group *missing_id = planet_doc.create_group("planet_missing_id");
    FT_ASSERT(missing_id != ft_nullptr);
    planet_doc.append_group(missing_id);
    json_item *missing_amount = planet_doc.create_item("resource_1", 15);
    FT_ASSERT(missing_amount != ft_nullptr);
    planet_doc.add_item(missing_id, missing_amount);

    json_group *invalid_planet = planet_doc.create_group("planet_invalid_values");
    FT_ASSERT(invalid_planet != ft_nullptr);
    planet_doc.append_group(invalid_planet);
    json_item *invalid_id = planet_doc.create_item("id", 5678);
    FT_ASSERT(invalid_id != ft_nullptr);
    planet_doc.add_item(invalid_planet, invalid_id);
    json_item *invalid_amount_item = planet_doc.create_item("resource_9000", -42);
    FT_ASSERT(invalid_amount_item != ft_nullptr);
    planet_doc.add_item(invalid_planet, invalid_amount_item);
    json_item *invalid_rate_item = planet_doc.create_item("rate_9000", "invalid");
    FT_ASSERT(invalid_rate_item != ft_nullptr);
    planet_doc.add_item(invalid_planet, invalid_rate_item);
    ft_string nan_sentinel = ft_to_string(FT_LONG_MIN);
    json_item *invalid_carry_item = planet_doc.create_item("carryover_9000",
        nan_sentinel.c_str());
    FT_ASSERT(invalid_carry_item != ft_nullptr);
    planet_doc.add_item(invalid_planet, invalid_carry_item);

    char *planet_raw = planet_doc.write_to_string();
    FT_ASSERT(planet_raw != ft_nullptr);
    ft_string planet_json(planet_raw);
    cma_free(planet_raw);

    ft_map<int, ft_sharedptr<ft_planet> > planets;
    FT_ASSERT(saves.deserialize_planets(planet_json.c_str(), planets));
    FT_ASSERT_EQ(2u, planets.size());

    Pair<int, ft_sharedptr<ft_planet> > *valid_entry = planets.find(1234);
    FT_ASSERT(valid_entry != ft_nullptr);
    ft_sharedptr<ft_planet> restored_valid = valid_entry->value;
    FT_ASSERT(restored_valid);
    FT_ASSERT_EQ(9001, restored_valid->get_resource(2000));
    FT_ASSERT(ft_absolute(restored_valid->get_rate(2000) - 2.5) < 0.000001);
    const ft_vector<Pair<int, double> > &valid_carry = restored_valid->get_carryover();
    bool found_carry = false;
    double carry_value = 0.0;
    for (size_t idx = 0; idx < valid_carry.size(); ++idx)
    {
        if (valid_carry[idx].key == 2000)
        {
            carry_value = valid_carry[idx].value;
            found_carry = true;
            break;
        }
    }
    FT_ASSERT(found_carry);
    FT_ASSERT(ft_absolute(carry_value + 1.25) < 0.000001);
    FT_ASSERT_EQ(33, restored_valid->get_resource(4000));

    Pair<int, ft_sharedptr<ft_planet> > *invalid_entry = planets.find(5678);
    FT_ASSERT(invalid_entry != ft_nullptr);
    ft_sharedptr<ft_planet> restored_invalid = invalid_entry->value;
    FT_ASSERT(restored_invalid);
    FT_ASSERT_EQ(-42, restored_invalid->get_resource(9000));
    FT_ASSERT(ft_absolute(restored_invalid->get_rate(9000)) < 0.000001);
    const ft_vector<Pair<int, double> > &invalid_carry = restored_invalid->get_carryover();
    bool found_invalid_carry = false;
    double invalid_carry_value = 0.0;
    for (size_t idx = 0; idx < invalid_carry.size(); ++idx)
    {
        if (invalid_carry[idx].key == 9000)
        {
            invalid_carry_value = invalid_carry[idx].value;
            found_invalid_carry = true;
            break;
        }
    }
    FT_ASSERT(found_invalid_carry);
    FT_ASSERT(ft_absolute(invalid_carry_value) < 0.000001);

    json_document fleet_doc;
    json_group *valid_fleet = fleet_doc.create_group("fleet_valid");
    FT_ASSERT(valid_fleet != ft_nullptr);
    fleet_doc.append_group(valid_fleet);
    json_item *fleet_id_item = fleet_doc.create_item("id", 2500);
    FT_ASSERT(fleet_id_item != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_id_item);
    json_item *fleet_type_item = fleet_doc.create_item("location_type", LOCATION_TRAVEL);
    FT_ASSERT(fleet_type_item != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_type_item);
    json_item *fleet_from_item = fleet_doc.create_item("location_from", PLANET_TERRA);
    FT_ASSERT(fleet_from_item != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_from_item);
    json_item *fleet_to_item = fleet_doc.create_item("location_to", PLANET_MARS);
    FT_ASSERT(fleet_to_item != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_to_item);
    json_item *fleet_travel_item = fleet_doc.create_item("travel_time", 12750000);
    FT_ASSERT(fleet_travel_item != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_travel_item);
    json_item *fleet_veterancy_item = fleet_doc.create_item("escort_veterancy", 18500000);
    FT_ASSERT(fleet_veterancy_item != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_veterancy_item);
    json_item *fleet_ship_count = fleet_doc.create_item("ship_count", 3);
    FT_ASSERT(fleet_ship_count != ft_nullptr);
    fleet_doc.add_item(valid_fleet, fleet_ship_count);

    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_id", 9100));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_type", SHIP_CAPITAL));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_armor", 450));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_hp", 620));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_shield", 500));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_max_hp", 700));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_max_shield", 560));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_max_speed", 24750000));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_acceleration", 4500000));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_turn_speed", 32500000));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_behavior", SHIP_BEHAVIOR_LAST_STAND));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_outnumbered", SHIP_BEHAVIOR_CHARGE));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_unescorted", SHIP_BEHAVIOR_WITHDRAW_SUPPORT));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_low_hp", SHIP_BEHAVIOR_RETREAT));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_0_role", SHIP_ROLE_SUPPORT));

    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_1_id", 9101));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_1_type", SHIP_SHIELD));

    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_2_id", 9102));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_2_type", SHIP_RADAR));
    fleet_doc.add_item(valid_fleet, fleet_doc.create_item("ship_2_max_speed", 16250000));

    json_group *missing_fleet = fleet_doc.create_group("fleet_missing_id");
    FT_ASSERT(missing_fleet != ft_nullptr);
    fleet_doc.append_group(missing_fleet);
    fleet_doc.add_item(missing_fleet, fleet_doc.create_item("ship_count", 1));
    fleet_doc.add_item(missing_fleet, fleet_doc.create_item("ship_0_id", 9999));

    json_group *sparse_fleet = fleet_doc.create_group("fleet_sparse");
    FT_ASSERT(sparse_fleet != ft_nullptr);
    fleet_doc.append_group(sparse_fleet);
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("id", 2600));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("location_type", LOCATION_MISC));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("location_misc", MISC_ASTEROID_HIDEOUT));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("escort_veterancy", -5000));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_count", 4));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_0_id", 9200));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_0_type", SHIP_TRANSPORT));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_0_hp", 180));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_0_behavior", SHIP_BEHAVIOR_SCREEN_SUPPORT));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_0_role", SHIP_ROLE_TRANSPORT));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_3_id", 9203));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_3_type", SHIP_INTERCEPTOR));
    fleet_doc.add_item(sparse_fleet, fleet_doc.create_item("ship_3_max_speed", 36500000));

    char *fleet_raw = fleet_doc.write_to_string();
    FT_ASSERT(fleet_raw != ft_nullptr);
    ft_string fleet_json(fleet_raw);
    cma_free(fleet_raw);

    ft_map<int, ft_sharedptr<ft_fleet> > fleets;
    FT_ASSERT(saves.deserialize_fleets(fleet_json.c_str(), fleets));
    FT_ASSERT_EQ(2u, fleets.size());

    Pair<int, ft_sharedptr<ft_fleet> > *valid_fleet_entry = fleets.find(2500);
    FT_ASSERT(valid_fleet_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_fleet_valid = valid_fleet_entry->value;
    FT_ASSERT(restored_fleet_valid);
    ft_location valid_location = restored_fleet_valid->get_location();
    FT_ASSERT_EQ(LOCATION_TRAVEL, valid_location.type);
    FT_ASSERT_EQ(PLANET_TERRA, valid_location.from);
    FT_ASSERT_EQ(PLANET_MARS, valid_location.to);
    FT_ASSERT(ft_absolute(restored_fleet_valid->get_travel_time() - 12.75) < 0.000001);
    FT_ASSERT(ft_absolute(restored_fleet_valid->get_escort_veterancy() - 18.5) < 0.000001);
    FT_ASSERT_EQ(3, restored_fleet_valid->get_ship_count());
    const ft_ship *valid_ship_zero = restored_fleet_valid->get_ship(9100);
    FT_ASSERT(valid_ship_zero != ft_nullptr);
    FT_ASSERT_EQ(SHIP_CAPITAL, valid_ship_zero->type);
    FT_ASSERT_EQ(450, valid_ship_zero->armor);
    FT_ASSERT_EQ(620, valid_ship_zero->hp);
    FT_ASSERT_EQ(500, valid_ship_zero->shield);
    FT_ASSERT_EQ(700, valid_ship_zero->max_hp);
    FT_ASSERT_EQ(560, valid_ship_zero->max_shield);
    FT_ASSERT(ft_absolute(valid_ship_zero->max_speed - 24.75) < 0.000001);
    FT_ASSERT(ft_absolute(valid_ship_zero->acceleration - 4.5) < 0.000001);
    FT_ASSERT(ft_absolute(valid_ship_zero->turn_speed - 32.5) < 0.000001);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_LAST_STAND, valid_ship_zero->combat_behavior);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_CHARGE, valid_ship_zero->outnumbered_behavior);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_WITHDRAW_SUPPORT, valid_ship_zero->unescorted_behavior);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_RETREAT, valid_ship_zero->low_hp_behavior);
    FT_ASSERT_EQ(SHIP_ROLE_SUPPORT, valid_ship_zero->role);
    const ft_ship *valid_ship_one = restored_fleet_valid->get_ship(9101);
    FT_ASSERT(valid_ship_one != ft_nullptr);
    FT_ASSERT_EQ(SHIP_SHIELD, valid_ship_one->type);
    FT_ASSERT_EQ(0, valid_ship_one->hp);
    const ft_ship *valid_ship_two = restored_fleet_valid->get_ship(9102);
    FT_ASSERT(valid_ship_two != ft_nullptr);
    FT_ASSERT_EQ(SHIP_RADAR, valid_ship_two->type);
    FT_ASSERT(ft_absolute(valid_ship_two->max_speed - 16.25) < 0.000001);

    Pair<int, ft_sharedptr<ft_fleet> > *sparse_fleet_entry = fleets.find(2600);
    FT_ASSERT(sparse_fleet_entry != ft_nullptr);
    ft_sharedptr<ft_fleet> restored_sparse = sparse_fleet_entry->value;
    FT_ASSERT(restored_sparse);
    ft_location sparse_location = restored_sparse->get_location();
    FT_ASSERT_EQ(LOCATION_MISC, sparse_location.type);
    FT_ASSERT_EQ(MISC_ASTEROID_HIDEOUT, sparse_location.misc);
    FT_ASSERT(ft_absolute(restored_sparse->get_escort_veterancy()) < 0.000001);
    FT_ASSERT_EQ(2, restored_sparse->get_ship_count());
    const ft_ship *sparse_ship_zero = restored_sparse->get_ship(9200);
    FT_ASSERT(sparse_ship_zero != ft_nullptr);
    FT_ASSERT_EQ(SHIP_TRANSPORT, sparse_ship_zero->type);
    FT_ASSERT_EQ(180, sparse_ship_zero->hp);
    FT_ASSERT_EQ(SHIP_BEHAVIOR_SCREEN_SUPPORT, sparse_ship_zero->combat_behavior);
    FT_ASSERT_EQ(SHIP_ROLE_TRANSPORT, sparse_ship_zero->role);
    const ft_ship *sparse_ship_three = restored_sparse->get_ship(9203);
    FT_ASSERT(sparse_ship_three != ft_nullptr);
    FT_ASSERT_EQ(SHIP_INTERCEPTOR, sparse_ship_three->type);
    FT_ASSERT(ft_absolute(sparse_ship_three->max_speed - 36.5) < 0.000001);

    return 1;
}

int verify_planet_inventory_save_round_trip()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_FUSION_REACTOR);
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART);
    game.set_ore(PLANET_TERRA, ORE_IRON, 135);
    game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 7);
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 11);

    FT_ASSERT(game.save_campaign_checkpoint(ft_string("inventory_roundtrip")));
    ft_string planet_json = game.get_campaign_planet_checkpoint();
    ft_string fleet_json = game.get_campaign_fleet_checkpoint();
    ft_string research_json = game.get_campaign_research_checkpoint();
    ft_string achievement_json = game.get_campaign_achievement_checkpoint();
    ft_string building_json = game.get_campaign_building_checkpoint();

    game.set_ore(PLANET_TERRA, ORE_IRON, 0);
    game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 0);
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 0);

    FT_ASSERT(game.load_campaign_from_save(planet_json, fleet_json, research_json,
        achievement_json, building_json));

    FT_ASSERT_EQ(135, game.get_ore(PLANET_TERRA, ORE_IRON));
    FT_ASSERT_EQ(7, game.get_ore(PLANET_TERRA, ITEM_FUSION_REACTOR));
    FT_ASSERT_EQ(11, game.get_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART));

    return 1;
}

int verify_building_save_round_trip()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    game.set_ore(PLANET_TERRA, ORE_IRON, 120);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 120);
    game.set_ore(PLANET_TERRA, ORE_COAL, 120);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 0);

    int generator_uid = game.place_building(PLANET_TERRA, BUILDING_POWER_GENERATOR, 2, 0);
    FT_ASSERT(generator_uid != 0);
    int smelter_uid = game.place_building(PLANET_TERRA, BUILDING_SMELTER, 0, 1);
    FT_ASSERT(smelter_uid != 0);

    game.tick(2.0);

    int logistic_usage_before = game.get_planet_logistic_usage(PLANET_TERRA);
    double energy_generation_before = game.get_planet_energy_generation(PLANET_TERRA);
    double energy_consumption_before = game.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(logistic_usage_before > 0);
    FT_ASSERT(energy_generation_before > 0.0);
    FT_ASSERT(energy_consumption_before > 0.0);

    FT_ASSERT(game.save_campaign_checkpoint(ft_string("building_roundtrip")));
    ft_string planet_json = game.get_campaign_planet_checkpoint();
    ft_string fleet_json = game.get_campaign_fleet_checkpoint();
    ft_string research_json = game.get_campaign_research_checkpoint();
    ft_string achievement_json = game.get_campaign_achievement_checkpoint();
    ft_string building_json = game.get_campaign_building_checkpoint();
    FT_ASSERT(building_json.size() > 0);

    FT_ASSERT(game.remove_building(PLANET_TERRA, generator_uid));
    FT_ASSERT(game.remove_building(PLANET_TERRA, smelter_uid));
    game.set_ore(PLANET_TERRA, ORE_IRON, 0);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 0);

    FT_ASSERT(game.load_campaign_from_save(planet_json, fleet_json, research_json,
        achievement_json, building_json));

    FT_ASSERT_EQ(logistic_usage_before, game.get_planet_logistic_usage(PLANET_TERRA));
    double restored_generation = game.get_planet_energy_generation(PLANET_TERRA);
    double restored_consumption = game.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(ft_absolute(restored_generation - energy_generation_before) < 0.000001);
    FT_ASSERT(ft_absolute(restored_consumption - energy_consumption_before) < 0.000001);

    int restored_generator = game.get_building_instance(PLANET_TERRA, 2, 0);
    int restored_smelter = game.get_building_instance(PLANET_TERRA, 0, 1);
    FT_ASSERT(restored_generator != 0);
    FT_ASSERT(restored_smelter != 0);
    FT_ASSERT_EQ(generator_uid, restored_generator);
    FT_ASSERT_EQ(smelter_uid, restored_smelter);

    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 0);
    int ore_before = game.get_ore(PLANET_TERRA, ORE_IRON);
    game.tick(3.1);
    int ore_after = game.get_ore(PLANET_TERRA, ORE_IRON);
    int bars_after = game.get_ore(PLANET_TERRA, ITEM_IRON_BAR);
    FT_ASSERT_EQ(ore_before - 3, ore_after);
    FT_ASSERT_EQ(2, bars_after);

    return 1;
}

int verify_research_save_round_trip()
{
    SaveSystem saves;
    ResearchManager research;
    research.set_duration_scale(1.5);
    FT_ASSERT(research.start(RESEARCH_UNLOCK_MARS));
    ft_vector<int> completed;
    research.tick(12.0, completed);
    FT_ASSERT_EQ(RESEARCH_STATUS_IN_PROGRESS, research.get_status(RESEARCH_UNLOCK_MARS));
    research.mark_completed(RESEARCH_UNLOCK_ZALTHOR);

    ft_string json = saves.serialize_research(research);
    FT_ASSERT(json.size() > 0);

    ResearchManager restored;
    FT_ASSERT(saves.deserialize_research(json.c_str(), restored));
    FT_ASSERT(ft_absolute(restored.get_duration_scale() - 1.5) < 0.000001);
    FT_ASSERT_EQ(RESEARCH_STATUS_IN_PROGRESS, restored.get_status(RESEARCH_UNLOCK_MARS));
    double original_remaining = research.get_remaining_time(RESEARCH_UNLOCK_MARS);
    double restored_remaining = restored.get_remaining_time(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(ft_absolute(restored_remaining - original_remaining) < 0.000001);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, restored.get_status(RESEARCH_UNLOCK_ZALTHOR));

    return 1;
}

int verify_achievement_save_round_trip()
{
    SaveSystem saves;
    AchievementManager achievements;
    achievements.record_event(ACHIEVEMENT_EVENT_PLANET_UNLOCKED, 1);
    achievements.record_event(ACHIEVEMENT_EVENT_CONVOY_STREAK_BEST, 3);
    achievements.record_event(ACHIEVEMENT_EVENT_CONVOY_STREAK_BEST, 6);
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, achievements.get_status(ACHIEVEMENT_CONVOY_STREAK_GUARDIAN));

    ft_string json = saves.serialize_achievements(achievements);
    FT_ASSERT(json.size() > 0);

    AchievementManager restored;
    FT_ASSERT(saves.deserialize_achievements(json.c_str(), restored));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, restored.get_status(ACHIEVEMENT_SECOND_HOME));
    FT_ASSERT_EQ(ACHIEVEMENT_STATUS_COMPLETED, restored.get_status(ACHIEVEMENT_CONVOY_STREAK_GUARDIAN));
    FT_ASSERT_EQ(6, restored.get_progress(ACHIEVEMENT_CONVOY_STREAK_GUARDIAN));
    FT_ASSERT_EQ(1, restored.get_progress(ACHIEVEMENT_SECOND_HOME));

    return 1;
}

int verify_campaign_checkpoint_flow()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    FT_ASSERT(game.has_campaign_checkpoint());
    FT_ASSERT(game.get_campaign_planet_checkpoint().size() > 0);

    game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    game.tick(0.0);
    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_INITIAL_SKIRMISHES));
    FT_ASSERT(game.has_campaign_checkpoint());
    FT_ASSERT(ft_strncmp(game.get_campaign_checkpoint_tag().c_str(), "quest_completed_", 16) == 0);

    game.create_fleet(72);
    int ship_id = game.create_ship(72, SHIP_SHIELD);
    FT_ASSERT(ship_id != 0);
    game.set_ship_hp(72, ship_id, 144);
    game.set_ship_shield(72, ship_id, 222);
    game.set_ore(PLANET_TERRA, ORE_IRON, 85);
    FT_ASSERT(game.save_campaign_checkpoint(ft_string("manual_checkpoint")));
    FT_ASSERT(game.has_campaign_checkpoint());
    FT_ASSERT(ft_strcmp(game.get_campaign_checkpoint_tag().c_str(), "manual_checkpoint") == 0);
    ft_string checkpoint_planets = game.get_campaign_planet_checkpoint();
    ft_string checkpoint_fleets = game.get_campaign_fleet_checkpoint();
    ft_string checkpoint_research = game.get_campaign_research_checkpoint();
    ft_string checkpoint_achievements = game.get_campaign_achievement_checkpoint();
    ft_string checkpoint_buildings = game.get_campaign_building_checkpoint();
    FT_ASSERT(checkpoint_research.size() > 0);
    FT_ASSERT(checkpoint_achievements.size() > 0);
    int baseline_research_achievement_status = game.get_achievement_status(ACHIEVEMENT_RESEARCH_PIONEER);
    int baseline_research_achievement_progress = game.get_achievement_progress(ACHIEVEMENT_RESEARCH_PIONEER);

    game.set_ore(PLANET_TERRA, ORE_IRON, 0);
    game.remove_fleet(72, -1, -1);
    FT_ASSERT(game.reload_campaign_checkpoint());
    FT_ASSERT_EQ(85, game.get_ore(PLANET_TERRA, ORE_IRON));
    FT_ASSERT(game.get_fleet_location(72).type != 0);
    FT_ASSERT_EQ(144, game.get_ship_hp(72, ship_id));
    FT_ASSERT_EQ(222, game.get_ship_shield(72, ship_id));

    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_UNLOCK_MARS));
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    FT_ASSERT(game.save_campaign_checkpoint(ft_string("research_in_progress")));
    ft_string progress_planets = game.get_campaign_planet_checkpoint();
    ft_string progress_fleets = game.get_campaign_fleet_checkpoint();
    ft_string progress_research = game.get_campaign_research_checkpoint();
    ft_string progress_achievements = game.get_campaign_achievement_checkpoint();
    ft_string progress_buildings = game.get_campaign_building_checkpoint();
    double remaining_before_tick = game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(remaining_before_tick > 0.0);

    game.tick(120.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_MARS));
    FT_ASSERT(game.get_achievement_progress(ACHIEVEMENT_RESEARCH_PIONEER)
        >= baseline_research_achievement_progress);

    FT_ASSERT(game.load_campaign_from_save(progress_planets, progress_fleets,
        progress_research, progress_achievements, progress_buildings));
    FT_ASSERT_EQ(RESEARCH_STATUS_IN_PROGRESS, game.get_research_status(RESEARCH_UNLOCK_MARS));
    double remaining_after_restore = game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(remaining_after_restore > 0.0);
    FT_ASSERT(remaining_after_restore <= remaining_before_tick);
    FT_ASSERT_EQ(baseline_research_achievement_status,
        game.get_achievement_status(ACHIEVEMENT_RESEARCH_PIONEER));
    FT_ASSERT_EQ(baseline_research_achievement_progress,
        game.get_achievement_progress(ACHIEVEMENT_RESEARCH_PIONEER));

    FT_ASSERT(game.load_campaign_from_save(checkpoint_planets, checkpoint_fleets,
        checkpoint_research, checkpoint_achievements, checkpoint_buildings));

    game.set_force_checkpoint_failure(true);
    ft_string forced_tag("forced_checkpoint_failure");
    FT_ASSERT(!game.save_campaign_checkpoint(forced_tag));
    game.set_force_checkpoint_failure(false);
    const ft_vector<ft_string> &failures = game.get_failed_checkpoint_tags();
    size_t failure_count = failures.size();
    FT_ASSERT(failure_count > 0);
    const ft_string &last_failure = failures[failure_count - 1];
    FT_ASSERT(ft_strcmp(last_failure.c_str(), forced_tag.c_str()) == 0);
    FT_ASSERT(ft_strcmp(game.get_campaign_checkpoint_tag().c_str(), "research_in_progress") == 0);
    const ft_vector<ft_string> &lore_log = game.get_lore_log();
    size_t lore_count = lore_log.size();
    FT_ASSERT(lore_count > 0);
    const ft_string &failure_entry = lore_log[lore_count - 1];
    ft_string expected_message("Checkpoint save failed: ");
    expected_message.append(forced_tag);
    FT_ASSERT(ft_strcmp(failure_entry.c_str(), expected_message.c_str()) == 0);
    FT_ASSERT(game.has_campaign_checkpoint());
    FT_ASSERT(game.save_campaign_checkpoint(ft_string("recovered_checkpoint")));
    FT_ASSERT(ft_strcmp(game.get_campaign_checkpoint_tag().c_str(), "recovered_checkpoint") == 0);

    return 1;
}

int verify_campaign_rejects_invalid_save()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    game.set_ore(PLANET_TERRA, ORE_IRON, 42);
    game.create_fleet(54);
    int ship_id = game.create_ship(54, SHIP_SHIELD);
    FT_ASSERT(ship_id != 0);
    game.set_ship_hp(54, ship_id, 133);
    ft_location baseline_location = game.get_fleet_location(54);
    int baseline_achievement_status = game.get_achievement_status(ACHIEVEMENT_SECOND_HOME);
    int baseline_achievement_progress = game.get_achievement_progress(ACHIEVEMENT_SECOND_HOME);
    FT_ASSERT(game.start_research(RESEARCH_UNLOCK_MARS));
    double remaining_before = game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(remaining_before > 0.0);
    int baseline_research_status = game.get_research_status(RESEARCH_UNLOCK_MARS);

    ft_string invalid_payload("not json");
    FT_ASSERT(!game.load_campaign_from_save(invalid_payload, invalid_payload,
        invalid_payload, invalid_payload, invalid_payload));

    FT_ASSERT_EQ(42, game.get_ore(PLANET_TERRA, ORE_IRON));
    FT_ASSERT_EQ(133, game.get_ship_hp(54, ship_id));
    ft_location restored_location = game.get_fleet_location(54);
    FT_ASSERT_EQ(baseline_location.type, restored_location.type);
    FT_ASSERT_EQ(baseline_location.from, restored_location.from);
    FT_ASSERT_EQ(baseline_location.to, restored_location.to);
    FT_ASSERT_EQ(baseline_research_status, game.get_research_status(RESEARCH_UNLOCK_MARS));
    double remaining_after = game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(ft_absolute(remaining_after - remaining_before) < 0.000001);
    FT_ASSERT_EQ(baseline_achievement_status,
        game.get_achievement_status(ACHIEVEMENT_SECOND_HOME));
    FT_ASSERT_EQ(baseline_achievement_progress,
        game.get_achievement_progress(ACHIEVEMENT_SECOND_HOME));

    return 1;
}
