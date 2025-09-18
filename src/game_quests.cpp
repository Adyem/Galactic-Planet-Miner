#include "game.hpp"
#include "../libft/Libft/libft.hpp"
#include "../libft/Template/pair.hpp"

void Game::build_quest_context(ft_quest_context &context) const
{
    size_t planet_count = this->_planets.size();
    const Pair<int, ft_sharedptr<ft_planet> > *planet_entries = this->_planets.end();
    planet_entries -= planet_count;
    for (size_t i = 0; i < planet_count; ++i)
    {
        const ft_sharedptr<ft_planet> &planet = planet_entries[i].value;
        const ft_vector<Pair<int, double> > &resources = planet->get_resources();
        for (size_t j = 0; j < resources.size(); ++j)
        {
            int ore_id = resources[j].key;
            int amount = planet->get_resource(ore_id);
            Pair<int, int> *entry = context.resource_totals.find(ore_id);
            if (entry == ft_nullptr)
                context.resource_totals.insert(ore_id, amount);
            else
                entry->value += amount;
        }
    }

    context.research_status.insert(RESEARCH_UNLOCK_MARS, this->_research.is_completed(RESEARCH_UNLOCK_MARS) ? 1 : 0);
    context.research_status.insert(RESEARCH_UNLOCK_ZALTHOR, this->_research.is_completed(RESEARCH_UNLOCK_ZALTHOR) ? 1 : 0);
    context.research_status.insert(RESEARCH_UNLOCK_VULCAN, this->_research.is_completed(RESEARCH_UNLOCK_VULCAN) ? 1 : 0);
    context.research_status.insert(RESEARCH_UNLOCK_NOCTARIS, this->_research.is_completed(RESEARCH_UNLOCK_NOCTARIS) ? 1 : 0);
    context.research_status.insert(RESEARCH_SOLAR_PANELS, this->_research.is_completed(RESEARCH_SOLAR_PANELS) ? 1 : 0);
    context.research_status.insert(RESEARCH_CRAFTING_MASTERY, this->_research.is_completed(RESEARCH_CRAFTING_MASTERY) ? 1 : 0);
    context.research_status.insert(RESEARCH_SHIELD_TECHNOLOGY, this->_research.is_completed(RESEARCH_SHIELD_TECHNOLOGY) ? 1 : 0);
    context.research_status.insert(RESEARCH_REPAIR_DRONE_TECHNOLOGY, this->_research.is_completed(RESEARCH_REPAIR_DRONE_TECHNOLOGY) ? 1 : 0);
    context.research_status.insert(RESEARCH_CAPITAL_SHIP_INITIATIVE, this->_research.is_completed(RESEARCH_CAPITAL_SHIP_INITIATIVE) ? 1 : 0);

    size_t fleet_count = this->_fleets.size();
    const Pair<int, ft_sharedptr<ft_fleet> > *fleet_entries = this->_fleets.end();
    fleet_entries -= fleet_count;
    for (size_t i = 0; i < fleet_count; ++i)
    {
        const ft_sharedptr<ft_fleet> &fleet = fleet_entries[i].value;
        context.total_ship_count += fleet->get_ship_count();
        context.total_ship_hp += fleet->get_total_ship_hp();
    }
    size_t garrison_count = this->_planet_fleets.size();
    const Pair<int, ft_sharedptr<ft_fleet> > *garrison_entries = this->_planet_fleets.end();
    garrison_entries -= garrison_count;
    for (size_t i = 0; i < garrison_count; ++i)
    {
        const ft_sharedptr<ft_fleet> &fleet = garrison_entries[i].value;
        context.total_ship_count += fleet->get_ship_count();
        context.total_ship_hp += fleet->get_total_ship_hp();
    }
    context.successful_deliveries = this->_convoys_delivered_total;
    context.convoy_raid_losses = this->_convoy_raid_losses;
    context.delivery_streak = this->_current_delivery_streak;
}

void Game::handle_quest_completion(int quest_id)
{
    ft_string entry;
    if (quest_id == QUEST_INITIAL_SKIRMISHES)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ITEM_ENGINE_PART);
        this->add_ore(PLANET_TERRA, ITEM_ENGINE_PART, 2);
        entry = ft_string("Old Miner Joe cheers as Terra's convoys return with salvaged engine parts.");
    }
    else if (quest_id == QUEST_DEFENSE_OF_TERRA)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_COAL);
        this->add_ore(PLANET_TERRA, ORE_COAL, 6);
        entry = ft_string("Professor Lumen catalogs the victory over Terra: new coal shipments fuel the forges.");
    }
    else if (quest_id == QUEST_INVESTIGATE_RAIDERS)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_MITHRIL);
        this->add_ore(PLANET_TERRA, ORE_MITHRIL, 4);
        entry = ft_string("Farmer Daisy archives decoded mithril caches that hint at raider supply routes.");
    }
    else if (quest_id == QUEST_CLIMACTIC_BATTLE)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_TITANIUM);
        this->add_ore(PLANET_TERRA, ORE_TITANIUM, 3);
        entry = ft_string("Old Scout Finn records the climactic stand: captured titanium plating is repurposed.");
    }
    else if (quest_id == QUEST_ORDER_UPRISING)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ITEM_ENGINE_PART);
        this->add_ore(PLANET_TERRA, ITEM_ENGINE_PART, 3);
        entry = ft_string("Farmer Daisy distributes engine parts to keep loyalist transports operational.");
    }
    else if (quest_id == QUEST_REBELLION_FLEET)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_OBSIDIAN);
        this->add_ore(PLANET_TERRA, ORE_OBSIDIAN, 2);
        entry = ft_string("Professor Lumen preserves obsidian shards from allied rebels as evidence of hope.");
    }
    if (entry.size() > 0)
        this->_lore_log.push_back(entry);
}

void Game::handle_quest_failure(int quest_id)
{
    ft_string entry;
    if (quest_id == QUEST_DEFENSE_OF_TERRA)
    {
        this->sub_ore(PLANET_TERRA, ITEM_ENGINE_PART, 1);
        entry = ft_string("Professor Lumen warns that Terra's defenses falter and precious engine parts are lost.");
    }
    else if (quest_id == QUEST_ORDER_UPRISING)
    {
        this->sub_ore(PLANET_TERRA, ORE_COAL, 4);
        entry = ft_string("Old Miner Joe laments that unrest drains coal reserves meant for the foundries.");
    }
    else if (quest_id == QUEST_REBELLION_FLEET)
    {
        this->sub_ore(PLANET_TERRA, ORE_MITHRIL, 2);
        entry = ft_string("Farmer Daisy notes that promised mithril reinforcements never arrive for the rebellion.");
    }
    if (entry.size() > 0)
        this->_lore_log.push_back(entry);
}

void Game::handle_quest_choice_prompt(int quest_id)
{
    if (quest_id != QUEST_CRITICAL_DECISION)
        return ;
    ft_string entry("Navigator Zara's sacrifice forces a reckoning over Captain Blackthorne's fate.");
    this->_lore_log.push_back(entry);
}

void Game::handle_quest_choice_resolution(int quest_id, int choice_id)
{
    if (quest_id != QUEST_CRITICAL_DECISION)
        return ;
    ft_string entry;
    if (choice_id == QUEST_CHOICE_EXECUTE_BLACKTHORNE)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_COAL);
        this->add_ore(PLANET_TERRA, ORE_COAL, 5);
        entry = ft_string("Captain Blackthorne's execution steels Terra's loyalists; coal stockpiles surge for the war effort.");
    }
    else if (choice_id == QUEST_CHOICE_SPARE_BLACKTHORNE)
    {
        this->ensure_planet_item_slot(PLANET_TERRA, ORE_CRYSTAL);
        this->add_ore(PLANET_TERRA, ORE_CRYSTAL, 3);
        entry = ft_string("Sparing Blackthorne yields encoded crystal data that Professor Lumen studies for hidden conspiracies.");
    }
    if (entry.size() > 0)
        this->_lore_log.push_back(entry);
}

bool Game::resolve_quest_choice(int quest_id, int choice_id)
{
    if (!this->_quests.make_choice(quest_id, choice_id))
        return false;
    ft_quest_context context;
    this->build_quest_context(context);
    ft_vector<int> quest_completed;
    ft_vector<int> quest_failed;
    ft_vector<int> quest_choices;
    this->_quests.update(0.0, context, quest_completed, quest_failed, quest_choices);
    (void)quest_completed;
    (void)quest_failed;
    (void)quest_choices;
    this->handle_quest_choice_resolution(quest_id, choice_id);
    return true;
}

int Game::get_active_quest() const
{
    return this->_quests.get_active_quest_id();
}

int Game::get_quest_status(int quest_id) const
{
    return this->_quests.get_status(quest_id);
}

double Game::get_quest_time_remaining(int quest_id) const
{
    return this->_quests.get_time_remaining(quest_id);
}

int Game::get_quest_choice(int quest_id) const
{
    return this->_quests.get_choice(quest_id);
}

const ft_vector<ft_string> &Game::get_lore_log() const
{
    return this->_lore_log;
}

