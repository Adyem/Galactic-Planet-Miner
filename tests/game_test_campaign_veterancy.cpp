        veterancy_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 200);
        veterancy_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 0);
        FT_ASSERT(veterancy_game.assign_convoy_escort(PLANET_TERRA, PLANET_MARS, escort_fleet_id));
        int moved = veterancy_game.transfer_ore(PLANET_TERRA, PLANET_MARS, ITEM_IRON_BAR, 40);
        FT_ASSERT(moved >= 40);
        double elapsed = 0.0;
        while (veterancy_game.get_active_convoy_count() > 0 && elapsed < 400.0)
        {
            veterancy_game.tick(0.5);
            elapsed += 0.5;
        }
        FT_ASSERT(elapsed < 400.0);
        travel_times[run] = elapsed;
        bonuses[run] = veterancy_game.get_fleet_escort_veterancy_bonus(escort_fleet_id);
    }

    FT_ASSERT_EQ(0, bonuses[0]);
    FT_ASSERT_EQ(0, bonuses[1]);
    FT_ASSERT(bonuses[2] >= 1);
    double veterancy_xp = veterancy_game.get_fleet_escort_veterancy(escort_fleet_id);
    FT_ASSERT(veterancy_xp >= 60.0);
    FT_ASSERT(travel_times[2] + 0.99 < travel_times[0]);
    FT_ASSERT(travel_times[2] <= travel_times[1] + 0.5);
    FT_ASSERT_EQ(0, veterancy_game.get_convoy_raid_losses());

    return 1;
}
