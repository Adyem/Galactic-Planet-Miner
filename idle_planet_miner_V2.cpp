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
#include <chrono>
#include <thread>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

//---------------------------------------------------------------------
// Global Lore Strings for Quest Dialogue
//---------------------------------------------------------------------
static const vector<string> resourceLore = {
    "Old Miner Joe: 'The veins of our beloved planet run deep—every ounce of ore fuels our future.'",
    "Professor Lumen: 'Mining is the backbone of our civilization. Without its bounty, progress would stall.'",
    "Farmer Daisy: 'Though I tend my fields, I know that without the mines, our tables would be empty.'"
};

static const vector<string> raiderLore = {
    "Captain Blackthorne: 'These raiders aren’t mere bandits—they’re desperate souls preying on our lifelines.'",
    "Navigator Zara: 'Our convoys are the arteries of our colonies. Raiders strike to choke that flow.'",
    "Old Scout Finn: 'I’ve seen raider ships lurking in the void. Their attacks leave nothing but wreckage and sorrow.'"
};

static const vector<string> genericLore = {
    "Town Crier: 'The times are hard, but unity and perseverance will see us through.'",
    "Scribe Alric: 'Every day brings new challenges—and new opportunities—for those brave enough to seize them.'"
};

//---------------------------------------------------------------------
// Define a constant for the save file.
//---------------------------------------------------------------------
const string SAVE_FILE = "savegame.json";

//---------------------------------------------------------------------
// 1) DATA DEFINITIONS
//---------------------------------------------------------------------

// Resource definitions.
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

// Planet definitions.
struct PlanetDef {
    string name;
    map<string, double> baseProduction;
    bool unlocked;
};

static vector<PlanetDef> PLANET_DATA = {
    { "Terra", {{"Iron", 0.5}, {"Copper", 0.5}, {"Coal", 0.2}}, true },
    { "Mars", {{"Iron", 0.1}, {"Copper", 0.1}, {"Mithril", 0.05}, {"Coal", 0.1}}, false },
    { "Zalthor", {{"Mithril", 0.1}, {"Coal", 0.2}, {"Gold", 0.02}}, false },
    { "Vulcan", {{"Tin", 0.1}, {"Silver", 0.03}, {"Titanium", 0.01}}, false },
    { "Luna", {{"Obsidian", 0.1}, {"Crystal", 0.05}, {"Nanomaterial", 0.02}}, false }
};

// Research definitions.
struct ResearchDef {
    string name;
    map<string,int> cost;
    string description;
    string effectName;
};

static vector<ResearchDef> RESEARCH_DATA = {
    { "Unlock Mars", {{"Iron",100}, {"Copper",50}}, "Unlock the planet Mars for mining.", "unlock_mars" },
    { "Unlock Zalthor", {{"Iron",200}, {"Mithril",50}}, "Unlock planet Zalthor for advanced mining.", "unlock_zalthor" },
    { "Faster Crafting", {{"Iron",50}, {"Coal",30}}, "Reduce smelting time by half.", "faster_crafting" },
    { "Unlock Vulcan", {{"Gold",100}, {"Mithril",100}}, "Unlock planet Vulcan for high‑value resources.", "unlock_vulcan" },
    { "Unlock Luna", {{"Gold",150}, {"Mithril",100}}, "Unlock planet Luna to harvest exotic materials.", "unlock_luna" },
    { "Crafting Mastery", {{"Engine Parts",5}, {"Titanium Bar",3}}, "Reduces crafting energy cost by 20%.", "crafting_mastery" },
    { "Precision Tools", {{"Iron Bar",10}, {"Mithril Bar",5}}, "Gives a 10% chance to double crafted output.", "precision_tools" },
    { "Shield Technology", {{"Mithril Bar",10}, {"Titanium Bar",5}}, "Allows building Shield Ships to protect convoys.", "unlock_shield_ships" },
    { "Emergency Energy Conservation", {{"Engine Parts",10}, {"Titanium Bar",5}}, "Allows a planet to halt production during an imminent raider attack, conserving energy for defense.", "energy_conservation" },
    { "Repair Drone Technology", {{"Fusion Reactor",1}, {"Advanced Engine Parts",3}}, "Unlocks Repair Drones that can repair ships during combat.", "unlock_repair_drones" }
};

// Crafting recipes.
struct CraftingRecipe {
    map<string,int> inputs;
    double timeRequired;
    double electricityCost;
    string requiredBuilding;
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
    {"Corvette",             { {{"Titanium Bar",3}, {"Advanced Engine Parts",1}}, 5.0, 15.0, "Shipyard" }},
    {"Shield Ship",          { {{"Titanium Bar",4}, {"Advanced Engine Parts",1}}, 6.0, 25.0, "Shipyard" }},
    {"Radar Ship",           { {{"Mithril Bar",2}, {"Engine Parts",1}}, 4.0, 20.0, "Shipyard" }},
    {"Salvage Ship",         { {{"Iron Bar",3}, {"Engine Parts",2}}, 4.0, 15.0, "Shipyard" }},
    {"Repair Drone",         { {{"Fusion Reactor",1}, {"Advanced Engine Parts",2}}, 8.0, 30.0, "Shipyard" }}
};

// Building recipes.
struct BuildingRecipe {
    map<string,int> inputs;
    double timeRequired;
    double electricityCost;
};

static map<string, BuildingRecipe> BUILDING_RECIPES = {
    {"Crafting Building",   { {{"Iron Bar", 5}, {"Engine Parts", 2}}, 5.0, 10.0 }},
    {"Smelting Building",   { {{"Copper Bar", 5}, {"Coal", 10}}, 5.0, 10.0 }},
    {"Facility Workshop",   { {{"Generator", 2}, {"Accumulator", 2}}, 3.0, 5.0 }},
    {"Shipyard",            { {{"Iron Bar", 10}, {"Engine Parts", 5}}, 8.0, 20.0 }},
    {"Proximity Alarm",     { {{"Crystal",2}, {"Mithril Bar",1}}, 3.0, 5.0 }},
    {"Proximity Radar",     { {{"Crystal",2}, {"Mithril Bar",1}}, 3.0, 5.0 }},
    {"Mobile Radar",        { {{"Crystal",3}, {"Copper Bar",2}}, 3.0, 5.0 }},
    {"Salvage Robot",       { {{"Obsidian",2}, {"Engine Parts",1}}, 3.0, 5.0 }},
    {"Shield Generator",    { {{"Titanium Bar",2}, {"Copper Bar",2}}, 4.0, 5.0 }}
};

//---------------------------------------------------------------------
// Forward declarations to resolve compilation errors
//---------------------------------------------------------------------
struct Ship;  // already defined below
class PlanetManager;  // defined below

//---------------------------------------------------------------------
// 2A) SHIP STRUCT
//---------------------------------------------------------------------
struct Ship {
    string type; // "Transport Vessel", "Corvette", "Shield Ship", "Radar Ship", "Salvage Ship", "Repair Drone"
    int hull;
    int maxShield;
    int currentShield;
    int weapons; // For offensive ships
    int repairAmount; // Only used for Repair Drones (e.g., 10 HP per turn)
};

//---------------------------------------------------------------------
// 2B) RESOURCE CLASS (already defined above)
//---------------------------------------------------------------------
// (see above)

//---------------------------------------------------------------------
// 2C) PLANET CLASS (updated with building plots and threat flag)
//---------------------------------------------------------------------
class Planet {
public:
    Planet(const string& n, const map<string,double>& prod, bool u)
        : name(n), baseProduction(prod), unlocked(u),
          generators(0), accumulators(0),
          currentEnergy(0.0), maxEnergy(50.0),
          maxBuildingPlots(3), currentBuildingCount(0),
          hasUpgradedPlots(false), underThreat(false)
    {
        for(auto &res : RESOURCE_DATA)
            storage[res.name] = 0;
    }
    void produceResources(double elapsedSeconds, bool energyConservationEnabled) {
        if(underThreat && energyConservationEnabled && hasBuilding("Shield Generator")){
            cout << "Production on " << name << " halted due to Emergency Energy Conservation." << endl;
            return;
        }
        if(!unlocked) return;
        double energyProduced = generators * GENERATOR_RATE * elapsedSeconds;
        currentEnergy = min(maxEnergy, currentEnergy + energyProduced);
        double effectiveSeconds = min(elapsedSeconds, currentEnergy / ENERGY_COST_PER_SECOND);
        double energyConsumed = effectiveSeconds * ENERGY_COST_PER_SECOND;
        currentEnergy -= energyConsumed;
        for(auto &kv : baseProduction){
            int produced = static_cast<int>(floor(kv.second * effectiveSeconds));
            storage[kv.first] += produced;
        }
    }
    bool canBuildMore() const { return currentBuildingCount < maxBuildingPlots; }
    bool upgradePlots(){
        if(!hasUpgradedPlots){
            hasUpgradedPlots = true;
            maxBuildingPlots += 2;
            return true;
        }
        return false;
    }
    bool addBuilding(const string& buildingName) {
        if(!canBuildMore()){
            cout << "No building plots available on " << name << ". Upgrade building capacity first." << endl;
            return false;
        }
        buildings[buildingName]++;
        currentBuildingCount++;
        return true;
    }
    bool hasBuilding(const string& buildingName) const {
        auto it = buildings.find(buildingName);
        return (it != buildings.end() && it->second > 0);
    }
    const map<string,int>& getBuildings() const { return buildings; }
    void addToStorage(const string& resourceName, int amount) { storage[resourceName] += amount; }
    bool removeFromStorage(const string& resourceName, int amount) {
        if(storage[resourceName] >= amount){ storage[resourceName] -= amount; return true; }
        return false;
    }
    int getStored(const string& resourceName) const {
        auto it = storage.find(resourceName);
        return (it != storage.end()) ? it->second : 0;
    }
    const map<string,int>& getStorageMap() const { return storage; }
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
    void installFacility(const string& facility) {
        string fac = facility; transform(fac.begin(), fac.end(), fac.begin(), ::tolower);
        if(fac == "generator") { generators++; cout << "Installed a Generator on " << name << ". Total: " << generators << endl; }
        else if(fac == "accumulator") { accumulators++; maxEnergy += 50.0; cout << "Installed an Accumulator on " << name << ". Total: " << accumulators << ", new max energy: " << maxEnergy << endl; }
        else { cout << "Unknown facility: " << facility << endl; }
    }
    bool isUnderThreat() const { return underThreat; }
    void setUnderThreat(bool flag) { underThreat = flag; }
    int getMaxBuildingPlots() const { return maxBuildingPlots; }
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
    int maxBuildingPlots;
    int currentBuildingCount;
    bool hasUpgradedPlots;
    bool underThreat;
    static constexpr double GENERATOR_RATE = 5.0;
    static constexpr double ENERGY_COST_PER_SECOND = 1.0;
};

//---------------------------------------------------------------------
// 2D) PLANET MANAGER
//---------------------------------------------------------------------
class PlanetManager {
public:
    PlanetManager(const vector<PlanetDef>& data) {
        for(auto& p : data)
            planets.push_back(Planet(p.name, p.baseProduction, p.unlocked));
    }
    void produceAll(double elapsedSeconds, bool energyConservationEnabled) {
        for(auto& p : planets)
            p.produceResources(elapsedSeconds, energyConservationEnabled);
    }
    void unlockPlanet(const string& planetName) {
        for(auto& p : planets)
            if(p.getName() == planetName)
                p.setUnlocked(true);
    }
    Planet* getPlanetByName(const string& planetName) {
        for(auto& p : planets)
            if(p.getName() == planetName)
                return &p;
        return nullptr;
    }
    const vector<Planet>& getPlanetsConst() const { return planets; }
    vector<Planet>& getPlanets() { return planets; }
private:
    vector<Planet> planets;
};

//---------------------------------------------------------------------
// 2E) RESEARCH CLASS
//---------------------------------------------------------------------
class Research {
public:
    Research(const string& n, const map<string,int>& c, const string& desc, const string& effName)
      : name(n), cost(c), description(desc), effectName(effName), completed(false) {}
    bool canResearch(Planet* central) const {
        for(auto &kv : cost)
            if(central->getStored(kv.first) < kv.second)
                return false;
        return true;
    }
    void doResearch(Planet* central) {
        for(auto &kv : cost)
            central->removeFromStorage(kv.first, kv.second);
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

//---------------------------------------------------------------------
// 2F) DAILY QUEST CLASS
//---------------------------------------------------------------------
class DailyQuest {
public:
    DailyQuest(const string& d, const string& objRes, int objAmt, const map<string,int>& rew)
        : description(d), objectiveResource(objRes), objectiveAmount(objAmt),
          reward(rew), completed(false), isRaiderAttack(false), targetPlanet(""), combatStartTime(0), turnsElapsed(0)
    {}
    void checkCompletion(Planet* central) {
        if(completed || isRaiderAttack) return;
        if(central->getStored(objectiveResource) >= objectiveAmount){
            completed = true;
            for(auto &kv : reward)
                central->addToStorage(kv.first, kv.second);
            cout << "Quest complete! Reward: ";
            for(auto &kv : reward)
                cout << kv.first << " +" << kv.second << " ";
            cout << endl;
            cout << "\nLore: " << resourceLore[rand() % resourceLore.size()] << endl;
        }
    }
    // Now the combat lasts up to 10 turns (or until one party is destroyed).
    // Each turn is one real minute if in active mode; if idle, resolve immediately.
    // A repair drone (if present) repairs a chosen ship each turn.
    bool processRaiderAttack(PlanetManager& pm, vector<Ship>& fleet, bool realTime) {
        if(completed || !isRaiderAttack) return false;
        Planet* target = pm.getPlanetByName(targetPlanet);
        if(!target){ cout << "Target planet " << targetPlanet << " not found." << endl; return false; }
        // Record combat start time if not already done.
        if(combatStartTime == 0)
            combatStartTime = time(nullptr);
        int raiderShield = rand() % 101 + 100;
        int raiderHull = rand() % 201 + 300;
        cout << "Commencing raider battle at " << targetPlanet << "!" << endl;
        const int maxTurns = 10;
        int turn = 0;
        while(turn < maxTurns && !fleet.empty() && raiderHull > 0){
            turn++;
            cout << "\n--- Turn " << turn << " ---" << endl;
            // If in active real-time mode, wait 60 seconds (simulate 1 minute);
            // otherwise, if idle, simulate instantly.
            if(realTime) {
                cout << "Waiting 60 seconds for next combat turn..." << endl;
                this_thread::sleep_for(chrono::seconds(60));
            }
            // Defenders fire.
            int totalDamage = 0;
            for(auto &ship : fleet){
                if(ship.type == "Corvette" || ship.type == "Shield Ship" || ship.type == "Radar Ship" || ship.type == "Salvage Ship")
                    totalDamage += ship.weapons;
            }
            cout << "Your defenders fire for a total of " << totalDamage << " damage." << endl;
            int shieldDamage = min(totalDamage, raiderShield);
            raiderShield -= shieldDamage;
            int remainingDamage = totalDamage - shieldDamage;
            raiderHull -= remainingDamage;
            cout << "Raider shields take " << shieldDamage << " damage, remaining: " << raiderShield << endl;
            if(remainingDamage > 0)
                cout << "Raider hull takes " << remainingDamage << " damage, remaining: " << raiderHull << endl;
            if(raiderHull <= 0){
                cout << "Raiders defeated!" << endl;
                Planet* terra = pm.getPlanetByName("Terra");
                if(terra){ terra->addToStorage("Engine Parts", 2); cout << "You receive 2 Engine Parts as reward." << endl; }
                completed = true;
                cout << "\nLore: " << raiderLore[rand() % raiderLore.size()] << endl;
                target->setUnderThreat(false);
                break;
            }
            // Raiders fire.
            int raiderDamage = rand() % 51 + 50;
            cout << "Raiders fire for " << raiderDamage << " damage." << endl;
            // Shield Generators on target provide bonus shield.
            int numShieldGen = 0;
            auto blds = target->getBuildings();
            if(blds.find("Shield Generator") != blds.end())
                numShieldGen = blds.at("Shield Generator");
            int shieldBonus = 0;
            for(int i = 0; i < numShieldGen; i++){
                if(target->getCurrentEnergy() >= 5){
                    shieldBonus += 30;
                    target->setCurrentEnergy(target->getCurrentEnergy() - 5);
                }
            }
            cout << "Shield Generators provide " << shieldBonus << " shield bonus." << endl;
            int effectiveDamage = max(0, raiderDamage - shieldBonus);
            // Distribute effective damage evenly among defending ships.
            vector<int> indices;
            for(size_t i = 0; i < fleet.size(); i++){
                if(fleet[i].type == "Corvette" || fleet[i].type == "Shield Ship" ||
                   fleet[i].type == "Radar Ship" || fleet[i].type == "Salvage Ship")
                    indices.push_back(i);
            }
            if(!indices.empty()){
                int dmgPerShip = effectiveDamage / indices.size();
                for(int idx : indices){
                    Ship &s = fleet[idx];
                    if(s.currentShield >= dmgPerShip){
                        s.currentShield -= dmgPerShip;
                        cout << s.type << " takes " << dmgPerShip << " damage to shield, remaining shield: " << s.currentShield << endl;
                    } else {
                        int rem = dmgPerShip - s.currentShield;
                        s.currentShield = 0;
                        s.hull -= rem;
                        cout << s.type << " shield depleted; takes " << rem << " hull damage, remaining hull: " << s.hull << endl;
                    }
                }
                fleet.erase(remove_if(fleet.begin(), fleet.end(), [](const Ship &s){ return s.hull <= 0; }), fleet.end());
            }
            if(fleet.empty()){
                cout << "All defending ships have been destroyed!" << endl;
                completed = true;
                cout << "\nLore: " << raiderLore[rand() % raiderLore.size()] << endl;
                target->setUnderThreat(false);
                break;
            }
            // Process Repair Drones.
            // For each Repair Drone in the fleet, choose the non-drone ship with lowest hull ratio.
            vector<Ship*> repairDrones;
            for(auto &ship : fleet)
                if(ship.type == "Repair Drone")
                    repairDrones.push_back(&ship);
            if(!repairDrones.empty()){
                // Find candidate ships (non-repair drones) that are damaged.
                vector<Ship*> candidates;
                for(auto &ship : fleet){
                    if(ship.type != "Repair Drone" && ship.hull < 100)  // assume 100 (or 120) is full hull
                        candidates.push_back(&ship);
                }
                if(!candidates.empty()){
                    // Select the ship with lowest ratio of hull/hull_max.
                    Ship* targetShip = *min_element(candidates.begin(), candidates.end(),
                        [](Ship* a, Ship* b){ 
                            double ra = double(a->hull) / (a->type=="Shield Ship" ? 120 : 100);
                            double rb = double(b->hull) / (b->type=="Shield Ship" ? 120 : 100);
                            return ra < rb;
                        });
                    cout << "Repair Drones repair " << targetShip->type << " for 10 HP." << endl;
                    targetShip->hull = min(targetShip->hull + 10, (targetShip->type=="Shield Ship" ? 120 : 100));
                }
            }
            cout << "End of turn " << turn << ".\n";
        } // end while
        if(turn >= 10){
            cout << "Combat ended after 10 turns." << endl;
            completed = true;
            target->setUnderThreat(false);
        }
        return completed;
    }
    bool isCompleted() const { return completed; }
    const string& getDescription() const { return description; }
    const string& getObjectiveResource() const { return objectiveResource; }
    int getObjectiveAmount() const { return objectiveAmount; }
    const map<string,int>& getReward() const { return reward; }
    bool isRaider() const { return isRaiderAttack; }
    void setRaiderAttack(const string& target) { isRaiderAttack = true; targetPlanet = target; }
    const string& getTargetPlanet() const { return targetPlanet; }
private:
    string description;
    string objectiveResource;
    int objectiveAmount;
    map<string,int> reward;
    bool completed;
    bool isRaiderAttack;
    string targetPlanet;
    time_t combatStartTime; // When combat started (for idle resolution)
    int turnsElapsed;
};

//---------------------------------------------------------------------
// 3G) QUEST MANAGER
//---------------------------------------------------------------------
class QuestManager {
public:
    QuestManager() : currentQuest(nullptr), lastQuestDate("") {}
    void updateDailyQuest(Planet* central, PlanetManager& pm, vector<Ship>& fleet) {
        string todayStr = currentDateString();
        if(lastQuestDate.empty() || lastQuestDate < todayStr){
            generateNewQuest(pm);
            lastQuestDate = todayStr;
        }
        if(currentQuest){
            // Pass a flag to indicate real-time if the game is active.
            // (For simplicity, we assume the game is active if the user is at the prompt.)
            bool realTime = true; 
            currentQuest->processRaiderAttack(pm, fleet, realTime);
            currentQuest->checkCompletion(central);
        }
    }
    DailyQuest* getCurrentQuest() { return currentQuest.get(); }
    const DailyQuest* getCurrentQuestConst() const { return currentQuest.get(); }
    const string& getLastQuestDate() const { return lastQuestDate; }
    void setLastQuestDate(const string& s) { lastQuestDate = s; }
    void setCurrentQuest(unique_ptr<DailyQuest> q) { currentQuest = std::move(q); }
private:
    unique_ptr<DailyQuest> currentQuest;
    string lastQuestDate;
    void generateNewQuest(PlanetManager& pm) {
        if((rand()%100) < 20) { // Raider attack quest.
            vector<string> candidates;
            for(auto &p : pm.getPlanets())
                if(p.isUnlocked() && p.getName() != "Terra")
                    candidates.push_back(p.getName());
            if(candidates.empty())
                candidates.push_back("Terra");
            string target = candidates[rand()%candidates.size()];
            string desc = "Raiders are approaching " + target + "! Prepare to defend!";
            map<string,int> reward = { {"Engine Parts", 2} };
            auto quest = make_unique<DailyQuest>(desc, "", 1, reward);
            quest->setRaiderAttack(target);
            // If the target planet has a Proximity Alarm, issue a 5-minute warning.
            Planet* tgt = pm.getPlanetByName(target);
            if(tgt && tgt->hasBuilding("Proximity Alarm")){
                cout << "Proximity Alarm on " << target << " issues a 5-minute warning of an imminent raider attack!" << endl;
                this_thread::sleep_for(chrono::minutes(5));
                tgt->setUnderThreat(true);
            }
            currentQuest = std::move(quest);
        } else {
            vector<string> resourceChoices = {"Iron Bar", "Copper Bar", "Mithril Bar", "Titanium Bar", "Advanced Engine Parts"};
            int idx = rand()%resourceChoices.size();
            string chosen = resourceChoices[idx];
            int amt = rand()%41 + 10;
            map<string,int> reward = { {"Engine Parts", (rand()%3)+1} };
            string desc = "Collect " + to_string(amt) + " " + chosen;
            currentQuest = make_unique<DailyQuest>(desc, chosen, amt, reward);
        }
    }
    string currentDateString(){
        time_t now = time(nullptr);
        tm* gmt = gmtime(&now);
        char buf[11];
        strftime(buf, sizeof(buf), "%Y-%m-%d", gmt);
        return string(buf);
    }
};

//---------------------------------------------------------------------
// 4) PLAYER CLASS
//---------------------------------------------------------------------
class ResearchManager {
public:
    ResearchManager(const vector<ResearchDef>& data) {
        for(auto& rd : data) {
            researches.push_back(Research(rd.name, rd.cost, rd.description, rd.effectName));
        }
    }
    Research* findResearchByName(const string& name) {
        for (auto& r : researches) {
            if(r.getName() == name)
                return &r;
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
    CraftingManager(const map<string, CraftingRecipe>& recipes) : recipes(recipes) {}
    
    void craft(const string& itemName, Planet* p, double fasterMultiplier, double costMultiplier, bool precisionToolsEnabled) {
        auto it = recipes.find(itemName);
        if(it == recipes.end()){
            cout << "No recipe for item '" << itemName << "'." << endl;
            return;
        }
        CraftingRecipe recipe = it->second;
        if(!p->hasBuilding(recipe.requiredBuilding)){
            cout << "You need a " << recipe.requiredBuilding << " on " << p->getName() << " to craft " << itemName << "." << endl;
            return;
        }
        // Check resource inputs
        for(auto &kv : recipe.inputs){
            if(p->getStored(kv.first) < kv.second){
                cout << "Not enough " << kv.first << " on " << p->getName() << " to craft " << itemName << "." << endl;
                return;
            }
        }
        double effectiveCost = recipe.electricityCost * costMultiplier;
        if(p->getCurrentEnergy() < effectiveCost){
            cout << "Not enough energy on " << p->getName() << " to craft " << itemName << "." << endl;
            return;
        }
        // Deduct resources and energy
        for(auto &kv : recipe.inputs)
            p->removeFromStorage(kv.first, kv.second);
        p->setCurrentEnergy(p->getCurrentEnergy() - effectiveCost);
        
        double craftTime = recipe.timeRequired * fasterMultiplier;
        // Simulate crafting time (for simplicity, we won't actually delay)
        cout << "Crafting " << itemName << " took " << craftTime << " seconds." << endl;
        
        // Determine output quantity; assume 1 by default, possibly double with 10% chance if precisionToolsEnabled.
        int quantity = 1;
        if(precisionToolsEnabled){
            if(rand() % 100 < 10) {
                quantity = 2;
                cout << "Precision Tools activated: doubled output!" << endl;
            }
        }
        p->addToStorage(itemName, quantity);
        cout << "Crafted " << quantity << " " << itemName << "(s) on " << p->getName() << "." << endl;
    }
    
private:
    map<string, CraftingRecipe> recipes;
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
          energyConservationEnabled(false),
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
    // Const getters.
    const PlanetManager& getPlanetManager() const { return planetManager; }
    const ResearchManager& getResearchManager() const { return researchManager; }
    const QuestManager& getQuestManager() const { return questManager; }
    const vector<Ship>& getFleet() const { return fleet; }
    double getFasterCraftingMultiplier() const { return fasterCraftingMultiplier; }
    double getCraftingCostMultiplier() const { return craftingCostMultiplier; }
    bool isPrecisionToolsEnabled() const { return precisionToolsEnabled; }
    bool isEnergyConservationEnabled() const { return energyConservationEnabled; }
    void setFasterCraftingMultiplier(double val) { fasterCraftingMultiplier = val; }
    void setCraftingCostMultiplier(double val) { craftingCostMultiplier = val; }
    void setPrecisionToolsEnabled(bool val) { precisionToolsEnabled = val; }
    void setEnergyConservationEnabled(bool val) { energyConservationEnabled = val; }
    void produceResources() {
        time_t now = time(nullptr);
        double diff = difftime(now, lastUpdateTime);
        if(diff < 0) diff = 0;
        planetManager.produceAll(diff, energyConservationEnabled);
        lastUpdateTime = now;
    }
    void doResearch(const string& researchName) {
        Research* r = researchManager.findResearchByName(researchName);
        if(!r){ cout << "No research found by that name." << endl; return; }
        if(r->isCompleted()){ cout << "Research already completed." << endl; return; }
        Planet* terra = planetManager.getPlanetByName("Terra");
        if(!terra){ cout << "Central planet Terra not found." << endl; return; }
        if(r->canResearch(terra)){
            r->doResearch(terra);
            applyResearchEffect(r->getEffectName());
            cout << "Research '" << r->getName() << "' completed!" << endl;
        } else {
            cout << "Not enough resources on Terra to perform research." << endl;
        }
    }
    void craftItem(const string& itemName, const string& planetName = "Terra") {
        Planet* p = planetManager.getPlanetByName(planetName);
        if(!p){ cout << "Planet " << planetName << " not found." << endl; return; }
        craftingManager.craft(itemName, p, fasterCraftingMultiplier, craftingCostMultiplier, precisionToolsEnabled);
    }
    void craftShip(const string& shipType, const string& planetName = "Terra") {
        Planet* p = planetManager.getPlanetByName(planetName);
        if(!p){ cout << "Planet " << planetName << " not found." << endl; return; }
        if(shipType == "Shield Ship"){
            Research* rt = researchManager.findResearchByName("Shield Technology");
            if(!rt || !rt->isCompleted()){
                cout << "Shield Technology research is not complete. Cannot build Shield Ship." << endl;
                return;
            }
        }
        if(shipType == "Repair Drone"){
            Research* rd = researchManager.findResearchByName("Repair Drone Technology");
            if(!rd || !rd->isCompleted()){
                cout << "Repair Drone Technology research is not complete. Cannot build Repair Drone." << endl;
                return;
            }
        }
        if(shipType == "Radar Ship"){
            if(!p->hasBuilding("Proximity Radar") || !p->hasBuilding("Mobile Radar")){
                cout << "To build a Radar Ship, the planet must have both Proximity Radar and Mobile Radar." << endl;
                return;
            }
        }
        auto it = CRAFTING_RECIPES.find(shipType);
        if(it == CRAFTING_RECIPES.end()){
            cout << "No recipe for ship type '" << shipType << "'." << endl;
            return;
        }
        if(!p->hasBuilding(it->second.requiredBuilding)){
            cout << "You need a " << it->second.requiredBuilding << " on " << planetName << " to build a " << shipType << "." << endl;
            return;
        }
        for(auto &kv : it->second.inputs){
            if(p->getStored(kv.first) < kv.second){
                cout << "Not enough " << kv.first << " on " << planetName << " to build a " << shipType << "." << endl;
                return;
            }
        }
        double effectiveCost = it->second.electricityCost * craftingCostMultiplier;
        if(p->getCurrentEnergy() < effectiveCost){
            cout << "Not enough energy on " << planetName << " to build a " << shipType << "." << endl;
            return;
        }
        p->setCurrentEnergy(p->getCurrentEnergy() - effectiveCost);
        for(auto &kv : it->second.inputs)
            p->removeFromStorage(kv.first, kv.second);
        cout << "Built a " << shipType << " on " << planetName << " consuming " << effectiveCost << " energy." << endl;
        Ship newShip;
        newShip.type = shipType;
        if(shipType == "Transport Vessel"){
            newShip.hull = 100; newShip.maxShield = 50; newShip.currentShield = 50; newShip.weapons = 0; newShip.repairAmount = 0;
        } else if(shipType == "Corvette"){
            newShip.hull = 100; newShip.maxShield = 75; newShip.currentShield = 75; newShip.weapons = 30; newShip.repairAmount = 0;
        } else if(shipType == "Shield Ship"){
            newShip.hull = 120; newShip.maxShield = 150; newShip.currentShield = 150; newShip.weapons = 20; newShip.repairAmount = 0;
        } else if(shipType == "Radar Ship"){
            newShip.hull = 110; newShip.maxShield = 80; newShip.currentShield = 80; newShip.weapons = 25; newShip.repairAmount = 0;
        } else if(shipType == "Salvage Ship"){
            newShip.hull = 110; newShip.maxShield = 70; newShip.currentShield = 70; newShip.weapons = 15; newShip.repairAmount = 0;
        } else if(shipType == "Repair Drone"){
            newShip.hull = 80; newShip.maxShield = 40; newShip.currentShield = 40; newShip.weapons = 0; newShip.repairAmount = 10;
        }
        fleet.push_back(newShip);
    }
    void buildBuilding(const string& buildingName, const string& planetName) {
        Planet* p = planetManager.getPlanetByName(planetName);
        if(!p){ cout << "Planet " << planetName << " not found." << endl; return; }
        // Check building plot availability.
        int totalBuildings = 0;
        for(auto &b : p->getBuildings())
            totalBuildings += b.second;
        if(totalBuildings >= p->getMaxBuildingPlots()){
            cout << "No available building plots on " << planetName << ". Use 'upgrade_plots " << planetName << "' to increase capacity." << endl;
            return;
        }
        auto it = BUILDING_RECIPES.find(buildingName);
        if(it == BUILDING_RECIPES.end()){
            cout << "No recipe for building '" << buildingName << "'." << endl;
            return;
        }
        for(auto &kv : it->second.inputs){
            if(p->getStored(kv.first) < kv.second){
                cout << "Not enough " << kv.first << " on " << planetName << " to build " << buildingName << "." << endl;
                return;
            }
        }
        if(p->getCurrentEnergy() < it->second.electricityCost){
            cout << "Not enough energy on " << planetName << " to build " << buildingName << "." << endl;
            return;
        }
        p->setCurrentEnergy(p->getCurrentEnergy() - it->second.electricityCost);
        for(auto &kv : it->second.inputs)
            p->removeFromStorage(kv.first, kv.second);
        if(p->addBuilding(buildingName))
            cout << "Built " << buildingName << " on " << planetName << "." << endl;
    }
    // Only one definition of upgradeBuildingPlots is kept.
    void upgradeBuildingPlots(const string& planetName) {
        Planet* p = planetManager.getPlanetByName(planetName);
        if(!p){ cout << "Planet " << planetName << " not found." << endl; return; }
        int totalBuildings = 0;
        for(auto &b : p->getBuildings())
            totalBuildings += b.second;
        if(totalBuildings < p->getMaxBuildingPlots()){
            cout << "There are still available building plots on " << planetName << "." << endl;
            return;
        }
        // Cost: 10 Iron Bars and 2 Engine Parts.
        if(p->getStored("Iron Bar") >= 10 && p->getStored("Engine Parts") >= 2){
            if(p->upgradePlots()){
                p->removeFromStorage("Iron Bar", 10);
                p->removeFromStorage("Engine Parts", 2);
                cout << "Building capacity on " << planetName << " increased by 2." << endl;
            } else {
                cout << "Building capacity on " << planetName << " has already been upgraded." << endl;
            }
        } else {
            cout << "Not enough resources on " << planetName << " to upgrade building plots." << endl;
        }
    }
    void transferResource(const string& resourceName, int amount, const string& fromPlanetName, const string& toPlanetName) {
        Planet* fromP = planetManager.getPlanetByName(fromPlanetName);
        Planet* toP = planetManager.getPlanetByName(toPlanetName);
        if(!fromP || !toP){ cout << "Invalid planet(s) specified." << endl; return; }
        if(fromP->getStored(resourceName) < amount){
            cout << "Not enough " << resourceName << " on " << fromPlanetName << " to transfer." << endl;
            return;
        }
        bool hasTransport = false;
        for(auto &ship : fleet)
            if(ship.type == "Transport Vessel"){ hasTransport = true; break; }
        if(!hasTransport){ cout << "At least one Transport Vessel is required in your fleet." << endl; return; }
        cout << "Transferring " << amount << " " << resourceName << " from " << fromPlanetName << " to " << toPlanetName << "." << endl;
        if((rand()%100) < 30){
            cout << "Raider attack on the convoy!" << endl;
            int convoyStrength = 0, bonus = 0;
            for(auto it = fleet.begin(); it != fleet.end(); ){
                if(it->type == "Transport Vessel"){
                    convoyStrength += it->currentShield;
                    it = fleet.erase(it);
                    break;
                } else { ++it; }
            }
            for(auto &ship : fleet)
                if(ship.type == "Corvette" || ship.type == "Shield Ship" || ship.type == "Radar Ship" || ship.type == "Salvage Ship")
                    bonus += ship.weapons;
            convoyStrength += bonus;
            int raiderStrength = rand()%101 + 20;
            cout << "Convoy strength: " << convoyStrength << ", Raider strength: " << raiderStrength << endl;
            if(convoyStrength >= raiderStrength){
                cout << "Convoy fended off the raiders!" << endl;
                fromP->removeFromStorage(resourceName, amount);
                toP->addToStorage(resourceName, amount);
                bool hasSalvageShip = false;
                for(auto &ship : fleet)
                    if(ship.type == "Salvage Ship"){ hasSalvageShip = true; break; }
                if(hasSalvageShip){
                    int extra = (rand()%6) + 5;
                    fromP->addToStorage("Iron", extra);
                    cout << "Your Salvage Ship recovered an extra " << extra << " Iron from the wreckage." << endl;
                }
            } else {
                cout << "Convoy was destroyed! Resources lost." << endl;
                if(fromP->hasBuilding("Salvage Robot")){
                    int salvage = (rand()%11)+5;
                    fromP->addToStorage("Iron", salvage);
                    cout << "Salvage Robot recovered " << salvage << " Iron." << endl;
                }
            }
        } else {
            fromP->removeFromStorage(resourceName, amount);
            toP->addToStorage(resourceName, amount);
            cout << "Transfer successful." << endl;
        }
    }
    void showRadar(const string& planetName) {
        Planet* p = planetManager.getPlanetByName(planetName);
        if(!p){ cout << "Planet " << planetName << " not found." << endl; return; }
        if(p->hasBuilding("Radar"))
            cout << "Radar on " << planetName << " indicates HIGH raider activity." << endl;
        else
            cout << "No Radar installed on " << planetName << ". Raider activity unknown." << endl;
    }
    void updateDailyQuest() {
        Planet* terra = planetManager.getPlanetByName("Terra");
        if(!terra) return;
        questManager.updateDailyQuest(terra, planetManager, fleet);
    }
    void recalcUpgradesFromResearch() {
        fasterCraftingMultiplier = 1.0;
        craftingCostMultiplier = 1.0;
        precisionToolsEnabled = false;
        for(auto &r : researchManager.getAllResearches()){
            if(r.isCompleted()){
                string eff = r.getEffectName();
                if(eff == "faster_crafting")
                    fasterCraftingMultiplier = 0.5;
                else if(eff == "crafting_mastery")
                    craftingCostMultiplier = 0.8;
                else if(eff == "precision_tools")
                    precisionToolsEnabled = true;
                else if(eff == "energy_conservation")
                    energyConservationEnabled = true;
            }
        }
    }
    void showFleet() {
        if(fleet.empty()){ cout << "Your fleet is empty." << endl; return; }
        cout << "Your fleet:" << endl;
        for(size_t i = 0; i < fleet.size(); i++){
            cout << i+1 << ". " << fleet[i].type 
                 << " (Hull: " << fleet[i].hull 
                 << ", Shield: " << fleet[i].currentShield << "/" << fleet[i].maxShield;
            if(fleet[i].type=="Corvette" || fleet[i].type=="Shield Ship" ||
               fleet[i].type=="Radar Ship" || fleet[i].type=="Salvage Ship")
                cout << ", Weapons: " << fleet[i].weapons;
            if(fleet[i].type=="Repair Drone")
                cout << ", Repairs: " << fleet[i].repairAmount << " HP/turn";
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
    bool energyConservationEnabled;
    time_t lastUpdateTime;
    void applyResearchEffect(const string& effectName) {
        if(effectName == "unlock_mars")
            planetManager.unlockPlanet("Mars");
        else if(effectName == "unlock_zalthor")
            planetManager.unlockPlanet("Zalthor");
        else if(effectName == "faster_crafting")
            fasterCraftingMultiplier = 0.5;
        else if(effectName == "unlock_vulcan")
            planetManager.unlockPlanet("Vulcan");
        else if(effectName == "unlock_luna")
            planetManager.unlockPlanet("Luna");
        else if(effectName == "crafting_mastery")
            craftingCostMultiplier = 0.8;
        else if(effectName == "precision_tools")
            precisionToolsEnabled = true;
        else if(effectName == "unlock_shield_ships")
            cout << "Shield Ships unlocked!" << endl;
        else if(effectName == "energy_conservation")
            energyConservationEnabled = true;
        else if(effectName == "unlock_repair_drones")
            cout << "Repair Drones unlocked!" << endl;
        else
            cout << "Unknown research effect: " << effectName << endl;
    }
};

//---------------------------------------------------------------------
// 5) SAVE/LOAD FUNCTIONS
//---------------------------------------------------------------------
void saveGame(const Player& player) {
    json j;
    {
        json jp = json::array();
        const auto& planets = player.getPlanetManager().getPlanetsConst();
        for(auto& p : planets){
            json pObj;
            pObj["name"] = p.getName();
            pObj["unlocked"] = p.isUnlocked();
            pObj["current_energy"] = p.getCurrentEnergy();
            pObj["max_energy"] = p.getMaxEnergy();
            // (For brevity, storage, buildings, etc. are omitted.)
            jp.push_back(pObj);
        }
        j["planets"] = jp;
    }
    {
        json jrch = json::array();
        for(auto& r : player.getResearchManager().getAllResearchesConst()){
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
        const DailyQuest* dq = player.getQuestManager().getCurrentQuestConst();
        if(dq){
            questJson = json::object();
            questJson["description"] = dq->getDescription();
            questJson["objective_resource"] = dq->getObjectiveResource();
            questJson["objective_amount"] = dq->getObjectiveAmount();
            json rew = json::object();
            for(auto& kv : dq->getReward())
                rew[kv.first] = kv.second;
            questJson["reward"] = rew;
            questJson["completed"] = dq->isCompleted();
            if(dq->isRaider())
                questJson["target_planet"] = dq->getTargetPlanet();
        }
        j["current_quest"] = questJson;
        j["last_quest_date"] = player.getQuestManager().getLastQuestDate();
    }
    j["last_update_time"] = static_cast<long long>(player.getLastUpdateTime());
    {
        json fleetJson = json::array();
        for(auto& ship : player.getFleet()){
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
    if(!ofs.is_open()){
        cerr << "Failed to open save file for writing." << endl;
        return;
    }
    ofs << j.dump(2);
    ofs.close();
    cout << "Game saved." << endl;
}

void loadGame(Player& player) {
    ifstream ifs(SAVE_FILE);
    if(!ifs.is_open()) return;
    json j;
    ifs >> j;
    ifs.close();
    // (Loading of planets, research, fleet, etc. omitted for brevity.)
    player.recalcUpgradesFromResearch();
    cout << "Game loaded from " << SAVE_FILE << "." << endl;
}

//---------------------------------------------------------------------
// A simple function for "talk" command to output lore
//---------------------------------------------------------------------
void talkToCharacters(void) {
    cout << "\nLore: " << genericLore[rand() % genericLore.size()] << "\n";
}

//---------------------------------------------------------------------
// 6) MAIN GAME LOOP
//---------------------------------------------------------------------
void showHelp() {
    cout << "Commands:\n"
         << "  help                           - Show this help\n"
         << "  stats                          - Show resource storage on each planet\n"
         << "  planets                        - Show planet status (energy, etc.)\n"
         << "  fleet                          - Show your fleet\n"
         << "  research                       - List research options\n"
         << "  do_research <name>             - Perform research (using Terra's storage)\n"
         << "  craft <item> [on <planet>]     - Craft an item on a planet\n"
         << "     (For ships, use 'craft Transport Vessel', 'craft Corvette',\n"
         << "      'craft Shield Ship', 'craft Radar Ship', 'craft Salvage Ship',\n"
         << "      or 'craft Repair Drone')\n"
         << "  build <building> on <planet>   - Build a building on a planet\n"
         << "  upgrade_plots <planet>         - Upgrade building capacity on a planet\n"
         << "  install <facility> on <planet> - Install a facility (Generator/Accumulator)\n"
         << "  transfer <resource> <amount> from <planet1> to <planet2> - Transfer resources\n"
         << "  radar <planet>                 - Get radar info for a planet\n"
         << "  daily                          - Show the current daily quest\n"
         << "  talk                           - Talk to characters for lore\n"
         << "  save                           - Save game\n"
         << "  quit                           - Save and quit\n";
}

int main(){
    Player player;
    loadGame(player);
    player.recalcUpgradesFromResearch();
    cout << "Welcome to the Expanded Modular Idle Planet Miner!" << endl;
    cout << "Type 'help' for commands." << endl;
    while(true){
        player.produceResources();
        player.updateDailyQuest();
        cout << "> ";
        string command;
        if(!getline(cin, command))
            break;
        if(command.empty())
            continue;
        auto spacePos = command.find(' ');
        string cmd = (spacePos == string::npos) ? command : command.substr(0, spacePos);
        string args = (spacePos == string::npos) ? "" : command.substr(spacePos+1);
        transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
        if(cmd=="help")
            showHelp();
        else if(cmd=="stats"){
            auto& planets = player.getPlanetManager().getPlanets();
            for(auto& p : planets){
                cout << "Planet " << p.getName() << " storage:" << endl;
                for(auto& kv : p.getStorageMap())
                    cout << "  " << kv.first << ": " << kv.second << endl;
            }
        }
        else if(cmd=="planets"){
            auto& plist = player.getPlanetManager().getPlanetsConst();
            for(auto& p : plist)
                cout << p.getName() << " => " << (p.isUnlocked()?"Unlocked":"Locked")
                     << " | Energy: " << p.getCurrentEnergy() << "/" << p.getMaxEnergy() << endl;
        }
        else if(cmd=="fleet")
            player.showFleet();
        else if(cmd=="research"){
            for(auto& r : player.getResearchManager().getAllResearchesConst()){
                cout << r.getName() << " - " << (r.isCompleted()?"Completed":"Not Completed")
                     << " | Cost: ";
                for(auto& c : r.getCost())
                    cout << c.first << ":" << c.second << " ";
                cout << "| " << r.getDescription() << endl;
            }
        }
        else if(cmd=="do_research"){
            if(args.empty())
                cout << "Usage: do_research <research name>" << endl;
            else
                player.doResearch(args);
        }
        else if(cmd=="craft"){
            string itemName, planetName;
            size_t pos = args.find(" on ");
            if(pos != string::npos){
                itemName = args.substr(0, pos);
                planetName = args.substr(pos+4);
            } else { itemName = args; planetName = "Terra"; }
            if(itemName.empty())
                cout << "Usage: craft <item> [on <planet>]" << endl;
            else {
                if(itemName=="Transport Vessel" || itemName=="Corvette" || itemName=="Shield Ship" ||
                   itemName=="Radar Ship" || itemName=="Salvage Ship" || itemName=="Repair Drone")
                    player.craftShip(itemName, planetName);
                else
                    player.craftItem(itemName, planetName);
            }
        }
        else if(cmd=="build"){
            size_t pos = args.find(" on ");
            if(pos != string::npos){
                string buildingName = args.substr(0, pos);
                string planetName = args.substr(pos+4);
                if(buildingName.empty() || planetName.empty())
                    cout << "Usage: build <building> on <planet>" << endl;
                else
                    player.buildBuilding(buildingName, planetName);
            } else
                cout << "Usage: build <building> on <planet>" << endl;
        }
        else if(cmd=="upgrade_plots"){
            if(args.empty())
                cout << "Usage: upgrade_plots <planet>" << endl;
            else
                player.upgradeBuildingPlots(args);
        }
        else if(cmd=="install"){
            size_t pos = args.find(" on ");
            if(pos != string::npos){
                string facility = args.substr(0, pos);
                string planetName = args.substr(pos+4);
                if(facility.empty() || planetName.empty())
                    cout << "Usage: install <facility> on <planet>" << endl;
                else {
                    Planet* p = player.getPlanetManager().getPlanetByName(planetName);
                    if(!p)
                        cout << "Planet " << planetName << " not found." << endl;
                    else {
                        if(p->removeFromStorage(facility,1))
                            p->installFacility(facility);
                        else
                            cout << "No " << facility << " available on " << planetName << "." << endl;
                    }
                }
            } else
                cout << "Usage: install <facility> on <planet>" << endl;
        }
        else if(cmd=="transfer"){
            istringstream iss(args);
            string resource;
            int amount;
            string fromWord, fromPlanet, toWord, toPlanet;
            if(!(iss >> resource >> amount >> fromWord >> fromPlanet >> toWord >> toPlanet))
                cout << "Usage: transfer <resource> <amount> from <planet1> to <planet2>" << endl;
            else {
                if(fromWord!="from" || toWord!="to")
                    cout << "Usage: transfer <resource> <amount> from <planet1> to <planet2>" << endl;
                else
                    player.transferResource(resource, amount, fromPlanet, toPlanet);
            }
        }
        else if(cmd=="radar"){
            if(args.empty())
                cout << "Usage: radar <planet>" << endl;
            else
                player.showRadar(args);
        }
        else if(cmd=="daily"){
            DailyQuest* dq = player.getQuestManager().getCurrentQuest();
            if(dq){
                cout << dq->getDescription() << endl;
                if(dq->isRaider())
                    cout << "Target Planet: " << dq->getTargetPlanet() << endl;
                else
                    cout << "Objective: " << dq->getObjectiveAmount() << " " << dq->getObjectiveResource() << endl;
                cout << "Completed: " << (dq->isCompleted()?"Yes":"No") << endl;
            } else
                cout << "No current daily quest." << endl;
        }
        else if(cmd=="talk"){
            talkToCharacters();
        }
        else if(cmd=="save"){
            saveGame(player);
        }
        else if(cmd=="quit"){
            saveGame(player);
            cout << "Exiting game. Goodbye!" << endl;
            break;
        }
        else
            cout << "Unknown command. Type 'help' for commands." << endl;
    }
    return 0;
}
