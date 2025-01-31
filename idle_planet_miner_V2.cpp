#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <memory>
#include <sstream>
#include <utility>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

static const string SAVE_FILE = "save_game.json";

// -----------------------------------------------------------------------------
// 1) DATA DEFINITIONS
// -----------------------------------------------------------------------------

// Global resource definitions.
struct ResourceDef {
    string name;
    int initial;
};

static vector<ResourceDef> RESOURCE_DATA = {
    {"Iron",           0},
    {"Copper",         0},
    {"Mithril",        0},
    {"Coal",           0},
    {"Tin",            0},
    {"Silver",         0},
    {"Gold",           0},
    {"Iron Bar",       0},
    {"Copper Bar",     0},
    {"Mithril Bar",    0},
    {"Engine Parts",   0},
    {"Titanium",       0},
    {"Titanium Bar",   0},
    {"Generator",      0},
    {"Accumulator",    0},
    {"Obsidian",       0},
    {"Crystal",        0},
    {"Nanomaterial",   0},
    {"Advanced Engine Parts", 0},
    {"Fusion Reactor", 0}
};

// Planet definitions. Each planet produces resources per second.
struct PlanetDef {
    string name;
    map<string, double> baseProduction;
    bool unlocked;
};

static vector<PlanetDef> PLANET_DATA = {
    {
        "Terra",
        {{"Iron", 0.5}, {"Copper", 0.5}, {"Coal", 0.2}},
        true
    },
    {
        "Mars",
        {{"Iron", 0.1}, {"Copper", 0.1}, {"Mithril", 0.05}, {"Coal", 0.1}},
        false
    },
    {
        "Zalthor",
        {{"Mithril", 0.1}, {"Coal", 0.2}, {"Gold", 0.02}},
        false
    },
    {
        "Vulcan",
        {{"Tin", 0.1}, {"Silver", 0.03}, {"Titanium", 0.01}},
        false
    },
    {
        "Luna",
        {{"Obsidian", 0.1}, {"Crystal", 0.05}, {"Nanomaterial", 0.02}},
        false
    }
};

// Research definitions.
// (Note: The "Raider Incursions" research has been removed in favor of a quest‐based raider event.)
struct ResearchDef {
    string name;
    map<string,int> cost;
    string description;
    string effectName;
};

static vector<ResearchDef> RESEARCH_DATA = {
    {
        "Unlock Mars",
        {{"Iron",100}, {"Copper",50}},
        "Unlock the planet Mars for mining.",
        "unlock_mars"
    },
    {
        "Unlock Zalthor",
        {{"Iron",200}, {"Mithril",50}},
        "Unlock planet Zalthor for advanced mining.",
        "unlock_zalthor"
    },
    {
        "Faster Crafting",
        {{"Iron",50}, {"Coal",30}},
        "Reduce smelting time by half.",
        "faster_crafting"
    },
    {
        "Unlock Vulcan",
        {{"Gold",100}, {"Mithril",100}},
        "Unlock planet Vulcan for high‑value resources.",
        "unlock_vulcan"
    },
    {
        "Unlock Luna",
        {{"Gold",150}, {"Mithril",100}},
        "Unlock planet Luna to harvest exotic materials.",
        "unlock_luna"
    },
    {
        "Crafting Mastery",
        {{"Engine Parts",5}, {"Titanium Bar",3}},
        "Reduces crafting energy cost by 20%.",
        "crafting_mastery"
    },
    {
        "Precision Tools",
        {{"Iron Bar",10}, {"Mithril Bar",5}},
        "Gives a 10% chance to double crafted output.",
        "precision_tools"
    }
};

// Crafting recipes now include a requiredBuilding field.
struct CraftingRecipe {
    map<string,int> inputs;
    double timeRequired;
    double electricityCost;
    string requiredBuilding; // e.g., "Smelting Building", "Crafting Building", etc.
};

static map<string, CraftingRecipe> CRAFTING_RECIPES = {
    {"Iron Bar",             { {{"Iron",5}},              1.0, 2.0, "Smelting Building" }},
    {"Copper Bar",           { {{"Copper",5}},            1.0, 2.0, "Smelting Building" }},
    {"Mithril Bar",          { {{"Mithril",5}},           2.0, 3.0, "Smelting Building" }},
    {"Titanium Bar",         { {{"Titanium",10}},         5.0, 7.0, "Smelting Building" }},
    {"Engine Parts",         { {{"Iron Bar",2}},          3.0, 5.0, "Crafting Building" }},
    {"Advanced Engine Parts",{ {{"Engine Parts",2}, {"Nanomaterial",1}}, 4.0, 6.0, "Crafting Building" }},
    {"Fusion Reactor",       { {{"Titanium Bar",5}, {"Crystal",3}, {"Nanomaterial",2}}, 10.0, 15.0, "Crafting Building" }},
    {"Generator",            { {{"Copper",10}, {"Iron Bar",5}}, 2.0, 0.0, "Facility Workshop" }},
    {"Accumulator",          { {{"Tin",10}, {"Copper Bar",5}}, 3.0, 0.0, "Facility Workshop" }},
    {"Transport Vessel",     { {{"Iron Bar",5}, {"Engine Parts",1}}, 3.0, 10.0, "Shipyard" }},
    {"Corvette",             { {{"Titanium Bar",3}, {"Advanced Engine Parts",1}}, 5.0, 15.0, "Shipyard" }}
};

// Building recipes for constructing buildings on a planet.
struct BuildingRecipe {
    map<string,int> inputs;
    double timeRequired;
    double electricityCost;
};

static map<string, BuildingRecipe> BUILDING_RECIPES = {
    {"Crafting Building", { {{"Iron Bar", 5}, {"Engine Parts", 2}}, 5.0, 10.0 }},
    {"Smelting Building", { {{"Copper Bar", 5}, {"Coal", 10}}, 5.0, 10.0 }},
    {"Facility Workshop", { {{"Generator", 2}, {"Accumulator", 2}}, 3.0, 5.0 }},
    {"Shipyard",          { {{"Iron Bar", 10}, {"Engine Parts", 5}}, 8.0, 20.0 }},
    {"Radar",             { {{"Crystal", 3}, {"Mithril Bar", 1}}, 4.0, 5.0 }},
    {"Salvage Robot",     { {{"Obsidian", 2}, {"Engine Parts", 1}}, 3.0, 5.0 }},
    {"Shield Generator",  { {{"Titanium Bar", 2}, {"Copper Bar", 2}}, 4.0, 5.0 }}
};

// -----------------------------------------------------------------------------
// 2) MODEL CLASSES
// -----------------------------------------------------------------------------

// The Resource class remains unchanged.
class Resource {
public:
    Resource() : name(""), quantity(0) {}
    Resource(const string& n, int q) : name(n), quantity(q) {}

    void add(int amount) { quantity += amount; }
    bool remove(int amount) {
        if (quantity >= amount) {
            quantity -= amount;
            return true;
        }
        return false;
    }
    const string& getName() const { return name; }
    int getQuantity() const { return quantity; }
private:
    string name;
    int quantity;
};

// Each Planet now has its own resource storage and a building list.
class Planet {
public:
    Planet(const string& n, const map<string,double>& prod, bool u)
        : name(n), baseProduction(prod), unlocked(u),
          generators(0), accumulators(0),
          currentEnergy(0.0), maxEnergy(50.0)
    {
        // Initialize storage for all resources.
        for (auto &res : RESOURCE_DATA) {
            storage[res.name] = 0;
        }
    }

    // Produce resources (and add them to storage) based on elapsed time and available energy.
    void produceResources(double elapsedSeconds) {
        if (!unlocked) return;
        double energyProduced = generators * GENERATOR_RATE * elapsedSeconds;
        currentEnergy = min(maxEnergy, currentEnergy + energyProduced);
        double effectiveSeconds = min(elapsedSeconds, currentEnergy / ENERGY_COST_PER_SECOND);
        double energyConsumed = effectiveSeconds * ENERGY_COST_PER_SECOND;
        currentEnergy -= energyConsumed;
        for (auto& kv : baseProduction) {
            const string& resourceName = kv.first;
            double totalProduced = kv.second * effectiveSeconds;
            int produceAmt = static_cast<int>(floor(totalProduced));
            if (produceAmt > 0) {
                storage[resourceName] += produceAmt;
            }
        }
    }

    // Storage management.
    void addToStorage(const string& resourceName, int amount) {
        storage[resourceName] += amount;
    }
    bool removeFromStorage(const string& resourceName, int amount) {
        if (storage[resourceName] >= amount) {
            storage[resourceName] -= amount;
            return true;
        }
        return false;
    }
    int getStored(const string& resourceName) const {
        auto it = storage.find(resourceName);
        return (it != storage.end()) ? it->second : 0;
    }
    const map<string,int>& getStorageMap() const { return storage; }

    // Building management.
    void addBuilding(const string& buildingName) {
        buildings[buildingName] += 1;
    }
    bool hasBuilding(const string& buildingName) const {
        auto it = buildings.find(buildingName);
        return (it != buildings.end() && it->second > 0);
    }
    const map<string,int>& getBuildings() const { return buildings; }

    const string& getName() const { return name; }
    bool isUnlocked() const { return unlocked; }
    void setUnlocked(bool u) { unlocked = u; }

    double getCurrentEnergy() const { return currentEnergy; }
    double getMaxEnergy() const { return maxEnergy; }
    int getGenerators() const { return generators; }
    int getAccumulators() const { return accumulators; }

    void setCurrentEnergy(double e) { currentEnergy = e; }
    void setMaxEnergy(double e) { maxEnergy = e; }
    void setGenerators(int g) { generators = g; }
    void setAccumulators(int a) { accumulators = a; }

    // Install a facility item (e.g. Generator, Accumulator) on this planet.
    void installFacility(const string& facility) {
        string fac = facility;
        transform(fac.begin(), fac.end(), fac.begin(), ::tolower);
        if (fac == "generator") {
            generators++;
            cout << "Installed a Generator on " << name 
                 << ". Total generators: " << generators << endl;
        } else if (fac == "accumulator") {
            accumulators++;
            maxEnergy += 50.0;
            cout << "Installed an Accumulator on " << name 
                 << ". Total accumulators: " << accumulators 
                 << ", new max energy: " << maxEnergy << endl;
        } else {
            cout << "Unknown facility: " << facility << endl;
        }
    }

    // Energy constants.
    static constexpr double GENERATOR_RATE = 5.0;
    static constexpr double ENERGY_COST_PER_SECOND = 1.0;
private:
    string name;
    map<string,double> baseProduction;
    bool unlocked;
    int generators;
    int accumulators;
    double currentEnergy;
    double maxEnergy;
    map<string,int> storage;
    map<string,int> buildings;
};

// The Research class now uses a planet’s storage (assumed to be Terra) for research.
class Research {
public:
    Research(const string& n, const map<string,int>& c,
             const string& desc, const string& effName)
      : name(n), cost(c), description(desc),
        effectName(effName), completed(false) {}

    bool canResearch(Planet* central) const {
        for (auto& kv : cost) {
            if (central->getStored(kv.first) < kv.second)
                return false;
        }
        return true;
    }

    void doResearch(Planet* central) {
        for (auto& kv : cost) {
            central->removeFromStorage(kv.first, kv.second);
        }
        completed = true;
    }

    const string& getName() const { return name; }
    const string& getEffectName() const { return effectName; }
    bool isCompleted() const { return completed; }
    void setCompleted(bool val) { completed = val; }
    const map<string,int>& getCost() const { return cost; }
    const string& getDescription() const { return description; }
private:
    string name;
    map<string,int> cost;
    string description;
    string effectName;
    bool completed;
};

// The DailyQuest class now can represent both resource‐collection quests and raider attack quests.
class DailyQuest {
public:
    DailyQuest(const string& d, const string& objRes, int objAmt, const map<string,int>& rew)
        : description(d), objectiveResource(objRes), objectiveAmount(objAmt),
          reward(rew), completed(false), isRaiderAttack(false), targetPlanet("")
    {}

    // For normal quests: check if the central planet has enough of the objective resource.
    void checkCompletion(Planet* central) {
        if (completed) return;
        if (!isRaiderAttack) {
            if (central->getStored(objectiveResource) >= objectiveAmount) {
                completed = true;
                for (auto& kv : reward) {
                    central->addToStorage(kv.first, kv.second);
                }
                cout << "Quest complete! Reward: ";
                for (auto& kv : reward) {
                    cout << kv.first << " +" << kv.second << " ";
                }
                cout << endl;
            }
        }
    }

    // For raider attack quests, simulate a battle.
    // Returns true if successfully defended; false if defense fails.
    bool processRaiderAttack(class PlanetManager& pm, vector<struct Ship>& fleet) {
        if (completed || !isRaiderAttack) return false;
        Planet* target = pm.getPlanetByName(targetPlanet);
        if (!target) {
            cout << "Target planet " << targetPlanet << " not found for raider attack simulation." << endl;
            return false;
        }
        // Compute defense rating from planet's Shield Generators.
        int numShieldGen = 0;
        const auto& blds = target->getBuildings();
        auto it = blds.find("Shield Generator");
        if (it != blds.end())
            numShieldGen = it->second;
        int defenseRating = numShieldGen * 30;
        // Add bonus from corvettes in the global fleet.
        int numCorvettes = 0;
        for (auto& ship : fleet) {
            if (ship.type == "Corvette")
                numCorvettes++;
        }
        defenseRating += numCorvettes * 20;
        int raiderStrength = rand() % 101 + 50; // random between 50 and 150
        cout << "Raiders are attacking " << targetPlanet << "!" << endl;
        cout << "Defense rating: " << defenseRating << " vs Raider strength: " << raiderStrength << endl;
        if (defenseRating >= raiderStrength) {
            cout << "You successfully defended " << targetPlanet << " against the raiders!" << endl;
            // Reward: add bonus resources (to Terra).
            Planet* terra = pm.getPlanetByName("Terra");
            if (terra) {
                terra->addToStorage("Engine Parts", 2);
                cout << "You received 2 Engine Parts as a reward." << endl;
            }
            completed = true;
            return true;
        } else {
            cout << "Your defenses failed! All corvettes have been destroyed in the battle." << endl;
            // Remove all corvettes from fleet.
            for (auto it = fleet.begin(); it != fleet.end(); ) {
                if (it->type == "Corvette")
                    it = fleet.erase(it);
                else
                    ++it;
            }
            // If the target planet has a Salvage Robot, salvage some debris.
            int numSalvage = 0;
            auto sit = blds.find("Salvage Robot");
            if (sit != blds.end())
                numSalvage = sit->second;
            if (numSalvage > 0) {
                int salvageAmount = (rand() % 11) + 5; // salvage between 5 and 15 Iron, for example.
                target->addToStorage("Iron", salvageAmount);
                cout << "Your Salvage Robot recovered " << salvageAmount << " Iron from the wreckage." << endl;
            }
            completed = true;
            return false;
        }
    }

    bool isCompleted() const { return completed; }
    const string& getDescription() const { return description; }
    const string& getObjectiveResource() const { return objectiveResource; }
    int getObjectiveAmount() const { return objectiveAmount; }
    const map<string,int>& getReward() const { return reward; }

    // Additional members for raider attack quests:
    bool isRaider() const { return isRaiderAttack; }
    void setRaiderAttack(const string& target) { isRaiderAttack = true; targetPlanet = target; }
    const string& getTargetPlanet() const { return targetPlanet; }

private:
    string description;
    string objectiveResource;
    int objectiveAmount;
    map<string,int> reward;
    bool completed;

    // For raider attack quests:
    bool isRaiderAttack;
    string targetPlanet;
};

// -----------------------------------------------------------------------------
// 3) MANAGERS
// -----------------------------------------------------------------------------

// PlanetManager manages all planets.
class PlanetManager {
public:
    PlanetManager(const vector<PlanetDef>& data) {
        for (auto& p : data) {
            planets.push_back(Planet(p.name, p.baseProduction, p.unlocked));
        }
    }

    void produceAll(double elapsedSeconds) {
        for (auto& planet : planets) {
            planet.produceResources(elapsedSeconds);
        }
    }

    void unlockPlanet(const string& planetName) {
        for (auto& p : planets) {
            if (p.getName() == planetName)
                p.setUnlocked(true);
        }
    }

    Planet* getPlanetByName(const string& planetName) {
        for (auto& p : planets) {
            if (p.getName() == planetName)
                return &p;
        }
        return nullptr;
    }

    const vector<Planet>& getPlanetsConst() const { return planets; }
    vector<Planet>& getPlanets() { return planets; }
private:
    vector<Planet> planets;
};

// ResearchManager manages research.
class ResearchManager {
public:
    ResearchManager(const vector<ResearchDef>& data) {
        for (auto& rd : data) {
            researches.push_back(Research(rd.name, rd.cost, rd.description, rd.effectName));
        }
    }

    Research* findResearchByName(const string& name) {
        for (auto& r : researches) {
            string lower1 = r.getName(), lower2 = name;
            transform(lower1.begin(), lower1.end(), lower1.begin(), ::tolower);
            transform(lower2.begin(), lower2.end(), lower2.begin(), ::tolower);
            if (lower1 == lower2)
                return &r;
        }
        return nullptr;
    }

    const vector<Research>& getAllResearchesConst() const { return researches; }
    vector<Research>& getAllResearches() { return researches; }
private:
    vector<Research> researches;
};

// CraftingManager works on a given planet's storage.
class CraftingManager {
public:
    CraftingManager(const map<string, CraftingRecipe>& data)
        : recipes(data) {}

    void craft(const string& outputItem,
               Planet* planet,
               double speedMultiplier = 1.0,
               double costMultiplier = 1.0,
               bool precisionEnabled = false)
    {
        auto it = recipes.find(outputItem);
        if (it == recipes.end()) {
            cout << "No known recipe for '" << outputItem << "'!" << endl;
            return;
        }
        // Check required building.
        if (!it->second.requiredBuilding.empty()) {
            if (!planet->hasBuilding(it->second.requiredBuilding)) {
                cout << "You need a " << it->second.requiredBuilding << " on " << planet->getName() 
                     << " to craft " << outputItem << "." << endl;
                return;
            }
        }
        // Check input resources.
        for (auto& kv : it->second.inputs) {
            if (planet->getStored(kv.first) < kv.second) {
                cout << "Not enough " << kv.first << " on " << planet->getName() 
                     << " to craft " << outputItem << "." << endl;
                return;
            }
        }
        double effectiveCost = it->second.electricityCost * costMultiplier;
        if (planet->getCurrentEnergy() < effectiveCost) {
            cout << "Not enough energy on " << planet->getName() << " to craft " << outputItem << "." << endl;
            return;
        }
        planet->setCurrentEnergy(planet->getCurrentEnergy() - effectiveCost);
        for (auto& kv : it->second.inputs) {
            planet->removeFromStorage(kv.first, kv.second);
        }
        planet->addToStorage(outputItem, 1);
        cout << "Crafted 1 " << outputItem << " on " << planet->getName() 
             << " using { ";
        for (auto& kv : it->second.inputs) {
            cout << kv.first << ":" << kv.second << " ";
        }
        cout << "} and consuming " << effectiveCost << " energy." << endl;
        if (precisionEnabled && (rand() % 100) < 10) {
            planet->addToStorage(outputItem, 1);
            cout << "Precision Tools activated! Bonus " << outputItem << " produced!" << endl;
        }
    }
private:
    map<string, CraftingRecipe> recipes;
};

// QuestManager generates daily quests. Sometimes (20% chance) it creates a Raider Attack quest.
class QuestManager {
public:
    QuestManager() : currentQuest(nullptr), lastQuestDate("") {}

    void updateDailyQuest(Planet* central, PlanetManager& pm, vector<struct Ship>& fleet) {
        string todayStr = currentDateString();
        if (lastQuestDate.empty() || lastQuestDate < todayStr) {
            generateNewQuest(pm);
            lastQuestDate = todayStr;
        }
        if (currentQuest) {
            if (currentQuest->isRaider()) {
                // Process raider attack quest.
                currentQuest->processRaiderAttack(pm, fleet);
            } else {
                currentQuest->checkCompletion(central);
            }
        }
    }

    DailyQuest* getCurrentQuest() { return currentQuest.get(); }
    const DailyQuest* getCurrentQuestConst() const { return currentQuest.get(); }
    const string& getLastQuestDate() const { return lastQuestDate; }
    void setLastQuestDate(const string& s) { lastQuestDate = s; }
    void setCurrentQuest(unique_ptr<DailyQuest> q) { currentQuest = move(q); }
private:
    unique_ptr<DailyQuest> currentQuest;
    string lastQuestDate;

    void generateNewQuest(PlanetManager& pm) {
        // 20% chance to generate a Raider Attack quest.
        if ((rand() % 100) < 20) {
            // Choose a random unlocked planet (other than Terra if possible).
            vector<string> candidates;
            for (auto& p : pm.getPlanets()) {
                if (p.isUnlocked() && p.getName() != "Terra")
                    candidates.push_back(p.getName());
            }
            if (candidates.empty())
                candidates.push_back("Terra");
            string target = candidates[rand() % candidates.size()];
            string desc = "Raiders are attacking " + target + "! Defend it!";
            map<string,int> reward = { {"Engine Parts", 2} };
            // For raider quests, objectiveAmount is 1 and objectiveResource is not used.
            auto quest = make_unique<DailyQuest>(desc, "", 1, reward);
            quest->setRaiderAttack(target);
            currentQuest = move(quest);
        } else {
            // Normal quest: collect a random amount of a randomly chosen resource.
            vector<string> resourceChoices = {"Iron Bar", "Copper Bar", "Mithril Bar", "Titanium Bar", "Advanced Engine Parts"};
            int idx = rand() % resourceChoices.size();
            string chosenResource = resourceChoices[idx];
            int objective_amount = rand() % 41 + 10;
            map<string,int> reward = { {"Engine Parts", (rand() % 3) + 1} };
            string desc = "Collect " + to_string(objective_amount) + " " + chosenResource;
            currentQuest = make_unique<DailyQuest>(desc, chosenResource, objective_amount, reward);
        }
    }

    string currentDateString() {
        time_t now = time(nullptr);
        tm* gmt = gmtime(&now);
        char buf[11];
        strftime(buf, sizeof(buf), "%Y-%m-%d", gmt);
        return string(buf);
    }
};

// -----------------------------------------------------------------------------
// 4) PLAYER CLASS
// -----------------------------------------------------------------------------

// A simple Ship struct representing either a Transport Vessel or a Corvette.
struct Ship {
    string type; // "Transport Vessel" or "Corvette"
    int shield;
    int hull;
    int weapons; // For Corvette; 0 for Transport Vessel.
};

class Player {
public:
    Player()
        : planetManager(PLANET_DATA),
          researchManager(RESEARCH_DATA),
          craftingManager(CRAFTING_RECIPES),
          fasterCraftingMultiplier(1.0),
          craftingCostMultiplier(1.0),
          precisionToolsEnabled(false),
          lastUpdateTime(time(nullptr))
    {
        srand(static_cast<unsigned>(time(nullptr)));
    }

    // Non-const getters.
    PlanetManager& getPlanetManager() { return planetManager; }
    ResearchManager& getResearchManager() { return researchManager; }
    CraftingManager& getCraftingManager() { return craftingManager; }
    QuestManager& getQuestManager() { return questManager; }
    vector<Ship>& getFleet() { return fleet; }

    // Const versions.
    const PlanetManager& getPlanetManager() const { return planetManager; }
    const ResearchManager& getResearchManager() const { return researchManager; }
    const QuestManager& getQuestManager() const { return questManager; }
    const vector<Ship>& getFleet() const { return fleet; }

    double getFasterCraftingMultiplier() const { return fasterCraftingMultiplier; }
    double getCraftingCostMultiplier() const { return craftingCostMultiplier; }
    bool isPrecisionToolsEnabled() const { return precisionToolsEnabled; }

    void setFasterCraftingMultiplier(double val) { fasterCraftingMultiplier = val; }
    void setCraftingCostMultiplier(double val) { craftingCostMultiplier = val; }
    void setPrecisionToolsEnabled(bool val) { precisionToolsEnabled = val; }

    // Produce resources on all planets.
    void produceResources() {
        time_t now = time(nullptr);
        double diffSeconds = difftime(now, lastUpdateTime);
        if (diffSeconds < 0) diffSeconds = 0;
        planetManager.produceAll(diffSeconds);
        lastUpdateTime = now;
    }

    // Perform research using Terra's storage.
    void doResearch(const string& researchName) {
        Research* r = researchManager.findResearchByName(researchName);
        if (!r) {
            cout << "No research found by that name." << endl;
            return;
        }
        if (r->isCompleted()) {
            cout << "Research already completed." << endl;
            return;
        }
        Planet* terra = planetManager.getPlanetByName("Terra");
        if (!terra) {
            cout << "Central planet Terra not found." << endl;
            return;
        }
        if (r->canResearch(terra)) {
            r->doResearch(terra);
            applyResearchEffect(r->getEffectName());
            cout << "Research '" << r->getName() << "' completed!" << endl;
        } else {
            cout << "Not enough resources on Terra to perform research." << endl;
        }
    }

    // Craft an item on a specified planet.
    void craftItem(const string& itemName, const string& planetName = "Terra") {
        Planet* p = planetManager.getPlanetByName(planetName);
        if (!p) {
            cout << "Planet " << planetName << " not found." << endl;
            return;
        }
        craftingManager.craft(itemName, p, fasterCraftingMultiplier, craftingCostMultiplier, precisionToolsEnabled);
    }

    // Build a ship (Transport Vessel or Corvette) on a planet.
    void craftShip(const string& shipType, const string& planetName = "Terra") {
        Planet* p = planetManager.getPlanetByName(planetName);
        if (!p) {
            cout << "Planet " << planetName << " not found." << endl;
            return;
        }
        auto it = CRAFTING_RECIPES.find(shipType);
        if (it == CRAFTING_RECIPES.end()) {
            cout << "No recipe for ship type '" << shipType << "'." << endl;
            return;
        }
        if (!p->hasBuilding(it->second.requiredBuilding)) {
            cout << "You need a " << it->second.requiredBuilding << " on " << planetName 
                 << " to build a " << shipType << "." << endl;
            return;
        }
        for (auto& kv : it->second.inputs) {
            if (p->getStored(kv.first) < kv.second) {
                cout << "Not enough " << kv.first << " on " << planetName << " to build a " << shipType << "." << endl;
                return;
            }
        }
        double effectiveCost = it->second.electricityCost * craftingCostMultiplier;
        if (p->getCurrentEnergy() < effectiveCost) {
            cout << "Not enough energy on " << planetName << " to build a " << shipType << "." << endl;
            return;
        }
        p->setCurrentEnergy(p->getCurrentEnergy() - effectiveCost);
        for (auto& kv : it->second.inputs) {
            p->removeFromStorage(kv.first, kv.second);
        }
        cout << "Built a " << shipType << " on " << planetName << " consuming " << effectiveCost << " energy." << endl;
        Ship newShip;
        newShip.type = shipType;
        if (shipType == "Transport Vessel") {
            newShip.shield = 50;
            newShip.hull = 100;
            newShip.weapons = 0;
        } else if (shipType == "Corvette") {
            newShip.shield = 75;
            newShip.hull = 100;
            newShip.weapons = 30;
        }
        fleet.push_back(newShip);
    }

    // Build a building on a specified planet.
    void buildBuilding(const string& buildingName, const string& planetName) {
        Planet* p = planetManager.getPlanetByName(planetName);
        if (!p) {
            cout << "Planet " << planetName << " not found." << endl;
            return;
        }
        auto it = BUILDING_RECIPES.find(buildingName);
        if (it == BUILDING_RECIPES.end()) {
            cout << "No recipe for building '" << buildingName << "'." << endl;
            return;
        }
        for (auto& kv : it->second.inputs) {
            if (p->getStored(kv.first) < kv.second) {
                cout << "Not enough " << kv.first << " on " << planetName << " to build a " << buildingName << "." << endl;
                return;
            }
        }
        if (p->getCurrentEnergy() < it->second.electricityCost) {
            cout << "Not enough energy on " << planetName << " to build a " << buildingName << "." << endl;
            return;
        }
        p->setCurrentEnergy(p->getCurrentEnergy() - it->second.electricityCost);
        for (auto& kv : it->second.inputs) {
            p->removeFromStorage(kv.first, kv.second);
        }
        p->addBuilding(buildingName);
        cout << "Built a " << buildingName << " on " << planetName << "." << endl;
    }

    // Transfer resources between planets using a transport convoy.
    void transferResource(const string& resourceName, int amount, const string& fromPlanetName, const string& toPlanetName) {
        Planet* fromP = planetManager.getPlanetByName(fromPlanetName);
        Planet* toP = planetManager.getPlanetByName(toPlanetName);
        if (!fromP || !toP) {
            cout << "Invalid planet(s) specified for transfer." << endl;
            return;
        }
        if (fromP->getStored(resourceName) < amount) {
            cout << "Not enough " << resourceName << " on " << fromPlanetName << " to transfer." << endl;
            return;
        }
        bool hasTransport = false;
        for (auto& ship : fleet) {
            if (ship.type == "Transport Vessel") {
                hasTransport = true;
                break;
            }
        }
        if (!hasTransport) {
            cout << "You need at least one Transport Vessel in your fleet to transfer resources." << endl;
            return;
        }
        cout << "Initiating transfer of " << amount << " " << resourceName << " from " << fromPlanetName << " to " << toPlanetName << "." << endl;
        // 30% chance of a raider attack during transfer.
        if ((rand() % 100) < 30) {
            cout << "Raider attack on the convoy!" << endl;
            int convoyStrength = 0;
            int corvetteBonus = 0;
            // Remove one Transport Vessel from fleet for the convoy.
            for (auto it = fleet.begin(); it != fleet.end(); ) {
                if (it->type == "Transport Vessel") {
                    convoyStrength += it->shield;
                    it = fleet.erase(it);
                    break;
                } else {
                    ++it;
                }
            }
            for (auto& ship : fleet) {
                if (ship.type == "Corvette")
                    corvetteBonus += ship.weapons;
            }
            convoyStrength += corvetteBonus;
            int raiderStrength = rand() % 101 + 20;
            cout << "Convoy strength: " << convoyStrength << ", Raider strength: " << raiderStrength << endl;
            if (convoyStrength >= raiderStrength) {
                cout << "The convoy fended off the raiders!" << endl;
                fromP->removeFromStorage(resourceName, amount);
                toP->addToStorage(resourceName, amount);
            } else {
                cout << "The convoy was destroyed by raiders! Resources lost." << endl;
                // If the from-planet has a Salvage Robot, salvage some debris.
                if (fromP->hasBuilding("Salvage Robot")) {
                    int salvageAmount = (rand() % 11) + 5;
                    fromP->addToStorage("Iron", salvageAmount);
                    cout << "Your Salvage Robot recovered " << salvageAmount << " Iron from the wreckage." << endl;
                }
            }
        } else {
            fromP->removeFromStorage(resourceName, amount);
            toP->addToStorage(resourceName, amount);
            cout << "Transfer completed successfully." << endl;
        }
    }

    // Display radar information for a planet.
    void showRadar(const string& planetName) {
        Planet* p = planetManager.getPlanetByName(planetName);
        if (!p) {
            cout << "Planet " << planetName << " not found." << endl;
            return;
        }
        if (p->hasBuilding("Radar"))
            cout << "Radar on " << planetName << " indicates HIGH probability of raider activity." << endl;
        else
            cout << "No Radar installed on " << planetName << ". Raider activity unknown." << endl;
    }

    // Update daily quest using Terra's storage.
    void updateDailyQuest() {
        Planet* terra = planetManager.getPlanetByName("Terra");
        if (!terra) return;
        questManager.updateDailyQuest(terra, planetManager, fleet);
    }

    // Recalculate upgrade multipliers from completed research.
    void recalcUpgradesFromResearch() {
        fasterCraftingMultiplier = 1.0;
        craftingCostMultiplier = 1.0;
        precisionToolsEnabled = false;
        for (auto& r : researchManager.getAllResearches()) {
            if (r.isCompleted()) {
                string effect = r.getEffectName();
                if (effect == "faster_crafting")
                    fasterCraftingMultiplier = 0.5;
                else if (effect == "crafting_mastery")
                    craftingCostMultiplier = 0.8;
                else if (effect == "precision_tools")
                    precisionToolsEnabled = true;
            }
        }
    }

    // Display fleet information.
    void showFleet() {
        if (fleet.empty()) {
            cout << "Your fleet is empty." << endl;
            return;
        }
        cout << "Your fleet:" << endl;
        for (size_t i = 0; i < fleet.size(); i++) {
            cout << i+1 << ". " << fleet[i].type 
                 << " (Shield: " << fleet[i].shield 
                 << ", Hull: " << fleet[i].hull;
            if (fleet[i].type == "Corvette")
                cout << ", Weapons: " << fleet[i].weapons;
            cout << ")" << endl;
        }
    }

    time_t getLastUpdateTime() const { return lastUpdateTime; }
    void setLastUpdateTime(time_t t) { lastUpdateTime = t; }

private:
    PlanetManager planetManager;
    ResearchManager researchManager;
    CraftingManager craftingManager;
    QuestManager questManager;
    vector<Ship> fleet;

    double fasterCraftingMultiplier;
    double craftingCostMultiplier;
    bool precisionToolsEnabled;

    time_t lastUpdateTime;

    void applyResearchEffect(const string& effectName) {
        if (effectName == "unlock_mars")
            planetManager.unlockPlanet("Mars");
        else if (effectName == "unlock_zalthor")
            planetManager.unlockPlanet("Zalthor");
        else if (effectName == "faster_crafting")
            fasterCraftingMultiplier = 0.5;
        else if (effectName == "unlock_vulcan")
            planetManager.unlockPlanet("Vulcan");
        else if (effectName == "unlock_luna")
            planetManager.unlockPlanet("Luna");
        else if (effectName == "crafting_mastery")
            craftingCostMultiplier = 0.8;
        else if (effectName == "precision_tools")
            precisionToolsEnabled = true;
        else
            cout << "Unknown research effect: " << effectName << endl;
    }
};

// -----------------------------------------------------------------------------
// 5) SAVE/LOAD FUNCTIONS
// -----------------------------------------------------------------------------

void saveGame(const Player& player) {
    json j;
    // Save planets.
    {
        json jp = json::array();
        const auto& planets = player.getPlanetManager().getPlanetsConst();
        for (auto& p : planets) {
            json pObj;
            pObj["name"] = p.getName();
            pObj["unlocked"] = p.isUnlocked();
            pObj["current_energy"] = p.getCurrentEnergy();
            pObj["max_energy"] = p.getMaxEnergy();
            pObj["generators"] = p.getGenerators();
            pObj["accumulators"] = p.getAccumulators();
            json storageJson = json::object();
            for (auto& kv : p.getStorageMap()) {
                storageJson[kv.first] = kv.second;
            }
            pObj["storage"] = storageJson;
            json buildingJson = json::object();
            for (auto& kv : p.getBuildings()) {
                buildingJson[kv.first] = kv.second;
            }
            pObj["buildings"] = buildingJson;
            jp.push_back(pObj);
        }
        j["planets"] = jp;
    }
    // Save research.
    {
        json jrch = json::array();
        for (auto& r : player.getResearchManager().getAllResearchesConst()) {
            json rObj;
            rObj["name"] = r.getName();
            rObj["completed"] = r.isCompleted();
            jrch.push_back(rObj);
        }
        j["researches"] = jrch;
    }
    j["faster_crafting_multiplier"] = player.getFasterCraftingMultiplier();
    j["crafting_cost_multiplier"] = player.getCraftingCostMultiplier();
    j["precision_tools_enabled"] = player.isPrecisionToolsEnabled();
    // Save daily quest.
    {
        json questJson = nullptr;
        const DailyQuest* dq = player.getQuestManager().getCurrentQuestConst();
        if (dq) {
            questJson = json::object();
            questJson["description"] = dq->getDescription();
            questJson["objective_resource"] = dq->getObjectiveResource();
            questJson["objective_amount"] = dq->getObjectiveAmount();
            json rew = json::object();
            for (auto& kv : dq->getReward()) {
                rew[kv.first] = kv.second;
            }
            questJson["reward"] = rew;
            questJson["completed"] = dq->isCompleted();
            // If this is a raider attack quest, save the target planet.
            if (dq->isRaider())
                questJson["target_planet"] = dq->getTargetPlanet();
        }
        j["current_quest"] = questJson;
        j["last_quest_date"] = player.getQuestManager().getLastQuestDate();
    }
    j["last_update_time"] = static_cast<long long>(player.getLastUpdateTime());
    // Save fleet.
    {
        json fleetJson = json::array();
        for (auto& ship : player.getFleet()) {
            json shipJson;
            shipJson["type"] = ship.type;
            shipJson["shield"] = ship.shield;
            shipJson["hull"] = ship.hull;
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

void loadGame(Player& player) {
    ifstream ifs(SAVE_FILE);
    if (!ifs.is_open()) return;
    json j;
    ifs >> j;
    ifs.close();
    if (j.contains("planets") && j["planets"].is_array()) {
        auto& plist = player.getPlanetManager().getPlanets();
        for (auto& pj : j["planets"]) {
            string pname = pj["name"].get<string>();
            for (auto& p : plist) {
                if (p.getName() == pname) {
                    p.setUnlocked(pj["unlocked"].get<bool>());
                    if (pj.contains("current_energy"))
                        p.setCurrentEnergy(pj["current_energy"].get<double>());
                    if (pj.contains("max_energy"))
                        p.setMaxEnergy(pj["max_energy"].get<double>());
                    if (pj.contains("generators"))
                        p.setGenerators(pj["generators"].get<int>());
                    if (pj.contains("accumulators"))
                        p.setAccumulators(pj["accumulators"].get<int>());
                    if (pj.contains("storage") && pj["storage"].is_object()) {
                        for (auto& item : pj["storage"].items()) {
                            int stored = item.value().get<int>();
                            p.addToStorage(item.key(), stored - p.getStored(item.key()));
                        }
                    }
                    // (Buildings loading omitted for brevity.)
                    break;
                }
            }
        }
    }
    if (j.contains("researches") && j["researches"].is_array()) {
        for (auto& rr : j["researches"]) {
            string rname = rr["name"].get<string>();
            bool completed = rr["completed"].get<bool>();
            Research* res = player.getResearchManager().findResearchByName(rname);
            if (res)
                res->setCompleted(completed);
        }
    }
    if (j.contains("faster_crafting_multiplier"))
        player.setFasterCraftingMultiplier(j["faster_crafting_multiplier"].get<double>());
    if (j.contains("crafting_cost_multiplier"))
        player.setCraftingCostMultiplier(j["crafting_cost_multiplier"].get<double>());
    if (j.contains("precision_tools_enabled"))
        player.setPrecisionToolsEnabled(j["precision_tools_enabled"].get<bool>());
    if (j.contains("current_quest") && !j["current_quest"].is_null()) {
        auto questObj = j["current_quest"];
        string desc = questObj["description"].get<string>();
        string oRes = questObj["objective_resource"].get<string>();
        int oAmt = questObj["objective_amount"].get<int>();
        map<string,int> rew;
        if (questObj.contains("reward") && questObj["reward"].is_object()) {
            for (auto& kv : questObj["reward"].items())
                rew[kv.key()] = kv.value().get<int>();
        }
        bool completed = questObj["completed"].get<bool>();
        auto dq = make_unique<DailyQuest>(desc, oRes, oAmt, rew);
        if (questObj.contains("target_planet"))
            dq->setRaiderAttack(questObj["target_planet"].get<string>());
        player.getQuestManager().setCurrentQuest(move(dq));
        if (completed) {
            Planet* terra = player.getPlanetManager().getPlanetByName("Terra");
            if (terra)
                player.getQuestManager().getCurrentQuest()->checkCompletion(terra);
        }
    }
    if (j.contains("last_quest_date") && !j["last_quest_date"].is_null())
        player.getQuestManager().setLastQuestDate(j["last_quest_date"].get<string>());
    if (j.contains("last_update_time"))
        player.setLastUpdateTime(j["last_update_time"].get<long long>());
    if (j.contains("fleet") && j["fleet"].is_array()) {
        vector<Ship> fleet;
        for (auto& shipJson : j["fleet"]) {
            Ship s;
            s.type = shipJson["type"].get<string>();
            s.shield = shipJson["shield"].get<int>();
            s.hull = shipJson["hull"].get<int>();
            s.weapons = shipJson["weapons"].get<int>();
            fleet.push_back(s);
        }
        player.getFleet() = fleet;
    }
    player.recalcUpgradesFromResearch();
    cout << "Game loaded from " << SAVE_FILE << "." << endl;
}

// -----------------------------------------------------------------------------
// 6) MAIN GAME LOOP
// -----------------------------------------------------------------------------

void showHelp() {
    cout << "Commands:\n"
         << "  help                           - Show this help\n"
         << "  stats                          - Show resource storage on each planet\n"
         << "  planets                        - Show planet status (energy, buildings, storage)\n"
         << "  fleet                          - Show your fleet\n"
         << "  research                       - List research options\n"
         << "  do_research <name>             - Perform research (using Terra's storage)\n"
         << "  craft <item> [on <planet>]     - Craft an item on a planet\n"
         << "     (For ships, use 'craft Transport Vessel' or 'craft Corvette')\n"
         << "  build <building> on <planet>   - Build a building on a planet\n"
         << "  install <facility> on <planet> - Install a facility (Generator/Accumulator)\n"
         << "  transfer <resource> <amount> from <planet1> to <planet2> - Transfer resources\n"
         << "  radar <planet>                 - Get radar info for a planet\n"
         << "  daily                          - Show the current daily quest\n"
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
        string cmd = (spacePos == string::npos) ? command : command.substr(0, spacePos);
        string args = (spacePos == string::npos) ? "" : command.substr(spacePos + 1);
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        if (cmd == "help") {
            showHelp();
        }
        else if (cmd == "stats") {
            auto& planets = player.getPlanetManager().getPlanets();
            for (auto& p : planets) {
                cout << "Planet " << p.getName() << " storage:" << endl;
                for (auto& kv : p.getStorageMap())
                    cout << "  " << kv.first << ": " << kv.second << endl;
            }
        }
        else if (cmd == "planets") {
            auto& plist = player.getPlanetManager().getPlanetsConst();
            for (auto& p : plist) {
                cout << p.getName() << " => " << (p.isUnlocked() ? "Unlocked" : "Locked")
                     << " | Energy: " << p.getCurrentEnergy() << "/" << p.getMaxEnergy()
                     << " | Generators: " << p.getGenerators()
                     << ", Accumulators: " << p.getAccumulators() << endl;
                cout << "  Buildings: ";
                for (auto& b : p.getBuildings())
                    cout << b.first << " (" << b.second << ") ";
                cout << endl;
            }
        }
        else if (cmd == "fleet") {
            player.showFleet();
        }
        else if (cmd == "research") {
            for (auto& r : player.getResearchManager().getAllResearchesConst()) {
                cout << r.getName() << " - " << (r.isCompleted() ? "Completed" : "Not Completed")
                     << " | Cost: ";
                for (auto& c : r.getCost())
                    cout << c.first << ":" << c.second << " ";
                cout << "| " << r.getDescription() << endl;
            }
        }
        else if (cmd == "do_research") {
            if (args.empty())
                cout << "Usage: do_research <research name>" << endl;
            else
                player.doResearch(args);
        }
        else if (cmd == "craft") {
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
                if (itemName == "Transport Vessel" || itemName == "Corvette")
                    player.craftShip(itemName, planetName);
                else
                    player.craftItem(itemName, planetName);
            }
        }
        else if (cmd == "build") {
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
        }
        else if (cmd == "install") {
            size_t pos = args.find(" on ");
            if (pos != string::npos) {
                string facility = args.substr(0, pos);
                string planetName = args.substr(pos + 4);
                if (facility.empty() || planetName.empty())
                    cout << "Usage: install <facility> on <planet>" << endl;
                else {
                    Planet* p = player.getPlanetManager().getPlanetByName(planetName);
                    if (!p)
                        cout << "Planet " << planetName << " not found." << endl;
                    else {
                        if (p->removeFromStorage(facility, 1))
                            p->installFacility(facility);
                        else
                            cout << "No " << facility << " available in storage on " << planetName << "." << endl;
                    }
                }
            } else
                cout << "Usage: install <facility> on <planet>" << endl;
        }
        else if (cmd == "transfer") {
            istringstream iss(args);
            string resource;
            int amount;
            string fromWord, fromPlanet, toWord, toPlanet;
            if (!(iss >> resource >> amount >> fromWord >> fromPlanet >> toWord >> toPlanet))
                cout << "Usage: transfer <resource> <amount> from <planet1> to <planet2>" << endl;
            else {
                if (fromWord != "from" || toWord != "to")
                    cout << "Usage: transfer <resource> <amount> from <planet1> to <planet2>" << endl;
                else
                    player.transferResource(resource, amount, fromPlanet, toPlanet);
            }
        }
        else if (cmd == "radar") {
            if (args.empty())
                cout << "Usage: radar <planet>" << endl;
            else
                player.showRadar(args);
        }
        else if (cmd == "daily") {
            DailyQuest* dq = player.getQuestManager().getCurrentQuest();
            if (dq) {
                cout << dq->getDescription() << endl;
                if (dq->isRaider())
                    cout << "Target Planet: " << dq->getTargetPlanet() << endl;
                else
                    cout << "Objective: " << dq->getObjectiveAmount() << " " << dq->getObjectiveResource() << endl;
                cout << "Completed: " << (dq->isCompleted() ? "Yes" : "No") << endl;
            } else {
                cout << "No current daily quest." << endl;
            }
        }
        else if (cmd == "save") {
            saveGame(player);
        }
        else if (cmd == "quit") {
            saveGame(player);
            cout << "Exiting game. Goodbye!" << endl;
            break;
        }
        else
            cout << "Unknown command. Type 'help' for commands." << endl;
    }
    return 0;
}
