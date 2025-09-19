#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/map.hpp"
#include "../libft/Template/pair.hpp"
#include "../libft/JSon/document.hpp"
#include "../libft/JSon/json.hpp"
#include "../libft/CMA/CMA.hpp"
#include "save_system.hpp"
#include "game_test_scenarios.hpp"

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
    game.save_campaign_checkpoint(ft_string("manual_checkpoint"));
    FT_ASSERT(game.has_campaign_checkpoint());
    FT_ASSERT(ft_strcmp(game.get_campaign_checkpoint_tag().c_str(), "manual_checkpoint") == 0);
    ft_string checkpoint_planets = game.get_campaign_planet_checkpoint();
    ft_string checkpoint_fleets = game.get_campaign_fleet_checkpoint();
    ft_string checkpoint_research = game.get_campaign_research_checkpoint();
    ft_string checkpoint_achievements = game.get_campaign_achievement_checkpoint();
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
    game.save_campaign_checkpoint(ft_string("research_in_progress"));
    ft_string progress_planets = game.get_campaign_planet_checkpoint();
    ft_string progress_fleets = game.get_campaign_fleet_checkpoint();
    ft_string progress_research = game.get_campaign_research_checkpoint();
    ft_string progress_achievements = game.get_campaign_achievement_checkpoint();
    double remaining_before_tick = game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(remaining_before_tick > 0.0);

    game.tick(120.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_MARS));
    FT_ASSERT(game.get_achievement_progress(ACHIEVEMENT_RESEARCH_PIONEER)
        >= baseline_research_achievement_progress);

    FT_ASSERT(game.load_campaign_from_save(progress_planets, progress_fleets,
        progress_research, progress_achievements));
    FT_ASSERT_EQ(RESEARCH_STATUS_IN_PROGRESS, game.get_research_status(RESEARCH_UNLOCK_MARS));
    double remaining_after_restore = game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(remaining_after_restore > 0.0);
    FT_ASSERT(remaining_after_restore <= remaining_before_tick);
    FT_ASSERT_EQ(baseline_research_achievement_status,
        game.get_achievement_status(ACHIEVEMENT_RESEARCH_PIONEER));
    FT_ASSERT_EQ(baseline_research_achievement_progress,
        game.get_achievement_progress(ACHIEVEMENT_RESEARCH_PIONEER));

    FT_ASSERT(game.load_campaign_from_save(checkpoint_planets, checkpoint_fleets,
        checkpoint_research, checkpoint_achievements));

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
        invalid_payload, invalid_payload));

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
