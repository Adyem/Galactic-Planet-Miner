#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <string>
#include <map>
#include <vector>

extern const double SOLAR_SHIP_BONUS_RATE;
extern const int SUNFLARE_SHIELD_REGEN;

struct ResourceDef {
    std::string name;
    int initial;
};
extern const std::vector<ResourceDef> RESOURCE_DATA;

extern const std::vector<std::string> resourceLore;
extern const std::vector<std::string> raiderLore;
extern const std::vector<std::string> genericLore;

extern const std::string SAVE_FILE;

struct PlanetDef {
    std::string name;
    std::map<std::string, double> baseProduction;
    bool unlocked;
};
extern const std::vector<PlanetDef> PLANET_DATA;

struct ResearchDef {
    std::string name;
    std::map<std::string, int> cost;
    std::string description;
    std::string effectName;
};
extern const std::vector<ResearchDef> RESEARCH_DATA;

struct CraftingRecipe {
    std::map<std::string, int> inputs;
    double timeRequired;
    double electricityCost;
    std::string requiredBuilding;
};
extern const std::map<std::string, CraftingRecipe> CRAFTING_RECIPES;

struct BuildingRecipe {
    std::map<std::string, int> inputs;
    double timeRequired;
    double electricityCost;
    int plotCost;
};
extern const std::map<std::string, BuildingRecipe> BUILDING_RECIPES;

#endif // GAME_DATA_H
