#pragma once

#include "libft/Libft/libft.hpp"
#include "player_profile.hpp"
#include "ui_menu.hpp"

#if defined(__has_include)
#  if __has_include(<SDL2/SDL.h>) && __has_include(<SDL2/SDL_ttf.h>)
#    define GALACTIC_HAVE_SDL2 1
#  else
#    define GALACTIC_HAVE_SDL2 0
#  endif
#else
#  define GALACTIC_HAVE_SDL2 0
#endif

#if GALACTIC_HAVE_SDL2
#    include <SDL2/SDL.h>
#    include <SDL2/SDL_ttf.h>
#else
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;

struct SDL_Color
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct SDL_Rect
{
    int x;
    int y;
    int w;
    int h;
};

struct _TTF_Font;
typedef _TTF_Font TTF_Font;
#endif

// Font utilities
TTF_Font *resolve_font(int point_size);
SDL_Texture *create_text_texture(SDL_Renderer &renderer, TTF_Font &font, const ft_string &text,
    const SDL_Color &color, SDL_Rect &out_rect);

// Main menu rendering
ft_vector<ft_menu_item> build_main_menu_items();
ft_rect                 build_main_menu_viewport();
struct MainMenuTutorialContext
{
    const ft_vector<ft_string> *tips;
    bool                        visible;

    MainMenuTutorialContext() noexcept : tips(ft_nullptr), visible(false) {}
};

struct MainMenuOverlayContext
{
    bool                        visible;
    ft_string                   heading;
    ft_vector<ft_string>        lines;
    ft_string                   footer;

    MainMenuOverlayContext() noexcept : visible(false), heading(), lines(), footer() {}
};

struct MainMenuAlertBanner
{
    bool      visible;
    bool      is_error;
    ft_string message;

    MainMenuAlertBanner() noexcept : visible(false), is_error(false), message() {}
};

enum e_main_menu_connectivity_state
{
    MAIN_MENU_CONNECTIVITY_CHECKING = 0,
    MAIN_MENU_CONNECTIVITY_ONLINE = 1,
    MAIN_MENU_CONNECTIVITY_OFFLINE = 2
};

struct MainMenuConnectivityStatus
{
    e_main_menu_connectivity_state state;
    long                           last_attempt_ms;
    long                           last_result_ms;
    int                            last_status_code;

    MainMenuConnectivityStatus() noexcept
        : state(MAIN_MENU_CONNECTIVITY_CHECKING)
        , last_attempt_ms(0)
        , last_result_ms(0)
        , last_status_code(0)
    {}
};

void main_menu_mark_connectivity_checking(MainMenuConnectivityStatus &status, long timestamp_ms) noexcept;
void main_menu_apply_connectivity_result(MainMenuConnectivityStatus &status, bool success, int status_code,
    long timestamp_ms) noexcept;
ft_string main_menu_resolve_connectivity_label(const MainMenuConnectivityStatus &status);
SDL_Color main_menu_resolve_connectivity_color(const MainMenuConnectivityStatus &status);
bool main_menu_append_connectivity_failure_log(const ft_string &host, int status_code, long timestamp_ms) noexcept;
ft_string main_menu_resolve_build_label();
bool      main_menu_can_launch_campaign(const ft_string &save_path) noexcept;
bool      main_menu_preload_commander_portrait(const ft_string &commander_name) noexcept;

void render_main_menu(SDL_Renderer &renderer, const ft_ui_menu &menu, TTF_Font *title_font, TTF_Font *menu_font,
    int window_width, int window_height, const ft_string &active_profile_name, const PlayerProfilePreferences *preferences,
    const MainMenuTutorialContext *tutorial,
    const MainMenuOverlayContext *manual, const MainMenuOverlayContext *changelog,
    const MainMenuOverlayContext *cloud_confirmation, const MainMenuConnectivityStatus *connectivity,
    const MainMenuAlertBanner *alert);

const ft_vector<ft_string> &get_main_menu_tutorial_tips();
const ft_vector<ft_string> &get_main_menu_manual_lines();
ft_vector<ft_string>        main_menu_split_patch_notes(const ft_string &body);

// Profile creation and selection flows
ft_string run_profile_entry_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    const ft_vector<ft_string> *existing_profiles, bool &out_quit_requested);
ft_string run_profile_management_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    const ft_string &current_profile, bool &out_quit_requested);
bool run_new_game_creation_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    const ft_string &commander_name, ft_string &out_created_save_path, bool &out_quit_requested);
bool run_load_game_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    const ft_string &commander_name, ft_string &out_selected_save, bool &out_quit_requested);
bool run_settings_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    PlayerProfilePreferences &preferences, bool &out_quit_requested);
bool audit_save_directory_for_errors(const ft_string &commander_name, ft_vector<ft_string> &out_errors) noexcept;
bool resolve_latest_resume_slot(const ft_string &commander_name, ft_string &out_slot_name, ft_string &out_save_path,
    ft_string &out_metadata_label, bool &out_metadata_available) noexcept;

// Profile window preference helpers
bool save_profile_preferences(SDL_Window *window, const ft_string &profile_name) noexcept;
bool save_profile_preferences(SDL_Window *window, const PlayerProfilePreferences &preferences) noexcept;
void apply_profile_preferences(SDL_Window *window, const ft_string &profile_name) noexcept;

namespace new_game_flow_testing
{
    unsigned int max_save_name_length() noexcept;
    bool         is_character_allowed(char character) noexcept;
    bool         append_character(ft_string &save_name, char character) noexcept;
    void         remove_last_character(ft_string &save_name) noexcept;
    bool         validate_save_name(const ft_string &save_name) noexcept;
    ft_string    compute_save_file_path(const ft_string &commander_name, const ft_string &save_name);
    bool         create_save_file(const ft_string &commander_name, const ft_string &save_name, ft_string &out_error) noexcept;
    bool         create_save_file_with_path(const ft_string &commander_name, const ft_string &save_name,
                ft_string &out_save_path, ft_string &out_error) noexcept;
}

namespace profile_preferences_testing
{
    PlayerProfilePreferences build_window_preference_snapshot(
        const ft_string &profile_name, unsigned int window_width, unsigned int window_height) noexcept;
}

namespace main_menu_testing
{
    ft_string resolve_active_description(const ft_ui_menu &menu);
    ft_string resolve_navigation_hint(const ft_ui_menu &menu, const PlayerProfilePreferences *preferences = ft_nullptr);
    ft_vector<ft_string> collect_tutorial_tips();
    ft_vector<ft_string> collect_manual_lines();
    ft_string resolve_connectivity_label(const MainMenuConnectivityStatus &status);
    SDL_Color resolve_connectivity_color(const MainMenuConnectivityStatus &status);
    void      mark_connectivity_checking(MainMenuConnectivityStatus &status, long timestamp_ms);
    void      apply_connectivity_result(MainMenuConnectivityStatus &status, bool success, int status_code,
             long timestamp_ms);
    bool      append_connectivity_failure_log(const ft_string &host, int status_code, long timestamp_ms,
                const ft_string &log_path);
    ft_string resolve_build_label();
    ft_vector<ft_string> split_patch_note_lines(const ft_string &body);
    void      reset_commander_portrait_cache();
    bool      commander_portrait_attempted(const ft_string &commander_name);
    bool      commander_portrait_loaded(const ft_string &commander_name);
    size_t    commander_portrait_cached_size(const ft_string &commander_name);
    ft_string resolve_cached_portrait_path(const ft_string &commander_name);
    ft_string resolve_commander_portrait_filename(const ft_string &commander_name);
}

namespace settings_flow_testing
{
    unsigned int clamp_ui_scale(unsigned int value) noexcept;
    unsigned int increment_ui_scale(unsigned int value) noexcept;
    unsigned int decrement_ui_scale(unsigned int value) noexcept;
    unsigned int clamp_combat_speed(unsigned int value) noexcept;
    unsigned int increment_combat_speed(unsigned int value) noexcept;
    unsigned int decrement_combat_speed(unsigned int value) noexcept;
    unsigned int clamp_music_volume(unsigned int value) noexcept;
    unsigned int increment_music_volume(unsigned int value) noexcept;
    unsigned int decrement_music_volume(unsigned int value) noexcept;
    unsigned int clamp_effects_volume(unsigned int value) noexcept;
    unsigned int increment_effects_volume(unsigned int value) noexcept;
    unsigned int decrement_effects_volume(unsigned int value) noexcept;
    unsigned int clamp_brightness(unsigned int value) noexcept;
    unsigned int increment_brightness(unsigned int value) noexcept;
    unsigned int decrement_brightness(unsigned int value) noexcept;
    unsigned int clamp_contrast(unsigned int value) noexcept;
    unsigned int increment_contrast(unsigned int value) noexcept;
    unsigned int decrement_contrast(unsigned int value) noexcept;
    unsigned int toggle_lore_anchor(unsigned int anchor) noexcept;
    ft_string    format_ui_scale_option(unsigned int value);
    ft_string    format_combat_speed_option(unsigned int value);
    ft_string    format_music_volume_option(unsigned int value);
    ft_string    format_effects_volume_option(unsigned int value);
    ft_string    format_brightness_option(unsigned int value);
    ft_string    format_contrast_option(unsigned int value);
    ft_string    format_lore_anchor_option(unsigned int anchor);
    bool         toggle_accessibility_preset(bool enabled) noexcept;
    ft_string    format_accessibility_preset_option(bool enabled);
    bool         toggle_colorblind_palette(bool enabled) noexcept;
    ft_string    format_colorblind_palette_option(bool enabled);
}

namespace load_game_flow_testing
{
    ft_vector<ft_string> collect_save_slot_names(const ft_string &commander_name);
    ft_string             resolve_save_file_path(const ft_string &commander_name, const ft_string &slot_name);
    bool                  delete_save_slot(const ft_string &commander_name, const ft_string &slot_name, ft_string &out_error);
    bool                  rename_save_slot(
        const ft_string &commander_name, const ft_string &old_name, const ft_string &new_name, ft_string &out_error);
    ft_vector<ft_string> collect_save_slot_metadata_labels(const ft_string &commander_name);
    ft_vector<ft_string> collect_save_slot_errors(const ft_string &commander_name);
}
