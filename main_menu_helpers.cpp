#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/Time/time.hpp"

#if GALACTIC_HAVE_SDL2
#    include <SDL2/SDL.h>
#    include <SDL2/SDL_ttf.h>
#endif

#if !defined(GALACTIC_HAVE_SDL2) || !GALACTIC_HAVE_SDL2
#    error "main_menu_helpers.cpp requires GALACTIC_HAVE_SDL2"
#endif
int main_menu_resolve_title_font_points() noexcept
{
    return 48;
}

int main_menu_resolve_menu_font_points() noexcept
{
    return 28;
}

namespace
{
    const ft_menu_item *menu_item_from_index(const ft_ui_menu &menu, int index)
    {
        if (index < 0)
            return ft_nullptr;

        const ft_vector<ft_menu_item> &items = menu.get_items();
        const size_t                  size = items.size();
        const size_t                  converted_index = static_cast<size_t>(index);

        if (converted_index >= size)
            return ft_nullptr;

        return &items[converted_index];
    }

    void destroy_renderer(SDL_Renderer *renderer)
    {
        if (renderer != ft_nullptr)
            SDL_DestroyRenderer(renderer);
    }

    void destroy_window(SDL_Window *window)
    {
        if (window != ft_nullptr)
            SDL_DestroyWindow(window);
    }
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
        return 1;

    if (TTF_Init() != 0)
    {
        SDL_Quit();
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Galactic Planet Miner",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        static_cast<int>(app_constants::kWindowWidth),
        static_cast<int>(app_constants::kWindowHeight),
        SDL_WINDOW_SHOWN);

    if (window == ft_nullptr)
    {
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == ft_nullptr)
    {
        destroy_window(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    TTF_Font *title_font = resolve_font(main_menu_resolve_title_font_points());
    TTF_Font *menu_font = resolve_font(main_menu_resolve_menu_font_points());

    auto refresh_fonts = [&]() {
        title_font = resolve_font(main_menu_resolve_title_font_points());
        menu_font = resolve_font(main_menu_resolve_menu_font_points());
    };

    bool quit_requested = false;
    ft_vector<ft_string> available_profiles;
    player_profile_list(available_profiles);

    ft_string active_profile_name;
    if (available_profiles.empty())
    {
        ft_string created_profile = run_profile_entry_flow(window, renderer, title_font, menu_font, &available_profiles,
            quit_requested);
        if (quit_requested || created_profile.empty())
        {
            destroy_renderer(renderer);
            destroy_window(window);
            TTF_Quit();
            SDL_Quit();
            return 0;
        }

        active_profile_name = created_profile;
        player_profile_list(available_profiles);
    }
    else
    {
        active_profile_name = available_profiles[0];
    }

    if (active_profile_name.empty())
    {
        destroy_renderer(renderer);
        destroy_window(window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    PlayerProfilePreferences active_preferences;
    if (!player_profile_load_or_create(active_preferences, active_profile_name))
    {
        active_preferences = PlayerProfilePreferences();
        active_preferences.commander_name = active_profile_name;
    }

    apply_profile_preferences(window, active_profile_name);
    refresh_fonts();
    main_menu_audio_apply_preferences(active_preferences);

    ft_ui_menu              menu;
    ft_vector<ft_menu_item> menu_items = build_main_menu_items(&active_preferences);
    menu.set_viewport_bounds(build_main_menu_viewport());

    auto rebuild_menu_items = [&](const ft_string &preferred_selection) {
        menu.set_items(menu_items);
        menu.set_viewport_bounds(build_main_menu_viewport());
        if (!preferred_selection.empty())
        {
            for (size_t index = 0; index < menu_items.size(); ++index)
            {
                if (menu_items[index].identifier == preferred_selection && menu_items[index].enabled)
                {
                    menu.set_selected_index(static_cast<int>(index));
                    break;
                }
            }
        }
        e_ft_input_device seeded_device
            = static_cast<e_ft_input_device>(active_preferences.last_menu_input_device);
        if (seeded_device != FT_INPUT_DEVICE_NONE)
            menu.set_active_device(seeded_device);
    };

    auto refresh_menu_selection = [&]() {
        ft_string preferred_selection;
        const ft_menu_item *selected_item = menu.get_selected_item();
        if (selected_item != ft_nullptr)
            preferred_selection = selected_item->identifier;
        rebuild_menu_items(preferred_selection);
    };

    rebuild_menu_items(ft_string());

    const ft_vector<ft_string> &tutorial_tips = get_main_menu_tutorial_tips();
    MainMenuTutorialContext     tutorial_context;
    tutorial_context.tips = &tutorial_tips;
    tutorial_context.visible = !active_preferences.menu_tutorial_seen;
    MainMenuOverlayContext      changelog_context;
    MainMenuOverlayContext      manual_context;
    MainMenuOverlayContext      clear_cloud_context;
    MainMenuOverlayContext      crash_prompt_context;
    MainMenuAlertBanner         alert_banner;
    MainMenuAutosaveStatus      autosave_status;
    MainMenuSyncStatus          sync_status;
    MainMenuPerformanceStats    performance_stats;
    MainMenuAchievementsSummary achievements_summary;
    MainMenuCrashReport         crash_report_status;
    bool                        crash_prompt_dismissed = false;
    long                        crash_prompt_last_timestamp = 0L;
    ft_string                   crash_prompt_last_summary;
    ft_string                   crash_prompt_last_details;
    const long                  crash_prompt_poll_interval_ms = 5000L;
    long                        next_crash_prompt_check_ms = 0L;
    bool                        crash_prompt_focus_pending = false;

    auto refresh_achievement_summary = [&]() {
        achievements_summary = main_menu_build_achievements_summary(&active_preferences);
    };
    refresh_achievement_summary();

    bool      resume_available = false;
    bool      resume_metadata_known = false;
    ft_string resume_slot_label;
    ft_string resume_metadata_label;
    ft_string resume_save_path;

    auto update_resume_menu_entry = [&]() {
        const bool resume_enabled = resume_available && !resume_save_path.empty();
        main_menu_apply_resume_state(
            menu_items, resume_enabled, resume_slot_label, resume_metadata_label, resume_metadata_known);
        refresh_menu_selection();
    };

    const ft_string backend_host("127.0.0.1:8080");
    const ft_string backend_path("/");
    const ft_string backend_patch_notes_path("/patch-notes/latest");
    const ft_string backend_clear_cloud_path("/cloud-data/clear");
    const ft_string backend_crash_report_path("/crash-report");
    const ft_string crash_report_log_path("crash_reports/pending_report.log");
    MainMenuConnectivityStatus connectivity_status;
    const long connectivity_interval_ms = 7000;
    long       next_connectivity_check_ms = 0;

    auto refresh_save_alert = [&]() {
        if (active_profile_name.empty())
        {
            alert_banner.visible = false;
            alert_banner.is_error = false;
            alert_banner.message.clear();
            resume_available = false;
            resume_metadata_known = false;
            resume_slot_label.clear();
            resume_metadata_label.clear();
            resume_save_path.clear();
            update_resume_menu_entry();
            return;
        }

        ft_vector<ft_string> audit_errors;
        bool                 audited = audit_save_directory_for_errors(active_profile_name, audit_errors);
        if (!audited)
        {
            alert_banner.visible = true;
            alert_banner.is_error = true;
            alert_banner.message
                = menu_localize("main_menu.alerts.audit_failure", "Unable to inspect saves for this commander.");
            return;
        }

        if (!audit_errors.empty())
        {
            alert_banner.visible = true;
            alert_banner.is_error = true;
            alert_banner.message = audit_errors[0];
            const size_t additional = audit_errors.size() - 1U;
            if (additional > 0U)
            {
