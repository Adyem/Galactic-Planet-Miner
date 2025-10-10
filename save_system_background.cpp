#include "save_system_background.hpp"

#include "libft/Template/vector.hpp"

namespace
{
    ft_vector<SaveSystemBackgroundEvent> g_background_queue;
}

void save_system_background_push_started(const ft_string &slot_name, long timestamp_ms) noexcept
{
    SaveSystemBackgroundEvent event;
    event.type = SaveSystemBackgroundEvent::SAVE_SYSTEM_BACKGROUND_EVENT_STARTED;
    event.slot_name = slot_name;
    event.success = false;
    event.error_message.clear();
    event.timestamp_ms = timestamp_ms;
    g_background_queue.push_back(event);
}

void save_system_background_push_completed(
    const ft_string &slot_name, bool success, const ft_string &error_message, long timestamp_ms) noexcept
{
    SaveSystemBackgroundEvent event;
    event.type = SaveSystemBackgroundEvent::SAVE_SYSTEM_BACKGROUND_EVENT_COMPLETED;
    event.slot_name = slot_name;
    event.success = success;
    event.error_message = error_message;
    event.timestamp_ms = timestamp_ms;
    g_background_queue.push_back(event);
}

bool save_system_background_poll_event(SaveSystemBackgroundEvent &out_event) noexcept
{
    if (g_background_queue.empty())
        return false;

    out_event = g_background_queue[0];
    g_background_queue.erase(g_background_queue.begin());
    return true;
}

void save_system_background_reset() noexcept
{
    g_background_queue.clear();
}
