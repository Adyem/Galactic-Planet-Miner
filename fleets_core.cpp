int ft_fleet::_next_ship_id = 1;

ft_fleet::ft_fleet() noexcept : _id(0), _travel_time(0), _escort_veterancy(0.0)
{
    return ;
}

ft_fleet::ft_fleet(int id) noexcept : _id(id), _travel_time(0), _escort_veterancy(0.0)
{
    return ;
}

ft_ship *ft_fleet::find_ship(int ship_uid) noexcept
{
    Pair<int, ft_ship> *entry = this->_ships.find(ship_uid);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
}

const ft_ship *ft_fleet::find_ship(int ship_uid) const noexcept
{
    const Pair<int, ft_ship> *entry = this->_ships.find(ship_uid);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
}

int ft_fleet::get_id() const noexcept
{
    return this->_id;
}

int ft_fleet::get_ship_count() const noexcept
{
    return static_cast<int>(this->_ships.size());
}

int ft_fleet::create_ship(int ship_type) noexcept
{
    if (_next_ship_id >= FT_INT_MAX)
        return 0;
    int uid = _next_ship_id++;
    ft_ship ship(uid, ship_type);
    assign_ship_defaults(ship);
    this->_ships.insert(uid, ship);
    return uid;
}

void ft_fleet::clear_ships() noexcept
{
    this->_ships.clear();
}

void ft_fleet::add_ship_snapshot(const ft_ship &ship) noexcept
{
    ft_ship snapshot = ship;
    ship_profile profile = get_ship_profile(snapshot.type);
    if (snapshot.max_hp <= 0)
        snapshot.max_hp = profile.hp;
    if (snapshot.max_hp <= 0)
        snapshot.max_hp = 0;
    if (snapshot.max_hp <= 0)
        snapshot.hp = 0;
    else
    {
        if (snapshot.hp > snapshot.max_hp)
            snapshot.hp = snapshot.max_hp;
        else if (snapshot.hp < 0)
            snapshot.hp = 0;
    }
    if (snapshot.max_shield <= 0)
        snapshot.max_shield = profile.shield;
    if (snapshot.max_shield <= 0)
        snapshot.max_shield = 0;
    if (snapshot.max_shield <= 0)
        snapshot.shield = 0;
    else
    {
        if (snapshot.shield > snapshot.max_shield)
            snapshot.shield = snapshot.max_shield;
        else if (snapshot.shield < 0)
            snapshot.shield = 0;
    }
    if (snapshot.max_speed <= 0.0)
        snapshot.max_speed = profile.max_speed;
    if (snapshot.acceleration <= 0.0)
        snapshot.acceleration = profile.acceleration;
    if (snapshot.deceleration <= 0.0)
        snapshot.deceleration = profile.deceleration;
    if (snapshot.turn_speed <= 0.0)
        snapshot.turn_speed = profile.turn_speed;
    if (snapshot.optimal_range <= 0.0)
        snapshot.optimal_range = profile.optimal_range;
    if (snapshot.max_range <= 0.0)
        snapshot.max_range = profile.max_range;
    if (snapshot.attack_speed <= 0.0)
        snapshot.attack_speed = profile.attack_speed;
    if (snapshot.base_damage <= 0.0)
        snapshot.base_damage = profile.base_damage;
    if (snapshot.combat_behavior == 0)
        snapshot.combat_behavior = profile.combat_behavior;
    if (snapshot.outnumbered_behavior == 0)
        snapshot.outnumbered_behavior = profile.outnumbered_behavior;
    if (snapshot.unescorted_behavior == 0)
        snapshot.unescorted_behavior = profile.unescorted_behavior;
    if (snapshot.low_hp_behavior == 0)
        snapshot.low_hp_behavior = profile.low_hp_behavior;
    if (snapshot.role == 0)
        snapshot.role = profile.role;
    this->_ships.insert(snapshot.id, snapshot);
    if (snapshot.id >= _next_ship_id)
    {
        if (snapshot.id >= FT_INT_MAX - 1)
            _next_ship_id = FT_INT_MAX;
        else
        {
            int next_id = snapshot.id + 1;
            if (next_id <= snapshot.id || next_id >= FT_INT_MAX)
                _next_ship_id = FT_INT_MAX;
            else
                _next_ship_id = next_id;
        }
    }
}

void ft_fleet::remove_ship(int ship_uid) noexcept
{
    this->_ships.remove(ship_uid);
}

bool ft_fleet::move_ship_to(ft_fleet &target, int ship_uid) noexcept
{
    ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return false;
    target._ships.insert(ship_uid, *ship);
    this->_ships.remove(ship_uid);
    return true;
}

void ft_fleet::move_ships_to(ft_fleet &target) noexcept
{
    while (this->_ships.size() > 0)
    {
        Pair<int, ft_ship> *entry = this->_ships.end();
        entry -= 1;
        target._ships.insert(entry->key, entry->value);
        this->_ships.remove(entry->key);
    }
}

void ft_fleet::get_ship_ids(ft_vector<int> &out) const noexcept
{
    out.clear();
    size_t count = this->_ships.size();
    if (count == 0)
        return ;
    const Pair<int, ft_ship> *entries = this->_ships.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
        out.push_back(entries[i].key);
}

int ft_fleet::get_ship_type(int ship_uid) const noexcept
{
    const ft_ship *ship = this->find_ship(ship_uid);
    if (ship == ft_nullptr)
        return 0;
    return ship->type;
}

const ft_ship *ft_fleet::get_ship(int ship_uid) const noexcept
{
    return this->find_ship(ship_uid);
}
