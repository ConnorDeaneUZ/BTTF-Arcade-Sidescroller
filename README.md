

<!-- Used to compile code -->

clang++ -std=c++17 main.cpp -o sfml-app \
-I/opt/homebrew/opt/sfml/include \
-L/opt/homebrew/opt/sfml/lib \
-lsfml-graphics -lsfml-window -lsfml-system




<!-- Used to run compiled code -->
./sfml-app




<!--   // sf::CircleShape shape(100.f);
  // shape.move({300.f, 5.f});
  // sf::Vector2f circlePosition = shape.getPosition();

  // sf::CircleShape triangle(80.f, 3);
  // triangle.setFillColor(sf::Color::Red);
  // triangle.move({600.f, 5.f});
  // triangle.setRotation(sf::degrees(0));
  // sf::Vector2f trianglePosition = triangle.getPosition();

  // sf::CircleShape hexagon(80.f, 6);
  // hexagon.setFillColor(sf::Color::Green); -->
