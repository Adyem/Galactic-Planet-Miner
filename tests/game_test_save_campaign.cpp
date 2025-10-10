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
    ft_string checkpoint_progress = game.get_campaign_progress_checkpoint();
    FT_ASSERT(checkpoint_research.size() > 0);
    FT_ASSERT(checkpoint_achievements.size() > 0);
    FT_ASSERT(checkpoint_progress.size() > 0);
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
    ft_string progress_progress = game.get_campaign_progress_checkpoint();
    double remaining_before_tick = game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(remaining_before_tick > 0.0);

    game.tick(120.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_UNLOCK_MARS));
    FT_ASSERT(game.get_achievement_progress(ACHIEVEMENT_RESEARCH_PIONEER)
        >= baseline_research_achievement_progress);

    FT_ASSERT(game.load_campaign_from_save(progress_planets, progress_fleets,
        progress_research, progress_achievements, progress_buildings,
        progress_progress));
    FT_ASSERT_EQ(RESEARCH_STATUS_IN_PROGRESS, game.get_research_status(RESEARCH_UNLOCK_MARS));
    double remaining_after_restore = game.get_research_time_remaining(RESEARCH_UNLOCK_MARS);
    FT_ASSERT(remaining_after_restore > 0.0);
    FT_ASSERT(remaining_after_restore <= remaining_before_tick);
    FT_ASSERT_EQ(baseline_research_achievement_status,
        game.get_achievement_status(ACHIEVEMENT_RESEARCH_PIONEER));
    FT_ASSERT_EQ(baseline_research_achievement_progress,
        game.get_achievement_progress(ACHIEVEMENT_RESEARCH_PIONEER));

    FT_ASSERT(game.load_campaign_from_save(checkpoint_planets, checkpoint_fleets,
        checkpoint_research, checkpoint_achievements, checkpoint_buildings,
        checkpoint_progress));

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
        invalid_payload, invalid_payload, invalid_payload, invalid_payload));

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

int verify_buildings_unchanged_on_failed_load()
{
    Game game(ft_string("127.0.0.1:8080"), ft_string("/"));
    game.set_ore(PLANET_TERRA, ORE_IRON, 120);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 120);

    int smelter_uid = game.place_building(PLANET_TERRA, BUILDING_SMELTER, 0, 1);
    FT_ASSERT(smelter_uid != 0);
    game.tick(0.0);

    FT_ASSERT(game.save_campaign_checkpoint(ft_string("buildings_snapshot")));
    ft_string checkpoint_research = game.get_campaign_research_checkpoint();
    ft_string checkpoint_achievements = game.get_campaign_achievement_checkpoint();
    ft_string checkpoint_buildings = game.get_campaign_building_checkpoint();
    ft_string checkpoint_progress = game.get_campaign_progress_checkpoint();
    FT_ASSERT(checkpoint_buildings.size() > 0);

    int logistic_with_building = game.get_planet_logistic_usage(PLANET_TERRA);
    FT_ASSERT(logistic_with_building > 0);

    FT_ASSERT(game.remove_building(PLANET_TERRA, smelter_uid));
    int logistic_after_removal = game.get_planet_logistic_usage(PLANET_TERRA);
    FT_ASSERT(logistic_after_removal < logistic_with_building);

    ft_string invalid_payload("not json");
    FT_ASSERT(!game.load_campaign_from_save(invalid_payload, invalid_payload,
        checkpoint_research, checkpoint_achievements, checkpoint_buildings,
        checkpoint_progress));

    FT_ASSERT_EQ(logistic_after_removal, game.get_planet_logistic_usage(PLANET_TERRA));
    FT_ASSERT_EQ(0, game.get_building_count(PLANET_TERRA, BUILDING_SMELTER));

    return 1;
}

