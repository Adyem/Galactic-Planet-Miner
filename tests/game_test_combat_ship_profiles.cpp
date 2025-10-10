int verify_ship_range_defaults()
{
    ft_fleet fleet(101);

    int shield_id = fleet.create_ship(SHIP_SHIELD);
    const ft_ship *shield = fleet.get_ship(shield_id);
    FT_ASSERT(shield != ft_nullptr);
    FT_ASSERT(math_fabs(shield->optimal_range - 205.0) < 1e-6);
    FT_ASSERT(math_fabs(shield->max_range - 265.0) < 1e-6);
    FT_ASSERT(math_fabs(shield->base_damage - 20.0) < 1e-6);

    int radar_id = fleet.create_ship(SHIP_RADAR);
    const ft_ship *radar = fleet.get_ship(radar_id);
    FT_ASSERT(radar != ft_nullptr);
    FT_ASSERT(math_fabs(radar->optimal_range - 235.0) < 1e-6);
    FT_ASSERT(math_fabs(radar->max_range - 320.0) < 1e-6);
    FT_ASSERT(math_fabs(radar->base_damage - 25.0) < 1e-6);

    int corvette_id = fleet.create_ship(SHIP_CORVETTE);
    const ft_ship *corvette = fleet.get_ship(corvette_id);
    FT_ASSERT(corvette != ft_nullptr);
    FT_ASSERT(math_fabs(corvette->optimal_range - 190.0) < 1e-6);
    FT_ASSERT(math_fabs(corvette->max_range - 235.0) < 1e-6);
    FT_ASSERT(math_fabs(corvette->base_damage - 30.0) < 1e-6);

    return 1;
}

static void prepare_tracker(CombatManager::ft_ship_tracker &tracker,
    double base_damage, double optimal, double maximum, double hp_ratio,
    double spatial_z)
{
    tracker.base_damage = base_damage;
    tracker.optimal_range = optimal;
    tracker.max_range = maximum;
    tracker.hp_ratio = hp_ratio;
    tracker.spatial.z = spatial_z;
}

int verify_range_aware_combat_power()
{
    CombatManager manager;
    CombatManager::ft_combat_encounter encounter;
    encounter.raider_frontline = 150.0;
    encounter.defender_line = -60.0;

    CombatManager::ft_ship_tracker tracker;

    prepare_tracker(tracker, 12.0, 100.0, 140.0, 1.0,
        encounter.raider_frontline - 80.0);
    encounter.defender_tracks.clear();
    encounter.defender_tracks.insert(1, tracker);
    double in_range = manager.calculate_player_power(encounter);
    FT_ASSERT(math_fabs(in_range - 12.0) < 1e-6);

    prepare_tracker(tracker, 12.0, 100.0, 140.0, 1.0,
        encounter.raider_frontline - 120.0);
    encounter.defender_tracks.clear();
    encounter.defender_tracks.insert(2, tracker);
    double falloff = manager.calculate_player_power(encounter);
    FT_ASSERT(math_fabs(falloff - 6.0) < 1e-6);

    prepare_tracker(tracker, 12.0, 100.0, 140.0, 1.0,
        encounter.raider_frontline - 150.0);
    encounter.defender_tracks.clear();
    encounter.defender_tracks.insert(3, tracker);
    double out_of_range = manager.calculate_player_power(encounter);
    FT_ASSERT(math_fabs(out_of_range - 0.0) < 1e-6);

    prepare_tracker(tracker, 20.0, 80.0, 120.0, 0.05,
        encounter.raider_frontline - 60.0);
    encounter.defender_tracks.clear();
    encounter.defender_tracks.insert(4, tracker);
    double low_hp = manager.calculate_player_power(encounter);
    FT_ASSERT(low_hp > 1.9);
    FT_ASSERT(low_hp < 2.1);

    CombatManager::ft_ship_tracker raider_tracker;
    prepare_tracker(raider_tracker, 9.0, 70.0, 110.0, 1.0,
        encounter.defender_line + 60.0);
    encounter.raider_tracks.clear();
    encounter.raider_tracks.insert(7, raider_tracker);
    double raider_in_range = manager.calculate_raider_power(encounter);
    FT_ASSERT(math_fabs(raider_in_range - 9.0) < 1e-6);

    prepare_tracker(raider_tracker, 9.0, 70.0, 110.0, 1.0,
        encounter.defender_line + 90.0);
    encounter.raider_tracks.clear();
    encounter.raider_tracks.insert(8, raider_tracker);
    double raider_falloff = manager.calculate_raider_power(encounter);
    FT_ASSERT(math_fabs(raider_falloff - 4.5) < 1e-6);

    prepare_tracker(raider_tracker, 9.0, 70.0, 110.0, 1.0,
        encounter.defender_line + 130.0);
    encounter.raider_tracks.clear();
    encounter.raider_tracks.insert(9, raider_tracker);
    double raider_out = manager.calculate_raider_power(encounter);
    FT_ASSERT(math_fabs(raider_out - 0.0) < 1e-6);

    prepare_tracker(raider_tracker, 16.0, 90.0, 130.0, 0.02,
        encounter.defender_line + 70.0);
    encounter.raider_tracks.clear();
    encounter.raider_tracks.insert(10, raider_tracker);
    double raider_low_hp = manager.calculate_raider_power(encounter);
    FT_ASSERT(raider_low_hp > 1.5);
    FT_ASSERT(raider_low_hp < 1.7);

    return 1;
}

struct expected_ship_profile
{
    int    type;
    int    hp;
    int    shield;
    double damage;
    int    role;
};

static bool compare_ship_profile(const ft_ship *ship,
    const expected_ship_profile &expected)
{
    if (ship == ft_nullptr)
        return false;
    if (ship->hp != expected.hp)
        return false;
    if (ship->max_hp != expected.hp)
        return false;
    if (ship->shield != expected.shield)
        return false;
    if (ship->max_shield != expected.shield)
        return false;
    if (math_fabs(ship->base_damage - expected.damage) > 1e-6)
        return false;
    if (ship->role != expected.role)
        return false;
    return true;
}

int verify_design_doc_ship_roster()
{
    const expected_ship_profile design_doc_ships[] = {
        {SHIP_SHIELD, 120, 150, 20.0, SHIP_ROLE_SUPPORT},
        {SHIP_RADAR, 110, 80, 25.0, SHIP_ROLE_SUPPORT},
        {SHIP_SALVAGE, 110, 70, 5.0, SHIP_ROLE_TRANSPORT},
        {SHIP_TRANSPORT, 100, 50, 10.0, SHIP_ROLE_TRANSPORT},
        {SHIP_CORVETTE, 100, 75, 30.0, SHIP_ROLE_LINE},
        {SHIP_INTERCEPTOR, 90, 60, 40.0, SHIP_ROLE_LINE},
        {SHIP_REPAIR_DRONE, 80, 40, 5.0, SHIP_ROLE_SUPPORT},
        {SHIP_SUNFLARE_SLOOP, 80, 60, 10.0, SHIP_ROLE_SUPPORT},
        {SHIP_CAPITAL_JUGGERNAUT, 550, 100, 80.0, SHIP_ROLE_TRANSPORT},
        {SHIP_CAPITAL_NOVA, 530, 120, 35.0, SHIP_ROLE_SUPPORT},
        {SHIP_CAPITAL_OBSIDIAN, 600, 80, 85.0, SHIP_ROLE_LINE},
        {SHIP_CAPITAL_PREEMPTOR, 520, 110, 250.0, SHIP_ROLE_LINE},
        {SHIP_CAPITAL_PROTECTOR, 540, 200, 82.0, SHIP_ROLE_SUPPORT},
        {SHIP_CAPITAL_ECLIPSE, 1000, 0, 70.0, SHIP_ROLE_SUPPORT},
        {SHIP_FRIGATE_JUGGERNAUT, 150, 100, 50.0, SHIP_ROLE_TRANSPORT},
        {SHIP_FRIGATE_CARRIER, 140, 90, 45.0, SHIP_ROLE_SUPPORT},
        {SHIP_FRIGATE_SOVEREIGN, 160, 110, 55.0, SHIP_ROLE_LINE},
        {SHIP_FRIGATE_PREEMPTOR, 130, 95, 100.0, SHIP_ROLE_LINE},
        {SHIP_FRIGATE_PROTECTOR, 145, 100, 48.0, SHIP_ROLE_SUPPORT},
        {SHIP_FRIGATE_ECLIPSE, 325, 0, 40.0, SHIP_ROLE_SUPPORT}
    };

    ft_fleet fleet(777);
    const size_t count = sizeof(design_doc_ships) / sizeof(design_doc_ships[0]);
    for (size_t i = 0; i < count; ++i)
    {
        int uid = fleet.create_ship(design_doc_ships[i].type);
        FT_ASSERT(uid != 0);
        const ft_ship *ship = fleet.get_ship(uid);
        FT_ASSERT(compare_ship_profile(ship, design_doc_ships[i]));
    }

    FT_ASSERT(fleet.get_ship_count() == static_cast<int>(count));
    return 1;
}
