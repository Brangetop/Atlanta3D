#include <SFML/Graphics.hpp>
#include <vector>

const int MAP_WIDTH = 8;
const int MAP_HEIGHT = 8;
const int TILE_SIZE = 64;

const int map[MAP_HEIGHT][MAP_WIDTH] = 
{
    {1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1}
};

int main() 
{
    sf::RenderWindow window(sf::VideoMode(MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE), "Atlanta3D Alpha");

    sf::RectangleShape wallTile(sf::Vector2f(TILE_SIZE - 2, TILE_SIZE - 2));
    wallTile.setFillColor(sf::Color::Blue); 

    float radius = (TILE_SIZE - 2) / 2.f;
    sf::CircleShape playerShape(radius);
    playerShape.setFillColor(sf::Color::Green);
    playerShape.setOrigin(radius, radius); 

    float playerX = 3 * TILE_SIZE + TILE_SIZE / 2.f;
    float playerY = 4 * TILE_SIZE + TILE_SIZE / 2.f;

    // main loop
    while (window.isOpen()) 
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        // player movement
        float player_speed = 0.2f;
        float next_x = playerX;
        float next_y = playerY;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) next_y -= player_speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) next_y += player_speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) next_x -= player_speed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) next_x += player_speed;

        int cell_x = static_cast<int>(next_x / TILE_SIZE);
        int cell_y = static_cast<int>(next_y / TILE_SIZE);

        if (cell_x >= 0 && cell_x < MAP_WIDTH && cell_y >= 0 && cell_y < MAP_HEIGHT)
        {
            if (map[cell_y][cell_x] == 0) 
            {
                playerX = next_x;
                playerY = next_y;
            }
        }


        //draw
        window.clear(sf::Color::Yellow);

        for (int y = 0; y < MAP_HEIGHT; y++) 
        {
            for (int x = 0; x < MAP_WIDTH; x++) 
            {
                if (map[y][x] == 1) {
                    wallTile.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                    window.draw(wallTile);
                }
            }
        }

        // draw the player once
        playerShape.setPosition(playerX, playerY);
        window.draw(playerShape);

        window.display();
    }

    return 0;
}
