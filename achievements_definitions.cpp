#include "achievements.hpp"
#include "libft/Template/function.hpp"

void AchievementManager::bootstrap_definitions()
{
    const ft_function<void (int, const char *, const char *, int, int, e_achievement_progress_mode)> add_definition = [this](
        int id,
        const char *name,
        const char *description,
        int event_id,
        int target,
        e_achievement_progress_mode mode)
    {
        ft_sharedptr<ft_achievement_definition> definition(new ft_achievement_definition());
        definition->id = id;
        definition->name = ft_string(name);
        definition->description = ft_string(description);
        definition->event_id = event_id;
        definition->target_value = target;
        definition->progress_mode = mode;
        this->register_achievement(definition);
    };

    add_definition(
        ACHIEVEMENT_SECOND_HOME,
        "Second Home",
        "Unlock an additional colony world.",
        ACHIEVEMENT_EVENT_PLANET_UNLOCKED,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_RESEARCH_PIONEER,
        "Research Pioneer",
        "Complete three major research projects.",
        ACHIEVEMENT_EVENT_RESEARCH_COMPLETED,
        3,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_LOGISTICS_PILOT,
        "Logistics Pilot",
        "Deliver three convoys successfully.",
        ACHIEVEMENT_EVENT_CONVOY_DELIVERED,
        3,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_CONVOY_STREAK_GUARDIAN,
        "Streak Guardian",
        "Maintain a five convoy success streak.",
        ACHIEVEMENT_EVENT_CONVOY_STREAK_BEST,
        5,
        ACHIEVEMENT_PROGRESS_BEST_VALUE);

    add_definition(
        ACHIEVEMENT_QUEST_INITIAL_SKIRMISHES,
        "Perimeter Guardian",
        "Complete the Initial Raider Skirmishes quest.",
        ACHIEVEMENT_EVENT_QUEST_INITIAL_SKIRMISHES,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_DEFENSE_OF_TERRA,
        "Terra's Shield",
        "Complete the Defense of Terra quest.",
        ACHIEVEMENT_EVENT_QUEST_DEFENSE_OF_TERRA,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_INVESTIGATE_RAIDERS,
        "Cipher Breaker",
        "Complete the Investigate Raider Motives quest.",
        ACHIEVEMENT_EVENT_QUEST_INVESTIGATE_RAIDERS,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_SECURE_SUPPLY_LINES,
        "Supply Line Sentinel",
        "Complete the Secure Supply Lines quest.",
        ACHIEVEMENT_EVENT_QUEST_SECURE_SUPPLY_LINES,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_STEADY_SUPPLY_STREAK,
        "Streak Specialist",
        "Complete the Steady Supply Streak quest.",
        ACHIEVEMENT_EVENT_QUEST_STEADY_SUPPLY_STREAK,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_HIGH_VALUE_ESCORT,
        "Escort Commander",
        "Complete the High-Value Escort quest.",
        ACHIEVEMENT_EVENT_QUEST_HIGH_VALUE_ESCORT,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_CLIMACTIC_BATTLE,
        "Climactic Victor",
        "Complete the Climactic Battle quest.",
        ACHIEVEMENT_EVENT_QUEST_CLIMACTIC_BATTLE,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_CRITICAL_DECISION,
        "Decisive Arbiter",
        "Resolve The Critical Decision quest.",
        ACHIEVEMENT_EVENT_QUEST_CRITICAL_DECISION,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_ORDER_UPRISING,
        "Order's Hammer",
        "Complete the Order's Last Stand quest.",
        ACHIEVEMENT_EVENT_QUEST_ORDER_UPRISING,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_REBELLION_FLEET,
        "Rebellion's Hope",
        "Complete the Rebellion Rising quest.",
        ACHIEVEMENT_EVENT_QUEST_REBELLION_FLEET,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_ORDER_SUPPRESS_RAIDS,
        "Sensor Web Architect",
        "Complete the Suppress the Raider Cells quest.",
        ACHIEVEMENT_EVENT_QUEST_ORDER_SUPPRESS_RAIDS,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_ORDER_DOMINION,
        "Dominion Enforcer",
        "Complete the Order Dominion quest.",
        ACHIEVEMENT_EVENT_QUEST_ORDER_DOMINION,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_ORDER_FINAL_VERDICT,
        "Order Tribunal Arbiter",
        "Resolve the Final Order Mandate quest.",
        ACHIEVEMENT_EVENT_QUEST_ORDER_FINAL_VERDICT,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_REBELLION_NETWORK,
        "Shadow Network Weaver",
        "Complete the Shadow Network quest.",
        ACHIEVEMENT_EVENT_QUEST_REBELLION_NETWORK,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_REBELLION_LIBERATION,
        "Liberation Vanguard",
        "Complete the Liberation of the Frontier quest.",
        ACHIEVEMENT_EVENT_QUEST_REBELLION_LIBERATION,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);

    add_definition(
        ACHIEVEMENT_QUEST_REBELLION_FINAL_PUSH,
        "Freedom's Edge",
        "Complete the Battle for Freedom quest.",
        ACHIEVEMENT_EVENT_QUEST_REBELLION_FINAL_PUSH,
        1,
        ACHIEVEMENT_PROGRESS_INCREMENTAL);
}

