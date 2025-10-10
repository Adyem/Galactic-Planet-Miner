void Game::produce(double seconds)
{
    ft_vector<Pair<int, ft_sharedptr<ft_planet> > > planet_entries;
    ft_map_snapshot(this->_planets, planet_entries);
    size_t planet_count = planet_entries.size();
    for (size_t i = 0; i < planet_count; ++i)
    {
        int planet_id = planet_entries[i].key;
        ft_sharedptr<ft_planet> planet = planet_entries[i].value;
        if (!planet)
            continue;
        ft_vector<Pair<int, int> > produced = planet->produce(seconds);
        double mine_multiplier = this->_buildings.get_mine_multiplier(planet_id);
        for (size_t j = 0; j < produced.size(); ++j)
        {
            int ore_id = produced[j].key;
            int base_amount = produced[j].value;
            int final_amount = base_amount;
            if (base_amount > 0)
            {
                double multiplier_delta = this->_resource_multiplier - 1.0;
                if (multiplier_delta < 0.0)
                    multiplier_delta = -multiplier_delta;
                if (multiplier_delta > 0.000001)
                {
                    double scaled_amount = static_cast<double>(base_amount) * this->_resource_multiplier;
                    int target = base_amount;
                    if (this->_resource_multiplier < 1.0)
                    {
                        const double epsilon = 0.0000001;
                        Pair<int, ft_resource_accumulator> *ore_accumulator = this->get_resource_accumulator(planet_id, ore_id, true);
                        double carryover = 0.0;
                        if (ore_accumulator != ft_nullptr)
                            carryover = ore_accumulator->value.multiplier_deficit;
                        double total = scaled_amount + carryover;
                        target = static_cast<int>(total + epsilon);
                        double remainder = total - static_cast<double>(target);
                        if (remainder < 0.0)
                            remainder = 0.0;
                        if (ore_accumulator != ft_nullptr)
                            ore_accumulator->value.multiplier_deficit = remainder;
                        if (target > base_amount)
                            target = base_amount;
                        if (target < 0)
                            target = 0;
                    }
                    else
                    {
                        int scaled_target = static_cast<int>(scaled_amount);
                        target = scaled_target;
                        if (this->_resource_multiplier > 1.0)
                        {
                            double fractional = scaled_amount - static_cast<double>(scaled_target);
                            if (fractional > 0.000001)
                                target += 1;
                        }
                        if (target < 0)
                            target = 0;
                    }
                    final_amount = target;
                    int diff = final_amount - base_amount;
                    if (diff > 0)
                        planet->add_resource(ore_id, diff);
                    else if (diff < 0)
                        planet->sub_resource(ore_id, -diff);
                }
            }
            if (mine_multiplier > 1.0 && final_amount > 0)
            {
                Pair<int, ft_resource_accumulator> *ore_accumulator = this->get_resource_accumulator(planet_id, ore_id, false);
                double bonus_amount = (mine_multiplier - 1.0) * static_cast<double>(final_amount);
                if (ore_accumulator != ft_nullptr)
                    bonus_amount += ore_accumulator->value.mine_bonus_remainder;
                int bonus = static_cast<int>(bonus_amount);
                double remainder = bonus_amount - static_cast<double>(bonus);
                if (remainder < 0.0)
                    remainder = 0.0;
                if (ore_accumulator == ft_nullptr)
                {
                    if (bonus > 0 || remainder > 0.0)
                        ore_accumulator = this->get_resource_accumulator(planet_id, ore_id, true);
                }
                if (bonus > 0)
                {
                    planet->add_resource(ore_id, bonus);
                }
                if (ore_accumulator != ft_nullptr)
                    ore_accumulator->value.mine_bonus_remainder = remainder;
            }
            this->send_state(planet_id, ore_id);
        }
    }
}

void Game::update_gameplay(double milliseconds)
{
    double seconds = milliseconds * 0.001;
    if (seconds < 0.0)
        seconds = 0.0;
    this->produce(seconds);
    this->process_supply_contracts(seconds);
    this->advance_convoys(seconds);
    this->_buildings.tick(*this, seconds);
    ft_vector<Pair<int, ft_sharedptr<ft_fleet> > > fleet_entries;
    ft_map_snapshot(this->_fleets, fleet_entries);
    size_t count = fleet_entries.size();
    for (size_t i = 0; i < count; ++i)
    {
        ft_sharedptr<ft_fleet> fleet = fleet_entries[i].value;
        fleet->tick(seconds);
    }
    ft_vector<int> completed;
    this->_research.tick(seconds, completed);
    for (size_t i = 0; i < completed.size(); ++i)
        this->handle_research_completion(completed[i]);

    ft_quest_context quest_context;
    this->build_quest_context(quest_context);
    ft_vector<int> quest_completed;
    ft_vector<int> quest_failed;
    ft_vector<int> quest_choices;
    this->_quests.update(seconds, quest_context, quest_completed, quest_failed, quest_choices);
    for (size_t i = 0; i < quest_completed.size(); ++i)
        this->handle_quest_completion(quest_completed[i]);
    for (size_t i = 0; i < quest_failed.size(); ++i)
        this->handle_quest_failure(quest_failed[i]);
    for (size_t i = 0; i < quest_choices.size(); ++i)
        this->handle_quest_choice_prompt(quest_choices[i]);
}

void Game::update_combat(double milliseconds)
{
    double seconds = milliseconds * 0.001;
    if (seconds < 0.0)
        seconds = 0.0;
    seconds *= this->_combat_speed_multiplier;
    ft_vector<int> active_planets;
    this->_combat.get_active_planets(active_planets);
    for (size_t i = 0; i < active_planets.size(); ++i)
    {
        double stability = this->compute_auto_shield_generator_stability(active_planets[i]);
        this->_combat.set_auto_shield_generator(active_planets[i], stability);
    }
    ft_vector<int> assault_completed;
    ft_vector<int> assault_failed;
    this->_combat.tick(seconds, this->_fleets, this->_planet_fleets, assault_completed, assault_failed);
    for (size_t i = 0; i < assault_completed.size(); ++i)
    {
        int planet_id = assault_completed[i];
        ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
        if (planet)
        {
            int reward_ore = this->select_planet_resource_for_assault(planet, 0, true);
            if (reward_ore != 0)
            {
                planet->add_resource(reward_ore, 3);
                this->send_state(planet_id, reward_ore);
            }
        }
        ft_string entry("Old Miner Joe records a victory at planet ");
        entry.append(ft_to_string(planet_id));
        entry.append(ft_string(": the raider fleet was repelled."));
        this->append_lore_entry(entry);
        if (planet_id == this->_order_branch_pending_assault)
        {
            this->_order_branch_assault_victories += 1;
            this->_order_branch_pending_assault = 0;
            ft_string branch_entry("Marshal Rhea honors the Dominion strike force triumphant at planet ");
            branch_entry.append(ft_to_string(planet_id));
            branch_entry.append(ft_string("."));
            this->append_lore_entry(branch_entry);
        }
        if (planet_id == this->_rebellion_branch_pending_assault)
        {
            this->_rebellion_branch_assault_victories += 1;
            this->_rebellion_branch_pending_assault = 0;
            ft_string branch_entry("Captain Blackthorne salutes the liberated crews holding planet ");
            branch_entry.append(ft_to_string(planet_id));
            branch_entry.append(ft_string("."));
            this->append_lore_entry(branch_entry);
        }
        this->record_combat_victory_narrative(planet_id);
    }
    for (size_t i = 0; i < assault_failed.size(); ++i)
    {
        int planet_id = assault_failed[i];
        ft_sharedptr<ft_planet> planet = this->get_planet(planet_id);
        if (planet)
        {
            int penalty_ore = this->select_planet_resource_for_assault(planet, 2, false);
            if (penalty_ore != 0)
            {
                planet->sub_resource(penalty_ore, 2);
                this->send_state(planet_id, penalty_ore);
            }
        }
        ft_string entry("Professor Lumen warns of losses on planet ");
        entry.append(ft_to_string(planet_id));
        entry.append(ft_string(": raiders breached the defenses."));
        this->append_lore_entry(entry);
        if (planet_id == this->_order_branch_pending_assault)
        {
            if (this->get_quest_status(QUEST_ORDER_DOMINION) == QUEST_STATUS_ACTIVE)
                this->trigger_branch_assault(planet_id, 1.2, true);
            else
                this->_order_branch_pending_assault = 0;
        }
        if (planet_id == this->_rebellion_branch_pending_assault)
        {
            if (this->get_quest_status(QUEST_REBELLION_LIBERATION) == QUEST_STATUS_ACTIVE)
                this->trigger_branch_assault(planet_id, 1.1, false);
            else
                this->_rebellion_branch_pending_assault = 0;
        }
    }
    if (this->_order_branch_pending_assault == 0 && this->_rebellion_branch_pending_assault == 0)
    {
        this->_combat.clear_branch_assault_state();
    }
}
