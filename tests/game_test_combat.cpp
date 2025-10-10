#include "../libft/Libft/libft.hpp"
#include "../libft/System_utils/test_runner.hpp"
#include "../libft/Template/vector.hpp"
#include "../libft_math_bridge.hpp"
#include "fleets.hpp"

#define private public
#define protected public
#include "combat.hpp"
#include "game.hpp"
#undef private
#undef protected

#include "game_test_scenarios.hpp"

#include "game_test_combat_ship_profiles.cpp"
#include "game_test_combat_raider_behaviors.cpp"
