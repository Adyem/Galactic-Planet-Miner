#ifndef QUEST_H
#define QUEST_H

#include "planet.h"
#include "journal.h"
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <ctime>

class DailyQuest {
public:
    DailyQuest(const std::string &desc, const std::string &objectiveRes, int objectiveAmt, const std::map<std::string,int> &reward);
    void checkCompletion(Planet *central);
    bool processRaiderAttack(PlanetManager &pm, std::vector<Ship> &fleet, bool realTime, bool escapePodActive);
    bool isCompleted() const;
    const std::string &getDescription() const;
    const std::string &getObjectiveResource() const;
    int getObjectiveAmount() const;
    const std::map<std::string,int> &getReward() const;
    bool isRaider() const;
    void setRaiderAttack(const std::string &target);
    const std::string &getTargetPlanet() const;
    void setStoryQuest(bool val);
    bool isStoryQuest() const;
    void setFinalConfrontation(bool val);
    bool isFinalConfrontation() const;
private:
    std::string description_;
    std::string objectiveResource_;
    int objectiveAmount_;
    std::map<std::string,int> reward_;
    bool completed_;
    bool isRaiderAttack_;
    std::string targetPlanet_;
    time_t combatStartTime_;
    int turnsElapsed_;
    bool isStoryQuest_;
    bool isFinalConfrontation_;
};

class QuestManager {
public:
    QuestManager();
    void updateDailyQuest(Planet *central, PlanetManager &pm, std::vector<Ship> &fleet, bool escapePodActive);
    DailyQuest *getCurrentQuest();
    const DailyQuest *getCurrentQuestConst() const;
    const std::string &getLastQuestDate() const;
    void setLastQuestDate(const std::string &s);
    void setCurrentQuest(std::unique_ptr<DailyQuest> q);
private:
    std::unique_ptr<DailyQuest> currentQuest_;
    std::string lastQuestDate_;
    int storyStage_;
    void generateNewQuest(PlanetManager &pm);
    std::string currentDateString();
};

#endif // QUEST_H
