#include <array>
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
  Playing,
  GameOver
};

struct Point {
  explicit Point(const sf::Texture& tex) : sprite(tex) {}
  sf::Sprite sprite;
};

int main() {
  sf::RenderWindow window(sf::VideoMode({1200, 1200}), "BACK TO THE FUTURE");
  
  GameState gameState = GameState::Menu;
  
  // Load font
  sf::Font font;
  bool fontLoaded = font.openFromFile("/Library/Fonts/Arial.ttf") ||
                    font.openFromFile("/System/Library/Fonts/Helvetica.ttc");

  // Background music (one of three tracks chosen at random each game)
  const std::array<const char*, 3> gameMusicPaths = {
    "audio/back-to-the-arcade.mp3",
    "audio/back-to-the-arcade-2.mp3",
    "audio/back-to-the-arcade-3.mp3"
  };
  sf::Music backgroundMusic;
  backgroundMusic.setVolume(50.f);

  sf::Music menuMusic;
  if (menuMusic.openFromFile("audio/menu-theme.mp3")) {
    menuMusic.setVolume(50.f);
    menuMusic.play();
  }

  sf::Music collectPointSound;
  if (collectPointSound.openFromFile("audio/collect-point.mp3")) {
    collectPointSound.setVolume(70.f);
  }

  sf::Music gameOverSound;
  if (gameOverSound.openFromFile("audio/game-over.mp3")) {
    gameOverSound.setVolume(10.f);
  }

  sf::Music biffVocalSound;
  if (biffVocalSound.openFromFile("audio/biff-vocal.mp3")) {
    biffVocalSound.setVolume(100.f);
}

  sf::Texture playerTexture;
  if (!playerTexture.loadFromFile("sprites/car-small.png")) {
    return -1;
  }

  sf::Sprite playerSprite(playerTexture);
  playerSprite.setOrigin(sf::Vector2f(playerTexture.getSize().x * 0.5f, playerTexture.getSize().y * 0.5f));
  playerSprite.setPosition(sf::Vector2f(window.getSize().x * 0.5f, window.getSize().y * 0.5f));
  playerSprite.setScale(sf::Vector2f(2.f, 2.f));


  sf::Texture enemyBlockTexture;
  if (!enemyBlockTexture.loadFromFile("sprites/biff-face.png")) {
    return -1;
  }

  sf::Texture pointTexture;
  if (!pointTexture.loadFromFile("sprites/pepsi-free.png")) {
    return -1;
  }

  // Load menu background image
  sf::Texture menuBackgroundTexture;
  bool menuBgLoaded = menuBackgroundTexture.loadFromFile("sprites/bttf-screen.png");
  sf::Sprite menuBackground(menuBackgroundTexture);
  if (menuBgLoaded) {
    sf:: Vector2f menuBackgroundSize = sf::Vector2f(menuBackgroundTexture.getSize().x, menuBackgroundTexture.getSize().y);
    menuBackground.setOrigin(menuBackgroundSize * 0.5f);
    menuBackground.setPosition(sf::Vector2f(window.getSize().x * 0.5f, window.getSize().y * 0.5f));
  }
  
  // Load gameplay background image
  sf::Texture gameBackgroundTexture;
  bool gameBgLoaded = gameBackgroundTexture.loadFromFile("sprites/skyline-art.png");
  sf::Sprite gameBackground(gameBackgroundTexture);
  if (gameBgLoaded) {
    sf::Vector2f gameBackgroundSize = sf::Vector2f(gameBackgroundTexture.getSize().x, gameBackgroundTexture.getSize().y);
    gameBackground.setOrigin(gameBackgroundSize * 0.5f);
    gameBackground.setPosition(sf::Vector2f(window.getSize().x * 0.5f, window.getSize().y * 0.5f));
    float scaleX = static_cast<float>(window.getSize().x) / gameBackgroundSize.x;
    float scaleY = static_cast<float>(window.getSize().y) / gameBackgroundSize.y;
    float coverScale = std::max(scaleX, scaleY);
    gameBackground.setScale(sf::Vector2f(coverScale, coverScale));
  }

  std::vector<sf::Sprite> blocks;
  std::vector<Point> points;
  std::mt19937 rng(std::random_device{}());
  std::uniform_real_distribution<float> distY(0.f, 1200.f);
  std::uniform_int_distribution<int> musicChoice(0, 2);
  
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
  sf::Clock menuFloatClock;

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
          menuMusic.stop();
          if (backgroundMusic.openFromFile(gameMusicPaths[musicChoice(rng)])) {
            backgroundMusic.play();
          }
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
        
        sf::Sprite enemyBlock(enemyBlockTexture);
        enemyBlock.setPosition(sf::Vector2f(1200.f, distY(rng)));
        enemyBlock.setScale(sf::Vector2f(0.5f, 0.5f));
        blocks.push_back(enemyBlock);
        
        // Randomly spawn a point every 2nd-3rd block
        if (rng() % 3 == 0) {
          Point newPoint(pointTexture);
          sf::Vector2u texSize = pointTexture.getSize();
          newPoint.sprite.setScale(sf::Vector2f(0.3f, 0.3f));
          newPoint.sprite.setPosition(sf::Vector2f(1200.f, distY(rng)));
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
        point.sprite.move(sf::Vector2f(-blockSpeed, 0.f));
      }

      // Check collision between player and any block
      bool collided = false;
      sf::FloatRect playerBounds = playerSprite.getGlobalBounds();
      for (const auto& block : blocks) {
        if (block.getGlobalBounds().findIntersection(playerBounds)) {
          collided = true;
          break;
        }
      }
      if (collided) {
        backgroundMusic.stop();
        gameOverSound.play();
        biffVocalSound.play();
        gameState = GameState::GameOver;
        velocityX = 0.f;
        velocityY = 0.f;
      }
      
      // Check collision with collectible points
      bool collectedAny = false;
      for (int i = static_cast<int>(points.size()) - 1; i >= 0; --i) {
        if (points[i].sprite.getGlobalBounds().findIntersection(playerBounds)) {
          score += 1;
          points.erase(points.begin() + i);
          collectedAny = true;
        }
      }
      if (collectedAny) {
        collectPointSound.play();
      } 
      // Remove points that leave screen
      points.erase(
        std::remove_if(points.begin(), points.end(),
          [](const Point& p) {
            return p.sprite.getGlobalBounds().position.x + p.sprite.getGlobalBounds().size.x < 0.f;
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
        window.draw(point.sprite);
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
        float centerX = window.getSize().x * 0.5f;
        float centerY = window.getSize().y * 0.5f;
        float amplitude = 15.f;
        float floatSpeed = 0.5f;
        float offset = amplitude * std::sin(menuFloatClock.getElapsedTime().asSeconds() * floatSpeed * 2.f * 3.14159f);
        menuBackground.setPosition(sf::Vector2f(centerX, centerY + offset));
        window.draw(menuBackground);
      }
    } else if (gameState == GameState::GameOver) {
        window.clear(sf::Color::Black);
        if (fontLoaded) {
          sf::Text gameOverText(font, "GAME OVER", 80);
          gameOverText.setFillColor(sf::Color::Red);
          sf::FloatRect textBounds = gameOverText.getLocalBounds();
          gameOverText.setPosition(sf::Vector2f(window.getSize().x * 0.5f, window.getSize().y * 0.5f - 50.f));
          window.draw(gameOverText);

          sf::Text scoreText(font, "Final Score: " + std::to_string(score), 40);
          scoreText.setFillColor(sf::Color::White);
          sf::FloatRect scoreBounds = scoreText.getLocalBounds();
          scoreText.setPosition(sf::Vector2f(window.getSize().x * 0.5f, window.getSize().y * 0.5f + 30.f));
          window.draw(scoreText);

        }
    }

    window.display();
  }

  return 0;
}
