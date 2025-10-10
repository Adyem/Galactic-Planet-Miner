namespace
{
    const double ESCORT_VETERANCY_PER_BONUS = 60.0;
    const int    ESCORT_VETERANCY_MAX_BONUS = 8;
    const double ESCORT_VETERANCY_MAX_XP =
        ESCORT_VETERANCY_PER_BONUS * static_cast<double>(ESCORT_VETERANCY_MAX_BONUS);
}

double ft_fleet::get_escort_veterancy() const noexcept
{
    if (this->_escort_veterancy < 0.0)
        return 0.0;
    return this->_escort_veterancy;
}

void ft_fleet::set_escort_veterancy(double value) noexcept
{
    if (value < 0.0)
        value = 0.0;
    if (value > ESCORT_VETERANCY_MAX_XP)
        value = ESCORT_VETERANCY_MAX_XP;
    this->_escort_veterancy = value;
}

int ft_fleet::get_escort_veterancy_bonus() const noexcept
{
    if (this->_escort_veterancy <= 0.0)
        return 0;
    double xp = this->_escort_veterancy;
    if (xp > ESCORT_VETERANCY_MAX_XP)
        xp = ESCORT_VETERANCY_MAX_XP;
    int bonus = static_cast<int>(xp / ESCORT_VETERANCY_PER_BONUS);
    if (bonus < 0)
        bonus = 0;
    if (bonus > ESCORT_VETERANCY_MAX_BONUS)
        bonus = ESCORT_VETERANCY_MAX_BONUS;
    return bonus;
}

bool ft_fleet::add_escort_veterancy(double amount) noexcept
{
    if (amount <= 0.0)
        return false;
    int before = this->get_escort_veterancy_bonus();
    this->_escort_veterancy += amount;
    if (this->_escort_veterancy > ESCORT_VETERANCY_MAX_XP)
        this->_escort_veterancy = ESCORT_VETERANCY_MAX_XP;
    int after = this->get_escort_veterancy_bonus();
    return after > before;
}

bool ft_fleet::decay_escort_veterancy(double amount) noexcept
{
    if (amount <= 0.0)
        return false;
    int before = this->get_escort_veterancy_bonus();
    this->_escort_veterancy -= amount;
    if (this->_escort_veterancy < 0.0)
        this->_escort_veterancy = 0.0;
    int after = this->get_escort_veterancy_bonus();
    return after < before;
}
