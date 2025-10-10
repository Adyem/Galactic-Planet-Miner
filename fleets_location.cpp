void ft_fleet::set_location_planet(int planet_id) noexcept
{
    this->_loc.type = LOCATION_PLANET;
    this->_loc.from = planet_id;
    this->_loc.to = planet_id;
    this->_loc.misc = 0;
    this->_travel_time = 0;
}

void ft_fleet::set_location_travel(int from, int to, double time) noexcept
{
    this->_loc.type = LOCATION_TRAVEL;
    this->_loc.from = from;
    this->_loc.to = to;
    this->_loc.misc = 0;
    this->_travel_time = time;
}

void ft_fleet::set_location_misc(int misc_id) noexcept
{
    this->_loc.type = LOCATION_MISC;
    this->_loc.from = 0;
    this->_loc.to = 0;
    this->_loc.misc = misc_id;
    this->_travel_time = 0;
}

ft_location ft_fleet::get_location() const noexcept
{
    return this->_loc;
}

double ft_fleet::get_travel_time() const noexcept
{
    return this->_travel_time;
}

void ft_fleet::tick(double seconds) noexcept
{
    if (this->_loc.type == LOCATION_TRAVEL)
    {
        if (this->_travel_time > seconds)
            this->_travel_time -= seconds;
        else
        {
            this->set_location_planet(this->_loc.to);
        }
    }
}
