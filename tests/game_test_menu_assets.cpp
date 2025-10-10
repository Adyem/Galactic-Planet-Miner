#include "game_test_menu_shared.hpp"

int verify_main_menu_commander_portrait_preload()
{
    main_menu_testing::reset_commander_portrait_cache();

    FT_ASSERT(ensure_directory_exists("assets"));
    FT_ASSERT(ensure_directory_exists("assets/portraits"));

    ft_string default_path("assets/portraits/default.png");
    ft_ofstream default_stream;
    FT_ASSERT_EQ(0, default_stream.open(default_path.c_str()));
    FT_ASSERT(default_stream.write("default portrait data") >= 0);
    default_stream.close();

    long timestamp = ft_time_ms();

    ft_string missing_commander("PortraitMissing_");
    missing_commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));
    PlayerProfilePreferences missing_preferences;
    missing_preferences.commander_name = missing_commander;
    FT_ASSERT(player_profile_save(missing_preferences));

    FT_ASSERT(main_menu_preload_commander_portrait(missing_commander));
    FT_ASSERT(main_menu_testing::commander_portrait_attempted(missing_commander));
    FT_ASSERT(main_menu_testing::commander_portrait_loaded(missing_commander));
    ft_string missing_cached_path = main_menu_testing::resolve_cached_portrait_path(missing_commander);
    FT_ASSERT_EQ(default_path, missing_cached_path);
    FT_ASSERT(main_menu_testing::commander_portrait_cached_size(missing_commander) > 0U);

    ft_string specific_commander("PortraitSpecific_");
    specific_commander.append(ft_to_string(static_cast<int>((timestamp + 1) % 1000000L)));
    PlayerProfilePreferences specific_preferences;
    specific_preferences.commander_name = specific_commander;
    FT_ASSERT(player_profile_save(specific_preferences));

    ft_string portrait_filename = main_menu_testing::resolve_commander_portrait_filename(specific_commander);
    FT_ASSERT(!portrait_filename.empty());
    ft_string specific_path("assets/portraits/");
    specific_path.append(portrait_filename);
    specific_path.append(".png");

    ft_ofstream specific_stream;
    FT_ASSERT_EQ(0, specific_stream.open(specific_path.c_str()));
    FT_ASSERT(specific_stream.write("specific portrait data") >= 0);
    specific_stream.close();

    FT_ASSERT(main_menu_preload_commander_portrait(specific_commander));
    FT_ASSERT(main_menu_testing::commander_portrait_attempted(specific_commander));
    FT_ASSERT(main_menu_testing::commander_portrait_loaded(specific_commander));
    ft_string specific_cached_path = main_menu_testing::resolve_cached_portrait_path(specific_commander);
    FT_ASSERT_EQ(specific_path, specific_cached_path);
    FT_ASSERT(main_menu_testing::commander_portrait_cached_size(specific_commander) > 0U);

    FT_ASSERT(player_profile_delete(missing_commander));
    FT_ASSERT(player_profile_delete(specific_commander));
    FT_ASSERT_EQ(0, file_delete(default_path.c_str()));
    FT_ASSERT_EQ(0, file_delete(specific_path.c_str()));

    main_menu_testing::reset_commander_portrait_cache();

    return 1;
}

int verify_main_menu_campaign_launch_guard()
{
    FT_ASSERT(!main_menu_can_launch_campaign(ft_string()));

    ft_string invalid_path("data/saves/does-not-exist.json");
    FT_ASSERT(!main_menu_can_launch_campaign(invalid_path));

    long timestamp = ft_time_ms();
    ft_string commander("LaunchGuard_");
    commander.append(ft_to_string(static_cast<int>(timestamp % 1000000L)));

    PlayerProfilePreferences preferences;
    preferences.commander_name = commander;
    FT_ASSERT(player_profile_save(preferences));

    ft_string error;
    FT_ASSERT(new_game_flow_testing::create_save_file(commander, ft_string("launchslot"), error));
    FT_ASSERT(error.empty());

    ft_string save_path = load_game_flow_testing::resolve_save_file_path(commander, ft_string("launchslot"));
    FT_ASSERT(!save_path.empty());
    FT_ASSERT(main_menu_can_launch_campaign(save_path));

    FT_ASSERT(player_profile_delete(commander));

    return 1;
}
