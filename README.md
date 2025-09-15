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

Basic components are also enumerated for future crafting systems:

- `ITEM_IRON_BAR` (1001)
- `ITEM_COPPER_BAR` (1002)
- `ITEM_MITHRIL_BAR` (1003)
- `ITEM_ENGINE_PART` (1004)

Use the helper methods in `Game` to add, subtract, set, transfer, and query ore amounts
by planet ID (`PLANET_TERRA` etc.) and ore ID, and to read mining rates with
`get_rate` or list all resources for a planet with `get_planet_resources`.

## Fleets and Ships

Ship identifiers:

- `SHIP_SHIELD` (1)
- `SHIP_RADAR` (2)
- `SHIP_SALVAGE` (3)
- `SHIP_CAPITAL` (4)

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
