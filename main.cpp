#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <vector>
#include <random>

int main() {
  sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML Playground");
  sf::RectangleShape player(sf::Vector2f(50.f, 50.f));


  std::vector<sf::CircleShape> blocks;
  std::mt19937 rng(std::random_device{}());
  std::uniform_real_distribution<float> distX(0.f, 770.f);


  player.setPosition({375.f, 550.f});
  float speed = 0.3f;
  float blockSpeed = 0.2f;
  float spawnInterval = 0.3f;

  sf::Clock spawnClock;


  while (window.isOpen()) {
    // Handle events
    while (const std::optional event = window.pollEvent())
      if (event->is<sf::Event::Closed>())
        window.close();

    // Contorl player
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
      player.move(sf::Vector2f(-speed, 0));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
      player.move(sf::Vector2f(+speed, 0));
    }

    // Keep player inside window
    sf::Vector2f pos = player.getPosition();
    if (pos.x < 0.f) {
      player.setPosition(sf::Vector2f(0.f, pos.y));
    }
    if (pos.x + player.getSize().x > 800.f) {
      player.setPosition(sf::Vector2f(800.f - player.getSize().x, pos.y));
    }

    // Spawn new blocks
    if (spawnClock.getElapsedTime().asSeconds() > spawnInterval) {
      // sf::RectangleShape block(sf::Vector2f(30.f, 1.f));
      sf::CircleShape block(10.f);
      block.setPosition(sf::Vector2f(distX(rng), 0.f));
      block.setFillColor(sf::Color::Red);
      blocks.push_back(block);
      spawnClock.restart();
    }

    // Move blocks down
    for (auto& block : blocks) {
      block.move(sf::Vector2f(0.f, blockSpeed));
    }

    // Remove blocks that leave screen
    blocks.erase(
      std::remove_if(blocks.begin(), blocks.end(),
        [](const sf::CircleShape& b) {
          return b.getPosition().y > 600.f;
        }
      ),
      blocks.end()
    );

    // Draw elements
    window.clear();
    for (const auto& block : blocks) {
      window.draw(block);
    }
    window.draw(player);
    window.display();
  }

  return 0;
}
