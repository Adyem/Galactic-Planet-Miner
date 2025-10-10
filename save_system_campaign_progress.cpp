                size_t instance_count = instance_lookup.size();
                if (instance_count > 0)
                {
                    const Pair<int, ft_building_instance> *inst_entries = instance_lookup.end();
                    inst_entries -= instance_count;
                    for (size_t j = 0; j < instance_count; ++j)
                    {
                        ft_building_instance instance = inst_entries[j].value;
                        if (instance.uid == 0)
                            instance.uid = inst_entries[j].key;
                        state.instances.insert(inst_entries[j].key, instance);
                    }
                }
            }

            parsed_planets.remove(planet_id);
            parsed_planets.insert(planet_id, ft_move(state));
        }
        current = current->next;
    }

    json_free_groups(groups);
    buildings._planets.clear();
    buildings._building_unlocks.clear();
    buildings._crafting_energy_multiplier = parsed_crafting_energy;
    buildings._crafting_speed_multiplier = parsed_crafting_speed;
    buildings._global_energy_multiplier = parsed_global_energy;

    size_t unlock_count = parsed_unlocks.size();
    if (unlock_count > 0)
    {
        const Pair<int, bool> *unlock_entries = parsed_unlocks.end();
        unlock_entries -= unlock_count;
        for (size_t i = 0; i < unlock_count; ++i)
            buildings._building_unlocks.insert(unlock_entries[i].key, unlock_entries[i].value);
    }

    size_t planet_count = parsed_planets.size();
    if (planet_count > 0)
    {
        Pair<int, ft_planet_build_state> *entries = parsed_planets.end();
        entries -= planet_count;
        for (size_t i = 0; i < planet_count; ++i)
            buildings._planets.insert(entries[i].key, ft_move(entries[i].value));
    }
    return true;
}

ft_string SaveSystem::serialize_campaign_progress(int convoys_delivered_total,
    int convoy_raid_losses, int current_delivery_streak,
    int longest_delivery_streak, size_t next_streak_milestone_index,
    int order_branch_assault_victories,
    int rebellion_branch_assault_victories,
    int order_branch_pending_assault,
    int rebellion_branch_pending_assault) const noexcept
{
    json_document document;
    json_group *group = save_system_create_group(document, "campaign_progress");
    if (!group)
        return save_system_abort_serialization(document);
    if (!save_system_add_item(document, group, "convoys_delivered_total",
        convoys_delivered_total))
    {
        return save_system_abort_serialization(document);
    }
    if (!save_system_add_item(document, group, "convoy_raid_losses",
        convoy_raid_losses))
    {
        return save_system_abort_serialization(document);
    }
    if (!save_system_add_item(document, group, "current_delivery_streak",
        current_delivery_streak))
    {
        return save_system_abort_serialization(document);
    }
    if (!save_system_add_item(document, group, "longest_delivery_streak",
        longest_delivery_streak))
    {
        return save_system_abort_serialization(document);
    }
    ft_string milestone_value = ft_to_string(static_cast<long>(next_streak_milestone_index));
    if (!save_system_add_item(document, group, "next_streak_milestone_index",
        milestone_value.c_str()))
    {
        return save_system_abort_serialization(document);
    }
    if (!save_system_add_item(document, group, "order_branch_assault_victories",
        order_branch_assault_victories))
    {
        return save_system_abort_serialization(document);
    }
    if (!save_system_add_item(document, group, "rebellion_branch_assault_victories",
        rebellion_branch_assault_victories))
    {
        return save_system_abort_serialization(document);
    }
    if (!save_system_add_item(document, group, "order_branch_pending_assault",
        order_branch_pending_assault))
    {
        return save_system_abort_serialization(document);
    }
    if (!save_system_add_item(document, group, "rebellion_branch_pending_assault",
        rebellion_branch_pending_assault))
    {
        return save_system_abort_serialization(document);
    }
    char *serialized = document.write_to_string();
    if (!serialized)
        return ft_string();
    ft_string result(serialized);
    cma_free(serialized);
    return result;
}

bool SaveSystem::deserialize_campaign_progress(const char *content,
    int &convoys_delivered_total, int &convoy_raid_losses,
    int &current_delivery_streak, int &longest_delivery_streak,
    size_t &next_streak_milestone_index,
    int &order_branch_assault_victories,
    int &rebellion_branch_assault_victories,
    int &order_branch_pending_assault,
    int &rebellion_branch_pending_assault) const noexcept
{
    if (!content)
        return false;
    json_group *groups = json_read_from_string(content);
    if (!groups)
        return false;
    json_group *current = groups;
    while (current)
    {
        json_item *item = current->items;
        while (item)
        {
            if (!item->key)
            {
                item = item->next;
                continue;
            }
            if (ft_strcmp(item->key, "convoys_delivered_total") == 0)
            {
                long parsed = ft_atol(item->value);
                if (parsed < 0)
                    parsed = 0;
                if (parsed > FT_INT_MAX)
                    parsed = FT_INT_MAX;
                convoys_delivered_total = static_cast<int>(parsed);
            }
            else if (ft_strcmp(item->key, "convoy_raid_losses") == 0)
            {
                long parsed = ft_atol(item->value);
                if (parsed < 0)
                    parsed = 0;
                if (parsed > FT_INT_MAX)
                    parsed = FT_INT_MAX;
                convoy_raid_losses = static_cast<int>(parsed);
            }
            else if (ft_strcmp(item->key, "current_delivery_streak") == 0)
            {
                long parsed = ft_atol(item->value);
                if (parsed < 0)
                    parsed = 0;
                if (parsed > FT_INT_MAX)
                    parsed = FT_INT_MAX;
                current_delivery_streak = static_cast<int>(parsed);
            }
            else if (ft_strcmp(item->key, "longest_delivery_streak") == 0)
            {
                long parsed = ft_atol(item->value);
                if (parsed < 0)
                    parsed = 0;
                if (parsed > FT_INT_MAX)
                    parsed = FT_INT_MAX;
                longest_delivery_streak = static_cast<int>(parsed);
            }
            else if (ft_strcmp(item->key, "next_streak_milestone_index") == 0)
            {
                long parsed = ft_atol(item->value);
                if (parsed < 0)
                    parsed = 0;
                next_streak_milestone_index = static_cast<size_t>(parsed);
            }
            else if (ft_strcmp(item->key,
                "order_branch_assault_victories") == 0)
            {
                long parsed = ft_atol(item->value);
                if (parsed < 0)
                    parsed = 0;
                if (parsed > FT_INT_MAX)
                    parsed = FT_INT_MAX;
                order_branch_assault_victories = static_cast<int>(parsed);
            }
            else if (ft_strcmp(item->key,
                "rebellion_branch_assault_victories") == 0)
            {
                long parsed = ft_atol(item->value);
                if (parsed < 0)
                    parsed = 0;
                if (parsed > FT_INT_MAX)
                    parsed = FT_INT_MAX;
                rebellion_branch_assault_victories = static_cast<int>(parsed);
            }
            else if (ft_strcmp(item->key, "order_branch_pending_assault") == 0)
            {
                long parsed = ft_atol(item->value);
                if (parsed < 0)
                    parsed = 0;
                if (parsed > FT_INT_MAX)
                    parsed = FT_INT_MAX;
                order_branch_pending_assault = static_cast<int>(parsed);
            }
            else if (ft_strcmp(item->key,
                "rebellion_branch_pending_assault") == 0)
            {
                long parsed = ft_atol(item->value);
                if (parsed < 0)
                    parsed = 0;
                if (parsed > FT_INT_MAX)
                    parsed = FT_INT_MAX;
                rebellion_branch_pending_assault = static_cast<int>(parsed);
            }
            item = item->next;
        }
        current = current->next;
    }
    json_free_groups(groups);
    return true;
}
