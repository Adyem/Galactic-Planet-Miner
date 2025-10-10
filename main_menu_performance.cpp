#include "main_menu_system.hpp"

#include "menu_localization.hpp"

#include "libft/Libft/libft.hpp"

void main_menu_performance_record_frame(
    MainMenuPerformanceStats &stats, long frame_start_ms, long frame_end_ms) noexcept
{
    if (frame_end_ms < frame_start_ms)
        return;

    long duration_ms = frame_end_ms - frame_start_ms;
    if (duration_ms < 0L)
        duration_ms = 0L;

    stats.fps_accumulated_ms += duration_ms;
    stats.fps_frame_count += 1U;

    if (stats.fps_accumulated_ms >= 500L)
    {
        long         total_ms = stats.fps_accumulated_ms;
        unsigned int frames = stats.fps_frame_count;
        if (frames > 0U && total_ms > 0L)
        {
            long         numerator = static_cast<long>(frames) * 1000L + total_ms / 2L;
            unsigned int computed_fps = static_cast<unsigned int>(numerator / total_ms);
            if (computed_fps == 0U)
                computed_fps = 1U;
            if (computed_fps > 999U)
                computed_fps = 999U;
            stats.fps_value = computed_fps;
            stats.has_fps = true;
            stats.fps_last_update_ms = frame_end_ms;
        }

        stats.fps_accumulated_ms = 0L;
        stats.fps_frame_count = 0U;
    }
}

void main_menu_performance_begin_latency_sample(MainMenuPerformanceStats &stats, long timestamp_ms) noexcept
{
    stats.latency_pending = true;
    stats.latency_last_update_ms = timestamp_ms;
}

void main_menu_performance_complete_latency_sample(MainMenuPerformanceStats &stats, bool success, long duration_ms,
    long timestamp_ms) noexcept
{
    stats.latency_pending = false;
    stats.latency_sampled = true;
    stats.latency_last_update_ms = timestamp_ms;

    if (duration_ms < 0L)
        duration_ms = -duration_ms;

    if (success)
    {
        stats.latency_successful = true;
        stats.latency_ms = duration_ms;
    }
    else
    {
        stats.latency_successful = false;
        stats.latency_ms = duration_ms;
    }
}

ft_string main_menu_format_performance_fps_label(const MainMenuPerformanceStats &stats)
{
    if (stats.has_fps)
    {
        unsigned int fps_value = stats.fps_value;
        if (fps_value == 0U)
            fps_value = 1U;

        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement fps_placeholder;
        fps_placeholder.key = ft_string("fps");
        fps_placeholder.value = ft_to_string(static_cast<int>(fps_value));
        replacements.push_back(fps_placeholder);
        return menu_localize_format("main_menu.performance.fps_value", "FPS: {{fps}}", replacements);
    }

    return menu_localize("main_menu.performance.fps_unknown", "FPS: --");
}

ft_string main_menu_format_performance_latency_label(const MainMenuPerformanceStats &stats)
{
    if (stats.latency_pending)
        return menu_localize("main_menu.performance.latency_pending", "Latency: Measuring...");

    if (!stats.latency_sampled)
        return menu_localize("main_menu.performance.latency_unknown", "Latency: --");

    if (!stats.latency_successful)
        return menu_localize("main_menu.performance.latency_offline", "Latency: Offline");

    long latency_ms = stats.latency_ms;
    if (latency_ms < 0L)
        latency_ms = 0L;

    ft_vector<StringTableReplacement> replacements;
    replacements.reserve(1U);
    StringTableReplacement ms_placeholder;
    ms_placeholder.key = ft_string("ms");
    ms_placeholder.value = ft_to_string(static_cast<int>(latency_ms));
    replacements.push_back(ms_placeholder);
    return menu_localize_format("main_menu.performance.latency_value", "Latency: {{ms}} ms", replacements);
}

namespace main_menu_testing
{
    void performance_record_frame(MainMenuPerformanceStats &stats, long frame_start_ms, long frame_end_ms)
    {
        main_menu_performance_record_frame(stats, frame_start_ms, frame_end_ms);
    }

    void performance_begin_latency(MainMenuPerformanceStats &stats, long timestamp_ms)
    {
        main_menu_performance_begin_latency_sample(stats, timestamp_ms);
    }

    void performance_complete_latency(
        MainMenuPerformanceStats &stats, bool success, long duration_ms, long timestamp_ms)
    {
        main_menu_performance_complete_latency_sample(stats, success, duration_ms, timestamp_ms);
    }

    ft_string format_performance_fps(const MainMenuPerformanceStats &stats)
    {
        return main_menu_format_performance_fps_label(stats);
    }

    ft_string format_performance_latency(const MainMenuPerformanceStats &stats)
    {
        return main_menu_format_performance_latency_label(stats);
    }
}
