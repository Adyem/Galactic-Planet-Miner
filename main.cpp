#include "app_constants.hpp"
#include "backend_client.hpp"
#include "menu_localization.hpp"
#include "main_menu_system.hpp"
#include "game_bootstrap.hpp"
#include "save_system_background.hpp"
#include "player_profile.hpp"
#include "ui_input.hpp"
#include "ui_menu.hpp"

// Aggregated from refactored main menu modules
#if GALACTIC_HAVE_SDL2
#    include "main_menu_helpers.cpp"
#    include "main_menu_alerts.cpp"
#    include "main_menu_overlays.cpp"
#    include "main_menu_event_loop.cpp"
#    include "main_menu_actions.cpp"
#else
#    include "libft/Printf/printf.hpp"

int main()
{
    ft_fprintf(stderr,
        "Galactic Planet Miner was built without SDL2 support. Install SDL2 (libsdl2 and SDL2_ttf) and rebuild to access the main menu.\n");
    return 1;
}
#endif
