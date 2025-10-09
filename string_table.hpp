#pragma once

#include "libft/Libft/libft.hpp"
#include "libft/Template/map.hpp"
#include "libft/Template/vector.hpp"

struct StringTable
{
    ft_string                     language;
    ft_map<ft_string, ft_string>  entries;

    StringTable() noexcept : language(), entries() {}
};

struct StringTableReplacement
{
    ft_string key;
    ft_string value;

    StringTableReplacement() noexcept : key(), value() {}
};

bool string_table_load_from_yaml(const ft_string &file_path, StringTable &out_table) noexcept;
const ft_string *string_table_find(const StringTable &table, const ft_string &key) noexcept;
ft_string string_table_format(const ft_string &pattern, const ft_vector<StringTableReplacement> &replacements) noexcept;
