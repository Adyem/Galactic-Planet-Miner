#ifndef FT_MAP_SNAPSHOT_HPP
#define FT_MAP_SNAPSHOT_HPP

#include "libft/Template/map.hpp"
#include "libft/Template/vector.hpp"
#include "libft/Template/pair.hpp"

template <typename Key, typename Value>
void ft_map_snapshot(const ft_map<Key, Value> &map, ft_vector<Pair<Key, Value> > &out)
{
    size_t count = map.size();
    out.clear();
    if (count == 0)
        return;
    out.reserve(count);
    const Pair<Key, Value> *entries = map.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
        out.push_back(entries[i]);
}

#endif
