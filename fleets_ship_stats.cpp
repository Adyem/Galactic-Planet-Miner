void ft_fleet::set_ship_armor(int ship_uid, int value) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return ;
    ship->armor = value;
}

int ft_fleet::get_ship_armor(int ship_uid) const noexcept
{
    const ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    return ship->armor;
}

int ft_fleet::add_ship_armor(int ship_uid, int amount) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    ship->armor += amount;
    return ship->armor;
}

int ft_fleet::sub_ship_armor(int ship_uid, int amount) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    ship->armor -= amount;
    if (ship->armor < 0)
        ship->armor = 0;
    return ship->armor;
}

void ft_fleet::set_ship_hp(int ship_uid, int value) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return ;
    ship->hp = value;
    if (value > ship->max_hp)
        ship->max_hp = value;
    if (ship->max_hp < 0)
        ship->max_hp = 0;
}

int ft_fleet::get_ship_hp(int ship_uid) const noexcept
{
    const ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    return ship->hp;
}

int ft_fleet::add_ship_hp(int ship_uid, int amount) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    ship->hp += amount;
    if (ship->max_hp < ship->hp)
        ship->max_hp = ship->hp;
    if (ship->hp < 0)
        ship->hp = 0;
    return ship->hp;
}

int ft_fleet::sub_ship_hp(int ship_uid, int amount) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    ship->hp -= amount;
    if (ship->hp < 0)
        ship->hp = 0;
    return ship->hp;
}

void ft_fleet::set_ship_shield(int ship_uid, int value) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return ;
    ship->shield = value;
    if (value > ship->max_shield)
        ship->max_shield = value;
    if (ship->max_shield < 0)
        ship->max_shield = 0;
}

int ft_fleet::get_ship_shield(int ship_uid) const noexcept
{
    const ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    return ship->shield;
}

int ft_fleet::add_ship_shield(int ship_uid, int amount) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    ship->shield += amount;
    if (ship->max_shield < ship->shield)
        ship->max_shield = ship->shield;
    if (ship->shield < 0)
        ship->shield = 0;
    return ship->shield;
}

int ft_fleet::sub_ship_shield(int ship_uid, int amount) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    ship->shield -= amount;
    if (ship->shield < 0)
        ship->shield = 0;
    return ship->shield;
}
