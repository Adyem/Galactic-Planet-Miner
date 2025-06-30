#include "planet.h"
#include <cmath>

Planet::Planet(const std::string &name, const std::map<std::string, double> &production, bool unlocked)
    : name_(name), baseProduction_(production), unlocked_(unlocked),
      generators_(0), accumulators_(0), currentEnergy_(0.0), maxEnergy_(50.0),
      maxBuildingPlots_(8), currentBuildingCount_(0), plotsUpgraded_(false), underThreat_(false) {
    for (const auto &res : RESOURCE_DATA)
        storage_[res.name] = 0;
}

void Planet::produceResources(double elapsedSeconds, bool energyConservationEnabled) {
    if (underThreat_ && energyConservationEnabled && hasBuilding("Shield Generator")) {
        std::cout << "Production on " << name_ << " halted due to Emergency Energy Conservation." << std::endl;
        return;
    }
    if (!unlocked_)
        return;
    double productionTime = elapsedSeconds;
    if (generators_ > 0) {
        double possibleTime = static_cast<double>(getStored("Coal")) / (generators_ * COAL_CONSUMPTION);
        productionTime = std::min(elapsedSeconds, possibleTime);
        int coalNeeded = static_cast<int>(ceil(generators_ * COAL_CONSUMPTION * productionTime));
        removeFromStorage("Coal", coalNeeded);
    }
    double energyProduced = generators_ * GENERATOR_RATE * productionTime;
    currentEnergy_ = std::min(maxEnergy_, currentEnergy_ + energyProduced);
    if (hasBuilding("Solar Panel")) {
        int solarCount = getBuildings().at("Solar Panel");
        double solarEnergy = solarCount * SOLAR_RATE * elapsedSeconds;
        currentEnergy_ = std::min(maxEnergy_, currentEnergy_ + solarEnergy);
    }
    double effectiveSeconds = std::min(productionTime, currentEnergy_ / ENERGY_COST_PER_SECOND);
    double energyConsumed = effectiveSeconds * ENERGY_COST_PER_SECOND;
    currentEnergy_ -= energyConsumed;
    for (auto &entry : baseProduction_) {
        int produced = static_cast<int>(floor(entry.second * productionTime));
        storage_[entry.first] += produced;
    }
    if (hasBuilding("Tritium Extractor")) {
        int extractorCount = getBuildings().at("Tritium Extractor");
        int tritiumProduced = static_cast<int>(floor(extractorCount * TRITIUM_EXTRACTION_RATE * productionTime));
        storage_["Tritium"] += tritiumProduced;
        if (tritiumProduced > 0)
            std::cout << "Tritium Extractor on " << name_ << " produced " << tritiumProduced << " Tritium." << std::endl;
    }
}

bool Planet::canBuildMore() const { return currentBuildingCount_ < maxBuildingPlots_; }

bool Planet::upgradePlots() {
    if (!plotsUpgraded_) {
        plotsUpgraded_ = true;
        maxBuildingPlots_ += 2;
        return true;
    }
    return false;
}

void Planet::increaseMaxBuildingPlots(int amount) { maxBuildingPlots_ += amount; }

bool Planet::addBuildingWithCost(const std::string &buildingName, int plotCost) {
    if (currentBuildingCount_ + plotCost > maxBuildingPlots_) {
        std::cout << "No building plots available on " << name_ << " for " << buildingName << "." << std::endl;
        return false;
    }
    buildings_[buildingName]++;
    currentBuildingCount_ += plotCost;
    return true;
}

bool Planet::addBuilding(const std::string &buildingName) {
    if (!canBuildMore()) {
        std::cout << "No building plots available on " << name_ << ". Upgrade building capacity first." << std::endl;
        return false;
    }
    buildings_[buildingName]++;
    currentBuildingCount_++;
    return true;
}

bool Planet::hasBuilding(const std::string &buildingName) const {
    auto it = buildings_.find(buildingName);
    return (it != buildings_.end() && it->second > 0);
}

const std::map<std::string, int> &Planet::getBuildings() const { return buildings_; }

void Planet::addToStorage(const std::string &resourceName, int amount) { storage_[resourceName] += amount; }

bool Planet::removeFromStorage(const std::string &resourceName, int amount) {
    if (storage_[resourceName] >= amount) { storage_[resourceName] -= amount; return true; }
    return false;
}

int Planet::getStored(const std::string &resourceName) const {
    auto it = storage_.find(resourceName);
    return (it != storage_.end()) ? it->second : 0;
}

const std::map<std::string, int> &Planet::getStorageMap() const { return storage_; }

const std::string &Planet::getName() const { return name_; }

bool Planet::isUnlocked() const { return unlocked_; }

void Planet::setUnlocked(bool unlocked) { unlocked_ = unlocked; }

double Planet::getCurrentEnergy() const { return currentEnergy_; }

double Planet::getMaxEnergy() const { return maxEnergy_; }

int Planet::getGenerators() const { return generators_; }

int Planet::getAccumulators() const { return accumulators_; }

void Planet::setCurrentEnergy(double energy) { currentEnergy_ = energy; }

void Planet::setMaxEnergy(double energy) { maxEnergy_ = energy; }

void Planet::setGenerators(int generators) { generators_ = generators; }

void Planet::setAccumulators(int accumulators) { accumulators_ = accumulators; }

void Planet::installFacility(const std::string &facility) {
    std::string fac = facility;
    std::transform(fac.begin(), fac.end(), fac.begin(), ::tolower);
    if (fac == "generator") {
        if (!canBuildMore()) { std::cout << "No building plots available on " << name_ << " for Generator." << std::endl; return; }
        addBuilding("Generator");
        generators_++;
        std::cout << "Installed a Generator on " << name_ << ". Total: " << generators_ << std::endl;
    } else if (fac == "accumulator") {
        accumulators_++;
        maxEnergy_ += 50.0;
        std::cout << "Installed an Accumulator on " << name_ << ". Total: " << accumulators_ << ", new max energy: " << maxEnergy_ << std::endl;
    } else { std::cout << "Unknown facility: " << facility << std::endl; }
}

bool Planet::isUnderThreat() const { return underThreat_; }

void Planet::setUnderThreat(bool flag) { underThreat_ = flag; }

int Planet::getMaxBuildingPlots() const { return maxBuildingPlots_; }

int Planet::getUsedBuildingPlots() const { return currentBuildingCount_; }

PlanetManager::PlanetManager(const std::vector<PlanetDef> &data) {
    for (const auto &planet : data)
        planets_.push_back(Planet(planet.name, planet.baseProduction, planet.unlocked));
}

void PlanetManager::produceAll(double elapsedSeconds, bool energyConservationEnabled) {
    for (auto &planet : planets_)
        planet.produceResources(elapsedSeconds, energyConservationEnabled);
}

void PlanetManager::unlockPlanet(const std::string &planetName) {
    for (auto &planet : planets_)
        if (planet.getName() == planetName)
            planet.setUnlocked(true);
}

Planet *PlanetManager::getPlanetByName(const std::string &planetName) {
    for (auto &planet : planets_)
        if (planet.getName() == planetName)
            return &planet;
    return nullptr;
}

const std::vector<Planet> &PlanetManager::getPlanetsConst() const { return planets_; }

std::vector<Planet> &PlanetManager::getPlanets() { return planets_; }

