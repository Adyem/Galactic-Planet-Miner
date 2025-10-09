#include "main_menu_system.hpp"

#include "app_constants.hpp"
#include "game.hpp"
#include "player_profile.hpp"
#include "ui_input.hpp"
#include "menu_localization.hpp"

#include "libft/JSon/document.hpp"
#include "libft/JSon/json.hpp"
#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/File/file_utils.hpp"
#include "libft/File/open_dir.hpp"
#include "libft/Libft/libft.hpp"
#include "libft/Template/algorithm.hpp"

#include <sys/stat.h>

namespace
{
    struct save_slot_entry
    {
        ft_string identifier;
        ft_string label;
        ft_string file_path;
        ft_string metadata_label;
        bool      metadata_available;
        bool      metadata_error;
        bool      difficulty_known;
        int       difficulty_value;
    };

    constexpr unsigned int kMaxSaveNameLength = 24U;

    bool is_save_character_allowed(char character) noexcept
    {
        const bool is_lower = character >= 'a' && character <= 'z';
        const bool is_upper = character >= 'A' && character <= 'Z';
        const bool is_digit = character >= '0' && character <= '9';
        return is_lower || is_upper || is_digit;
    }

#if GALACTIC_HAVE_SDL2
    bool append_save_character(ft_string &value, char character) noexcept
    {
        if (!is_save_character_allowed(character))
            return false;

        if (value.size() >= static_cast<size_t>(kMaxSaveNameLength))
            return false;

        value.append(character);
        return true;
    }

    void remove_last_save_character(ft_string &value) noexcept
    {
        const size_t current_size = value.size();
        if (current_size == 0U)
            return;
        value.erase(current_size - 1U, 1U);
    }
#endif

    bool save_name_is_valid(const ft_string &value) noexcept
    {
        return !value.empty();
    }

    bool has_save_extension(const char *name) noexcept
    {
        if (name == ft_nullptr)
            return false;
        const size_t length = ft_strlen(name);
        if (length < 6U)
            return false;
        const char *extension = name + length - 5U;
        if (extension[0] != '.')
            return false;
        if (extension[1] != 'j' && extension[1] != 'J')
            return false;
        if (extension[2] != 's' && extension[2] != 'S')
            return false;
        if (extension[3] != 'o' && extension[3] != 'O')
            return false;
        if (extension[4] != 'n' && extension[4] != 'N')
            return false;
        return true;
    }

    void strip_save_extension(const char *name, ft_string &out) noexcept
    {
        out.clear();
        if (name == ft_nullptr)
            return;
        const size_t length = ft_strlen(name);
        if (length == 0U)
            return;
        size_t end = length;
        if (length > 5U && has_save_extension(name))
            end = length - 5U;
        for (size_t index = 0; index < end; ++index)
            out += name[index];
    }

    bool parse_int_value(const char *value, int &out) noexcept
    {
        if (value == ft_nullptr)
            return false;

        char *end = ft_nullptr;
        long  parsed = ft_strtol(value, &end, 10);
        if (end == value)
            return false;

        if (parsed < static_cast<long>(FT_INT_MIN))
            parsed = static_cast<long>(FT_INT_MIN);
        if (parsed > static_cast<long>(FT_INT_MAX))
            parsed = static_cast<long>(FT_INT_MAX);

        out = static_cast<int>(parsed);
        return true;
    }

    bool read_save_metadata(const ft_string &file_path, bool &out_day_valid, int &out_day, bool &out_level_valid,
        int &out_level, bool &out_difficulty_valid, int &out_difficulty) noexcept
    {
        out_day_valid = false;
        out_level_valid = false;
        out_difficulty_valid = false;
        out_day = 0;
        out_level = 0;
        out_difficulty = 0;

        if (file_path.empty())
            return false;

        json_document document;
        if (document.read_from_file(file_path.c_str()) != 0)
            return false;

        json_group *campaign_group = document.find_group("campaign");
        if (campaign_group != ft_nullptr)
        {
            json_item *day_item = document.find_item(campaign_group, "day");
            if (day_item != ft_nullptr)
            {
                int parsed_day = 0;
                if (parse_int_value(day_item->value, parsed_day))
                {
                    if (parsed_day < 1)
                        parsed_day = 1;
                    out_day = parsed_day;
                    out_day_valid = true;
                }
            }

            json_item *difficulty_item = document.find_item(campaign_group, "difficulty");
            if (difficulty_item != ft_nullptr)
            {
                int parsed_difficulty = 0;
                if (parse_int_value(difficulty_item->value, parsed_difficulty))
                {
                    if (parsed_difficulty == GAME_DIFFICULTY_EASY
                        || parsed_difficulty == GAME_DIFFICULTY_STANDARD
                        || parsed_difficulty == GAME_DIFFICULTY_HARD)
                    {
                        out_difficulty = parsed_difficulty;
                        out_difficulty_valid = true;
                    }
                }
            }
        }

        json_group *player_group = document.find_group("player");
        if (player_group != ft_nullptr)
        {
            json_item *level_item = document.find_item(player_group, "commander_level");
            if (level_item != ft_nullptr)
            {
                int parsed_level = 0;
                if (parse_int_value(level_item->value, parsed_level))
                {
                    if (parsed_level < 1)
                        parsed_level = 1;
                    out_level = parsed_level;
                    out_level_valid = true;
                }
            }
        }

        return true;
    }

    ft_string format_difficulty_component(bool difficulty_valid, int difficulty)
    {
        if (!difficulty_valid)
            return menu_localize("load_menu.metadata.difficulty_unknown", "Difficulty ?");

        const char *difficulty_key = ft_nullptr;
        const char *difficulty_fallback = ft_nullptr;
        switch (difficulty)
        {
        case GAME_DIFFICULTY_EASY:
            difficulty_key = "load_menu.metadata.difficulty.easy";
            difficulty_fallback = "Easy";
            break;
        case GAME_DIFFICULTY_STANDARD:
            difficulty_key = "load_menu.metadata.difficulty.standard";
            difficulty_fallback = "Standard";
            break;
        case GAME_DIFFICULTY_HARD:
            difficulty_key = "load_menu.metadata.difficulty.hard";
            difficulty_fallback = "Hard";
            break;
        default:
            break;
        }

        if (difficulty_key == ft_nullptr || difficulty_fallback == ft_nullptr)
            return menu_localize("load_menu.metadata.difficulty_unknown", "Difficulty ?");

        ft_string difficulty_label = menu_localize(difficulty_key, difficulty_fallback);
        ft_vector<StringTableReplacement> replacements;
        replacements.reserve(1U);
        StringTableReplacement replacement;
        replacement.key = ft_string("difficulty");
        replacement.value = difficulty_label;
        replacements.push_back(replacement);
        return menu_localize_format(
            "load_menu.metadata.difficulty_known", "Difficulty: {{difficulty}}", replacements);
    }

    ft_string format_save_metadata_label(
        bool day_valid, int day, bool level_valid, int level, bool difficulty_valid, int difficulty)
    {
        if (!day_valid && !level_valid && !difficulty_valid)
            return menu_localize("load_menu.metadata.unavailable", "Metadata unavailable");

        ft_string day_label;
        if (day_valid)
        {
            ft_vector<StringTableReplacement> replacements;
            replacements.reserve(1U);
            StringTableReplacement replacement;
            replacement.key = ft_string("value");
            replacement.value = ft_to_string(day);
            replacements.push_back(replacement);
            day_label = menu_localize_format("load_menu.metadata.day_known", "Day {{value}}", replacements);
        }
        else
            day_label = menu_localize("load_menu.metadata.day_unknown", "Day ?");

        ft_string level_label;
        if (level_valid)
        {
            ft_vector<StringTableReplacement> replacements;
            replacements.reserve(1U);
            StringTableReplacement replacement;
            replacement.key = ft_string("value");
            replacement.value = ft_to_string(level);
            replacements.push_back(replacement);
            level_label = menu_localize_format("load_menu.metadata.level_known", "Level {{value}}", replacements);
        }
        else
            level_label = menu_localize("load_menu.metadata.level_unknown", "Level ?");

        ft_string difficulty_label = format_difficulty_component(difficulty_valid, difficulty);

        ft_vector<StringTableReplacement> combined_replacements;
        combined_replacements.reserve(3U);

        StringTableReplacement day_replacement;
        day_replacement.key = ft_string("day");
        day_replacement.value = day_label;
        combined_replacements.push_back(day_replacement);

        StringTableReplacement level_replacement;
        level_replacement.key = ft_string("level");
        level_replacement.value = level_label;
        combined_replacements.push_back(level_replacement);

        StringTableReplacement difficulty_replacement;
        difficulty_replacement.key = ft_string("difficulty");
        difficulty_replacement.value = difficulty_label;
        combined_replacements.push_back(difficulty_replacement);

        return menu_localize_format(
            "load_menu.metadata.combined", "{{day}} • {{level}} • {{difficulty}}", combined_replacements);
    }

    ft_string build_save_file_path(const ft_string &commander_name, const ft_string &slot_name)
    {
        if (commander_name.empty() || slot_name.empty())
            return ft_string();

        ft_string save_directory = player_profile_resolve_save_directory(commander_name);
        if (save_directory.empty())
            return ft_string();

        ft_string file_name(slot_name);
        file_name.append(".json");
        return file_path_join(save_directory.c_str(), file_name.c_str());
    }

    bool rename_save_slot(
        const ft_string &commander_name, const ft_string &old_name, const ft_string &new_name, ft_string &out_error) noexcept
    {
        out_error.clear();

        if (commander_name.empty())
        {
            out_error = menu_localize("load_menu.error.no_commander", "Commander profile missing.");
            return false;
        }

        if (old_name.empty())
        {
            out_error = menu_localize("load_menu.error.no_original_name", "Original save name missing.");
            return false;
        }

        if (!save_name_is_valid(new_name))
        {
            out_error = menu_localize("load_menu.error.name_required", "Please enter a new save name.");
            return false;
        }

        if (new_name.size() > static_cast<size_t>(kMaxSaveNameLength))
        {
            out_error = menu_localize(
                "load_menu.error.name_length", "Save names are limited to 24 characters.");
            return false;
        }

        for (size_t index = 0; index < new_name.size(); ++index)
        {
            if (!is_save_character_allowed(new_name[index]))
            {
                out_error = menu_localize("load_menu.error.name_charset", "Use letters and numbers only.");
                return false;
            }
        }

        ft_string source_path = build_save_file_path(commander_name, old_name);
        if (source_path.empty())
        {
            out_error
                = menu_localize("load_menu.error.resolve_existing", "Unable to resolve existing save location.");
            return false;
        }

        int source_exists = file_exists(source_path.c_str());
        if (source_exists <= 0)
        {
            if (source_exists < 0)
                out_error
                    = menu_localize("load_menu.error.verify_existing", "Unable to verify existing save file.");
            else
                out_error = menu_localize("load_menu.error.missing_original", "Original save file not found.");
            return false;
        }

        ft_string destination_path = build_save_file_path(commander_name, new_name);
        if (destination_path.empty())
        {
            out_error
                = menu_localize("load_menu.error.resolve_new", "Unable to resolve new save location.");
            return false;
        }

        int destination_exists = file_exists(destination_path.c_str());
        if (destination_exists < 0)
        {
            out_error
                = menu_localize("load_menu.error.check_conflict", "Unable to check for conflicting saves.");
            return false;
        }
        if (destination_exists > 0)
        {
            out_error = menu_localize("load_menu.error.conflict", "A save with that name already exists.");
            return false;
        }

        if (file_move(source_path.c_str(), destination_path.c_str()) != 0)
        {
            out_error = menu_localize("load_menu.error.rename_failed", "Failed to rename save file.");
            return false;
        }

        return true;
    }

    bool delete_save_slot(const ft_string &commander_name, const ft_string &slot_name, ft_string &out_error) noexcept
    {
        out_error.clear();

        ft_string file_path = build_save_file_path(commander_name, slot_name);
        if (file_path.empty())
        {
            out_error = menu_localize("load_menu.error.resolve_path", "Unable to resolve save location.");
            return false;
        }

        int exists_result = file_exists(file_path.c_str());
        if (exists_result < 0)
        {
            out_error = menu_localize("load_menu.error.verify_path", "Unable to verify save file.");
            return false;
        }
        if (exists_result == 0)
        {
            out_error = menu_localize("load_menu.error.not_found", "Save file not found.");
            return false;
        }

        if (file_delete(file_path.c_str()) != 0)
        {
            out_error = menu_localize("load_menu.error.delete_failed", "Failed to delete save file.");
            return false;
        }

        return true;
    }

    bool collect_save_slots(const ft_string &commander_name, ft_vector<save_slot_entry> &out_slots,
        ft_vector<ft_string> *out_errors = ft_nullptr)
    {
        out_slots.clear();
        if (commander_name.empty())
            return false;

        if (out_errors != ft_nullptr)
            out_errors->clear();

        ft_string save_directory = player_profile_resolve_save_directory(commander_name);
        if (save_directory.empty())
            return false;

        int exists_result = file_dir_exists(save_directory.c_str());
        if (exists_result < 0)
            return false;
        if (exists_result == 0)
            return true;

        file_dir *directory_stream = file_opendir(save_directory.c_str());
        if (directory_stream == ft_nullptr)
            return false;

        file_dirent *entry = ft_nullptr;
        while ((entry = file_readdir(directory_stream)) != ft_nullptr)
        {
            if (entry->d_name[0] == '\0')
                continue;
            if (entry->d_name[0] == '.')
            {
                if (entry->d_name[1] == '\0')
                    continue;
                if (entry->d_name[1] == '.' && entry->d_name[2] == '\0')
                    continue;
            }
            if (!has_save_extension(entry->d_name))
                continue;

            ft_string label;
            strip_save_extension(entry->d_name, label);
            if (label.empty())
                continue;

            save_slot_entry slot;
            slot.identifier = ft_string("save:");
            slot.identifier.append(label);
            slot.label = label;
            slot.file_path = file_path_join(save_directory.c_str(), entry->d_name);
            slot.difficulty_known = false;
            slot.difficulty_value = 0;
            bool metadata_day_valid = false;
            bool metadata_level_valid = false;
            bool metadata_difficulty_valid = false;
            int  metadata_day = 0;
            int  metadata_level = 0;
            int  metadata_difficulty = 0;
            bool metadata_loaded = read_save_metadata(slot.file_path, metadata_day_valid, metadata_day, metadata_level_valid,
                metadata_level, metadata_difficulty_valid, metadata_difficulty);
            if (metadata_loaded)
            {
                slot.difficulty_known = metadata_difficulty_valid;
                slot.difficulty_value = metadata_difficulty;
                if (metadata_day_valid || metadata_level_valid || metadata_difficulty_valid)
                {
                    slot.metadata_label = format_save_metadata_label(
                        metadata_day_valid, metadata_day, metadata_level_valid, metadata_level, metadata_difficulty_valid,
                        metadata_difficulty);
                    slot.metadata_available = true;
                }
                else
                {
                    slot.metadata_label
                        = menu_localize("load_menu.metadata.unavailable", "Metadata unavailable");
                    slot.metadata_available = false;
                    slot.difficulty_known = false;
                    slot.difficulty_value = 0;
                }
                slot.metadata_error = false;
            }
            else
            {
                slot.metadata_label
                    = menu_localize("load_menu.metadata.unreadable", "Unable to read metadata");
                slot.metadata_available = false;
                slot.metadata_error = true;
                slot.difficulty_known = false;
                slot.difficulty_value = 0;
                if (out_errors != ft_nullptr)
                {
                    ft_vector<StringTableReplacement> replacements;
                    replacements.reserve(1U);
                    StringTableReplacement replacement;
                    replacement.key = ft_string("slot");
                    replacement.value = label;
                    replacements.push_back(replacement);
                    out_errors->push_back(menu_localize_format(
                        "load_menu.errors.metadata_parse",
                        "Save \"{{slot}}\" could not be read (metadata parsing failed).",
                        replacements));
                }
            }
            out_slots.push_back(slot);
        }

        file_closedir(directory_stream);

        ft_sort(out_slots.begin(), out_slots.end(), [](const save_slot_entry &lhs, const save_slot_entry &rhs) {
            const int compare = ft_strcmp(lhs.label.c_str(), rhs.label.c_str());
            return compare < 0;
        });

        return true;
    }

#if GALACTIC_HAVE_SDL2
    void rebuild_load_menu(ft_ui_menu &menu, const ft_vector<save_slot_entry> &slots, bool has_selection)
    {
        const ft_rect base_rect(460, 220, 360, 72);
        const int      spacing = 18;

        ft_vector<ft_menu_item> items;
        const size_t             slot_count = slots.size();
        items.reserve(slot_count + 1U);

        for (size_t index = 0; index < slot_count; ++index)
        {
            ft_rect item_rect = base_rect;
            item_rect.top += static_cast<int>(index) * (base_rect.height + spacing);

            const save_slot_entry &slot = slots[index];
            ft_menu_item           item(slot.identifier, slot.label, item_rect);
            item.enabled = true;
            if (!slot.metadata_label.empty())
                item.description = slot.metadata_label;
            items.push_back(item);
        }

        ft_rect cancel_rect = base_rect;
        cancel_rect.top += static_cast<int>(slot_count) * (base_rect.height + spacing);
        ft_menu_item cancel_item(ft_string("action:cancel"),
            menu_localize("load_menu.menu.cancel", "Cancel"), cancel_rect);
        items.push_back(cancel_item);

        menu.set_items(items);

        ft_rect viewport = base_rect;
        const int window_height = static_cast<int>(app_constants::kWindowHeight);
        const int reserved_bottom = 220;
        viewport.height = window_height - base_rect.top - reserved_bottom;
        if (viewport.height < base_rect.height)
            viewport.height = base_rect.height;
        menu.set_viewport_bounds(viewport);

        if (has_selection && slot_count > 0U)
            menu.set_selected_index(0);
        else
            menu.set_selected_index(static_cast<int>(slot_count));
    }

    void render_load_screen(SDL_Renderer &renderer, const ft_ui_menu &menu, TTF_Font *title_font, TTF_Font *menu_font,
        const ft_string &commander_name, const ft_string &status_message, bool status_is_error)
    {
        SDL_SetRenderDrawColor(&renderer, 12, 16, 28, 255);
        SDL_RenderClear(&renderer);

        int output_width = static_cast<int>(app_constants::kWindowWidth);
        int output_height = static_cast<int>(app_constants::kWindowHeight);
        SDL_GetRendererOutputSize(&renderer, &output_width, &output_height);

        if (title_font != ft_nullptr)
        {
            ft_string heading = menu_localize("load_menu.menu.heading", "Load Campaign");
            SDL_Color title_color = {220, 220, 245, 255};
            SDL_Rect  title_rect;
            SDL_Texture *title_texture = create_text_texture(renderer, *title_font, heading, title_color, title_rect);
            if (title_texture != ft_nullptr)
            {
                title_rect.x = output_width / 2 - title_rect.w / 2;
                title_rect.y = 96;
                SDL_RenderCopy(&renderer, title_texture, ft_nullptr, &title_rect);
                SDL_DestroyTexture(title_texture);
            }
        }

        if (menu_font != ft_nullptr)
        {
            ft_string info_text = menu_localize("load_menu.menu.instructions",
                "Select a save to prepare for campaign resume. Enter loads; Delete removes; R renames; Esc cancels.");
            if (!commander_name.empty())
            {
                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(1U);
                StringTableReplacement replacement;
                replacement.key = ft_string("commander");
                replacement.value = commander_name;
                replacements.push_back(replacement);
                info_text.append(menu_localize_format("load_menu.menu.instructions_commander",
                    " Commander: {{commander}}.", replacements));
            }
            SDL_Color info_color = {170, 180, 210, 255};
            SDL_Rect  info_rect;
            SDL_Texture *info_texture = create_text_texture(renderer, *menu_font, info_text, info_color, info_rect);
            if (info_texture != ft_nullptr)
            {
                info_rect.x = output_width / 2 - info_rect.w / 2;
                info_rect.y = 164;
                SDL_RenderCopy(&renderer, info_texture, ft_nullptr, &info_rect);
                SDL_DestroyTexture(info_texture);
            }
        }

        const ft_vector<ft_menu_item> &items = menu.get_items();
        const int hovered_index = menu.get_hovered_index();
        const int selected_index = menu.get_selected_index();
        const ft_rect &viewport = menu.get_viewport_bounds();
        const bool clip_enabled = viewport.width > 0 && viewport.height > 0;
        SDL_Rect clip_rect = {0, 0, 0, 0};
        int clip_bottom = 0;

        if (clip_enabled)
        {
            clip_rect.x = viewport.left;
            clip_rect.y = viewport.top;
            clip_rect.w = viewport.width;
            clip_rect.h = viewport.height;
            clip_bottom = clip_rect.y + clip_rect.h;
            SDL_RenderSetClipRect(&renderer, &clip_rect);
        }

        const int scroll_offset = menu.get_scroll_offset();

        for (size_t index = 0; index < items.size(); ++index)
        {
            const ft_menu_item &item = items[index];
            const bool          is_hovered = static_cast<int>(index) == hovered_index;
            const bool          is_selected = static_cast<int>(index) == selected_index;
            const bool          is_disabled = !item.enabled;

            Uint8 r = 28;
            Uint8 g = 36;
            Uint8 b = 60;

            if (is_disabled)
            {
                r = 30;
                g = 34;
                b = 44;
                if (is_hovered)
                {
                    r = 42;
                    g = 46;
                    b = 60;
                }
            }
            else if (is_hovered)
            {
                r = 56;
                g = 84;
                b = 140;
            }
            else if (is_selected)
            {
                r = 40;
                g = 64;
                b = 112;
            }

            SDL_Rect button_rect;
            button_rect.x = item.bounds.left;
            button_rect.y = item.bounds.top - scroll_offset;
            button_rect.w = item.bounds.width;
            button_rect.h = item.bounds.height;
            if (clip_enabled && (button_rect.y + button_rect.h <= clip_rect.y || button_rect.y >= clip_bottom))
                continue;

            SDL_SetRenderDrawColor(&renderer, r, g, b, 255);
            SDL_RenderFillRect(&renderer, &button_rect);
            const Uint8 border_r = is_disabled ? 70 : 90;
            const Uint8 border_g = is_disabled ? 80 : 110;
            const Uint8 border_b = is_disabled ? 120 : 160;
            SDL_SetRenderDrawColor(&renderer, border_r, border_g, border_b, 255);
            SDL_RenderDrawRect(&renderer, &button_rect);

            if (menu_font != ft_nullptr)
            {
                SDL_Color text_color = {255, 255, 255, 255};
                if (is_disabled)
                {
                    text_color.r = 188;
                    text_color.g = 196;
                    text_color.b = 210;
                }
                SDL_Rect     text_rect;
                SDL_Texture *text_texture = create_text_texture(renderer, *menu_font, item.label, text_color, text_rect);
                if (text_texture != ft_nullptr)
                {
                    const int text_left = item.bounds.left + 18;
                    int       text_top = button_rect.y + 12;
                    text_rect.x = text_left;
                    text_rect.y = text_top;
                    SDL_RenderCopy(&renderer, text_texture, ft_nullptr, &text_rect);
                    SDL_DestroyTexture(text_texture);
                    text_top = text_rect.y + text_rect.h + 6;

                    if (!item.description.empty())
                    {
                        SDL_Color description_color = {210, 220, 240, 255};
                        if (is_disabled)
                        {
                            description_color.r = 170;
                            description_color.g = 178;
                            description_color.b = 196;
                        }
                        SDL_Rect     description_rect;
                        SDL_Texture *description_texture = create_text_texture(
                            renderer, *menu_font, item.description, description_color, description_rect);
                        if (description_texture != ft_nullptr)
                        {
                            description_rect.x = text_left;
                            description_rect.y = text_top;
                            SDL_RenderCopy(&renderer, description_texture, ft_nullptr, &description_rect);
                            SDL_DestroyTexture(description_texture);
                        }
                    }
                }
            }
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
            {
                mouse_state.moved = true;
                mouse_state.x = event.motion.x;
                mouse_state.y = event.motion.y;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse_state.left_pressed = true;
                    mouse_state.x = event.button.x;
                    mouse_state.y = event.button.y;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONUP)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    mouse_state.left_released = true;
                    mouse_state.x = event.button.x;
                    mouse_state.y = event.button.y;
                }
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (rename_active)
                {
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                        rename_cancel_requested = true;
                    else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
                        rename_confirm_now = true;
                    else if (event.key.keysym.sym == SDLK_BACKSPACE || event.key.keysym.sym == SDLK_DELETE)
                    {
                        remove_last_save_character(rename_input);
                        update_rename_status();
                    }
                    continue;
                }

                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    if (delete_confirmation_active)
                    {
                        clear_delete_confirmation();
                        status_message
                            = menu_localize("load_menu.status.delete_canceled", "Deletion canceled.");
                        status_is_error = false;
                    }
                    else
                    {
                        running = false;
                        break;
                    }
                }
                else if (event.key.keysym.sym == SDLK_UP)
                    keyboard_state.pressed_up = true;
                else if (event.key.keysym.sym == SDLK_DOWN)
                    keyboard_state.pressed_down = true;
                else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE)
                    keyboard_state.pressed_confirm = true;
                else if (event.key.keysym.sym == SDLK_DELETE || event.key.keysym.sym == SDLK_BACKSPACE)
                    keyboard_state.pressed_delete = true;
                else if (event.key.keysym.sym == SDLK_r)
                    rename_requested = true;
            }
            else if (event.type == SDL_TEXTINPUT)
            {
                if (rename_active)
                {
                    const char *text = event.text.text;
                    if (text != ft_nullptr)
                    {
                        bool appended_character = false;
                        bool rejected_for_length = false;
                        bool rejected_for_invalid = false;

                        for (int index = 0; text[index] != '\0'; ++index)
                        {
                            const char character = text[index];
                            if (append_save_character(rename_input, character))
                                appended_character = true;
                            else if (!is_save_character_allowed(character))
                                rejected_for_invalid = true;
                            else
                                rejected_for_length = true;
                        }

                        if (appended_character)
                        {
                            update_rename_status();
                        }
                        else if (rejected_for_length)
                        {
                            ft_vector<StringTableReplacement> replacements;
                            replacements.reserve(1U);
                            StringTableReplacement replacement;
                            replacement.key = ft_string("name");
                            replacement.value = rename_input;
                            replacements.push_back(replacement);
                            status_message = menu_localize(
                                "load_menu.error.name_length", "Save names are limited to 24 characters.");
                            status_message.append(menu_localize_format(
                                "load_menu.status.current_name", " Current name: \"{{name}}\".", replacements));
                            status_is_error = true;
                        }
                        else if (rejected_for_invalid)
                        {
                            ft_vector<StringTableReplacement> replacements;
                            replacements.reserve(1U);
                            StringTableReplacement replacement;
                            replacement.key = ft_string("name");
                            replacement.value = rename_input;
                            replacements.push_back(replacement);
                            status_message = menu_localize(
                                "load_menu.error.name_charset", "Use letters and numbers only.");
                            status_message.append(menu_localize_format(
                                "load_menu.status.current_name", " Current name: \"{{name}}\".", replacements));
                            status_is_error = true;
                        }
                    }
                }
            }
        }

        if (!running)
            break;

        if (rename_cancel_requested)
            cancel_rename(menu_localize("load_menu.status.rename_canceled", "Rename canceled."), false);

        if (!mouse_state.moved)
        {
            int x = 0;
            int y = 0;
            SDL_GetMouseState(&x, &y);
            mouse_state.x = x;
            mouse_state.y = y;
        }

        const int previous_selected_index = menu.get_selected_index();
        menu.handle_mouse_input(mouse_state);
        menu.handle_keyboard_input(keyboard_state);
        const int current_selected_index = menu.get_selected_index();
        if (delete_confirmation_active && current_selected_index != previous_selected_index)
        {
            clear_delete_confirmation();
            status_message = menu_localize("load_menu.status.delete_canceled", "Deletion canceled.");
            status_is_error = false;
        }
        if (delete_confirmation_active && (current_selected_index < 0
                                              || current_selected_index >= static_cast<int>(slots.size())))
        {
            clear_delete_confirmation();
            status_message = menu_localize("load_menu.status.delete_canceled", "Deletion canceled.");
            status_is_error = false;
        }

        ft_string selected_identifier;
        const ft_menu_item *current_selected_item = menu.get_selected_item();
        if (current_selected_item != ft_nullptr)
            selected_identifier = current_selected_item->identifier;

        const save_slot_entry *selected_slot = ft_nullptr;
        if (!selected_identifier.empty() && selected_identifier.compare(0, 5, "save:") == 0)
        {
            for (size_t index = 0; index < slots.size(); ++index)
            {
                if (slots[index].identifier == selected_identifier)
                {
                    selected_slot = &slots[index];
                    break;
                }
            }
        }

        if (rename_active)
        {
            if (selected_slot == ft_nullptr || selected_identifier != rename_identifier)
                cancel_rename(menu_localize("load_menu.status.rename_canceled", "Rename canceled."), false);
        }

        if (rename_requested)
        {
            if (selected_slot != ft_nullptr)
                begin_rename(*selected_slot);
            else if (!slots.empty())
            {
                status_message = menu_localize(
                    "load_menu.status.rename_select", "Select a save before pressing R to rename.");
                status_is_error = true;
            }
        }

        bool skip_menu_activation = false;
        bool confirm_delete_now = false;

        if (rename_active)
            skip_menu_activation = true;

        if (rename_confirm_now && rename_active)
        {
            skip_menu_activation = true;
            if (rename_input == rename_original_label)
            {
                cancel_rename(menu_localize("load_menu.status.rename_unchanged", "Save name unchanged."), false);
            }
            else
            {
                const ft_string previous_label = rename_original_label;
                const ft_string new_label = rename_input;
                ft_string rename_error;
                bool renamed = ::rename_save_slot(commander_name, previous_label, new_label, rename_error);
                if (renamed)
                {
                    ft_string new_identifier("save:");
                    new_identifier.append(new_label);
                    clear_rename_state();

                    bool refreshed = collect_save_slots(commander_name, slots);
                    if (!refreshed)
                    {
                        status_message = menu_localize(
                            "load_menu.status.refresh_failed_rename",
                            "Unable to refresh save directory after rename.");
                        status_is_error = true;
                        slots.clear();
                        rebuild_load_menu(menu, slots, false);
                    }
                    else
                    {
                        rebuild_load_menu(menu, slots, !slots.empty());
                        if (slots.empty())
                        {
                            status_message
                                = menu_localize("load_menu.status.no_saves",
                                    "No campaign saves found for this commander.");
                            status_is_error = false;
                        }
                        else
                        {
                            ft_vector<StringTableReplacement> replacements;
                            replacements.reserve(2U);
                            StringTableReplacement from_replacement;
                            from_replacement.key = ft_string("from");
                            from_replacement.value = previous_label;
                            replacements.push_back(from_replacement);
                            StringTableReplacement to_replacement;
                            to_replacement.key = ft_string("to");
                            to_replacement.value = new_label;
                            replacements.push_back(to_replacement);
                            status_message = menu_localize_format(
                                "load_menu.status.rename_success",
                                "Renamed save \"{{from}}\" to \"{{to}}\".",
                                replacements);
                            status_is_error = false;

                            for (size_t index = 0; index < slots.size(); ++index)
                            {
                                if (slots[index].identifier == new_identifier)
                                {
                                    menu.set_selected_index(static_cast<int>(index));
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (rename_error.empty())
                        status_message
                            = menu_localize("load_menu.error.rename_failed", "Failed to rename save file.");
                    else
                        status_message = rename_error;
                    ft_vector<StringTableReplacement> replacements;
                    replacements.reserve(1U);
                    StringTableReplacement replacement;
                    replacement.key = ft_string("name");
                    replacement.value = rename_input;
                    replacements.push_back(replacement);
                    status_message.append(menu_localize_format(
                        "load_menu.status.current_name", " Current name: \"{{name}}\".", replacements));
                    status_is_error = true;
                }
            }

            keyboard_state.pressed_confirm = false;
            rename_confirm_now = false;
        }

        if (keyboard_state.pressed_delete)
        {
            if (selected_slot != ft_nullptr)
            {
                if (delete_confirmation_active && selected_slot->identifier == delete_confirmation_identifier)
                    confirm_delete_now = true;
                else
                    begin_delete_confirmation(*selected_slot);
            }
            else
            {
                if (!slots.empty())
                {
                    status_message = menu_localize(
                        "load_menu.status.delete_select", "Select a save before pressing Delete.");
                    status_is_error = true;
                }
                clear_delete_confirmation();
            }
            keyboard_state.pressed_delete = false;
        }

        if (delete_confirmation_active && keyboard_state.pressed_confirm)
        {
            if (selected_slot != ft_nullptr && selected_slot->identifier == delete_confirmation_identifier)
                confirm_delete_now = true;
            keyboard_state.pressed_confirm = false;
        }

        if (confirm_delete_now)
        {
            ft_string delete_error;
            bool deleted = delete_save_slot(commander_name, delete_confirmation_label, delete_error);
            if (deleted)
            {
                ft_vector<StringTableReplacement> replacements;
                replacements.reserve(1U);
                StringTableReplacement replacement;
                replacement.key = ft_string("slot");
                replacement.value = delete_confirmation_label;
                replacements.push_back(replacement);
                status_message = menu_localize_format(
                    "load_menu.status.delete_success", "Deleted save \"{{slot}}\".", replacements);
                status_is_error = false;
            }
            else
            {
                if (delete_error.empty())
                    status_message
                        = menu_localize("load_menu.error.delete_failed", "Failed to delete save file.");
                else
                    status_message = delete_error;
                status_is_error = true;
            }

            clear_delete_confirmation();

            bool refreshed = collect_save_slots(commander_name, slots);
            if (!refreshed)
            {
                status_message = menu_localize(
                    "load_menu.status.refresh_failed_delete",
                    "Unable to refresh save directory after deletion.");
                status_is_error = true;
                slots.clear();
            }

            rebuild_load_menu(menu, slots, !slots.empty());
            if (slots.empty() && refreshed)
            {
                status_message = menu_localize(
                    "load_menu.status.no_saves", "No campaign saves found for this commander.");
                status_is_error = false;
            }

            skip_menu_activation = true;
        }

        auto process_selection = [&](const ft_menu_item &item) {
            if (item.identifier == "action:cancel")
            {
                running = false;
                return;
            }
            if (item.identifier.compare(0, 5, "save:") == 0)
            {
                for (size_t index = 0; index < slots.size(); ++index)
                {
                    if (slots[index].identifier == item.identifier)
                    {
                        out_selected_save = slots[index].file_path;
                        loaded = true;
                        running = false;
                        return;
                    }
                }
            }
        };

        if (!skip_menu_activation)
        {
            if (mouse_state.left_released)
            {
                const int hovered_index = menu.get_hovered_index();
                const ft_menu_item *hovered_item = menu_item_from_index(menu, hovered_index);
                if (hovered_item != ft_nullptr)
                    process_selection(*hovered_item);
            }

            const ft_menu_item *selected_item = menu.get_selected_item();
            if (keyboard_state.pressed_confirm && selected_item != ft_nullptr)
                process_selection(*selected_item);
        }
        else
            keyboard_state.pressed_confirm = false;

        int window_width = 0;
        int window_height = 0;
        SDL_GetWindowSize(window, &window_width, &window_height);
        (void)window_width;
        (void)window_height;

        render_load_screen(*renderer, menu, title_font, menu_font, commander_name, status_message, status_is_error);
        SDL_RenderPresent(renderer);
    }

    if (rename_active)
        SDL_StopTextInput();

    return loaded;
#endif
}

namespace load_game_flow_testing
{
    ft_vector<ft_string> collect_save_slot_names(const ft_string &commander_name)
    {
        ft_vector<save_slot_entry> slots;
        ft_vector<ft_string>        names;
        if (!collect_save_slots(commander_name, slots))
            return names;
        names.reserve(slots.size());
        for (size_t index = 0; index < slots.size(); ++index)
            names.push_back(slots[index].label);
        return names;
    }

    ft_vector<ft_string> collect_save_slot_metadata_labels(const ft_string &commander_name)
    {
        ft_vector<save_slot_entry> slots;
        ft_vector<ft_string>        labels;
        if (!collect_save_slots(commander_name, slots))
            return labels;
        labels.reserve(slots.size());
        for (size_t index = 0; index < slots.size(); ++index)
            labels.push_back(slots[index].metadata_label);
        return labels;
    }

    ft_string resolve_save_file_path(const ft_string &commander_name, const ft_string &slot_name)
    {
        if (commander_name.empty() || slot_name.empty())
            return ft_string();
        return build_save_file_path(commander_name, slot_name);
    }

    ft_vector<ft_string> collect_save_slot_errors(const ft_string &commander_name)
    {
        ft_vector<save_slot_entry> slots;
        ft_vector<ft_string>        errors;
        if (!collect_save_slots(commander_name, slots, &errors))
            errors.clear();
        return errors;
    }

    bool delete_save_slot(const ft_string &commander_name, const ft_string &slot_name, ft_string &out_error)
    {
        return ::delete_save_slot(commander_name, slot_name, out_error);
    }

    bool rename_save_slot(const ft_string &commander_name, const ft_string &old_name, const ft_string &new_name,
        ft_string &out_error)
    {
        return ::rename_save_slot(commander_name, old_name, new_name, out_error);
    }
}
