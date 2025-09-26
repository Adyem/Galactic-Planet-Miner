#include "buildings.hpp"
#include "game.hpp"
#include "research.hpp"
#include "../libft/Libft/libft.hpp"
#include "ft_map_snapshot.hpp"

ft_planet_build_state::ft_planet_build_state()
    : planet_id(0), width(0), height(0), base_logistic(0), research_logistic_bonus(0), used_plots(0),
      logistic_capacity(0), logistic_usage(0), base_energy_generation(0.0),
      energy_generation(0.0), energy_consumption(0.0), support_energy(0.0),
      mine_multiplier(1.0), convoy_speed_bonus(0.0), convoy_raid_risk_modifier(0.0),
      energy_deficit_pressure(0.0), next_instance_id(1), grid(), instances()
{
}

ft_planet_build_state::ft_planet_build_state(const ft_planet_build_state &other)
    : planet_id(0), width(0), height(0), base_logistic(0), research_logistic_bonus(0), used_plots(0),
      logistic_capacity(0), logistic_usage(0), base_energy_generation(0.0),
      energy_generation(0.0), energy_consumption(0.0), support_energy(0.0),
      mine_multiplier(1.0), convoy_speed_bonus(0.0), convoy_raid_risk_modifier(0.0),
      energy_deficit_pressure(0.0), next_instance_id(1), grid(), instances()
{
    *this = other;
}

ft_planet_build_state &ft_planet_build_state::operator=(const ft_planet_build_state &other)
{
    if (this == &other)
        return *this;
    this->planet_id = other.planet_id;
    this->width = other.width;
    this->height = other.height;
    this->base_logistic = other.base_logistic;
    this->research_logistic_bonus = other.research_logistic_bonus;
    this->used_plots = other.used_plots;
    this->logistic_capacity = other.logistic_capacity;
    this->logistic_usage = other.logistic_usage;
    this->base_energy_generation = other.base_energy_generation;
    this->energy_generation = other.energy_generation;
    this->energy_consumption = other.energy_consumption;
    this->support_energy = other.support_energy;
    this->mine_multiplier = other.mine_multiplier;
    this->convoy_speed_bonus = other.convoy_speed_bonus;
    this->convoy_raid_risk_modifier = other.convoy_raid_risk_modifier;
    this->energy_deficit_pressure = other.energy_deficit_pressure;
    this->next_instance_id = other.next_instance_id;

    this->grid.clear();
    size_t grid_size = other.grid.size();
    if (grid_size > 0)
    {
        this->grid.reserve(grid_size);
        for (size_t index = 0; index < grid_size; ++index)
            this->grid.push_back(other.grid[index]);
    }

    this->instances.clear();
    ft_vector<Pair<int, ft_building_instance> > instance_entries;
    ft_map_snapshot(other.instances, instance_entries);
    for (size_t index = 0; index < instance_entries.size(); ++index)
        this->instances.insert(instance_entries[index].key, instance_entries[index].value);

    return *this;
}

ft_planet_build_state::ft_planet_build_state(ft_planet_build_state &&other) noexcept
    : planet_id(other.planet_id), width(other.width), height(other.height),
      base_logistic(other.base_logistic), research_logistic_bonus(other.research_logistic_bonus),
      used_plots(other.used_plots), logistic_capacity(other.logistic_capacity),
      logistic_usage(other.logistic_usage), base_energy_generation(other.base_energy_generation),
      energy_generation(other.energy_generation), energy_consumption(other.energy_consumption),
      support_energy(other.support_energy), mine_multiplier(other.mine_multiplier),
      convoy_speed_bonus(other.convoy_speed_bonus),
      convoy_raid_risk_modifier(other.convoy_raid_risk_modifier),
      energy_deficit_pressure(other.energy_deficit_pressure), next_instance_id(other.next_instance_id),
      grid(ft_move(other.grid)), instances(ft_move(other.instances))
{
    other.planet_id = 0;
    other.width = 0;
    other.height = 0;
    other.base_logistic = 0;
    other.research_logistic_bonus = 0;
    other.used_plots = 0;
    other.logistic_capacity = 0;
    other.logistic_usage = 0;
    other.base_energy_generation = 0.0;
    other.energy_generation = 0.0;
    other.energy_consumption = 0.0;
    other.support_energy = 0.0;
    other.mine_multiplier = 1.0;
    other.convoy_speed_bonus = 0.0;
    other.convoy_raid_risk_modifier = 0.0;
    other.energy_deficit_pressure = 0.0;
    other.next_instance_id = 1;
}

ft_planet_build_state &ft_planet_build_state::operator=(ft_planet_build_state &&other) noexcept
{
    if (this == &other)
        return *this;

    this->planet_id = other.planet_id;
    this->width = other.width;
    this->height = other.height;
    this->base_logistic = other.base_logistic;
    this->research_logistic_bonus = other.research_logistic_bonus;
    this->used_plots = other.used_plots;
    this->logistic_capacity = other.logistic_capacity;
    this->logistic_usage = other.logistic_usage;
    this->base_energy_generation = other.base_energy_generation;
    this->energy_generation = other.energy_generation;
    this->energy_consumption = other.energy_consumption;
    this->support_energy = other.support_energy;
    this->mine_multiplier = other.mine_multiplier;
    this->convoy_speed_bonus = other.convoy_speed_bonus;
    this->convoy_raid_risk_modifier = other.convoy_raid_risk_modifier;
    this->energy_deficit_pressure = other.energy_deficit_pressure;
    this->next_instance_id = other.next_instance_id;

    this->grid = ft_move(other.grid);
    this->instances = ft_move(other.instances);

    other.planet_id = 0;
    other.width = 0;
    other.height = 0;
    other.base_logistic = 0;
    other.research_logistic_bonus = 0;
    other.used_plots = 0;
    other.logistic_capacity = 0;
    other.logistic_usage = 0;
    other.base_energy_generation = 0.0;
    other.energy_generation = 0.0;
    other.energy_consumption = 0.0;
    other.support_energy = 0.0;
    other.mine_multiplier = 1.0;
    other.convoy_speed_bonus = 0.0;
    other.convoy_raid_risk_modifier = 0.0;
    other.energy_deficit_pressure = 0.0;
    other.next_instance_id = 1;

    return *this;
}

BuildingManager::BuildingManager()
    : _definitions(),
      _planets(),
      _building_unlocks(),
      _crafting_energy_multiplier(1.0),
      _crafting_speed_multiplier(1.0),
      _global_energy_multiplier(1.0)
{
    ft_sharedptr<ft_building_definition> mine(new ft_building_definition());
    mine->id = BUILDING_MINE_CORE;
    mine->name = ft_string("Mine Core");
    mine->width = 1;
    mine->height = 1;
    mine->logistic_cost = 0;
    mine->logistic_gain = 0;
    mine->energy_cost = 0.0;
    mine->energy_gain = 0.0;
    mine->cycle_time = 0.0;
    mine->inputs.clear();
    mine->outputs.clear();
    mine->build_costs.clear();
    mine->mine_bonus = 0.0;
    mine->unique = true;
    mine->occupies_grid = true;
    mine->removable = false;
    this->register_definition(mine);

    ft_sharedptr<ft_building_definition> smelter(new ft_building_definition());
    smelter->id = BUILDING_SMELTER;
    smelter->name = ft_string("Smelting Facility");
    smelter->width = 2;
    smelter->height = 2;
    smelter->logistic_cost = 1;
    smelter->logistic_gain = 0;
    smelter->energy_cost = 2.0;
    smelter->energy_gain = 0.0;
    smelter->cycle_time = 5.0;
    smelter->inputs.clear();
    Pair<int, int> recipe;
    recipe.key = ORE_IRON;
    recipe.value = 3;
    smelter->inputs.push_back(recipe);
    smelter->outputs.clear();
    recipe.key = ITEM_IRON_BAR;
    recipe.value = 2;
    smelter->outputs.push_back(recipe);
    smelter->build_costs.clear();
    recipe.key = ORE_IRON;
    recipe.value = 12;
    smelter->build_costs.push_back(recipe);
    recipe.key = ORE_COPPER;
    recipe.value = 8;
    smelter->build_costs.push_back(recipe);
    smelter->mine_bonus = 0.0;
    smelter->unique = false;
    smelter->occupies_grid = true;
    smelter->removable = true;
    this->register_definition(smelter);

    ft_sharedptr<ft_building_definition> processor(new ft_building_definition());
    processor->id = BUILDING_PROCESSOR;
    processor->name = ft_string("Processing Unit");
    processor->width = 2;
    processor->height = 2;
    processor->logistic_cost = 1;
    processor->logistic_gain = 0;
    processor->energy_cost = 2.5;
    processor->energy_gain = 0.0;
    processor->cycle_time = 6.0;
    processor->inputs.clear();
    recipe.key = ORE_COPPER;
    recipe.value = 3;
    processor->inputs.push_back(recipe);
    processor->outputs.clear();
    recipe.key = ITEM_COPPER_BAR;
    recipe.value = 2;
    processor->outputs.push_back(recipe);
    processor->build_costs.clear();
    recipe.key = ORE_COPPER;
    recipe.value = 10;
    processor->build_costs.push_back(recipe);
    recipe.key = ORE_COAL;
    recipe.value = 6;
    processor->build_costs.push_back(recipe);
    processor->mine_bonus = 0.0;
    processor->unique = false;
    processor->occupies_grid = true;
    processor->removable = true;
    this->register_definition(processor);

    ft_sharedptr<ft_building_definition> crafting(new ft_building_definition());
    crafting->id = BUILDING_CRAFTING_BAY;
    crafting->name = ft_string("Crafting Bay");
    crafting->width = 3;
    crafting->height = 2;
    crafting->logistic_cost = 2;
    crafting->logistic_gain = 0;
    crafting->energy_cost = 4.0;
    crafting->energy_gain = 0.0;
    crafting->cycle_time = 8.0;
    crafting->inputs.clear();
    recipe.key = ITEM_IRON_BAR;
    recipe.value = 2;
    crafting->inputs.push_back(recipe);
    recipe.key = ITEM_COPPER_BAR;
    recipe.value = 1;
    crafting->inputs.push_back(recipe);
    crafting->outputs.clear();
    recipe.key = ITEM_ENGINE_PART;
    recipe.value = 1;
    crafting->outputs.push_back(recipe);
    crafting->build_costs.clear();
    recipe.key = ORE_IRON;
    recipe.value = 8;
    crafting->build_costs.push_back(recipe);
    recipe.key = ORE_COPPER;
    recipe.value = 10;
    crafting->build_costs.push_back(recipe);
    recipe.key = ORE_COAL;
    recipe.value = 6;
    crafting->build_costs.push_back(recipe);
    crafting->mine_bonus = 0.0;
    crafting->unique = false;
    crafting->occupies_grid = true;
    crafting->removable = true;
    this->register_definition(crafting);

    ft_sharedptr<ft_building_definition> conveyor(new ft_building_definition());
    conveyor->id = BUILDING_CONVEYOR;
    conveyor->name = ft_string("Conveyor Belt");
    conveyor->width = 1;
    conveyor->height = 1;
    conveyor->logistic_cost = 0;
    conveyor->logistic_gain = 1;
    conveyor->energy_cost = 0.0;
    conveyor->energy_gain = 0.0;
    conveyor->cycle_time = 0.0;
    conveyor->inputs.clear();
    conveyor->outputs.clear();
    conveyor->build_costs.clear();
    recipe.key = ORE_IRON;
    recipe.value = 4;
    conveyor->build_costs.push_back(recipe);
    conveyor->mine_bonus = 0.0;
    conveyor->unique = false;
    conveyor->occupies_grid = true;
    conveyor->removable = true;
    this->register_definition(conveyor);

    ft_sharedptr<ft_building_definition> transfer(new ft_building_definition());
    transfer->id = BUILDING_TRANSFER_NODE;
    transfer->name = ft_string("Resource Transfer Node");
    transfer->width = 1;
    transfer->height = 1;
    transfer->logistic_cost = 0;
    transfer->logistic_gain = 2;
    transfer->energy_cost = 0.0;
    transfer->energy_gain = 0.0;
    transfer->cycle_time = 0.0;
    transfer->inputs.clear();
    transfer->outputs.clear();
    transfer->build_costs.clear();
    recipe.key = ORE_IRON;
    recipe.value = 6;
    transfer->build_costs.push_back(recipe);
    recipe.key = ORE_COPPER;
    recipe.value = 4;
    transfer->build_costs.push_back(recipe);
    transfer->mine_bonus = 0.0;
    transfer->unique = false;
    transfer->occupies_grid = true;
    transfer->removable = true;
    this->register_definition(transfer);

    ft_sharedptr<ft_building_definition> relay(new ft_building_definition());
    relay->id = BUILDING_TRADE_RELAY;
    relay->name = ft_string("Trade Relay Nexus");
    relay->width = 2;
    relay->height = 2;
    relay->logistic_cost = 1;
    relay->logistic_gain = 0;
    relay->energy_cost = 3.5;
    relay->energy_gain = 0.0;
    relay->cycle_time = 0.0;
    relay->inputs.clear();
    relay->outputs.clear();
    relay->build_costs.clear();
    recipe.key = ITEM_COPPER_BAR;
    recipe.value = 10;
    relay->build_costs.push_back(recipe);
    recipe.key = ITEM_ADVANCED_ENGINE_PART;
    recipe.value = 3;
    relay->build_costs.push_back(recipe);
    recipe.key = ITEM_ACCUMULATOR;
    recipe.value = 2;
    relay->build_costs.push_back(recipe);
    relay->mine_bonus = 0.0;
    relay->convoy_speed_bonus = 0.12;
    relay->convoy_raid_risk_modifier = 0.18;
    relay->unique = false;
    relay->occupies_grid = true;
    relay->removable = true;
    this->register_definition(relay);

    ft_sharedptr<ft_building_definition> generator(new ft_building_definition());
    generator->id = BUILDING_POWER_GENERATOR;
    generator->name = ft_string("Power Generator");
    generator->width = 2;
    generator->height = 2;
    generator->logistic_cost = 0;
    generator->logistic_gain = 0;
    generator->energy_cost = 0.0;
    generator->energy_gain = 6.0;
    generator->cycle_time = 0.0;
    generator->inputs.clear();
    generator->outputs.clear();
    generator->build_costs.clear();
    recipe.key = ORE_COAL;
    recipe.value = 10;
    generator->build_costs.push_back(recipe);
    recipe.key = ORE_COPPER;
    recipe.value = 8;
    generator->build_costs.push_back(recipe);
    generator->mine_bonus = 0.0;
    generator->unique = false;
    generator->occupies_grid = true;
    generator->removable = true;
    this->register_definition(generator);

    ft_sharedptr<ft_building_definition> solar(new ft_building_definition());
    solar->id = BUILDING_SOLAR_ARRAY;
    solar->name = ft_string("Solar Array");
    solar->width = 1;
    solar->height = 1;
    solar->logistic_cost = 0;
    solar->logistic_gain = 0;
    solar->energy_cost = 0.0;
    solar->energy_gain = 3.0;
    solar->cycle_time = 0.0;
    solar->inputs.clear();
    solar->outputs.clear();
    solar->build_costs.clear();
    recipe.key = ORE_IRON;
    recipe.value = 20;
    solar->build_costs.push_back(recipe);
    recipe.key = ORE_COPPER;
    recipe.value = 30;
    solar->build_costs.push_back(recipe);
    solar->mine_bonus = 0.0;
    solar->unique = false;
    solar->occupies_grid = true;
    solar->removable = true;
    this->register_definition(solar);

    ft_sharedptr<ft_building_definition> upgrade(new ft_building_definition());
    upgrade->id = BUILDING_UPGRADE_STATION;
    upgrade->name = ft_string("Mine Upgrade Station");
    upgrade->width = 2;
    upgrade->height = 2;
    upgrade->logistic_cost = 0;
    upgrade->logistic_gain = 0;
    upgrade->energy_cost = 1.0;
    upgrade->energy_gain = 0.0;
    upgrade->cycle_time = 0.0;
    upgrade->inputs.clear();
    upgrade->outputs.clear();
    upgrade->build_costs.clear();
    recipe.key = ORE_IRON;
    recipe.value = 10;
    upgrade->build_costs.push_back(recipe);
    recipe.key = ORE_MITHRIL;
    recipe.value = 4;
    upgrade->build_costs.push_back(recipe);
    upgrade->mine_bonus = 0.15;
    upgrade->unique = false;
    upgrade->occupies_grid = true;
    upgrade->removable = true;
    this->register_definition(upgrade);

    ft_sharedptr<ft_building_definition> workshop(new ft_building_definition());
    workshop->id = BUILDING_FACILITY_WORKSHOP;
    workshop->name = ft_string("Facility Workshop");
    workshop->width = 3;
    workshop->height = 3;
    workshop->logistic_cost = 3;
    workshop->logistic_gain = 0;
    workshop->energy_cost = 5.0;
    workshop->energy_gain = 0.0;
    workshop->cycle_time = 12.0;
    workshop->inputs.clear();
    recipe.key = ITEM_IRON_BAR;
    recipe.value = 3;
    workshop->inputs.push_back(recipe);
    recipe.key = ITEM_COPPER_BAR;
    recipe.value = 2;
    workshop->inputs.push_back(recipe);
    recipe.key = ITEM_MITHRIL_BAR;
    recipe.value = 1;
    workshop->inputs.push_back(recipe);
    workshop->outputs.clear();
    recipe.key = ITEM_ADVANCED_ENGINE_PART;
    recipe.value = 1;
    workshop->outputs.push_back(recipe);
    workshop->build_costs.clear();
    recipe.key = ITEM_IRON_BAR;
    recipe.value = 12;
    workshop->build_costs.push_back(recipe);
    recipe.key = ITEM_COPPER_BAR;
    recipe.value = 8;
    workshop->build_costs.push_back(recipe);
    recipe.key = ITEM_ENGINE_PART;
    recipe.value = 4;
    workshop->build_costs.push_back(recipe);
    workshop->mine_bonus = 0.0;
    workshop->unique = false;
    workshop->occupies_grid = true;
    workshop->removable = true;
    this->register_definition(workshop);

    ft_sharedptr<ft_building_definition> shipyard(new ft_building_definition());
    shipyard->id = BUILDING_SHIPYARD;
    shipyard->name = ft_string("Orbital Shipyard");
    shipyard->width = 4;
    shipyard->height = 3;
    shipyard->logistic_cost = 4;
    shipyard->logistic_gain = 0;
    shipyard->energy_cost = 8.0;
    shipyard->energy_gain = 0.0;
    shipyard->cycle_time = 18.0;
    shipyard->inputs.clear();
    recipe.key = ITEM_ADVANCED_ENGINE_PART;
    recipe.value = 2;
    shipyard->inputs.push_back(recipe);
    recipe.key = ITEM_TITANIUM_BAR;
    recipe.value = 3;
    shipyard->inputs.push_back(recipe);
    recipe.key = ITEM_ACCUMULATOR;
    recipe.value = 1;
    shipyard->inputs.push_back(recipe);
    shipyard->outputs.clear();
    recipe.key = ITEM_FUSION_REACTOR;
    recipe.value = 1;
    shipyard->outputs.push_back(recipe);
    shipyard->build_costs.clear();
    recipe.key = ITEM_TITANIUM_BAR;
    recipe.value = 10;
    shipyard->build_costs.push_back(recipe);
    recipe.key = ORE_GOLD;
    recipe.value = 6;
    shipyard->build_costs.push_back(recipe);
    recipe.key = ITEM_ADVANCED_ENGINE_PART;
    recipe.value = 4;
    shipyard->build_costs.push_back(recipe);
    shipyard->mine_bonus = 0.0;
    shipyard->unique = false;
    shipyard->occupies_grid = true;
    shipyard->removable = true;
    this->register_definition(shipyard);

    ft_sharedptr<ft_building_definition> proximity_radar(new ft_building_definition());
    proximity_radar->id = BUILDING_PROXIMITY_RADAR;
    proximity_radar->name = ft_string("Proximity Radar Array");
    proximity_radar->width = 2;
    proximity_radar->height = 2;
    proximity_radar->logistic_cost = 0;
    proximity_radar->logistic_gain = 1;
    proximity_radar->energy_cost = 1.0;
    proximity_radar->energy_gain = 0.0;
    proximity_radar->cycle_time = 0.0;
    proximity_radar->inputs.clear();
    proximity_radar->outputs.clear();
    proximity_radar->build_costs.clear();
    recipe.key = ITEM_IRON_BAR;
    recipe.value = 6;
    proximity_radar->build_costs.push_back(recipe);
    recipe.key = ITEM_COPPER_BAR;
    recipe.value = 6;
    proximity_radar->build_costs.push_back(recipe);
    proximity_radar->mine_bonus = 0.0;
    proximity_radar->unique = false;
    proximity_radar->occupies_grid = true;
    proximity_radar->removable = true;
    this->register_definition(proximity_radar);

    ft_sharedptr<ft_building_definition> mobile_radar(new ft_building_definition());
    mobile_radar->id = BUILDING_MOBILE_RADAR;
    mobile_radar->name = ft_string("Mobile Defense Radar");
    mobile_radar->width = 2;
    mobile_radar->height = 2;
    mobile_radar->logistic_cost = 0;
    mobile_radar->logistic_gain = 2;
    mobile_radar->energy_cost = 1.5;
    mobile_radar->energy_gain = 0.0;
    mobile_radar->cycle_time = 0.0;
    mobile_radar->inputs.clear();
    mobile_radar->outputs.clear();
    mobile_radar->build_costs.clear();
    recipe.key = ITEM_COPPER_BAR;
    recipe.value = 10;
    mobile_radar->build_costs.push_back(recipe);
    recipe.key = ITEM_MITHRIL_BAR;
    recipe.value = 4;
    mobile_radar->build_costs.push_back(recipe);
    recipe.key = ITEM_ADVANCED_ENGINE_PART;
    recipe.value = 2;
    mobile_radar->build_costs.push_back(recipe);
    mobile_radar->mine_bonus = 0.0;
    mobile_radar->unique = false;
    mobile_radar->occupies_grid = true;
    mobile_radar->removable = true;
    this->register_definition(mobile_radar);

    ft_sharedptr<ft_building_definition> shield_generator(new ft_building_definition());
    shield_generator->id = BUILDING_SHIELD_GENERATOR;
    shield_generator->name = ft_string("Shield Generator");
    shield_generator->width = 3;
    shield_generator->height = 3;
    shield_generator->logistic_cost = 0;
    shield_generator->logistic_gain = 0;
    shield_generator->energy_cost = 6.0;
    shield_generator->energy_gain = 0.0;
    shield_generator->cycle_time = 0.0;
    shield_generator->inputs.clear();
    shield_generator->outputs.clear();
    shield_generator->build_costs.clear();
    recipe.key = ITEM_TITANIUM_BAR;
    recipe.value = 8;
    shield_generator->build_costs.push_back(recipe);
    recipe.key = ITEM_ACCUMULATOR;
    recipe.value = 4;
    shield_generator->build_costs.push_back(recipe);
    recipe.key = ITEM_ADVANCED_ENGINE_PART;
    recipe.value = 2;
    shield_generator->build_costs.push_back(recipe);
    shield_generator->mine_bonus = 0.0;
    shield_generator->unique = false;
    shield_generator->occupies_grid = true;
    shield_generator->removable = true;
    this->register_definition(shield_generator);

    ft_sharedptr<ft_building_definition> tritium_extractor(new ft_building_definition());
    tritium_extractor->id = BUILDING_TRITIUM_EXTRACTOR;
    tritium_extractor->name = ft_string("Tritium Extractor");
    tritium_extractor->width = 2;
    tritium_extractor->height = 2;
    tritium_extractor->logistic_cost = 2;
    tritium_extractor->logistic_gain = 0;
    tritium_extractor->energy_cost = 4.0;
    tritium_extractor->energy_gain = 0.0;
    tritium_extractor->cycle_time = 10.0;
    tritium_extractor->inputs.clear();
    recipe.key = ORE_OBSIDIAN;
    recipe.value = 2;
    tritium_extractor->inputs.push_back(recipe);
    recipe.key = ORE_CRYSTAL;
    recipe.value = 1;
    tritium_extractor->inputs.push_back(recipe);
    tritium_extractor->outputs.clear();
    recipe.key = ORE_TRITIUM;
    recipe.value = 1;
    tritium_extractor->outputs.push_back(recipe);
    tritium_extractor->build_costs.clear();
    recipe.key = ORE_OBSIDIAN;
    recipe.value = 8;
    tritium_extractor->build_costs.push_back(recipe);
    recipe.key = ORE_CRYSTAL;
    recipe.value = 6;
    tritium_extractor->build_costs.push_back(recipe);
    recipe.key = ITEM_ACCUMULATOR;
    recipe.value = 2;
    tritium_extractor->build_costs.push_back(recipe);
    tritium_extractor->mine_bonus = 0.0;
    tritium_extractor->unique = false;
    tritium_extractor->occupies_grid = true;
    tritium_extractor->removable = true;
    this->register_definition(tritium_extractor);

    ft_sharedptr<ft_building_definition> defense_turret(new ft_building_definition());
    defense_turret->id = BUILDING_DEFENSE_TURRET;
    defense_turret->name = ft_string("Defense Turret");
    defense_turret->width = 1;
    defense_turret->height = 1;
    defense_turret->logistic_cost = 0;
    defense_turret->logistic_gain = 0;
    defense_turret->energy_cost = 1.5;
    defense_turret->energy_gain = 0.0;
    defense_turret->cycle_time = 0.0;
    defense_turret->inputs.clear();
    defense_turret->outputs.clear();
    defense_turret->build_costs.clear();
    recipe.key = ITEM_IRON_BAR;
    recipe.value = 5;
    defense_turret->build_costs.push_back(recipe);
    recipe.key = ITEM_ENGINE_PART;
    recipe.value = 2;
    defense_turret->build_costs.push_back(recipe);
    defense_turret->mine_bonus = 0.0;
    defense_turret->unique = false;
    defense_turret->occupies_grid = true;
    defense_turret->removable = true;
    this->register_definition(defense_turret);

    ft_sharedptr<ft_building_definition> plasma_turret(new ft_building_definition());
    plasma_turret->id = BUILDING_PLASMA_TURRET;
    plasma_turret->name = ft_string("Plasma Turret");
    plasma_turret->width = 2;
    plasma_turret->height = 1;
    plasma_turret->logistic_cost = 0;
    plasma_turret->logistic_gain = 0;
    plasma_turret->energy_cost = 3.0;
    plasma_turret->energy_gain = 0.0;
    plasma_turret->cycle_time = 0.0;
    plasma_turret->inputs.clear();
    plasma_turret->outputs.clear();
    plasma_turret->build_costs.clear();
    recipe.key = ITEM_MITHRIL_BAR;
    recipe.value = 6;
    plasma_turret->build_costs.push_back(recipe);
    recipe.key = ITEM_ADVANCED_ENGINE_PART;
    recipe.value = 2;
    plasma_turret->build_costs.push_back(recipe);
    recipe.key = ITEM_ACCUMULATOR;
    recipe.value = 1;
    plasma_turret->build_costs.push_back(recipe);
    plasma_turret->mine_bonus = 0.0;
    plasma_turret->unique = false;
    plasma_turret->occupies_grid = true;
    plasma_turret->removable = true;
    this->register_definition(plasma_turret);

    ft_sharedptr<ft_building_definition> railgun_turret(new ft_building_definition());
    railgun_turret->id = BUILDING_RAILGUN_TURRET;
    railgun_turret->name = ft_string("Railgun Turret");
    railgun_turret->width = 2;
    railgun_turret->height = 2;
    railgun_turret->logistic_cost = 0;
    railgun_turret->logistic_gain = 0;
    railgun_turret->energy_cost = 4.5;
    railgun_turret->energy_gain = 0.0;
    railgun_turret->cycle_time = 0.0;
    railgun_turret->inputs.clear();
    railgun_turret->outputs.clear();
    railgun_turret->build_costs.clear();
    recipe.key = ITEM_TITANIUM_BAR;
    recipe.value = 8;
    railgun_turret->build_costs.push_back(recipe);
    recipe.key = ITEM_ADVANCED_ENGINE_PART;
    recipe.value = 3;
    railgun_turret->build_costs.push_back(recipe);
    recipe.key = ITEM_FUSION_REACTOR;
    recipe.value = 1;
    railgun_turret->build_costs.push_back(recipe);
    railgun_turret->mine_bonus = 0.0;
    railgun_turret->unique = false;
    railgun_turret->occupies_grid = true;
    railgun_turret->removable = true;
    this->register_definition(railgun_turret);

    ft_sharedptr<ft_building_definition> flagship_dock(new ft_building_definition());
    flagship_dock->id = BUILDING_FLAGSHIP_DOCK;
    flagship_dock->name = ft_string("Flagship Dock");
    flagship_dock->width = 4;
    flagship_dock->height = 4;
    flagship_dock->logistic_cost = 0;
    flagship_dock->logistic_gain = 2;
    flagship_dock->energy_cost = 5.0;
    flagship_dock->energy_gain = 0.0;
    flagship_dock->cycle_time = 0.0;
    flagship_dock->inputs.clear();
    flagship_dock->outputs.clear();
    flagship_dock->build_costs.clear();
    recipe.key = ITEM_TITANIUM_BAR;
    recipe.value = 12;
    flagship_dock->build_costs.push_back(recipe);
    recipe.key = ORE_NANOMATERIAL;
    recipe.value = 6;
    flagship_dock->build_costs.push_back(recipe);
    recipe.key = ITEM_FUSION_REACTOR;
    recipe.value = 1;
    flagship_dock->build_costs.push_back(recipe);
    flagship_dock->mine_bonus = 0.0;
    flagship_dock->unique = true;
    flagship_dock->occupies_grid = true;
    flagship_dock->removable = true;
    this->register_definition(flagship_dock);

    ft_sharedptr<ft_building_definition> helios_beacon(new ft_building_definition());
    helios_beacon->id = BUILDING_HELIOS_BEACON;
    helios_beacon->name = ft_string("Helios Beacon");
    helios_beacon->width = 2;
    helios_beacon->height = 3;
    helios_beacon->logistic_cost = 0;
    helios_beacon->logistic_gain = 3;
    helios_beacon->energy_cost = 4.0;
    helios_beacon->energy_gain = 2.0;
    helios_beacon->cycle_time = 0.0;
    helios_beacon->inputs.clear();
    helios_beacon->outputs.clear();
    helios_beacon->build_costs.clear();
    recipe.key = ITEM_FUSION_REACTOR;
    recipe.value = 1;
    helios_beacon->build_costs.push_back(recipe);
    recipe.key = ORE_CRYSTAL;
    recipe.value = 10;
    helios_beacon->build_costs.push_back(recipe);
    recipe.key = ORE_TRITIUM;
    recipe.value = 6;
    helios_beacon->build_costs.push_back(recipe);
    helios_beacon->mine_bonus = 0.0;
    helios_beacon->unique = true;
    helios_beacon->occupies_grid = true;
    helios_beacon->removable = true;
    this->register_definition(helios_beacon);

    this->set_building_unlocked(BUILDING_MINE_CORE, true);
    this->set_building_unlocked(BUILDING_SMELTER, true);
    this->set_building_unlocked(BUILDING_PROCESSOR, true);
    this->set_building_unlocked(BUILDING_CRAFTING_BAY, true);
    this->set_building_unlocked(BUILDING_CONVEYOR, true);
    this->set_building_unlocked(BUILDING_TRANSFER_NODE, true);
    this->set_building_unlocked(BUILDING_TRADE_RELAY, false);
    this->set_building_unlocked(BUILDING_POWER_GENERATOR, true);
    this->set_building_unlocked(BUILDING_SOLAR_ARRAY, false);
    this->set_building_unlocked(BUILDING_UPGRADE_STATION, true);
    this->set_building_unlocked(BUILDING_FACILITY_WORKSHOP, false);
    this->set_building_unlocked(BUILDING_SHIPYARD, false);
    this->set_building_unlocked(BUILDING_PROXIMITY_RADAR, false);
    this->set_building_unlocked(BUILDING_MOBILE_RADAR, false);
    this->set_building_unlocked(BUILDING_SHIELD_GENERATOR, false);
    this->set_building_unlocked(BUILDING_TRITIUM_EXTRACTOR, false);
    this->set_building_unlocked(BUILDING_DEFENSE_TURRET, false);
    this->set_building_unlocked(BUILDING_PLASMA_TURRET, false);
    this->set_building_unlocked(BUILDING_RAILGUN_TURRET, false);
    this->set_building_unlocked(BUILDING_FLAGSHIP_DOCK, false);
    this->set_building_unlocked(BUILDING_HELIOS_BEACON, false);
}

