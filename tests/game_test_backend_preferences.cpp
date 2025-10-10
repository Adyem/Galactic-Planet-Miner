int verify_player_preference_application()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    PlayerProfilePreferences preferences;
    preferences.ui_scale_percent = 135U;
    preferences.combat_speed_percent = 180U;
    preferences.lore_panel_anchor = PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT;
    game.apply_preferences(preferences);

    double scale_delta = math_fabs(game.get_ui_scale() - 1.35);
    FT_ASSERT(scale_delta < 0.0001);
    FT_ASSERT_EQ(PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT, game.get_lore_panel_anchor());

    double speed_delta = math_fabs(game.get_combat_speed_multiplier() - 1.8);
    FT_ASSERT(speed_delta < 0.0001);

    Game::ft_quest_log_snapshot snapshot;
    game.get_quest_log_snapshot(snapshot);
    double snapshot_scale_delta = math_fabs(snapshot.ui_scale - game.get_ui_scale());
    FT_ASSERT(snapshot_scale_delta < 0.0001);
    FT_ASSERT_EQ(game.get_lore_panel_anchor(), snapshot.lore_panel_anchor);

    game.set_ui_scale(0.2);
    FT_ASSERT(game.get_ui_scale() >= 0.5);
    game.set_combat_speed_multiplier(5.0);
    FT_ASSERT(game.get_combat_speed_multiplier() <= 2.0);
    game.set_lore_panel_anchor(99);
    FT_ASSERT_EQ(PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT, game.get_lore_panel_anchor());

    preferences.ui_scale_percent = 300U;
    preferences.combat_speed_percent = 10U;
    preferences.lore_panel_anchor = 0U;
    game.apply_preferences(preferences);
    FT_ASSERT(game.get_ui_scale() <= static_cast<double>(PLAYER_PROFILE_UI_SCALE_MAX_PERCENT) / 100.0 + 0.0001);
    FT_ASSERT(game.get_combat_speed_multiplier() >= static_cast<double>(PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT) / 100.0 - 0.0001);
    FT_ASSERT_EQ(PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT, game.get_lore_panel_anchor());

    return 1;
}

int verify_quick_quest_completion_bonus()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));

    double initial_scale = game.get_effective_quest_time_scale();
    FT_ASSERT(initial_scale > 0.99 && initial_scale < 1.01);

    game.set_ore(PLANET_TERRA, ORE_IRON, 40);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 30);
    game.set_ore(PLANET_TERRA, ORE_COAL, 12);
    game.tick(0.0);

    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_INITIAL_SKIRMISHES));
    FT_ASSERT_EQ(QUEST_DEFENSE_OF_TERRA, game.get_active_quest());

    int parts_before = game.get_ore(PLANET_TERRA, ITEM_ENGINE_PART);
    size_t lore_before = game.get_lore_log().size();

    game.create_fleet(1);
    int flagship = game.create_ship(1, SHIP_CAPITAL);
    FT_ASSERT(flagship != 0);
    game.set_ship_hp(1, flagship, 140);
    game.create_fleet(2);
    int shield = game.create_ship(2, SHIP_SHIELD);
    FT_ASSERT(shield != 0);
    game.set_ship_hp(2, shield, 90);

    game.tick(0.0);

    FT_ASSERT_EQ(QUEST_STATUS_COMPLETED, game.get_quest_status(QUEST_DEFENSE_OF_TERRA));

    int parts_after = game.get_ore(PLANET_TERRA, ITEM_ENGINE_PART);
    FT_ASSERT(parts_after >= parts_before + 1);

    const ft_vector<ft_string> &lore_after = game.get_lore_log();
    FT_ASSERT(lore_after.size() >= lore_before + 2);
    const ft_string &quick_entry = lore_after[lore_after.size() - 1];
    FT_ASSERT(ft_strstr(quick_entry.c_str(), "praises the swift resolution") != ft_nullptr);

    double scaled = game.get_effective_quest_time_scale();
    FT_ASSERT(scaled < initial_scale);
    FT_ASSERT(scaled > 0.94);

    return 1;
}

