#include "buildings.hpp"
#include "game.hpp"
#include "research.hpp"
#include "libft/Libft/libft.hpp"
#include "ft_map_snapshot.hpp"

BuildingManager::BuildingManager()
    : _definitions(),
      _planets(),
      _building_unlocks(),
      _crafting_energy_multiplier(1.0),
      _crafting_speed_multiplier(1.0),
      _global_energy_multiplier(1.0)
{
#include "buildings_catalog_manager_constructor_industry.cpp"
#include "buildings_catalog_manager_constructor_logistics.cpp"
#include "buildings_catalog_manager_constructor_energy.cpp"
#include "buildings_catalog_manager_constructor_advanced.cpp"
#include "buildings_catalog_manager_constructor_unlocks.cpp"
}
