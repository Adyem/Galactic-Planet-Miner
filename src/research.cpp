#include "research.h"

Research::Research(const std::string &name, const std::map<std::string,int> &cost, const std::string &description, const std::string &effectName)
    : name_(name), cost_(cost), description_(description), effectName_(effectName), completed_(false) {}

bool Research::canResearch(Planet *central) const {
    for (const auto &entry : cost_)
        if (central->getStored(entry.first) < entry.second)
            return false;
    return true;
}

void Research::doResearch(Planet *central) {
    for (const auto &entry : cost_)
        central->removeFromStorage(entry.first, entry.second);
    completed_ = true;
}

const std::string &Research::getName() const { return name_; }
const std::string &Research::getEffectName() const { return effectName_; }
bool Research::isCompleted() const { return completed_; }
void Research::setCompleted(bool val) { completed_ = val; }
const std::map<std::string,int> &Research::getCost() const { return cost_; }
const std::string &Research::getDescription() const { return description_; }

ResearchManager::ResearchManager(const std::vector<ResearchDef> &data) {
    for (const auto &rd : data)
        researches_.push_back(Research(rd.name, rd.cost, rd.description, rd.effectName));
}

Research *ResearchManager::findResearchByName(const std::string &name) {
    for (auto &research : researches_)
        if (research.getName() == name)
            return &research;
    return nullptr;
}

std::vector<Research> &ResearchManager::getAllResearches() { return researches_; }
const std::vector<Research> &ResearchManager::getAllResearchesConst() const { return researches_; }

