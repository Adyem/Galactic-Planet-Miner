    void init_raider_profiles(ship_profile_table &table)
    {
        ship_profile &raider_corvette = table.profiles[SHIP_RAIDER_CORVETTE];
        raider_corvette.armor = 24;
        raider_corvette.hp = 190;
        raider_corvette.shield = 70;
        raider_corvette.max_speed = 27.0;
        raider_corvette.acceleration = 6.8;
        raider_corvette.deceleration = 6.3;
        raider_corvette.turn_speed = 118.0;
        raider_corvette.optimal_range = 185.0;
        raider_corvette.max_range = 245.0;
        raider_corvette.attack_speed = 1.05;
        raider_corvette.base_damage = 9.5;
        raider_corvette.combat_behavior = SHIP_BEHAVIOR_CHARGE;
        raider_corvette.outnumbered_behavior = SHIP_BEHAVIOR_FLANK_SWEEP;
        raider_corvette.unescorted_behavior = SHIP_BEHAVIOR_CHARGE;
        raider_corvette.low_hp_behavior = SHIP_BEHAVIOR_RETREAT;

        ship_profile &raider_destroyer = table.profiles[SHIP_RAIDER_DESTROYER];
        raider_destroyer.armor = 36;
        raider_destroyer.hp = 260;
        raider_destroyer.shield = 110;
        raider_destroyer.max_speed = 20.5;
        raider_destroyer.acceleration = 4.8;
        raider_destroyer.deceleration = 4.0;
        raider_destroyer.turn_speed = 92.0;
        raider_destroyer.optimal_range = 210.0;
        raider_destroyer.max_range = 280.0;
        raider_destroyer.attack_speed = 0.95;
        raider_destroyer.base_damage = 12.5;
        raider_destroyer.combat_behavior = SHIP_BEHAVIOR_LINE_HOLD;
        raider_destroyer.outnumbered_behavior = SHIP_BEHAVIOR_LINE_HOLD;
        raider_destroyer.unescorted_behavior = SHIP_BEHAVIOR_LINE_HOLD;
        raider_destroyer.low_hp_behavior = SHIP_BEHAVIOR_WITHDRAW_SUPPORT;

        ship_profile &raider_battleship = table.profiles[SHIP_RAIDER_BATTLESHIP];
        raider_battleship.armor = 68;
        raider_battleship.hp = 430;
        raider_battleship.shield = 210;
        raider_battleship.max_speed = 18.0;
        raider_battleship.acceleration = 3.0;
        raider_battleship.deceleration = 2.1;
        raider_battleship.turn_speed = 50.0;
        raider_battleship.optimal_range = 240.0;
        raider_battleship.max_range = 320.0;
        raider_battleship.attack_speed = 0.85;
        raider_battleship.base_damage = 16.0;
        raider_battleship.combat_behavior = SHIP_BEHAVIOR_CHARGE;
        raider_battleship.outnumbered_behavior = SHIP_BEHAVIOR_LINE_HOLD;
        raider_battleship.unescorted_behavior = SHIP_BEHAVIOR_CHARGE;
        raider_battleship.low_hp_behavior = SHIP_BEHAVIOR_LAST_STAND;
    }
