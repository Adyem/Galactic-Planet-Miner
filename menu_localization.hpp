#pragma once

#include "string_table.hpp"

ft_string menu_localize(const char *key, const char *fallback) noexcept;
ft_string menu_localize_format(
    const char *key, const char *fallback, const ft_vector<StringTableReplacement> &replacements) noexcept;
void      menu_localization_reset_for_testing() noexcept;
