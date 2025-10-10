    for (size_t i = 0; i < definition_count; ++i)
    {
        const ft_sharedptr<ft_quest_definition> &definition_ptr = definitions[i].value;
        if (!definition_ptr)
            continue;
        const ft_quest_definition &definition = *definition_ptr;

        ft_quest_log_entry entry;
        entry.quest_id = definition.id;
        entry.name = definition.name;
        entry.description = definition.description;
        entry.status = this->_quests.get_status(definition.id);
        entry.time_remaining = this->_quests.get_time_remaining(definition.id);
        if (definition.time_limit > 0.0)
            entry.time_limit = definition.time_limit * this->get_effective_quest_time_scale();
        else
            entry.time_limit = 0.0;
        if (entry.time_limit > 0.0)
        {
            double ratio = entry.time_remaining / entry.time_limit;
            if (ratio < 0.0)
                ratio = 0.0;
            if (ratio > 1.0)
                ratio = 1.0;
            entry.time_remaining_ratio = ratio;
        }
        else
            entry.time_remaining_ratio = 0.0;
        entry.is_side_quest = definition.is_side_quest;
        entry.requires_choice = definition.requires_choice;
        entry.awaiting_choice = (entry.status == QUEST_STATUS_AWAITING_CHOICE);
        entry.selected_choice = this->_quests.get_choice(definition.id);

        entry.prerequisites_met = true;
        for (size_t j = 0; j < definition.prerequisites.size(); ++j)
        {
            int prerequisite_id = definition.prerequisites[j];
            if (this->_quests.get_status(prerequisite_id) != QUEST_STATUS_COMPLETED)
            {
                entry.prerequisites_met = false;
                break;
            }
        }

        entry.branch_requirement_met = true;
        if (definition.required_choice_quest != 0)
        {
            int required_choice = this->_quests.get_choice(definition.required_choice_quest);
            entry.branch_requirement_met = (required_choice == definition.required_choice_value);
        }

        entry.objectives_completed = true;
        for (size_t j = 0; j < definition.objectives.size(); ++j)
        {
            const ft_quest_objective &objective = definition.objectives[j];
            ft_quest_objective_snapshot snapshot;
            snapshot.type = objective.type;
            snapshot.target_id = objective.target_id;
            snapshot.required_amount = objective.amount;
            snapshot.current_amount = get_objective_current_amount(objective, context);
            snapshot.is_met = is_objective_met_for_snapshot(objective, context);
            if (!snapshot.is_met)
                entry.objectives_completed = false;
            entry.objectives.push_back(snapshot);
        }

        for (size_t j = 0; j < definition.choices.size(); ++j)
        {
            const ft_quest_choice_definition &choice_definition = definition.choices[j];
            ft_quest_choice_snapshot choice_entry;
            choice_entry.choice_id = choice_definition.choice_id;
            choice_entry.description = choice_definition.description;
            choice_entry.is_selected = (entry.selected_choice == choice_definition.choice_id);
            choice_entry.is_available = entry.awaiting_choice;
            entry.choices.push_back(choice_entry);
        }

        if (entry.awaiting_choice)
            out.awaiting_choice_ids.push_back(entry.quest_id);

        if (definition.is_side_quest)
            out.side_quests.push_back(ft_move(entry));
        else
            out.main_quests.push_back(ft_move(entry));
    }
