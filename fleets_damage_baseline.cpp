double ft_fleet::get_ship_damage_baseline(int ship_type) noexcept
{
    ship_profile profile = get_ship_profile(ship_type);
    return profile.base_damage;
}

bool is_capital_ship_type(int ship_type) noexcept
{
    switch (ship_type)
    {
    case SHIP_CAPITAL_JUGGERNAUT:
    case SHIP_CAPITAL_NOVA:
    case SHIP_CAPITAL_OBSIDIAN:
    case SHIP_CAPITAL_PREEMPTOR:
    case SHIP_CAPITAL_PROTECTOR:
    case SHIP_CAPITAL_ECLIPSE:
    case SHIP_RAIDER_BATTLESHIP:
        return true;
    default:
        break;
    }
    return false;
}
