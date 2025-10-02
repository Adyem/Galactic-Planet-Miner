#include "planets.hpp"
#include "libft/Math/math.hpp"

namespace
{
    static bool planet_is_finite(double value) noexcept
    {
        if (math_isnan(value))
            return false;
        double difference = value - value;
        if (math_isnan(difference))
            return false;
        return true;
    }
}

ft_planet::ft_planet(int id) noexcept : _id(id), _items(), _rates(), _carryover(), _rates_cache(), _rates_cache_dirty(true), _carryover_cache(), _carryover_cache_dirty(true)
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

    Pair<int, ft_sharedptr<ft_item> > *entry = this->_items.find(item_id);
    if (entry == ft_nullptr)
        this->_items.insert(item_id, new_item);
    else
        entry->value = new_item;
}

void ft_planet::register_resource(int ore_id, double rate) noexcept
{
    this->ensure_item_slot(ore_id);
    Pair<int, double> *rate_entry = this->_rates.find(ore_id);
    if (rate_entry == ft_nullptr)
        this->_rates.insert(ore_id, rate);
    else
        rate_entry->value = rate;
    this->_rates_cache_dirty = true;

    Pair<int, double> *carry_entry = this->_carryover.find(ore_id);
    if (carry_entry == ft_nullptr)
        this->_carryover.insert(ore_id, 0.0);
    this->_carryover_cache_dirty = true;
}

ft_sharedptr<ft_item> ft_planet::find_item(int ore_id) noexcept
{
    Pair<int, ft_sharedptr<ft_item> > *entry = this->_items.find(ore_id);
    if (entry == ft_nullptr)
        return ft_sharedptr<ft_item>();
    return entry->value;
}

ft_sharedptr<const ft_item> ft_planet::find_item(int ore_id) const noexcept
{
    const Pair<int, ft_sharedptr<ft_item> > *entry = this->_items.find(ore_id);
    if (entry == ft_nullptr)
        return ft_sharedptr<const ft_item>();
    return entry->value;
}

Pair<int, double> *ft_planet::find_carryover(int ore_id) noexcept
{
    return this->_carryover.find(ore_id);
}

const Pair<int, double> *ft_planet::find_carryover(int ore_id) const noexcept
{
    return this->_carryover.find(ore_id);
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

int ft_planet::clamp_resource_amount(int ore_id, int amount) const noexcept
{
    if (amount < 0)
        amount = 0;
    int max_stack = 1000000;
    ft_sharedptr<const ft_item> item = this->find_item(ore_id);
    if (item)
    {
        int item_max = item->get_max_stack();
        if (item_max > 0)
            max_stack = item_max;
    }
    if (amount > max_stack)
        amount = max_stack;
    return amount;
}

double ft_planet::get_rate(int ore_id) const noexcept
{
    const Pair<int, double> *entry = this->_rates.find(ore_id);
    if (entry == ft_nullptr)
        return 0;
    return entry->value;
}

const ft_vector<Pair<int, double> > &ft_planet::get_resources() const noexcept
{
    if (this->_rates_cache_dirty)
    {
        this->_rates_cache.clear();
        size_t rate_count = this->_rates.size();
        if (rate_count > 0)
        {
            const Pair<int, double> *entries = this->_rates.end();
            entries -= rate_count;
            for (size_t i = 0; i < rate_count; ++i)
                this->_rates_cache.push_back(entries[i]);
        }
        this->_rates_cache_dirty = false;
    }
    return this->_rates_cache;
}

ft_vector<Pair<int, int> > ft_planet::get_items_snapshot() const noexcept
{
    ft_vector<Pair<int, int> > snapshot;
    size_t item_count = this->_items.size();
    if (item_count == 0)
        return snapshot;
    const Pair<int, ft_sharedptr<ft_item> > *entries = this->_items.end();
    entries -= item_count;
    for (size_t i = 0; i < item_count; ++i)
    {
        Pair<int, int> entry;
        entry.key = entries[i].key;
        int amount = 0;
        if (entries[i].value)
            amount = entries[i].value->get_stack_size();
        entry.value = amount;
        snapshot.push_back(entry);
    }
    return snapshot;
}

const ft_vector<Pair<int, double> > &ft_planet::get_carryover() const noexcept
{
    if (this->_carryover_cache_dirty)
    {
        this->_carryover_cache.clear();
        size_t carry_count = this->_carryover.size();
        if (carry_count > 0)
        {
            const Pair<int, double> *entries = this->_carryover.end();
            entries -= carry_count;
            for (size_t i = 0; i < carry_count; ++i)
                this->_carryover_cache.push_back(entries[i]);
        }
        this->_carryover_cache_dirty = false;
    }
    return this->_carryover_cache;
}

void ft_planet::set_carryover(int ore_id, double amount) noexcept
{
    Pair<int, double> *entry = this->find_carryover(ore_id);
    if (entry)
    {
        entry->value = amount;
        this->_carryover_cache_dirty = true;
        return ;
    }
    this->_carryover.insert(ore_id, amount);
    this->_carryover_cache_dirty = true;
}

ft_vector<Pair<int, int> > ft_planet::produce(double seconds) noexcept
{
    ft_vector<Pair<int, int> > produced;
    const double epsilon = 0.0000001;
    size_t rate_count = this->_rates.size();
    if (rate_count == 0)
        return produced;
    Pair<int, double> *rate_entries = this->_rates.end();
    rate_entries -= rate_count;
    for (size_t i = 0; i < rate_count; ++i)
    {
        int ore_id = rate_entries[i].key;
        double rate = rate_entries[i].value;
        if (!planet_is_finite(rate))
            rate = 0.0;
        Pair<int, double> *carry = this->find_carryover(ore_id);
        if (carry == ft_nullptr)
        {
            this->_carryover.insert(ore_id, 0.0);
            this->_carryover_cache_dirty = true;
            carry = this->find_carryover(ore_id);
        }
        if (carry && !planet_is_finite(carry->value))
        {
            carry->value = 0.0;
            this->_carryover_cache_dirty = true;
        }
        double total = rate * seconds;
        if (!planet_is_finite(total))
            total = 0.0;
        if (carry)
        {
            total += carry->value;
            if (!planet_is_finite(total))
                total = 0.0;
        }
        int amount = static_cast<int>(total + epsilon);
        double remainder = total - static_cast<double>(amount);
        if (remainder < 0.0)
            remainder = 0.0;
        if (carry)
        {
            carry->value = remainder;
            this->_carryover_cache_dirty = true;
        }
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

