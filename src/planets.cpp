#include "planets.hpp"

ft_planet::ft_planet(int id) noexcept : _id(id)
{
    this->_inventory.resize(32);
}

void ft_planet::ensure_item_slot(int item_id) noexcept
{
    ft_sharedptr<ft_item> item = this->find_item(item_id);
    if (item)
        return ;
    ft_sharedptr<ft_item> new_item(new ft_item());
    new_item->set_item_id(item_id);
    new_item->set_max_stack(1000000);
    new_item->set_stack_size(0);
    this->_inventory.add_item(new_item);

    Pair<int, ft_sharedptr<ft_item> > pair_item;
    pair_item.key = item_id;
    pair_item.value = new_item;
    this->_items.push_back(pair_item);
}

void ft_planet::register_resource(int ore_id, double rate) noexcept
{
    this->ensure_item_slot(ore_id);
    bool found = false;
    for (size_t i = 0; i < this->_rates.size(); ++i)
    {
        if (this->_rates[i].key == ore_id)
        {
            this->_rates[i].value = rate;
            found = true;
            break;
        }
    }

    if (!found)
    {
        Pair<int, double> pair_rate;
        pair_rate.key = ore_id;
        pair_rate.value = rate;
        this->_rates.push_back(pair_rate);
    }

    if (this->find_carryover(ore_id) == ft_nullptr)
    {
        Pair<int, double> entry;
        entry.key = ore_id;
        entry.value = 0.0;
        this->_carryover.push_back(entry);
    }
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

Pair<int, double> *ft_planet::find_carryover(int ore_id) noexcept
{
    for (size_t i = 0; i < this->_carryover.size(); ++i)
    {
        if (this->_carryover[i].key == ore_id)
            return &this->_carryover[i];
    }
    return ft_nullptr;
}

const Pair<int, double> *ft_planet::find_carryover(int ore_id) const noexcept
{
    for (size_t i = 0; i < this->_carryover.size(); ++i)
    {
        if (this->_carryover[i].key == ore_id)
            return &this->_carryover[i];
    }
    return ft_nullptr;
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

ft_vector<Pair<int, int> > ft_planet::produce(double seconds) noexcept
{
    ft_vector<Pair<int, int> > produced;
    const double epsilon = 0.0000001;
    for (size_t i = 0; i < this->_rates.size(); ++i)
    {
        int ore_id = this->_rates[i].key;
        double rate = this->_rates[i].value;
        Pair<int, double> *carry = this->find_carryover(ore_id);
        if (carry == ft_nullptr)
        {
            Pair<int, double> entry;
            entry.key = ore_id;
            entry.value = 0.0;
            this->_carryover.push_back(entry);
            carry = this->find_carryover(ore_id);
        }
        double total = rate * seconds;
        if (carry)
            total += carry->value;
        int amount = static_cast<int>(total + epsilon);
        double remainder = total - static_cast<double>(amount);
        if (remainder < 0.0)
            remainder = 0.0;
        if (carry)
            carry->value = remainder;
        if (amount > 0)
        {
            this->add_resource(ore_id, amount);
            Pair<int, int> entry;
            entry.key = ore_id;
            entry.value = amount;
            produced.push_back(entry);
        }
    }
    return produced;
}

