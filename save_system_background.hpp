#pragma once

#include "libft/Libft/libft.hpp"

struct SaveSystemBackgroundEvent
{
    enum e_type
    {
        SAVE_SYSTEM_BACKGROUND_EVENT_NONE = 0,
        SAVE_SYSTEM_BACKGROUND_EVENT_STARTED = 1,
        SAVE_SYSTEM_BACKGROUND_EVENT_COMPLETED = 2
    };

    e_type    type;
    ft_string slot_name;
    bool      success;
    ft_string error_message;
    long      timestamp_ms;

    SaveSystemBackgroundEvent() noexcept
        : type(SAVE_SYSTEM_BACKGROUND_EVENT_NONE)
        , slot_name()
        , success(false)
        , error_message()
        , timestamp_ms(0)
    {}
};

void save_system_background_push_started(const ft_string &slot_name, long timestamp_ms) noexcept;
void save_system_background_push_completed(
    const ft_string &slot_name, bool success, const ft_string &error_message, long timestamp_ms) noexcept;
bool save_system_background_poll_event(SaveSystemBackgroundEvent &out_event) noexcept;
void save_system_background_reset() noexcept;
