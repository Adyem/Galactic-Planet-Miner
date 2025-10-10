    game.set_ore(PLANET_VULCAN, ORE_COPPER, 150);
    game.set_ore(PLANET_VULCAN, ORE_COAL, 150);
    game.set_ore(PLANET_VULCAN, ITEM_IRON_BAR, 0);
    game.set_ore(PLANET_VULCAN, ITEM_COPPER_BAR, 0);
    game.set_ore(PLANET_VULCAN, ITEM_ENGINE_PART, 0);

    FT_ASSERT(game.place_building(PLANET_VULCAN, BUILDING_POWER_GENERATOR, 2, 0) != 0);
    FT_ASSERT(game.place_building(PLANET_VULCAN, BUILDING_CONVEYOR, 1, 0) != 0);
    FT_ASSERT(game.place_building(PLANET_VULCAN, BUILDING_TRANSFER_NODE, 1, 1) != 0);
    FT_ASSERT(game.place_building(PLANET_VULCAN, BUILDING_CRAFTING_BAY, 0, 2) != 0);
    FT_ASSERT_EQ(4, game.get_planet_logistic_capacity(PLANET_VULCAN));

    int moved_iron = game.transfer_ore(PLANET_TERRA, PLANET_VULCAN, ITEM_IRON_BAR, 4);
    int moved_copper = game.transfer_ore(PLANET_TERRA, PLANET_VULCAN, ITEM_COPPER_BAR, 2);
    FT_ASSERT(moved_iron >= 4);
    FT_ASSERT(moved_copper >= 2);
    FT_ASSERT(game.get_active_convoy_count() >= 2);
    FT_ASSERT_EQ(0, game.get_ore(PLANET_VULCAN, ITEM_IRON_BAR));
    FT_ASSERT_EQ(0, game.get_ore(PLANET_VULCAN, ITEM_COPPER_BAR));

    double vulcan_energy = game.get_planet_energy_generation(PLANET_VULCAN);
    FT_ASSERT(vulcan_energy > 5.9 && vulcan_energy < 6.1);
    game.tick(30.0);
    FT_ASSERT(game.get_active_convoy_count() >= 1);
    FT_ASSERT_EQ(0, game.get_ore(PLANET_VULCAN, ITEM_IRON_BAR));
    game.tick(20.0);
    FT_ASSERT_EQ(0, game.get_active_convoy_count());
    FT_ASSERT(game.get_ore(PLANET_VULCAN, ITEM_IRON_BAR) >= 4);
    FT_ASSERT(game.get_ore(PLANET_VULCAN, ITEM_COPPER_BAR) >= 2);
    game.tick(16.0);
    FT_ASSERT(game.get_planet_logistic_usage(PLANET_VULCAN) >= 2);
    double vulcan_use = game.get_planet_energy_consumption(PLANET_VULCAN);
    FT_ASSERT(vulcan_use > 3.9 && vulcan_use < 4.1);
    FT_ASSERT(game.get_ore(PLANET_VULCAN, ITEM_ENGINE_PART) >= 2);
    FT_ASSERT(game.get_planet_logistic_usage(PLANET_TERRA) >= 2);
    double terra_use_after = game.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(terra_use_after > 4.49 && terra_use_after < 4.51);
    double terra_mine_bonus = game.get_planet_mine_multiplier(PLANET_TERRA);
    FT_ASSERT(terra_mine_bonus > 0.99 && terra_mine_bonus < 1.01);

    int terra_capacity_before = game.get_planet_logistic_capacity(PLANET_TERRA);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 80);
    game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 20);
    FT_ASSERT(game.can_start_research(RESEARCH_URBAN_PLANNING_TERRA));
    FT_ASSERT(game.start_research(RESEARCH_URBAN_PLANNING_TERRA));
    double terra_planning_remaining = game.get_research_time_remaining(RESEARCH_URBAN_PLANNING_TERRA);
    FT_ASSERT(terra_planning_remaining > 19.9 && terra_planning_remaining < 20.1);
    game.tick(20.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_URBAN_PLANNING_TERRA));
    FT_ASSERT(game.get_planet_logistic_capacity(PLANET_TERRA) >= terra_capacity_before + 4);

    int mars_capacity_before = game.get_planet_logistic_capacity(PLANET_MARS);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 160);
    game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 40);
    FT_ASSERT(game.can_start_research(RESEARCH_URBAN_PLANNING_MARS));
    FT_ASSERT(game.start_research(RESEARCH_URBAN_PLANNING_MARS));
    double mars_planning_remaining = game.get_research_time_remaining(RESEARCH_URBAN_PLANNING_MARS);
    FT_ASSERT(mars_planning_remaining > 24.9 && mars_planning_remaining < 25.1);
    game.tick(25.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_URBAN_PLANNING_MARS));
    FT_ASSERT(game.get_planet_logistic_capacity(PLANET_MARS) >= mars_capacity_before + 4);

    int zalthor_capacity_before = game.get_planet_logistic_capacity(PLANET_ZALTHOR);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 240);
    game.set_ore(PLANET_TERRA, ITEM_ENGINE_PART, 80);
    FT_ASSERT(game.can_start_research(RESEARCH_URBAN_PLANNING_ZALTHOR));
    FT_ASSERT(game.start_research(RESEARCH_URBAN_PLANNING_ZALTHOR));
    double zalthor_planning_remaining = game.get_research_time_remaining(RESEARCH_URBAN_PLANNING_ZALTHOR);
    FT_ASSERT(zalthor_planning_remaining > 29.9 && zalthor_planning_remaining < 30.1);
    game.tick(30.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_URBAN_PLANNING_ZALTHOR));
    FT_ASSERT(game.get_planet_logistic_capacity(PLANET_ZALTHOR) >= zalthor_capacity_before + 4);

    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_SOLAR_PANELS));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_CRAFTING_MASTERY));
    game.set_ore(PLANET_TERRA, ORE_IRON, 240);
    game.set_ore(PLANET_TERRA, ORE_COPPER, 240);
    game.set_ore(PLANET_TERRA, ITEM_ACCUMULATOR, 6);
    FT_ASSERT(game.can_start_research(RESEARCH_SOLAR_PANELS));
    FT_ASSERT(game.start_research(RESEARCH_SOLAR_PANELS));
    double solar_remaining = game.get_research_time_remaining(RESEARCH_SOLAR_PANELS);
    FT_ASSERT(solar_remaining > 24.9 && solar_remaining < 25.1);
    game.tick(25.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_SOLAR_PANELS));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_CRAFTING_MASTERY));
    FT_ASSERT(game.can_place_building(PLANET_TERRA, BUILDING_SOLAR_ARRAY, 0, 1));
    int solar_instance = game.place_building(PLANET_TERRA, BUILDING_SOLAR_ARRAY, 0, 1);
    FT_ASSERT(solar_instance != 0);
    double terra_energy_with_solar = game.get_planet_energy_generation(PLANET_TERRA);
    FT_ASSERT(terra_energy_with_solar > 8.9 && terra_energy_with_solar < 9.1);

    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 260);
    game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 260);
    game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 6);
    FT_ASSERT(game.can_start_research(RESEARCH_CRAFTING_MASTERY));
    FT_ASSERT(game.start_research(RESEARCH_CRAFTING_MASTERY));
    double mastery_remaining = game.get_research_time_remaining(RESEARCH_CRAFTING_MASTERY);
    FT_ASSERT(mastery_remaining > 34.9 && mastery_remaining < 35.1);
    game.tick(35.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_CRAFTING_MASTERY));
    game.tick(10.0);
    double terra_energy_mastery = game.get_planet_energy_consumption(PLANET_TERRA);
    FT_ASSERT(terra_energy_mastery > 3.5 && terra_energy_mastery < 3.7);
    FT_ASSERT(terra_energy_mastery < terra_use_after - 0.7);
    FT_ASSERT(game.get_planet_logistic_usage(PLANET_TERRA) >= 2);
    return 1;
}

int evaluate_ship_upgrade_research(Game &game)
{
    FT_ASSERT(game.get_ship_hull_multiplier() > 0.99 && game.get_ship_hull_multiplier() < 1.01);
    FT_ASSERT(game.get_ship_shield_multiplier() > 0.99 && game.get_ship_shield_multiplier() < 1.01);
    FT_ASSERT(game.get_ship_weapon_multiplier() > 0.99 && game.get_ship_weapon_multiplier() < 1.01);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_I));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_II));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_I));
    FT_ASSERT_EQ(RESEARCH_STATUS_LOCKED, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_II));

    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_IRON_BAR);
    game.ensure_planet_item_slot(PLANET_TERRA, ORE_COAL);
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 220);
    game.set_ore(PLANET_TERRA, ORE_COAL, 220);
    FT_ASSERT(game.can_start_research(RESEARCH_STRUCTURAL_REINFORCEMENT_I));
    FT_ASSERT(game.start_research(RESEARCH_STRUCTURAL_REINFORCEMENT_I));
    double structural_one = game.get_research_time_remaining(RESEARCH_STRUCTURAL_REINFORCEMENT_I);
    FT_ASSERT(structural_one > 24.9 && structural_one < 25.1);
    game.tick(25.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_I));
    FT_ASSERT(game.get_ship_hull_multiplier() > 1.09 && game.get_ship_hull_multiplier() < 1.11);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_II));
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 260);
    game.set_ore(PLANET_TERRA, ORE_COAL, 260);
    FT_ASSERT(game.can_start_research(RESEARCH_STRUCTURAL_REINFORCEMENT_II));
    FT_ASSERT(game.start_research(RESEARCH_STRUCTURAL_REINFORCEMENT_II));
    double structural_two = game.get_research_time_remaining(RESEARCH_STRUCTURAL_REINFORCEMENT_II);
    FT_ASSERT(structural_two > 34.9 && structural_two < 35.1);
    game.tick(35.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_II));
    FT_ASSERT(game.get_ship_hull_multiplier() > 1.19 && game.get_ship_hull_multiplier() < 1.21);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_III));
    game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 320);
    game.set_ore(PLANET_TERRA, ORE_COAL, 320);
    FT_ASSERT(game.can_start_research(RESEARCH_STRUCTURAL_REINFORCEMENT_III));
    FT_ASSERT(game.start_research(RESEARCH_STRUCTURAL_REINFORCEMENT_III));
    double structural_three = game.get_research_time_remaining(RESEARCH_STRUCTURAL_REINFORCEMENT_III);
    FT_ASSERT(structural_three > 44.9 && structural_three < 45.1);
    game.tick(45.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_STRUCTURAL_REINFORCEMENT_III));
    FT_ASSERT(game.get_ship_hull_multiplier() > 1.29 && game.get_ship_hull_multiplier() < 1.31);

    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_COPPER_BAR);
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_MITHRIL_BAR);
    game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 220);
    game.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 220);
    FT_ASSERT(game.can_start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    FT_ASSERT(game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    double defensive_one = game.get_research_time_remaining(RESEARCH_DEFENSIVE_FORTIFICATION_I);
    FT_ASSERT(defensive_one > 29.9 && defensive_one < 30.1);
    game.tick(30.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_I));
    FT_ASSERT(game.get_ship_shield_multiplier() > 1.09 && game.get_ship_shield_multiplier() < 1.11);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 260);
    game.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 260);
    FT_ASSERT(game.can_start_research(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    FT_ASSERT(game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    double defensive_two = game.get_research_time_remaining(RESEARCH_DEFENSIVE_FORTIFICATION_II);
    FT_ASSERT(defensive_two > 39.9 && defensive_two < 40.1);
    game.tick(40.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_II));
    FT_ASSERT(game.get_ship_shield_multiplier() > 1.19 && game.get_ship_shield_multiplier() < 1.21);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_III));
    game.set_ore(PLANET_TERRA, ITEM_COPPER_BAR, 320);
    game.set_ore(PLANET_TERRA, ITEM_MITHRIL_BAR, 320);
    FT_ASSERT(game.can_start_research(RESEARCH_DEFENSIVE_FORTIFICATION_III));
    FT_ASSERT(game.start_research(RESEARCH_DEFENSIVE_FORTIFICATION_III));
    double defensive_three = game.get_research_time_remaining(RESEARCH_DEFENSIVE_FORTIFICATION_III);
    FT_ASSERT(defensive_three > 49.9 && defensive_three < 50.1);
    game.tick(50.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_DEFENSIVE_FORTIFICATION_III));
    FT_ASSERT(game.get_ship_shield_multiplier() > 1.29 && game.get_ship_shield_multiplier() < 1.31);

    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART);
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_TITANIUM_BAR);
    game.ensure_planet_item_slot(PLANET_TERRA, ORE_TRITIUM);
    game.ensure_planet_item_slot(PLANET_TERRA, ITEM_FUSION_REACTOR);
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 220);
    game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 220);
    game.set_ore(PLANET_TERRA, ORE_TRITIUM, 15);
    game.set_ore(PLANET_TERRA, ITEM_FUSION_REACTOR, 6);
    FT_ASSERT(game.can_start_research(RESEARCH_ARMAMENT_ENHANCEMENT_I));
    FT_ASSERT(game.start_research(RESEARCH_ARMAMENT_ENHANCEMENT_I));
    double armament_one = game.get_research_time_remaining(RESEARCH_ARMAMENT_ENHANCEMENT_I);
    FT_ASSERT(armament_one > 34.9 && armament_one < 35.1);
    game.tick(35.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_I));
    FT_ASSERT(game.get_ship_weapon_multiplier() > 1.09 && game.get_ship_weapon_multiplier() < 1.11);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_II));
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 260);
    game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 260);
    game.set_ore(PLANET_TERRA, ORE_TRITIUM, 20);
    FT_ASSERT(game.can_start_research(RESEARCH_ARMAMENT_ENHANCEMENT_II));
    FT_ASSERT(game.start_research(RESEARCH_ARMAMENT_ENHANCEMENT_II));
    double armament_two = game.get_research_time_remaining(RESEARCH_ARMAMENT_ENHANCEMENT_II);
    FT_ASSERT(armament_two > 44.9 && armament_two < 45.1);
    game.tick(45.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_II));
    FT_ASSERT(game.get_ship_weapon_multiplier() > 1.19 && game.get_ship_weapon_multiplier() < 1.21);
    FT_ASSERT_EQ(RESEARCH_STATUS_AVAILABLE, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_III));
    game.set_ore(PLANET_TERRA, ITEM_ADVANCED_ENGINE_PART, 320);
    game.set_ore(PLANET_TERRA, ITEM_TITANIUM_BAR, 320);
    game.set_ore(PLANET_TERRA, ORE_TRITIUM, 30);
    FT_ASSERT(game.can_start_research(RESEARCH_ARMAMENT_ENHANCEMENT_III));
    FT_ASSERT(game.start_research(RESEARCH_ARMAMENT_ENHANCEMENT_III));
    double armament_three = game.get_research_time_remaining(RESEARCH_ARMAMENT_ENHANCEMENT_III);
    FT_ASSERT(armament_three > 54.9 && armament_three < 55.1);
    game.tick(55.0);
    FT_ASSERT_EQ(RESEARCH_STATUS_COMPLETED, game.get_research_status(RESEARCH_ARMAMENT_ENHANCEMENT_III));
    FT_ASSERT(game.get_ship_weapon_multiplier() > 1.29 && game.get_ship_weapon_multiplier() < 1.31);
    return 1;
}

int verify_supply_contract_automation()
{
    Game contract_game(ft_string("127.0.0.1:8080"), ft_string("/"));
    contract_game.set_ore(PLANET_TERRA, ITEM_IRON_BAR, 60);
    contract_game.set_ore(PLANET_MARS, ITEM_IRON_BAR, 10);

    int invalid_contract = contract_game.create_supply_contract(PLANET_TERRA, PLANET_TERRA,
                                                                ITEM_IRON_BAR, 20, 30.0);
    FT_ASSERT_EQ(0, invalid_contract);

    int contract_id = contract_game.create_supply_contract(PLANET_TERRA, PLANET_MARS,
                                                           ITEM_IRON_BAR, 30, 45.0, 25, 2);
    FT_ASSERT(contract_id > 0);

    ft_vector<int> contract_ids;
    contract_game.get_supply_contract_ids(contract_ids);
    bool found = false;
    for (size_t idx = 0; idx < contract_ids.size(); ++idx)
    {
        if (contract_ids[idx] == contract_id)
        {
            found = true;
            break;
        }
    }
    FT_ASSERT(found);
