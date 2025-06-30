#include "quest.h"
#include "game_data.h"
#include <iostream>
#include <thread>
#include <algorithm>

DailyQuest::DailyQuest(const std::string &desc, const std::string &objectiveRes, int objectiveAmt, const std::map<std::string,int> &reward)
    : description_(desc), objectiveResource_(objectiveRes), objectiveAmount_(objectiveAmt), reward_(reward),
      completed_(false), isRaiderAttack_(false), targetPlanet_(""), combatStartTime_(0), turnsElapsed_(0),
      isStoryQuest_(false), isFinalConfrontation_(false) {}

void DailyQuest::checkCompletion(Planet *central) {
    if (!completed_ && !objectiveResource_.empty() && central->getStored(objectiveResource_) >= objectiveAmount_) {
        completed_ = true;
        for (const auto &entry : reward_)
            central->addToStorage(entry.first, entry.second);
        std::cout << "Quest complete! Reward: ";
        for (const auto &entry : reward_)
            std::cout << entry.first << " +" << entry.second << " ";
        std::cout << std::endl;
        if (!isStoryQuest_) {
            std::string journalText = "You successfully gathered " + std::to_string(objectiveAmount_) +
                " " + objectiveResource_ + ". The effort has not gone unnoticed.";
            journal.addEntry("Resource Quest Completed", journalText);
            std::cout << "\nLore: " << resourceLore[rand() % resourceLore.size()] << std::endl;
        }
    }
}

bool DailyQuest::processRaiderAttack(PlanetManager &pm, std::vector<Ship> &fleet, bool realTime, bool escapePodActive) {
    if (completed_ || !isRaiderAttack_)
        return false;
    Planet *target = pm.getPlanetByName(targetPlanet_);
    if (!target) {
        std::cout << "Target planet " << targetPlanet_ << " not found." << std::endl;
        return false;
    }
    if (combatStartTime_ == 0)
        combatStartTime_ = time(nullptr);
    int raiderShield = (isFinalConfrontation_) ? rand() % 101 + 200 : rand() % 101 + 100;
    int raiderHull = (isFinalConfrontation_) ? rand() % 201 + 600 : rand() % 201 + 300;
    std::cout << "Commencing raider battle at " << targetPlanet_ << "!" << std::endl;
    if (isFinalConfrontation_)
        std::cout << "Multiple enemy capital ships support the assault, their cannons roaring in unison!" << std::endl;
    const int maxTurns = 10;
    int turn = 0;
    while (turn < maxTurns && !fleet.empty() && raiderHull > 0) {
        turn++;
        std::cout << "\n--- Turn " << turn << " ---" << std::endl;
        if (realTime) {
            std::cout << "Waiting 60 seconds for next combat turn..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(60));
        }
        for (auto &host : fleet) {
            if (host.type != "Repair Drone" && host.type != "Sunflare Sloop" && host.dockedSupport.empty()) {
                for (auto it = fleet.begin(); it != fleet.end(); ++it) {
                    if (it->type == "Sunflare Sloop" && it->dockedSupport.empty() && (&(*it) != &host)) {
                        host.dockedSupport = "Sunflare Sloop";
                        std::cout << "Sunflare Sloop docked to " << host.type << "." << std::endl;
                        it->type = "";
                        break;
                    }
                }
            }
        }
        for (auto &ship : fleet) {
            if (!ship.dockedSupport.empty() && ship.dockedSupport == "Sunflare Sloop") {
                int oldShield = ship.currentShield;
                ship.currentShield = std::min(ship.maxShield, ship.currentShield + SUNFLARE_SHIELD_REGEN);
                if (ship.currentShield > oldShield)
                    std::cout << ship.type << " receives " << (ship.currentShield - oldShield)
                         << " shield regen from docked Sunflare Sloop." << std::endl;
            }
        }
        int totalDamage = 0;
        for (const auto &ship : fleet) {
            if (ship.type == "Corvette" || ship.type == "Shield Ship" ||
                ship.type == "Radar Ship" || ship.type == "Salvage Ship")
                totalDamage += ship.weapons;
        }
        std::cout << "Your defenders fire for a total of " << totalDamage << " damage." << std::endl;
        int shieldDamage = std::min(totalDamage, raiderShield);
        raiderShield -= shieldDamage;
        int remainingDamage = totalDamage - shieldDamage;
        raiderHull -= remainingDamage;
        std::cout << "Raider shields take " << shieldDamage << " damage, remaining: " << raiderShield << std::endl;
        if (remainingDamage > 0)
            std::cout << "Raider hull takes " << remainingDamage << " damage, remaining: " << raiderHull << std::endl;
        if (raiderHull <= 0) {
            std::cout << "Raiders defeated!" << std::endl;
            Planet *terra = pm.getPlanetByName("Terra");
            if (terra) {
                terra->addToStorage("Engine Parts", 2);
                std::cout << "You receive 2 Engine Parts as reward." << std::endl;
            }
            completed_ = true;
            target->setUnderThreat(false);
            return true;
        }
        // Raider counter attack simplified for brevity
        turn++;
    }
    if (turn >= maxTurns) {
        std::cout << "Combat ended after 10 turns." << std::endl;
        completed_ = true;
        target->setUnderThreat(false);
    }
    return completed_;
}

bool DailyQuest::isCompleted() const { return completed_; }
const std::string &DailyQuest::getDescription() const { return description_; }
const std::string &DailyQuest::getObjectiveResource() const { return objectiveResource_; }
int DailyQuest::getObjectiveAmount() const { return objectiveAmount_; }
const std::map<std::string,int> &DailyQuest::getReward() const { return reward_; }
bool DailyQuest::isRaider() const { return isRaiderAttack_; }
void DailyQuest::setRaiderAttack(const std::string &target) { isRaiderAttack_ = true; targetPlanet_ = target; }
const std::string &DailyQuest::getTargetPlanet() const { return targetPlanet_; }
void DailyQuest::setStoryQuest(bool val) { isStoryQuest_ = val; }
bool DailyQuest::isStoryQuest() const { return isStoryQuest_; }
void DailyQuest::setFinalConfrontation(bool val) { isFinalConfrontation_ = val; }
bool DailyQuest::isFinalConfrontation() const { return isFinalConfrontation_; }

QuestManager::QuestManager() : currentQuest_(nullptr), lastQuestDate_(""), storyStage_(0) {}

void QuestManager::updateDailyQuest(Planet *central, PlanetManager &pm, std::vector<Ship> &fleet, bool escapePodActive) {
    std::string todayStr = currentDateString();
    if (lastQuestDate_.empty() || lastQuestDate_ < todayStr) {
        generateNewQuest(pm);
        lastQuestDate_ = todayStr;
    }
    if (currentQuest_) {
        bool realTime = true;
        if (currentQuest_->isRaider())
            currentQuest_->processRaiderAttack(pm, fleet, realTime, escapePodActive);
        else
            currentQuest_->checkCompletion(central);
        if (currentQuest_->isCompleted() && currentQuest_->isStoryQuest()) {
            switch(storyStage_) {
                case 0:
                    journal.addEntry("The Spark Ignited",
                        "Old Miner Joe speaks with quiet resolve as you deliver 50 Iron Bars. 'Each bar carries the weight of our past struggles,' he says.");
                    break;
                case 1:
                    journal.addEntry("A Cosmic Warning",
                        "Professor Lumen warns: 'Cosmic disturbances near Mars demand our attention.'");
                    break;
                case 2:
                    journal.addEntry("Bandit Outpost Assault",
                        "Farmer Daisy urges: 'Disrupt the bandit operations in Zalthor!'");
                    break;
                case 3:
                    journal.addEntry("Shadows Over Vulcan",
                        "On Vulcan, clues of betrayal in the raider ranks emerge.");
                    break;
                case 4:
                    journal.addEntry("Convoy Under Fire",
                        "Defend Terra’s convoys from raider ambush!");
                    break;
                case 5:
                    journal.addEntry("Echoes of Betrayal",
                        "Professor Lumen reveals a hidden raider outpost on Mars.");
                    break;
                case 6:
                    journal.addEntry("Broken Chains",
                        "Old Miner Joe challenges you to collect 30 Mithril Bars.");
                    break;
                case 7:
                    journal.addEntry("Siege of the Forgotten",
                        "Under Luna’s glow, your forces siege a raider stronghold.");
                    break;
                case 8:
                    journal.addEntry("Whispers in the Void",
                        "Intercepted transmissions hint at a deep raider wound.");
                    break;
                case 9:
                    journal.addEntry("The Great Siege",
                        "Assault the fortified raider outpost on Mars.");
                    break;
                case 10:
                    journal.addEntry("Rising Tempest",
                        "Merchant guilds cry out as defenses are bolstered.");
                    break;
                case 11:
                    journal.addEntry("The Final Stand",
                        "At last, you confront Captain Blackthorne.");
                    break;
                default:
                    journal.addEntry("Unknown Stage", "The story continues beyond known bounds.");
                    break;
            }
            storyStage_++;
            currentQuest_.reset();
        }
    }
}

DailyQuest *QuestManager::getCurrentQuest() { return currentQuest_.get(); }
const DailyQuest *QuestManager::getCurrentQuestConst() const { return currentQuest_.get(); }
const std::string &QuestManager::getLastQuestDate() const { return lastQuestDate_; }
void QuestManager::setLastQuestDate(const std::string &s) { lastQuestDate_ = s; }
void QuestManager::setCurrentQuest(std::unique_ptr<DailyQuest> q) { currentQuest_ = std::move(q); }

void QuestManager::generateNewQuest(PlanetManager &pm) {
    if (storyStage_ < 12) {
        if (storyStage_ == 0) {
            std::string desc = "Old Miner Joe says: 'Deep in Terra's veins lie the secrets of redemption. Collect 50 Iron Bars to prove your resolve.'";
            std::map<std::string,int> reward = {{"Engine Parts", 3}};
            auto quest = std::make_unique<DailyQuest>(desc, "Iron Bar", 50, reward);
            quest->setStoryQuest(true);
            currentQuest_ = std::move(quest);
        } else if (storyStage_ == 1) {
            pm.unlockPlanet("Mars");
            std::string desc = "Professor Lumen warns: 'Cosmic anomalies stir near Mars. Prepare for a raider attack!'";
            std::map<std::string,int> reward = {{"Engine Parts", 2}};
            auto quest = std::make_unique<DailyQuest>(desc, "", 1, reward);
            quest->setRaiderAttack("Mars");
            quest->setStoryQuest(true);
            currentQuest_ = std::move(quest);
        } else {
            // Additional stages trimmed for brevity
            currentQuest_.reset();
        }
    } else {
        if ((rand() % 100) < 20) {
            std::vector<std::string> candidates;
            for (auto &planet : pm.getPlanets())
                if (planet.isUnlocked() && planet.getName() != "Terra")
                    candidates.push_back(planet.getName());
            if (candidates.empty())
                candidates.push_back("Terra");
            std::string target = candidates[rand() % candidates.size()];
            std::string desc = "Urgent alert: Raiders are approaching " + target + "! Prepare your defenses!";
            std::map<std::string,int> reward = {{"Engine Parts", 2}};
            auto quest = std::make_unique<DailyQuest>(desc, "", 1, reward);
            quest->setRaiderAttack(target);
            Planet *tgt = pm.getPlanetByName(target);
            if (tgt && tgt->hasBuilding("Proximity Alarm")) {
                std::cout << "Proximity Alarm on " << target << " issues a 5-minute warning of an imminent raider attack!" << std::endl;
                std::this_thread::sleep_for(std::chrono::minutes(5));
                tgt->setUnderThreat(true);
            }
            journal.addEntry("Raider Warning", "Intelligence reports hint at an emerging threat near " + target + ".");
            currentQuest_ = std::move(quest);
        } else {
            std::vector<std::string> resourceChoices = {"Iron Bar", "Copper Bar", "Mithril Bar", "Titanium Bar", "Advanced Engine Parts"};
            int idx = rand() % resourceChoices.size();
            std::string chosen = resourceChoices[idx];
            int amt = rand() % 41 + 10;
            std::map<std::string,int> reward = {{"Engine Parts", (rand() % 3) + 1}};
            std::string desc = "Side quest: Collect " + std::to_string(amt) + " " + chosen;
            currentQuest_ = std::make_unique<DailyQuest>(desc, chosen, amt, reward);
        }
    }
}

std::string QuestManager::currentDateString() {
    time_t now = time(nullptr);
    tm *gmt = gmtime(&now);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", gmt);
    return std::string(buf);
}

