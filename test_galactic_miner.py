#!/usr/bin/env python3
"""
Expanded Modular Idle Planet Miner – Python/Tkinter Version
------------------------------------------------------------
This is a full conversion (in spirit) of the provided C++ code. It implements:
  - Planets with resource production and energy management
  - Research and crafting (with recipes for items, ships, and buildings)
  - A quest system (daily and story quests)
  - Fleet management and rudimentary combat (with raider attacks)
  - A journal (lore entries)
  - Save/load functionality (JSON)
  - A Tkinter–based GUI interface instead of a terminal command line

Author: ChatGPT
Date: 2025-02-02
"""

import tkinter as tk
from tkinter import simpledialog, messagebox, scrolledtext
import json, time, random, math, os

# ---------------------------
# Global Lore and Data Values
# ---------------------------

resourceLore = [
    "Old Miner Joe: 'The veins of our beloved planet run deep—every ounce of ore fuels our future. I once lost everything to a mining accident, and now I seek redemption in every pick strike.'",
    "Professor Lumen: 'Mining is the backbone of our civilization. Behind every ore lies a story of sacrifice and hope, echoing the trials of those who came before us.'",
    "Farmer Daisy: 'Though I tend my fields, I know that without the mines, our tables would be empty. The earth gives generously, yet it also demands respect for its hidden treasures.'"
]

raiderLore = [
    "Captain Blackthorne: 'I never chose this path. Forced into desperation by a system that failed us, we strike not out of malice, but as a cry for justice.'",
    "Navigator Zara: 'Every course we chart is a reminder of the choices we were left with. We become raiders because survival left us no other way, not because we relish conflict.'",
    "Old Scout Finn: 'In the void, I've seen the truth—our fury is the echo of neglect, a tragic response by souls abandoned by hope.'"
]

genericLore = [
    "Town Crier: 'The times are hard, but unity and perseverance will see us through. Remember, every challenge is an opportunity in disguise.'",
    "Scribe Alric: 'Every day brings new challenges—and new opportunities—for those brave enough to seize them. History is written by those who dare to dream.'"
]

SAVE_FILE = "savegame.json"

# Resource definitions – a list of resource names (each starting at 0)
RESOURCE_DATA = [
    {"name": "Iron", "initial": 0},
    {"name": "Copper", "initial": 0},
    {"name": "Mithril", "initial": 0},
    {"name": "Coal", "initial": 0},
    {"name": "Tin", "initial": 0},
    {"name": "Silver", "initial": 0},
    {"name": "Gold", "initial": 0},
    {"name": "Iron Bar", "initial": 0},
    {"name": "Copper Bar", "initial": 0},
    {"name": "Mithril Bar", "initial": 0},
    {"name": "Engine Parts", "initial": 0},
    {"name": "Titanium", "initial": 0},
    {"name": "Titanium Bar", "initial": 0},
    {"name": "Generator", "initial": 0},
    {"name": "Accumulator", "initial": 0},
    {"name": "Obsidian", "initial": 0},
    {"name": "Crystal", "initial": 0},
    {"name": "Nanomaterial", "initial": 0},
    {"name": "Advanced Engine Parts", "initial": 0},
    {"name": "Fusion Reactor", "initial": 0},
    {"name": "Tritium", "initial": 0}
]

# Planet definitions
PLANET_DATA = [
    {"name": "Terra", "baseProduction": {"Iron": 0.5, "Copper": 0.5, "Coal": 0.2}, "unlocked": True},
    {"name": "Mars", "baseProduction": {"Iron": 0.1, "Copper": 0.1, "Mithril": 0.05, "Coal": 0.1}, "unlocked": False},
    {"name": "Zalthor", "baseProduction": {"Mithril": 0.1, "Coal": 0.2, "Gold": 0.02}, "unlocked": False},
    {"name": "Vulcan", "baseProduction": {"Tin": 0.1, "Silver": 0.03, "Titanium": 0.01}, "unlocked": False},
    {"name": "Luna", "baseProduction": {"Obsidian": 0.1, "Crystal": 0.05, "Nanomaterial": 0.02}, "unlocked": False}
]

# Research definitions
RESEARCH_DATA = [
    {"name": "Unlock Mars", "cost": {"Iron": 100, "Copper": 50},
     "description": "Unlock the planet Mars for mining.", "effectName": "unlock_mars"},
    {"name": "Unlock Zalthor", "cost": {"Iron": 200, "Mithril": 50},
     "description": "Unlock planet Zalthor for advanced mining.", "effectName": "unlock_zalthor"},
    {"name": "Faster Crafting", "cost": {"Iron": 50, "Coal": 30},
     "description": "Reduce smelting time by half.", "effectName": "faster_crafting"},
    {"name": "Unlock Vulcan", "cost": {"Gold": 100, "Mithril": 100},
     "description": "Unlock planet Vulcan for high‑value resources.", "effectName": "unlock_vulcan"},
    {"name": "Unlock Luna", "cost": {"Gold": 150, "Mithril": 100},
     "description": "Unlock planet Luna to harvest exotic materials.", "effectName": "unlock_luna"},
    {"name": "Crafting Mastery", "cost": {"Engine Parts": 5, "Titanium Bar": 3},
     "description": "Reduces crafting energy cost by 20%.", "effectName": "crafting_mastery"},
    {"name": "Precision Tools", "cost": {"Iron Bar": 10, "Mithril Bar": 5},
     "description": "Gives a 10% chance to double crafted output.", "effectName": "precision_tools"},
    {"name": "Shield Technology", "cost": {"Mithril Bar": 10, "Titanium Bar": 5},
     "description": "Allows building Shield Ships to protect convoys.", "effectName": "unlock_shield_ships"},
    {"name": "Emergency Energy Conservation",
     "cost": {"Engine Parts": 10, "Titanium Bar": 5},
     "description": "Allows a planet to halt production during an imminent raider attack, conserving energy for defense.", "effectName": "energy_conservation"},
    {"name": "Repair Drone Technology", "cost": {"Fusion Reactor": 1, "Advanced Engine Parts": 3},
     "description": "Unlocks Repair Drones that can repair ships during combat.", "effectName": "unlock_repair_drones"},
    {"name": "Solar Panels", "cost": {"Iron": 20, "Copper": 30},
     "description": "Unlock Solar Panels for energy production on planets.", "effectName": "unlock_solar_panels"},
    {"name": "Tritium Extraction", "cost": {"Mithril Bar": 5, "Advanced Engine Parts": 2},
     "description": "Unlocks the ability to build Tritium Extractors to harvest tritium for Fusion Reactors.", "effectName": "unlock_tritium_extraction"},
    {"name": "Urban Planning I", "cost": {"Iron Bar": 15, "Engine Parts": 5},
     "description": "Increases Terra's building capacity by 4.", "effectName": "urban_planning_terra"},
    {"name": "Urban Planning II", "cost": {"Iron Bar": 30, "Engine Parts": 10},
     "description": "Increases Mars' building capacity by 4.", "effectName": "urban_planning_mars"},
    {"name": "Urban Planning III", "cost": {"Iron Bar": 45, "Engine Parts": 15},
     "description": "Increases Zalthor's building capacity by 4.", "effectName": "urban_planning_zalthor"},
    {"name": "Capital Ship Initiative",
     "cost": {"Advanced Engine Parts": 5, "Fusion Reactor": 1, "Titanium Bar": 10},
     "description": "Unlock the ability to construct a powerful capital ship. Only one such vessel may be active at any time.", "effectName": "unlock_capital_ships"},
    {"name": "Auxiliary Frigate Development",
     "cost": {"Engine Parts": 20, "Advanced Engine Parts": 5, "Mithril Bar": 10},
     "description": "Unlock smaller versions of capital ships (frigates) that are less powerful but can be built without limits.", "effectName": "unlock_capital_frigates"}
]

# Crafting recipes (for items and ships)
CRAFTING_RECIPES = {
    "Iron Bar": {"inputs": {"Iron": 5}, "timeRequired": 1.0, "electricityCost": 2.0, "requiredBuilding": "Smelting Building"},
    "Interceptor": {"inputs": {"Engine Parts": 3, "Titanium Bar": 2}, "timeRequired": 4.0, "electricityCost": 20.0, "requiredBuilding": "Shipyard"},
    "Celestial Juggernaut": {"inputs": {"Fusion Reactor": 1, "Titanium Bar": 10, "Advanced Engine Parts": 5, "Crystal": 5},
                              "timeRequired": 20.0, "electricityCost": 40.0, "requiredBuilding": "Flagship Dock"},
    "Nova Carrier": {"inputs": {"Fusion Reactor": 1, "Titanium Bar": 8, "Advanced Engine Parts": 4, "Mithril Bar": 10},
                     "timeRequired": 18.0, "electricityCost": 35.0, "requiredBuilding": "Flagship Dock"},
    "Obsidian Sovereign": {"inputs": {"Fusion Reactor": 1, "Titanium Bar": 12, "Advanced Engine Parts": 6, "Crystal": 7},
                           "timeRequired": 25.0, "electricityCost": 45.0, "requiredBuilding": "Flagship Dock"},
    "Preemptor": {"inputs": {"Fusion Reactor": 1, "Titanium Bar": 9, "Advanced Engine Parts": 5, "Mithril Bar": 8},
                  "timeRequired": 15.0, "electricityCost": 50.0, "requiredBuilding": "Flagship Dock"},
    "Aurora Protector": {"inputs": {"Fusion Reactor": 1, "Titanium Bar": 10, "Advanced Engine Parts": 5, "Crystal": 6},
                         "timeRequired": 20.0, "electricityCost": 30.0, "requiredBuilding": "Flagship Dock"},
    # Frigate recipes:
    "Juggernaut Frigate": {"inputs": {"Titanium Bar": 5, "Advanced Engine Parts": 2, "Crystal": 2},
                           "timeRequired": 10.0, "electricityCost": 15.0, "requiredBuilding": "Shipyard"},
    "Carrier Frigate": {"inputs": {"Titanium Bar": 4, "Advanced Engine Parts": 2, "Mithril Bar": 5},
                        "timeRequired": 9.0, "electricityCost": 12.0, "requiredBuilding": "Shipyard"},
    "Sovereign Frigate": {"inputs": {"Titanium Bar": 6, "Advanced Engine Parts": 3, "Crystal": 3},
                          "timeRequired": 12.0, "electricityCost": 18.0, "requiredBuilding": "Shipyard"},
    "Preemptor Frigate": {"inputs": {"Titanium Bar": 5, "Advanced Engine Parts": 2, "Mithril Bar": 4},
                          "timeRequired": 8.0, "electricityCost": 20.0, "requiredBuilding": "Shipyard"},
    "Protector Frigate": {"inputs": {"Titanium Bar": 5, "Advanced Engine Parts": 2, "Crystal": 3},
                          "timeRequired": 10.0, "electricityCost": 15.0, "requiredBuilding": "Shipyard"}
}

# Building recipes – note each building has a "plotCost" value.
BUILDING_RECIPES = {
    "Crafting Building": {"inputs": {"Iron Bar": 5, "Copper Bar": 2}, "timeRequired": 5.0, "electricityCost": 10.0, "plotCost": 1},
    "Smelting Building": {"inputs": {"Iron": 5, "Coal": 10}, "timeRequired": 5.0, "electricityCost": 10.0, "plotCost": 1},
    "Facility Workshop": {"inputs": {"Generator": 2, "Accumulator": 2}, "timeRequired": 3.0, "electricityCost": 5.0, "plotCost": 1},
    "Shipyard": {"inputs": {"Iron Bar": 10, "Engine Parts": 5}, "timeRequired": 8.0, "electricityCost": 20.0, "plotCost": 1},
    "Proximity Alarm": {"inputs": {"Crystal": 2, "Mithril Bar": 1}, "timeRequired": 3.0, "electricityCost": 5.0, "plotCost": 1},
    "Proximity Radar": {"inputs": {"Crystal": 2, "Mithril Bar": 1}, "timeRequired": 3.0, "electricityCost": 5.0, "plotCost": 1},
    "Mobile Radar": {"inputs": {"Crystal": 3, "Copper Bar": 2}, "timeRequired": 3.0, "electricityCost": 5.0, "plotCost": 1},
    "Salvage Robot": {"inputs": {"Obsidian": 2, "Engine Parts": 1}, "timeRequired": 3.0, "electricityCost": 5.0, "plotCost": 1},
    "Shield Generator": {"inputs": {"Titanium Bar": 2, "Copper Bar": 2}, "timeRequired": 4.0, "electricityCost": 5.0, "plotCost": 1},
    "Solar Panel": {"inputs": {}, "timeRequired": 3.0, "electricityCost": 0.0, "plotCost": 1},
    "Tritium Extractor": {"inputs": {"Mithril Bar": 2, "Advanced Engine Parts": 1}, "timeRequired": 4.0, "electricityCost": 8.0, "plotCost": 1},
    "Defense Turret": {"inputs": {"Iron Bar": 3, "Engine Parts": 1}, "timeRequired": 3.0, "electricityCost": 5.0, "plotCost": 1},
    "Flagship Dock": {"inputs": {"Iron Bar": 10, "Engine Parts": 5, "Titanium Bar": 5}, "timeRequired": 10.0, "electricityCost": 30.0, "plotCost": 2}
}

# ---------------------------
# Game Classes
# ---------------------------

class Journal:
    def __init__(self):
        self.entries = []
        self.next_id = 1

    def add_entry(self, title, text):
        entry = {"id": self.next_id, "title": title, "text": text, "unlocked": True}
        self.entries.append(entry)
        self.next_id += 1
        log_message(f"New journal entry unlocked: [{entry['id']}] {entry['title']}")

    def list_entries(self):
        if not self.entries:
            log_message("No journal entries yet.")
            return
        log_message("Journal Entries:")
        for entry in self.entries:
            if entry["unlocked"]:
                log_message(f"[{entry['id']}] {entry['title']}")

    def view_entry(self, entry_id):
        for entry in self.entries:
            if entry["id"] == entry_id and entry["unlocked"]:
                log_message(f"Journal Entry [{entry['id']}] {entry['title']}:\n{entry['text']}")
                return
        log_message("Journal entry not found or locked.")

# Create a global journal instance
journal = Journal()


class Planet:
    # Some production constants
    GENERATOR_RATE = 5.0
    ENERGY_COST_PER_SECOND = 1.0
    COAL_CONSUMPTION = 0.5
    SOLAR_RATE = 2.0
    TRITIUM_EXTRACTION_RATE = 0.05

    def __init__(self, name, baseProduction, unlocked):
        self.name = name
        self.baseProduction = baseProduction.copy()
        self.unlocked = unlocked
        self.generators = 0
        self.accumulators = 0
        self.current_energy = 0.0
        self.max_energy = 50.0
        self.storage = {res["name"]: 0 for res in RESOURCE_DATA}
        self.buildings = {}  # building name -> count
        self.max_building_plots = 8
        self.used_building_plots = 0
        self.plots_upgraded = False
        self.under_threat = False

    def produce_resources(self, elapsed, energy_conservation_enabled):
        if self.under_threat and energy_conservation_enabled and self.has_building("Shield Generator"):
            log_message(f"Production on {self.name} halted due to Emergency Energy Conservation.")
            return
        if not self.unlocked:
            return

        production_time = elapsed
        # If generators exist, consume coal to limit production time.
        if self.generators > 0:
            possible_time = self.storage.get("Coal", 0) / (self.generators * Planet.COAL_CONSUMPTION)
            production_time = min(elapsed, possible_time)
            coal_needed = math.ceil(self.generators * Planet.COAL_CONSUMPTION * production_time)
            self.remove_from_storage("Coal", coal_needed)
        # Produce energy from generators
        energyProduced = self.generators * Planet.GENERATOR_RATE * production_time
        self.current_energy = min(self.max_energy, self.current_energy + energyProduced)
        # Solar panels add energy if present
        if self.has_building("Solar Panel"):
            solar_count = self.buildings.get("Solar Panel", 0)
            solar_energy = solar_count * Planet.SOLAR_RATE * elapsed
            self.current_energy = min(self.max_energy, self.current_energy + solar_energy)
        # Consume energy to produce resources
        effective_seconds = min(production_time, self.current_energy / Planet.ENERGY_COST_PER_SECOND)
        energyConsumed = effective_seconds * Planet.ENERGY_COST_PER_SECOND
        self.current_energy -= energyConsumed
        for res, rate in self.baseProduction.items():
            produced = math.floor(rate * production_time)
            self.storage[res] = self.storage.get(res, 0) + produced
        # Tritium Extractor production
        if self.has_building("Tritium Extractor"):
            extractorCount = self.buildings.get("Tritium Extractor", 0)
            tritiumProduced = math.floor(extractorCount * Planet.TRITIUM_EXTRACTION_RATE * production_time)
            self.storage["Tritium"] = self.storage.get("Tritium", 0) + tritiumProduced
            if tritiumProduced > 0:
                log_message(f"Tritium Extractor on {self.name} produced {tritiumProduced} Tritium.")

    def can_build_more(self):
        return self.used_building_plots < self.max_building_plots

    def upgrade_plots(self):
        if not self.plots_upgraded:
            self.plots_upgraded = True
            self.max_building_plots += 2
            return True
        return False

    def increase_max_building_plots(self, amount):
        self.max_building_plots += amount

    def add_building_with_cost(self, buildingName, plotCost):
        if self.used_building_plots + plotCost > self.max_building_plots:
            log_message(f"No building plots available on {self.name} for {buildingName}.")
            return False
        self.buildings[buildingName] = self.buildings.get(buildingName, 0) + 1
        self.used_building_plots += plotCost
        return True

    def add_building(self, buildingName):
        if not self.can_build_more():
            log_message(f"No building plots available on {self.name}. Upgrade building capacity first.")
            return False
        self.buildings[buildingName] = self.buildings.get(buildingName, 0) + 1
        self.used_building_plots += 1
        return True

    def has_building(self, buildingName):
        return self.buildings.get(buildingName, 0) > 0

    def add_to_storage(self, resourceName, amount):
        self.storage[resourceName] = self.storage.get(resourceName, 0) + amount

    def remove_from_storage(self, resourceName, amount):
        if self.storage.get(resourceName, 0) >= amount:
            self.storage[resourceName] -= amount
            return True
        return False

    def set_unlocked(self, unlocked):
        self.unlocked = unlocked

    def set_under_threat(self, flag):
        self.under_threat = flag


class PlanetManager:
    def __init__(self, planet_data):
        self.planets = []
        for pd in planet_data:
            self.planets.append(Planet(pd["name"], pd["baseProduction"], pd["unlocked"]))

    def produce_all(self, elapsed, energy_conservation_enabled):
        for planet in self.planets:
            planet.produce_resources(elapsed, energy_conservation_enabled)

    def unlock_planet(self, planetName):
        p = self.get_planet_by_name(planetName)
        if p:
            p.set_unlocked(True)

    def get_planet_by_name(self, planetName):
        for planet in self.planets:
            if planet.name.lower() == planetName.lower():
                return planet
        return None


class Research:
    def __init__(self, name, cost, description, effectName):
        self.name = name
        self.cost = cost
        self.description = description
        self.effectName = effectName
        self.completed = False

    def can_research(self, central: Planet):
        for res, amt in self.cost.items():
            if central.storage.get(res, 0) < amt:
                return False
        return True

    def do_research(self, central: Planet):
        for res, amt in self.cost.items():
            central.remove_from_storage(res, amt)
        self.completed = True


class ResearchManager:
    def __init__(self, research_data):
        self.researches = []
        for rd in research_data:
            self.researches.append(Research(rd["name"], rd["cost"], rd["description"], rd["effectName"]))

    def find_research_by_name(self, name):
        for r in self.researches:
            if r.name.lower() == name.lower():
                return r
        return None


class DailyQuest:
    def __init__(self, desc, objectiveRes, objectiveAmt, reward):
        self.description = desc
        self.objectiveResource = objectiveRes
        self.objectiveAmount = objectiveAmt
        self.reward = reward.copy()
        self.completed = False
        self.is_raider_attack = False
        self.target_planet = ""
        self.combat_start_time = 0
        self.turns_elapsed = 0
        self.is_story_quest = False
        self.is_final_confrontation = False

    def check_completion(self, central: Planet):
        if self.completed or self.is_raider_attack:
            return
        if self.objectiveResource and central.storage.get(self.objectiveResource, 0) >= self.objectiveAmount:
            self.completed = True
            for res, amt in self.reward.items():
                central.add_to_storage(res, amt)
            log_message("Quest complete! Reward: " + ", ".join(f"{res} +{amt}" for res, amt in self.reward.items()))
            if not self.is_story_quest:
                journal.add_entry("Resource Quest Completed",
                                  f"You successfully gathered {self.objectiveAmount} {self.objectiveResource}. The effort has not gone unnoticed.")
                log_message("Lore: " + random.choice(resourceLore))

    def process_raider_attack(self, pm, fleet, realTime=True):
        # Simplified combat simulation (turn-based)
        if self.completed or not self.is_raider_attack:
            return False
        target = pm.get_planet_by_name(self.target_planet)
        if not target:
            log_message(f"Target planet {self.target_planet} not found.")
            return False
        if self.combat_start_time == 0:
            self.combat_start_time = time.time()
        # Adjust enemy stats (simplified)
        raider_shield = random.randint(100, 200) if not self.is_final_confrontation else random.randint(200, 300)
        raider_hull = random.randint(300, 500) if not self.is_final_confrontation else random.randint(600, 800)
        log_message(f"Commencing raider battle at {self.target_planet}!")
        if self.is_final_confrontation:
            log_message("Multiple enemy capital ships support the assault!")
        maxTurns = 10
        turn = 0
        while turn < maxTurns and fleet and raider_hull > 0:
            turn += 1
            log_message(f"--- Turn {turn} ---")
            if realTime:
                # In a GUI we avoid long blocking sleeps; here we simply simulate a delay
                time.sleep(1)
            totalDamage = 0
            for ship in fleet:
                if ship["type"] in ["Corvette", "Shield Ship", "Radar Ship", "Salvage Ship"]:
                    totalDamage += ship["weapons"]
            log_message(f"Your defenders fire for a total of {totalDamage} damage.")
            shieldDamage = min(totalDamage, raider_shield)
            raider_shield -= shieldDamage
            remainingDamage = totalDamage - shieldDamage
            raider_hull -= remainingDamage
            log_message(f"Raider shields take {shieldDamage} damage, remaining: {raider_shield}")
            if remainingDamage > 0:
                log_message(f"Raider hull takes {remainingDamage} damage, remaining: {raider_hull}")
            if raider_hull <= 0:
                log_message("Raiders defeated!")
                terra = pm.get_planet_by_name("Terra")
                if terra:
                    terra.add_to_storage("Engine Parts", 2)
                    log_message("You receive 2 Engine Parts as reward.")
                self.completed = True
                if not self.is_story_quest:
                    journal.add_entry("Raider Attack Repelled", "In a fierce battle, your fleet repelled the raider onslaught.")
                    log_message("Lore: " + random.choice(raiderLore))
                target.set_under_threat(False)
                break
            # Raider damage to fleet (simplified)
            raiderDamage = random.randint(50, 100)
            if self.is_final_confrontation:
                raiderDamage += 30
            log_message(f"Raiders fire for {raiderDamage} damage.")
            # Damage distribution among ships:
            if fleet:
                dmg_per_ship = raiderDamage // len(fleet)
                for ship in fleet:
                    if ship["current_shield"] >= dmg_per_ship:
                        ship["current_shield"] -= dmg_per_ship
                        log_message(f"{ship['type']} takes {dmg_per_ship} shield damage, remaining shield: {ship['current_shield']}")
                    else:
                        rem = dmg_per_ship - ship["current_shield"]
                        ship["current_shield"] = 0
                        ship["hull"] -= rem
                        log_message(f"{ship['type']} shield depleted; takes {rem} hull damage, remaining hull: {ship['hull']}")
                fleet[:] = [s for s in fleet if s["hull"] > 0]
            if not fleet:
                log_message("All defending ships have been destroyed!")
                self.completed = True
                log_message("Lore: " + random.choice(raiderLore))
                target.set_under_threat(False)
                break
            log_message(f"End of turn {turn}.")
        if turn >= maxTurns:
            log_message("Combat ended after 10 turns.")
            self.completed = True
            target.set_under_threat(False)
        return self.completed


class QuestManager:
    def __init__(self):
        self.current_quest = None
        self.last_quest_date = ""
        self.story_stage = 0

    def update_daily_quest(self, central: Planet, pm, fleet):
        today_str = time.strftime("%Y-%m-%d", time.gmtime())
        if self.last_quest_date == "" or self.last_quest_date < today_str:
            self.generate_new_quest(pm)
            self.last_quest_date = today_str
        if self.current_quest:
            if self.current_quest.is_raider_attack:
                self.current_quest.process_raider_attack(pm, fleet)
            else:
                self.current_quest.check_completion(central)
            # If a story quest was just completed, add a journal entry and clear the quest.
            if self.current_quest.completed and self.current_quest.is_story_quest:
                if self.story_stage == 0:
                    journal.add_entry("The Spark Ignited",
                                      ("Old Miner Joe speaks with quiet resolve as you deliver 50 Iron Bars. "
                                       "Each bar is not merely forged from metal but from the very essence of our past struggles."))
                elif self.story_stage == 1:
                    journal.add_entry("A Cosmic Warning",
                                      ("Professor Lumen warns: 'Cosmic anomalies stir near Mars.' "
                                       "Her tone reveals that the raider threat may stem from desperate circumstances."))
                elif self.story_stage == 2:
                    journal.add_entry("Bandit Outpost Assault",
                                      ("Farmer Daisy urges: 'Strike at the raider outpost on Zalthor to protect our supplies!'"))
                elif self.story_stage == 3:
                    journal.add_entry("Shadows Over Vulcan",
                                      ("A distress call from Vulcan: 'Raider forces have established a foothold. Reclaim our honor!'"))
                elif self.story_stage == 4:
                    journal.add_entry("Convoy Under Fire",
                                      ("Merchant voices cry out: 'Defend Terra from raider ambushes on the convoys!'"))
                elif self.story_stage == 5:
                    journal.add_entry("Echoes of Betrayal",
                                      ("Professor Lumen reveals a hidden raider outpost on Mars – a site of deep-seated betrayal."))
                elif self.story_stage == 6:
                    journal.add_entry("Broken Chains",
                                      ("Old Miner Joe challenges you: 'Collect 30 Mithril Bars from Terra to break the chains of oppression.'"))
                elif self.story_stage == 7:
                    journal.add_entry("Siege of the Forgotten",
                                      ("Under Luna’s eerie glow, your forces lay siege to a raider stronghold." ))
                elif self.story_stage == 8:
                    journal.add_entry("Whispers in the Void",
                                      ("Intercepted transmissions hint at internal turmoil among the raiders on Terra." ))
                elif self.story_stage == 9:
                    journal.add_entry("The Great Siege",
                                      ("On Mars, you lead an assault against a well-fortified raider outpost." ))
                elif self.story_stage == 10:
                    journal.add_entry("Rising Tempest",
                                      ("Merchant guilds cry out for aid as war approaches. Collect 200 Coal from Terra!" ))
                elif self.story_stage == 11:
                    journal.add_entry("The Final Stand",
                                      ("The decisive moment: Face Captain Blackthorne in his lair for the final confrontation." ))
                else:
                    journal.add_entry("Unknown Stage", "The story continues beyond known bounds.")
                self.story_stage += 1
                self.current_quest = None

    def generate_new_quest(self, pm: PlanetManager):
        # If story quest stages remain, generate a story quest
        if self.story_stage < 12:
            if self.story_stage == 0:
                # The Spark of Ambition
                desc = "Old Miner Joe says: 'Deep in Terra’s veins lie secrets of redemption. Collect 50 Iron Bars.'"
                reward = {"Engine Parts": 3}
                quest = DailyQuest(desc, "Iron Bar", 50, reward)
                quest.is_story_quest = True
                self.current_quest = quest
            elif self.story_stage == 1:
                pm.unlock_planet("Mars")
                desc = "Professor Lumen warns: 'Cosmic anomalies stir near Mars. Prepare for a raider attack!'"
                reward = {"Engine Parts": 2}
                quest = DailyQuest(desc, "", 1, reward)
                quest.is_raider_attack = True
                quest.target_planet = "Mars"
                quest.is_story_quest = True
                self.current_quest = quest
            elif self.story_stage == 2:
                pm.unlock_planet("Zalthor")
                desc = "Farmer Daisy urges: 'Assault the raider outpost on Zalthor!'"
                reward = {"Engine Parts": 3}
                quest = DailyQuest(desc, "", 1, reward)
                quest.is_raider_attack = True
                quest.target_planet = "Zalthor"
                quest.is_story_quest = True
                self.current_quest = quest
            elif self.story_stage == 3:
                pm.unlock_planet("Vulcan")
                desc = "A distress call from Vulcan: 'Raiders have established a foothold. Strike now!'"
                reward = {"Engine Parts": 3}
                quest = DailyQuest(desc, "", 1, reward)
                quest.is_raider_attack = True
                quest.target_planet = "Vulcan"
                quest.is_story_quest = True
                self.current_quest = quest
            elif self.story_stage == 4:
                desc = "Merchant voices cry out: 'Defend Terra! Convoys are under attack!'"
                reward = {"Engine Parts": 2}
                quest = DailyQuest(desc, "", 1, reward)
                quest.is_raider_attack = True
                quest.target_planet = "Terra"
                quest.is_story_quest = True
                self.current_quest = quest
            elif self.story_stage == 5:
                desc = "Professor Lumen reveals: 'A hidden raider outpost on Mars holds secrets of betrayal.'"
                reward = {"Engine Parts": 3}
                quest = DailyQuest(desc, "", 1, reward)
                quest.is_raider_attack = True
                quest.target_planet = "Mars"
                quest.is_story_quest = True
                self.current_quest = quest
            elif self.story_stage == 6:
                desc = "Old Miner Joe challenges: 'Collect 30 Mithril Bars from Terra.'"
                reward = {"Engine Parts": 2}
                quest = DailyQuest(desc, "Mithril Bar", 30, reward)
                quest.is_story_quest = True
                self.current_quest = quest
            elif self.story_stage == 7:
                pm.unlock_planet("Luna")
                desc = "A desperate plea: 'Lay siege to the raider stronghold on Luna!'"
                reward = {"Engine Parts": 4}
                quest = DailyQuest(desc, "", 1, reward)
                quest.is_raider_attack = True
                quest.target_planet = "Luna"
                quest.is_story_quest = True
                self.current_quest = quest
            elif self.story_stage == 8:
                desc = "Urgent message: 'Defend Terra from a sudden raider strike.'"
                reward = {"Engine Parts": 2}
                quest = DailyQuest(desc, "", 1, reward)
                quest.is_raider_attack = True
                quest.target_planet = "Terra"
                quest.is_story_quest = True
                self.current_quest = quest
            elif self.story_stage == 9:
                desc = "A clarion call: 'Assault the fortified raider outpost on Mars!'"
                reward = {"Engine Parts": 4}
                quest = DailyQuest(desc, "", 1, reward)
                quest.is_raider_attack = True
                quest.target_planet = "Mars"
                quest.is_story_quest = True
                self.current_quest = quest
            elif self.story_stage == 10:
                desc = "Merchant guilds request: 'Collect 200 Coal from Terra as war approaches.'"
                reward = {"Engine Parts": 3}
                quest = DailyQuest(desc, "Coal", 200, reward)
                quest.is_story_quest = True
                self.current_quest = quest
            elif self.story_stage == 11:
                desc = "The decisive moment: 'Face Blackthorne and end the cycle of despair!'"
                reward = {"Engine Parts": 5}
                quest = DailyQuest(desc, "", 1, reward)
                quest.is_raider_attack = True
                quest.target_planet = "Terra"
                quest.is_story_quest = True
                quest.is_final_confrontation = True
                self.current_quest = quest
        else:
            # Fallback: random daily quest (side quest)
            if random.randint(0, 99) < 20:
                candidates = [p.name for p in pm.planets if p.unlocked and p.name.lower() != "terra"]
                if not candidates:
                    candidates.append("Terra")
                target = random.choice(candidates)
                desc = f"Urgent alert: Raiders are approaching {target}! Prepare your defenses!"
                reward = {"Engine Parts": 2}
                quest = DailyQuest(desc, "", 1, reward)
                quest.is_raider_attack = True
                quest.target_planet = target
                self.current_quest = quest
                journal.add_entry("Raider Warning", f"Intelligence reports hint at an emerging threat near {target}.")
            else:
                resourceChoices = ["Iron Bar", "Copper Bar", "Mithril Bar", "Titanium Bar", "Advanced Engine Parts"]
                chosen = random.choice(resourceChoices)
                amt = random.randint(10, 50)
                reward = {"Engine Parts": random.randint(1,3)}
                desc = f"Side quest: Collect {amt} {chosen}."
                self.current_quest = DailyQuest(desc, chosen, amt, reward)


class CraftingManager:
    def __init__(self, recipes):
        self.recipes = recipes

    def craft(self, itemName, planet: Planet, fasterMultiplier, costMultiplier, precisionToolsEnabled):
        recipe = self.recipes.get(itemName)
        if recipe is None:
            log_message(f"No recipe for item '{itemName}'.")
            return
        if not planet.has_building(recipe["requiredBuilding"]):
            log_message(f"You need a {recipe['requiredBuilding']} on {planet.name} to craft {itemName}.")
            return
        # Check inputs
        for res, amt in recipe["inputs"].items():
            if planet.storage.get(res, 0) < amt:
                log_message(f"Not enough {res} on {planet.name} to craft {itemName}.")
                return
        effectiveCost = recipe["electricityCost"] * costMultiplier
        if planet.current_energy < effectiveCost:
            log_message(f"Not enough energy on {planet.name} to craft {itemName}.")
            return
        for res, amt in recipe["inputs"].items():
            planet.remove_from_storage(res, amt)
        planet.current_energy -= effectiveCost
        craftTime = recipe["timeRequired"] * fasterMultiplier
        log_message(f"Crafting {itemName} took {craftTime:.1f} seconds.")
        quantity = 1
        if precisionToolsEnabled:
            if random.randint(0,99) < 10:
                quantity = 2
                log_message("Precision Tools activated: doubled output!")
        planet.add_to_storage(itemName, quantity)
        log_message(f"Crafted {quantity} {itemName}(s) on {planet.name}.")


# Ship is represented as a dictionary with keys: type, hull, max_shield, current_shield, weapons, repairAmount
def create_ship(shipType):
    ship = {"type": shipType}
    if shipType == "Transport Vessel":
        ship.update({"hull": 100, "max_shield": 50, "current_shield": 50, "weapons": 0, "repairAmount": 0})
    elif shipType == "Corvette":
        ship.update({"hull": 100, "max_shield": 75, "current_shield": 75, "weapons": 30, "repairAmount": 0})
    elif shipType == "Shield Ship":
        ship.update({"hull": 120, "max_shield": 150, "current_shield": 150, "weapons": 20, "repairAmount": 0})
    elif shipType == "Radar Ship":
        ship.update({"hull": 110, "max_shield": 80, "current_shield": 80, "weapons": 25, "repairAmount": 0})
    elif shipType == "Salvage Ship":
        ship.update({"hull": 110, "max_shield": 70, "current_shield": 70, "weapons": 15, "repairAmount": 0})
    elif shipType == "Repair Drone":
        ship.update({"hull": 80, "max_shield": 40, "current_shield": 40, "weapons": 0, "repairAmount": 10})
    elif shipType == "Interceptor":
        ship.update({"hull": 90, "max_shield": 60, "current_shield": 60, "weapons": 40, "repairAmount": 0})
    elif shipType == "Celestial Juggernaut":
        ship.update({"hull": 300, "max_shield": 200, "current_shield": 200, "weapons": 100, "repairAmount": 0})
    elif shipType == "Nova Carrier":
        ship.update({"hull": 250, "max_shield": 150, "current_shield": 150, "weapons": 80, "repairAmount": 0})
    elif shipType == "Obsidian Sovereign":
        ship.update({"hull": 350, "max_shield": 250, "current_shield": 250, "weapons": 120, "repairAmount": 0})
    elif shipType == "Preemptor":
        ship.update({"hull": 200, "max_shield": 100, "current_shield": 100, "weapons": 150, "repairAmount": 0})
    elif shipType == "Aurora Protector":
        ship.update({"hull": 280, "max_shield": 300, "current_shield": 300, "weapons": 70, "repairAmount": 0})
    elif shipType == "Juggernaut Frigate":
        ship.update({"hull": 150, "max_shield": 100, "current_shield": 100, "weapons": 40, "repairAmount": 0})
    elif shipType == "Carrier Frigate":
        ship.update({"hull": 130, "max_shield": 90, "current_shield": 90, "weapons": 30, "repairAmount": 0})
    elif shipType == "Sovereign Frigate":
        ship.update({"hull": 160, "max_shield": 110, "current_shield": 110, "weapons": 50, "repairAmount": 0})
    elif shipType == "Preemptor Frigate":
        ship.update({"hull": 120, "max_shield": 80, "current_shield": 80, "weapons": 60, "repairAmount": 0})
    elif shipType == "Protector Frigate":
        ship.update({"hull": 140, "max_shield": 120, "current_shield": 120, "weapons": 35, "repairAmount": 0})
    return ship


class Player:
    def __init__(self):
        self.planetManager = PlanetManager(PLANET_DATA)
        self.researchManager = ResearchManager(RESEARCH_DATA)
        self.craftingManager = CraftingManager(CRAFTING_RECIPES)
        self.questManager = QuestManager()
        self.fleet = []  # list of ship dictionaries
        self.fasterCraftingMultiplier = 1.0
        self.craftingCostMultiplier = 1.0
        self.precisionToolsEnabled = False
        self.energyConservationEnabled = False
        self.quantumCommunicationEnabled = False
        self.last_update_time = time.time()

    def produce_resources(self):
        now = time.time()
        diff = now - self.last_update_time
        if diff < 0:
            diff = 0
        self.planetManager.produce_all(diff, self.energyConservationEnabled)
        self.last_update_time = now

    def do_research(self, researchName):
        research = self.researchManager.find_research_by_name(researchName)
        if research is None:
            log_message("No research found by that name.")
            return
        if research.completed:
            log_message("Research already completed.")
            return
        terra = self.planetManager.get_planet_by_name("Terra")
        if terra is None:
            log_message("Central planet Terra not found.")
            return
        if research.can_research(terra):
            research.do_research(terra)
            self.apply_research_effect(research.effectName)
            log_message(f"Research '{research.name}' completed!")
        else:
            log_message("Not enough resources on Terra to perform research.")

    def craft_item(self, itemName, planetName="Terra"):
        planet = self.planetManager.get_planet_by_name(planetName)
        if planet is None:
            log_message(f"Planet {planetName} not found.")
            return
        self.craftingManager.craft(itemName, planet, self.fasterCraftingMultiplier,
                                   self.craftingCostMultiplier, self.precisionToolsEnabled)

    def craft_ship(self, shipType, planetName="Terra"):
        planet = self.planetManager.get_planet_by_name(planetName)
        if planet is None:
            log_message(f"Planet {planetName} not found.")
            return
        # For capital ships allow only one at a time
        if shipType in ["Celestial Juggernaut", "Nova Carrier", "Obsidian Sovereign", "Preemptor", "Aurora Protector"]:
            for ship in self.fleet:
                if ship["type"] in ["Celestial Juggernaut", "Nova Carrier", "Obsidian Sovereign", "Preemptor", "Aurora Protector"]:
                    log_message("A capital ship is already active. Only one capital ship can be active at a time.")
                    return
        recipe = CRAFTING_RECIPES.get(shipType)
        if recipe is None:
            log_message(f"No recipe for ship type '{shipType}'.")
            return
        if not planet.has_building(recipe["requiredBuilding"]):
            log_message(f"You need a {recipe['requiredBuilding']} on {planetName} to build a {shipType}.")
            return
        for res, amt in recipe["inputs"].items():
            if planet.storage.get(res, 0) < amt:
                log_message(f"Not enough {res} on {planetName} to build a {shipType}.")
                return
        effectiveCost = recipe["electricityCost"] * self.craftingCostMultiplier
        if planet.current_energy < effectiveCost:
            log_message(f"Not enough energy on {planetName} to build a {shipType}.")
            return
        planet.current_energy -= effectiveCost
        for res, amt in recipe["inputs"].items():
            planet.remove_from_storage(res, amt)
        log_message(f"Built a {shipType} on {planetName} using {effectiveCost} energy.")
        newShip = create_ship(shipType)
        self.fleet.append(newShip)

    def build_building(self, buildingName, planetName):
        planet = self.planetManager.get_planet_by_name(planetName)
        if planet is None:
            log_message(f"Planet {planetName} not found.")
            return
        recipe = BUILDING_RECIPES.get(buildingName)
        if recipe is None:
            log_message(f"No recipe for building '{buildingName}'.")
            return
        plotCost = recipe["plotCost"]
        if planet.used_building_plots + plotCost > planet.max_building_plots:
            log_message(f"Not enough building plots on {planetName} to build {buildingName}.")
            return
        for res, amt in recipe["inputs"].items():
            if planet.storage.get(res, 0) < amt:
                log_message(f"Not enough {res} on {planetName} to build {buildingName}.")
                return
        if planet.current_energy < recipe["electricityCost"]:
            log_message(f"Not enough energy on {planetName} to build {buildingName}.")
            return
        planet.current_energy -= recipe["electricityCost"]
        for res, amt in recipe["inputs"].items():
            planet.remove_from_storage(res, amt)
        if planet.add_building_with_cost(buildingName, plotCost):
            log_message(f"Built {buildingName} on {planetName}.")

    def upgrade_building_plots(self, planetName):
        planet = self.planetManager.get_planet_by_name(planetName)
        if planet is None:
            log_message(f"Planet {planetName} not found.")
            return
        if planet.used_building_plots < planet.max_building_plots:
            log_message(f"There are still available building plots on {planetName}.")
            return
        if planet.storage.get("Iron Bar", 0) >= 10 and planet.storage.get("Engine Parts", 0) >= 2:
            if planet.upgrade_plots():
                planet.remove_from_storage("Iron Bar", 10)
                planet.remove_from_storage("Engine Parts", 2)
                log_message(f"Building capacity on {planetName} increased by 2.")
            else:
                log_message(f"Building capacity on {planetName} has already been upgraded.")
        else:
            log_message(f"Not enough resources on {planetName} to upgrade building plots.")

    def install_facility(self, facility, planetName):
        planet = self.planetManager.get_planet_by_name(planetName)
        if planet is None:
            log_message(f"Planet {planetName} not found.")
            return
        fac_lower = facility.lower()
        if fac_lower == "generator":
            if not planet.can_build_more():
                log_message(f"No building plots available on {planetName} for Generator.")
                return
            planet.add_building("Generator")
            planet.generators += 1
            log_message(f"Installed a Generator on {planetName}. Total: {planet.generators}")
        elif fac_lower == "accumulator":
            planet.accumulators += 1
            planet.max_energy += 50.0
            log_message(f"Installed an Accumulator on {planetName}. Total: {planet.accumulators}, new max energy: {planet.max_energy}")
        else:
            log_message(f"Unknown facility: {facility}")

    def transfer_resource(self, resource, amount, fromPlanetName, toPlanetName):
        fromPlanet = self.planetManager.get_planet_by_name(fromPlanetName)
        toPlanet = self.planetManager.get_planet_by_name(toPlanetName)
        if fromPlanet is None or toPlanet is None:
            log_message("Invalid planet(s) specified.")
            return
        if fromPlanet.storage.get(resource, 0) < amount:
            log_message(f"Not enough {resource} on {fromPlanetName} to transfer.")
            return
        # Check that at least one Transport Vessel is in fleet
        if not any(ship["type"] == "Transport Vessel" for ship in self.fleet):
            log_message("At least one Transport Vessel is required in your fleet.")
            return
        log_message(f"Transferring {amount} {resource} from {fromPlanetName} to {toPlanetName}.")
        attackChance = 15 if self.quantumCommunicationEnabled else 30
        if random.randint(0, 99) < attackChance:
            log_message("Raider attack on the convoy!")
            # Remove one transport vessel
            for i, ship in enumerate(self.fleet):
                if ship["type"] == "Transport Vessel":
                    self.fleet.pop(i)
                    break
            bonus = 0
            for ship in self.fleet:
                if ship["type"] in ["Corvette", "Shield Ship", "Radar Ship", "Salvage Ship"]:
                    bonus += ship["weapons"]
            hasInterceptor = any(ship["type"] == "Interceptor" for ship in self.fleet)
            raiderStrength = random.randint(20, 120)
            if hasInterceptor:
                log_message("Interceptor in convoy reduces raider strength by 20!")
                raiderStrength = max(0, raiderStrength - 20)
            log_message(f"Convoy strength: {bonus}, Raider strength: {raiderStrength}")
            if bonus >= raiderStrength:
                log_message("Convoy fended off the raiders!")
                fromPlanet.remove_from_storage(resource, amount)
                toPlanet.add_to_storage(resource, amount)
                if any(ship["type"] == "Salvage Ship" for ship in self.fleet):
                    extra = random.randint(5, 10)
                    fromPlanet.add_to_storage("Iron", extra)
                    log_message(f"Your Salvage Ship recovered an extra {extra} Iron from the wreckage.")
            else:
                log_message("Convoy was destroyed! Resources lost.")
                if fromPlanet.has_building("Salvage Robot"):
                    salvage = random.randint(5, 15)
                    fromPlanet.add_to_storage("Iron", salvage)
                    log_message(f"Salvage Robot recovered {salvage} Iron.")
        else:
            fromPlanet.remove_from_storage(resource, amount)
            toPlanet.add_to_storage(resource, amount)
            log_message("Transfer successful.")

    def show_radar(self, planetName):
        planet = self.planetManager.get_planet_by_name(planetName)
        if planet is None:
            log_message(f"Planet {planetName} not found.")
            return
        if planet.has_building("Radar") or planet.has_building("Mobile Radar"):
            log_message(f"Radar on {planetName} indicates HIGH raider activity.")
        else:
            log_message(f"No Radar installed on {planetName}. Raider activity unknown.")

    def update_daily_quest(self):
        terra = self.planetManager.get_planet_by_name("Terra")
        if terra is None:
            return
        self.questManager.update_daily_quest(terra, self.planetManager, self.fleet)

    def recalc_upgrades_from_research(self):
        self.fasterCraftingMultiplier = 1.0
        self.craftingCostMultiplier = 1.0
        self.precisionToolsEnabled = False
        self.energyConservationEnabled = False
        self.quantumCommunicationEnabled = False
        for research in self.researchManager.researches:
            if research.completed:
                effect = research.effectName
                if effect == "faster_crafting":
                    self.fasterCraftingMultiplier = 0.5
                elif effect == "crafting_mastery":
                    self.craftingCostMultiplier = 0.8
                elif effect == "precision_tools":
                    self.precisionToolsEnabled = True
                elif effect == "energy_conservation":
                    self.energyConservationEnabled = True
                elif effect == "quantum_communication":
                    self.quantumCommunicationEnabled = True

    def show_fleet(self):
        if not self.fleet:
            log_message("Your fleet is empty.")
            return
        log_message("Your fleet:")
        for i, ship in enumerate(self.fleet):
            info = f"{i+1}. {ship['type']} (Hull: {ship['hull']}, Shield: {ship['current_shield']}/{ship['max_shield']}"
            if ship["type"] in ["Corvette", "Shield Ship", "Radar Ship", "Salvage Ship"]:
                info += f", Weapons: {ship['weapons']}"
            if ship["type"] == "Repair Drone":
                info += f", Repairs: {ship['repairAmount']} HP/turn"
            info += ")"
            log_message(info)

    def apply_research_effect(self, effectName):
        if effectName == "unlock_mars":
            self.planetManager.unlock_planet("Mars")
            log_message("Mars unlocked!")
            journal.add_entry("Mars Unlocked", "Mars is now open for resource extraction. Ancient relics stir excitement.")
        elif effectName == "unlock_zalthor":
            self.planetManager.unlock_planet("Zalthor")
            log_message("Zalthor unlocked!")
            journal.add_entry("Zalthor Revealed", "Zalthor's mysterious landscapes hold both peril and promise.")
        elif effectName == "faster_crafting":
            self.fasterCraftingMultiplier = 0.5
            log_message("Faster crafting unlocked!")
            journal.add_entry("Crafting Breakthrough", "New techniques have halved crafting times.")
        elif effectName == "unlock_vulcan":
            self.planetManager.unlock_planet("Vulcan")
            log_message("Vulcan unlocked!")
            journal.add_entry("Vulcan Emerges", "Vulcan now offers rare resources amid its volatile environment.")
        elif effectName == "unlock_luna":
            self.planetManager.unlock_planet("Luna")
            log_message("Luna unlocked!")
            journal.add_entry("Luna Unbound", "Luna's icy plains hold untapped potential.")
        elif effectName == "crafting_mastery":
            self.craftingCostMultiplier = 0.8
            log_message("Crafting mastery achieved!")
            journal.add_entry("Crafting Mastery", "Skilled hands have refined the art of crafting, reducing costs.")
        elif effectName == "precision_tools":
            self.precisionToolsEnabled = True
            log_message("Precision tools enabled!")
            journal.add_entry("Precision Engineering", "Advanced tools increase your chance to double production.")
        elif effectName == "energy_conservation":
            self.energyConservationEnabled = True
            log_message("Emergency Energy Conservation active!")
            journal.add_entry("Energy Conservation", "New protocols allow for rapid energy shutdown during crises.")
        elif effectName == "unlock_solar_panels":
            log_message("Solar Panels unlocked! You can now build Solar Panels.")
            journal.add_entry("Solar Revolution", "Harnessing the sun’s power, solar panels fuel your empire.")
        elif effectName == "unlock_capital_ships":
            log_message("Capital Ship Initiative research completed! You can now build one powerful capital ship at a time.")
            journal.add_entry("Capital Ship Initiative", "Colossal capital ships now stand as behemoths of war.")
        elif effectName == "unlock_capital_frigates":
            log_message("Auxiliary Frigate Development research completed! You can now build smaller capital ship variants without limits.")
            journal.add_entry("Frigate Development", "Streamlined designs allow rapid deployment of frigates.")
        elif effectName == "urban_planning_terra":
            terra = self.planetManager.get_planet_by_name("Terra")
            if terra:
                terra.increase_max_building_plots(4)
                log_message("Terra's building capacity increased by 4.")
                journal.add_entry("Urban Planning in Terra", "New infrastructure plans allow an expansion of facilities on Terra.")
        elif effectName == "urban_planning_mars":
            mars = self.planetManager.get_planet_by_name("Mars")
            if mars:
                mars.increase_max_building_plots(4)
                log_message("Mars' building capacity increased by 4.")
                journal.add_entry("Urban Planning in Mars", "Mars is transforming into thriving outposts.")
        elif effectName == "urban_planning_zalthor":
            zalthor = self.planetManager.get_planet_by_name("Zalthor")
            if zalthor:
                zalthor.increase_max_building_plots(4)
                log_message("Zalthor's building capacity increased by 4.")
                journal.add_entry("Urban Planning in Zalthor", "Ambitious projects reshape Zalthor's enigmatic landscapes.")
        else:
            log_message(f"Unknown research effect: {effectName}")


# ---------------------------
# Save/Load Functions
# ---------------------------

def save_game(player: Player):
    j = {}
    # Save planet data (only a few key fields for brevity)
    planets_list = []
    for planet in player.planetManager.planets:
        planets_list.append({
            "name": planet.name,
            "unlocked": planet.unlocked,
            "current_energy": planet.current_energy,
            "max_energy": planet.max_energy,
            "storage": planet.storage,
            "buildings": planet.buildings,
            "used_building_plots": planet.used_building_plots,
            "max_building_plots": planet.max_building_plots
        })
    j["planets"] = planets_list
    # Save research data
    researches_list = []
    for research in player.researchManager.researches:
        researches_list.append({"name": research.name, "completed": research.completed})
    j["researches"] = researches_list
    j["faster_crafting_multiplier"] = player.fasterCraftingMultiplier
    j["crafting_cost_multiplier"] = player.craftingCostMultiplier
    j["precision_tools_enabled"] = player.precisionToolsEnabled
    # Save quest info (if any)
    if player.questManager.current_quest:
        q = player.questManager.current_quest
        j["current_quest"] = {
            "description": q.description,
            "objective_resource": q.objectiveResource,
            "objective_amount": q.objectiveAmount,
            "reward": q.reward,
            "completed": q.completed,
            "is_raider_attack": q.is_raider_attack,
            "target_planet": q.target_planet,
            "is_story_quest": q.is_story_quest,
            "is_final_confrontation": q.is_final_confrontation
        }
    else:
        j["current_quest"] = None
    j["last_quest_date"] = player.questManager.last_quest_date
    j["last_update_time"] = player.last_update_time
    # Save fleet
    j["fleet"] = player.fleet
    # (Journal entries could be saved as well – omitted for brevity)
    try:
        with open(SAVE_FILE, "w") as f:
            json.dump(j, f, indent=2)
        log_message("Game saved.")
    except Exception as e:
        log_message(f"Failed to save game: {e}")


def load_game(player: Player):
    if not os.path.exists(SAVE_FILE):
        return
    try:
        with open(SAVE_FILE, "r") as f:
            j = json.load(f)
        # For simplicity, we only load a subset of fields.
        for p_obj in j.get("planets", []):
            planet = player.planetManager.get_planet_by_name(p_obj["name"])
            if planet:
                planet.unlocked = p_obj.get("unlocked", planet.unlocked)
                planet.current_energy = p_obj.get("current_energy", planet.current_energy)
                planet.max_energy = p_obj.get("max_energy", planet.max_energy)
                planet.storage = p_obj.get("storage", planet.storage)
                planet.buildings = p_obj.get("buildings", planet.buildings)
                planet.used_building_plots = p_obj.get("used_building_plots", planet.used_building_plots)
                planet.max_building_plots = p_obj.get("max_building_plots", planet.max_building_plots)
        for r_obj in j.get("researches", []):
            research = player.researchManager.find_research_by_name(r_obj["name"])
            if research:
                research.completed = r_obj.get("completed", research.completed)
        player.fasterCraftingMultiplier = j.get("faster_crafting_multiplier", player.fasterCraftingMultiplier)
        player.craftingCostMultiplier = j.get("crafting_cost_multiplier", player.craftingCostMultiplier)
        player.precisionToolsEnabled = j.get("precision_tools_enabled", player.precisionToolsEnabled)
        quest_obj = j.get("current_quest")
        if quest_obj:
            quest = DailyQuest(quest_obj["description"], quest_obj["objective_resource"],
                               quest_obj["objective_amount"], quest_obj["reward"])
            quest.completed = quest_obj.get("completed", False)
            quest.is_raider_attack = quest_obj.get("is_raider_attack", False)
            quest.target_planet = quest_obj.get("target_planet", "")
            quest.is_story_quest = quest_obj.get("is_story_quest", False)
            quest.is_final_confrontation = quest_obj.get("is_final_confrontation", False)
            player.questManager.current_quest = quest
        player.questManager.last_quest_date = j.get("last_quest_date", "")
        player.last_update_time = j.get("last_update_time", time.time())
        player.fleet = j.get("fleet", [])
        player.recalc_upgrades_from_research()
        log_message("Game loaded.")
    except Exception as e:
        log_message(f"Failed to load game: {e}")

# ---------------------------
# GUI Code using Tkinter
# ---------------------------

# Global player instance
player = Player()
load_game(player)

# Create the main window
root = tk.Tk()
root.title("Expanded Modular Idle Planet Miner")

# Create a scrolled text widget for logging game messages
log_widget = scrolledtext.ScrolledText(root, width=100, height=25, state='normal', wrap=tk.WORD)
log_widget.grid(row=0, column=0, columnspan=4, padx=5, pady=5)

def log_message(message):
    """Append a message to the log widget."""
    log_widget.insert(tk.END, message + "\n")
    log_widget.see(tk.END)

# Command functions – each corresponds to one “button”
def cmd_stats():
    for planet in player.planetManager.planets:
        log_message(f"Planet {planet.name} storage:")
        for res, amt in planet.storage.items():
            log_message(f"  {res}: {amt}")

def cmd_planets():
    for planet in player.planetManager.planets:
        status = "Unlocked" if planet.unlocked else "Locked"
        log_message(f"{planet.name} => {status} | Energy: {planet.current_energy:.1f}/{planet.max_energy} | Building Plots: {planet.used_building_plots}/{planet.max_building_plots}")

def cmd_fleet():
    player.show_fleet()

def cmd_research():
    for research in player.researchManager.researches:
        status = "Completed" if research.completed else "Not Completed"
        cost_str = " ".join(f"{res}:{amt}" for res, amt in research.cost.items())
        log_message(f"{research.name} - {status} | Cost: {cost_str} | {research.description}")

def cmd_do_research():
    name = simpledialog.askstring("Do Research", "Enter research name:")
    if name:
        player.do_research(name)

def cmd_craft():
    item = simpledialog.askstring("Craft", "Enter item to craft:")
    planetName = simpledialog.askstring("Craft", "Enter planet name (default Terra):") or "Terra"
    if item:
        # Check if the item is one of the ship types:
        ship_types = ["Transport Vessel", "Corvette", "Shield Ship", "Radar Ship", "Salvage Ship", "Repair Drone", "Interceptor",
                      "Celestial Juggernaut", "Nova Carrier", "Obsidian Sovereign", "Preemptor", "Aurora Protector",
                      "Juggernaut Frigate", "Carrier Frigate", "Sovereign Frigate", "Preemptor Frigate", "Protector Frigate"]
        if item in ship_types:
            player.craft_ship(item, planetName)
        else:
            player.craft_item(item, planetName)

def cmd_build():
    building = simpledialog.askstring("Build", "Enter building to build:")
    planetName = simpledialog.askstring("Build", "Enter planet name:")
    if building and planetName:
        player.build_building(building, planetName)

def cmd_upgrade_plots():
    planetName = simpledialog.askstring("Upgrade Plots", "Enter planet name:")
    if planetName:
        player.upgrade_building_plots(planetName)

def cmd_install():
    facility = simpledialog.askstring("Install Facility", "Enter facility (Generator/Accumulator):")
    planetName = simpledialog.askstring("Install Facility", "Enter planet name:")
    if facility and planetName:
        player.install_facility(facility, planetName)

def cmd_transfer():
    resource = simpledialog.askstring("Transfer Resource", "Enter resource to transfer:")
    try:
        amount = int(simpledialog.askstring("Transfer Resource", "Enter amount:"))
    except (TypeError, ValueError):
        log_message("Invalid amount.")
        return
    fromPlanet = simpledialog.askstring("Transfer Resource", "Enter source planet:")
    toPlanet = simpledialog.askstring("Transfer Resource", "Enter destination planet:")
    if resource and fromPlanet and toPlanet:
        player.transfer_resource(resource, amount, fromPlanet, toPlanet)

def cmd_radar():
    planetName = simpledialog.askstring("Radar", "Enter planet name:")
    if planetName:
        player.show_radar(planetName)

def cmd_daily():
    dq = player.questManager.current_quest
    if dq:
        log_message(dq.description)
        if dq.is_raider_attack:
            log_message(f"Target Planet: {dq.target_planet}")
        elif dq.objectiveResource:
            log_message(f"Objective: {dq.objectiveAmount} {dq.objectiveResource}")
        log_message(f"Completed: {'Yes' if dq.completed else 'No'}")
    else:
        log_message("No current daily quest.")

def cmd_talk():
    log_message("Lore: " + random.choice(genericLore))

def cmd_journal():
    journal.list_entries()
    entry_num = simpledialog.askstring("Journal", "Enter journal entry number to view (or leave blank):")
    if entry_num:
        try:
            num = int(entry_num)
            journal.view_entry(num)
        except ValueError:
            log_message("Invalid entry number.")

def cmd_save():
    save_game(player)

def cmd_quit():
    save_game(player)
    root.quit()

# Place command buttons in a grid
btn_texts = [("Stats", cmd_stats), ("Planets", cmd_planets), ("Fleet", cmd_fleet),
             ("Research", cmd_research), ("Do Research", cmd_do_research),
             ("Craft", cmd_craft), ("Build", cmd_build), ("Upgrade Plots", cmd_upgrade_plots),
             ("Install", cmd_install), ("Transfer", cmd_transfer), ("Radar", cmd_radar),
             ("Daily Quest", cmd_daily), ("Talk", cmd_talk), ("Journal", cmd_journal),
             ("Save", cmd_save), ("Quit", cmd_quit)]

row = 1
col = 0
for (text, cmd) in btn_texts:
    b = tk.Button(root, text=text, width=15, command=cmd)
    b.grid(row=row, column=col, padx=3, pady=3)
    col += 1
    if col >= 4:
        col = 0
        row += 1

# Periodic update function – called every second
def update_game():
    player.produce_resources()
    player.update_daily_quest()
    # (Optionally, you could update a status panel here)
    root.after(1000, update_game)

# Start the update loop
update_game()

# Start the GUI main loop
root.mainloop()
