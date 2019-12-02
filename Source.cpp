#include "SFML-2.5.1\include\SFML\Graphics.hpp"
#include "SFML-2.5.1/include/SFML/Audio.hpp"
#include <vector>

#include "Hero.h"
#include "Enemy.h"
#include "Rocket.h"

sf::Vector2f viewSize(1024, 768);
sf::VideoMode vm(viewSize.x, viewSize.y);
sf::RenderWindow window(vm, "Hello SFML Game !!!", sf::Style::Default);

void spawnEnemy();
void shoot();
bool checkCollision(sf::Sprite sprite1, sf::Sprite sprite2);
void reset();

sf::Vector2f playerPosition;
bool playerMoving = false;

sf::Texture skyTexture;
sf::Texture bgTexture;

sf::Sprite skySprite;
sf::Sprite bgSprite;

Hero hero;
std::vector<Enemy*> enemies;
std::vector<Rocket*> rockets;

float currentTime;
float previousTime = 0.0f;

int score = 0;
bool gameOver = true;

// text
sf::Font headingFont;
sf::Text headingText;
sf::Font scoreFont;
sf::Text scoreText;
sf::Text tutorialText;

// audio
sf::Music bgMusic; 
sf::SoundBuffer fireBuffer;
sf::SoundBuffer hitBuffer;
sf::Sound fireSound(fireBuffer);
sf::Sound hitSound(hitBuffer);

void init()
{
	// load textures
	skyTexture.loadFromFile("Assets/graphics/sky.png");
	bgTexture.loadFromFile("Assets/graphics/bg.png");

	// load font
	headingFont.loadFromFile("Assets/fonts/SnackerComic.ttf");
	scoreFont.loadFromFile("Assets/fonts/arial.ttf");

	// set and attach a texture to sprite
	skySprite.setTexture(skyTexture);
	bgSprite.setTexture(bgTexture);

	// set heading text
	headingText.setFont(headingFont);
	headingText.setString("Tiny Bazooka");
	headingText.setCharacterSize(84);
	headingText.setFillColor(sf::Color::Red);
	sf::FloatRect headingBounds = headingText.getLocalBounds();
	headingText.setOrigin(headingBounds.width / 2, headingBounds.height / 2);
	headingText.setPosition(sf::Vector2f(viewSize.x * 0.5f, viewSize.y * 0.1f));

	// set score text
	scoreText.setFont(scoreFont);
	scoreText.setString("Score: 0");
	scoreText.setCharacterSize(45);
	scoreText.setFillColor(sf::Color::Red);
	sf::FloatRect scoreBounds = scoreText.getLocalBounds();
	scoreText.setOrigin(scoreBounds.width / 2, scoreBounds.height / 2);
	scoreText.setPosition(sf::Vector2f(viewSize.x * 0.5f, viewSize.y * 0.1f));

	// set tutorial text
	tutorialText.setFont(scoreFont);
	tutorialText.setString("Press Down Arrow to Fire and Start Game, Up arrow to Jump");
	tutorialText.setCharacterSize(35);
	tutorialText.setFillColor(sf::Color::Red);
	sf::FloatRect tutorialBounds = tutorialText.getLocalBounds();
	tutorialText.setOrigin(tutorialBounds.width / 2, tutorialBounds.height / 2);
	tutorialText.setPosition(sf::Vector2f(viewSize.x * 0.5f, viewSize.y * 0.2f));

	// Audio
	bgMusic.openFromFile("Assets/audio/bgMusic.ogg");
	bgMusic.play();

	fireBuffer.loadFromFile("Assets/audio/fire.ogg");
	hitBuffer.loadFromFile("Assets/audio/hit.ogg");

	// initialize hero
	hero.init("Assets/graphics/heroAnim.png", 4, 1.0f, sf::Vector2f(viewSize.x * 0.25f, viewSize.y * 0.5f), 200);
	srand((int)time(0));
}

void draw()
{
	window.draw(skySprite);
	window.draw(bgSprite);
	window.draw(hero.getSprite());
	for (Enemy* enemy : enemies)
	{
		window.draw(enemy->getSprite());
	}
	for (Rocket* rocket : rockets)
	{
		window.draw(rocket->getSprite());
	}
	if (gameOver)
	{
		window.draw(headingText);
		window.draw(tutorialText);
	}
	else
	{
		window.draw(scoreText);
	}
}

void updateInput()
{
	sf::Event event;

	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Up)
			{
				hero.jump(750.0f);
			}
			if (event.key.code == sf::Keyboard::Down)
			{
				if (gameOver)
				{
					gameOver = false;
					reset();
				}
				else
				{
					shoot();
				}
			}
		}
		if (event.key.code == sf::Keyboard::Escape || event.type == sf::Event::Closed)
		{
			window.close();
		}
	}
}

void update(float dt)
{
	hero.update(dt);

	currentTime += dt;

	// spawn enemies
	if (currentTime >= previousTime + 1.125f)
	{
		spawnEnemy();
		previousTime = currentTime;
	}

	// Update enemies
	for (int i = 0; i < enemies.size(); i++)
	{
		Enemy* enemy = enemies[i];
		enemy->update(dt);
		if (enemy->getSprite().getPosition().x < 0)
		{
			enemies.erase(enemies.begin() + i);
			delete(enemy);
			gameOver = true;
		}
	}

	// Update rockets
	for (int i = 0; i < rockets.size(); i++)
	{
		Rocket* rocket = rockets[i];
		rocket->update(dt);

		if (rocket->getSprite().getPosition().x > viewSize.x)
		{
			rockets.erase(rockets.begin() + i);
			delete(rocket);
		}
	}

	// check collision between rocket and enemies
	for (int i = 0; i < rockets.size(); i++)
	{
		for (int j = 0; j < enemies.size(); j++)
		{
			Rocket* rocket = rockets[i];
			Enemy* enemy = enemies[j];

			if (checkCollision(rocket->getSprite(), enemy->getSprite()))
			{
				hitSound.play();

				// set score
				score++;
				std::string finalScore = "Score: " + std::to_string(score);
				scoreText.setString(finalScore);
				sf::FloatRect scoreBounds = scoreText.getLocalBounds();
				scoreText.setOrigin(scoreBounds.width / 2, scoreBounds.height / 2);
				scoreText.setPosition(sf::Vector2f(viewSize.x * 0.5f, viewSize.y * 0.1f));

				// erase rockets and enemies
				rockets.erase(rockets.begin() + i);
				enemies.erase(enemies.begin() + j);

				delete(rocket);
				delete(enemy);

				printf(" rocket intersects enemy \n");
			}
		}
	}
}

int main()
{
	sf::Clock clock;
	window.setFramerateLimit(60);

	init();

	//initialize game objects
	while (window.isOpen())
	{
		// handle keyboard events
		updateInput();

		// update game objects in the scene
		sf::Time dt = clock.restart();
		if (!gameOver)
		{
			update(dt.asSeconds());
		}

		// render game objects
		window.clear(sf::Color::Red);
		draw();
		window.display();

	}

	return 0;
}

void spawnEnemy()
{
	int randomLocation = rand() % 3;
	sf::Vector2f enemyPosition;
	float speed;

	switch (randomLocation)
	{
	case 0:
		enemyPosition = sf::Vector2f(viewSize.x, viewSize.y * 0.75f);
		speed = -400;
		break;

	case 1:
		enemyPosition = sf::Vector2f(viewSize.x, viewSize.y * 0.60f);
		speed = -550;
		break;

	case 2:
		enemyPosition = sf::Vector2f(viewSize.x, viewSize.y * 0.40f);
		speed = -650;
		break;

	default:
		printf("incorrect y value \n");
		return;
	}

	Enemy* enemy = new Enemy();
	enemy->init("Assets/graphics/enemy.png", enemyPosition, speed);
	enemies.push_back(enemy);
}

void shoot()
{
	Rocket* rocket = new Rocket();
	rocket->init("Assets/graphics/rocket.png", hero.getSprite().getPosition(), 400.0f);
	rockets.push_back(rocket);
	fireSound.play();
}

bool checkCollision(sf::Sprite sprite1, sf::Sprite sprite2)
{
	sf::FloatRect shape1 = sprite1.getGlobalBounds();
	sf::FloatRect shape2 = sprite2.getGlobalBounds();

	if (shape1.intersects(shape2))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void reset()
{
	score = 0;
	currentTime = 0.0f;
	previousTime = 0.0f;

	for (Enemy* enemy : enemies)
	{
		delete(enemy);
	}

	for (Rocket* rocket : rockets)
	{
		delete(rocket);
	}

	enemies.clear();
	rockets.clear(); 
}