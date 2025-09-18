# Galactic Planet Miner

This repository contains early backend logic for a Galactic Planet Miner game. The code demonstrates
basic game state management and communication with a backend server using the `libft` library.

## Building and Testing

The project depends on the bundled `libft` static library. To build the test binary and run the
demonstration tests:

```sh
make
./test
```

The tests start a minimal HTTP server provided by `libft`, send a POST request through the backend
client, and verify simple resource mining logic.

## Game State

The core `ft_game_state` spawns planetary characters for Terra, Mars, Zalthor,
Vulcan, and Noctaris Prime. Each planet exposes its available ores and base
production rates so game logic can query what is mined where:

| Planet | Resources (rate/sec) |
| ------ | -------------------- |
| Terra | Iron 0.5, Copper 0.5, Coal 0.2 |
| Mars | Iron 0.1, Copper 0.1, Mithril 0.05, Coal 0.1 |
| Zalthor | Mithril 0.1, Coal 0.2, Gold 0.02 |
| Vulcan | Tin 0.1, Silver 0.03, Titanium 0.01 |
| Noctaris Prime | Obsidian 0.1, Crystal 0.05, Nanomaterial 0.02 |

Alongside these mining rates the backend now enumerates a full 21-resource economy
covering raw ores, refined bars, advanced components, and special items used for
late-game research and construction.

Resource identifiers:

- `ORE_IRON` (1)
- `ORE_COPPER` (2)
- `ORE_MITHRIL` (3)
- `ORE_COAL` (4)
- `ORE_TIN` (5)
- `ORE_SILVER` (6)
- `ORE_GOLD` (7)
- `ORE_TITANIUM` (8)
- `ORE_OBSIDIAN` (9)
- `ORE_CRYSTAL` (10)
- `ORE_NANOMATERIAL` (11)
- `ORE_TRITIUM` (12)

Basic components are also enumerated for future crafting systems:

- `ITEM_IRON_BAR` (1001)
- `ITEM_COPPER_BAR` (1002)
- `ITEM_MITHRIL_BAR` (1003)
- `ITEM_ENGINE_PART` (1004)
- `ITEM_TITANIUM_BAR` (1005)
- `ITEM_ADVANCED_ENGINE_PART` (1006)
- `ITEM_FUSION_REACTOR` (1007)
- `ITEM_ACCUMULATOR` (1008)

Use the helper methods in `Game` to add, subtract, set, transfer, and query ore amounts
by planet ID (`PLANET_TERRA` etc.) and ore ID, and to read mining rates with
`get_rate` or list all resources for a planet with `get_planet_resources`.

## Supply Routes and Convoys

Planet-to-planet transfers no longer teleport resources. Calling
`Game::transfer_ore` schedules a convoy on the appropriate supply route,
immediately deducting cargo from the origin but requiring the shipment to
travel for a set duration before it arrives. Each route tracks a baseline
travel time, minimum escort rating, and ambient raid risk; planets contribute
escort strength from their garrisons and any fleets in orbit, accelerating
convoys and suppressing raids when the requirement is met. Raider assaults
dramatically increase interdiction chances—if a planet is under attack while a
convoy is en route, the cargo may be partially or completely destroyed. Query
`Game::get_active_convoy_count()` to monitor shipments that are still in
transit. Successful deliveries and ambushes are recorded in the lore log so
players can react by reinforcing escort fleets or clearing assault threats.
Colony infrastructure now feeds into this logistics layer: constructing a Trade
Relay Nexus on either endpoint of a route grants a 12% speed bonus and an 18%
raid-risk reduction per relay, so outfitting both the origin and destination
builds meaningfully faster, safer trade lanes.

Routes also track how long they linger at high threat levels. When sustained
losses keep a route above a threat rating of five for roughly a minute, the
Game backend will escalate the pressure into a full raider assault on whichever
endpoint is unlocked and not already under siege. The assault is started via
`Game::start_raider_assault`, noted in the lore log with the affected route, and
resets the escalation timer so commanders must suffer renewed convoy losses
before another pressure spike can occur.

### Escort Veterancy

Dedicated escort fleets now build convoy experience that directly improves
their performance. Each successful delivery grants veterancy experience scaled
by the route's escort requirement, lingering threat level, and whether the
convoy survived a raid. Every 60 experience unlocks an additional escort-rating
bonus point (up to +8), which feeds into `Game::calculate_fleet_escort_rating`
alongside ship composition. Higher escort ratings shorten travel times and cut
raid odds, so veteran formations measurably improve supply reliability. Heavy
losses drain this experience—if raiders destroy the convoy or inflict severe
casualties, `Game::record_convoy_loss` applies a veterancy penalty and logs the
setback in the lore feed. UI layers can surface progression with
`Game::get_fleet_escort_veterancy` and `Game::get_fleet_escort_veterancy_bonus`,
while level-up and decay events are automatically narrated so designers can
react to the evolving logistics story.

## Base Building

The `BuildingManager` now exposes a full roster of factory, defense, and infrastructure
structures so late-game research opens new construction options. Each definition carries
width/height footprint, energy draw or generation, logistics impact, and crafting inputs
where applicable.

| Building | Purpose | Unlocked by |
| -------- | ------- | ----------- |
| Mine Core | Central extractor that anchors every colony | Always available |
| Smelting Facility | Refines iron bars from raw ore | Always available |
| Processing Unit | Refines copper bars | Always available |
| Crafting Bay | Assembles engine parts | Always available |
| Facility Workshop | Crafts advanced engine parts | Crafting Mastery |
| Orbital Shipyard | Builds fusion reactors for fleet production | Crafting Mastery |
| Trade Relay Nexus | Upgrades convoy routes with faster travel and lower raid risk | Interstellar Trade Networks |
| Tritium Extractor | Distills tritium from obsidian and crystal | Armament Enhancement II |
| Proximity Radar Array | Expands logistics while powering static defenses | Defensive Fortification I |
| Mobile Defense Radar | Further boosts logistics support coverage | Defensive Fortification II |
| Shield Generator | Consumes power to harden a colony | Defensive Fortification II |
| Defense/Plasma/Railgun Turrets | Tiered defensive emplacements with escalating upkeep | Def. Fortification I & Arm. Enh. I/III |
| Flagship Dock | Unique facility that expands logistics for the flagship | Armament Enhancement III |
| Helios Beacon | Unique beacon that supplies logistics and auxiliary power | Defensive Fortification III |

Conveyors, transfer nodes, power generators, solar arrays, and upgrade stations remain
available early, providing the logistics and energy throughput needed to run the new
advanced structures.

## Fleets and Ships

Ship identifiers now span the complete combat roster:

- `SHIP_SHIELD` (1) – Guardian cruiser that screens the line.
- `SHIP_RADAR` (2) – Tactical scout that coordinates flanking strikes.
- `SHIP_SALVAGE` (3) – Heavy hauler for post-battle recovery.
- `SHIP_CAPITAL` (4) / `SHIP_CAPITAL_BATTLESHIP` – Battleship-class flagship.
- `SHIP_TRANSPORT` (5) – Logistic shuttle that ferries supplies between fleets.
- `SHIP_CORVETTE` (6) – Fast attack corvette unlocked with Armament Enhancement I.
- `SHIP_INTERCEPTOR` (7) – Strike interceptor fielded after Armament Enhancement II.
- `SHIP_REPAIR_DRONE` (8) – Automated hull repair platform from Repair Drone Technology.
- `SHIP_SUNFLARE_SLOOP` (9) – Helios-aligned support craft that recharges shields.
- `SHIP_FRIGATE_ESCORT` (10) – Auxiliary escort frigate unlocked via Auxiliary Frigate Development.
- `SHIP_FRIGATE_SUPPORT` (11) – Support frigate that bolsters both hull and shield sustain.
- `SHIP_CAPITAL_CARRIER` (12) – Carrier capital hull granted by Auxiliary Frigate Development.
- `SHIP_CAPITAL_DREADNOUGHT` (13) – Dreadnought hull available alongside the carrier.

Research milestones gate construction: Crafting Mastery enables transports, Armament
Enhancement tiers unlock corvettes and interceptors, Shield Technology enables
Sunflare sloops, Repair Drone Technology deploys automated drones, and Auxiliary
Frigate Development expands the auxiliary frigate and capital roster while raising
the global capital ship limit to two hulls. Capital Ship Initiative is still required
for the first flagship slot. Sunflare sloops and repair drones also provide tangible
combat benefits—when docked during an assault they regenerate defender shields and hull,
scaling with the number of support craft on the field.

Miscellaneous location identifiers:

- `MISC_ASTEROID_HIDEOUT` (1)
- `MISC_OUTPOST_NEBULA_X` (2)

Use `create_fleet` to register a new fleet. Add individual ships with
`create_ship`, which returns a unique ship ID even for multiple ships of the
same class. Each ship tracks armor, hit points, and shields; manipulate these
values with `set_ship_*`, `add_ship_*`, `sub_ship_*`, and query them with
`get_ship_*`. Control a fleet's position with `set_fleet_location_planet`,
`set_fleet_location_travel`, or `set_fleet_location_misc`, and read its current
location with `get_fleet_location`.

## Achievements

The backend now tracks persistent achievements so campaign progress can trigger
global rewards and lore entries. The catalog spans logistics milestones and the
full sequence of story quests:

- Progression anchors:
  - `Second Home` – Unlock any additional colony world.
  - `Research Pioneer` – Complete three major research projects.
  - `Logistics Pilot` – Deliver three convoys successfully.
  - `Streak Guardian` – Push the longest delivery streak to five runs.
- Story milestones:
  - `Perimeter Guardian` – Complete Initial Raider Skirmishes.
  - `Terra's Shield` – Complete Defense of Terra.
  - `Cipher Breaker` – Complete Investigate Raider Motives.
  - `Supply Line Sentinel` – Complete Secure Supply Lines.
  - `Streak Specialist` – Complete Steady Supply Streak.
  - `Escort Commander` – Complete High-Value Escort.
- `Climactic Victor` – Complete Climactic Battle.
- `Decisive Arbiter` – Resolve The Critical Decision.
- `Order's Hammer` – Complete Order's Last Stand.
- `Rebellion's Hope` – Complete Rebellion Rising.

## Branch endgames and scripted assaults

Resolving The Critical Decision now branches the campaign into multi-stage finales
that tie logistics to late-game assaults. Executing Blackthorne unlocks Order
quests that demand calm convoy lanes (maximum route threat below roughly 2.5) and
construction of Proximity Radar coverage before Marshal Rhea authorizes a
scripted strike on Mars. Sparing Blackthorne unlocks a rebellion arc that hinges
on Interstellar Trade research, Trade Relay deployment, and keeping route threat
under 3.0 before Captain Blackthorne rallies a liberation assault on Zalthor.

When these quests complete, `Game::handle_quest_completion` clears threat on the
relevant supply routes, logs lore entries, and calls
`Game::trigger_branch_assault` so the game client can assign fleets to the
branch-specific battle. Victories increment branch assault counters exposed via
`Game::build_quest_context`, letting designers gate the final reward quest on
successful defense rather than timers. Failing the assault re-triggers the strike
while the quest remains active, ensuring repeated attacks require renewed threat
buildup and fresh preparations.

Use `Game::get_achievement_ids` to enumerate known achievements and
`Game::get_achievement_info` to inspect the current status, progress, and target
for each entry. Whenever an achievement is earned, Archivist Lyra records the
milestone in the lore log so UI layers can surface the celebration.
