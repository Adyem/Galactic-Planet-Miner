#ifndef PLANET_H
#define PLANET_H

#include "game_data.h"
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

class Planet {
public:
    Planet(const std::string &name, const std::map<std::string, double> &production, bool unlocked);
    void produceResources(double elapsedSeconds, bool energyConservationEnabled);
    bool canBuildMore() const;
    bool upgradePlots();
    void increaseMaxBuildingPlots(int amount);
    bool addBuildingWithCost(const std::string &buildingName, int plotCost);
    bool addBuilding(const std::string &buildingName);
    bool hasBuilding(const std::string &buildingName) const;
    const std::map<std::string, int> &getBuildings() const;
    void addToStorage(const std::string &resourceName, int amount);
    bool removeFromStorage(const std::string &resourceName, int amount);
    int getStored(const std::string &resourceName) const;
    const std::map<std::string, int> &getStorageMap() const;
    const std::string &getName() const;
    bool isUnlocked() const;
    void setUnlocked(bool unlocked);
    double getCurrentEnergy() const;
    double getMaxEnergy() const;
    int getGenerators() const;
    int getAccumulators() const;
    void setCurrentEnergy(double energy);
    void setMaxEnergy(double energy);
    void setGenerators(int generators);
    void setAccumulators(int accumulators);
    void installFacility(const std::string &facility);
    bool isUnderThreat() const;
    void setUnderThreat(bool flag);
    int getMaxBuildingPlots() const;
    int getUsedBuildingPlots() const;
private:
    std::string name_;
    std::map<std::string, double> baseProduction_;
    bool unlocked_;
    int generators_;
    int accumulators_;
    double currentEnergy_;
    double maxEnergy_;
    std::map<std::string, int> storage_;
    std::map<std::string, int> buildings_;
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

class PlanetManager {
public:
    PlanetManager(const std::vector<PlanetDef> &data);
    void produceAll(double elapsedSeconds, bool energyConservationEnabled);
    void unlockPlanet(const std::string &planetName);
    Planet *getPlanetByName(const std::string &planetName);
    const std::vector<Planet> &getPlanetsConst() const;
    std::vector<Planet> &getPlanets();
private:
    std::vector<Planet> planets_;
};

#endif // PLANET_H
