struct ResearchCostEntry
{
    int resource_id;
    int amount;
};

struct ResearchTemplate
{
    int                             id;
    const char                      *name;
    double                          duration;
    const int                       *prerequisites;
    size_t                          prerequisite_count;
    const ResearchCostEntry         *costs;
    size_t                          cost_count;
    const int                       *unlock_planets;
    size_t                          unlock_planet_count;
};

static void copy_ids(const int *data, size_t count, ft_vector<int> &out)
{
    out.clear();
    for (size_t i = 0; i < count; ++i)
        out.push_back(data[i]);
}

static void copy_costs(const ResearchCostEntry *data, size_t count, ft_vector<Pair<int, int> > &out)
{
    out.clear();
    for (size_t i = 0; i < count; ++i)
    {
        Pair<int, int> entry;
        entry.key = data[i].resource_id;
        entry.value = data[i].amount;
        out.push_back(entry);
    }
}

static const ResearchCostEntry kMarsCosts[] = {
    {ORE_IRON, 18},
    {ORE_COPPER, 12},
    {ORE_COAL, 6}
};

static const int kMarsUnlockPlanets[] = {PLANET_MARS};

static const int kZalthorPrereqs[] = {RESEARCH_UNLOCK_MARS};
static const ResearchCostEntry kZalthorCosts[] = {
    {ORE_MITHRIL, 8},
    {ORE_COAL, 6}
};
static const int kZalthorUnlockPlanets[] = {PLANET_ZALTHOR};

static const int kVulcanPrereqs[] = {RESEARCH_UNLOCK_ZALTHOR};
static const ResearchCostEntry kVulcanCosts[] = {
    {ORE_GOLD, 6},
    {ORE_MITHRIL, 8}
};
static const int kVulcanUnlockPlanets[] = {PLANET_VULCAN};

static const int kNoctarisPrereqs[] = {RESEARCH_UNLOCK_VULCAN};
static const ResearchCostEntry kNoctarisCosts[] = {
    {ORE_TITANIUM, 5},
    {ORE_SILVER, 6},
    {ORE_TIN, 6}
};
static const int kNoctarisUnlockPlanets[] = {PLANET_NOCTARIS_PRIME};

static const ResearchCostEntry kPlanningTerraCosts[] = {
    {ITEM_IRON_BAR, 15},
    {ITEM_ENGINE_PART, 5}
};

static const int kPlanningMarsPrereqs[] = {RESEARCH_UNLOCK_MARS};
static const ResearchCostEntry kPlanningMarsCosts[] = {
    {ITEM_IRON_BAR, 30},
    {ITEM_ENGINE_PART, 10}
};

static const int kPlanningZalthorPrereqs[] = {RESEARCH_UNLOCK_ZALTHOR};
static const ResearchCostEntry kPlanningZalthorCosts[] = {
    {ITEM_IRON_BAR, 45},
    {ITEM_ENGINE_PART, 15}
};

static const int kSolarPrereqs[] = {RESEARCH_UNLOCK_MARS};
static const ResearchCostEntry kSolarCosts[] = {
    {ORE_IRON, 20},
    {ORE_COPPER, 30},
    {ITEM_ACCUMULATOR, 2}
};

static const int kMasteryPrereqs[] = {RESEARCH_SOLAR_PANELS};
static const ResearchCostEntry kMasteryCosts[] = {
    {ITEM_ADVANCED_ENGINE_PART, 5},
    {ITEM_TITANIUM_BAR, 10},
    {ITEM_FUSION_REACTOR, 1}
};

static const int kTradePrereqs[] = {RESEARCH_CRAFTING_MASTERY};
static const ResearchCostEntry kTradeCosts[] = {
    {ITEM_ADVANCED_ENGINE_PART, 6},
    {ITEM_ACCUMULATOR, 3},
    {ITEM_TITANIUM_BAR, 6}
};

static const int kStructuralIPrereqs[] = {RESEARCH_UNLOCK_MARS};
static const ResearchCostEntry kStructuralICosts[] = {
    {ITEM_IRON_BAR, 10},
    {ORE_COAL, 10}
};

static const int kStructuralIIPrereqs[] = {RESEARCH_STRUCTURAL_REINFORCEMENT_I};
static const ResearchCostEntry kStructuralIICosts[] = {
    {ITEM_IRON_BAR, 20},
    {ORE_COAL, 20}
};

static const int kStructuralIIIPrereqs[] = {RESEARCH_STRUCTURAL_REINFORCEMENT_II};
static const ResearchCostEntry kStructuralIIICosts[] = {
    {ITEM_IRON_BAR, 30},
    {ORE_COAL, 30}
};

static const int kDefensiveIPrereqs[] = {RESEARCH_UNLOCK_ZALTHOR};
static const ResearchCostEntry kDefensiveICosts[] = {
    {ITEM_COPPER_BAR, 10},
    {ITEM_MITHRIL_BAR, 5}
};

static const int kDefensiveIIPrereqs[] = {RESEARCH_DEFENSIVE_FORTIFICATION_I};
static const ResearchCostEntry kDefensiveIICosts[] = {
    {ITEM_COPPER_BAR, 20},
    {ITEM_MITHRIL_BAR, 10}
};

static const int kDefensiveIIIPrereqs[] = {RESEARCH_DEFENSIVE_FORTIFICATION_II};
static const ResearchCostEntry kDefensiveIIICosts[] = {
    {ITEM_COPPER_BAR, 30},
    {ITEM_MITHRIL_BAR, 15}
};

static const int kArmamentIPrereqs[] = {RESEARCH_UNLOCK_VULCAN};
static const ResearchCostEntry kArmamentICosts[] = {
    {ITEM_ADVANCED_ENGINE_PART, 5},
    {ITEM_TITANIUM_BAR, 5}
};

static const int kArmamentIIPrereqs[] = {RESEARCH_ARMAMENT_ENHANCEMENT_I};
static const ResearchCostEntry kArmamentIICosts[] = {
    {ITEM_ADVANCED_ENGINE_PART, 10},
    {ITEM_TITANIUM_BAR, 10},
    {ORE_TRITIUM, 5}
};

static const int kArmamentIIIPrereqs[] = {RESEARCH_ARMAMENT_ENHANCEMENT_II};
static const ResearchCostEntry kArmamentIIICosts[] = {
    {ITEM_ADVANCED_ENGINE_PART, 15},
    {ITEM_TITANIUM_BAR, 15},
    {ORE_TRITIUM, 10},
    {ITEM_FUSION_REACTOR, 1}
};

static const int kLunaPrereqs[] = {RESEARCH_UNLOCK_MARS};
static const ResearchCostEntry kLunaCosts[] = {
    {ORE_IRON, 25},
    {ORE_COPPER, 20},
    {ITEM_ENGINE_PART, 6}
};
static const int kLunaUnlockPlanets[] = {PLANET_LUNA};

static const int kFasterCraftingPrereqs[] = {RESEARCH_CRAFTING_MASTERY};
static const ResearchCostEntry kFasterCraftingCosts[] = {
    {ITEM_ADVANCED_ENGINE_PART, 8},
    {ITEM_TITANIUM_BAR, 12},
    {ITEM_ACCUMULATOR, 4}
};

static const int kShieldPrereqs[] = {RESEARCH_DEFENSIVE_FORTIFICATION_I};
static const ResearchCostEntry kShieldCosts[] = {
    {ITEM_COPPER_BAR, 15},
    {ITEM_MITHRIL_BAR, 12},
    {ITEM_ACCUMULATOR, 2}
};

static const int kEmergencyPrereqs[] = {RESEARCH_SOLAR_PANELS};
static const ResearchCostEntry kEmergencyCosts[] = {
    {ORE_COAL, 20},
    {ITEM_ACCUMULATOR, 3}
};

static const int kRepairPrereqs[] = {
    RESEARCH_SHIELD_TECHNOLOGY,
    RESEARCH_DEFENSIVE_FORTIFICATION_II
};
static const ResearchCostEntry kRepairCosts[] = {
    {ITEM_ADVANCED_ENGINE_PART, 6},
    {ITEM_ACCUMULATOR, 4},
    {ITEM_FUSION_REACTOR, 1}
};

static const int kTritiumPrereqs[] = {RESEARCH_UNLOCK_NOCTARIS};
static const ResearchCostEntry kTritiumCosts[] = {
    {ORE_OBSIDIAN, 12},
    {ORE_CRYSTAL, 6},
    {ITEM_ACCUMULATOR, 3}
};

static const int kCapitalPrereqs[] = {RESEARCH_ARMAMENT_ENHANCEMENT_II};
static const ResearchCostEntry kCapitalCosts[] = {
    {ITEM_ADVANCED_ENGINE_PART, 12},
    {ITEM_TITANIUM_BAR, 12},
    {ITEM_FUSION_REACTOR, 2}
};

static const int kAuxiliaryPrereqs[] = {RESEARCH_CAPITAL_SHIP_INITIATIVE};
static const ResearchCostEntry kAuxiliaryCosts[] = {
    {ITEM_ADVANCED_ENGINE_PART, 15},
    {ITEM_TITANIUM_BAR, 15},
    {ORE_TRITIUM, 6},
    {ITEM_FUSION_REACTOR, 2}
};

static const int kEscapePrereqs[] = {RESEARCH_REPAIR_DRONE_TECHNOLOGY};
static const ResearchCostEntry kEscapeCosts[] = {
    {ITEM_ACCUMULATOR, 5},
    {ORE_TRITIUM, 4},
    {ITEM_FUSION_REACTOR, 1}
};

static const ResearchTemplate kResearchTemplates[] = {
    {RESEARCH_UNLOCK_MARS, "Unlock Mars", 30.0, ft_nullptr, 0, kMarsCosts, sizeof(kMarsCosts) / sizeof(kMarsCosts[0]), kMarsUnlockPlanets, sizeof(kMarsUnlockPlanets) / sizeof(kMarsUnlockPlanets[0])},
    {RESEARCH_UNLOCK_ZALTHOR, "Unlock Zalthor", 40.0, kZalthorPrereqs, sizeof(kZalthorPrereqs) / sizeof(kZalthorPrereqs[0]), kZalthorCosts, sizeof(kZalthorCosts) / sizeof(kZalthorCosts[0]), kZalthorUnlockPlanets, sizeof(kZalthorUnlockPlanets) / sizeof(kZalthorUnlockPlanets[0])},
    {RESEARCH_UNLOCK_VULCAN, "Unlock Vulcan", 55.0, kVulcanPrereqs, sizeof(kVulcanPrereqs) / sizeof(kVulcanPrereqs[0]), kVulcanCosts, sizeof(kVulcanCosts) / sizeof(kVulcanCosts[0]), kVulcanUnlockPlanets, sizeof(kVulcanUnlockPlanets) / sizeof(kVulcanUnlockPlanets[0])},
    {RESEARCH_UNLOCK_NOCTARIS, "Unlock Noctaris Prime", 60.0, kNoctarisPrereqs, sizeof(kNoctarisPrereqs) / sizeof(kNoctarisPrereqs[0]), kNoctarisCosts, sizeof(kNoctarisCosts) / sizeof(kNoctarisCosts[0]), kNoctarisUnlockPlanets, sizeof(kNoctarisUnlockPlanets) / sizeof(kNoctarisUnlockPlanets[0])},
    {RESEARCH_URBAN_PLANNING_TERRA, "Urban Planning I", 20.0, ft_nullptr, 0, kPlanningTerraCosts, sizeof(kPlanningTerraCosts) / sizeof(kPlanningTerraCosts[0]), ft_nullptr, 0},
    {RESEARCH_URBAN_PLANNING_MARS, "Urban Planning II", 25.0, kPlanningMarsPrereqs, sizeof(kPlanningMarsPrereqs) / sizeof(kPlanningMarsPrereqs[0]), kPlanningMarsCosts, sizeof(kPlanningMarsCosts) / sizeof(kPlanningMarsCosts[0]), ft_nullptr, 0},
    {RESEARCH_URBAN_PLANNING_ZALTHOR, "Urban Planning III", 30.0, kPlanningZalthorPrereqs, sizeof(kPlanningZalthorPrereqs) / sizeof(kPlanningZalthorPrereqs[0]), kPlanningZalthorCosts, sizeof(kPlanningZalthorCosts) / sizeof(kPlanningZalthorCosts[0]), ft_nullptr, 0},
    {RESEARCH_SOLAR_PANELS, "Solar Panel Engineering", 25.0, kSolarPrereqs, sizeof(kSolarPrereqs) / sizeof(kSolarPrereqs[0]), kSolarCosts, sizeof(kSolarCosts) / sizeof(kSolarCosts[0]), ft_nullptr, 0},
    {RESEARCH_CRAFTING_MASTERY, "Crafting Mastery", 35.0, kMasteryPrereqs, sizeof(kMasteryPrereqs) / sizeof(kMasteryPrereqs[0]), kMasteryCosts, sizeof(kMasteryCosts) / sizeof(kMasteryCosts[0]), ft_nullptr, 0},
    {RESEARCH_INTERSTELLAR_TRADE, "Interstellar Trade Networks", 32.0, kTradePrereqs, sizeof(kTradePrereqs) / sizeof(kTradePrereqs[0]), kTradeCosts, sizeof(kTradeCosts) / sizeof(kTradeCosts[0]), ft_nullptr, 0},
    {RESEARCH_STRUCTURAL_REINFORCEMENT_I, "Structural Reinforcement I", 25.0, kStructuralIPrereqs, sizeof(kStructuralIPrereqs) / sizeof(kStructuralIPrereqs[0]), kStructuralICosts, sizeof(kStructuralICosts) / sizeof(kStructuralICosts[0]), ft_nullptr, 0},
    {RESEARCH_STRUCTURAL_REINFORCEMENT_II, "Structural Reinforcement II", 35.0, kStructuralIIPrereqs, sizeof(kStructuralIIPrereqs) / sizeof(kStructuralIIPrereqs[0]), kStructuralIICosts, sizeof(kStructuralIICosts) / sizeof(kStructuralIICosts[0]), ft_nullptr, 0},
    {RESEARCH_STRUCTURAL_REINFORCEMENT_III, "Structural Reinforcement III", 45.0, kStructuralIIIPrereqs, sizeof(kStructuralIIIPrereqs) / sizeof(kStructuralIIIPrereqs[0]), kStructuralIIICosts, sizeof(kStructuralIIICosts) / sizeof(kStructuralIIICosts[0]), ft_nullptr, 0},
    {RESEARCH_DEFENSIVE_FORTIFICATION_I, "Defensive Fortification I", 30.0, kDefensiveIPrereqs, sizeof(kDefensiveIPrereqs) / sizeof(kDefensiveIPrereqs[0]), kDefensiveICosts, sizeof(kDefensiveICosts) / sizeof(kDefensiveICosts[0]), ft_nullptr, 0},
    {RESEARCH_DEFENSIVE_FORTIFICATION_II, "Defensive Fortification II", 40.0, kDefensiveIIPrereqs, sizeof(kDefensiveIIPrereqs) / sizeof(kDefensiveIIPrereqs[0]), kDefensiveIICosts, sizeof(kDefensiveIICosts) / sizeof(kDefensiveIICosts[0]), ft_nullptr, 0},
    {RESEARCH_DEFENSIVE_FORTIFICATION_III, "Defensive Fortification III", 50.0, kDefensiveIIIPrereqs, sizeof(kDefensiveIIIPrereqs) / sizeof(kDefensiveIIIPrereqs[0]), kDefensiveIIICosts, sizeof(kDefensiveIIICosts) / sizeof(kDefensiveIIICosts[0]), ft_nullptr, 0},
    {RESEARCH_ARMAMENT_ENHANCEMENT_I, "Armament Enhancement I", 35.0, kArmamentIPrereqs, sizeof(kArmamentIPrereqs) / sizeof(kArmamentIPrereqs[0]), kArmamentICosts, sizeof(kArmamentICosts) / sizeof(kArmamentICosts[0]), ft_nullptr, 0},
    {RESEARCH_ARMAMENT_ENHANCEMENT_II, "Armament Enhancement II", 45.0, kArmamentIIPrereqs, sizeof(kArmamentIIPrereqs) / sizeof(kArmamentIIPrereqs[0]), kArmamentIICosts, sizeof(kArmamentIICosts) / sizeof(kArmamentIICosts[0]), ft_nullptr, 0},
    {RESEARCH_ARMAMENT_ENHANCEMENT_III, "Armament Enhancement III", 55.0, kArmamentIIIPrereqs, sizeof(kArmamentIIIPrereqs) / sizeof(kArmamentIIIPrereqs[0]), kArmamentIIICosts, sizeof(kArmamentIIICosts) / sizeof(kArmamentIIICosts[0]), ft_nullptr, 0},
    {RESEARCH_UNLOCK_LUNA, "Unlock Luna", 35.0, kLunaPrereqs, sizeof(kLunaPrereqs) / sizeof(kLunaPrereqs[0]), kLunaCosts, sizeof(kLunaCosts) / sizeof(kLunaCosts[0]), kLunaUnlockPlanets, sizeof(kLunaUnlockPlanets) / sizeof(kLunaUnlockPlanets[0])},
    {RESEARCH_FASTER_CRAFTING, "Faster Crafting", 40.0, kFasterCraftingPrereqs, sizeof(kFasterCraftingPrereqs) / sizeof(kFasterCraftingPrereqs[0]), kFasterCraftingCosts, sizeof(kFasterCraftingCosts) / sizeof(kFasterCraftingCosts[0]), ft_nullptr, 0},
    {RESEARCH_SHIELD_TECHNOLOGY, "Shield Technology", 35.0, kShieldPrereqs, sizeof(kShieldPrereqs) / sizeof(kShieldPrereqs[0]), kShieldCosts, sizeof(kShieldCosts) / sizeof(kShieldCosts[0]), ft_nullptr, 0},
    {RESEARCH_EMERGENCY_ENERGY_CONSERVATION, "Emergency Energy Conservation", 30.0, kEmergencyPrereqs, sizeof(kEmergencyPrereqs) / sizeof(kEmergencyPrereqs[0]), kEmergencyCosts, sizeof(kEmergencyCosts) / sizeof(kEmergencyCosts[0]), ft_nullptr, 0},
    {RESEARCH_REPAIR_DRONE_TECHNOLOGY, "Repair Drone Technology", 45.0, kRepairPrereqs, sizeof(kRepairPrereqs) / sizeof(kRepairPrereqs[0]), kRepairCosts, sizeof(kRepairCosts) / sizeof(kRepairCosts[0]), ft_nullptr, 0},
    {RESEARCH_TRITIUM_EXTRACTION, "Tritium Extraction", 50.0, kTritiumPrereqs, sizeof(kTritiumPrereqs) / sizeof(kTritiumPrereqs[0]), kTritiumCosts, sizeof(kTritiumCosts) / sizeof(kTritiumCosts[0]), ft_nullptr, 0},
    {RESEARCH_CAPITAL_SHIP_INITIATIVE, "Capital Ship Initiative", 55.0, kCapitalPrereqs, sizeof(kCapitalPrereqs) / sizeof(kCapitalPrereqs[0]), kCapitalCosts, sizeof(kCapitalCosts) / sizeof(kCapitalCosts[0]), ft_nullptr, 0},
    {RESEARCH_AUXILIARY_FRIGATE_DEVELOPMENT, "Auxiliary Frigate Development", 60.0, kAuxiliaryPrereqs, sizeof(kAuxiliaryPrereqs) / sizeof(kAuxiliaryPrereqs[0]), kAuxiliaryCosts, sizeof(kAuxiliaryCosts) / sizeof(kAuxiliaryCosts[0]), ft_nullptr, 0},
    {RESEARCH_ESCAPE_POD_LIFELINE, "Escape Pod Lifeline", 45.0, kEscapePrereqs, sizeof(kEscapePrereqs) / sizeof(kEscapePrereqs[0]), kEscapeCosts, sizeof(kEscapeCosts) / sizeof(kEscapeCosts[0]), ft_nullptr, 0}
};

static void collect_research_definitions(ft_vector<ft_sharedptr<ft_research_definition> > &out)
{
    out.clear();
    size_t template_count = sizeof(kResearchTemplates) / sizeof(kResearchTemplates[0]);
    for (size_t i = 0; i < template_count; ++i)
    {
        const ResearchTemplate &entry = kResearchTemplates[i];
        ft_sharedptr<ft_research_definition> definition(new ft_research_definition());
        definition->id = entry.id;
        definition->name = ft_string(entry.name);
        definition->duration = entry.duration;
        copy_ids(entry.prerequisites, entry.prerequisite_count, definition->prerequisites);
        copy_costs(entry.costs, entry.cost_count, definition->costs);
        copy_ids(entry.unlock_planets, entry.unlock_planet_count, definition->unlock_planets);
        out.push_back(definition);
    }
}
