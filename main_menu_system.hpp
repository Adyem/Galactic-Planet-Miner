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
int main_menu_resolve_title_font_points() noexcept;
int main_menu_resolve_menu_font_points() noexcept;

// Main menu rendering
struct MainMenuPalette
{
    SDL_Color background;
    SDL_Color title;
    SDL_Color profile;
    SDL_Color button_idle;
    SDL_Color button_hover;
    SDL_Color button_selected;
    SDL_Color button_disabled;
    SDL_Color button_disabled_hover;
    SDL_Color button_border_enabled;
    SDL_Color button_border_disabled;
    SDL_Color button_text_enabled;
    SDL_Color button_text_disabled;
    SDL_Color description;
    SDL_Color hint;
    SDL_Color build;
    SDL_Color alert_text_error;
    SDL_Color alert_text_info;
    SDL_Color alert_background_error;
    SDL_Color alert_border_error;
    SDL_Color alert_background_info;
    SDL_Color alert_border_info;
    SDL_Color overlay_heading;
    SDL_Color overlay_line;
    SDL_Color overlay_footer;
    SDL_Color overlay_background;
    SDL_Color overlay_border;
    SDL_Color tutorial_title;
    SDL_Color tutorial_primary;
    SDL_Color tutorial_secondary;
    SDL_Color tutorial_background;
    SDL_Color tutorial_border;
    SDL_Color autosave_icon_idle;
    SDL_Color autosave_icon_in_progress;
    SDL_Color autosave_icon_success;
    SDL_Color autosave_icon_failure;
    SDL_Color autosave_text_in_progress;
    SDL_Color autosave_text_success;
    SDL_Color autosave_text_failure;
    SDL_Color sync_heading;
    SDL_Color sync_idle;
    SDL_Color sync_pending;
    SDL_Color sync_success;
    SDL_Color sync_failure;
    SDL_Color sync_background;
    SDL_Color sync_border;
    SDL_Color achievements_heading;
    SDL_Color achievements_primary;
    SDL_Color achievements_secondary;
    SDL_Color achievements_background;
    SDL_Color achievements_border;
    SDL_Color performance_heading;
    SDL_Color performance_value;
    SDL_Color performance_warning;
    SDL_Color performance_background;
    SDL_Color performance_border;

    MainMenuPalette() noexcept
        : background()
        , title()
        , profile()
        , button_idle()
        , button_hover()
        , button_selected()
        , button_disabled()
        , button_disabled_hover()
        , button_border_enabled()
        , button_border_disabled()
        , button_text_enabled()
        , button_text_disabled()
        , description()
        , hint()
        , build()
        , alert_text_error()
        , alert_text_info()
        , alert_background_error()
        , alert_border_error()
        , alert_background_info()
        , alert_border_info()
        , overlay_heading()
        , overlay_line()
        , overlay_footer()
        , overlay_background()
        , overlay_border()
        , tutorial_title()
        , tutorial_primary()
        , tutorial_secondary()
        , tutorial_background()
        , tutorial_border()
        , autosave_icon_idle()
        , autosave_icon_in_progress()
        , autosave_icon_success()
        , autosave_icon_failure()
        , autosave_text_in_progress()
        , autosave_text_success()
        , autosave_text_failure()
        , sync_heading()
        , sync_idle()
        , sync_pending()
        , sync_success()
        , sync_failure()
        , sync_background()
        , sync_border()
        , achievements_heading()
        , achievements_primary()
        , achievements_secondary()
        , achievements_background()
        , achievements_border()
        , performance_heading()
        , performance_value()
        , performance_warning()
        , performance_background()
        , performance_border()
    {}
};

MainMenuPalette resolve_main_menu_palette(
    bool colorblind_enabled, unsigned int brightness_percent, unsigned int contrast_percent);
ft_vector<ft_menu_item> build_main_menu_items(const PlayerProfilePreferences *preferences = ft_nullptr);
void main_menu_apply_resume_state(ft_vector<ft_menu_item> &items, bool resume_enabled,
    const ft_string &slot_label, const ft_string &metadata_label, bool metadata_known);
ft_string main_menu_format_resume_description(const ft_string &slot_label,
    const ft_string &metadata_label, bool metadata_known);
ft_string main_menu_resolve_description(const ft_ui_menu &menu);
ft_string main_menu_resolve_navigation_hint(
    const ft_ui_menu &menu, const PlayerProfilePreferences *preferences);
ft_rect   build_main_menu_viewport();
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

struct MainMenuCrashReport
{
    bool      available;
    long      timestamp_ms;
    ft_string summary;
    ft_string details_path;

    MainMenuCrashReport() noexcept : available(false), timestamp_ms(0L), summary(), details_path() {}
};

struct MainMenuAlertBanner
{
    bool      visible;
    bool      is_error;
    ft_string message;

    MainMenuAlertBanner() noexcept : visible(false), is_error(false), message() {}
};

enum e_main_menu_sync_state
{
    MAIN_MENU_SYNC_IDLE = 0,
    MAIN_MENU_SYNC_CHECKING = 1,
    MAIN_MENU_SYNC_SUCCESS = 2,
    MAIN_MENU_SYNC_FAILURE = 3
};

enum e_main_menu_sync_channel
{
    MAIN_MENU_SYNC_CHANNEL_CONVOYS = 0,
    MAIN_MENU_SYNC_CHANNEL_LEADERBOARDS = 1
};

struct MainMenuSyncEntry
{
    e_main_menu_sync_state state;
    long                   last_attempt_ms;
    long                   last_success_ms;
    long                   last_failure_ms;
    int                    last_status_code;

    MainMenuSyncEntry() noexcept
        : state(MAIN_MENU_SYNC_IDLE)
        , last_attempt_ms(0)
        , last_success_ms(0)
        , last_failure_ms(0)
        , last_status_code(0)
    {}
};

struct MainMenuSyncStatus
{
    MainMenuSyncEntry convoys;
    MainMenuSyncEntry leaderboards;

    MainMenuSyncStatus() noexcept : convoys(), leaderboards() {}
};

enum e_main_menu_autosave_state
{
    MAIN_MENU_AUTOSAVE_IDLE = 0,
    MAIN_MENU_AUTOSAVE_IN_PROGRESS = 1,
    MAIN_MENU_AUTOSAVE_SUCCEEDED = 2,
    MAIN_MENU_AUTOSAVE_FAILED = 3
};

struct MainMenuAutosaveStatus
{
    e_main_menu_autosave_state state;
    ft_string                  active_slot;
    ft_string                  last_error;
    long                       last_change_ms;

    MainMenuAutosaveStatus() noexcept
        : state(MAIN_MENU_AUTOSAVE_IDLE)
        , active_slot()
        , last_error()
        , last_change_ms(0)
    {}
};

enum e_main_menu_audio_cue
{
    MAIN_MENU_AUDIO_CUE_NAVIGATE = 0,
    MAIN_MENU_AUDIO_CUE_CONFIRM = 1,
    MAIN_MENU_AUDIO_CUE_ERROR = 2
};

struct MainMenuAudioEvent
{
    e_main_menu_audio_cue cue;
    unsigned int          volume_percent;

    MainMenuAudioEvent() noexcept : cue(MAIN_MENU_AUDIO_CUE_NAVIGATE), volume_percent(0U) {}
};

struct MainMenuPerformanceStats
{
    bool has_fps;
    unsigned int fps_value;
    long fps_last_update_ms;
    unsigned int fps_frame_count;
    long fps_accumulated_ms;

    bool latency_sampled;
    bool latency_successful;
    bool latency_pending;
    long latency_ms;
    long latency_last_update_ms;

    MainMenuPerformanceStats() noexcept
        : has_fps(false)
        , fps_value(0U)
        , fps_last_update_ms(0L)
        , fps_frame_count(0U)
        , fps_accumulated_ms(0L)
        , latency_sampled(false)
        , latency_successful(false)
        , latency_pending(false)
        , latency_ms(0L)
        , latency_last_update_ms(0L)
    {}
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

struct MainMenuAchievementsSummary
{
    unsigned int total_count;
    unsigned int completed_count;
    ft_string    highlight_label;
    ft_string    progress_note;
    bool         has_highlight;
    bool         has_progress_note;

    MainMenuAchievementsSummary() noexcept
        : total_count(0U)
        , completed_count(0U)
        , highlight_label()
        , progress_note()
        , has_highlight(false)
        , has_progress_note(false)
    {}
};

void main_menu_mark_connectivity_checking(MainMenuConnectivityStatus &status, long timestamp_ms) noexcept;
void main_menu_apply_connectivity_result(MainMenuConnectivityStatus &status, bool success, int status_code,
    long timestamp_ms) noexcept;
ft_string main_menu_resolve_connectivity_label(const MainMenuConnectivityStatus &status);
SDL_Color main_menu_resolve_connectivity_color(const MainMenuConnectivityStatus &status);
bool main_menu_append_connectivity_failure_log(const ft_string &host, int status_code, long timestamp_ms) noexcept;
ft_string main_menu_resolve_build_label();
void main_menu_performance_record_frame(
    MainMenuPerformanceStats &stats, long frame_start_ms, long frame_end_ms) noexcept;
void main_menu_performance_begin_latency_sample(MainMenuPerformanceStats &stats, long timestamp_ms) noexcept;
void main_menu_performance_complete_latency_sample(MainMenuPerformanceStats &stats, bool success, long duration_ms,
    long timestamp_ms) noexcept;
ft_string main_menu_format_performance_fps_label(const MainMenuPerformanceStats &stats);
ft_string main_menu_format_performance_latency_label(const MainMenuPerformanceStats &stats);
void      main_menu_audio_set_global_mute(bool muted) noexcept;
bool      main_menu_audio_is_globally_muted() noexcept;
void      main_menu_audio_apply_preferences(const PlayerProfilePreferences &preferences) noexcept;
void      main_menu_audio_set_effects_volume(unsigned int value) noexcept;
unsigned int main_menu_audio_get_effects_volume() noexcept;
unsigned int main_menu_audio_get_music_volume() noexcept;
void      main_menu_audio_queue_event(e_main_menu_audio_cue cue) noexcept;
bool      main_menu_audio_poll_event(MainMenuAudioEvent &out_event) noexcept;
void      main_menu_audio_reset() noexcept;
bool      main_menu_can_launch_campaign(const ft_string &save_path) noexcept;
bool      main_menu_preload_commander_portrait(const ft_string &commander_name) noexcept;
unsigned int main_menu_resolve_total_achievements() noexcept;
MainMenuAchievementsSummary main_menu_build_achievements_summary(const PlayerProfilePreferences *preferences) noexcept;
ft_string main_menu_format_achievements_completion_label(const MainMenuAchievementsSummary &summary);
ft_vector<ft_string> main_menu_collect_achievement_lines(const MainMenuAchievementsSummary &summary);
void      main_menu_mark_autosave_in_progress(MainMenuAutosaveStatus &status, const ft_string &slot_name,
         long timestamp_ms) noexcept;
void      main_menu_mark_autosave_result(MainMenuAutosaveStatus &status, bool success, const ft_string &slot_name,
         const ft_string &error_message, long timestamp_ms) noexcept;
void      main_menu_autosave_tick(MainMenuAutosaveStatus &status, long timestamp_ms) noexcept;
bool      main_menu_autosave_is_visible(const MainMenuAutosaveStatus &status, long timestamp_ms) noexcept;
ft_string main_menu_resolve_autosave_label(const MainMenuAutosaveStatus &status);
void      main_menu_sync_begin(MainMenuSyncStatus &status, e_main_menu_sync_channel channel, long timestamp_ms) noexcept;
void      main_menu_sync_apply(MainMenuSyncStatus &status, e_main_menu_sync_channel channel, bool success,
         int status_code, long timestamp_ms) noexcept;
ft_string main_menu_resolve_sync_entry_label(const MainMenuSyncStatus &status, e_main_menu_sync_channel channel) noexcept;
bool      main_menu_load_crash_report(const ft_string &log_path, MainMenuCrashReport &out_report) noexcept;
bool      main_menu_clear_crash_report(const ft_string &log_path) noexcept;
bool      main_menu_append_crash_cleanup_failure_log(const ft_string &log_path, long timestamp_ms) noexcept;
ft_string main_menu_format_crash_submission_payload(const MainMenuCrashReport &report) noexcept;
ft_string main_menu_format_crash_metric_payload(const MainMenuCrashReport &report, bool submission_success,
    int status_code, const ft_string &response_body) noexcept;
typedef bool (*MainMenuCrashMetricSubmitter)(
    const ft_string &host, const ft_string &path, const ft_string &payload, int &out_status_code);
void main_menu_crash_metrics_enqueue(ft_vector<ft_string> &queue, const ft_string &payload);
bool main_menu_crash_metrics_flush(ft_vector<ft_string> &queue, const ft_string &host, const ft_string &path,
    MainMenuCrashMetricSubmitter submitter) noexcept;
bool main_menu_load_crash_metric_queue(const ft_string &commander_name, ft_vector<ft_string> &out_queue) noexcept;
bool main_menu_save_crash_metric_queue(
    const ft_string &commander_name, const ft_vector<ft_string> &queue) noexcept;
void      main_menu_build_crash_prompt_overlay(const MainMenuCrashReport &report,
         const MainMenuConnectivityStatus &connectivity, bool cleanup_retry_available,
         MainMenuOverlayContext &out_overlay) noexcept;

void main_menu_render_overlay(SDL_Renderer &renderer, TTF_Font *menu_font, int output_width, int output_height,
    const MainMenuOverlayContext *overlay, const MainMenuPalette &palette);
void main_menu_render_tutorial_overlay(SDL_Renderer &renderer, TTF_Font *menu_font, int output_width,
    const MainMenuTutorialContext *tutorial, const MainMenuPalette &palette);
void main_menu_render_achievements(SDL_Renderer &renderer, TTF_Font *menu_font,
    const MainMenuAchievementsSummary *achievements, int output_width, int output_height,
    const MainMenuPalette &palette);
void main_menu_render_autosave(SDL_Renderer &renderer, TTF_Font *menu_font, const MainMenuAutosaveStatus *autosave,
    int output_height, const MainMenuPalette &palette);

void render_main_menu(SDL_Renderer &renderer, const ft_ui_menu &menu, TTF_Font *title_font, TTF_Font *menu_font,
    int window_width, int window_height, const ft_string &active_profile_name, const PlayerProfilePreferences *preferences,
    const MainMenuTutorialContext *tutorial,
    const MainMenuOverlayContext *manual, const MainMenuOverlayContext *changelog,
    const MainMenuOverlayContext *cloud_confirmation, const MainMenuOverlayContext *crash_prompt,
    const MainMenuConnectivityStatus *connectivity,
    const MainMenuSyncStatus *sync_status, const MainMenuAchievementsSummary *achievements,
    const MainMenuAutosaveStatus *autosave, const MainMenuPerformanceStats *performance,
    const MainMenuAlertBanner *alert);

const ft_vector<ft_string> &get_main_menu_tutorial_tips();
const ft_vector<ft_string> &get_main_menu_manual_lines();
ft_vector<ft_string>        main_menu_split_patch_notes(const ft_string &body);

namespace new_game_flow
{
    bool is_save_character_allowed(char character) noexcept;
    bool append_save_character(ft_string &save_name, char character) noexcept;
    void remove_last_save_character(ft_string &save_name) noexcept;
    bool save_name_is_valid(const ft_string &save_name) noexcept;
    ft_string build_save_file_path(const ft_string &commander_name, const ft_string &save_name);
    bool create_new_game_save(
        const ft_string &commander_name,
        const ft_string &save_name,
        ft_string &out_error,
        ft_string &out_save_path) noexcept;
#if GALACTIC_HAVE_SDL2
    void render_new_game_screen(SDL_Renderer &renderer, TTF_Font *title_font, TTF_Font *menu_font,
        const ft_string &save_name, const ft_string &status_message, bool status_is_error);
#endif
}

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
    bool      autosave_is_visible(const MainMenuAutosaveStatus &status, long timestamp_ms) noexcept;
    ft_string resolve_sync_entry_label(const MainMenuSyncStatus &status, e_main_menu_sync_channel channel);
    void      begin_sync(MainMenuSyncStatus &status, e_main_menu_sync_channel channel, long timestamp_ms);
    void      apply_sync(MainMenuSyncStatus &status, e_main_menu_sync_channel channel, bool success, int status_code,
             long timestamp_ms);
    MainMenuAchievementsSummary build_achievements_summary(const PlayerProfilePreferences &preferences) noexcept;
    ft_string                   format_achievements_completion(const MainMenuAchievementsSummary &summary);
    ft_vector<ft_string>        collect_achievement_lines(const MainMenuAchievementsSummary &summary);
    ft_string                   format_crash_submission_payload(const MainMenuCrashReport &report);
    ft_string                   format_crash_metric_payload(
        const MainMenuCrashReport &report, bool submission_success, int status_code, const ft_string &response_body);
    void                        build_crash_prompt_overlay(const MainMenuCrashReport &report,
                               const MainMenuConnectivityStatus &connectivity, bool cleanup_retry_available,
                               MainMenuOverlayContext &out_overlay);
    void                        performance_record_frame(MainMenuPerformanceStats &stats, long frame_start_ms, long frame_end_ms);
    void                        performance_begin_latency(MainMenuPerformanceStats &stats, long timestamp_ms);
    void                        performance_complete_latency(
        MainMenuPerformanceStats &stats, bool success, long duration_ms, long timestamp_ms);
    ft_string                   format_performance_fps(const MainMenuPerformanceStats &stats);
    ft_string                   format_performance_latency(const MainMenuPerformanceStats &stats);
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
    bool         toggle_experimental_features(bool enabled) noexcept;
    ft_string    format_experimental_features_option(bool enabled);
    bool         toggle_analytics_opt_in(bool enabled) noexcept;
    ft_string    format_analytics_opt_in_option(bool enabled);
    ft_string    format_controller_up_option(int button);
    ft_string    format_controller_down_option(int button);
    ft_string    format_controller_left_option(int button);
    ft_string    format_controller_right_option(int button);
    ft_string    format_controller_confirm_option(int button);
    ft_string    format_controller_cancel_option(int button);
    ft_string    format_controller_delete_option(int button);
    ft_string    format_controller_rename_option(int button);
    int          increment_controller_button_option(int button) noexcept;
    int          decrement_controller_button_option(int button) noexcept;
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
