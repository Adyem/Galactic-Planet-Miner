#include "game_data.h"

const double SOLAR_SHIP_BONUS_RATE = 5.0;
const int SUNFLARE_SHIELD_REGEN = 10;

const std::vector<std::string> resourceLore = {
    "Old Miner Joe: 'The veins of our beloved planet run deep—every ounce of ore fuels our future. I once lost everything to a mining accident, and now I seek redemption in every pick strike.'",
    "Professor Lumen: 'Mining is the backbone of our civilization. Behind every ore lies a story of sacrifice and hope, echoing the trials of those who came before us.'",
    "Farmer Daisy: 'Though I tend my fields, I know that without the mines, our tables would be empty. The earth gives generously, yet it also demands respect for its hidden treasures.'"
};

const std::vector<std::string> raiderLore = {
    "Captain Blackthorne: 'I never chose this path. Forced into desperation by a system that failed us, we strike not out of malice, but as a cry for justice.'",
    "Navigator Zara: 'Every course we chart is a reminder of the choices we were left with. We become raiders because survival left us no other way, not because we relish conflict.'",
    "Old Scout Finn: 'In the void, I've seen the truth—our fury is the echo of neglect, a tragic response by souls abandoned by hope.'"
};

const std::vector<std::string> genericLore = {
    "Town Crier: 'The times are hard, but unity and perseverance will see us through. Remember, every challenge is an opportunity in disguise.'",
    "Scribe Alric: 'Every day brings new challenges—and new opportunities—for those brave enough to seize them. History is written by those who dare to dream.'"
};

const std::string SAVE_FILE = "savegame.json";

const std::vector<ResourceDef> RESOURCE_DATA = {
    {"Iron", 0}, {"Copper", 0}, {"Mithril", 0}, {"Coal", 0}, {"Tin", 0},
    {"Silver", 0}, {"Gold", 0}, {"Iron Bar", 0}, {"Copper Bar", 0}, {"Mithril Bar", 0},
    {"Engine Parts", 0}, {"Titanium", 0}, {"Titanium Bar", 0}, {"Generator", 0},
    {"Accumulator", 0}, {"Obsidian", 0}, {"Crystal", 0}, {"Nanomaterial", 0},
    {"Advanced Engine Parts", 0}, {"Fusion Reactor", 0}, {"Tritium", 0}
};

const std::vector<PlanetDef> PLANET_DATA = {
    {"Terra", {{"Iron", 0.5}, {"Copper", 0.5}, {"Coal", 0.2}}, true},
    {"Mars", {{"Iron", 0.1}, {"Copper", 0.1}, {"Mithril", 0.05}, {"Coal", 0.1}}, false},
    {"Zalthor", {{"Mithril", 0.1}, {"Coal", 0.2}, {"Gold", 0.02}}, false},
    {"Vulcan", {{"Tin", 0.1}, {"Silver", 0.03}, {"Titanium", 0.01}}, false},
    {"Luna", {{"Obsidian", 0.1}, {"Crystal", 0.05}, {"Nanomaterial", 0.02}}, false}
};

const std::vector<ResearchDef> RESEARCH_DATA = {
    {"Unlock Mars", {{"Iron", 100}, {"Copper", 50}},
     "Unlock the planet Mars for mining.", "unlock_mars"},
    {"Unlock Zalthor", {{"Iron", 200}, {"Mithril", 50}},
     "Unlock planet Zalthor for advanced mining.", "unlock_zalthor"},
    {"Faster Crafting", {{"Iron", 50}, {"Coal", 30}},
     "Reduce smelting time by half.", "faster_crafting"},
    {"Unlock Vulcan", {{"Gold", 100}, {"Mithril", 100}},
     "Unlock planet Vulcan for high‑value resources.", "unlock_vulcan"},
    {"Unlock Luna", {{"Gold", 150}, {"Mithril", 100}},
     "Unlock planet Luna to harvest exotic materials.", "unlock_luna"},
    {"Crafting Mastery", {{"Engine Parts", 5}, {"Titanium Bar", 3}},
     "Reduces crafting energy cost by 20%.", "crafting_mastery"},
    {"Precision Tools", {{"Iron Bar", 10}, {"Mithril Bar", 5}},
     "Gives a 10% chance to double crafted output.", "precision_tools"},
    {"Shield Technology", {{"Mithril Bar", 10}, {"Titanium Bar", 5}},
     "Allows building Shield Ships to protect convoys.", "unlock_shield_ships"},
    {"Emergency Energy Conservation",
     {{"Engine Parts", 10}, {"Titanium Bar", 5}},
     "Allows a planet to halt production during an imminent raider attack, conserving energy for defense.", "energy_conservation"},
    {"Repair Drone Technology", {{"Fusion Reactor", 1}, {"Advanced Engine Parts", 3}},
     "Unlocks Repair Drones that can repair ships during combat.", "unlock_repair_drones"},
    {"Solar Panels", {{"Iron", 20}, {"Copper", 30}},
     "Unlock Solar Panels for energy production on planets.", "unlock_solar_panels"},
    {"Tritium Extraction", {{"Mithril Bar", 5}, {"Advanced Engine Parts", 2}},
     "Unlocks the ability to build Tritium Extractors to harvest tritium for Fusion Reactors.", "unlock_tritium_extraction"},
    {"Urban Planning I", {{"Iron Bar", 15}, {"Engine Parts", 5}},
     "Increases Terra's building capacity by 4.", "urban_planning_terra"},
    {"Urban Planning II", {{"Iron Bar", 30}, {"Engine Parts", 10}},
     "Increases Mars' building capacity by 4.", "urban_planning_mars"},
    {"Urban Planning III", {{"Iron Bar", 45}, {"Engine Parts", 15}},
     "Increases Zalthor's building capacity by 4.", "urban_planning_zalthor"},
    {"Capital Ship Initiative",
     {{"Advanced Engine Parts", 5}, {"Fusion Reactor", 1}, {"Titanium Bar", 10}},
     "Unlock the ability to construct a powerful capital ship. Only one such vessel may be active at any time.", "unlock_capital_ships"},
    {"Auxiliary Frigate Development",
     {{"Engine Parts", 20}, {"Advanced Engine Parts", 5}, {"Mithril Bar", 10}},
     "Unlock smaller versions of capital ships (frigates) that are less powerful but can be built without limits.", "unlock_capital_frigates"},
    {"Escape Pod Lifeline", {{"Advanced Engine Parts", 5}, {"Titanium Bar", 3}},
         "Allows repair drones and Sunflare Sloops to detach upon ship destruction with a 50% chance to survive.", "escape_pod_lifeline"},
    {"Armament Enhancement I", {{"Engine Parts", 10}, {"Titanium Bar", 5}},
         "Increase all ships' weapons by 10%.", "weapons_upgrade_1"},
    {"Armament Enhancement II", {{"Engine Parts", 20}, {"Titanium Bar", 10}},
         "Increase all ships' weapons by an additional 10%. Requires previous tier.", "weapons_upgrade_2"},
    {"Armament Enhancement III", {{"Engine Parts", 30}, {"Titanium Bar", 15}},
         "Increase all ships' weapons by an additional 10%. Requires previous tier.", "weapons_upgrade_3"},
    {"Defensive Fortification I", {{"Copper Bar", 10}, {"Mithril Bar", 5}},
         "Increase all ships' shields by 10%.", "shield_upgrade_1"},
    {"Defensive Fortification II", {{"Copper Bar", 20}, {"Mithril Bar", 10}},
         "Increase all ships' shields by an additional 10%. Requires previous tier.", "shield_upgrade_2"},
    {"Defensive Fortification III", {{"Copper Bar", 30}, {"Mithril Bar", 15}},
         "Increase all ships' shields by an additional 10%. Requires previous tier.", "shield_upgrade_3"},
    {"Structural Reinforcement I", {{"Iron Bar", 10}, {"Coal", 10}},
         "Increase all ships' hull by 10%.", "hull_upgrade_1"},
    {"Structural Reinforcement II", {{"Iron Bar", 20}, {"Coal", 20}},
         "Increase all ships' hull by an additional 10%. Requires previous tier.", "hull_upgrade_2"},
    {"Structural Reinforcement III", {{"Iron Bar", 30}, {"Coal", 30}},
         "Increase all ships' hull by an additional 10%. Requires previous tier.", "hull_upgrade_3"}
};

const std::map<std::string, CraftingRecipe> CRAFTING_RECIPES = {
    {"Iron Bar", { {{"Iron", 5}}, 1.0, 2.0, "Smelting Building" }},
    {"Interceptor", { {{"Engine Parts", 3}, {"Titanium Bar", 2}}, 4.0, 20.0, "Shipyard" }},
    {"Celestial Juggernaut", { {{"Fusion Reactor", 1}, {"Titanium Bar", 10},
                                 {"Advanced Engine Parts", 5}, {"Crystal", 5}},
                                20.0, 40.0, "Flagship Dock" }},
    {"Nova Carrier", { {{"Fusion Reactor", 1}, {"Titanium Bar", 8},
                         {"Advanced Engine Parts", 4}, {"Mithril Bar", 10}},
                        18.0, 35.0, "Flagship Dock" }},
    {"Obsidian Sovereign", { {{"Fusion Reactor", 1}, {"Titanium Bar", 12},
                               {"Advanced Engine Parts", 6}, {"Crystal", 7}},
                              25.0, 45.0, "Flagship Dock" }},
    {"Preemptor", { {{"Fusion Reactor", 1}, {"Titanium Bar", 9},
                      {"Advanced Engine Parts", 5}, {"Mithril Bar", 8}},
                    15.0, 50.0, "Flagship Dock" }},
    {"Aurora Protector", { {{"Fusion Reactor", 1}, {"Titanium Bar", 10},
                             {"Advanced Engine Parts", 5}, {"Crystal", 6}},
                           20.0, 30.0, "Flagship Dock" }},
    {"Juggernaut Frigate", { {{"Titanium Bar", 5}, {"Advanced Engine Parts", 2},
                              {"Crystal", 2}},
                            10.0, 15.0, "Shipyard" }},
    {"Carrier Frigate", { {{"Titanium Bar", 4}, {"Advanced Engine Parts", 2},
                           {"Mithril Bar", 5}},
                         9.0, 12.0, "Shipyard" }},
    {"Sovereign Frigate", { {{"Titanium Bar", 6}, {"Advanced Engine Parts", 3},
                             {"Crystal", 3}},
                           12.0, 18.0, "Shipyard" }},
    {"Preemptor Frigate", { {{"Titanium Bar", 5}, {"Advanced Engine Parts", 2},
                             {"Mithril Bar", 4}},
                           8.0, 20.0, "Shipyard" }},
    {"Protector Frigate", { {{"Titanium Bar", 5}, {"Advanced Engine Parts", 2},
                             {"Crystal", 3}},
                           10.0, 15.0, "Shipyard" }},
    {"Sunflare Sloop", { {{"Titanium Bar", 3}, {"Advanced Engine Parts", 1}},
                         5.0, 15.0, "Shipyard" }},
    {"Eclipse Monolith", { {{"Fusion Reactor", 1}, {"Titanium Bar", 15},
                            {"Advanced Engine Parts", 10}, {"Crystal", 5}},
                           30.0, 50.0, "Flagship Dock" }}
};

const std::map<std::string, BuildingRecipe> BUILDING_RECIPES = {
    {"Crafting Building", { {{"Iron Bar", 5}, {"Copper Bar", 2}}, 5.0, 10.0, 1 }},
    {"Smelting Building", { {{"Iron", 5}, {"Coal", 10}}, 5.0, 10.0, 1 }},
    {"Facility Workshop", { {{"Generator", 2}, {"Accumulator", 2}}, 3.0, 5.0, 1 }},
    {"Shipyard", { {{"Iron Bar", 10}, {"Engine Parts", 5}}, 8.0, 20.0, 1 }},
    {"Proximity Alarm", { {{"Crystal", 2}, {"Mithril Bar", 1}}, 3.0, 5.0, 1 }},
    {"Proximity Radar", { {{"Crystal", 2}, {"Mithril Bar", 1}}, 3.0, 5.0, 1 }},
    {"Mobile Radar", { {{"Crystal", 3}, {"Copper Bar", 2}}, 3.0, 5.0, 1 }},
    {"Salvage Robot", { {{"Obsidian", 2}, {"Engine Parts", 1}}, 3.0, 5.0, 1 }},
    {"Shield Generator", { {{"Titanium Bar", 2}, {"Copper Bar", 2}}, 4.0, 5.0, 1 }},
    {"Solar Panel", { {}, 3.0, 0.0, 1 }},
    {"Tritium Extractor", { {{"Mithril Bar", 2}, {"Advanced Engine Parts", 1}}, 4.0, 8.0, 1 }},
    {"Defense Turret", { {{"Iron Bar", 3}, {"Engine Parts", 1}}, 3.0, 5.0, 1 }},
    {"Flagship Dock", { {{"Iron Bar", 10}, {"Engine Parts", 5}, {"Titanium Bar", 5}}, 10.0, 30.0, 2 }},
    {"Helios Beacon", { {{"Crystal", 5}, {"Advanced Engine Parts", 2}}, 6.0, 10.0, 1 }}
};

