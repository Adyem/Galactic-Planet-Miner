#include "menu_localization.hpp"

#include "libft/CPP_class/class_nullptr.hpp"
#include "libft/Errno/errno.hpp"
#include "libft/Printf/printf.hpp"

namespace
{
    bool         g_initialized = false;
    bool         g_available = false;
    StringTable  g_table;

    bool ensure_table_loaded() noexcept
    {
        if (!g_initialized)
        {
            ft_string path("strings/menu_en.yaml");
            g_available = string_table_load_from_yaml(path, g_table);
            g_initialized = true;
            if (!g_available)
            {
                const char *error_message = ft_strerror(ft_errno);
                if (error_message == ft_nullptr)
                    error_message = "unknown error";
                ft_fprintf(stderr, "[menu] Failed to load localization bundle \"%s\": %s\n", path.c_str(), error_message);
            }
        }
        return g_available;
    }
}

ft_string menu_localize(const char *key, const char *fallback) noexcept
{
    if (key == ft_nullptr)
    {
        if (fallback != ft_nullptr)
            return ft_string(fallback);
        return ft_string();
    }

    if (ensure_table_loaded())
    {
        const ft_string key_string(key);
        const ft_string *value = string_table_find(g_table, key_string);
        if (value != ft_nullptr)
            return *value;
    }

    if (fallback != ft_nullptr)
        return ft_string(fallback);
    return ft_string();
}

ft_string menu_localize_format(
    const char *key, const char *fallback, const ft_vector<StringTableReplacement> &replacements) noexcept
{
    ft_string pattern = menu_localize(key, fallback);
    return string_table_format(pattern, replacements);
}

void menu_localization_reset_for_testing() noexcept
{
    g_initialized = false;
    g_available = false;
    g_table.language.clear();
    g_table.entries.clear();
}
