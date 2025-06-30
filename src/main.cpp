#include <SFML/Graphics.hpp>
#include "planet.h"
#include "game_data.h"
#include <iostream>
#include <sstream>

// Simple interface based on SFML. The previous version only opened an empty
// window.  This version displays basic planet information so the game can be
// interacted with visually.  Use the left and right arrow keys to cycle through
// planets.

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Galactic Planet Miner");

    // Load a system font for displaying text.  We use DejaVuSans which is
    // available on most Linux systems.
    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        std::cerr << "Failed to load font.\n";
        return 1;
    }

    // Setup game data
    PlanetManager planetManager(PLANET_DATA);
    std::size_t selected = 0;

    sf::Clock clock;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Right)
                    selected = (selected + 1) % planetManager.getPlanetsConst().size();
                else if (event.key.code == sf::Keyboard::Left)
                    selected = (selected + planetManager.getPlanetsConst().size() - 1) % planetManager.getPlanetsConst().size();
            }
        }

        // Update production based on elapsed time
        float elapsed = clock.restart().asSeconds();
        planetManager.produceAll(elapsed, false);

        // Prepare texts
        const Planet &p = planetManager.getPlanetsConst()[selected];
        std::ostringstream info;
        info << "Planet: " << p.getName() << "\n";
        info << "Energy: " << static_cast<int>(p.getCurrentEnergy()) << "/" << static_cast<int>(p.getMaxEnergy()) << "\n\nResources:\n";
        for (const auto &entry : p.getStorageMap()) {
            if (entry.second > 0)
                info << "  " << entry.first << ": " << entry.second << "\n";
        }
        info << "\nUse Left/Right arrows to change planet";

        sf::Text text(info.str(), font, 16);
        text.setFillColor(sf::Color::White);

        window.clear(sf::Color::Black);
        window.draw(text);
        window.display();
    }
    return 0;
}
