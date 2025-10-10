bool Game::is_backend_online() const
{
    return this->_backend_online;
}

long Game::get_backend_retry_delay_ms_for_testing() const
{
    return this->_backend_retry_delay_ms;
}

long Game::get_backend_next_retry_ms_for_testing() const
{
    return this->_backend_next_retry_ms;
}
