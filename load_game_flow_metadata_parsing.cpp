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
