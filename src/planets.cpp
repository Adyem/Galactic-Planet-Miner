#include "planets.hpp"

ft_planet::ft_planet(int id) noexcept : _id(id)
{
    this->_inventory.resize(10);
}

void ft_planet::register_resource(int ore_id, double rate) noexcept
{
    ft_sharedptr<ft_item> item(new ft_item());
    item->set_item_id(ore_id);
    item->set_max_stack(1000000);
    item->set_stack_size(0);
    this->_inventory.add_item(item);

    Pair<int, ft_sharedptr<ft_item> > pair_item;
    pair_item.key = ore_id;
    pair_item.value = item;
    this->_items.push_back(pair_item);

    Pair<int, double> pair_rate;
    pair_rate.key = ore_id;
    pair_rate.value = rate;
    this->_rates.push_back(pair_rate);
}

ft_sharedptr<ft_item> ft_planet::find_item(int ore_id) noexcept
{
    for (size_t i = 0; i < this->_items.size(); ++i)
    {
        if (this->_items[i].key == ore_id)
            return this->_items[i].value;
    }
    return ft_sharedptr<ft_item>();
}

ft_sharedptr<const ft_item> ft_planet::find_item(int ore_id) const noexcept
{
    for (size_t i = 0; i < this->_items.size(); ++i)
    {
        if (this->_items[i].key == ore_id)
            return this->_items[i].value;
    }
    return ft_sharedptr<const ft_item>();
}

int ft_planet::get_id() const noexcept
{
    return this->_id;
}

int ft_planet::add_resource(int ore_id, int amount) noexcept
{
    ft_sharedptr<ft_item> item = this->find_item(ore_id);
    if (!item)
        return 0;
    item->add_to_stack(amount);
    return item->get_stack_size();
}

int ft_planet::sub_resource(int ore_id, int amount) noexcept
{
    ft_sharedptr<ft_item> item = this->find_item(ore_id);
    if (!item)
        return 0;
    item->sub_from_stack(amount);
    return item->get_stack_size();
}

int ft_planet::get_resource(int ore_id) const noexcept
{
    ft_sharedptr<const ft_item> item = this->find_item(ore_id);
    if (!item)
        return 0;
    return item->get_stack_size();
}

void ft_planet::set_resource(int ore_id, int amount) noexcept
{
    ft_sharedptr<ft_item> item = this->find_item(ore_id);
    if (item)
        item->set_stack_size(amount);
}

double ft_planet::get_rate(int ore_id) const noexcept
{
    for (size_t i = 0; i < this->_rates.size(); ++i)
    {
        if (this->_rates[i].key == ore_id)
            return this->_rates[i].value;
    }
    return 0;
}

const ft_vector<Pair<int, double> > &ft_planet::get_resources() const noexcept
{
    return this->_rates;
}

