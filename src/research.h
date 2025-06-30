#ifndef RESEARCH_H
#define RESEARCH_H

#include "planet.h"
#include "game_data.h"
#include <string>
#include <vector>
#include <map>

class Research {
public:
    Research(const std::string &name, const std::map<std::string,int> &cost, const std::string &description, const std::string &effectName);
    bool canResearch(Planet *central) const;
    void doResearch(Planet *central);
    const std::string &getName() const;
    const std::string &getEffectName() const;
    bool isCompleted() const;
    void setCompleted(bool val);
    const std::map<std::string,int> &getCost() const;
    const std::string &getDescription() const;
private:
    std::string name_;
    std::map<std::string,int> cost_;
    std::string description_;
    std::string effectName_;
    bool completed_;
};

class ResearchManager {
public:
    ResearchManager(const std::vector<ResearchDef> &data);
    Research *findResearchByName(const std::string &name);
    std::vector<Research> &getAllResearches();
    const std::vector<Research> &getAllResearchesConst() const;
private:
    std::vector<Research> researches_;
};

#endif // RESEARCH_H
