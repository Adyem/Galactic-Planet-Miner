#include "game.hpp"

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
    else if (quest_id == QUEST_ORDER_SUPPRESS_RAIDS)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_MARS);
        if (route != ft_nullptr)
            this->modify_route_threat(*route, 1.5, true);
        entry = ft_string("Marshal Rhea fumes as raider cells resurge and convoys report renewed ambushes.");
    }
    else if (quest_id == QUEST_ORDER_DOMINION)
    {
        this->_order_branch_pending_assault = 0;
        this->sub_ore(PLANET_TERRA, ORE_TITANIUM, 2);
        entry = ft_string("Professor Lumen laments that Dominion plans falter and titanium stockpiles burn away.");
    }
    else if (quest_id == QUEST_REBELLION_NETWORK)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
        if (route != ft_nullptr)
            this->modify_route_threat(*route, 1.0, true);
        entry = ft_string("Navigator Zara whispers that rebel relays fall dark as raids intensify.");
    }
    else if (quest_id == QUEST_REBELLION_LIBERATION)
    {
        this->_rebellion_branch_pending_assault = 0;
        this->sub_ore(PLANET_NOCTARIS_PRIME, ORE_OBSIDIAN, 3);
        entry = ft_string("Old Miner Joe recounts how the liberation bid falters and obsidian caches are seized.");
    }
    else if (quest_id == QUEST_SIDE_CONVOY_RESCUE)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_MARS);
        if (route != ft_nullptr)
            this->modify_route_threat(*route, 0.5, true);
        entry = ft_string("Old Miner Joe mourns that the relief convoy never reached the stranded habitats.");
    }
    else if (quest_id == QUEST_SIDE_OUTPOST_REPAIR)
    {
        this->sub_ore(PLANET_MARS, ITEM_ENGINE_PART, 1);
        entry = ft_string("Professor Lumen reports that redline outposts remain dark without the promised repairs.");
    }
    else if (quest_id == QUEST_SIDE_ORDER_PROPAGANDA)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_MARS);
        if (route != ft_nullptr)
            this->modify_route_threat(*route, 0.6, true);
        entry = ft_string("Marshal Rhea fumes as loyalist broadcasts fail to quiet the raider chatter.");
    }
    else if (quest_id == QUEST_SIDE_REBELLION_BROADCAST)
    {
        ft_supply_route *route = this->ensure_supply_route(PLANET_TERRA, PLANET_NOCTARIS_PRIME);
        if (route != ft_nullptr)
            this->modify_route_threat(*route, 0.5, true);
        entry = ft_string("Captain Blackthorne laments that rebel relays went silent before the uprising could spark.");
    }
    if (entry.size() > 0)
        this->append_lore_entry(entry);
}
