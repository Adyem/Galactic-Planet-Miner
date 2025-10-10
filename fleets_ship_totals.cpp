int ft_fleet::get_total_ship_hp() const noexcept
{
    size_t count = this->_ships.size();
    if (count == 0)
        return 0;
    const Pair<int, ft_ship> *entries = this->_ships.end();
    entries -= count;
    int total = 0;
    for (size_t i = 0; i < count; ++i)
        total += entries[i].value.hp;
    return total;
}

int ft_fleet::get_total_ship_shield() const noexcept
{
    size_t count = this->_ships.size();
    if (count == 0)
        return 0;
    const Pair<int, ft_ship> *entries = this->_ships.end();
    entries -= count;
    int total = 0;
    for (size_t i = 0; i < count; ++i)
        total += entries[i].value.shield;
    return total;
}
