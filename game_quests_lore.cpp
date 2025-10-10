const ft_vector<ft_string> &Game::get_lore_log() const
{
    if (!this->_lore_log_cache_dirty)
        return this->_lore_log_cache;

    this->_lore_log_cache.clear();
    if (this->_lore_log_count == 0)
    {
        this->_lore_log_cache_dirty = false;
        return this->_lore_log_cache;
    }

    size_t buffer_size = this->_lore_log.size();
    if (buffer_size == 0)
    {
        this->_lore_log_cache_dirty = false;
        return this->_lore_log_cache;
    }

    this->_lore_log_cache.reserve(this->_lore_log_count);
    for (size_t i = 0; i < this->_lore_log_count; ++i)
    {
        size_t index = (this->_lore_log_start + i) % buffer_size;
        this->_lore_log_cache.push_back(this->_lore_log[index]);
    }
    this->_lore_log_cache_dirty = false;
    return this->_lore_log_cache;
}
