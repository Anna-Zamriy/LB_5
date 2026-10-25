#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <optional>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const float GROUND_Y = 550.f;
const float START_X = 50.f;
const float START_Y = GROUND_Y;
const float GRAVITY = 980.f;

int main()
{
    sf::RenderWindow window(sf::VideoMode({ 1000, 650 }), "SFML");
    window.setFramerateLimit(60);

    sf::Font font;
    bool hasFont = font.openFromFile("../resources/fonts/arial.ttf");

    sf::Text infoText(font, "", 18);
    infoText.setFillColor(sf::Color::White);
    infoText.setPosition({ 20.f, 20.f });

    sf::CircleShape projectile(10.f);
    projectile.setOrigin({ 10.f, 10.f });
    projectile.setFillColor(sf::Color::Red);

    sf::RectangleShape ground({ 1000.f, 100.f });
    ground.setFillColor(sf::Color(80, 80, 80));
    ground.setPosition({ 0.f, GROUND_Y });

    float v0 = 500.f;          // Початкова швидкість
    float angleDeg = 45.f;      // Кут кидання

    sf::Vector2f position(START_X, START_Y);
    sf::Vector2f velocity(0.f, 0.f);

    bool isSimulating = false;
    bool isPaused = false;
    float flightTime = 0.f;
    float maxReachX = 0.f;

    std::vector<sf::Vector2f> trajectory;
    sf::Clock clock;

    auto resetSimulation = [&]() {
        position = sf::Vector2f(START_X, START_Y);
        float rad = angleDeg * static_cast<float>(M_PI) / 180.f;
        velocity.x = v0 * std::cos(rad);
        velocity.y = -v0 * std::sin(rad); // Вісь Y у SFML напрямлена вниз
        flightTime = 0.f;
        maxReachX = 0.f;
        trajectory.clear();
        trajectory.push_back(position);
        projectile.setPosition(position);
        };

    resetSimulation();

    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();

        // 1. Обробка подій
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }

            if (event->is<sf::Event::KeyPressed>())
            {
                const auto key = event->getIf<sf::Event::KeyPressed>();

                if (key->code == sf::Keyboard::Key::Up)
                {
                    v0 = std::clamp(v0 + 20.f, 100.f, 1200.f);
                    if (!isSimulating) resetSimulation();
                }
                if (key->code == sf::Keyboard::Key::Down)
                {
                    v0 = std::clamp(v0 - 20.f, 100.f, 1200.f);
                    if (!isSimulating) resetSimulation();
                }
                if (key->code == sf::Keyboard::Key::Right)
                {
                    angleDeg = std::clamp(angleDeg + 1.f, 5.f, 85.f);
                    if (!isSimulating) resetSimulation();
                }
                if (key->code == sf::Keyboard::Key::Left)
                {
                    angleDeg = std::clamp(angleDeg - 1.f, 5.f, 85.f);
                    if (!isSimulating) resetSimulation();
                }
                if (key->code == sf::Keyboard::Key::Space)
                {
                    resetSimulation();
                    isSimulating = true;
                    isPaused = false;
                }
                if (key->code == sf::Keyboard::Key::P && isSimulating)
                {
                    isPaused = !isPaused;
                }
                if (key->code == sf::Keyboard::Key::R)
                {
                    isSimulating = false;
                    isPaused = false;
                    resetSimulation();
                }
            }
        }

        // 2. Оновлення фізичної моделі
        if (isSimulating && !isPaused)
        {
            flightTime += dt;
            velocity.y += GRAVITY * dt;
            position.x += velocity.x * dt;
            position.y += velocity.y * dt;

            if (position.y >= GROUND_Y)
            {
                position.y = GROUND_Y;
                isSimulating = false;
            }

            projectile.setPosition(position);
            trajectory.push_back(position);
            maxReachX = position.x - START_X;
        }

        // 3. Відображення сцени 
        window.clear(sf::Color(30, 30, 40));
        window.draw(ground);

        if (trajectory.size() > 1)
        {
            sf::VertexArray lines(sf::PrimitiveType::LineStrip);
            for (const auto& point : trajectory)
            {
                lines.append(sf::Vertex{point, sf::Color::Yellow});
            }
            window.draw(lines);
        }

        window.draw(projectile);

        if (hasFont)
        {
            std::string statusStr = isSimulating ? (isPaused ? "PAUSED" : "FLYING") : "READY";
            std::string info = "Status: " + statusStr + "\n" +
                "v0: " + std::to_string(static_cast<int>(v0)) + " px/s [Up/Down]\n" +
                "Angle: " + std::to_string(static_cast<int>(angleDeg)) + " deg [Left/Right]\n" +
                "Flight Time: " + std::to_string(flightTime) + " s\n" +
                "Distance: " + std::to_string(maxReachX) + " px\n\n" +
                "Controls: [Space] Launch, [P] Pause, [R] Reset";
            infoText.setString(info);
            window.draw(infoText);
        }

        window.display();
    }

    return 0;
}