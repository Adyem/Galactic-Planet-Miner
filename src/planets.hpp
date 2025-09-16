#ifndef PLANETS_HPP
#define PLANETS_HPP

#include "../libft/Game/game_character.hpp"
#include "../libft/Game/game_item.hpp"
#include "../libft/Template/shared_ptr.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft/Template/pair.hpp"
#include "../libft/CPP_class/class_nullptr.hpp"

enum e_planet_id
{
    PLANET_TERRA = 1,
    PLANET_MARS,
    PLANET_ZALTHOR,
    PLANET_VULCAN,
    PLANET_NOCTARIS_PRIME
};

enum e_ore_id
{
    ORE_IRON = 1,
    ORE_COPPER,
    ORE_MITHRIL,
    ORE_COAL,
    ORE_TIN,
    ORE_SILVER,
    ORE_GOLD,
    ORE_TITANIUM,
    ORE_OBSIDIAN,
    ORE_CRYSTAL,
    ORE_NANOMATERIAL
};

enum e_component_id
{
    ITEM_IRON_BAR = 1001,
    ITEM_COPPER_BAR,
    ITEM_MITHRIL_BAR,
    ITEM_ENGINE_PART
};

class ft_planet : public ft_character
{
protected:
    int                                     _id;
    ft_vector<Pair<int, ft_sharedptr<ft_item> > > _items;
    ft_vector<Pair<int, double> >           _rates;

    ft_sharedptr<ft_item> find_item(int ore_id) noexcept;
    ft_sharedptr<const ft_item> find_item(int ore_id) const noexcept;

public:
    explicit ft_planet(int id) noexcept;
    void ensure_item_slot(int item_id) noexcept;
    void register_resource(int ore_id, double rate) noexcept;

    int get_id() const noexcept;
    int add_resource(int ore_id, int amount) noexcept;
    int sub_resource(int ore_id, int amount) noexcept;
    int get_resource(int ore_id) const noexcept;
    void set_resource(int ore_id, int amount) noexcept;
    double get_rate(int ore_id) const noexcept;
    const ft_vector<Pair<int, double> > &get_resources() const noexcept;
    ft_vector<Pair<int, int> > produce(double seconds) noexcept;
};

class ft_planet_terra : public ft_planet
{
public:
    ft_planet_terra() noexcept : ft_planet(PLANET_TERRA)
    {
        this->register_resource(ORE_IRON, 0.5);
        this->register_resource(ORE_COPPER, 0.5);
        this->register_resource(ORE_COAL, 0.2);
    }
};

class ft_planet_mars : public ft_planet
{
public:
    ft_planet_mars() noexcept : ft_planet(PLANET_MARS)
    {
        this->register_resource(ORE_IRON, 0.1);
        this->register_resource(ORE_COPPER, 0.1);
        this->register_resource(ORE_MITHRIL, 0.05);
        this->register_resource(ORE_COAL, 0.1);
    }
};

class ft_planet_zalthor : public ft_planet
{
public:
    ft_planet_zalthor() noexcept : ft_planet(PLANET_ZALTHOR)
    {
        this->register_resource(ORE_MITHRIL, 0.1);
        this->register_resource(ORE_COAL, 0.2);
        this->register_resource(ORE_GOLD, 0.02);
    }
};

class ft_planet_vulcan : public ft_planet
{
public:
    ft_planet_vulcan() noexcept : ft_planet(PLANET_VULCAN)
    {
        this->register_resource(ORE_TIN, 0.1);
        this->register_resource(ORE_SILVER, 0.03);
        this->register_resource(ORE_TITANIUM, 0.01);
    }
};

class ft_planet_noctaris_prime : public ft_planet
{
public:
    ft_planet_noctaris_prime() noexcept : ft_planet(PLANET_NOCTARIS_PRIME)
    {
        this->register_resource(ORE_OBSIDIAN, 0.1);
        this->register_resource(ORE_CRYSTAL, 0.05);
        this->register_resource(ORE_NANOMATERIAL, 0.02);
    }
};

#endif
