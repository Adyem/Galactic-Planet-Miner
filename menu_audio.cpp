#include "main_menu_system.hpp"

#include "libft/Template/vector.hpp"

namespace
{
    bool g_menu_audio_muted = false;
    unsigned int g_menu_music_volume_percent = 100U;
    unsigned int g_menu_effects_volume_percent = 100U;
    ft_vector<MainMenuAudioEvent> g_menu_audio_queue;

    unsigned int clamp_volume_percent(unsigned int value) noexcept
    {
        if (value < PLAYER_PROFILE_VOLUME_MIN_PERCENT)
            return PLAYER_PROFILE_VOLUME_MIN_PERCENT;
        if (value > PLAYER_PROFILE_VOLUME_MAX_PERCENT)
            return PLAYER_PROFILE_VOLUME_MAX_PERCENT;
        return value;
    }
}

void main_menu_audio_set_global_mute(bool muted) noexcept
{
    g_menu_audio_muted = muted;
}

bool main_menu_audio_is_globally_muted() noexcept
{
    return g_menu_audio_muted;
}

void main_menu_audio_apply_preferences(const PlayerProfilePreferences &preferences) noexcept
{
    g_menu_music_volume_percent = clamp_volume_percent(preferences.music_volume_percent);
    g_menu_effects_volume_percent = clamp_volume_percent(preferences.effects_volume_percent);
}

void main_menu_audio_set_effects_volume(unsigned int value) noexcept
{
    g_menu_effects_volume_percent = clamp_volume_percent(value);
}

unsigned int main_menu_audio_get_effects_volume() noexcept
{
    return g_menu_effects_volume_percent;
}

unsigned int main_menu_audio_get_music_volume() noexcept
{
    return g_menu_music_volume_percent;
}

void main_menu_audio_queue_event(e_main_menu_audio_cue cue) noexcept
{
    if (g_menu_audio_muted)
        return;
    if (g_menu_effects_volume_percent == 0U)
        return;

    MainMenuAudioEvent event;
    event.cue = cue;
    event.volume_percent = g_menu_effects_volume_percent;
    g_menu_audio_queue.push_back(event);
}

bool main_menu_audio_poll_event(MainMenuAudioEvent &out_event) noexcept
{
    if (g_menu_audio_queue.empty())
        return false;

    out_event = g_menu_audio_queue[0];
    g_menu_audio_queue.erase(g_menu_audio_queue.begin());
    return true;
}

void main_menu_audio_reset() noexcept
{
    g_menu_audio_queue.clear();
}
