#ifndef PLAYER_PROFILE_HPP
#define PLAYER_PROFILE_HPP

#include "libft/CPP_class/class_string_class.hpp"
#include "libft/Template/vector.hpp"

enum e_player_lore_panel_anchor
{
    PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT = 1,
    PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_LEFT = 2
};

static const unsigned int PLAYER_PROFILE_UI_SCALE_MIN_PERCENT = 75U;
static const unsigned int PLAYER_PROFILE_UI_SCALE_MAX_PERCENT = 150U;
static const unsigned int PLAYER_PROFILE_COMBAT_SPEED_MIN_PERCENT = 50U;
static const unsigned int PLAYER_PROFILE_COMBAT_SPEED_MAX_PERCENT = 200U;
static const unsigned int PLAYER_PROFILE_VOLUME_MIN_PERCENT = 0U;
static const unsigned int PLAYER_PROFILE_VOLUME_MAX_PERCENT = 100U;

struct PlayerProfilePreferences
{
    ft_string    commander_name;
    unsigned int window_width;
    unsigned int window_height;
    unsigned int ui_scale_percent;
    unsigned int combat_speed_percent;
    unsigned int music_volume_percent;
    unsigned int effects_volume_percent;
    unsigned int lore_panel_anchor;
    bool         menu_tutorial_seen;

    PlayerProfilePreferences() noexcept
        : commander_name(),
          window_width(1280U),
          window_height(720U),
          ui_scale_percent(100U),
          combat_speed_percent(100U),
          music_volume_percent(100U),
          effects_volume_percent(100U),
          lore_panel_anchor(PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT),
          menu_tutorial_seen(false)
    {}
};

bool player_profile_load_or_create(PlayerProfilePreferences &out_preferences, const ft_string &commander_name) noexcept;
bool player_profile_save(const PlayerProfilePreferences &preferences) noexcept;
ft_string player_profile_resolve_path(const ft_string &commander_name) noexcept;
bool player_profile_list(ft_vector<ft_string> &out_profiles) noexcept;
bool player_profile_delete(const ft_string &commander_name) noexcept;
ft_string player_profile_resolve_save_directory(const ft_string &commander_name) noexcept;

#endif
