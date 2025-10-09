#include "app_constants.hpp"
#include "backend_client.hpp"
#include "menu_localization.hpp"
#include "main_menu_system.hpp"
#include "player_profile.hpp"
#include "ui_input.hpp"
#include "ui_menu.hpp"

#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/Time/time.hpp"

#if GALACTIC_HAVE_SDL2
#    include <SDL2/SDL.h>
#    include <SDL2/SDL_ttf.h>
#endif

#if GALACTIC_HAVE_SDL2

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

    ft_ui_menu              menu;
    ft_vector<ft_menu_item> menu_items = build_main_menu_items();
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
    MainMenuAlertBanner         alert_banner;

    bool      resume_available = false;
    bool      resume_metadata_known = false;
    ft_string resume_slot_label;
    ft_string resume_metadata_label;
    ft_string resume_save_path;

    auto update_resume_menu_entry = [&]() {
        for (size_t index = 0; index < menu_items.size(); ++index)
        {
            if (menu_items[index].identifier != "resume")
                continue;

            if (resume_available && !resume_save_path.empty())
            {
                menu_items[index].enabled = true;
                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(2U);
                StringTableReplacement slot_placeholder;
                slot_placeholder.key = ft_string("slot");
                slot_placeholder.value = resume_slot_label;
                replacements.push_back(slot_placeholder);
                StringTableReplacement metadata_placeholder;
                metadata_placeholder.key = ft_string("metadata");
                if (!resume_metadata_label.empty())
                {
                    ft_string metadata_value;
                    metadata_value.append(" (");
                    metadata_value.append(resume_metadata_label);
                    metadata_value.append(")");
                    metadata_placeholder.value = metadata_value;
                }
                replacements.push_back(metadata_placeholder);
                ft_string description = menu_localize_format("main_menu.resume.active_description",
                    "Jump back into \"{{slot}}\"{{metadata}} without opening the load menu.", replacements);
                if (!resume_metadata_known && resume_metadata_label.empty())
                {
                    ft_string metadata_hint = menu_localize(
                        "main_menu.resume.metadata_unavailable", " Metadata details unavailable.");
                    description.append(metadata_hint);
                }
                menu_items[index].description = description;
            }
            else
            {
                menu_items[index].enabled = false;
                menu_items[index].description = menu_localize(
                    "main_menu.resume.empty_description",
                    "No healthy campaign saves found yet. Create or load a game to enable quick resume.");
            }
            break;
        }

        refresh_menu_selection();
    };

    const ft_string backend_host("127.0.0.1:8080");
    const ft_string backend_path("/");
    const ft_string backend_patch_notes_path("/patch-notes/latest");
    const ft_string backend_clear_cloud_path("/cloud-data/clear");
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
                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(2U);
                StringTableReplacement count_placeholder;
                count_placeholder.key = ft_string("count");
                count_placeholder.value = ft_to_string(static_cast<int>(additional));
                replacements.push_back(count_placeholder);
                StringTableReplacement suffix_placeholder;
                suffix_placeholder.key = ft_string("suffix");
                if (additional > 1U)
                    suffix_placeholder.value = ft_string("s");
                replacements.push_back(suffix_placeholder);
                ft_string extra = menu_localize_format("main_menu.alerts.audit_additional",
                    " ({{count}} more issue{{suffix}})", replacements);
                alert_banner.message.append(extra);
            }
        }
        else
        {
            alert_banner.visible = false;
            alert_banner.is_error = false;
            alert_banner.message.clear();
        }

        ft_string latest_slot;
        ft_string latest_path;
        ft_string latest_metadata;
        bool      latest_metadata_available = false;
        if (resolve_latest_resume_slot(active_profile_name, latest_slot, latest_path, latest_metadata,
                latest_metadata_available))
        {
            resume_available = true;
            resume_slot_label = latest_slot;
            resume_save_path = latest_path;
            resume_metadata_label = latest_metadata;
            resume_metadata_known = latest_metadata_available;
            if (!active_profile_name.empty())
                main_menu_preload_commander_portrait(active_profile_name);
        }
        else
        {
            resume_available = false;
            resume_metadata_known = false;
            resume_slot_label.clear();
            resume_save_path.clear();
            resume_metadata_label.clear();
        }

        update_resume_menu_entry();
    };

    auto perform_connectivity_check = [&](long now_ms) {
        main_menu_mark_connectivity_checking(connectivity_status, now_ms);
        int  status_code = 0;
        bool success = backend_client_ping(backend_host, backend_path, status_code);
        long result_ms = ft_time_ms();
        main_menu_apply_connectivity_result(connectivity_status, success, status_code, result_ms);
        if (!success)
            main_menu_append_connectivity_failure_log(backend_host, status_code, result_ms);
        next_connectivity_check_ms = result_ms + connectivity_interval_ms;
    };

    refresh_save_alert();
    perform_connectivity_check(ft_time_ms());

    bool running = true;

    auto attempt_campaign_launch = [&](const ft_string &save_path) -> bool {
        if (!main_menu_can_launch_campaign(save_path))
        {
            alert_banner.visible = true;
            alert_banner.is_error = true;
            alert_banner.message = menu_localize("main_menu.alerts.launch_failure",
                "Unable to launch the selected save. It may have been moved or deleted.");
            return false;
        }

        return true;
    };

    auto dismiss_tutorial_overlay = [&]() -> bool {
        if (!tutorial_context.visible)
            return false;
        tutorial_context.visible = false;
        if (!active_preferences.menu_tutorial_seen)
        {
            active_preferences.menu_tutorial_seen = true;
            player_profile_save(active_preferences);
        }
        return true;
    };

    auto dismiss_changelog_overlay = [&]() -> bool {
        if (!changelog_context.visible)
            return false;
        changelog_context.visible = false;
        return true;
    };

    auto dismiss_manual_overlay = [&]() -> bool {
        if (!manual_context.visible)
            return false;
        manual_context.visible = false;
        return true;
    };

    auto close_clear_cloud_prompt = [&]() -> bool {
        if (!clear_cloud_context.visible)
            return false;
        clear_cloud_context.visible = false;
        clear_cloud_context.heading.clear();
        clear_cloud_context.lines.clear();
        clear_cloud_context.footer.clear();
        return true;
    };

    auto open_clear_cloud_prompt = [&]() {
        dismiss_tutorial_overlay();
        dismiss_changelog_overlay();
        dismiss_manual_overlay();
        clear_cloud_context.visible = true;
        clear_cloud_context.heading
            = menu_localize("main_menu.clear_cloud.heading", "Clear Cloud Data?");
        clear_cloud_context.lines.clear();
        clear_cloud_context.lines.push_back(menu_localize(
            "main_menu.clear_cloud.summary",
            "This will remove any cloud-synced progress for this commander."));
        clear_cloud_context.lines.push_back(menu_localize(
            "main_menu.clear_cloud.local_remains", "Local saves on this device will remain intact."));
        clear_cloud_context.footer = menu_localize(
            "main_menu.clear_cloud.footer", "Press Enter / A to confirm, Esc / B or click to cancel.");
    };

    auto confirm_clear_cloud_prompt = [&]() -> bool {
        if (!clear_cloud_context.visible)
            return false;

        close_clear_cloud_prompt();

        if (connectivity_status.state != MAIN_MENU_CONNECTIVITY_ONLINE)
        {
            alert_banner.visible = true;
            alert_banner.is_error = true;
            alert_banner.message = menu_localize("main_menu.clear_cloud.requires_online",
                "Cloud data can only be cleared while online. Reconnect and try again.");
            return true;
        }

        ft_string response_body;
        int       status_code = 0;
        bool      success
            = backend_client_clear_cloud_data(backend_host, backend_clear_cloud_path, response_body, status_code);

        alert_banner.visible = true;
        if (success)
        {
            alert_banner.is_error = false;
            if (!response_body.empty())
                alert_banner.message = response_body;
            else
            {
                if (!active_profile_name.empty())
                {
                    ft_vector<StringTableReplacement> replacements;
                    replacements.reserve(1U);
                    StringTableReplacement commander_placeholder;
                    commander_placeholder.key = ft_string("commander");
                    commander_placeholder.value = active_profile_name;
                    replacements.push_back(commander_placeholder);
                    alert_banner.message = menu_localize_format("main_menu.clear_cloud.cleared_named",
                        "Cleared cloud data for \"{{commander}}\".", replacements);
                }
                else
                {
                    alert_banner.message = menu_localize("main_menu.clear_cloud.cleared_generic",
                        "Cleared cloud data for this commander.");
                }
            }
        }
        else
        {
            alert_banner.is_error = true;
            alert_banner.message = menu_localize(
                "main_menu.clear_cloud.failure", "Failed to clear cloud data.");
            if (status_code != 0)
            {
                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(1U);
                StringTableReplacement code_placeholder;
                code_placeholder.key = ft_string("code");
                code_placeholder.value = ft_to_string(status_code);
                replacements.push_back(code_placeholder);
                ft_string suffix
                    = menu_localize_format("main_menu.clear_cloud.error_code", " (HTTP {{code}})", replacements);
                alert_banner.message.append(suffix);
            }
            if (!response_body.empty())
            {
                alert_banner.message.append(" ");
                alert_banner.message.append(response_body);
            }
            main_menu_append_connectivity_failure_log(backend_host, status_code, ft_time_ms());
        }
        return true;
    };

    auto open_changelog_overlay = [&]() {
        dismiss_tutorial_overlay();
        dismiss_manual_overlay();
        changelog_context.visible = false;
        changelog_context.heading = ft_string();
        changelog_context.lines.clear();
        changelog_context.footer
            = menu_localize("main_menu.overlay.close", "Press Enter / Space or click to close.");

        if (connectivity_status.state != MAIN_MENU_CONNECTIVITY_ONLINE)
        {
            changelog_context.heading
                = menu_localize("main_menu.changelog.offline_heading", "Patch Notes Unavailable");
            changelog_context.lines.push_back(menu_localize(
                "main_menu.changelog.offline_required", "Patch notes require an online connection."));
            if (connectivity_status.last_status_code != 0)
            {
                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(1U);
                StringTableReplacement code_placeholder;
                code_placeholder.key = ft_string("code");
                code_placeholder.value = ft_to_string(connectivity_status.last_status_code);
                replacements.push_back(code_placeholder);
                ft_string status_line = menu_localize_format(
                    "main_menu.changelog.offline_status", "Last backend status: HTTP {{code}}", replacements);
                changelog_context.lines.push_back(status_line);
            }
            changelog_context.lines.push_back(menu_localize(
                "main_menu.changelog.offline_retry", "Reconnect to view the latest updates."));
            changelog_context.visible = true;
            return;
        }

        ft_string notes_body;
        int       status_code = 0;
        bool      success = backend_client_fetch_patch_notes(backend_host, backend_patch_notes_path, notes_body, status_code);

        if (success)
        {
            changelog_context.heading
                = menu_localize("main_menu.changelog.latest_heading", "Latest Patch Notes");
            ft_vector<ft_string> note_lines = main_menu_split_patch_notes(notes_body);
            for (size_t index = 0; index < note_lines.size(); ++index)
                changelog_context.lines.push_back(note_lines[index]);
            if (changelog_context.lines.empty())
                changelog_context.lines.push_back(menu_localize(
                    "main_menu.changelog.empty_backend", "No patch notes were returned by the backend."));
        }
        else
        {
            changelog_context.heading
                = menu_localize("main_menu.changelog.unavailable_heading", "Patch Notes Unavailable");
            changelog_context.lines.push_back(menu_localize(
                "main_menu.changelog.fetch_failure", "Unable to fetch patch notes from the backend."));
            if (status_code != 0)
            {
                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(1U);
                StringTableReplacement code_placeholder;
                code_placeholder.key = ft_string("code");
                code_placeholder.value = ft_to_string(status_code);
                replacements.push_back(code_placeholder);
                ft_string status_line
                    = menu_localize_format("main_menu.changelog.error_status", "HTTP status: {{code}}", replacements);
                changelog_context.lines.push_back(status_line);
            }
            if (!notes_body.empty())
                changelog_context.lines.push_back(notes_body);
            if (changelog_context.lines.size() == 1U)
                changelog_context.lines.push_back(
                    menu_localize("main_menu.changelog.try_later", "Please try again later."));
        }

        if (changelog_context.lines.empty())
            changelog_context.lines.push_back(
                menu_localize("main_menu.changelog.empty_fallback", "Patch notes are currently empty."));

        changelog_context.visible = true;
    };

    auto open_manual_overlay = [&]() {
        dismiss_tutorial_overlay();
        dismiss_changelog_overlay();
        manual_context.visible = false;
        manual_context.heading
            = menu_localize("main_menu.manual.heading", "Galactic Encyclopedia");
        manual_context.lines.clear();

        const ft_vector<ft_string> &manual_lines = get_main_menu_manual_lines();
        for (size_t index = 0; index < manual_lines.size(); ++index)
            manual_context.lines.push_back(manual_lines[index]);

        manual_context.footer
            = menu_localize("main_menu.overlay.close", "Press Enter / Space or click to close.");
        manual_context.visible = true;
    };

    while (running)
    {
        ft_mouse_state    mouse_state;
        ft_keyboard_state keyboard_state;
        bool              activate_requested = false;
        bool              tutorial_click_in_progress = false;
        bool              changelog_click_in_progress = false;
        bool              manual_click_in_progress = false;
        bool              clear_cloud_click_in_progress = false;

        SDL_Event event;
        while (SDL_PollEvent(&event) == 1)
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                mouse_state.moved = true;
                mouse_state.x = event.motion.x;
                mouse_state.y = event.motion.y;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse_state.x = event.button.x;
                    mouse_state.y = event.button.y;
                    if (tutorial_context.visible)
                        tutorial_click_in_progress = true;
                    else if (clear_cloud_context.visible)
                        clear_cloud_click_in_progress = true;
                    else if (manual_context.visible)
                        manual_click_in_progress = true;
                    else if (changelog_context.visible)
                        changelog_click_in_progress = true;
                    else
                        mouse_state.left_pressed = true;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse_state.x = event.button.x;
                    mouse_state.y = event.button.y;
                    if (tutorial_click_in_progress || tutorial_context.visible)
                    {
                        dismiss_tutorial_overlay();
                        tutorial_click_in_progress = false;
                    }
                    else if (manual_click_in_progress || manual_context.visible)
                    {
                        dismiss_manual_overlay();
                        manual_click_in_progress = false;
                    }
                    else if (changelog_click_in_progress || changelog_context.visible)
                    {
                        dismiss_changelog_overlay();
                        changelog_click_in_progress = false;
                    }
                    else if (clear_cloud_click_in_progress || clear_cloud_context.visible)
                    {
                        close_clear_cloud_prompt();
                        clear_cloud_click_in_progress = false;
                    }
                    else
                        mouse_state.left_released = true;
                }
            }
            else if (event.type == SDL_KEYDOWN)
            {
                auto matches_key = [&](SDL_Keycode key, int mapping) {
                    if (mapping == 0)
                        return false;
                    return key == static_cast<SDL_Keycode>(mapping);
                };

                SDL_Keycode key = event.key.keysym.sym;
                if (matches_key(key, active_preferences.hotkey_menu_up))
                    keyboard_state.pressed_up = true;
                else if (matches_key(key, active_preferences.hotkey_menu_down))
                    keyboard_state.pressed_down = true;
                else if (matches_key(key, active_preferences.hotkey_menu_confirm))
                {
                    if (clear_cloud_context.visible)
                    {
                        confirm_clear_cloud_prompt();
                        clear_cloud_click_in_progress = false;
                    }
                    else if (manual_context.visible)
                    {
                        dismiss_manual_overlay();
                        manual_click_in_progress = false;
                    }
                    else if (changelog_context.visible)
                    {
                        dismiss_changelog_overlay();
                        changelog_click_in_progress = false;
                    }
                    else if (!dismiss_tutorial_overlay())
                        activate_requested = true;
                    tutorial_click_in_progress = false;
                }
                else if (matches_key(key, active_preferences.hotkey_menu_cancel))
                {
                    if (clear_cloud_context.visible)
                        close_clear_cloud_prompt();
                    else if (!dismiss_manual_overlay() && !dismiss_changelog_overlay())
                        running = false;
                }
            }
        }

        if (!mouse_state.moved)
        {
            int x = 0;
            int y = 0;
            SDL_GetMouseState(&x, &y);
            mouse_state.x = x;
            mouse_state.y = y;
        }

        menu.handle_mouse_input(mouse_state);
        menu.handle_keyboard_input(keyboard_state);

        e_ft_input_device active_device = menu.get_active_device();
        if (active_device != FT_INPUT_DEVICE_NONE
            && active_device != static_cast<e_ft_input_device>(active_preferences.last_menu_input_device))
        {
            active_preferences.last_menu_input_device = static_cast<int>(active_device);
        }

        auto process_menu_activation = [&](const ft_menu_item &item) {
            if (clear_cloud_context.visible)
                return;

            if (item.identifier == "new_game")
            {
                bool      creation_quit = false;
                ft_string created_save_path;
                bool      created_save = run_new_game_creation_flow(window, renderer, title_font, menu_font,
                    active_profile_name, created_save_path, creation_quit);
                if (creation_quit)
                {
                    running = false;
                    return;
                }

                if (created_save)
                {
                    player_profile_list(available_profiles);
                    refresh_save_alert();
                    if (!created_save_path.empty() && attempt_campaign_launch(created_save_path))
                    {
                        running = false;
                        return;
                    }
                }
                return;
            }

            if (item.identifier == "resume")
            {
                if (!resume_available || resume_save_path.empty())
                    return;

                if (attempt_campaign_launch(resume_save_path))
                {
                    running = false;
                    return;
                }

                refresh_save_alert();
                return;
            }

            if (item.identifier == "load")
            {
                bool      load_quit = false;
                ft_string selected_save_path;
                bool      loaded = run_load_game_flow(window, renderer, title_font, menu_font, active_profile_name,
                    selected_save_path, load_quit);
                if (load_quit)
                {
                    running = false;
                    return;
                }

                if (loaded)
                {
                    if (attempt_campaign_launch(selected_save_path))
                    {
                        running = false;
                        return;
                    }
                }
                refresh_save_alert();
                return;
            }

            if (item.identifier == "exit")
            {
                running = false;
                return;
            }

            if (item.identifier == "swap_profile")
            {
                bool management_quit = false;
                ft_string selected_profile = run_profile_management_flow(window, renderer, title_font, menu_font, active_profile_name,
                    management_quit);
                if (management_quit)
                {
                    running = false;
                    return;
                }

                if (!selected_profile.empty() && selected_profile != active_profile_name)
                {
                    active_profile_name = selected_profile;
                    apply_profile_preferences(window, active_profile_name);
                    refresh_fonts();
                    if (!player_profile_load_or_create(active_preferences, active_profile_name))
                    {
                        active_preferences = PlayerProfilePreferences();
                        active_preferences.commander_name = active_profile_name;
                    }
                    tutorial_context.visible = !active_preferences.menu_tutorial_seen;
                    perform_connectivity_check(ft_time_ms());
                }

                player_profile_list(available_profiles);
                refresh_save_alert();
            }

            if (item.identifier == "settings")
            {
                bool settings_quit = false;
                bool saved = run_settings_flow(window, renderer, title_font, menu_font, active_preferences, settings_quit);
                if (settings_quit)
                {
                    running = false;
                    return;
                }

                if (saved)
                    apply_profile_preferences(window, active_profile_name);
                if (saved)
                    refresh_fonts();
                return;
            }

            if (item.identifier == "clear_cloud")
            {
                if (connectivity_status.state != MAIN_MENU_CONNECTIVITY_ONLINE)
                {
                    alert_banner.visible = true;
                    alert_banner.is_error = true;
            alert_banner.message = menu_localize("main_menu.clear_cloud.requires_online",
                "Cloud data can only be cleared while online. Reconnect and try again.");
            return;
        }
                open_clear_cloud_prompt();
                return;
            }

            if (item.identifier == "changelog")
            {
                open_changelog_overlay();
                return;
            }

            if (item.identifier == "manual")
            {
                open_manual_overlay();
                return;
            }
        };

        if (mouse_state.left_released)
        {
            const int index = menu.get_hovered_index();
            const ft_menu_item *hovered_item = menu_item_from_index(menu, index);
            if (hovered_item != ft_nullptr)
            {
                process_menu_activation(*hovered_item);
                if (!running)
                    break;
            }
        }

        if (activate_requested)
        {
            const ft_menu_item *selected_item = menu.get_selected_item();
            if (selected_item != ft_nullptr)
            {
                process_menu_activation(*selected_item);
                if (!running)
                    break;
            }
        }

        int window_width = 0;
        int window_height = 0;
        SDL_GetWindowSize(window, &window_width, &window_height);
        long current_time_ms = ft_time_ms();
        if (current_time_ms >= next_connectivity_check_ms)
            perform_connectivity_check(current_time_ms);

        render_main_menu(*renderer, menu, title_font, menu_font, window_width, window_height, active_profile_name,
            &active_preferences, &tutorial_context, &manual_context, &changelog_context, &clear_cloud_context,
            &connectivity_status, &alert_banner);
    }

    if (window != ft_nullptr && !active_profile_name.empty())
        save_profile_preferences(window, active_preferences);

    destroy_renderer(renderer);
    destroy_window(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}

#else

int main()
{
    return 0;
}

#endif

