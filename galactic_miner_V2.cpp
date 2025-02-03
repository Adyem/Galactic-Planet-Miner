/*
 * GALACTIC MINERS: A COHESIVE STORY OUTLINE
 *
 * BACKSTORY:
 * In a distant era, humanity has spread across star systems, with Terra as its core.
 * Colonies on Mars, Zalthor, Vulcan, and Luna supply vital resources: iron, rare metals,
 * titanium, crystals, and obsidian. Amid these efforts, raiders led by Captain Blackthorne
 * and Navigator Zara threaten fragile peace, driven by their own quest for survival.
 *
 * MAIN CHARACTERS:
 * - Old Miner Joe:
 *   A hardworking veteran who mines tirelessly to cope with personal tragedy. His passion
 *   for refining mining techniques pushes resource extraction forward.
 *
 * - Professor Lumen:
 *   An academic who studies planetary anomalies, warning of cosmic dangers. Her research
 *   hints at looming threats requiring urgent technological innovation.
 *
 * - Farmer Daisy:
 *   A dedicated agriculturalist who underscores the vital link between farming and mining.
 *   She struggles to sustain colonies reliant on stable resource convoys.
 *
 * - Captain Blackthorne & Navigator Zara:
 *   Former citizens turned raider leaders. Betrayed and cast aside, they now fight for
 *   justice—or vengeance—using hidden enclaves to strike the colonies.
 *
 * QUEST SYSTEM OVERVIEW (Modified):
 * - Story Quests:
 *   Your journey unfolds through a series of story and daily quests.
 *
 * - Random Daily Tasks:
 *   Once the story arc is complete, random daily quests (resource collection or raider warnings)
 *   continue as before.
 */

#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include "include/json.hpp"
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cctype>

using json = nlohmann::json;
using namespace std;

/*
 * GALACTIC MINERS: A COHESIVE STORY OUTLINE
 *
 * BACKSTORY:
 * In a distant era, humanity has spread across star systems, with Terra as its core.
 * Colonies on Mars, Zalthor, Vulcan, and Luna supply vital resources: iron, rare metals,
 * titanium, crystals, and obsidian. Amid these efforts, raiders led by Captain Blackthorne
 * and Navigator Zara threaten fragile peace, driven by their own quest for survival.
 *
 * MAIN CHARACTERS:
 * - Old Miner Joe:
 *   A hardworking veteran who mines tirelessly to cope with personal tragedy. His passion
 *   for refining mining techniques pushes resource extraction forward.
 *
 * - Professor Lumen:
 *   An academic who studies planetary anomalies, warning of cosmic dangers. Her research
 *   hints at looming threats requiring urgent technological innovation.
 *
 * - Farmer Daisy:
 *   A dedicated agriculturalist who underscores the vital link between farming and mining.
 *   She struggles to sustain colonies reliant on stable resource convoys.
 *
 * - Captain Blackthorne & Navigator Zara:
 *   Former citizens turned raider leaders. Betrayed and cast aside, they now fight for
 *   justice—or vengeance—using hidden enclaves to strike the colonies.
 *
 * QUEST SYSTEM OVERVIEW (Modified):
 * - Story Quests:
 *   Your journey unfolds through a series of story and daily quests.
 *
 * - Random Daily Tasks:
 *   Once the story arc is complete, random daily quests (resource collection or raider warnings)
 *   continue as before.
 */

// Expanded lore arrays with more creative back‐stories
static const vector<string> resourceLore = {
    "Old Miner Joe: 'The veins of our beloved planet run deep—every ounce of ore fuels our future. I once lost everything to a mining accident, and now I seek redemption in every pick strike.'",
    "Professor Lumen: 'Mining is the backbone of our civilization. Behind every ore lies a story of sacrifice and hope, echoing the trials of those who came before us.'",
    "Farmer Daisy: 'Though I tend my fields, I know that without the mines, our tables would be empty. The earth gives generously, yet it also demands respect for its hidden treasures.'"
};

static const vector<string> raiderLore = {
    "Captain Blackthorne: 'I never chose this path. Forced into desperation by a system that failed us, we strike not out of malice, but as a cry for justice.'",
    "Navigator Zara: 'Every course we chart is a reminder of the choices we were left with. We become raiders because survival left us no other way, not because we relish conflict.'",
    "Old Scout Finn: 'In the void, I've seen the truth—our fury is the echo of neglect, a tragic response by souls abandoned by hope.'"
};

static const vector<string> genericLore = {
    "Town Crier: 'The times are hard, but unity and perseverance will see us through. Remember, every challenge is an opportunity in disguise.'",
    "Scribe Alric: 'Every day brings new challenges—and new opportunities—for those brave enough to seize them. History is written by those who dare to dream.'"
};

const string SAVE_FILE = "savegame.json";

struct ResourceDef {
    string name;
    int initial;
};

static vector<ResourceDef> RESOURCE_DATA = {
    {"Iron", 0}, {"Copper", 0}, {"Mithril", 0}, {"Coal", 0}, {"Tin", 0},
    {"Silver", 0}, {"Gold", 0}, {"Iron Bar", 0}, {"Copper Bar", 0}, {"Mithril Bar", 0},
    {"Engine Parts", 0}, {"Titanium", 0}, {"Titanium Bar", 0}, {"Generator", 0},
    {"Accumulator", 0}, {"Obsidian", 0}, {"Crystal", 0}, {"Nanomaterial", 0},
    {"Advanced Engine Parts", 0}, {"Fusion Reactor", 0}, {"Tritium", 0}
};

struct PlanetDef {
    string name;
    map<string, double> baseProduction;
    bool unlocked;
};

static vector<PlanetDef> PLANET_DATA = {
    {"Terra", {{"Iron", 0.5}, {"Copper", 0.5}, {"Coal", 0.2}}, true},
    {"Mars", {{"Iron", 0.1}, {"Copper", 0.1}, {"Mithril", 0.05}, {"Coal", 0.1}}, false},
    {"Zalthor", {{"Mithril", 0.1}, {"Coal", 0.2}, {"Gold", 0.02}}, false},
    {"Vulcan", {{"Tin", 0.1}, {"Silver", 0.03}, {"Titanium", 0.01}}, false},
    {"Luna", {{"Obsidian", 0.1}, {"Crystal", 0.05}, {"Nanomaterial", 0.02}}, false}
};

struct ResearchDef {
    string name;
    map<string, int> cost;
    string description;
    string effectName;
};

// Removed the old "Urban Planning" research and replaced it with three separate entries.
static vector<ResearchDef> RESEARCH_DATA = {
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
     "Unlock smaller versions of capital ships (frigates) that are less powerful but can be built without limits.", "unlock_capital_frigates"}
};

struct CraftingRecipe {
    map<string, int> inputs;
    double timeRequired;
    double electricityCost;
    string requiredBuilding;
};

// Note: Only capital ship recipes have been changed to require the "Flagship Dock" now.
static map<string, CraftingRecipe> CRAFTING_RECIPES = {
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
                           10.0, 15.0, "Shipyard" }}
};

struct BuildingRecipe {
    map<string, int> inputs;
    double timeRequired;
    double electricityCost;
    int plotCost; // New field: the number of building plots this structure uses.
};

static map<string, BuildingRecipe> BUILDING_RECIPES = {
    {"Crafting Building", { {{"Iron Bar", 5}, {"Copper Bar", 2}}, 5.0, 10.0, 1 }},
    {"Smelting Building", { {{"Iron", 5}, {"Coal", 10}}, 5.0, 10.0, 1 }}, // modified recipe
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
    {"Flagship Dock", { {{"Iron Bar", 10}, {"Engine Parts", 5}, {"Titanium Bar", 5}}, 10.0, 30.0, 2 }}
};

struct Ship {
    string type;
    int hull;
    int maxShield;
    int currentShield;
    int weapons;
    int repairAmount;
};

// Forward declaration for use in Planet
class PlanetManager;

//
// JOURNAL SYSTEM
//
struct JournalEntry {
    int id;
    string title;
    string text;
    bool unlocked;
};

class Journal {
public:
    Journal() : nextId(1) {}
    void addEntry(const string &title, const string &text) {
        JournalEntry entry;
        entry.id = nextId++;
        entry.title = title;
        entry.text = text;
        entry.unlocked = true;
        entries.push_back(entry);
        cout << "New journal entry unlocked: [" << entry.id << "] " << entry.title << endl;
    }
    void listEntries() {
        cout << "Journal Entries:" << endl;
        for (const auto &entry : entries) {
            if (entry.unlocked)
                cout << "[" << entry.id << "] " << entry.title << endl;
        }
    }
    void viewEntry(int id) {
        for (const auto &entry : entries) {
            if (entry.id == id && entry.unlocked) {
                cout << "Journal Entry [" << entry.id << "] " << entry.title << ":\n";
                cout << entry.text << "\n";
                return;
            }
        }
        cout << "Journal entry not found or locked." << endl;
    }
private:
    vector<JournalEntry> entries;
    int nextId;
};

Journal journal;

//
// PLANET CLASS
//
class Planet {
public:
    Planet(const string &name, const map<string, double> &production,
           bool unlocked)
        : name_(name), baseProduction_(production), unlocked_(unlocked),
          generators_(0), accumulators_(0), currentEnergy_(0.0), maxEnergy_(50.0),
          maxBuildingPlots_(8), currentBuildingCount_(0), plotsUpgraded_(false),
          underThreat_(false) {
        for (const auto &res : RESOURCE_DATA)
            storage_[res.name] = 0;
    }
    void produceResources(double elapsedSeconds, bool energyConservationEnabled) {
        if (underThreat_ && energyConservationEnabled &&
            hasBuilding("Shield Generator")) {
            cout << "Production on " << name_
                 << " halted due to Emergency Energy Conservation." << endl;
            return;
        }
        if (!unlocked_)
            return;
        double productionTime = elapsedSeconds;
        if (generators_ > 0) {
            double possibleTime = static_cast<double>(getStored("Coal")) /
                                  (generators_ * COAL_CONSUMPTION);
            productionTime = min(elapsedSeconds, possibleTime);
            int coalNeeded = static_cast<int>(ceil(generators_ * COAL_CONSUMPTION *
                                                   productionTime));
            removeFromStorage("Coal", coalNeeded);
        }
        double energyProduced = generators_ * GENERATOR_RATE * productionTime;
        currentEnergy_ = min(maxEnergy_, currentEnergy_ + energyProduced);
        if (hasBuilding("Solar Panel")) {
            int solarCount = getBuildings().at("Solar Panel");
            double solarEnergy = solarCount * SOLAR_RATE * elapsedSeconds;
            currentEnergy_ = min(maxEnergy_, currentEnergy_ + solarEnergy);
        }
        double effectiveSeconds = min(productionTime,
                                      currentEnergy_ / ENERGY_COST_PER_SECOND);
        double energyConsumed = effectiveSeconds * ENERGY_COST_PER_SECOND;
        currentEnergy_ -= energyConsumed;
        for (auto &entry : baseProduction_) {
            int produced = static_cast<int>(floor(entry.second * productionTime));
            storage_[entry.first] += produced;
        }
        if (hasBuilding("Tritium Extractor")) {
            int extractorCount = getBuildings().at("Tritium Extractor");
            int tritiumProduced = static_cast<int>(
                floor(extractorCount * TRITIUM_EXTRACTION_RATE * productionTime));
            storage_["Tritium"] += tritiumProduced;
            if (tritiumProduced > 0)
                cout << "Tritium Extractor on " << name_ << " produced "
                     << tritiumProduced << " Tritium." << endl;
        }
    }
    bool canBuildMore() const {
        return currentBuildingCount_ < maxBuildingPlots_;
    }
    bool upgradePlots() {
        if (!plotsUpgraded_) {
            plotsUpgraded_ = true;
            maxBuildingPlots_ += 2;
            return true;
        }
        return false;
    }
    void increaseMaxBuildingPlots(int amount) {
        maxBuildingPlots_ += amount;
    }
    // New method: add a building using a specific number of plots.
    bool addBuildingWithCost(const string &buildingName, int plotCost) {
        if (currentBuildingCount_ + plotCost > maxBuildingPlots_) {
            cout << "No building plots available on " << name_
                 << " for " << buildingName << "." << endl;
            return false;
        }
        buildings_[buildingName]++;
        currentBuildingCount_ += plotCost;
        return true;
    }
    bool addBuilding(const string &buildingName) {
        // Used for facilities which always cost 1 plot.
        if (!canBuildMore()) {
            cout << "No building plots available on " << name_
                 << ". Upgrade building capacity first." << endl;
            return false;
        }
        buildings_[buildingName]++;
        currentBuildingCount_++;
        return true;
    }
    bool hasBuilding(const string &buildingName) const {
        auto it = buildings_.find(buildingName);
        return (it != buildings_.end() && it->second > 0);
    }
    const map<string, int> &getBuildings() const {
        return buildings_;
    }
    void addToStorage(const string &resourceName, int amount) {
        storage_[resourceName] += amount;
    }
    bool removeFromStorage(const string &resourceName, int amount) {
        if (storage_[resourceName] >= amount) {
            storage_[resourceName] -= amount;
            return true;
        }
        return false;
    }
    int getStored(const string &resourceName) const {
        auto it = storage_.find(resourceName);
        return (it != storage_.end()) ? it->second : 0;
    }
    const map<string, int> &getStorageMap() const {
        return storage_;
    }
    const string &getName() const {
        return name_;
    }
    bool isUnlocked() const {
        return unlocked_;
    }
    void setUnlocked(bool unlocked) {
        unlocked_ = unlocked;
    }
    double getCurrentEnergy() const {
        return currentEnergy_;
    }
    double getMaxEnergy() const {
        return maxEnergy_;
    }
    int getGenerators() const {
        return generators_;
    }
    int getAccumulators() const {
        return accumulators_;
    }
    void setCurrentEnergy(double energy) {
        currentEnergy_ = energy;
    }
    void setMaxEnergy(double energy) {
        maxEnergy_ = energy;
    }
    void setGenerators(int generators) {
        generators_ = generators;
    }
    void setAccumulators(int accumulators) {
        accumulators_ = accumulators;
    }
    void installFacility(const string &facility) {
        string fac = facility;
        transform(fac.begin(), fac.end(), fac.begin(), ::tolower);
        if (fac == "generator") {
            if (!canBuildMore()) {
                cout << "No building plots available on " << name_
                     << " for Generator." << endl;
                return;
            }
            addBuilding("Generator");
            generators_++;
            cout << "Installed a Generator on " << name_
                 << ". Total: " << generators_ << endl;
        } else if (fac == "accumulator") {
            accumulators_++;
            maxEnergy_ += 50.0;
            cout << "Installed an Accumulator on " << name_
                 << ". Total: " << accumulators_
                 << ", new max energy: " << maxEnergy_ << endl;
        } else {
            cout << "Unknown facility: " << facility << endl;
        }
    }
    bool isUnderThreat() const {
        return underThreat_;
    }
    void setUnderThreat(bool flag) {
        underThreat_ = flag;
    }
    int getMaxBuildingPlots() const {
        return maxBuildingPlots_;
    }
    int getUsedBuildingPlots() const {
        return currentBuildingCount_;
    }
private:
    string name_;
    map<string, double> baseProduction_;
    bool unlocked_;
    int generators_;
    int accumulators_;
    double currentEnergy_;
    double maxEnergy_;
    map<string, int> storage_;
    map<string, int> buildings_;
    int maxBuildingPlots_;
    int currentBuildingCount_;
    bool plotsUpgraded_;
    bool underThreat_;
    static constexpr double GENERATOR_RATE = 5.0;
    static constexpr double ENERGY_COST_PER_SECOND = 1.0;
    static constexpr double COAL_CONSUMPTION = 0.5;
    static constexpr double SOLAR_RATE = 2.0;
    static constexpr double TRITIUM_EXTRACTION_RATE = 0.05;
};

//
// PLANET MANAGER
//
class PlanetManager {
public:
    PlanetManager(const vector<PlanetDef> &data) {
        for (const auto &planet : data)
            planets_.push_back(Planet(planet.name, planet.baseProduction,
                                      planet.unlocked));
    }
    void produceAll(double elapsedSeconds, bool energyConservationEnabled) {
        for (auto &planet : planets_)
            planet.produceResources(elapsedSeconds, energyConservationEnabled);
    }
    void unlockPlanet(const string &planetName) {
        for (auto &planet : planets_)
            if (planet.getName() == planetName)
                planet.setUnlocked(true);
    }
    Planet *getPlanetByName(const string &planetName) {
        for (auto &planet : planets_)
            if (planet.getName() == planetName)
                return &planet;
        return nullptr;
    }
    const vector<Planet> &getPlanetsConst() const {
        return planets_;
    }
    vector<Planet> &getPlanets() {
        return planets_;
    }
private:
    vector<Planet> planets_;
};

//
// RESEARCH
//
class Research {
public:
    Research(const string &name, const map<string, int> &cost,
             const string &description, const string &effectName)
        : name_(name), cost_(cost), description_(description),
          effectName_(effectName), completed_(false) {}
    bool canResearch(Planet *central) const {
        for (const auto &entry : cost_)
            if (central->getStored(entry.first) < entry.second)
                return false;
        return true;
    }
    void doResearch(Planet *central) {
        for (const auto &entry : cost_)
            central->removeFromStorage(entry.first, entry.second);
        completed_ = true;
    }
    const string &getName() const {
        return name_;
    }
    const string &getEffectName() const {
        return effectName_;
    }
    bool isCompleted() const {
        return completed_;
    }
    void setCompleted(bool val) {
        completed_ = val;
    }
    const map<string, int> &getCost() const {
        return cost_;
    }
    const string &getDescription() const {
        return description_;
    }
private:
    string name_;
    map<string, int> cost_;
    string description_;
    string effectName_;
    bool completed_;
};

//
// DAILY QUEST / STORY QUEST
//
class DailyQuest {
public:
    DailyQuest(const string &desc, const string &objectiveRes, int objectiveAmt,
               const map<string, int> &reward)
        : description_(desc), objectiveResource_(objectiveRes), objectiveAmount_(objectiveAmt), reward_(reward),
          completed_(false), isRaiderAttack_(false), targetPlanet_(""), combatStartTime_(0), turnsElapsed_(0),
          isStoryQuest_(false), isFinalConfrontation_(false) {}
    void checkCompletion(Planet *central) {
        if (completed_ || isRaiderAttack_)
            return;
        if (!objectiveResource_.empty() && central->getStored(objectiveResource_) >= objectiveAmount_) {
            completed_ = true;
            for (const auto &entry : reward_)
                central->addToStorage(entry.first, entry.second);
            cout << "Quest complete! Reward: ";
            for (const auto &entry : reward_)
                cout << entry.first << " +" << entry.second << " ";
            cout << endl;
            // For non-story quests, add a generic journal entry.
            if (!isStoryQuest_) {
                string journalText = "You successfully gathered " + to_string(objectiveAmount_) +
                                 " " + objectiveResource_ + ". The effort has not gone unnoticed.";
                journal.addEntry("Resource Quest Completed", journalText);
                cout << "\nLore: " << resourceLore[rand() % resourceLore.size()] << endl;
            }
        }
    }
    bool processRaiderAttack(PlanetManager &pm, vector<Ship> &fleet, bool realTime) {
        if (completed_ || !isRaiderAttack_)
            return false;
        Planet *target = pm.getPlanetByName(targetPlanet_);
        if (!target) {
            cout << "Target planet " << targetPlanet_ << " not found." << endl;
            return false;
        }
        if (combatStartTime_ == 0)
            combatStartTime_ = time(nullptr);
        // Adjust enemy stats if this is the final confrontation.
        int raiderShield = (isFinalConfrontation_) ? rand() % 101 + 200 : rand() % 101 + 100;
        int raiderHull = (isFinalConfrontation_) ? rand() % 201 + 600 : rand() % 201 + 300;
        cout << "Commencing raider battle at " << targetPlanet_ << "!" << endl;
        if (isFinalConfrontation_)
            cout << "Multiple enemy capital ships support the assault, their cannons roaring in unison!" << endl;
        const int maxTurns = 10;
        int turn = 0;
        while (turn < maxTurns && !fleet.empty() && raiderHull > 0) {
            turn++;
            cout << "\n--- Turn " << turn << " ---" << endl;
            if (realTime) {
                cout << "Waiting 60 seconds for next combat turn..." << endl;
                this_thread::sleep_for(chrono::seconds(60));
            }
            int totalDamage = 0;
            for (const auto &ship : fleet) {
                if (ship.type == "Corvette" || ship.type == "Shield Ship" ||
                    ship.type == "Radar Ship" || ship.type == "Salvage Ship")
                    totalDamage += ship.weapons;
            }
            cout << "Your defenders fire for a total of " << totalDamage
                 << " damage." << endl;
            int shieldDamage = min(totalDamage, raiderShield);
            raiderShield -= shieldDamage;
            int remainingDamage = totalDamage - shieldDamage;
            raiderHull -= remainingDamage;
            cout << "Raider shields take " << shieldDamage
                 << " damage, remaining: " << raiderShield << endl;
            if (remainingDamage > 0)
                cout << "Raider hull takes " << remainingDamage
                     << " damage, remaining: " << raiderHull << endl;
            if (raiderHull <= 0) {
                cout << "Raiders defeated!" << endl;
                Planet *terra = pm.getPlanetByName("Terra");
                if (terra) {
                    terra->addToStorage("Engine Parts", 2);
                    cout << "You receive 2 Engine Parts as reward." << endl;
                }
                completed_ = true;
                if (!isStoryQuest_) {
                    string journalText = "In a fierce battle, your fleet repelled the raider onslaught.";
                    journal.addEntry("Raider Attack Repelled", journalText);
                    cout << "\nLore: " << raiderLore[rand() % raiderLore.size()] << endl;
                }
                target->setUnderThreat(false);
                break;
            }
            int raiderDamage = rand() % 51 + 50;
            // If final confrontation, add extra damage to simulate capital ship firepower.
            if (isFinalConfrontation_)
                raiderDamage += 30;
            cout << "Raiders fire for " << raiderDamage << " damage." << endl;
            int shieldBonus = 0;
            int numShieldGen = 0;
            auto blds = target->getBuildings();
            if (blds.find("Shield Generator") != blds.end())
                numShieldGen = blds.at("Shield Generator");
            for (int i = 0; i < numShieldGen; i++) {
                if (target->getCurrentEnergy() >= 5) {
                    shieldBonus += 30;
                    target->setCurrentEnergy(target->getCurrentEnergy() - 5);
                }
            }
            cout << "Shield Generators provide " << shieldBonus
                 << " shield bonus." << endl;
            int effectiveDamage = max(0, raiderDamage - shieldBonus);
            vector<int> indices;
            for (size_t i = 0; i < fleet.size(); i++) {
                if (fleet[i].type == "Corvette" || fleet[i].type == "Shield Ship" ||
                    fleet[i].type == "Radar Ship" || fleet[i].type == "Salvage Ship")
                    indices.push_back(i);
            }
            if (!indices.empty()) {
                int dmgPerShip = effectiveDamage / indices.size();
                for (int idx : indices) {
                    Ship &s = fleet[idx];
                    if (s.currentShield >= dmgPerShip) {
                        s.currentShield -= dmgPerShip;
                        cout << s.type << " takes " << dmgPerShip
                             << " damage to shield, remaining shield: "
                             << s.currentShield << endl;
                    } else {
                        int rem = dmgPerShip - s.currentShield;
                        s.currentShield = 0;
                        s.hull -= rem;
                        cout << s.type << " shield depleted; takes " << rem
                             << " hull damage, remaining hull: " << s.hull << endl;
                    }
                }
                fleet.erase(remove_if(fleet.begin(), fleet.end(),
                                      [](const Ship &s) { return s.hull <= 0; }),
                            fleet.end());
            }
            if (fleet.empty()) {
                cout << "All defending ships have been destroyed!" << endl;
                completed_ = true;
                cout << "\nLore: " << raiderLore[rand() % raiderLore.size()] << endl;
                target->setUnderThreat(false);
                break;
            }
            vector<Ship *> repairDrones;
            for (auto &ship : fleet)
                if (ship.type == "Repair Drone")
                    repairDrones.push_back(&ship);
            if (!repairDrones.empty()) {
                vector<Ship *> candidates;
                for (auto &ship : fleet) {
                    if (ship.type != "Repair Drone" && ship.hull < 100)
                        candidates.push_back(&ship);
                }
                if (!candidates.empty()) {
                    Ship *targetShip = *min_element(
                        candidates.begin(), candidates.end(),
                        [](Ship *a, Ship *b) {
                            double ratioA = static_cast<double>(a->hull) /
                                            (a->type == "Shield Ship" ? 120 : 100);
                            double ratioB = static_cast<double>(b->hull) /
                                            (b->type == "Shield Ship" ? 120 : 100);
                            return ratioA < ratioB;
                        });
                    cout << "Repair Drones repair " << targetShip->type
                         << " for 10 HP." << endl;
                    targetShip->hull = min(targetShip->hull + 10,
                                           (targetShip->type == "Shield Ship" ? 120 : 100));
                }
            }
            cout << "End of turn " << turn << ".\n";
        }
        if (turn >= maxTurns) {
            cout << "Combat ended after 10 turns." << endl;
            completed_ = true;
            target->setUnderThreat(false);
        }
        return completed_;
    }
    bool isCompleted() const {
        return completed_;
    }
    const string &getDescription() const {
        return description_;
    }
    const string &getObjectiveResource() const {
        return objectiveResource_;
    }
    int getObjectiveAmount() const {
        return objectiveAmount_;
    }
    const map<string, int> &getReward() const {
        return reward_;
    }
    bool isRaider() const {
        return isRaiderAttack_;
    }
    void setRaiderAttack(const string &target) {
        isRaiderAttack_ = true;
        targetPlanet_ = target;
    }
    const string &getTargetPlanet() const {
        return targetPlanet_;
    }
    // New setters/getters for story quest flags:
    void setStoryQuest(bool val) { isStoryQuest_ = val; }
    bool isStoryQuest() const { return isStoryQuest_; }
    void setFinalConfrontation(bool val) { isFinalConfrontation_ = val; }
    bool isFinalConfrontation() const { return isFinalConfrontation_; }
private:
    string description_;
    string objectiveResource_;
    int objectiveAmount_;
    map<string, int> reward_;
    bool completed_;
    bool isRaiderAttack_;
    string targetPlanet_;
    time_t combatStartTime_;
    int turnsElapsed_;
    bool isStoryQuest_;
    bool isFinalConfrontation_;
};

//
// QUEST MANAGER
//
class QuestManager {
public:
    QuestManager() : currentQuest_(nullptr), lastQuestDate_(""), storyStage_(0) {}
    void updateDailyQuest(Planet *central, PlanetManager &pm, vector<Ship> &fleet) {
        string todayStr = currentDateString();
        if (lastQuestDate_.empty() || lastQuestDate_ < todayStr) {
            generateNewQuest(pm);
            lastQuestDate_ = todayStr;
        }
        if (currentQuest_) {
            bool realTime = true;
            if (currentQuest_->isRaider())
                currentQuest_->processRaiderAttack(pm, fleet, realTime);
            else
                currentQuest_->checkCompletion(central);
            if (currentQuest_->isCompleted() && currentQuest_->isStoryQuest()) {
                switch(storyStage_) {
                    case 0:
                        journal.addEntry("The Spark Ignited",
                            "Old Miner Joe, his voice roughened by years of hardship and loss, speaks with quiet resolve as you deliver 50 Iron Bars. 'Each bar is not merely forged from metal but from the very essence of our past struggles,' he murmurs. In that moment, his words resonate as both a personal confession and a rallying cry—a spark of hope amid the darkness of Terra's endless mines.");
                        break;
                    case 1:
                        journal.addEntry("A Cosmic Warning",
                            "Professor Lumen’s eyes glisten with a mix of academic curiosity and genuine concern. 'The cosmos itself trembles under the weight of disturbances near Mars,' she intones softly. Her measured tone—half observation, half lament—reveals a world where cosmic anomalies and human neglect converge, suggesting that the raider threat may stem not only from greed but also from desperation born of systemic failures.");
                        break;
                    case 2:
                        journal.addEntry("Bandit Outpost Assault",
                            "Farmer Daisy urges: 'Bandit outposts in Zalthor threaten our supplies. Lead an assault to disrupt their operations!'");
                        break;
                    case 3:
                        journal.addEntry("Shadows Over Vulcan",
                            "On the scorched plains of Vulcan, amidst the ruin and relentless heat, the truth begins to surface. Observers note that the raiders’ precision, often dismissed as sheer cruelty, may actually be fueled by a deep-seated sense of betrayal. It is as if every calculated strike carries with it an echo of past wounds—a grim reminder that even those deemed villains are sometimes the products of circumstance and neglect.");
                        break;
                    case 4:
                        journal.addEntry("Convoy Under Fire",
                            "In the chaos of a desperate defense on Terra, your eyes catch a fleeting glimpse of regret in a raider's gaze. As vital convoys are battered under heavy fire, there emerges an unsettling realization: behind every aggressive act, there may lie a silent lament—a wish for another way.");
                        break;
                    case 5:
                        journal.addEntry("Echoes of Betrayal",
                            "Professor Lumen reveals: 'A hidden raider outpost on Mars holds secrets of betrayal. Infiltrate and expose their past.'");
                        break;
                    case 6:
                        journal.addEntry("Broken Chains",
                            "Old Miner Joe challenges: 'Collect 30 Mithril Bars from Terra; let each bar be a link breaking the chains of oppression.'");
                        break;
                    case 7:
                        journal.addEntry("Siege of the Forgotten",
                            "Under Luna's eerie glow, your forces lay siege to a raider stronghold. The tactical brilliance of your adversaries is matched only by their tragic backstory—a tale of survival marred by neglect.");
                        break;
                    case 8:
                        journal.addEntry("Whispers in the Void",
                            "Intercepted transmissions reveal voices that are as harsh as they are sorrowful. These fragmented messages hint at a deep-seated wound among the raiders—a cry for understanding amidst the chaos of war.");
                        break;
                    case 9:
                        journal.addEntry("The Great Siege",
                            "On the ravaged battlegrounds of Mars, you lead an assault against a fortified raider outpost. Every counterstrike echoes with the weight of lost honor and the bitter cost of survival.");
                        break;
                    case 10:
                        journal.addEntry("Rising Tempest",
                            "As merchant guilds cry out for aid, you begin to notice an unsettling truth: every resource you gather not only bolsters your defenses but also fuels the raiders’ relentless evolution.");
                        break;
                    case 11:
                        journal.addEntry("The Final Stand",
                            "At last, you stand face-to-face with Captain Blackthorne in his hidden lair—a final confrontation where redemption and retribution blur into one.");
                        break;
                    default:
                        journal.addEntry("Unknown Stage", "The story continues beyond known bounds, each moment a testament to the complexity of survival and the enduring search for truth.");
                        break;
                }
                storyStage_++;
                currentQuest_.reset();
            }
        }
    }
    DailyQuest *getCurrentQuest() {
        return currentQuest_.get();
    }
    const DailyQuest *getCurrentQuestConst() const {
        return currentQuest_.get();
    }
    const string &getLastQuestDate() const {
        return lastQuestDate_;
    }
    void setLastQuestDate(const string &s) {
        lastQuestDate_ = s;
    }
    void setCurrentQuest(unique_ptr<DailyQuest> q) {
        currentQuest_ = std::move(q);
    }
private:
    unique_ptr<DailyQuest> currentQuest_;
    string lastQuestDate_;
    int storyStage_;
    // ----- EXPANDED DAILY QUEST SYSTEM -----
    void generateNewQuest(PlanetManager &pm) {
        if (storyStage_ < 12) {
            if (storyStage_ == 0) {
                // The Spark of Ambition
                string desc = "Old Miner Joe says: 'Deep in Terra's veins lie the secrets of redemption. Collect 50 Iron Bars to prove your resolve.'";
                map<string, int> reward = {{"Engine Parts", 3}};
                auto quest = make_unique<DailyQuest>(desc, "Iron Bar", 50, reward);
                quest->setStoryQuest(true);
                currentQuest_ = std::move(quest);
            } else if (storyStage_ == 1) {
                // A Cosmic Warning
                pm.unlockPlanet("Mars");
                string desc = "Professor Lumen warns: 'Cosmic anomalies stir near Mars. Prepare to defend against an impending raider attack!'";
                map<string, int> reward = {{"Engine Parts", 2}};
                auto quest = make_unique<DailyQuest>(desc, "", 1, reward);
                quest->setRaiderAttack("Mars");
                quest->setStoryQuest(true);
                currentQuest_ = std::move(quest);
            } else if (storyStage_ == 2) {
                // Bandit Outpost Assault
                pm.unlockPlanet("Zalthor");
                string desc = "Farmer Daisy urges: 'Bandit outposts in Zalthor threaten our supplies. Lead an assault to disrupt their operations!'";
                map<string, int> reward = {{"Engine Parts", 3}};
                auto quest = make_unique<DailyQuest>(desc, "", 1, reward);
                quest->setRaiderAttack("Zalthor");
                quest->setStoryQuest(true);
                currentQuest_ = std::move(quest);
            } else if (storyStage_ == 3) {
                // Shadows Over Vulcan
                pm.unlockPlanet("Vulcan");
                string desc = "A distress call from Vulcan: 'Raider forces have established a foothold. Strike at their encampments and reclaim our honor!'";
                map<string, int> reward = {{"Engine Parts", 3}};
                auto quest = make_unique<DailyQuest>(desc, "", 1, reward);
                quest->setRaiderAttack("Vulcan");
                quest->setStoryQuest(true);
                currentQuest_ = std::move(quest);
            } else if (storyStage_ == 4) {
                // Convoy Under Fire
                string desc = "Merchant voices cry out: 'Our convoys are ambushed! Secure the passage by defending the transport routes on Terra!'";
                map<string, int> reward = {{"Engine Parts", 2}};
                auto quest = make_unique<DailyQuest>(desc, "", 1, reward);
                quest->setRaiderAttack("Terra");
                quest->setStoryQuest(true);
                currentQuest_ = std::move(quest);
            } else if (storyStage_ == 5) {
                // Echoes of Betrayal
                string desc = "Professor Lumen reveals: 'A hidden raider outpost on Mars holds secrets of betrayal. Infiltrate and expose their past.'";
                map<string, int> reward = {{"Engine Parts", 3}};
                auto quest = make_unique<DailyQuest>(desc, "", 1, reward);
                quest->setRaiderAttack("Mars");
                quest->setStoryQuest(true);
                currentQuest_ = std::move(quest);
            } else if (storyStage_ == 6) {
                // Broken Chains
                string desc = "Old Miner Joe challenges: 'Collect 30 Mithril Bars from Terra; let each bar be a link breaking the chains of oppression.'";
                map<string, int> reward = {{"Engine Parts", 2}};
                auto quest = make_unique<DailyQuest>(desc, "Mithril Bar", 30, reward);
                quest->setStoryQuest(true);
                currentQuest_ = std::move(quest);
            } else if (storyStage_ == 7) {
                // Siege of the Forgotten
                pm.unlockPlanet("Luna");
                string desc = "A desperate plea echoes: 'A raider stronghold in Luna must fall. Lead the siege and reclaim what was lost.'";
                map<string, int> reward = {{"Engine Parts", 4}};
                auto quest = make_unique<DailyQuest>(desc, "", 1, reward);
                quest->setRaiderAttack("Luna");
                quest->setStoryQuest(true);
                currentQuest_ = std::move(quest);
            } else if (storyStage_ == 8) {
                // Whispers in the Void
                string desc = "An urgent message: 'Defend Terra from a sudden raider strike. The whispers of dissent hint at internal turmoil among the raiders.'";
                map<string, int> reward = {{"Engine Parts", 2}};
                auto quest = make_unique<DailyQuest>(desc, "", 1, reward);
                quest->setRaiderAttack("Terra");
                quest->setStoryQuest(true);
                currentQuest_ = std::move(quest);
            } else if (storyStage_ == 9) {
                // The Great Siege
                string desc = "A clarion call resounds: 'Assault the fortified raider outpost on Mars. Their defenses may be strong, but their resolve wavers.'";
                map<string, int> reward = {{"Engine Parts", 4}};
                auto quest = make_unique<DailyQuest>(desc, "", 1, reward);
                quest->setRaiderAttack("Mars");
                quest->setStoryQuest(true);
                currentQuest_ = std::move(quest);
            } else if (storyStage_ == 10) {
                // Rising Tempest
                string desc = "Merchant guilds request: 'Collect 200 Coal from Terra to fuel our defenses as tempests of war approach.'";
                map<string, int> reward = {{"Engine Parts", 3}};
                auto quest = make_unique<DailyQuest>(desc, "Coal", 200, reward);
                quest->setStoryQuest(true);
                currentQuest_ = std::move(quest);
            } else if (storyStage_ == 11) {
                // The Final Confrontation
                string desc = "The decisive moment: 'Blackthorne's hideout looms. Lead your fleet to its heart and end the cycle of despair once and for all!'";
                map<string, int> reward = {{"Engine Parts", 5}};
                auto quest = make_unique<DailyQuest>(desc, "", 1, reward);
                quest->setRaiderAttack("Terra");
                quest->setStoryQuest(true);
                quest->setFinalConfrontation(true);
                currentQuest_ = std::move(quest);
            }
        } else {
            // Fallback to random daily quests with extra variety.
            if ((rand() % 100) < 20) {
                vector<string> candidates;
                for (auto &planet : pm.getPlanets())
                    if (planet.isUnlocked() && planet.getName() != "Terra")
                        candidates.push_back(planet.getName());
                if (candidates.empty())
                    candidates.push_back("Terra");
                string target = candidates[rand() % candidates.size()];
                string desc = "Urgent alert: Raiders are approaching " + target + "! Prepare your defenses!";
                map<string, int> reward = {{"Engine Parts", 2}};
                auto quest = make_unique<DailyQuest>(desc, "", 1, reward);
                quest->setRaiderAttack(target);
                Planet *tgt = pm.getPlanetByName(target);
                if (tgt && tgt->hasBuilding("Proximity Alarm")) {
                    cout << "Proximity Alarm on " << target << " issues a 5-minute warning of an imminent raider attack!" << endl;
                    this_thread::sleep_for(chrono::minutes(5));
                    tgt->setUnderThreat(true);
                }
                journal.addEntry("Raider Warning", "Intelligence reports hint at an emerging threat near " + target + ". The raiders, though ruthless, bear the scars of neglect.");
                currentQuest_ = std::move(quest);
            } else {
                vector<string> resourceChoices = {"Iron Bar", "Copper Bar", "Mithril Bar",
                                                    "Titanium Bar", "Advanced Engine Parts"};
                int idx = rand() % resourceChoices.size();
                string chosen = resourceChoices[idx];
                int amt = rand() % 41 + 10;
                map<string, int> reward = {{"Engine Parts", (rand() % 3) + 1}};
                string desc = "Side quest: Collect " + to_string(amt) + " " + chosen;
                currentQuest_ = make_unique<DailyQuest>(desc, chosen, amt, reward);
            }
        }
    }
    string currentDateString() {
        time_t now = time(nullptr);
        tm *gmt = gmtime(&now);
        char buf[11];
        strftime(buf, sizeof(buf), "%Y-%m-%d", gmt);
        return string(buf);
    }
};

//
// RESEARCH MANAGER
//
class ResearchManager {
public:
    ResearchManager(const vector<ResearchDef> &data) {
        for (const auto &rd : data)
            researches_.push_back(Research(rd.name, rd.cost, rd.description,
                                           rd.effectName));
    }
    Research *findResearchByName(const string &name) {
        for (auto &research : researches_)
            if (research.getName() == name)
                return &research;
        return nullptr;
    }
    vector<Research> &getAllResearches() {
        return researches_;
    }
    const vector<Research> &getAllResearchesConst() const {
        return researches_;
    }
private:
    vector<Research> researches_;
};

//
// CRAFTING MANAGER
//
class CraftingManager {
public:
    CraftingManager(const map<string, CraftingRecipe> &recipes)
        : recipes_(recipes) {}
    void craft(const string &itemName, Planet *planet, double fasterMultiplier,
               double costMultiplier, bool precisionToolsEnabled) {
        auto it = recipes_.find(itemName);
        if (it == recipes_.end()) {
            cout << "No recipe for item '" << itemName << "'." << endl;
            return;
        }
        CraftingRecipe recipe = it->second;
        if (!planet->hasBuilding(recipe.requiredBuilding)) {
            cout << "You need a " << recipe.requiredBuilding << " on "
                 << planet->getName() << " to craft " << itemName << "." << endl;
            return;
        }
        for (const auto &entry : recipe.inputs) {
            if (planet->getStored(entry.first) < entry.second) {
                cout << "Not enough " << entry.first << " on "
                     << planet->getName() << " to craft " << itemName << "."
                     << endl;
                return;
            }
        }
        double effectiveCost = recipe.electricityCost * costMultiplier;
        if (planet->getCurrentEnergy() < effectiveCost) {
            cout << "Not enough energy on " << planet->getName() << " to craft "
                 << itemName << "." << endl;
            return;
        }
        for (const auto &entry : recipe.inputs)
            planet->removeFromStorage(entry.first, entry.second);
        planet->setCurrentEnergy(planet->getCurrentEnergy() - effectiveCost);
        double craftTime = recipe.timeRequired * fasterMultiplier;
        cout << "Crafting " << itemName << " took " << craftTime << " seconds." << endl;
        int quantity = 1;
        if (precisionToolsEnabled) {
            if (rand() % 100 < 10) {
                quantity = 2;
                cout << "Precision Tools activated: doubled output!" << endl;
            }
        }
        planet->addToStorage(itemName, quantity);
        cout << "Crafted " << quantity << " " << itemName << "(s) on "
             << planet->getName() << "." << endl;
    }
private:
    map<string, CraftingRecipe> recipes_;
};

//
// PLAYER
//
class Player {
public:
    Player()
        : planetManager_(PLANET_DATA),
          researchManager_(RESEARCH_DATA),
          craftingManager_(CRAFTING_RECIPES),
          fasterCraftingMultiplier_(1.0),
          craftingCostMultiplier_(1.0),
          precisionToolsEnabled_(false),
          energyConservationEnabled_(false),
          quantumCommunicationEnabled_(false),
          lastUpdateTime_(time(nullptr)) {
        srand(static_cast<unsigned>(time(nullptr)));
    }
    PlanetManager &getPlanetManager() {
        return planetManager_;
    }
    ResearchManager &getResearchManager() {
        return researchManager_;
    }
    CraftingManager &getCraftingManager() {
        return craftingManager_;
    }
    QuestManager &getQuestManager() {
        return questManager_;
    }
    vector<Ship> &getFleet() {
        return fleet_;
    }
    const PlanetManager &getPlanetManager() const {
        return planetManager_;
    }
    const ResearchManager &getResearchManager() const {
        return researchManager_;
    }
    const QuestManager &getQuestManager() const {
        return questManager_;
    }
    const vector<Ship> &getFleet() const {
        return fleet_;
    }
    double getFasterCraftingMultiplier() const {
        return fasterCraftingMultiplier_;
    }
    double getCraftingCostMultiplier() const {
        return craftingCostMultiplier_;
    }
    bool isPrecisionToolsEnabled() const {
        return precisionToolsEnabled_;
    }
    bool isEnergyConservationEnabled() const {
        return energyConservationEnabled_;
    }
    bool isQuantumCommunicationEnabled() const {
        return quantumCommunicationEnabled_;
    }
    void setFasterCraftingMultiplier(double val) {
        fasterCraftingMultiplier_ = val;
    }
    void setCraftingCostMultiplier(double val) {
        craftingCostMultiplier_ = val;
    }
    void setPrecisionToolsEnabled(bool val) {
        precisionToolsEnabled_ = val;
    }
    void setEnergyConservationEnabled(bool val) {
        energyConservationEnabled_ = val;
    }
    void setQuantumCommunicationEnabled(bool val) {
        quantumCommunicationEnabled_ = val;
    }
    void produceResources() {
        time_t now = time(nullptr);
        double diff = difftime(now, lastUpdateTime_);
        if (diff < 0)
            diff = 0;
        planetManager_.produceAll(diff, energyConservationEnabled_);
        lastUpdateTime_ = now;
    }
    void doResearch(const string &researchName) {
        Research *research = researchManager_.findResearchByName(researchName);
        if (!research) {
            cout << "No research found by that name." << endl;
            return;
        }
        if (research->isCompleted()) {
            cout << "Research already completed." << endl;
            return;
        }
        Planet *terra = planetManager_.getPlanetByName("Terra");
        if (!terra) {
            cout << "Central planet Terra not found." << endl;
            return;
        }
        if (research->canResearch(terra)) {
            research->doResearch(terra);
            applyResearchEffect(research->getEffectName());
            cout << "Research '" << research->getName() << "' completed!" << endl;
        } else {
            cout << "Not enough resources on Terra to perform research." << endl;
        }
    }
    void craftItem(const string &itemName, const string &planetName = "Terra") {
        Planet *planet = planetManager_.getPlanetByName(planetName);
        if (!planet) {
            cout << "Planet " << planetName << " not found." << endl;
            return;
        }
        craftingManager_.craft(itemName, planet, fasterCraftingMultiplier_,
                               craftingCostMultiplier_, precisionToolsEnabled_);
    }
    void craftShip(const string &shipType, const string &planetName = "Terra") {
        Planet *planet = planetManager_.getPlanetByName(planetName);
        if (!planet) {
            cout << "Planet " << planetName << " not found." << endl;
            return;
        }
        if (shipType == "Celestial Juggernaut" ||
            shipType == "Nova Carrier" || shipType == "Obsidian Sovereign" ||
            shipType == "Preemptor" || shipType == "Aurora Protector") {
            for (const auto &ship : fleet_) {
                if (ship.type == "Celestial Juggernaut" ||
                    ship.type == "Nova Carrier" || ship.type == "Obsidian Sovereign" ||
                    ship.type == "Preemptor" || ship.type == "Aurora Protector") {
                    cout << "A capital ship is already active. Only one capital ship can "
                         << "be active at a time." << endl;
                    return;
                }
            }
        }
        auto it = CRAFTING_RECIPES.find(shipType);
        if (it == CRAFTING_RECIPES.end()) {
            cout << "No recipe for ship type '" << shipType << "'." << endl;
            return;
        }
        if (!planet->hasBuilding(it->second.requiredBuilding)) {
            cout << "You need a " << it->second.requiredBuilding << " on " << planetName
                 << " to build a " << shipType << "." << endl;
            return;
        }
        for (const auto &entry : it->second.inputs) {
            if (planet->getStored(entry.first) < entry.second) {
                cout << "Not enough " << entry.first << " on " << planetName
                     << " to build a " << shipType << "." << endl;
                return;
            }
        }
        double effectiveCost = it->second.electricityCost * craftingCostMultiplier_;
        if (planet->getCurrentEnergy() < effectiveCost) {
            cout << "Not enough energy on " << planetName << " to build a " << shipType << "." << endl;
            return;
        }
        planet->setCurrentEnergy(planet->getCurrentEnergy() - effectiveCost);
        for (const auto &entry : it->second.inputs)
            planet->removeFromStorage(entry.first, entry.second);
        cout << "Built a " << shipType << " on " << planetName << " consuming "
             << effectiveCost << " energy." << endl;
        Ship newShip;
        newShip.type = shipType;
        if (shipType == "Transport Vessel") {
            newShip.hull = 100;
            newShip.maxShield = 50;
            newShip.currentShield = 50;
            newShip.weapons = 0;
            newShip.repairAmount = 0;
        } else if (shipType == "Corvette") {
            newShip.hull = 100;
            newShip.maxShield = 75;
            newShip.currentShield = 75;
            newShip.weapons = 30;
            newShip.repairAmount = 0;
        } else if (shipType == "Shield Ship") {
            newShip.hull = 120;
            newShip.maxShield = 150;
            newShip.currentShield = 150;
            newShip.weapons = 20;
            newShip.repairAmount = 0;
        } else if (shipType == "Radar Ship") {
            newShip.hull = 110;
            newShip.maxShield = 80;
            newShip.currentShield = 80;
            newShip.weapons = 25;
            newShip.repairAmount = 0;
        } else if (shipType == "Salvage Ship") {
            newShip.hull = 110;
            newShip.maxShield = 70;
            newShip.currentShield = 70;
            newShip.weapons = 15;
            newShip.repairAmount = 0;
        } else if (shipType == "Repair Drone") {
            newShip.hull = 80;
            newShip.maxShield = 40;
            newShip.currentShield = 40;
            newShip.weapons = 0;
            newShip.repairAmount = 10;
        } else if (shipType == "Interceptor") {
            newShip.hull = 90;
            newShip.maxShield = 60;
            newShip.currentShield = 60;
            newShip.weapons = 40;
            newShip.repairAmount = 0;
        } else if (shipType == "Celestial Juggernaut") {
            newShip.hull = 300;
            newShip.maxShield = 200;
            newShip.currentShield = 200;
            newShip.weapons = 100;
            newShip.repairAmount = 0;
        } else if (shipType == "Nova Carrier") {
            newShip.hull = 250;
            newShip.maxShield = 150;
            newShip.currentShield = 150;
            newShip.weapons = 80;
            newShip.repairAmount = 0;
        } else if (shipType == "Obsidian Sovereign") {
            newShip.hull = 350;
            newShip.maxShield = 250;
            newShip.currentShield = 250;
            newShip.weapons = 120;
            newShip.repairAmount = 0;
        } else if (shipType == "Preemptor") {
            newShip.hull = 200;
            newShip.maxShield = 100;
            newShip.currentShield = 100;
            newShip.weapons = 150;
            newShip.repairAmount = 0;
        } else if (shipType == "Aurora Protector") {
            newShip.hull = 280;
            newShip.maxShield = 300;
            newShip.currentShield = 300;
            newShip.weapons = 70;
            newShip.repairAmount = 0;
        } else if (shipType == "Juggernaut Frigate") {
            newShip.hull = 150;
            newShip.maxShield = 100;
            newShip.currentShield = 100;
            newShip.weapons = 40;
            newShip.repairAmount = 0;
        } else if (shipType == "Carrier Frigate") {
            newShip.hull = 130;
            newShip.maxShield = 90;
            newShip.currentShield = 90;
            newShip.weapons = 30;
            newShip.repairAmount = 0;
        } else if (shipType == "Sovereign Frigate") {
            newShip.hull = 160;
            newShip.maxShield = 110;
            newShip.currentShield = 110;
            newShip.weapons = 50;
            newShip.repairAmount = 0;
        } else if (shipType == "Preemptor Frigate") {
            newShip.hull = 120;
            newShip.maxShield = 80;
            newShip.currentShield = 80;
            newShip.weapons = 60;
            newShip.repairAmount = 0;
        } else if (shipType == "Protector Frigate") {
            newShip.hull = 140;
            newShip.maxShield = 120;
            newShip.currentShield = 120;
            newShip.weapons = 35;
            newShip.repairAmount = 0;
        }
        fleet_.push_back(newShip);
    }
    void buildBuilding(const string &buildingName, const string &planetName) {
        Planet *planet = planetManager_.getPlanetByName(planetName);
        if (!planet) {
            cout << "Planet " << planetName << " not found." << endl;
            return;
        }
        auto it = BUILDING_RECIPES.find(buildingName);
        if (it == BUILDING_RECIPES.end()) {
            cout << "No recipe for building '" << buildingName << "'." << endl;
            return;
        }
        int plotCost = it->second.plotCost;
        if (planet->getUsedBuildingPlots() + plotCost > planet->getMaxBuildingPlots()) {
            cout << "Not enough building plots on " << planetName << " to build " << buildingName << "." << endl;
            return;
        }
        for (const auto &entry : it->second.inputs) {
            if (planet->getStored(entry.first) < entry.second) {
                cout << "Not enough " << entry.first << " on " << planetName
                     << " to build " << buildingName << "." << endl;
                return;
            }
        }
        if (planet->getCurrentEnergy() < it->second.electricityCost) {
            cout << "Not enough energy on " << planetName << " to build " << buildingName << "." << endl;
            return;
        }
        planet->setCurrentEnergy(planet->getCurrentEnergy() - it->second.electricityCost);
        for (const auto &entry : it->second.inputs)
            planet->removeFromStorage(entry.first, entry.second);
        if (planet->addBuildingWithCost(buildingName, plotCost))
            cout << "Built " << buildingName << " on " << planetName << "." << endl;
    }
    void upgradeBuildingPlots(const string &planetName) {
        Planet *planet = planetManager_.getPlanetByName(planetName);
        if (!planet) {
            cout << "Planet " << planetName << " not found." << endl;
            return;
        }
        if (planet->getUsedBuildingPlots() < planet->getMaxBuildingPlots()) {
            cout << "There are still available building plots on " << planetName << "." << endl;
            return;
        }
        if (planet->getStored("Iron Bar") >= 10 &&
            planet->getStored("Engine Parts") >= 2) {
            if (planet->upgradePlots()) {
                planet->removeFromStorage("Iron Bar", 10);
                planet->removeFromStorage("Engine Parts", 2);
                cout << "Building capacity on " << planetName << " increased by 2." << endl;
            } else {
                cout << "Building capacity on " << planetName << " has already been upgraded." << endl;
            }
        } else {
            cout << "Not enough resources on " << planetName << " to upgrade building plots." << endl;
        }
    }
    void transferResource(const string &resourceName, int amount,
                          const string &fromPlanetName,
                          const string &toPlanetName) {
        Planet *fromPlanet = planetManager_.getPlanetByName(fromPlanetName);
        Planet *toPlanet = planetManager_.getPlanetByName(toPlanetName);
        if (!fromPlanet || !toPlanet) {
            cout << "Invalid planet(s) specified." << endl;
            return;
        }
        if (fromPlanet->getStored(resourceName) < amount) {
            cout << "Not enough " << resourceName << " on " << fromPlanetName
                 << " to transfer." << endl;
            return;
        }
        bool hasTransport = false;
        for (const auto &ship : fleet_)
            if (ship.type == "Transport Vessel") {
                hasTransport = true;
                break;
            }
        if (!hasTransport) {
            cout << "At least one Transport Vessel is required in your fleet." << endl;
            return;
        }
        cout << "Transferring " << amount << " " << resourceName << " from "
             << fromPlanetName << " to " << toPlanetName << "." << endl;
        int attackChance = quantumCommunicationEnabled_ ? 15 : 30;
        if ((rand() % 100) < attackChance) {
            cout << "Raider attack on the convoy!" << endl;
            int convoyStrength = 0, bonus = 0;
            for (auto it = fleet_.begin(); it != fleet_.end();) {
                if (it->type == "Transport Vessel") {
                    convoyStrength += it->currentShield;
                    it = fleet_.erase(it);
                    break;
                } else {
                    ++it;
                }
            }
            for (const auto &ship : fleet_)
                if (ship.type == "Corvette" || ship.type == "Shield Ship" ||
                    ship.type == "Radar Ship" || ship.type == "Salvage Ship")
                    bonus += ship.weapons;
            bool hasInterceptor = false;
            for (const auto &ship : fleet_)
                if (ship.type == "Interceptor") {
                    hasInterceptor = true;
                    break;
                }
            int raiderStrength = rand() % 101 + 20;
            if (hasInterceptor) {
                cout << "Interceptor in convoy reduces raider strength by 20!" << endl;
                raiderStrength = max(0, raiderStrength - 20);
            }
            cout << "Convoy strength: " << (convoyStrength + bonus)
                 << ", Raider strength: " << raiderStrength << endl;
            if ((convoyStrength + bonus) >= raiderStrength) {
                cout << "Convoy fended off the raiders!" << endl;
                fromPlanet->removeFromStorage(resourceName, amount);
                toPlanet->addToStorage(resourceName, amount);
                bool hasSalvageShip = false;
                for (const auto &ship : fleet_)
                    if (ship.type == "Salvage Ship") {
                        hasSalvageShip = true;
                        break;
                    }
                if (hasSalvageShip) {
                    int extra = (rand() % 6) + 5;
                    fromPlanet->addToStorage("Iron", extra);
                    cout << "Your Salvage Ship recovered an extra " << extra
                         << " Iron from the wreckage." << endl;
                }
            } else {
                cout << "Convoy was destroyed! Resources lost." << endl;
                if (fromPlanet->hasBuilding("Salvage Robot")) {
                    int salvage = (rand() % 11) + 5;
                    fromPlanet->addToStorage("Iron", salvage);
                    cout << "Salvage Robot recovered " << salvage << " Iron." << endl;
                }
            }
        } else {
            fromPlanet->removeFromStorage(resourceName, amount);
            toPlanet->addToStorage(resourceName, amount);
            cout << "Transfer successful." << endl;
        }
    }
    void showRadar(const string &planetName) {
        Planet *planet = planetManager_.getPlanetByName(planetName);
        if (!planet) {
            cout << "Planet " << planetName << " not found." << endl;
            return;
        }
        if (planet->hasBuilding("Radar"))
            cout << "Radar on " << planetName
                 << " indicates HIGH raider activity." << endl;
        else
            cout << "No Radar installed on " << planetName
                 << ". Raider activity unknown." << endl;
    }
    void updateDailyQuest() {
        Planet *terra = planetManager_.getPlanetByName("Terra");
        if (!terra)
            return;
        questManager_.updateDailyQuest(terra, planetManager_, fleet_);
    }
    void recalcUpgradesFromResearch() {
        fasterCraftingMultiplier_ = 1.0;
        craftingCostMultiplier_ = 1.0;
        precisionToolsEnabled_ = false;
        energyConservationEnabled_ = false;
        quantumCommunicationEnabled_ = false;
        for (const auto &research : researchManager_.getAllResearches()) {
            if (research.isCompleted()) {
                string effect = research.getEffectName();
                if (effect == "faster_crafting")
                    fasterCraftingMultiplier_ = 0.5;
                else if (effect == "crafting_mastery")
                    craftingCostMultiplier_ = 0.8;
                else if (effect == "precision_tools")
                    precisionToolsEnabled_ = true;
                else if (effect == "energy_conservation")
                    energyConservationEnabled_ = true;
                else if (effect == "quantum_communication")
                    quantumCommunicationEnabled_ = true;
            }
        }
    }
    void showFleet() {
        if (fleet_.empty()) {
            cout << "Your fleet is empty." << endl;
            return;
        }
        cout << "Your fleet:" << endl;
        for (size_t i = 0; i < fleet_.size(); i++) {
            cout << i + 1 << ". " << fleet_[i].type << " (Hull: " 
                 << fleet_[i].hull << ", Shield: " << fleet_[i].currentShield
                 << "/" << fleet_[i].maxShield;
            if (fleet_[i].type == "Corvette" ||
                fleet_[i].type == "Shield Ship" ||
                fleet_[i].type == "Radar Ship" ||
                fleet_[i].type == "Salvage Ship")
                cout << ", Weapons: " << fleet_[i].weapons;
            if (fleet_[i].type == "Repair Drone")
                cout << ", Repairs: " << fleet_[i].repairAmount << " HP/turn";
            cout << ")" << endl;
        }
    }
    time_t getLastUpdateTime() const {
        return lastUpdateTime_;
    }
    void setLastUpdateTime(time_t t) {
        lastUpdateTime_ = t;
    }
private:
    PlanetManager planetManager_;
    ResearchManager researchManager_;
    CraftingManager craftingManager_;
    QuestManager questManager_;
    vector<Ship> fleet_;
    double fasterCraftingMultiplier_;
    double craftingCostMultiplier_;
    bool precisionToolsEnabled_;
    bool energyConservationEnabled_;
    bool quantumCommunicationEnabled_;
    time_t lastUpdateTime_;
    void applyResearchEffect(const string &effectName) {
        if (effectName == "unlock_mars") {
            planetManager_.unlockPlanet("Mars");
            cout << "Mars unlocked!" << endl;
            journal.addEntry("Mars Unlocked", "The barren red plains of Mars are now open for resource extraction. Rumors of ancient relics stir excitement among pioneers.");
        } else if (effectName == "unlock_zalthor") {
            planetManager_.unlockPlanet("Zalthor");
            cout << "Zalthor unlocked!" << endl;
            journal.addEntry("Zalthor Revealed", "Zalthor's mysterious landscapes hold both peril and promise. Ancient secrets beckon those brave enough to mine its depths.");
        } else if (effectName == "faster_crafting") {
            fasterCraftingMultiplier_ = 0.5;
            cout << "Faster crafting unlocked!" << endl;
            journal.addEntry("Crafting Breakthrough", "New techniques have halved crafting times. Efficiency in production may tip the scales of future conflicts.");
        } else if (effectName == "unlock_vulcan") {
            planetManager_.unlockPlanet("Vulcan");
            cout << "Vulcan unlocked!" << endl;
            journal.addEntry("Vulcan Emerges", "Vulcan's volatile environment now offers rare resources. Its fiery heart burns with both danger and opportunity.");
        } else if (effectName == "unlock_luna") {
            planetManager_.unlockPlanet("Luna");
            cout << "Luna unlocked!" << endl;
            journal.addEntry("Luna Unbound", "Luna's icy plains hold untapped potential. The balance between light and shadow has shifted in its favor.");
        } else if (effectName == "crafting_mastery") {
            craftingCostMultiplier_ = 0.8;
            cout << "Crafting mastery achieved!" << endl;
            journal.addEntry("Crafting Mastery", "Skilled hands have refined the art of crafting, reducing costs and waste. The line between art and science blurs.");
        } else if (effectName == "precision_tools") {
            precisionToolsEnabled_ = true;
            cout << "Precision tools enabled!" << endl;
            journal.addEntry("Precision Engineering", "Advanced tools increase your chance to double production. Every measurement, every cut, resonates with perfection.");
        } else if (effectName == "energy_conservation") {
            energyConservationEnabled_ = true;
            cout << "Emergency Energy Conservation active!" << endl;
            journal.addEntry("Energy Conservation", "New protocols allow for rapid energy shutdown during crises, fortifying defenses in the face of raider aggression.");
        } else if (effectName == "unlock_solar_panels") {
            cout << "Solar Panels unlocked! You can now build Solar Panels." << endl;
            journal.addEntry("Solar Revolution", "Harnessing the power of the sun, new solar panels provide an abundant energy source, fueling the expansion of your empire.");
        } else if (effectName == "unlock_capital_ships") {
            cout << "Capital Ship Initiative research completed! You can now build one powerful capital ship at a time." << endl;
            journal.addEntry("Capital Ship Initiative", "A colossal leap in naval engineering, capital ships now stand as behemoths of war, their presence a deterrent to would-be aggressors.");
        } else if (effectName == "unlock_capital_frigates") {
            cout << "Auxiliary Frigate Development research completed! You can now build smaller capital ship variants without limits." << endl;
            journal.addEntry("Frigate Development", "Streamlined designs allow for the rapid deployment of frigates, versatile vessels that combine speed and firepower.");
        } else if (effectName == "urban_planning_terra") {
            Planet *terra = planetManager_.getPlanetByName("Terra");
            if (terra) {
                 terra->increaseMaxBuildingPlots(4);
                 cout << "Terra's building capacity increased by 4." << endl;
                 journal.addEntry("Urban Planning in Terra", "New infrastructure plans have been approved for Terra, allowing for an expansion of facilities. The promise of progress energizes the citizens.");
            }
        } else if (effectName == "urban_planning_mars") {
            Planet *mars = planetManager_.getPlanetByName("Mars");
            if (mars) {
                 mars->increaseMaxBuildingPlots(4);
                 cout << "Mars' building capacity increased by 4." << endl;
                 journal.addEntry("Urban Planning in Mars", "Mars is not just a barren wasteland; new urban planning initiatives promise to transform its rugged terrain into thriving outposts.");
            }
        } else if (effectName == "urban_planning_zalthor") {
            Planet *zalthor = planetManager_.getPlanetByName("Zalthor");
            if (zalthor) {
                 zalthor->increaseMaxBuildingPlots(4);
                 cout << "Zalthor's building capacity increased by 4." << endl;
                 journal.addEntry("Urban Planning in Zalthor", "Zalthor's enigmatic landscapes are set to be reshaped by ambitious urban planning projects, blending innovation with the mysteries of the past.");
            }
        } else {
            cout << "Unknown research effect: " << effectName << endl;
        }
    }
};

//
// SAVE / LOAD
//
void saveGame(const Player &player) {
    json j;
    {
        json planetsJson = json::array();
        const auto &planets = player.getPlanetManager().getPlanetsConst();
        for (const auto &planet : planets) {
            json planetObj;
            planetObj["name"] = planet.getName();
            planetObj["unlocked"] = planet.isUnlocked();
            planetObj["current_energy"] = planet.getCurrentEnergy();
            planetObj["max_energy"] = planet.getMaxEnergy();
            planetsJson.push_back(planetObj);
        }
        j["planets"] = planetsJson;
    }
    {
        json researchJson = json::array();
        for (const auto &research : player.getResearchManager().getAllResearchesConst()) {
            json researchObj;
            researchObj["name"] = research.getName();
            researchObj["completed"] = research.isCompleted();
            researchJson.push_back(researchObj);
        }
        j["researches"] = researchJson;
    }
    j["faster_crafting_multiplier"] = player.getFasterCraftingMultiplier();
    j["crafting_cost_multiplier"] = player.getCraftingCostMultiplier();
    j["precision_tools_enabled"] = player.isPrecisionToolsEnabled();
    {
        json questJson = nullptr;
        const DailyQuest *dq = player.getQuestManager().getCurrentQuestConst();
        if (dq) {
            questJson = json::object();
            questJson["description"] = dq->getDescription();
            questJson["objective_resource"] = dq->getObjectiveResource();
            questJson["objective_amount"] = dq->getObjectiveAmount();
            json rewardJson = json::object();
            for (const auto &entry : dq->getReward())
                rewardJson[entry.first] = entry.second;
            questJson["reward"] = rewardJson;
            questJson["completed"] = dq->isCompleted();
            if (dq->isRaider())
                questJson["target_planet"] = dq->getTargetPlanet();
        }
        j["current_quest"] = questJson;
        j["last_quest_date"] = player.getQuestManager().getLastQuestDate();
    }
    j["last_update_time"] = static_cast<long long>(player.getLastUpdateTime());
    {
        json fleetJson = json::array();
        for (const auto &ship : player.getFleet()) {
            json shipJson;
            shipJson["type"] = ship.type;
            shipJson["hull"] = ship.hull;
            shipJson["max_shield"] = ship.maxShield;
            shipJson["current_shield"] = ship.currentShield;
            shipJson["weapons"] = ship.weapons;
            fleetJson.push_back(shipJson);
        }
        j["fleet"] = fleetJson;
    }
    ofstream ofs(SAVE_FILE, ios::out | ios::trunc);
    if (!ofs.is_open()) {
        cerr << "Failed to open save file for writing." << endl;
        return;
    }
    ofs << j.dump(2);
    ofs.close();
    cout << "Game saved." << endl;
}

void loadGame(Player &player) {
    ifstream ifs(SAVE_FILE);
    if (!ifs.is_open())
        return;
    json j;
    ifs >> j;
    ifs.close();
    player.recalcUpgradesFromResearch();
    cout << "Game loaded from " << SAVE_FILE << "." << endl;
}

//
// CHARACTERS / LORE
//
void talkToCharacters() {
    cout << "\nLore: " << genericLore[rand() % genericLore.size()] << "\n";
}

void showHelp() {
    cout << "Commands:\n"
         << "  help                           - Show this help\n"
         << "  stats                          - Show resource storage on each planet\n"
         << "  planets                        - Show planet status (energy, building plots, and if locked, the research requirement)\n"
         << "  fleet                          - Show your fleet\n"
         << "  research                       - List research options (with resource costs)\n"
         << "  do_research <name>             - Perform research (using Terra's storage)\n"
         << "  craft [<item> [on <planet>]]     - Craft an item. If no arguments are given, you will be prompted interactively.\n"
         << "  build <building> on <planet>   - Build a building on a planet\n"
         << "  upgrade_plots <planet>         - Upgrade building capacity on a planet\n"
         << "  install <facility> on <planet> - Install a facility (Generator/Accumulator)\n"
         << "  transfer <resource> <amount> from <planet1> to <planet2> - Transfer resources\n"
         << "  radar <planet>                 - Get radar info for a planet\n"
         << "  daily                          - Show the current daily (or story) quest\n"
         << "  talk                           - Talk to characters for lore\n"
         << "  journal                        - List open journal entries and then select one to view\n"
         << "  save                           - Save game\n"
         << "  quit                           - Save and quit\n";
}

int main() {
    Player player;
    loadGame(player);
    player.recalcUpgradesFromResearch();
    cout << "Welcome to the Expanded Modular Idle Planet Miner!" << endl;
    cout << "Type 'help' for commands." << endl;
    while (true) {
        player.produceResources();
        player.updateDailyQuest();
        cout << "> ";
        string command;
        if (!getline(cin, command))
            break;
        if (command.empty())
            continue;
        auto spacePos = command.find(' ');
        string cmd = (spacePos == string::npos) ? command
                                                : command.substr(0, spacePos);
        string args = (spacePos == string::npos) ? "" : command.substr(spacePos + 1);
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
        if (cmd == "help")
            showHelp();
        else if (cmd == "stats") {
            auto &planets = player.getPlanetManager().getPlanets();
            for (auto &planet : planets) {
                cout << "Planet " << planet.getName() << " storage:" << endl;
                for (const auto &entry : planet.getStorageMap())
                    cout << "  " << entry.first << ": " << entry.second << endl;
            }
        } else if (cmd == "planets") {
            const auto &plist = player.getPlanetManager().getPlanetsConst();
            for (const auto &planet : plist) {
                cout << planet.getName() << " => " 
                     << (planet.isUnlocked() ? "Unlocked" : "Locked");
                if (!planet.isUnlocked()) {
                    if (planet.getName() == "Mars")
                        cout << " (Requires research: Unlock Mars)";
                    else if (planet.getName() == "Zalthor")
                        cout << " (Requires research: Unlock Zalthor)";
                    else if (planet.getName() == "Vulcan")
                        cout << " (Requires research: Unlock Vulcan)";
                    else if (planet.getName() == "Luna")
                        cout << " (Requires research: Unlock Luna)";
                }
                cout << " | Energy: " << planet.getCurrentEnergy() << "/" << planet.getMaxEnergy()
                     << " | Building Plots: " << planet.getUsedBuildingPlots() << "/" << planet.getMaxBuildingPlots() << endl;
            }
        } else if (cmd == "fleet")
            player.showFleet();
        else if (cmd == "research") {
            for (const auto &research : player.getResearchManager().getAllResearchesConst()) {
                cout << research.getName() << " - " 
                     << (research.isCompleted() ? "Completed" : "Not Completed")
                     << " | Cost: ";
                for (const auto &cost : research.getCost())
                    cout << cost.first << ":" << cost.second << " ";
                cout << "| " << research.getDescription() << endl;
            }
        } else if (cmd == "do_research") {
            if (args.empty())
                cout << "Usage: do_research <research name>" << endl;
            else
                player.doResearch(args);
        }
        else if (cmd == "craft") {
            // If no extra arguments are provided, use interactive mode.
            if (args.empty()) {
                cout << "Available craftable items:" << endl;
                // List each craftable item from the global CRAFTING_RECIPES
                for (const auto &entry : CRAFTING_RECIPES) {
                    string itemName = entry.first;
                    string reqBuilding = entry.second.requiredBuilding;
                    cout << " - " << itemName << " (Requires: " << reqBuilding << ")";
                    // List unlocked planets that already have the required building.
                    vector<string> availablePlanets;
                    for (const auto &planet : player.getPlanetManager().getPlanetsConst()) {
                        if (planet.isUnlocked() && planet.getBuildings().count(reqBuilding) && planet.getBuildings().at(reqBuilding) > 0)
                            availablePlanets.push_back(planet.getName());
                    }
                    if (!availablePlanets.empty()) {
                        cout << " Available on: ";
                        for (const auto &p : availablePlanets)
                            cout << p << " ";
                    } else {
                        cout << " [Locked: No planet has " << reqBuilding << "]";
                    }
                    cout << endl;
                }
                cout << "Enter the item you want to craft: ";
                string chosenItem;
                getline(cin, chosenItem);
                if (chosenItem.empty()) {
                    cout << "No item chosen." << endl;
                    continue;
                }
                if (CRAFTING_RECIPES.find(chosenItem) == CRAFTING_RECIPES.end()) {
                    cout << "Invalid item selected." << endl;
                    continue;
                }
                string reqBuilding = CRAFTING_RECIPES[chosenItem].requiredBuilding;
                vector<string> availablePlanets;
                for (const auto &planet : player.getPlanetManager().getPlanetsConst()) {
                    if (planet.isUnlocked() && planet.getBuildings().count(reqBuilding) && planet.getBuildings().at(reqBuilding) > 0)
                        availablePlanets.push_back(planet.getName());
                }
                if (availablePlanets.empty()) {
                    cout << "No available planets have the required building (" << reqBuilding << ") for crafting " << chosenItem << "." << endl;
                    continue;
                } else {
                    cout << "Available planets for crafting " << chosenItem << ": ";
                    for (const auto &p : availablePlanets)
                        cout << p << " ";
                    cout << endl;
                }
                cout << "Enter the planet you want to craft on: ";
                string chosenPlanet;
                getline(cin, chosenPlanet);
                if (chosenPlanet.empty()) {
                    cout << "No planet chosen." << endl;
                    continue;
                }
                // Determine if this is a ship (if required building is Shipyard or Flagship Dock)
                if (reqBuilding == "Shipyard" || reqBuilding == "Flagship Dock")
                    player.craftShip(chosenItem, chosenPlanet);
                else
                    player.craftItem(chosenItem, chosenPlanet);
            } else {
                // Fallback to non-interactive mode if arguments are provided.
                string itemName, planetName;
                size_t pos = args.find(" on ");
                if (pos != string::npos) {
                    itemName = args.substr(0, pos);
                    planetName = args.substr(pos + 4);
                } else {
                    itemName = args;
                    planetName = "Terra";
                }
                if (itemName.empty())
                    cout << "Usage: craft <item> [on <planet>]" << endl;
                else {
                    auto it = CRAFTING_RECIPES.find(itemName);
                    if (it == CRAFTING_RECIPES.end()) {
                        cout << "No recipe for item '" << itemName << "'." << endl;
                    } else {
                        string reqBuilding = it->second.requiredBuilding;
                        if (reqBuilding == "Shipyard" || reqBuilding == "Flagship Dock")
                            player.craftShip(itemName, planetName);
                        else
                            player.craftItem(itemName, planetName);
                    }
                }
            }
        } else if (cmd == "build") {
            size_t pos = args.find(" on ");
            if (pos != string::npos) {
                string buildingName = args.substr(0, pos);
                string planetName = args.substr(pos + 4);
                if (buildingName.empty() || planetName.empty())
                    cout << "Usage: build <building> on <planet>" << endl;
                else
                    player.buildBuilding(buildingName, planetName);
            } else
                cout << "Usage: build <building> on <planet>" << endl;
        } else if (cmd == "upgrade_plots") {
            if (args.empty())
                cout << "Usage: upgrade_plots <planet>" << endl;
            else
                player.upgradeBuildingPlots(args);
        } else if (cmd == "install") {
            size_t pos = args.find(" on ");
            if (pos != string::npos) {
                string facility = args.substr(0, pos);
                string planetName = args.substr(pos + 4);
                if (facility.empty() || planetName.empty())
                    cout << "Usage: install <facility> on <planet>" << endl;
                else {
                    Planet *planet = player.getPlanetManager().getPlanetByName(planetName);
                    if (!planet)
                        cout << "Planet " << planetName << " not found." << endl;
                    else {
                        string facLower = facility;
                        transform(facLower.begin(), facLower.end(), facLower.begin(),
                                  ::tolower);
                        if (facLower == "generator") {
                            if (!planet->canBuildMore()) {
                                cout << "No building plots available on " << planetName
                                     << " for Generator." << endl;
                                break;
                            }
                        }
                        if (planet->removeFromStorage(facility, 1))
                            planet->installFacility(facility);
                        else
                            cout << "No " << facility << " available on " << planetName
                                 << "." << endl;
                    }
                }
            } else
                cout << "Usage: install <facility> on <planet>" << endl;
        } else if (cmd == "transfer") {
            istringstream iss(args);
            string resource;
            int amount;
            string fromWord, fromPlanet, toWord, toPlanet;
            if (!(iss >> resource >> amount >> fromWord >> fromPlanet >> toWord >> toPlanet))
                cout << "Usage: transfer <resource> <amount> from <planet1> to <planet2>"
                     << endl;
            else {
                if (fromWord != "from" || toWord != "to")
                    cout << "Usage: transfer <resource> <amount> from <planet1> to <planet2>"
                         << endl;
                else
                    player.transferResource(resource, amount, fromPlanet, toPlanet);
            }
        } else if (cmd == "radar") {
            if (args.empty())
                cout << "Usage: radar <planet>" << endl;
            else
                player.showRadar(args);
        } else if (cmd == "daily") {
            DailyQuest *dq = player.getQuestManager().getCurrentQuest();
            if (dq) {
                cout << dq->getDescription() << endl;
                if (dq->isRaider())
                    cout << "Target Planet: " << dq->getTargetPlanet() << endl;
                else
                    cout << "Objective: " << dq->getObjectiveAmount() << " "
                         << dq->getObjectiveResource() << endl;
                cout << "Completed: " << (dq->isCompleted() ? "Yes" : "No") << endl;
            } else
                cout << "No current daily quest." << endl;
        } else if (cmd == "talk") {
            talkToCharacters();
        } else if (cmd == "journal") {
            // List all open journal entries and then prompt for which entry to view.
            journal.listEntries();
            cout << "Enter a journal entry number to read (or press enter to cancel): ";
            string input;
            getline(cin, input);
            if (!input.empty()) {
                try {
                    int entryNum = stoi(input);
                    journal.viewEntry(entryNum);
                } catch (...) {
                    cout << "Invalid entry number." << endl;
                }
            }
        } else if (cmd == "save") {
            saveGame(player);
        } else if (cmd == "quit") {
            saveGame(player);
            cout << "Exiting game. Goodbye!" << endl;
            break;
        } else
            cout << "Unknown command. Type 'help' for commands." << endl;
    }
    return 0;
}
