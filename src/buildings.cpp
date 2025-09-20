#include "buildings.hpp"
#include "game.hpp"
#include "research.hpp"
#include "../libft/Libft/libft.hpp"

void BuildingManager::clone_from(const BuildingManager &other)
{
    if (this == &other)
        return ;
    this->_crafting_energy_multiplier = other._crafting_energy_multiplier;
    this->_crafting_speed_multiplier = other._crafting_speed_multiplier;
    this->_global_energy_multiplier = other._global_energy_multiplier;
    this->_building_unlocks.clear();
    size_t unlock_count = other._building_unlocks.size();
    if (unlock_count > 0)
    {
        const Pair<int, bool> *unlock_entries = other._building_unlocks.end();
        unlock_entries -= unlock_count;
        for (size_t i = 0; i < unlock_count; ++i)
            this->_building_unlocks.insert(unlock_entries[i].key, unlock_entries[i].value);
    }
    this->_planets.clear();
    size_t planet_count = other._planets.size();
    if (planet_count == 0)
        return ;
    const Pair<int, ft_planet_build_state> *entries = other._planets.end();
    entries -= planet_count;
    for (size_t i = 0; i < planet_count; ++i)
    {
        const ft_planet_build_state &source = entries[i].value;
        this->_planets.insert(entries[i].key, ft_planet_build_state());
        Pair<int, ft_planet_build_state> *destination = this->_planets.find(entries[i].key);
        if (destination == ft_nullptr)
            continue;
        ft_planet_build_state &state = destination->value;
        state.planet_id = source.planet_id;
        state.width = source.width;
        state.height = source.height;
        state.base_logistic = source.base_logistic;
        state.research_logistic_bonus = source.research_logistic_bonus;
        state.used_plots = source.used_plots;
        state.logistic_capacity = source.logistic_capacity;
        state.logistic_usage = source.logistic_usage;
        state.base_energy_generation = source.base_energy_generation;
        state.energy_generation = source.energy_generation;
        state.energy_consumption = source.energy_consumption;
        state.support_energy = source.support_energy;
        state.mine_multiplier = source.mine_multiplier;
        state.convoy_speed_bonus = source.convoy_speed_bonus;
        state.convoy_raid_risk_modifier = source.convoy_raid_risk_modifier;
        state.energy_deficit_pressure = source.energy_deficit_pressure;
        state.next_instance_id = source.next_instance_id;
        state.grid.clear();
        size_t grid_size = source.grid.size();
        if (grid_size > 0)
        {
            state.grid.reserve(grid_size);
            for (size_t j = 0; j < grid_size; ++j)
                state.grid.push_back(source.grid[j]);
        }
        state.instances.clear();
        size_t instance_count = source.instances.size();
        if (instance_count > 0)
        {
            const Pair<int, ft_building_instance> *inst_entries = source.instances.end();
            inst_entries -= instance_count;
            for (size_t j = 0; j < instance_count; ++j)
                state.instances.insert(inst_entries[j].key, inst_entries[j].value);
        }
    }
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
    state.convoy_speed_bonus = 0.0;
    state.convoy_raid_risk_modifier = 0.0;
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
        state.convoy_speed_bonus += definition->convoy_speed_bonus;
        state.convoy_raid_risk_modifier += definition->convoy_raid_risk_modifier;
        if (definition->energy_cost > 0.0 && (definition->cycle_time <= 0.0 || definition->outputs.size() == 0))
            state.support_energy += definition->energy_cost;
    }
    if (state.mine_multiplier < 1.0)
        state.mine_multiplier = 1.0;
    if (state.convoy_speed_bonus < 0.0)
        state.convoy_speed_bonus = 0.0;
    if (state.convoy_raid_risk_modifier < 0.0)
        state.convoy_raid_risk_modifier = 0.0;
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
    else if (research_id == RESEARCH_INTERSTELLAR_TRADE)
        this->set_building_unlocked(BUILDING_TRADE_RELAY, true);
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

double BuildingManager::get_planet_convoy_speed_bonus(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0.0;
    if (state->convoy_speed_bonus < 0.0)
        return 0.0;
    return state->convoy_speed_bonus;
}

double BuildingManager::get_planet_convoy_raid_risk_modifier(int planet_id) const
{
    const ft_planet_build_state *state = this->get_state(planet_id);
    if (state == ft_nullptr)
        return 0.0;
    if (state->convoy_raid_risk_modifier < 0.0)
        return 0.0;
    return state->convoy_raid_risk_modifier;
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
            if (instance.progress > definition->cycle_time)
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
