#include "main_menu_system.hpp"

#include "libft/File/file_utils.hpp"
#include "libft/Libft/libft.hpp"

namespace
{
    struct commander_portrait_cache_entry
    {
        ft_string commander_key;
        ft_string asset_path;
        bool      attempted;
        bool      loaded;
        size_t    cached_size;

        commander_portrait_cache_entry() noexcept
            : commander_key(), asset_path(), attempted(false), loaded(false), cached_size(0)
        {}
    };

    ft_vector<commander_portrait_cache_entry> g_commander_portrait_cache;

    bool portrait_candidate_exists(const ft_vector<ft_string> &names, const ft_string &candidate) noexcept
    {
        for (size_t index = 0; index < names.size(); ++index)
        {
            if (names[index] == candidate)
                return true;
        }
        return false;
    }

    void portrait_add_candidate(ft_vector<ft_string> &names, const ft_string &candidate) noexcept
    {
        if (candidate.empty())
            return;
        if (!portrait_candidate_exists(names, candidate))
            names.push_back(candidate);
    }

    ft_string portrait_build_lowercase(const ft_string &value) noexcept
    {
        ft_string lower(value);
        char     *mutable_text = lower.print();
        if (mutable_text != ft_nullptr)
            ft_to_lower(mutable_text);
        return lower;
    }

    ft_string portrait_build_underscored(const ft_string &value) noexcept
    {
        ft_string    result;
        const char  *raw = value.c_str();
        if (raw == ft_nullptr)
            return result;
        for (size_t index = 0; raw[index] != '\0'; ++index)
        {
            const char character = raw[index];
            if (character == ' ' || character == '-' || character == '.' || character == '\\')
                result.append('_');
            else
                result.append(character);
        }
        return result;
    }

    ft_string portrait_resolve_commander_key(const ft_string &commander_name) noexcept
    {
        ft_string    path = player_profile_resolve_path(commander_name);
        const char  *raw_path = path.c_str();
        if (raw_path == ft_nullptr || *raw_path == '\0')
            return ft_string("Commander");

        const char *start = raw_path;
        const char *forward_slash = ft_strrchr(raw_path, '/');
        if (forward_slash != ft_nullptr && *(forward_slash + 1) != '\0')
            start = forward_slash + 1;
        const char *backslash = ft_strrchr(raw_path, '\\');
        if (backslash != ft_nullptr && *(backslash + 1) != '\0' && backslash > start)
            start = backslash + 1;

        const char *end = ft_strrchr(start, '.');
        if (end == ft_nullptr || end <= start)
            end = start + ft_strlen(start);

        ft_string key;
        while (start < end && *start != '\0')
        {
            key.append(*start);
            start += 1;
        }

        if (key.empty())
            key = ft_string("Commander");
        return key;
    }

    struct commander_portrait_resolution
    {
        ft_string commander_key;
        ft_string resolved_path;
        ft_string primary_candidate;
        bool      found_existing;

        commander_portrait_resolution() noexcept
            : commander_key(), resolved_path(), primary_candidate(), found_existing(false)
        {}
    };

    commander_portrait_resolution resolve_commander_portrait_resolution(const ft_string &commander_name) noexcept
    {
        commander_portrait_resolution result;
        result.commander_key = portrait_resolve_commander_key(commander_name);

        ft_vector<ft_string> candidates;
        portrait_add_candidate(candidates, result.commander_key);
        portrait_add_candidate(candidates, portrait_build_lowercase(result.commander_key));
        ft_string underscored = portrait_build_underscored(result.commander_key);
        portrait_add_candidate(candidates, underscored);
        portrait_add_candidate(candidates, portrait_build_lowercase(underscored));

        const char *directories[] = {"assets/portraits", "portraits", "data/portraits", "data/assets/portraits"};
        const size_t directory_count = sizeof(directories) / sizeof(directories[0]);
        const char *extensions[] = {".png", ".jpg", ".jpeg", ".bmp"};
        const size_t extension_count = sizeof(extensions) / sizeof(extensions[0]);

        bool primary_set = false;
        for (size_t dir_index = 0; dir_index < directory_count; ++dir_index)
        {
            const char *directory = directories[dir_index];
            if (directory == ft_nullptr)
                continue;
            for (size_t name_index = 0; name_index < candidates.size(); ++name_index)
            {
                const ft_string &name = candidates[name_index];
                if (name.empty())
                    continue;
                for (size_t ext_index = 0; ext_index < extension_count; ++ext_index)
                {
                    const char *extension = extensions[ext_index];
                    if (extension == ft_nullptr)
                        continue;

                    ft_string candidate_path(directory);
                    candidate_path.append("/");
                    candidate_path.append(name);
                    candidate_path.append(extension);

                    if (!primary_set)
                    {
                        result.primary_candidate = candidate_path;
                        primary_set = true;
                    }

                    int exists_result = file_exists(candidate_path.c_str());
                    if (exists_result > 0)
                    {
                        result.resolved_path = candidate_path;
                        result.found_existing = true;
                        return result;
                    }
                }
            }
        }

        if (!result.primary_candidate.empty())
            result.resolved_path = result.primary_candidate;
        else
            result.resolved_path = ft_string("assets/portraits/default.png");
        return result;
    }

    commander_portrait_cache_entry *find_portrait_cache_entry(const ft_string &commander_key) noexcept
    {
        for (size_t index = 0; index < g_commander_portrait_cache.size(); ++index)
        {
            if (g_commander_portrait_cache[index].commander_key == commander_key)
                return &g_commander_portrait_cache[index];
        }
        return ft_nullptr;
    }

    bool load_portrait_size(const ft_string &path, size_t &out_size) noexcept
    {
        out_size = 0U;
        if (path.empty())
            return false;

        FILE *file = ft_fopen(path.c_str(), "rb");
        if (file == ft_nullptr)
            return false;

        unsigned char buffer[4096];
        while (true)
        {
            size_t read_count = fread(buffer, 1, sizeof(buffer), file);
            if (read_count > 0U)
                out_size += read_count;
            if (read_count < sizeof(buffer))
            {
                if (ferror(file))
                {
                    ft_fclose(file);
                    out_size = 0U;
                    return false;
                }
                break;
            }
        }

        ft_fclose(file);
        return true;
    }
}

bool main_menu_preload_commander_portrait(const ft_string &commander_name) noexcept
{
    commander_portrait_resolution resolution = resolve_commander_portrait_resolution(commander_name);

    commander_portrait_cache_entry *entry = find_portrait_cache_entry(resolution.commander_key);
    if (entry == ft_nullptr)
    {
        commander_portrait_cache_entry new_entry;
        new_entry.commander_key = resolution.commander_key;
        g_commander_portrait_cache.push_back(new_entry);
        size_t cache_size = g_commander_portrait_cache.size();
        if (cache_size == 0U)
            return false;
        entry = &g_commander_portrait_cache[cache_size - 1U];
    }

    if (entry->attempted)
        return entry->loaded;

    entry->attempted = true;
    entry->asset_path = resolution.resolved_path;
    entry->cached_size = 0U;

    size_t loaded_size = 0U;
    if (load_portrait_size(resolution.resolved_path, loaded_size))
    {
        entry->cached_size = loaded_size;
        entry->loaded = true;
        return true;
    }

    entry->loaded = false;
    entry->cached_size = 0U;
    return false;
}

namespace main_menu_testing
{
    void reset_commander_portrait_cache()
    {
        g_commander_portrait_cache.clear();
    }

    bool commander_portrait_attempted(const ft_string &commander_name)
    {
        commander_portrait_cache_entry *entry
            = find_portrait_cache_entry(portrait_resolve_commander_key(commander_name));
        if (entry == ft_nullptr)
            return false;
        return entry->attempted;
    }

    bool commander_portrait_loaded(const ft_string &commander_name)
    {
        commander_portrait_cache_entry *entry
            = find_portrait_cache_entry(portrait_resolve_commander_key(commander_name));
        if (entry == ft_nullptr)
            return false;
        return entry->loaded;
    }

    size_t commander_portrait_cached_size(const ft_string &commander_name)
    {
        commander_portrait_cache_entry *entry
            = find_portrait_cache_entry(portrait_resolve_commander_key(commander_name));
        if (entry == ft_nullptr)
            return 0U;
        return entry->cached_size;
    }

    ft_string resolve_cached_portrait_path(const ft_string &commander_name)
    {
        commander_portrait_cache_entry *entry
            = find_portrait_cache_entry(portrait_resolve_commander_key(commander_name));
        if (entry == ft_nullptr)
            return ft_string();
        return entry->asset_path;
    }

    ft_string resolve_commander_portrait_filename(const ft_string &commander_name)
    {
        return portrait_resolve_commander_key(commander_name);
    }
}
