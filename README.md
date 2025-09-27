# Galactic Planet Miner

This repository contains early backend logic for a Galactic Planet Miner game. The code demonstrates
basic game state management and communication with a backend server using the `libft` library.

## Building and Testing

Before compiling, make sure the `libft` submodule is available so the custom containers, JSON
helpers, and test harness can link correctly:

```sh
git submodule update --init --recursive
```

### Running the suite

The default `make` target builds both the game binary and the consolidated `test` executable. After
the build finishes, run the automated regression tests:

```sh
make
./test
```

The harness boots the lightweight HTTP server that ships with `libft`, drives the backend client,
and executes the scenario helpers under `tests/`.

### Coverage highlights

The campaign regression in `validate_initial_campaign_flow` plays through early quests, research
unlocks, convoy shipments, and assault triggers to ensure normal pacing still works end-to-end after
logic changes.【F:tests/game_test_campaign.cpp†L40-L195】 Additional suites in `game_test_backend.cpp`,
`game_test_support.cpp`, `game_test_energy.cpp`, and friends probe combat hooks, building
requirements, and resource accounting so broad gameplay changes surface immediately when the suite is
run.

### Edge cases and extreme scenarios

* `verify_save_system_edge_cases` round-trips empty planet and fleet maps, then feeds sparsely
  populated JSON blobs to confirm deserializers synthesize default values without crashing.【F:tests/game_test_save.cpp†L158-L233】
* `verify_save_system_invalid_inputs` protects against null pointers and malformed payloads across
  planets, fleets, research, and achievements so bad data never mutates live state.【F:tests/game_test_save.cpp†L236-L291】
* `validate_save_system_serialized_samples` stress-tests serialization scaling by packing enormous
  stockpiles, veteran fleets, and precision timing into JSON and then verifying every scaled field on
  restore.【F:tests/game_test_save.cpp†L296-L445】
* `verify_save_system_allocation_failures` injects allocation hooks that force the JSON writers to
  fail and proves the save system cleans up and reports the error without leaving stale checkpoint
  blobs behind.【F:tests/game_test_save.cpp†L448-L491】
* `verify_save_system_extreme_scaling` pushes the serializers with values near floating-point limits,
  plus explicit NaN and infinity carryovers, and confirms the restored state preserves those
  boundary semantics safely.【F:tests/game_test_save.cpp†L493-L583】

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
- `SHIP_CAPITAL_JUGGERNAUT` (4) / `SHIP_CAPITAL` / `SHIP_CAPITAL_BATTLESHIP` – Celestial Juggernaut flagship that doubles as a cargo platform.
- `SHIP_TRANSPORT` (5) – Logistic shuttle that ferries supplies between fleets.
- `SHIP_CORVETTE` (6) – Fast attack corvette unlocked with Armament Enhancement I.
- `SHIP_INTERCEPTOR` (7) – Strike interceptor fielded after Armament Enhancement II.
- `SHIP_REPAIR_DRONE` (8) – Automated hull repair platform from Repair Drone Technology.
- `SHIP_SUNFLARE_SLOOP` (9) – Helios-aligned support craft that recharges shields.
- `SHIP_FRIGATE_SOVEREIGN` (10) / `SHIP_FRIGATE_ESCORT` – Sovereign escort frigate for battle-line reinforcement.
- `SHIP_FRIGATE_PROTECTOR` (11) / `SHIP_FRIGATE_SUPPORT` – Protector support frigate that bolsters shield regeneration.
- `SHIP_CAPITAL_NOVA` (12) / `SHIP_CAPITAL_CARRIER` – Nova Carrier that deploys strike craft support wings.
- `SHIP_CAPITAL_OBSIDIAN` (13) / `SHIP_CAPITAL_DREADNOUGHT` – Obsidian Sovereign dreadnought geared for direct assaults.
- `SHIP_CAPITAL_PREEMPTOR` (14) – Long-range Preemptor siege platform unlocked alongside auxiliary capital research.
- `SHIP_CAPITAL_PROTECTOR` (15) – Aurora Protector bulwark that emphasizes shield projection.
- `SHIP_CAPITAL_ECLIPSE` (16) – Eclipse Monolith support capital capable of remote hull repairs.
- `SHIP_FRIGATE_JUGGERNAUT` (17) – Juggernaut-class frigate that supplements convoy capacity.
- `SHIP_FRIGATE_CARRIER` (18) – Carrier frigate that fields light drone complements.
- `SHIP_FRIGATE_PREEMPTOR` (19) – Preemptor artillery frigate that provides ranged fire support.
- `SHIP_FRIGATE_ECLIPSE` (20) – Eclipse repair frigate that restores allied hull integrity in combat.

Research milestones gate construction: Crafting Mastery enables transports, Armament
Enhancement tiers unlock corvettes and interceptors, Shield Technology enables
Sunflare sloops, Repair Drone Technology deploys automated drones, and Auxiliary
Frigate Development expands the auxiliary frigate and capital roster—including
the Nova Carrier, Obsidian Sovereign, Preemptor, Aurora Protector, Eclipse Monolith,
and their frigate counterparts—while raising the global capital ship limit to two
hulls. Capital Ship Initiative is still required for the first flagship slot. Sunflare
sloops and repair drones also provide tangible
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

## Save and Load Scaffolding

Initial save/load support now lives in `SaveSystem`. The class wraps the `libft`
JSON helpers so campaign state can be exported and restored without depending on
the C++ standard library serializers. Two helpers are currently available:

- `serialize_planets` / `deserialize_planets` round-trip the unlocked planet
  map, including stored ore amounts, mining rates, and fractional carryover
  buffers so production math stays stable after a reload.
- `serialize_fleets` / `deserialize_fleets` persist fleet rosters, travel
  assignments, escort veterancy, and all ship statistics. Fleet snapshots use
  stable ship identifiers and scale floating-point values into integers to avoid
  precision loss in JSON.
- `serialize_research` / `deserialize_research` capture the entire research
  catalog, including active projects, time remaining, and the global duration
  scale so campaign pacing matches the moment a save was taken.
- `serialize_achievements` / `deserialize_achievements` preserve achievement
  progress and completion flags so lore entries and rewards remain consistent
  across reloads.

The serializers rely on new accessors such as `ft_planet::get_carryover` and
`ft_fleet::add_ship_snapshot` so game code can hand the `SaveSystem` concrete
state. Once integrated with the campaign flow, these helpers enable quick
checkpointing and prototyping of persistence features.

`Game` now checkpoints automatically whenever a major quest completes or a
research project finishes, stamping the snapshot with a descriptive tag such as
`quest_completed_3` or `research_completed_12`. Campaign layers can also call
`Game::save_campaign_checkpoint` directly to capture ad-hoc saves and
`Game::reload_campaign_checkpoint` to restore the most recent snapshot using the
embedded JSON state.

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
