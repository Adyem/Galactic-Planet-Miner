#ifndef GAME_TEST_MENU_SHARED_HPP
#define GAME_TEST_MENU_SHARED_HPP

#include "../libft/System_utils/test_runner.hpp"
#include "../libft/CPP_class/class_nullptr.hpp"
#include "../libft/CPP_class/class_ofstream.hpp"
#include "../libft/File/file_utils.hpp"
#include "../libft/File/open_dir.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Time/time.hpp"
#include "game_test_scenarios.hpp"
#include "main_menu_system.hpp"
#include "game_bootstrap.hpp"
#include "build_info.hpp"
#include "menu_localization.hpp"
#include "save_system_background.hpp"

#include <errno.h>

inline bool ensure_directory_exists(const char *path) noexcept
{
    if (path == ft_nullptr)
        return false;
    int result = file_create_directory(path, 0755);
    if (result == 0)
        return true;
    if (ft_errno == ERRNO_OFFSET + EEXIST)
        return true;
    return false;
}

inline ft_string build_expected_metadata_label(
    int day, int level, const char *difficulty_key, const char *difficulty_fallback)
{
    ft_vector<StringTableReplacement> value_replacements;
    value_replacements.reserve(1U);
    StringTableReplacement value_replacement;
    value_replacement.key = ft_string("value");
    value_replacement.value = ft_to_string(day);
    value_replacements.push_back(value_replacement);
    ft_string day_label
        = menu_localize_format("load_menu.metadata.day_known", "Day {{value}}", value_replacements);

    value_replacements[0].value = ft_to_string(level);
    ft_string level_label
        = menu_localize_format("load_menu.metadata.level_known", "Level {{value}}", value_replacements);

    ft_string difficulty_name = menu_localize(difficulty_key, difficulty_fallback);
    ft_vector<StringTableReplacement> difficulty_replacements;
    difficulty_replacements.reserve(1U);
    StringTableReplacement difficulty_replacement;
    difficulty_replacement.key = ft_string("difficulty");
    difficulty_replacement.value = difficulty_name;
    difficulty_replacements.push_back(difficulty_replacement);
    ft_string difficulty_label = menu_localize_format(
        "load_menu.metadata.difficulty_known", "Difficulty: {{difficulty}}", difficulty_replacements);

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

    StringTableReplacement combined_difficulty_replacement;
    combined_difficulty_replacement.key = ft_string("difficulty");
    combined_difficulty_replacement.value = difficulty_label;
    combined_replacements.push_back(combined_difficulty_replacement);

    return menu_localize_format(
        "load_menu.metadata.combined", "{{day}} • {{level}} • {{difficulty}}", combined_replacements);
}

#endif
