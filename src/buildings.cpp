#include "buildings.hpp"
#include "game.hpp"
#include "research.hpp"
#include "../libft/Libft/libft.hpp"

ft_planet_build_state::ft_planet_build_state()
    : planet_id(0), width(0), height(0), base_logistic(0), research_logistic_bonus(0), used_plots(0),
      logistic_capacity(0), logistic_usage(0), base_energy_generation(0.0),
      energy_generation(0.0), energy_consumption(0.0), support_energy(0.0),
      mine_multiplier(1.0), energy_deficit_pressure(0.0), next_instance_id(1), grid(), instances()
{
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

void BuildingManager::register_definition(const ft_sharedptr<ft_building_definition> &definition)
{
    this->_definitions.insert(definition->id, definition);
}

void BuildingManager::set_building_unlocked(int building_id, bool unlocked)
{
    Pair<int, bool> *entry = this->_building_unlocks.find(building_id);
    if (entry == ft_nullptr)
        this->_building_unlocks.insert(building_id, unlocked);
    else
        entry->value = unlocked;
}

bool BuildingManager::is_building_unlocked(int building_id) const
{
    const Pair<int, bool> *entry = this->_building_unlocks.find(building_id);
    if (entry == ft_nullptr)
        return true;
    return entry->value;
}

const ft_building_definition *BuildingManager::get_definition(int building_id) const
{
    const Pair<int, ft_sharedptr<ft_building_definition> > *entry = this->_definitions.find(building_id);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return entry->value.get();
}

ft_planet_build_state *BuildingManager::get_state(int planet_id)
{
    Pair<int, ft_planet_build_state> *entry = this->_planets.find(planet_id);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
}

const ft_planet_build_state *BuildingManager::get_state(int planet_id) const
{
    const Pair<int, ft_planet_build_state> *entry = this->_planets.find(planet_id);
    if (entry == ft_nullptr)
        return ft_nullptr;
    return &entry->value;
}

bool BuildingManager::is_area_free(const ft_planet_build_state &state, int x, int y, int width, int height) const
{
    if (x < 0 || y < 0 || width <= 0 || height <= 0)
        return false;
    if (x + width > state.width || y + height > state.height)
        return false;
    for (int dy = 0; dy < height; ++dy)
    {
        for (int dx = 0; dx < width; ++dx)
        {
            int index = (y + dy) * state.width + (x + dx);
            if (index < 0 || index >= static_cast<int>(state.grid.size()))
                return false;
            if (state.grid[index] != 0)
                return false;
        }
    }
    return true;
}

void BuildingManager::occupy_area(ft_planet_build_state &state, int instance_id, int x, int y, int width, int height)
{
    for (int dy = 0; dy < height; ++dy)
    {
        for (int dx = 0; dx < width; ++dx)
        {
            int index = (y + dy) * state.width + (x + dx);
            if (index >= 0 && index < static_cast<int>(state.grid.size()))
                state.grid[index] = instance_id;
        }
    }
    state.used_plots += width * height;
}

void BuildingManager::clear_area(ft_planet_build_state &state, int instance_id)
{
    int cleared = 0;
    for (size_t i = 0; i < state.grid.size(); ++i)
    {
        if (state.grid[i] == instance_id)
        {
            state.grid[i] = 0;
            ++cleared;
        }
    }
    if (cleared > 0 && state.used_plots >= cleared)
        state.used_plots -= cleared;
}

void BuildingManager::recalculate_planet_statistics(ft_planet_build_state &state)
{
    state.logistic_capacity = state.base_logistic + state.research_logistic_bonus;
    state.energy_generation = state.base_energy_generation;
    state.support_energy = 0.0;
    state.mine_multiplier = 1.0;
    state.energy_deficit_pressure = 0.0;
    size_t count = state.instances.size();
    Pair<int, ft_building_instance> *entries = state.instances.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
    {
        ft_building_instance &instance = entries[i].value;
        const ft_building_definition *definition = this->get_definition(instance.definition_id);
        if (definition == ft_nullptr)
            continue;
        state.logistic_capacity += definition->logistic_gain;
        state.energy_generation += definition->energy_gain;
        state.mine_multiplier += definition->mine_bonus;
        if (definition->energy_cost > 0.0 && (definition->cycle_time <= 0.0 || definition->outputs.size() == 0))
            state.support_energy += definition->energy_cost;
    }
    if (state.mine_multiplier < 1.0)
        state.mine_multiplier = 1.0;
    state.energy_consumption = state.support_energy;
    state.logistic_usage = 0;
}

bool BuildingManager::check_build_costs(const Game &game, int planet_id, const ft_vector<Pair<int, int> > &costs) const
{
    for (size_t i = 0; i < costs.size(); ++i)
    {
        int resource_id = costs[i].key;
        int required = costs[i].value;
        if (required <= 0)
            continue;
        int available = game.get_ore(planet_id, resource_id);
        if (available < required)
            return false;
    }
    return true;
}

void BuildingManager::pay_build_costs(Game &game, int planet_id, const ft_vector<Pair<int, int> > &costs)
{
    for (size_t i = 0; i < costs.size(); ++i)
    {
        int resource_id = costs[i].key;
        int amount = costs[i].value;
        if (amount <= 0)
            continue;
        game.sub_ore(planet_id, resource_id, amount);
    }
}

bool BuildingManager::consume_inputs(Game &game, int planet_id, const ft_vector<Pair<int, int> > &inputs)
{
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        int resource_id = inputs[i].key;
        int amount = inputs[i].value;
        if (amount <= 0)
            continue;
        if (game.get_ore(planet_id, resource_id) < amount)
            return false;
    }
    for (size_t i = 0; i < inputs.size(); ++i)
    {
        int resource_id = inputs[i].key;
        int amount = inputs[i].value;
        if (amount > 0)
            game.sub_ore(planet_id, resource_id, amount);
    }
    return true;
}

void BuildingManager::produce_outputs(Game &game, int planet_id, const ft_vector<Pair<int, int> > &outputs)
{
    for (size_t i = 0; i < outputs.size(); ++i)
    {
        int resource_id = outputs[i].key;
        int amount = outputs[i].value;
        if (amount > 0)
            game.add_ore(planet_id, resource_id, amount);
    }
}

void BuildingManager::ensure_outputs_registered(Game &game, int planet_id, const ft_vector<Pair<int, int> > &outputs)
{
    for (size_t i = 0; i < outputs.size(); ++i)
    {
        int resource_id = outputs[i].key;
        game.ensure_planet_item_slot(planet_id, resource_id);
    }
}

void BuildingManager::initialize_planet(Game &game, int planet_id)
{
    if (this->_planets.find(planet_id) != ft_nullptr)
        return ;
    this->_planets.insert(planet_id, ft_planet_build_state());
    Pair<int, ft_planet_build_state> *entry = this->_planets.find(planet_id);
    if (entry == ft_nullptr)
        return ;
    ft_planet_build_state &stored = entry->value;
    stored.planet_id = planet_id;
    stored.base_logistic = 1;
    stored.research_logistic_bonus = 0;
    stored.base_energy_generation = 0.0;
    if (planet_id == PLANET_TERRA)
    {
        stored.width = 4;
        stored.height = 4;
    }
    else if (planet_id == PLANET_MARS)
    {
        stored.width = 3;
        stored.height = 3;
    }
    else if (planet_id == PLANET_ZALTHOR)
    {
        stored.width = 3;
        stored.height = 4;
    }
    else if (planet_id == PLANET_VULCAN)
    {
        stored.width = 4;
        stored.height = 4;
    }
    else
    {
        stored.width = 4;
        stored.height = 5;
    }
    stored.grid.clear();
    stored.grid.resize(static_cast<size_t>(stored.width * stored.height), 0);
    stored.used_plots = 0;
    stored.logistic_capacity = 0;
    stored.logistic_usage = 0;
    stored.energy_generation = 0.0;
    stored.energy_consumption = 0.0;
    stored.support_energy = 0.0;
    stored.mine_multiplier = 1.0;
    stored.next_instance_id = 1;
    stored.instances.clear();
    const ft_building_definition *mine = this->get_definition(BUILDING_MINE_CORE);
    if (mine != ft_nullptr)
    {
        if (mine->occupies_grid)
            this->occupy_area(stored, stored.next_instance_id, 0, 0, mine->width, mine->height);
        ft_building_instance instance;
        instance.uid = stored.next_instance_id++;
        instance.definition_id = BUILDING_MINE_CORE;
        instance.x = 0;
        instance.y = 0;
        instance.progress = 0.0;
        instance.active = false;
        stored.instances.insert(instance.uid, instance);
    }
    recalculate_planet_statistics(stored);
    game.ensure_planet_item_slot(planet_id, ITEM_IRON_BAR);
    game.ensure_planet_item_slot(planet_id, ITEM_COPPER_BAR);
    game.ensure_planet_item_slot(planet_id, ITEM_MITHRIL_BAR);
    game.ensure_planet_item_slot(planet_id, ITEM_ENGINE_PART);
    game.ensure_planet_item_slot(planet_id, ITEM_TITANIUM_BAR);
    game.ensure_planet_item_slot(planet_id, ITEM_ADVANCED_ENGINE_PART);
    game.ensure_planet_item_slot(planet_id, ITEM_FUSION_REACTOR);
    game.ensure_planet_item_slot(planet_id, ITEM_ACCUMULATOR);
    game.ensure_planet_item_slot(planet_id, ORE_TRITIUM);
}

void BuildingManager::add_planet_logistic_bonus(int planet_id, int amount)
{
    if (amount <= 0)
        return ;
    ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return ;
    state->research_logistic_bonus += amount;
    if (state->research_logistic_bonus < 0)
        state->research_logistic_bonus = 0;
    recalculate_planet_statistics(*state);
}

void BuildingManager::apply_research_unlock(int research_id)
{
    if (research_id == RESEARCH_SOLAR_PANELS)
        this->set_building_unlocked(BUILDING_SOLAR_ARRAY, true);
    else if (research_id == RESEARCH_CRAFTING_MASTERY)
    {
        this->set_building_unlocked(BUILDING_FACILITY_WORKSHOP, true);
        this->set_building_unlocked(BUILDING_SHIPYARD, true);
    }
    else if (research_id == RESEARCH_DEFENSIVE_FORTIFICATION_I)
    {
        this->set_building_unlocked(BUILDING_PROXIMITY_RADAR, true);
        this->set_building_unlocked(BUILDING_DEFENSE_TURRET, true);
    }
    else if (research_id == RESEARCH_DEFENSIVE_FORTIFICATION_II)
        this->set_building_unlocked(BUILDING_MOBILE_RADAR, true);
    else if (research_id == RESEARCH_DEFENSIVE_FORTIFICATION_III)
        this->set_building_unlocked(BUILDING_HELIOS_BEACON, true);
    else if (research_id == RESEARCH_ARMAMENT_ENHANCEMENT_I)
        this->set_building_unlocked(BUILDING_PLASMA_TURRET, true);
    else if (research_id == RESEARCH_ARMAMENT_ENHANCEMENT_III)
        this->set_building_unlocked(BUILDING_RAILGUN_TURRET, true);
    else if (research_id == RESEARCH_SHIELD_TECHNOLOGY)
        this->set_building_unlocked(BUILDING_SHIELD_GENERATOR, true);
    else if (research_id == RESEARCH_TRITIUM_EXTRACTION)
        this->set_building_unlocked(BUILDING_TRITIUM_EXTRACTOR, true);
    else if (research_id == RESEARCH_CAPITAL_SHIP_INITIATIVE)
        this->set_building_unlocked(BUILDING_FLAGSHIP_DOCK, true);
}

void BuildingManager::set_crafting_energy_multiplier(double multiplier)
{
    if (multiplier <= 0.0)
        multiplier = 1.0;
    this->_crafting_energy_multiplier = multiplier;
}

void BuildingManager::set_crafting_speed_multiplier(double multiplier)
{
    if (multiplier <= 0.0)
        multiplier = 1.0;
    this->_crafting_speed_multiplier = multiplier;
}

void BuildingManager::set_global_energy_multiplier(double multiplier)
{
    if (multiplier <= 0.0)
        multiplier = 1.0;
    this->_global_energy_multiplier = multiplier;
}

int BuildingManager::place_building(Game &game, int planet_id, int building_id, int x, int y)
{
    ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    const ft_building_definition *definition = this->get_definition(building_id);
    if (definition == ft_nullptr)
        return 0;
    if (!this->is_building_unlocked(building_id))
        return 0;
    if (definition->unique && this->get_building_count(planet_id, building_id) > 0)
        return 0;
    if (definition->occupies_grid && !this->is_area_free(*state, x, y, definition->width, definition->height))
        return 0;
    if (!this->check_build_costs(game, planet_id, definition->build_costs))
        return 0;
    this->pay_build_costs(game, planet_id, definition->build_costs);
    if (definition->occupies_grid)
        this->occupy_area(*state, state->next_instance_id, x, y, definition->width, definition->height);
    ft_building_instance instance;
    instance.uid = state->next_instance_id++;
    instance.definition_id = building_id;
    instance.x = x;
    instance.y = y;
    instance.progress = 0.0;
    instance.active = false;
    state->instances.insert(instance.uid, instance);
    this->ensure_outputs_registered(game, planet_id, definition->outputs);
    recalculate_planet_statistics(*state);
    return instance.uid;
}

bool BuildingManager::remove_building(Game &game, int planet_id, int instance_id)
{
    (void)game;
    ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return false;
    Pair<int, ft_building_instance> *entry = state->instances.find(instance_id);
    if (entry == ft_nullptr)
        return false;
    const ft_building_definition *definition = this->get_definition(entry->value.definition_id);
    if (definition == ft_nullptr)
        return false;
    if (!definition->removable)
        return false;
    this->clear_area(*state, instance_id);
    state->instances.remove(instance_id);
    recalculate_planet_statistics(*state);
    return true;
}

bool BuildingManager::can_place_building(const Game &game, int planet_id, int building_id, int x, int y) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return false;
    const ft_building_definition *definition = this->get_definition(building_id);
    if (definition == ft_nullptr)
        return false;
    if (!this->is_building_unlocked(building_id))
        return false;
    if (definition->unique && this->get_building_count(planet_id, building_id) > 0)
        return false;
    if (definition->occupies_grid && !this->is_area_free(*state, x, y, definition->width, definition->height))
        return false;
    if (!this->check_build_costs(game, planet_id, definition->build_costs))
        return false;
    return true;
}

int BuildingManager::get_building_instance(int planet_id, int x, int y) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    if (x < 0 || y < 0 || x >= state->width || y >= state->height)
        return 0;
    int index = y * state->width + x;
    if (index < 0 || index >= static_cast<int>(state->grid.size()))
        return 0;
    return state->grid[index];
}

int BuildingManager::get_building_count(int planet_id, int building_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    int count = 0;
    size_t total = state->instances.size();
    const Pair<int, ft_building_instance> *entries = state->instances.end();
    entries -= total;
    for (size_t i = 0; i < total; ++i)
    {
        if (entries[i].value.definition_id == building_id)
            ++count;
    }
    return count;
}

int BuildingManager::get_planet_plot_capacity(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    return state->width * state->height;
}

int BuildingManager::get_planet_plot_usage(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    return state->used_plots;
}

int BuildingManager::get_planet_logistic_capacity(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    return state->logistic_capacity;
}

int BuildingManager::get_planet_logistic_usage(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0;
    return state->logistic_usage;
}

double BuildingManager::get_planet_energy_generation(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0.0;
    return state->energy_generation;
}

double BuildingManager::get_planet_energy_consumption(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0.0;
    return state->energy_consumption;
}

double BuildingManager::get_mine_multiplier(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 1.0;
    return state->mine_multiplier;
}

double BuildingManager::get_planet_energy_pressure(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0.0;
    if (state->energy_deficit_pressure < 0.0)
        return 0.0;
    return state->energy_deficit_pressure;
}

void BuildingManager::tick_planet(Game &game, ft_planet_build_state &state, double seconds)
{
    state.energy_deficit_pressure = 0.0;
    state.energy_consumption = state.support_energy;
    if (state.energy_consumption > state.energy_generation)
    {
        state.energy_deficit_pressure += state.energy_consumption - state.energy_generation;
        state.energy_consumption = state.energy_generation;
    }
    state.logistic_usage = 0;
    size_t total = state.instances.size();
    Pair<int, ft_building_instance> *entries = state.instances.end();
    entries -= total;
    for (size_t i = 0; i < total; ++i)
        entries[i].value.active = false;
    for (size_t i = 0; i < total; ++i)
    {
        ft_building_instance &instance = entries[i].value;
        const ft_building_definition *definition = this->get_definition(instance.definition_id);
        if (definition == ft_nullptr)
            continue;
        if (definition->cycle_time <= 0.0 || definition->outputs.size() == 0)
            continue;
        bool can_run = true;
        if (definition->logistic_cost > 0)
        {
            if (state.logistic_usage + definition->logistic_cost > state.logistic_capacity)
                can_run = false;
        }
        double energy_cost = definition->energy_cost * this->_global_energy_multiplier;
        if (definition->cycle_time > 0.0 && definition->outputs.size() > 0)
            energy_cost *= this->_crafting_energy_multiplier;
        if (energy_cost > 0.0)
        {
            double projected = state.energy_consumption + energy_cost;
            if (projected > state.energy_generation + 0.0001)
            {
                can_run = false;
                double available = state.energy_generation - state.energy_consumption;
                if (available < 0.0)
                    available = 0.0;
                double shortage = energy_cost - available;
                if (shortage < 0.0)
                    shortage = 0.0;
                state.energy_deficit_pressure += shortage;
            }
        }
        if (!can_run)
        {
            instance.progress = definition->cycle_time;
            continue;
        }
        instance.active = true;
        state.logistic_usage += definition->logistic_cost;
        state.energy_consumption += energy_cost;
    }
    for (size_t i = 0; i < total; ++i)
    {
        ft_building_instance &instance = entries[i].value;
        const ft_building_definition *definition = this->get_definition(instance.definition_id);
        if (definition == ft_nullptr)
            continue;
        if (definition->cycle_time <= 0.0 || definition->outputs.size() == 0)
            continue;
        if (!instance.active)
            continue;
        double elapsed = seconds;
        if (definition->cycle_time > 0.0 && definition->outputs.size() > 0)
            elapsed *= this->_crafting_speed_multiplier;
        instance.progress += elapsed;
        while (instance.progress >= definition->cycle_time)
        {
            if (!this->consume_inputs(game, state.planet_id, definition->inputs))
            {
                instance.progress = definition->cycle_time;
                break;
            }
            instance.progress -= definition->cycle_time;
            this->produce_outputs(game, state.planet_id, definition->outputs);
        }
    }
}

void BuildingManager::tick(Game &game, double seconds)
{
    size_t count = this->_planets.size();
    Pair<int, ft_planet_build_state> *entries = this->_planets.end();
    entries -= count;
    for (size_t i = 0; i < count; ++i)
        this->tick_planet(game, entries[i].value, seconds);
}
