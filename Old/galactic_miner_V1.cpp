#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>

enum ResourceType {
    IRON,
    COPPER,
    MITHRIL,
    RESOURCE_COUNT
};

struct Research {
    std::string name;
    double cost[RESOURCE_COUNT];
    bool unlocked;
};

struct Planet {
    std::string name;
    double baseProduction[RESOURCE_COUNT];
    int mines[RESOURCE_COUNT];
};

struct GameData {
    std::vector<Planet> planets;
    double playerResources[RESOURCE_COUNT];
    std::vector<Research> researches;
    std::time_t lastSaveTime;
};

GameData loadGameData(const std::string& filename) {
    GameData data;
    for (int i = 0; i < RESOURCE_COUNT; ++i) {
        data.playerResources[i] = 0.0;
    }
    std::ifstream inFile(filename);
    if (!inFile) {
        Planet earth{"Earth", {0.5, 0.2, 0.0}, {0, 0, 0}};
        Planet mars{"Mars", {0.4, 0.1, 0.0}, {0, 0, 0}};
        Planet jupiter{"Jupiter", {0.6, 0.2, 0.1}, {0, 0, 0}};
        data.planets.push_back(earth);
        data.planets.push_back(mars);
        data.planets.push_back(jupiter);
        Research r1{"Advanced Mining", {100.0, 50.0, 0.0}, false};
        Research r2{"Mithril Extraction", {200.0, 100.0, 50.0}, false};
        data.researches.push_back(r1);
        data.researches.push_back(r2);
        data.lastSaveTime = std::time(nullptr);
    } else {
        int planetCount;
        inFile >> planetCount;
        data.planets.resize(planetCount);
        for (int i = 0; i < planetCount; ++i) {
            inFile >> data.planets[i].name;
            for (int r = 0; r < RESOURCE_COUNT; ++r) {
                inFile >> data.planets[i].baseProduction[r];
            }
            for (int r = 0; r < RESOURCE_COUNT; ++r) {
                inFile >> data.planets[i].mines[r];
            }
        }
        for (int r = 0; r < RESOURCE_COUNT; ++r) {
            inFile >> data.playerResources[r];
        }
        int researchCount;
        inFile >> researchCount;
        data.researches.resize(researchCount);
        for (int i = 0; i < researchCount; ++i) {
            inFile >> data.researches[i].name;
            for (int r = 0; r < RESOURCE_COUNT; ++r) {
                inFile >> data.researches[i].cost[r];
            }
            inFile >> data.researches[i].unlocked;
        }
        inFile >> data.lastSaveTime;
        inFile.close();
    }
    return data;
}

void saveGameData(const GameData& data, const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile) return;
    outFile << data.planets.size() << std::endl;
    for (auto& p : data.planets) {
        outFile << p.name << " ";
        for (int r = 0; r < RESOURCE_COUNT; ++r) {
            outFile << p.baseProduction[r] << " ";
        }
        for (int r = 0; r < RESOURCE_COUNT; ++r) {
            outFile << p.mines[r] << " ";
        }
        outFile << std::endl;
    }
    for (int r = 0; r < RESOURCE_COUNT; ++r) {
        outFile << data.playerResources[r] << " ";
    }
    outFile << std::endl;
    outFile << data.researches.size() << std::endl;
    for (auto& rs : data.researches) {
        outFile << rs.name << " ";
        for (int r = 0; r < RESOURCE_COUNT; ++r) {
            outFile << rs.cost[r] << " ";
        }
        outFile << rs.unlocked << std::endl;
    }
    outFile << data.lastSaveTime << std::endl;
    outFile.close();
}

void applyIdleGains(GameData& data) {
    std::time_t currentTime = std::time(nullptr);
    double secondsPassed = std::difftime(currentTime, data.lastSaveTime);
    if (secondsPassed < 0) secondsPassed = 0;
    for (auto& planet : data.planets) {
        for (int r = 0; r < RESOURCE_COUNT; ++r) {
            if (!data.researches.empty()) {
                bool advancedUnlocked = false;
                bool mithrilUnlocked = false;
                for (auto& res : data.researches) {
                    if (res.name == "Advanced Mining" && res.unlocked) advancedUnlocked = true;
                    if (res.name == "Mithril Extraction" && res.unlocked) mithrilUnlocked = true;
                }
                if (r == MITHRIL && !mithrilUnlocked) continue;
                if ((r == IRON || r == COPPER) && !advancedUnlocked) {
                    data.playerResources[r] += planet.baseProduction[r] * planet.mines[r]
						* secondsPassed * 0.5;
                } else {
                    data.playerResources[r] += planet.baseProduction[r] * planet.mines[r]
						* secondsPassed;
                }
            } else {
                data.playerResources[r] += planet.baseProduction[r] * planet.mines[r]
					* secondsPassed;
            }
        }
    }
    data.lastSaveTime = currentTime;
}

void displayStatus(const GameData& data) {
    std::cout << "\nResources:\n";
    std::cout << "Iron: "    << std::fixed << std::setprecision(2) << data.playerResources[IRON]
		<< std::endl;
    std::cout << "Copper: "  << data.playerResources[COPPER] << std::endl;
    std::cout << "Mithril: " << data.playerResources[MITHRIL] << std::endl;
    std::cout << "\nPlanets:\n";
    for (auto& p : data.planets) {
        std::cout << p.name << " (";
        std::cout << "Mines: Iron=" << p.mines[IRON] << ", Copper=" << p.mines[COPPER]
                  << ", Mithril=" << p.mines[MITHRIL] << ")\n";
    }
    std::cout << "\nResearch:\n";
    for (auto& rs : data.researches) {
        std::cout << rs.name << (rs.unlocked ? " (Unlocked)" : " (Locked)") << std::endl;
    }
    std::cout << std::endl;
}

void buildMine(GameData& data) {
    std::cout << "\nChoose planet:\n";
    for (size_t i = 0; i < data.planets.size(); ++i) {
        std::cout << i << ") " << data.planets[i].name << std::endl;
    }
    int planetChoice;
    std::cin >> planetChoice;
    if (std::cin.fail() || planetChoice < 0 || planetChoice >= (int)data.planets.size()) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Invalid choice\n";
        return;
    }
    std::cout << "Choose resource to mine:\n";
    std::cout << "0) Iron\n1) Copper\n2) Mithril\n";
    int resourceChoice;
    std::cin >> resourceChoice;
    if (std::cin.fail() || resourceChoice < 0 || resourceChoice >= RESOURCE_COUNT) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Invalid choice\n";
        return;
    }
    double costIron = 50.0;
    double costCopper = 20.0;
    double costMithril = 5.0;
    if (data.playerResources[IRON] < costIron || data.playerResources[COPPER] < costCopper
        || data.playerResources[MITHRIL] < costMithril) {
        std::cout << "Not enough resources\n";
        return;
    }
    if (resourceChoice == MITHRIL) {
        bool mithrilUnlocked = false;
        for (auto& rs : data.researches) {
            if (rs.name == "Mithril Extraction" && rs.unlocked) {
                mithrilUnlocked = true;
                break;
            }
        }
        if (!mithrilUnlocked) {
            std::cout << "Research needed for Mithril mining\n";
            return;
        }
    }
    data.playerResources[IRON] -= costIron;
    data.playerResources[COPPER] -= costCopper;
    data.playerResources[MITHRIL] -= costMithril;
    data.planets[planetChoice].mines[resourceChoice] += 1;
    std::cout << "Mine built\n";
}

void craftResources(GameData& data) {
    std::cout << "\nChoose resource to craft:\n";
    std::cout << "1) Convert Iron to Copper (cost 10 Iron -> 2 Copper)\n";
    std::cout << "2) Convert Copper to Mithril (cost 10 Copper -> 1 Mithril)\n";
    int choice;
    std::cin >> choice;
    if (choice == 1) {
        if (data.playerResources[IRON] >= 10.0) {
            data.playerResources[IRON] -= 10.0;
            data.playerResources[COPPER] += 2.0;
            std::cout << "Crafted Copper\n";
        } else {
            std::cout << "Not enough Iron\n";
        }
    } else if (choice == 2) {
        bool mithrilUnlocked = false;
        for (auto& rs : data.researches) {
            if (rs.name == "Mithril Extraction" && rs.unlocked) {
                mithrilUnlocked = true;
                break;
            }
        }
        if (!mithrilUnlocked) {
            std::cout << "Research needed for Mithril\n";
            return;
        }
        if (data.playerResources[COPPER] >= 10.0) {
            data.playerResources[COPPER] -= 10.0;
            data.playerResources[MITHRIL] += 1.0;
            std::cout << "Crafted Mithril\n";
        } else {
            std::cout << "Not enough Copper\n";
        }
    }
}

void doResearch(GameData& data) {
    std::cout << "\nResearch options:\n";
    for (size_t i = 0; i < data.researches.size(); ++i) {
        std::cout << i << ") " << data.researches[i].name << (data.researches[i].unlocked
			? " (Unlocked)" : " (Locked)")
                  << " Cost: Iron=" << data.researches[i].cost[IRON]
                  << ", Copper=" << data.researches[i].cost[COPPER]
                  << ", Mithril=" << data.researches[i].cost[MITHRIL] << "\n";
    }
    int choice;
    std::cin >> choice;
    if (std::cin.fail() || choice < 0 || choice >= (int)data.researches.size()) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Invalid choice\n";
        return;
    }
    if (data.researches[choice].unlocked) {
        std::cout << "Already unlocked\n";
        return;
    }
    double needIron = data.researches[choice].cost[IRON];
    double needCopper = data.researches[choice].cost[COPPER];
    double needMithril = data.researches[choice].cost[MITHRIL];
    if (data.playerResources[IRON] >= needIron && data.playerResources[COPPER] >= needCopper
        && data.playerResources[MITHRIL] >= needMithril) {
        data.playerResources[IRON] -= needIron;
        data.playerResources[COPPER] -= needCopper;
        data.playerResources[MITHRIL] -= needMithril;
        data.researches[choice].unlocked = true;
        std::cout << "Research unlocked\n";
    } else {
        std::cout << "Not enough resources\n";
    }
}

int main() {
    const std::string saveFileName = "savegame.txt";
    GameData gameData = loadGameData(saveFileName);
    applyIdleGains(gameData);
    bool running = true;
    while (running) {
        std::cout << "=========================\n";
        std::cout << "      IDLE PLANET MINER \n";
        std::cout << "=========================\n";
        std::cout << "1) View Status\n";
        std::cout << "2) Build Mine\n";
        std::cout << "3) Craft\n";
        std::cout << "4) Research\n";
        std::cout << "5) Save & Exit\n";
        int choice;
        std::cin >> choice;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            continue;
        }
        switch (choice) {
            case 1:
                applyIdleGains(gameData);
                displayStatus(gameData);
                break;
            case 2:
                applyIdleGains(gameData);
                buildMine(gameData);
                break;
            case 3:
                applyIdleGains(gameData);
                craftResources(gameData);
                break;
            case 4:
                applyIdleGains(gameData);
                doResearch(gameData);
                break;
            case 5:
                saveGameData(gameData, saveFileName);
                running = false;
                break;
            default:
                break;
        }
    }
    std::cout << "Thanks for playing!\n";
    return 0;
}
