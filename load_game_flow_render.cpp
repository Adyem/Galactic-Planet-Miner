        }

        if (clip_enabled)
            SDL_RenderSetClipRect(&renderer, ft_nullptr);

        if (menu_font != ft_nullptr && !status_message.empty())
        {
            SDL_Color status_color;
            if (status_is_error)
            {
                status_color.r = 220;
                status_color.g = 90;
                status_color.b = 90;
                status_color.a = 255;
            }
            else
            {
                status_color.r = 170;
                status_color.g = 200;
                status_color.b = 235;
                status_color.a = 255;
            }
            SDL_Rect  status_rect;
            SDL_Texture *status_texture = create_text_texture(renderer, *menu_font, status_message, status_color, status_rect);
            if (status_texture != ft_nullptr)
            {
                status_rect.x = output_width / 2 - status_rect.w / 2;
                status_rect.y = viewport.top + viewport.height + 48;
                SDL_RenderCopy(&renderer, status_texture, ft_nullptr, &status_rect);
                SDL_DestroyTexture(status_texture);
            }
        }
    }
#endif
}

bool audit_save_directory_for_errors(const ft_string &commander_name, ft_vector<ft_string> &out_errors) noexcept
{
    ft_vector<save_slot_entry> slots;
    if (!collect_save_slots(commander_name, slots, &out_errors))
        return false;
    return true;
}

bool resolve_latest_resume_slot(const ft_string &commander_name, ft_string &out_slot_name, ft_string &out_save_path,
    ft_string &out_metadata_label, bool &out_metadata_available) noexcept
{
    out_slot_name.clear();
    out_save_path.clear();
    out_metadata_label.clear();
    out_metadata_available = false;

    ft_vector<save_slot_entry> slots;
    if (!collect_save_slots(commander_name, slots))
        return false;

    bool       found = false;
    long long  best_time = 0;
    bool       best_metadata_available = false;
    ft_string  best_label;

    for (size_t index = 0; index < slots.size(); ++index)
    {
        const save_slot_entry &slot = slots[index];
        if (slot.metadata_error)
            continue;
        if (slot.file_path.empty())
            continue;

        struct stat file_info;
        if (stat(slot.file_path.c_str(), &file_info) != 0)
            continue;

        long long candidate_time = static_cast<long long>(file_info.st_mtime);

        bool prefer_candidate = false;
        if (!found)
            prefer_candidate = true;
        else if (candidate_time > best_time)
            prefer_candidate = true;
        else if (candidate_time == best_time)
        {
            if (slot.metadata_available && !best_metadata_available)
                prefer_candidate = true;
            else if (slot.metadata_available == best_metadata_available)
            {
                if (ft_strcmp(slot.label.c_str(), best_label.c_str()) > 0)
                    prefer_candidate = true;
            }
        }

        if (!prefer_candidate)
            continue;

        found = true;
        best_time = candidate_time;
        best_metadata_available = slot.metadata_available;
        best_label = slot.label;
        out_slot_name = slot.label;
        out_save_path = slot.file_path;
        out_metadata_label = slot.metadata_label;
        out_metadata_available = slot.metadata_available;
    }

    if (!found)
    {
        out_slot_name.clear();
        out_save_path.clear();
        out_metadata_label.clear();
        out_metadata_available = false;
        return false;
    }

    return true;
}

bool run_load_game_flow(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *title_font, TTF_Font *menu_font,
    const ft_string &commander_name, ft_string &out_selected_save, bool &out_quit_requested)
{
    out_selected_save.clear();
    out_quit_requested = false;

#if !GALACTIC_HAVE_SDL2
    (void)window;
    (void)renderer;
    (void)title_font;
    (void)menu_font;
    (void)commander_name;
    return false;
#else
    if (window == ft_nullptr || renderer == ft_nullptr)
        return false;

    ft_vector<save_slot_entry> slots;
    ft_string status_message;
    bool status_is_error = false;
    bool collected = collect_save_slots(commander_name, slots);
    if (!collected)
    {
        status_message
            = menu_localize("load_menu.status.read_directory_failed",
                "Unable to read save directory for this commander.");
        status_is_error = true;
    }
    else if (slots.empty())
        status_message = menu_localize("load_menu.status.no_saves", "No campaign saves found for this commander.");

    ft_ui_menu menu;
    rebuild_load_menu(menu, slots, !slots.empty());

    bool running = true;
    bool loaded = false;
    bool delete_confirmation_active = false;
    ft_string delete_confirmation_identifier;
    ft_string delete_confirmation_label;

    bool rename_active = false;
    ft_string rename_identifier;
    ft_string rename_original_label;
    ft_string rename_input;

    auto clear_delete_confirmation = [&]() {
        delete_confirmation_active = false;
        delete_confirmation_identifier.clear();
        delete_confirmation_label.clear();
    };

    auto begin_delete_confirmation = [&](const save_slot_entry &slot) {
        delete_confirmation_active = true;
        delete_confirmation_identifier = slot.identifier;
        delete_confirmation_label = slot.label;
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement replacement;
        replacement.key = ft_string("slot");
        replacement.value = slot.label;
        replacements.push_back(replacement);
        status_message = menu_localize_format("load_menu.status.delete_confirm",
            "Press Delete again to remove \"{{slot}}\". Esc cancels.", replacements);
        status_is_error = true;
    };

    auto clear_rename_state = [&]() {
        if (rename_active)
            SDL_StopTextInput();
        rename_active = false;
        rename_identifier.clear();
        rename_original_label.clear();
        rename_input.clear();
    };

    auto update_rename_status = [&]() {
        if (!rename_active)
            return;
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(2U);
        StringTableReplacement original_replacement;
        original_replacement.key = ft_string("from");
        original_replacement.value = rename_original_label;
        replacements.push_back(original_replacement);
        StringTableReplacement input_replacement;
        input_replacement.key = ft_string("to");
        input_replacement.value = rename_input;
        replacements.push_back(input_replacement);
        status_message = menu_localize_format("load_menu.status.rename_progress",
            "Renaming \"{{from}}\" to \"{{to}}\". Press Enter to confirm; Esc cancels.", replacements);
        status_is_error = false;
    };

    auto begin_rename = [&](const save_slot_entry &slot) {
        if (rename_active)
            SDL_StopTextInput();
        SDL_StartTextInput();
        rename_active = true;
        rename_identifier = slot.identifier;
        rename_original_label = slot.label;
        rename_input = slot.label;
        update_rename_status();
    };

    auto cancel_rename = [&](const ft_string &message, bool message_is_error) {
        if (!rename_active)
            return;
        clear_rename_state();
        if (!message.empty())
        {
            status_message = message;
            status_is_error = message_is_error;
        }
    };

    while (running)
    {
        SDL_Event event;
        ft_mouse_state mouse_state;
        ft_keyboard_state keyboard_state;

        bool rename_requested = false;
        bool rename_cancel_requested = false;
        bool rename_confirm_now = false;

        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
            {
                out_quit_requested = true;
                running = false;
                break;
            }
            if (event.type == SDL_MOUSEMOTION)
