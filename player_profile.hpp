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

static const int PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_UP = 1073741906;    // SDLK_UP
static const int PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DOWN = 1073741905;  // SDLK_DOWN
static const int PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_LEFT = 1073741904;  // SDLK_LEFT
static const int PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_RIGHT = 1073741903; // SDLK_RIGHT
static const int PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CONFIRM = 13;       // SDLK_RETURN
static const int PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CANCEL = 27;        // SDLK_ESCAPE
static const int PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DELETE = 8;         // SDLK_BACKSPACE
static const int PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_RENAME = 114;       // SDLK_r
static const int PLAYER_PROFILE_INPUT_DEVICE_NONE = 0;
static const int PLAYER_PROFILE_INPUT_DEVICE_MOUSE = 1;
static const int PLAYER_PROFILE_INPUT_DEVICE_KEYBOARD = 2;
static const int PLAYER_PROFILE_INPUT_DEVICE_GAMEPAD = 3;

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
    bool         accessibility_preset_enabled;
    int          hotkey_menu_up;
    int          hotkey_menu_down;
    int          hotkey_menu_left;
    int          hotkey_menu_right;
    int          hotkey_menu_confirm;
    int          hotkey_menu_cancel;
    int          hotkey_menu_delete;
    int          hotkey_menu_rename;
    int          last_menu_input_device;

    PlayerProfilePreferences() noexcept
        : commander_name(),
          window_width(1280U),
          window_height(720U),
          ui_scale_percent(100U),
          combat_speed_percent(100U),
          music_volume_percent(100U),
          effects_volume_percent(100U),
          lore_panel_anchor(PLAYER_PREFERENCE_LORE_PANEL_ANCHOR_RIGHT),
          menu_tutorial_seen(false),
          accessibility_preset_enabled(false),
          hotkey_menu_up(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_UP),
          hotkey_menu_down(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DOWN),
          hotkey_menu_left(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_LEFT),
          hotkey_menu_right(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_RIGHT),
          hotkey_menu_confirm(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CONFIRM),
          hotkey_menu_cancel(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CANCEL),
          hotkey_menu_delete(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DELETE),
          hotkey_menu_rename(PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_RENAME),
          last_menu_input_device(PLAYER_PROFILE_INPUT_DEVICE_KEYBOARD)
    {}

    void reset_hotkeys() noexcept
    {
        this->hotkey_menu_up = PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_UP;
        this->hotkey_menu_down = PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DOWN;
        this->hotkey_menu_left = PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_LEFT;
        this->hotkey_menu_right = PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_RIGHT;
        this->hotkey_menu_confirm = PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CONFIRM;
        this->hotkey_menu_cancel = PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_CANCEL;
        this->hotkey_menu_delete = PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_DELETE;
        this->hotkey_menu_rename = PLAYER_PROFILE_DEFAULT_HOTKEY_MENU_RENAME;
    }
};

bool player_profile_load_or_create(PlayerProfilePreferences &out_preferences, const ft_string &commander_name) noexcept;
bool player_profile_save(const PlayerProfilePreferences &preferences) noexcept;
ft_string player_profile_resolve_path(const ft_string &commander_name) noexcept;
bool player_profile_list(ft_vector<ft_string> &out_profiles) noexcept;
bool player_profile_delete(const ft_string &commander_name) noexcept;
ft_string player_profile_resolve_save_directory(const ft_string &commander_name) noexcept;

#endif
