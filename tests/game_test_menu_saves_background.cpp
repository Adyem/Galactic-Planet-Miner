#include "game_test_menu_shared.hpp"

int verify_save_system_background_queue()
{
    save_system_background_reset();

    SaveSystemBackgroundEvent event;
    FT_ASSERT(!save_system_background_poll_event(event));

    const ft_string slot("alpha");
    save_system_background_push_started(slot, 100L);
    save_system_background_push_completed(slot, false, ft_string("write failure"), 200L);

    FT_ASSERT(save_system_background_poll_event(event));
    FT_ASSERT_EQ(SaveSystemBackgroundEvent::SAVE_SYSTEM_BACKGROUND_EVENT_STARTED, event.type);
    FT_ASSERT_EQ(slot, event.slot_name);
    FT_ASSERT(!event.success);
    FT_ASSERT_EQ(100L, event.timestamp_ms);

    FT_ASSERT(save_system_background_poll_event(event));
    FT_ASSERT_EQ(SaveSystemBackgroundEvent::SAVE_SYSTEM_BACKGROUND_EVENT_COMPLETED, event.type);
    FT_ASSERT_EQ(slot, event.slot_name);
    FT_ASSERT(!event.success);
    FT_ASSERT_EQ(ft_string("write failure"), event.error_message);
    FT_ASSERT_EQ(200L, event.timestamp_ms);

    FT_ASSERT(!save_system_background_poll_event(event));

    return 1;
}
