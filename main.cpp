#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <string>

enum class GameState {
  Menu,
  Playing
};

struct Point {
  sf::CircleShape shape;
  int value;
};

int main() {
  sf::RenderWindow window(sf::VideoMode({1200, 1200}), "BACK TO THE FUTURE");
  
  GameState gameState = GameState::Menu;
  
  // Load font
  sf::Font font;
  bool fontLoaded = font.openFromFile("/Library/Fonts/Arial.ttf") ||
                    font.openFromFile("/System/Library/Fonts/Helvetica.ttc") ||
                    font.openFromFile("/opt/homebrew/opt/sfml/share/SFML/fonts/DroidSansMono.ttf");

  // Background music
  sf::Music backgroundMusic;
  if (backgroundMusic.openFromFile("audio/background.ogg") ||
      backgroundMusic.openFromFile("audio/background.wav") ||
      backgroundMusic.openFromFile("audio/back-to-the-arcade.mp3")) {
    backgroundMusic.setVolume(50.f);
    backgroundMusic.play();
  }

  sf::Texture playerTexture;
  if (!playerTexture.loadFromFile("sprites/car.png")) {
    return -1;
  }

  sf::Sprite playerSprite(playerTexture);
  playerSprite.setOrigin(sf::Vector2f(playerTexture.getSize().x * 0.5f, playerTexture.getSize().y * 0.5f));
  playerSprite.setPosition(sf::Vector2f(window.getSize().x * 0.5f, window.getSize().y * 0.5f));
  playerSprite.setScale(sf::Vector2f(0.5f, 0.5f));
  
  // Load menu background image
  sf::Texture menuBackgroundTexture;
  bool menuBgLoaded = menuBackgroundTexture.loadFromFile("sprites/bttf-screen.png");
  sf::Sprite menuBackground(menuBackgroundTexture);
  if (menuBgLoaded) {
    menuBackground.setPosition(sf::Vector2f(0.f, 0.f));
  }
  
  // Load gameplay background image
  sf::Texture gameBackgroundTexture;
  bool gameBgLoaded = gameBackgroundTexture.loadFromFile("sprites/bttf-sky.webp");
  sf::Sprite gameBackground(gameBackgroundTexture);
  if (gameBgLoaded) {
    gameBackground.setPosition(sf::Vector2f(0.f, 0.f));
  }

  std::vector<sf::CircleShape> blocks;
  std::vector<Point> points;
  std::mt19937 rng(std::random_device{}());
  std::uniform_real_distribution<float> distY(0.f, 1200.f);
  std::uniform_int_distribution<int> pointValue(1, 3);
  
  int score = 0;

  float speed = 0.3f;
  float blockSpeed = 0.2f;
  float spawnInterval = 0.3f;
  
  // Player velocity for weight effect
  float velocityX = 0.f;
  float velocityY = 0.f;
  float acceleration = 0.05f;
  float friction = 0.88f;

  sf::Clock spawnClock;

  // Helper: circle vs axis-aligned-rectangle collision using FloatRect
  auto circleRectCollision = [](const sf::CircleShape& c, const sf::FloatRect& r) {
    float cx = c.getPosition().x + c.getRadius();
    float cy = c.getPosition().y + c.getRadius();

    float rx = r.position.x;
    float ry = r.position.y;
    float rw = r.size.x;
    float rh = r.size.y;

    float closestX = std::clamp(cx, rx, rx + rw);
    float closestY = std::clamp(cy, ry, ry + rh);

    float dx = cx - closestX;
    float dy = cy - closestY;

    return (dx * dx + dy * dy) <= (c.getRadius() * c.getRadius());
  };

  while (window.isOpen()) {
    // Handle events
    while (auto event = window.pollEvent()) {
      if (event->is<sf::Event::Closed>()) {
        window.close();
      }
      if (auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
        if (gameState == GameState::Menu && keyEvent->code == sf::Keyboard::Key::Space) {
          gameState = GameState::Playing;
          spawnClock.restart();
          blocks.clear();
          points.clear();
          score = 0;
          playerSprite.setPosition(sf::Vector2f(600.f, 600.f));
          backgroundMusic.play();
        }
      }
    }

    if (gameState == GameState::Playing) {
      // Control player with acceleration for weight effect
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
        velocityX -= acceleration;
      }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
        velocityX += acceleration;
      }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
        velocityY -= acceleration;
      }
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
        velocityY += acceleration;
      }
      
      // Apply friction/damping to create weight effect
      velocityX *= friction;
      velocityY *= friction;
      
      // Clamp velocity to max speed
      float maxVelocity = speed * 0.8f;
      if (velocityX > maxVelocity) velocityX = maxVelocity;
      if (velocityX < -maxVelocity) velocityX = -maxVelocity;
      if (velocityY > maxVelocity) velocityY = maxVelocity;
      if (velocityY < -maxVelocity) velocityY = -maxVelocity;
      
      // Move player by velocity
      playerSprite.move(sf::Vector2f(velocityX, velocityY));

      // Keep player inside window (sprite origin is center)
      sf::Vector2f pos = playerSprite.getPosition();
      sf::FloatRect pb = playerSprite.getGlobalBounds();
      float halfW = pb.size.x * 0.5f;
      float halfH = pb.size.y * 0.5f;
      if (pos.x - halfW < 0.f) {
        playerSprite.setPosition(sf::Vector2f(halfW, pos.y));
      }
      if (pos.x + halfW > 1200.f) {
        playerSprite.setPosition(sf::Vector2f(1200.f - halfW, pos.y));
      }
      if (pos.y - halfH < 0.f) {
        playerSprite.setPosition(sf::Vector2f(pos.x, halfH));
      }
      if (pos.y + halfH > 1200.f) {
        playerSprite.setPosition(sf::Vector2f(pos.x, 1200.f - halfH));
      }

      // Spawn new blocks from right side
      if (spawnClock.getElapsedTime().asSeconds() > spawnInterval) {
        sf::CircleShape block(10.f);
        block.setPosition(sf::Vector2f(1200.f, distY(rng)));
        block.setFillColor(sf::Color::Yellow);
        blocks.push_back(block);
        
        // Randomly spawn a point every 2nd-3rd block
        if (rng() % 3 == 0) {
          Point newPoint;
          newPoint.shape.setRadius(7.f);
          newPoint.value = pointValue(rng);
          newPoint.shape.setPosition(sf::Vector2f(1200.f, distY(rng)));
          newPoint.shape.setFillColor(sf::Color::Red);
          points.push_back(newPoint);
        }
        
        spawnClock.restart();
      }

      // Move blocks left
      for (auto& block : blocks) {
        block.move(sf::Vector2f(-blockSpeed, 0.f));
      }
      
      // Move points left
      for (auto& point : points) {
        point.shape.move(sf::Vector2f(-blockSpeed, 0.f));
      }

      // Check collision between player and any block
      bool collided = false;
      sf::FloatRect playerBounds = playerSprite.getGlobalBounds();
      for (const auto& block : blocks) {
        if (circleRectCollision(block, playerBounds)) {
          collided = true;
          break;
        }
      }
      if (collided) {
        backgroundMusic.stop();
        gameState = GameState::Menu;
        velocityX = 0.f;
        velocityY = 0.f;
      }
      
      // Check collision with collectible points
      for (int i = 0; i < points.size(); ++i) {
        if (circleRectCollision(points[i].shape, playerBounds)) {
          score += points[i].value;
          points.erase(points.begin() + i);
          --i;
        }
      }

      // Remove blocks that leave screen
      blocks.erase(
        std::remove_if(blocks.begin(), blocks.end(),
          [](const sf::CircleShape& b) {
            return b.getPosition().x < 0.f;
          }
        ),
        blocks.end()
      );
      
      // Remove points that leave screen
      points.erase(
        std::remove_if(points.begin(), points.end(),
          [](const Point& p) {
            return p.shape.getPosition().x < 0.f;
          }
        ),
        points.end()
      );

      // Draw game
      window.clear(sf::Color::Black);
      
      if (gameBgLoaded) {
        window.draw(gameBackground);
      }
      
      for (const auto& block : blocks) {
        window.draw(block);
      }
      for (const auto& point : points) {
        window.draw(point.shape);
      }
      window.draw(playerSprite);
      
      // Draw score at top
      if (fontLoaded) {
        sf::Text scoreText(font, "Score: " + std::to_string(score), 40);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(sf::Vector2f(10.f, 10.f));
        window.draw(scoreText);
      }
    } else if (gameState == GameState::Menu) {
      // Draw menu with background image
      window.clear(sf::Color::Black);
      
      if (menuBgLoaded) {
        window.draw(menuBackground);
      }
      
      // Draw semi-transparent overlay for button area
      sf::RectangleShape overlay(sf::Vector2f(600.f, 180.f));
      overlay.setPosition(sf::Vector2f(300.f, 1000.f));
      overlay.setFillColor(sf::Color(0, 0, 0, 180));
      window.draw(overlay);
      
      // Start button
      if (fontLoaded) {
        sf::RectangleShape button(sf::Vector2f(500.f, 120.f));
        button.setPosition(sf::Vector2f(350.f, 1020.f));
        button.setFillColor(sf::Color::Green);
        window.draw(button);
        
        sf::Text buttonText(font, "PRESS SPACE TO START", 40);
        buttonText.setFillColor(sf::Color::Black);
        buttonText.setPosition(sf::Vector2f(375.f, 1030.f));
        window.draw(buttonText);
      } else {
        // Fallback without font
        sf::RectangleShape button(sf::Vector2f(500.f, 120.f));
        button.setPosition(sf::Vector2f(350.f, 1020.f));
        button.setFillColor(sf::Color::Green);
        window.draw(button);
      }
    }

    window.display();
  }

  return 0;
}
