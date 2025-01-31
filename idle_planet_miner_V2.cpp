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
#include <utility>
#include <functional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

static const string SAVE_FILE = "save_game.json";

// -----------------------------------------------------------------------------
// 1) DATA DEFINITIONS
// -----------------------------------------------------------------------------

/**
 * Resources now include extra materials, advanced items, and defensive structures.
 */
struct ResourceDef {
    string name;
    int initial;
};

static vector<ResourceDef> RESOURCE_DATA = {
    {"Iron",               0},
    {"Copper",             0},
    {"Mithril",            0},
    {"Coal",               0},
    {"Tin",                0},
    {"Silver",             0},
    {"Gold",               0},
    {"Iron Bar",           0},
    {"Copper Bar",         0},
    {"Mithril Bar",        0},
    {"Engine Parts",       0},
    {"Titanium",           0},
    {"Titanium Bar",       0},
    {"Generator",          0},
    {"Accumulator",        0},
    {"Obsidian",           0},
    {"Crystal",            0},
    {"Nanomaterial",       0},
    {"Advanced Engine Parts", 0},
    {"Fusion Reactor",     0},
    {"Railgun Turret",     0},
    {"Shield",             0}
};

/**
 * Each planet produces resources per second and has its own power grid.
 */
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

/**
 * Research definitions. In addition to unlocking new planets, some research upgrades
 * directly affect crafting (reducing energy costs or giving a bonus output chance).
 */
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
        "Unlock planet Vulcan for high-value resources.",
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
        "Gives a 10% chance to double the output of crafted items.",
        "precision_tools"
    }
};

/**
 * Crafting recipes now include an electricityCost (energy required) and a time requirement.
 * New recipes for advanced items and defensive structures have been added.
 */
struct CraftingRecipe {
    map<string,int> inputs;
    double timeRequired;
    double electricityCost;
};

static map<string, CraftingRecipe> CRAFTING_RECIPES = {
    {"Iron Bar",             { {{"Iron",5}},                      1.0, 2.0 }},
    {"Copper Bar",           { {{"Copper",5}},                    1.0, 2.0 }},
    {"Mithril Bar",          { {{"Mithril",5}},                   2.0, 3.0 }},
    {"Engine Parts",         { {{"Iron Bar",2}},                  3.0, 5.0 }},
    {"Titanium Bar",         { {{"Titanium",10}},                 5.0, 7.0 }},
    {"Generator",            { {{"Copper",10}, {"Iron Bar",5}},    2.0, 0.0 }},
    {"Accumulator",          { {{"Tin",10}, {"Copper Bar",5}},     3.0, 0.0 }},
    {"Advanced Engine Parts", { {{"Engine Parts",2}, {"Nanomaterial",1}}, 4.0, 6.0 }},
    {"Fusion Reactor",       { {{"Titanium Bar",5}, {"Crystal",3}, {"Nanomaterial",2}}, 10.0, 15.0 }},
    {"Railgun Turret",       { {{"Mithril Bar",3}, {"Advanced Engine Parts",1}}, 4.0, 8.0 }},
    {"Shield",               { {{"Titanium Bar",2}, {"Crystal",2}}, 3.0, 6.0 }}
};

// -----------------------------------------------------------------------------
// 2) MODEL CLASSES
// -----------------------------------------------------------------------------

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

class Planet {
public:
    // Each planet now has an energy grid plus a count of defensive items.
    Planet(const string& n, const map<string,double>& prod, bool u)
        : name(n), baseProduction(prod), unlocked(u),
          generators(0), accumulators(0), currentEnergy(0.0), maxEnergy(50.0),
          turrets(0), shields(0)
    {}

    // The planet first “charges” via its generators, then uses available energy for production.
    void produceResources(map<string, Resource>& resources, double elapsedSeconds) {
        if (!unlocked) return;
        double energyProduced = generators * GENERATOR_RATE * elapsedSeconds;
        currentEnergy = min(maxEnergy, currentEnergy + energyProduced);

        double effectiveSeconds = min(elapsedSeconds, currentEnergy / ENERGY_COST_PER_SECOND);
        double energyConsumed = effectiveSeconds * ENERGY_COST_PER_SECOND;
        currentEnergy -= energyConsumed;

        for (auto& kv : baseProduction) {
            const string& resourceName = kv.first;
            double ratePerSec = kv.second;
            double totalProduced = ratePerSec * effectiveSeconds;
            int produceAmt = static_cast<int>(floor(totalProduced));
            if (produceAmt > 0 && resources.find(resourceName) != resources.end()) {
                resources[resourceName].add(produceAmt);
            }
        }
    }

    const string& getName() const { return name; }
    bool isUnlocked() const { return unlocked; }
    void setUnlocked(bool u) { unlocked = u; }

    double getCurrentEnergy() const { return currentEnergy; }
    double getMaxEnergy() const { return maxEnergy; }
    int getGenerators() const { return generators; }
    int getAccumulators() const { return accumulators; }
    int getTurrets() const { return turrets; }
    int getShields() const { return shields; }

    void setCurrentEnergy(double e) { currentEnergy = e; }
    void setMaxEnergy(double e) { maxEnergy = e; }
    void setGenerators(int g) { generators = g; }
    void setAccumulators(int a) { accumulators = a; }
    void setTurrets(int t) { turrets = t; }
    void setShields(int s) { shields = s; }

    // Install a facility or defensive structure.
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
        } else if (fac == "railgun turret") {
            turrets++;
            cout << "Installed a Railgun Turret on " << name 
                 << ". Total turrets: " << turrets << endl;
        } else if (fac == "shield") {
            shields++;
            cout << "Installed a Shield on " << name 
                 << ". Total shields: " << shields << endl;
        } else {
            cout << "Unknown facility: " << facility << endl;
        }
    }

private:
    string name;
    map<string,double> baseProduction;
    bool unlocked;

    int generators;
    int accumulators;
    double currentEnergy;
    double maxEnergy;

    int turrets;
    int shields;

    static constexpr double GENERATOR_RATE = 5.0;        // energy per second per generator
    static constexpr double ENERGY_COST_PER_SECOND = 1.0;  // energy cost per second for mining
};

class Research {
public:
    Research(const string& n, const map<string,int>& c,
             const string& desc, const string& effName)
      : name(n), cost(c), description(desc), effectName(effName), completed(false)
    {}

    bool canResearch(const map<string, Resource>& resources) const {
        if (completed) return false;
        for (auto& kv : cost) {
            const string& rname = kv.first;
            int required = kv.second;
            auto it = resources.find(rname);
            if (it == resources.end() || it->second.getQuantity() < required) {
                return false;
            }
        }
        return true;
    }

    void doResearch(map<string, Resource>& resources) {
        for (auto& kv : cost) {
            resources[kv.first].remove(kv.second);
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

class DailyQuest {
public:
    DailyQuest(const string& d, const string& objRes, int objAmt, const map<string,int>& rew)
        : description(d), objectiveResource(objRes), objectiveAmount(objAmt), reward(rew), completed(false)
    {}

    void checkCompletion(map<string, Resource>& resources) {
        if (completed) return;
        auto it = resources.find(objectiveResource);
        if (it != resources.end() && it->second.getQuantity() >= objectiveAmount) {
            completed = true;
            for (auto& kv : reward) {
                resources[kv.first].add(kv.second);
            }
            cout << "Daily Quest complete! Reward: ";
            for (auto& kv : reward) {
                cout << kv.first << " +" << kv.second << " ";
            }
            cout << endl;
        }
    }

    bool isCompleted() const { return completed; }
    const string& getDescription() const { return description; }
    const string& getObjectiveResource() const { return objectiveResource; }
    int getObjectiveAmount() const { return objectiveAmount; }
    const map<string,int>& getReward() const { return reward; }
private:
    string description;
    string objectiveResource;
    int objectiveAmount;
    map<string,int> reward;
    bool completed;
};

// -----------------------------------------------------------------------------
// STORY QUESTS: New quest chain explaining game mechanics and introducing raids.
// -----------------------------------------------------------------------------

struct StoryQuest {
    string title;
    string narrative;
    bool completed;
    function<bool(const class Player&)> condition;
};

class StoryQuestManager {
public:
    StoryQuestManager() : currentIndex(0) {
        // Quest 1: Tutorial
        StoryQuest q1;
        q1.title = "Welcome to the Frontier";
        q1.narrative =
            "Welcome, Miner! Here you will harvest resources, manage energy, and build your outpost.\n"
            "Use generators to produce energy, accumulators to expand capacity, and craft items to progress.\n"
            "Explore planets and install defenses to protect your assets.";
        q1.completed = false;
        // Automatically complete this quest.
        q1.condition = [](const class Player& p) -> bool { return true; };

        // Quest 2: Fortify Your Outpost
        StoryQuest q2;
        q2.title = "Fortify Your Outpost";
        q2.narrative =
            "Your outpost is vulnerable!\n"
            "Build and install a Railgun Turret and a Shield on any planet to secure your defenses.";
        q2.completed = false;
        q2.condition = [](const class Player& p) -> bool {
            const auto& planets = p.getPlanetManager().getPlanetsConst();
            for (auto& planet : planets) {
                if (planet.getTurrets() >= 1 && planet.getShields() >= 1)
                    return true;
            }
            return false;
        };

        // Quest 3: The Raider Menace
        StoryQuest q3;
        q3.title = "The Raider Menace";
        q3.narrative =
            "Rumors of raider attacks have spread. Prepare yourself—raiders may soon target your planets!\n"
            "Your defenses will be put to the test. Strengthen them and safeguard your resources.";
        q3.completed = false;
        // For simplicity, mark as complete immediately once reached.
        q3.condition = [](const class Player& p) -> bool { return true; };

        storyQuests.push_back(q1);
        storyQuests.push_back(q2);
        storyQuests.push_back(q3);
    }

    void update(const class Player& player) {
        if (currentIndex < storyQuests.size()) {
            StoryQuest& current = storyQuests[currentIndex];
            if (!current.completed && current.condition(player)) {
                current.completed = true;
                cout << "\n--- Story Quest Completed: " << current.title << " ---\n";
                cout << current.narrative << "\n" << endl;
                currentIndex++;
            }
        }
    }

    bool raidsEnabled() const {
        // Enable raids only after quest 2 is complete (i.e. when currentIndex >= 2)
        return currentIndex >= 2;
    }

    void displayCurrentQuest() const {
        if (currentIndex < storyQuests.size()) {
            const StoryQuest& current = storyQuests[currentIndex];
            cout << "Story Quest: " << current.title << "\n" << current.narrative << endl;
        }
    }
private:
    vector<StoryQuest> storyQuests;
    size_t currentIndex;
};

// -----------------------------------------------------------------------------
// MANAGERS
// -----------------------------------------------------------------------------

class ResourceManager {
public:
    ResourceManager(const vector<ResourceDef>& data) {
        for (auto& r : data) {
            resources[r.name] = Resource(r.name, r.initial);
        }
    }

    void addResource(const string& name, int amount) {
        resources[name].add(amount);
    }

    bool removeResource(const string& name, int amount) {
        auto it = resources.find(name);
        if (it == resources.end()) {
            cerr << "Warning: Resource " << name << " not found (cannot remove)." << endl;
            return false;
        }
        return it->second.remove(amount);
    }

    int getQuantity(const string& name) const {
        auto it = resources.find(name);
        if (it != resources.end()) {
            return it->second.getQuantity();
        }
        return 0;
    }

    map<string, Resource>& getAllResources() { return resources; }
    const map<string, Resource>& getAllResourcesConst() const { return resources; }
private:
    map<string, Resource> resources;
};

class PlanetManager {
public:
    PlanetManager(const vector<PlanetDef>& data) {
        for (auto& p : data) {
            planets.emplace_back(p.name, p.baseProduction, p.unlocked);
        }
    }

    void produceAll(map<string, Resource>& resources, double elapsedSeconds) {
        for (auto& planet : planets) {
            planet.produceResources(resources, elapsedSeconds);
        }
    }

    void unlockPlanet(const string& planetName) {
        for (auto& p : planets) {
            if (p.getName() == planetName) {
                p.setUnlocked(true);
            }
        }
    }

    Planet* getPlanetByName(const string& planetName) {
        for (auto& p : planets) {
            if (p.getName() == planetName) {
                return &p;
            }
        }
        return nullptr;
    }

    vector<Planet>& getPlanets() { return planets; }
    const vector<Planet>& getPlanetsConst() const { return planets; }
private:
    vector<Planet> planets;
};

class ResearchManager {
public:
    ResearchManager(const vector<ResearchDef>& data) {
        for (auto& rd : data) {
            researches.emplace_back(rd.name, rd.cost, rd.description, rd.effectName);
        }
    }

    Research* findResearchByName(const string& name) {
        for (auto& r : researches) {
            string n = r.getName();
            transform(n.begin(), n.end(), n.begin(), ::tolower);
            string searchName = name;
            transform(searchName.begin(), searchName.end(), searchName.begin(), ::tolower);
            if (n == searchName) {
                return &r;
            }
        }
        return nullptr;
    }

    vector<Research>& getAllResearches() { return researches; }
    const vector<Research>& getAllResearchesConst() const { return researches; }
private:
    vector<Research> researches;
};

class CraftingManager {
public:
    CraftingManager(const map<string, CraftingRecipe>& data)
        : recipes(data)
    {}

    // costMultiplier reduces energy cost (Crafting Mastery)
    // precisionEnabled gives a 10% chance to yield bonus output (Precision Tools)
    void craft(const string& outputItem,
               ResourceManager& resourceManager,
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
        for (auto& kv : it->second.inputs) {
            if (resourceManager.getQuantity(kv.first) < kv.second) {
                cout << "Not enough " << kv.first << " to craft " << outputItem << "." << endl;
                return;
            }
        }
        double effectiveCost = it->second.electricityCost * costMultiplier;
        if (planet == nullptr) {
            cout << "Invalid planet for crafting." << endl;
            return;
        }
        if (planet->getCurrentEnergy() < effectiveCost) {
            cout << "Not enough energy on planet " << planet->getName() << " to craft " << outputItem << "." << endl;
            return;
        }
        planet->setCurrentEnergy(planet->getCurrentEnergy() - effectiveCost);
        for (auto& kv : it->second.inputs) {
            resourceManager.removeResource(kv.first, kv.second);
        }
        resourceManager.addResource(outputItem, 1);
        cout << "Crafted 1 " << outputItem << " on planet " << planet->getName() 
             << " using { ";
        for (auto& kv : it->second.inputs) {
            cout << kv.first << ":" << kv.second << " ";
        }
        cout << "} and consuming " << effectiveCost << " energy." << endl;
        if (precisionEnabled && (rand() % 100) < 10) {
            resourceManager.addResource(outputItem, 1);
            cout << "Precision Tools activated! Bonus " << outputItem << " produced!" << endl;
        }
    }
private:
    map<string, CraftingRecipe> recipes;
};

class QuestManager {
public:
    QuestManager() : currentQuest(nullptr), lastQuestDate("") {}

    void updateDailyQuest(map<string, Resource>& resources) {
        string todayStr = currentDateString();
        if (lastQuestDate.empty() || lastQuestDate < todayStr) {
            generateNewQuest();
            lastQuestDate = todayStr;
        }
        if (currentQuest) {
            currentQuest->checkCompletion(resources);
        }
    }

    DailyQuest* getCurrentQuest() {
        return currentQuest.get();
    }
    const DailyQuest* getCurrentQuestConst() const {
        return currentQuest.get();
    }
    const string& getLastQuestDate() const { return lastQuestDate; }
    void setLastQuestDate(const string& s) { lastQuestDate = s; }
    void setCurrentQuest(unique_ptr<DailyQuest> q) {
        currentQuest = std::move(q);
    }
private:
    unique_ptr<DailyQuest> currentQuest;
    string lastQuestDate;

    void generateNewQuest() {
        vector<string> resourceChoices = {"Iron Bar", "Copper Bar", "Mithril Bar", "Titanium Bar", "Advanced Engine Parts"};
        int idx = rand() % resourceChoices.size();
        string chosenResource = resourceChoices[idx];
        int objective_amount = rand() % 41 + 10; // 10 to 50
        map<string,int> reward = { {"Engine Parts", (rand() % 3) + 1} };
        string desc = "Collect " + to_string(objective_amount) + " " + chosenResource;
        currentQuest = make_unique<DailyQuest>(desc, chosenResource, objective_amount, reward);
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

class Player {
public:
    Player()
        : resourceManager(RESOURCE_DATA),
          planetManager(PLANET_DATA),
          researchManager(RESEARCH_DATA),
          craftingManager(CRAFTING_RECIPES),
          fasterCraftingMultiplier(1.0),
          craftingCostMultiplier(1.0),
          precisionToolsEnabled(false),
          lastUpdateTime(std::time(nullptr))
    {
        srand(static_cast<unsigned>(time(nullptr)));
    }

    ResourceManager& getResourceManager() { return resourceManager; }
    PlanetManager& getPlanetManager() { return planetManager; }
    ResearchManager& getResearchManager() { return researchManager; }
    CraftingManager& getCraftingManager() { return craftingManager; }
    QuestManager& getQuestManager() { return questManager; }
    const ResourceManager& getResourceManager() const { return resourceManager; }
    const PlanetManager& getPlanetManager() const { return planetManager; }
    const ResearchManager& getResearchManager() const { return researchManager; }
    const CraftingManager& getCraftingManager() const { return craftingManager; }
    const QuestManager& getQuestManager() const { return questManager; }

    double getFasterCraftingMultiplier() const { return fasterCraftingMultiplier; }
    double getCraftingCostMultiplier() const { return craftingCostMultiplier; }
    bool isPrecisionToolsEnabled() const { return precisionToolsEnabled; }

    void setFasterCraftingMultiplier(double val) { fasterCraftingMultiplier = val; }
    void setCraftingCostMultiplier(double val) { craftingCostMultiplier = val; }
    void setPrecisionToolsEnabled(bool val) { precisionToolsEnabled = val; }

    void produceResources() {
        time_t now = time(nullptr);
        double diffSeconds = difftime(now, lastUpdateTime);
        if (diffSeconds < 0) diffSeconds = 0;
        planetManager.produceAll(resourceManager.getAllResources(), diffSeconds);
        lastUpdateTime = now;
    }

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
        if (r->canResearch(resourceManager.getAllResourcesConst())) {
            r->doResearch(resourceManager.getAllResources());
            applyResearchEffect(r->getEffectName());
            cout << "Research '" << r->getName() << "' completed!" << endl;
        } else {
            cout << "Not enough resources to start this research." << endl;
        }
    }

    // Craft on a specified planet.
    void craftItem(const string& itemName, const string& planetName = "Terra") {
        Planet* p = planetManager.getPlanetByName(planetName);
        if (!p) {
            cout << "Planet " << planetName << " not found." << endl;
            return;
        }
        craftingManager.craft(itemName, resourceManager, p, fasterCraftingMultiplier,
                              craftingCostMultiplier, precisionToolsEnabled);
    }

    // Install a facility or defensive structure.
    void installFacility(const string& facility, const string& planetName) {
        Planet* p = planetManager.getPlanetByName(planetName);
        if (!p) {
            cout << "Planet " << planetName << " not found." << endl;
            return;
        }
        if (resourceManager.getQuantity(facility) > 0) {
            resourceManager.removeResource(facility, 1);
            p->installFacility(facility);
            cout << facility << " installed on " << planetName << "." << endl;
        } else {
            cout << "You do not have any " << facility << " to install." << endl;
        }
    }

    void updateDailyQuest() {
        questManager.updateDailyQuest(resourceManager.getAllResources());
    }

    time_t getLastUpdateTime() const { return lastUpdateTime; }
    void setLastUpdateTime(time_t t) { lastUpdateTime = t; }

    // Recalculate multipliers from completed research.
    void recalcUpgradesFromResearch() {
        fasterCraftingMultiplier = 1.0;
        craftingCostMultiplier = 1.0;
        precisionToolsEnabled = false;
        auto& researches = researchManager.getAllResearches();
        for (auto& r : researches) {
            if (r.isCompleted()) {
                string effect = r.getEffectName();
                if (effect == "faster_crafting")
                    fasterCraftingMultiplier = 0.5;
                else if (effect == "crafting_mastery")
                    craftingCostMultiplier = 0.8;
                else if (effect == "precision_tools")
                    precisionToolsEnabled = true;
                else if (effect == "unlock_luna")
                    planetManager.unlockPlanet("Luna");
            }
        }
    }
private:
    ResourceManager resourceManager;
    PlanetManager planetManager;
    ResearchManager researchManager;
    CraftingManager craftingManager;
    QuestManager questManager;
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
// RAIDER EVENT SIMULATION
// -----------------------------------------------------------------------------

// Simulate a raid on a random unlocked planet.
void simulateRaid(Player& player) {
    auto& planets = player.getPlanetManager().getPlanets();
    vector<Planet*> candidates;
    for (auto& planet : planets) {
        if (planet.isUnlocked()) {
            candidates.push_back(&planet);
        }
    }
    if (candidates.empty()) return;
    Planet* target = candidates[rand() % candidates.size()];
    int defenseRating = target->getTurrets() * 20 + target->getShields() * 15;
    int raiderStrength = rand() % 51 + 10; // 10 to 60
    cout << "\n--- RAID ALERT on " << target->getName() << " ---\n";
    cout << "Your defenses (Rating " << defenseRating << ") vs Raider strength (" << raiderStrength << ")" << endl;
    if (defenseRating >= raiderStrength) {
        cout << "Raid repelled! Your defenses held strong.\n" << endl;
    } else {
        cout << "Raid successful! Raiders have plundered your resources and damaged your defenses.\n" << endl;
        // Steal 20% of each resource (except facilities/defenses)
        auto& res = player.getResourceManager().getAllResources();
        for (auto& kv : res) {
            string name = kv.first;
            if (name == "Generator" || name == "Accumulator" ||
                name == "Railgun Turret" || name == "Shield")
                continue;
            int current = kv.second.getQuantity();
            int loss = current / 5; // 20%
            if (loss > 0) {
                kv.second.remove(loss);
                cout << loss << " " << name << " stolen." << endl;
            }
        }
        // Damage defenses: remove one turret and one shield if available.
        if (target->getTurrets() > 0) {
            target->setTurrets(target->getTurrets() - 1);
            cout << "One Railgun Turret was destroyed." << endl;
        }
        if (target->getShields() > 0) {
            target->setShields(target->getShields() - 1);
            cout << "One Shield was destroyed." << endl;
        }
    }
}

// -----------------------------------------------------------------------------
// SAVE/LOAD FUNCTIONS
// -----------------------------------------------------------------------------

void saveGame(const Player& player) {
    json j;
    {
        json jr = json::object();
        const auto& allRes = player.getResourceManager().getAllResourcesConst();
        for (auto& kv : allRes) {
            jr[kv.first] = kv.second.getQuantity();
        }
        j["resources"] = jr;
    }
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
            pObj["turrets"] = p.getTurrets();
            pObj["shields"] = p.getShields();
            jp.push_back(pObj);
        }
        j["planets"] = jp;
    }
    {
        json jrch = json::array();
        const auto& rAll = player.getResearchManager().getAllResearchesConst();
        for (auto& r : rAll) {
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
    {
        json questJson = nullptr;
        const auto& questMgr = player.getQuestManager();
        const DailyQuest* cq = questMgr.getCurrentQuestConst();
        if (cq) {
            questJson = json::object();
            questJson["description"] = cq->getDescription();
            questJson["objective_resource"] = cq->getObjectiveResource();
            questJson["objective_amount"] = cq->getObjectiveAmount();
            json rew = json::object();
            for (auto& kv : cq->getReward()) {
                rew[kv.first] = kv.second;
            }
            questJson["reward"] = rew;
            questJson["completed"] = cq->isCompleted();
        }
        j["current_quest"] = questJson;
        j["last_quest_date"] = player.getQuestManager().getLastQuestDate();
    }
    j["last_update_time"] = static_cast<long long>(player.getLastUpdateTime());
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
    if (!ifs.is_open()) {
        return;
    }
    json j;
    ifs >> j;
    ifs.close();
    if (j.contains("resources") && j["resources"].is_object()) {
        for (auto& kv : j["resources"].items()) {
            string rname = kv.key();
            int qty = kv.value().get<int>();
            int currentQty = player.getResourceManager().getQuantity(rname);
            int diff = qty - currentQty;
            if (diff > 0) player.getResourceManager().addResource(rname, diff);
            else if (diff < 0) player.getResourceManager().removeResource(rname, -diff);
        }
    }
    if (j.contains("planets") && j["planets"].is_array()) {
        auto& plist = player.getPlanetManager().getPlanets();
        for (auto& pj : j["planets"]) {
            string pname = pj["name"].get<string>();
            bool unlocked = pj["unlocked"].get<bool>();
            for (auto& p : plist) {
                if (p.getName() == pname) {
                    p.setUnlocked(unlocked);
                    if (pj.contains("current_energy"))
                        p.setCurrentEnergy(pj["current_energy"].get<double>());
                    if (pj.contains("max_energy"))
                        p.setMaxEnergy(pj["max_energy"].get<double>());
                    if (pj.contains("generators"))
                        p.setGenerators(pj["generators"].get<int>());
                    if (pj.contains("accumulators"))
                        p.setAccumulators(pj["accumulators"].get<int>());
                    if (pj.contains("turrets"))
                        p.setTurrets(pj["turrets"].get<int>());
                    if (pj.contains("shields"))
                        p.setShields(pj["shields"].get<int>());
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
            if (res) res->setCompleted(completed);
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
        auto rewJson = questObj["reward"];
        map<string,int> rew;
        for (auto& kv : rewJson.items()) {
            rew[kv.key()] = kv.value().get<int>();
        }
        bool completed = questObj["completed"].get<bool>();
        auto dq = make_unique<DailyQuest>(desc, oRes, oAmt, rew);
        player.getQuestManager().setCurrentQuest(std::move(dq));
        if (completed) {
            auto& allRes = player.getResourceManager().getAllResources();
            player.getQuestManager().getCurrentQuest()->checkCompletion(allRes);
        }
    }
    if (j.contains("last_quest_date") && !j["last_quest_date"].is_null()) {
        string lastDate = j["last_quest_date"].get<string>();
        player.getQuestManager().setLastQuestDate(lastDate);
    }
    if (j.contains("last_update_time")) {
        long long storedTime = j["last_update_time"].get<long long>();
        player.setLastUpdateTime(static_cast<time_t>(storedTime));
    }
    player.recalcUpgradesFromResearch();
    cout << "Game loaded from " << SAVE_FILE << "." << endl;
}

// -----------------------------------------------------------------------------
// MAIN GAME LOOP
// -----------------------------------------------------------------------------

void showHelp() {
    cout << "Commands:\n"
         << "  help                           - Show this help\n"
         << "  stats                          - Show current resources\n"
         << "  planets                        - Show planet status (energy, facilities, defenses)\n"
         << "  research                       - List all research items\n"
         << "  do_research <name>             - Attempt a research by name\n"
         << "  craft <item> [on <planet>]     - Craft an item (default planet: Terra)\n"
         << "  install <facility> on <planet> - Install a facility/defense (e.g., Generator, Accumulator, Railgun Turret, Shield)\n"
         << "  daily                          - Show the current daily quest\n"
         << "  save                           - Save game\n"
         << "  quit                           - Quit (autosave)\n";
}

int main() {
    Player player;
    loadGame(player);
    StoryQuestManager storyQuestManager;
    cout << "Welcome to the Enhanced Modular Idle Planet Miner!" << endl;
    cout << "Type 'help' for commands." << endl;
    
    // For raid simulation timing.
    time_t lastRaidTime = time(nullptr);
    
    while (true) {
        player.produceResources();
        player.updateDailyQuest();
        storyQuestManager.update(player);

        cout << "> ";
        string command;
        if (!getline(cin, command)) break;
        if (command.empty()) continue;
        auto spacePos = command.find(' ');
        string cmd = (spacePos == string::npos) ? command : command.substr(0, spacePos);
        string arg = (spacePos == string::npos) ? "" : command.substr(spacePos+1);
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

        if (cmd == "help") {
            showHelp();
        }
        else if (cmd == "stats") {
            const auto& allRes = player.getResourceManager().getAllResourcesConst();
            for (auto& kv : allRes) {
                cout << kv.first << ": " << kv.second.getQuantity() << endl;
            }
        }
        else if (cmd == "planets") {
            const auto& plist = player.getPlanetManager().getPlanetsConst();
            for (auto& p : plist) {
                cout << p.getName() << " => " << (p.isUnlocked() ? "Unlocked" : "Locked")
                     << " | Energy: " << p.getCurrentEnergy() << "/" << p.getMaxEnergy()
                     << " | Generators: " << p.getGenerators() << ", Accumulators: " << p.getAccumulators()
                     << " | Turrets: " << p.getTurrets() << ", Shields: " << p.getShields() << endl;
            }
        }
        else if (cmd == "research") {
            const auto& rAll = player.getResearchManager().getAllResearchesConst();
            for (auto& r : rAll) {
                cout << r.getName() << " - " 
                     << (r.isCompleted() ? "Completed" : "Not Completed")
                     << " | Cost: ";
                for (auto& cost : r.getCost()) {
                    cout << cost.first << ":" << cost.second << " ";
                }
                cout << "| " << r.getDescription() << endl;
            }
        }
        else if (cmd == "do_research") {
            if (arg.empty())
                cout << "Usage: do_research <research name>" << endl;
            else
                player.doResearch(arg);
        }
        else if (cmd == "craft") {
            string itemName, planetName;
            size_t pos = arg.find(" on ");
            if (pos != string::npos) {
                itemName = arg.substr(0, pos);
                planetName = arg.substr(pos + 4);
            } else {
                itemName = arg;
                planetName = "Terra";
            }
            if (itemName.empty())
                cout << "Usage: craft <item> [on <planet>]" << endl;
            else
                player.craftItem(itemName, planetName);
        }
        else if (cmd == "install") {
            size_t pos = arg.find(" on ");
            if (pos != string::npos) {
                string facility = arg.substr(0, pos);
                string planetName = arg.substr(pos + 4);
                if (facility.empty() || planetName.empty())
                    cout << "Usage: install <facility> on <planet>" << endl;
                else
                    player.installFacility(facility, planetName);
            } else {
                cout << "Usage: install <facility> on <planet>" << endl;
            }
        }
        else if (cmd == "daily") {
            auto* cq = player.getQuestManager().getCurrentQuest();
            if (cq) {
                cout << cq->getDescription() << endl
                     << "Objective: " << cq->getObjectiveAmount() << " " << cq->getObjectiveResource() << endl
                     << "Completed: " << (cq->isCompleted() ? "Yes" : "No") << endl;
            } else {
                cout << "No current daily quest assigned." << endl;
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
        else {
            cout << "Unknown command. Type 'help' for a list of commands." << endl;
        }
        
        // Raid simulation: if story quests indicate raids are enabled and at least 30 seconds have passed.
        if (storyQuestManager.raidsEnabled() && (time(nullptr) - lastRaidTime >= 30)) {
            if ((rand() % 100) < 30) { // 30% chance to trigger a raid event.
                simulateRaid(player);
            }
            lastRaidTime = time(nullptr);
        }
    }
    return 0;
}
