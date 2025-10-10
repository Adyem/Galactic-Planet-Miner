#include "main_menu_system.hpp"

#include "achievements.hpp"
#include "menu_localization.hpp"
#include "libft/Libft/libft.hpp"

namespace
{
    unsigned int resolve_cached_achievement_total() noexcept
    {
        static unsigned int cached_total = 0U;
        if (cached_total == 0U)
        {
            AchievementManager manager;
            ft_vector<int>     ids;
            manager.get_achievement_ids(ids);
            cached_total = static_cast<unsigned int>(ids.size());
            if (cached_total > PLAYER_PROFILE_ACHIEVEMENTS_MAX_COUNT)
                cached_total = PLAYER_PROFILE_ACHIEVEMENTS_MAX_COUNT;
        }
        return cached_total;
    }
}

unsigned int main_menu_resolve_total_achievements() noexcept
{
    return resolve_cached_achievement_total();
}

MainMenuAchievementsSummary main_menu_build_achievements_summary(const PlayerProfilePreferences *preferences) noexcept
{
    MainMenuAchievementsSummary summary;
    if (preferences != ft_nullptr)
    {
        summary.total_count = preferences->achievements_total_count;
        summary.completed_count = preferences->achievements_completed_count;
        if (!preferences->achievements_highlight_label.empty())
        {
            summary.highlight_label = preferences->achievements_highlight_label;
            summary.has_highlight = true;
        }
        if (!preferences->achievements_progress_note.empty())
        {
            summary.progress_note = preferences->achievements_progress_note;
            summary.has_progress_note = true;
        }
    }

    if (summary.total_count == 0U)
        summary.total_count = resolve_cached_achievement_total();
    if (summary.total_count > PLAYER_PROFILE_ACHIEVEMENTS_MAX_COUNT)
        summary.total_count = PLAYER_PROFILE_ACHIEVEMENTS_MAX_COUNT;
    if (summary.completed_count > summary.total_count && summary.total_count > 0U)
        summary.completed_count = summary.total_count;

    return summary;
}

ft_string main_menu_format_achievements_completion_label(const MainMenuAchievementsSummary &summary)
{
    ft_vector<StringTableReplacement> replacements;
    replacements.reserve(2U);

    StringTableReplacement completed_placeholder;
    completed_placeholder.key = ft_string("completed");
    completed_placeholder.value = ft_to_string(static_cast<int>(summary.completed_count));
    replacements.push_back(completed_placeholder);

    if (summary.total_count > 0U)
    {
        StringTableReplacement total_placeholder;
        total_placeholder.key = ft_string("total");
        total_placeholder.value = ft_to_string(static_cast<int>(summary.total_count));
        replacements.push_back(total_placeholder);
        return menu_localize_format("main_menu.achievements.completed",
            "Achievements: {{completed}} / {{total}} Completed", replacements);
    }

    return menu_localize_format("main_menu.achievements.completed_unknown_total",
        "Achievements: {{completed}} Completed", replacements);
}

ft_vector<ft_string> main_menu_collect_achievement_lines(const MainMenuAchievementsSummary &summary)
{
    ft_vector<ft_string> lines;
    lines.reserve(2U);

    if (summary.has_highlight && !summary.highlight_label.empty())
    {
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement highlight_placeholder;
        highlight_placeholder.key = ft_string("highlight");
        highlight_placeholder.value = summary.highlight_label;
        replacements.push_back(highlight_placeholder);
        lines.push_back(menu_localize_format("main_menu.achievements.highlight",
            "Spotlight: {{highlight}}", replacements));
    }

    if (summary.has_progress_note && !summary.progress_note.empty())
    {
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement progress_placeholder;
        progress_placeholder.key = ft_string("progress");
        progress_placeholder.value = summary.progress_note;
        replacements.push_back(progress_placeholder);
        lines.push_back(menu_localize_format("main_menu.achievements.progress",
            "Next Goal: {{progress}}", replacements));
    }

    if (lines.empty())
        lines.push_back(menu_localize("main_menu.achievements.none",
            "Complete a campaign to start unlocking achievements."));

    return lines;
}

namespace main_menu_testing
{
    MainMenuAchievementsSummary build_achievements_summary(const PlayerProfilePreferences &preferences) noexcept
    {
        return main_menu_build_achievements_summary(&preferences);
    }

    ft_string format_achievements_completion(const MainMenuAchievementsSummary &summary)
    {
        return main_menu_format_achievements_completion_label(summary);
    }

    ft_vector<ft_string> collect_achievement_lines(const MainMenuAchievementsSummary &summary)
    {
        return main_menu_collect_achievement_lines(summary);
    }
}
