#include "string_table.hpp"

#include "libft/Errno/errno.hpp"
#include "libft/Template/pair.hpp"
#include "libft/YAML/yaml.hpp"

namespace
{
    void copy_range_to_string(ft_string &target, const char *begin, const char *end) noexcept
    {
        target.clear();
        while (begin < end)
        {
            target.append(*begin);
            ++begin;
        }
    }

    void append_range_to_string(ft_string &target, const char *begin, const char *end) noexcept
    {
        while (begin < end)
        {
            target.append(*begin);
            ++begin;
        }
    }

    bool flatten_yaml_node(const yaml_value *node, const ft_string &prefix, ft_map<ft_string, ft_string> &out_entries) noexcept
    {
        if (node == ft_nullptr)
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return false;
        }

        const yaml_type type = node->get_type();
        if (type == YAML_SCALAR)
        {
            if (prefix.empty())
                return true;
            out_entries.insert(prefix, node->get_scalar());
            return true;
        }

        if (type == YAML_MAP)
        {
            const ft_map<ft_string, yaml_value*> &map = node->get_map();
            const ft_vector<ft_string> &keys = node->get_map_keys();
            for (size_t index = 0; index < keys.size(); ++index)
            {
                const ft_string &child_key = keys[index];
                const Pair<ft_string, yaml_value*> *entry = map.find(child_key);
                if (entry == ft_nullptr)
                    continue;

                ft_string composite_key;
                if (!prefix.empty())
                {
                    composite_key = prefix;
                    composite_key.append('.');
                    composite_key.append(child_key);
                }
                else
                    composite_key = child_key;

                if (!flatten_yaml_node(entry->value, composite_key, out_entries))
                    return false;
            }
            return true;
        }

        if (type == YAML_LIST)
        {
            const ft_vector<yaml_value*> &items = node->get_list();
            for (size_t index = 0; index < items.size(); ++index)
            {
                ft_string composite_key;
                if (!prefix.empty())
                {
                    composite_key = prefix;
                    composite_key.append('.');
                }
                ft_string index_string = ft_to_string(static_cast<long>(index));
                composite_key.append(index_string);
                if (!flatten_yaml_node(items[index], composite_key, out_entries))
                    return false;
            }
            return true;
        }

        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return false;
    }
}

bool string_table_load_from_yaml(const ft_string &file_path, StringTable &out_table) noexcept
{
    out_table.language.clear();
    out_table.entries.clear();

    yaml_value *root = yaml_read_from_file(file_path.c_str());
    if (root == ft_nullptr)
        return false;

    const yaml_value *strings_node = root;

    if (root->get_type() == YAML_MAP)
    {
        const ft_map<ft_string, yaml_value*> &root_map = root->get_map();
        const Pair<ft_string, yaml_value*> *language_entry = root_map.find(ft_string("language"));
        if (language_entry != ft_nullptr && language_entry->value != ft_nullptr)
        {
            if (language_entry->value->get_type() == YAML_SCALAR)
                out_table.language = language_entry->value->get_scalar();
        }

        const Pair<ft_string, yaml_value*> *strings_entry = root_map.find(ft_string("strings"));
        if (strings_entry != ft_nullptr && strings_entry->value != ft_nullptr)
            strings_node = strings_entry->value;
    }

    bool success = flatten_yaml_node(strings_node, ft_string(), out_table.entries);
    yaml_free(root);

    if (!success)
    {
        out_table.entries.clear();
        out_table.language.clear();
        return false;
    }

    ft_errno = ER_SUCCESS;
    return true;
}

const ft_string *string_table_find(const StringTable &table, const ft_string &key) noexcept
{
    const Pair<ft_string, ft_string> *entry = table.entries.find(key);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
}

ft_string string_table_format(const ft_string &pattern, const ft_vector<StringTableReplacement> &replacements) noexcept
{
    ft_string result;

    const char *pattern_data = pattern.c_str();
    const size_t pattern_length = pattern.size();
    size_t index = 0;

    while (index < pattern_length)
    {
        size_t open_index = index;
        bool   open_found = false;
        while (open_index + 1 < pattern_length)
        {
            if (pattern_data[open_index] == '{' && pattern_data[open_index + 1] == '{')
            {
                open_found = true;
                break;
            }
            ++open_index;
        }

        if (!open_found)
        {
            append_range_to_string(result, pattern_data + index, pattern_data + pattern_length);
            break;
        }

        append_range_to_string(result, pattern_data + index, pattern_data + open_index);

        size_t close_index = open_index + 2;
        bool   close_found = false;
        while (close_index + 1 < pattern_length)
        {
            if (pattern_data[close_index] == '}' && pattern_data[close_index + 1] == '}')
            {
                close_found = true;
                break;
            }
            ++close_index;
        }

        if (!close_found)
        {
            append_range_to_string(result, pattern_data + open_index, pattern_data + pattern_length);
            break;
        }

        ft_string placeholder_key;
        copy_range_to_string(placeholder_key, pattern_data + open_index + 2, pattern_data + close_index);

        const ft_string *replacement_value = ft_nullptr;
        for (size_t replacement_index = 0; replacement_index < replacements.size(); ++replacement_index)
        {
            const StringTableReplacement &replacement = replacements[replacement_index];
            if (replacement.key == placeholder_key)
            {
                replacement_value = &replacement.value;
                break;
            }
        }

        if (replacement_value != ft_nullptr)
            result.append(*replacement_value);

        index = close_index + 2;
    }

    return result;
}
