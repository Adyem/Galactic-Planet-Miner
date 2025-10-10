double ft_fleet::absorb_damage(double damage, double shield_multiplier, double hull_multiplier) noexcept
{
    if (damage <= 0.0)
        return 0.0;
    if (shield_multiplier < 1.0)
        shield_multiplier = 1.0;
    if (hull_multiplier < 1.0)
        hull_multiplier = 1.0;
    int total_damage = static_cast<int>(damage + 0.5);
    if (total_damage <= 0)
        return 0.0;
    ft_vector<ft_ship*> active;
    size_t count = this->_ships.size();
    Pair<int, ft_ship> *entries = this->_ships.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        ft_ship &ship = entries[i].value;
        if (ship.hp > 0 || ship.shield > 0)
            active.push_back(&ship);
    }
    if (active.size() == 0)
        return static_cast<double>(total_damage);
    for (size_t i = 0; i < active.size() && total_damage > 0; ++i)
    {
        ft_ship *ship = active[i];
        size_t defenders_left = active.size() - i;
        int share = total_damage / static_cast<int>(defenders_left);
        if (share <= 0)
            share = 1;
        if (share > total_damage)
            share = total_damage;
        total_damage -= share;
        double incoming = static_cast<double>(share);
        int available_shield = ship->shield;
        if (available_shield > 0)
        {
            double ratio = incoming / shield_multiplier;
            int reduction_units = static_cast<int>(ratio);
            if (static_cast<double>(reduction_units) < ratio)
                reduction_units += 1;
            if (reduction_units > available_shield)
                reduction_units = available_shield;
            ship->shield -= reduction_units;
            double absorbed = static_cast<double>(reduction_units) * shield_multiplier;
            if (absorbed > incoming)
                absorbed = incoming;
            incoming -= absorbed;
            if (incoming < 0.0)
                incoming = 0.0;
        }
        if (incoming <= 0.0)
            continue;
        double base_damage = incoming / hull_multiplier;
        double reduction = static_cast<double>(ship->armor) * 0.01;
        if (reduction > 0.6)
            reduction = 0.6;
        double hull_damage_d = base_damage * (1.0 - reduction);
        int hull_damage = static_cast<int>(hull_damage_d + 0.5);
        if (hull_damage <= 0 && ship->hp > 0)
            hull_damage = 1;
        if (hull_damage > ship->hp)
            hull_damage = ship->hp;
        ship->hp -= hull_damage;
    }
    return static_cast<double>(total_damage);
}

void ft_fleet::apply_support(int shield_amount, int repair_amount) noexcept
{
    if (shield_amount <= 0 && repair_amount <= 0)
        return ;
    ft_vector<ft_ship*> shield_targets;
    ft_vector<ft_ship*> repair_targets;
    size_t count = this->_ships.size();
    Pair<int, ft_ship> *entries = this->_ships.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        ft_ship &ship = entries[i].value;
        if (ship.hp > 0 || ship.shield > 0)
            shield_targets.push_back(&ship);
        if (ship.hp > 0)
            repair_targets.push_back(&ship);
    }
    int shield_remaining = shield_amount;
    for (size_t i = 0; i < shield_targets.size() && shield_remaining > 0; ++i)
    {
        ft_ship *ship = shield_targets[i];
        if (ship->max_shield > 0 && ship->shield >= ship->max_shield)
            continue;
        size_t remain = shield_targets.size() - i;
        int share = shield_remaining / static_cast<int>(remain);
        if (share <= 0)
            share = 1;
        if (share > shield_remaining)
            share = shield_remaining;
        if (ship->max_shield > 0)
        {
            int capacity = ship->max_shield - ship->shield;
            if (capacity <= 0)
                continue;
            if (share > capacity)
                share = capacity;
        }
        ship->shield += share;
        shield_remaining -= share;
    }
    int repair_remaining = repair_amount;
    for (size_t i = 0; i < repair_targets.size() && repair_remaining > 0; ++i)
    {
        ft_ship *ship = repair_targets[i];
        if (ship->max_hp > 0 && ship->hp >= ship->max_hp)
            continue;
        size_t remain = repair_targets.size() - i;
        int share = repair_remaining / static_cast<int>(remain);
        if (share <= 0)
            share = 1;
        if (share > repair_remaining)
            share = repair_remaining;
        if (ship->max_hp > 0)
        {
            int capacity = ship->max_hp - ship->hp;
            if (capacity <= 0)
                continue;
            if (share > capacity)
                share = capacity;
        }
        ship->hp += share;
        repair_remaining -= share;
    }
}

int ft_fleet::apply_targeted_shield(int ship_uid, int amount) noexcept
{
    if (amount <= 0)
        return 0;
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    if (ship->max_shield <= 0)
        return 0;
    if (ship->shield >= ship->max_shield)
        return 0;
    if (ship->hp <= 0 && ship->shield <= 0)
        return 0;
    int capacity = ship->max_shield - ship->shield;
    if (capacity <= 0)
        return 0;
    int applied = amount;
    if (applied > capacity)
        applied = capacity;
    ship->shield += applied;
    return applied;
}

bool ft_fleet::has_operational_ships() const noexcept
{
    size_t count = this->_ships.size();
    if (count == 0)
        return false;
    const Pair<int, ft_ship> *entries = this->_ships.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        const ft_ship &ship = entries[i].value;
        if (ship.hp > 0 || ship.shield > 0)
            return true;
    }
    return false;
}
