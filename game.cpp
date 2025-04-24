#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <time.h>
#include <iostream>

using namespace sf;
using namespace std;

// Sound effects and music
sf::SoundBuffer matchBuffer;
sf::Sound match;

sf::SoundBuffer clickBuffer;
sf::Sound click;

sf::SoundBuffer gameoverBuffer;
sf::Sound gameover;

sf::Music backgroundMusic;

// Game constants
const int TILE_SIZE = 54;
Vector2i boardOffset(48, 24);

// Game state variables
int totalScore = 0;
int comboCount = 0;
int maxCombo = 0;
bool hasGameStarted = false;
bool gameOverSoundPlayed = false;
bool clockStarted = false;
int previousGameState = 0; // 1 for level1, 2 for level2

// Tile structure representing each game tile
struct Tile {
    int x, y;           // Screen position
    int col, row;       // Grid position
    int species;        // Type of animal
    int matched;        // Whether tile is matched
    int alpha;          // Transparency for animations

    Tile() {
        matched = 0;
        alpha = 255;
    }
} grid[10][10]; // 10x10 grid (with borders)

/**
 * Swaps two tiles on the game board
 */
void swapTiles(Tile tile1, Tile tile2) {
    swap(tile1.col, tile2.col);
    swap(tile1.row, tile2.row);

    grid[tile1.row][tile1.col] = tile1;
    grid[tile2.row][tile2.col] = tile2;
}

/**
 * Handles tile gravity and combo calculations
 */
void applyGravity(bool isMoving, int currentMatchPoints, bool isSwap) {
    if (!isMoving) {
        // Make matched tiles fall down
        for (int row = 8; row > 0; row--) {
            for (int col = 1; col <= 8; col++) {
                if (grid[row][col].matched) {
                    int targetRow = row;
                    while (targetRow > 0) {
                        if (!grid[targetRow][col].matched) {
                            swapTiles(grid[targetRow][col], grid[row][col]);
                            break;
                        }
                        targetRow--;
                    }
                }
            }
        }

        // Combo handling
        if (currentMatchPoints > 0) {
            comboCount++;
            maxCombo = max(maxCombo, comboCount);
        } else if (!isMoving && !isSwap) {
            if (comboCount > 1)
                cout << "Combo x" << comboCount << "!\n";
            comboCount = 0;
        }
    }
}

/**
 * Clears initial matches when game starts
 */
void clearInitialMatches() {
    for (int row = 1; row <= 8; row++)
        for (int col = 1; col <= 8; col++) {
            int& sp = grid[row][col].species;
            while ((sp == grid[row - 1][col].species && sp == grid[row - 2][col].species) ||
                   (sp == grid[row][col - 1].species && sp == grid[row][col - 2].species)) {
                sp = rand() % 7;
            }
        }
}

int main() {
    // Game state management
    int gameState = 0; // 0=start, 1=level1, 2=level2, 3=pause, 4=gameover, 5=reset, 6=level2intro
    int levelTime = 30;
    srand(time(0));

    // Window setup
    RenderWindow window(VideoMode(790, 475), "MENAGERIE");
    window.setFramerateLimit(60);

    // Font setup
    Font gameFont;
    gameFont.loadFromFile("fonts/hello.ttf");

    // UI Text elements
    Text moveText, moveLabel;
    moveText.setFont(gameFont);
    moveText.setCharacterSize(30);
    moveText.setFillColor(Color::Yellow);
    moveText.setPosition(600, 160);
    moveText.setStyle(Text::Bold);

    moveLabel.setFont(gameFont);
    moveLabel.setCharacterSize(30);
    moveLabel.setFillColor(Color::Yellow);
    moveLabel.setPosition(500, 160);
    moveLabel.setStyle(Text::Bold);

    // Texture loading
    Texture texBackground, texAnimals, texStart, texPause, textLevel2, texRestart;
    texBackground.loadFromFile("sprites/background.png");
    texAnimals.loadFromFile("sprites/animals.png");
    texStart.loadFromFile("sprites/start.png");
    texPause.loadFromFile("sprites/pause.png");
    texRestart.loadFromFile("sprites/restart.png");
    textLevel2.loadFromFile("sprites/level2.png");

    // Sprite setup
    Sprite background(texBackground), animalSprites(texAnimals), 
           startScreen(texStart), pauseScreen(texPause), 
           level2Screen(textLevel2), restartScreen(texRestart);

    // Sound setup
    if (!matchBuffer.loadFromFile("sounds/match.wav"))
        cout << "Failed to load match.wav\n";
    match.setBuffer(matchBuffer);

    if (!clickBuffer.loadFromFile("sounds/click.wav"))
        cout << "Failed to load click.wav\n";
    click.setBuffer(clickBuffer);
    
    if (!gameoverBuffer.loadFromFile("sounds/gameover.wav"))
        cout << "Failed to load gameover.wav\n";
    gameover.setBuffer(gameoverBuffer);
    
    if (!backgroundMusic.openFromFile("sounds/Rainbows.wav")) {
        cout << "Error loading background music!" << endl;
    } else {
        backgroundMusic.setLoop(true);     // Loop the music
        backgroundMusic.setVolume(50);     // Set volume (0-100)
        backgroundMusic.play();           // Start playing
    }

    // Initialize game grid
    for (int row = 1; row <= 8; row++) {
        for (int col = 1; col <= 8; col++) {
            grid[row][col].species = rand() % 7;
            grid[row][col].col = col;
            grid[row][col].row = row;
            grid[row][col].x = col * TILE_SIZE;
            grid[row][col].y = row * TILE_SIZE;
        }
    }
    clearInitialMatches();

    // Gameplay variables
    int selectedX0, selectedY0, selectedX, selectedY; // Tile selection positions
    int clickCount = 0;                               // Mouse click counter
    Vector2i mousePos;                                // Mouse position
    bool isSwapping = false, isMoving = false;        // Animation states
    int remainingMoves = 10;                          // Moves remaining
    
    // Time management
    sf::Clock clock;
    sf::Time pausedTime;         // When pause started
    sf::Time totalPausedTime;    // Accumulated paused duration
    bool isPaused = false;
    const float timeLimit = 30.0f;
    
    // Main game loop
    while (window.isOpen()) {
        window.clear();

        // =============================================
        // Game State: 0 - Start Screen
        // =============================================
        if (gameState == 0) {
            window.draw(startScreen);
            window.display();

            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed)
                    window.close();
                    
                // Pause handling
                if (event.type == Event::KeyPressed && event.key.code == Keyboard::P) {
                    if (gameState == 1 || gameState == 2) {
                        previousGameState = gameState;
                        gameState = 3; // Go to pause screen
                        pausedTime = clock.getElapsedTime();
                        isPaused = true;
                    }
                }
                
                // Start game options
                if (event.type == Event::KeyPressed) {
                    if (event.key.code == Keyboard::S) {
                        gameState = 1; // Start level 1
                        match.play();
                    }
                    if (event.key.code == Keyboard::E) {
                        gameState = 2; // Start level 2
                        clearInitialMatches();
                    }
                }
            }
        }

        // =============================================
        // Game State: 1 - Level 1 (Move-based)
        // =============================================
        else if (gameState == 1) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed)
                    window.close();
                    
                // Tile selection
                else if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                    if (!isSwapping && !isMoving)
                        clickCount++;
                    mousePos = Mouse::getPosition(window) - boardOffset;
                } 
                // Keyboard controls
                else if (event.type == Event::KeyPressed) {
                    if (event.key.code == Keyboard::P) gameState = 3; // Pause
                    if (event.key.code == Keyboard::X) {
                        gameState = 5; // Reset game
                        totalScore = 0;
                        hasGameStarted = false;
                        clearInitialMatches();
                    }
                }
            }

            // Update move counter display
            moveText.setString(to_string(remainingMoves));
            moveLabel.setString("Moves: ");
            
            // Check if moves are exhausted
            if (remainingMoves <= 0) {
                gameState = 6; // Transition to level 2 intro
                clock.restart();
                clockStarted = true;
                continue; 
            }

            // Handle tile selection and swapping
            if (clickCount == 1) {
                selectedX0 = mousePos.x / TILE_SIZE + 1;
                selectedY0 = mousePos.y / TILE_SIZE + 1;
            } 
            else if (clickCount == 2) {
                selectedX = mousePos.x / TILE_SIZE + 1;
                selectedY = mousePos.y / TILE_SIZE + 1;

                // Only allow adjacent swaps
                if (abs(selectedX - selectedX0) + abs(selectedY - selectedY0) == 1) {
                    swapTiles(grid[selectedY0][selectedX0], grid[selectedY][selectedX]);
                    isSwapping = true;
                    clickCount = 0;
                    remainingMoves--;
                    hasGameStarted = true;
                    click.play();
                } else {
                    clickCount = 1;
                }
            }

            // Check for matches
            for (int row = 1; row <= 8; row++) {
                for (int col = 1; col <= 8; col++) {
                    // Vertical matches
                    if (grid[row][col].species == grid[row + 1][col].species &&
                        grid[row][col].species == grid[row - 1][col].species) {
                        for (int offset = -1; offset <= 1; offset++) {
                            grid[row + offset][col].matched++;
                        }
                    }
                    
                    // Horizontal matches
                    if (grid[row][col].species == grid[row][col + 1].species &&
                        grid[row][col].species == grid[row][col - 1].species) {
                        for (int offset = -1; offset <= 1; offset++) {
                            grid[row][col + offset].matched++;
                        }
                    }
                }
            }

            // Tile movement animation
            isMoving = false;
            for (int row = 1; row <= 8; row++) {
                for (int col = 1; col <= 8; col++) {
                    Tile& tile = grid[row][col];
                    int dx, dy;
                    
                    // Smooth movement
                    for (int step = 0; step < 10; step++) {
                        dx = tile.x - tile.col * TILE_SIZE;
                        dy = tile.y - tile.row * TILE_SIZE;
                        if (dx) tile.x -= dx / abs(dx);
                        if (dy) tile.y -= dy / abs(dy);
                    }
                    if (dx || dy) isMoving = true;
                }
            }

            // Fade out matched tiles
            if (!isMoving) {
                for (int row = 1; row <= 8; row++) {
                    for (int col = 1; col <= 8; col++) {
                        if (grid[row][col].matched && grid[row][col].alpha > 10) {
                            grid[row][col].alpha -= 10;
                            isMoving = true;
                        }
                    }
                }
            }

            // Calculate current match points
            int currentMatchPoints = 0;
            for (int row = 1; row <= 8; row++) {
                for (int col = 1; col <= 8; col++) {
                    currentMatchPoints += grid[row][col].matched;
                }
            }

            // Play match sound if needed
            if (currentMatchPoints > 0) {
                if (match.getStatus() != sf::Sound::Playing) {
                    match.play();
                }
            }

            // Update score
            if (hasGameStarted) {
                totalScore += currentMatchPoints;
            }

            // Handle invalid swaps
            if (isSwapping && !isMoving) {
                if (!currentMatchPoints) {
                    swapTiles(grid[selectedY0][selectedX0], grid[selectedY][selectedX]);
                }
                isSwapping = false;
            }

            // Apply gravity to tiles
            applyGravity(isMoving, currentMatchPoints, isSwapping);

            // Replace matched tiles with new ones
            for (int col = 1; col <= 8; col++) {
                for (int row = 8, dropCount = 0; row > 0; row--) {
                    if (grid[row][col].matched) {
                        grid[row][col].species = rand() % 7;
                        grid[row][col].y = -TILE_SIZE * dropCount++;
                        grid[row][col].matched = 0;
                        grid[row][col].alpha = 255;
                    }
                }
            }

            // Draw game elements
            window.draw(background);

            // Draw tiles
            for (int row = 1; row <= 8; row++) {
                for (int col = 1; col <= 8; col++) {
                    Tile tile = grid[row][col];
                    animalSprites.setTextureRect(IntRect(tile.species * 49, 0, 49, 49));
                    animalSprites.setColor(Color(255, 255, 255, tile.alpha));
                    animalSprites.setPosition(tile.x, tile.y);
                    animalSprites.move(boardOffset.x - TILE_SIZE, boardOffset.y - TILE_SIZE);
                    window.draw(animalSprites);
                }
            }

            // Draw UI elements
            Text scoreText, scoreLabel;

            scoreText.setFont(gameFont);
            scoreText.setCharacterSize(27);
            scoreText.setString(to_string(totalScore));
            scoreText.setFillColor(Color::Yellow);
            scoreText.setPosition(600, 190);
            scoreText.setStyle(Text::Bold);

            scoreLabel.setFont(gameFont);
            scoreLabel.setCharacterSize(27);
            scoreLabel.setString("Score: ");
            scoreLabel.setFillColor(Color::Yellow);
            scoreLabel.setPosition(500, 190);
            scoreText.setStyle(Text::Bold);

            window.draw(moveText);
            window.draw(moveLabel);
            window.draw(scoreText);
            window.draw(scoreLabel);
            window.display();
        }

        // =============================================
        // Game State: 2 - Level 2 (Time-based)
        // =============================================
        else if (gameState == 2) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed)
                    window.close();

                // Tile selection
                else if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                    if (gameState == 2 && !isSwapping && !isMoving) {
                        mousePos = Mouse::getPosition(window) - boardOffset;

                        // Validate click is within bounds
                        int col = mousePos.x / TILE_SIZE;
                        int row = mousePos.y / TILE_SIZE;
                        if (col >= 0 && col < 8 && row >= 0 && row < 8) {
                            clickCount++;
                        }
                    }
                }
                // Keyboard controls
                else if (event.type == Event::KeyPressed) {
                    if (event.key.code == Keyboard::P) gameState = 3; // Pause
                    if (event.key.code == Keyboard::X) {
                        gameState = 5; // Reset game
                        totalScore = 0;
                        clockStarted = false;
                        hasGameStarted = false;
                        clearInitialMatches();
                        totalPausedTime = sf::Time::Zero;
                    }
                }
            }

            // Initialize clock if not started
            if (!clockStarted) {
                clock.restart();
                clockStarted = true;
                totalScore = 0;
                remainingMoves = 10;
            }

            // Calculate remaining time
            float elapsed = clock.getElapsedTime().asSeconds();
            int timeLeft = static_cast<int>(timeLimit - elapsed);

            // Check if time is up
            if (timeLeft <= 0) {
                gameState = 4; // Game over
                backgroundMusic.stop();
                continue;
            }
            
            // Handle tile selection and swapping (same as level 1)
            if (clickCount == 1) {
                selectedX0 = mousePos.x / TILE_SIZE + 1;
                selectedY0 = mousePos.y / TILE_SIZE + 1;
            } 
            else if (clickCount == 2) {
                selectedX = mousePos.x / TILE_SIZE + 1;
                selectedY = mousePos.y / TILE_SIZE + 1;

                if (abs(selectedX - selectedX0) + abs(selectedY - selectedY0) == 1) {
                    swapTiles(grid[selectedY0][selectedX0], grid[selectedY][selectedX]);
                    isSwapping = true;
                    clickCount = 0;
                    remainingMoves--;
                    hasGameStarted = true;
                    click.play();
                } else {
                    clickCount = 1;
                }
            }

            // Check for matches (same as level 1)
            for (int row = 1; row <= 8; row++) {
                for (int col = 1; col <= 8; col++) {
                    if (grid[row][col].species == grid[row + 1][col].species &&
                        grid[row][col].species == grid[row - 1][col].species) {
                        for (int offset = -1; offset <= 1; offset++) {
                            grid[row + offset][col].matched++;
                        }
                    }

                    if (grid[row][col].species == grid[row][col + 1].species &&
                        grid[row][col].species == grid[row][col - 1].species) {
                        for (int offset = -1; offset <= 1; offset++) {
                            grid[row][col + offset].matched++;
                        }
                    }
                }
            }

            // Tile movement animation (same as level 1)
            isMoving = false;
            for (int row = 1; row <= 8; row++) {
                for (int col = 1; col <= 8; col++) {
                    Tile& tile = grid[row][col];
                    int dx, dy;
                    for (int step = 0; step < 10; step++) {
                        dx = tile.x - tile.col * TILE_SIZE;
                        dy = tile.y - tile.row * TILE_SIZE;
                        if (dx) tile.x -= dx / abs(dx);
                        if (dy) tile.y -= dy / abs(dy);
                    }
                    if (dx || dy) isMoving = true;
                }
            }

            // Fade out matched tiles (same as level 1)
            if (!isMoving) {
                for (int row = 1; row <= 8; row++) {
                    for (int col = 1; col <= 8; col++) {
                        if (grid[row][col].matched && grid[row][col].alpha > 10) {
                            grid[row][col].alpha -= 10;
                            isMoving = true;
                        }
                    }
                }
            }

            // Calculate current match points (same as level 1)
            int currentMatchPoints = 0;
            for (int row = 1; row <= 8; row++) {
                for (int col = 1; col <= 8; col++) {
                    currentMatchPoints += grid[row][col].matched;
                }
            }

            // Play match sound if needed (same as level 1)
            if (currentMatchPoints > 0) {
                if (match.getStatus() != sf::Sound::Playing) {
                    match.play();
                }
            }

            // Update score (same as level 1)
            if (hasGameStarted) {
                totalScore += currentMatchPoints;
            }

            // Handle invalid swaps (same as level 1)
            if (isSwapping && !isMoving) {
                if (!currentMatchPoints) {
                    swapTiles(grid[selectedY0][selectedX0], grid[selectedY][selectedX]);
                }
                isSwapping = false;
            }

            // Apply gravity to tiles (same as level 1)
            applyGravity(isMoving, currentMatchPoints, isSwapping);

            // Replace matched tiles with new ones (same as level 1)
            for (int col = 1; col <= 8; col++) {
                for (int row = 8, dropCount = 0; row > 0; row--) {
                    if (grid[row][col].matched) {
                        grid[row][col].species = rand() % 7;
                        grid[row][col].y = -TILE_SIZE * dropCount++;
                        grid[row][col].matched = 0;
                        grid[row][col].alpha = 255;
                    }
                }
            }

            // Draw game elements
            window.draw(background);

            // Draw tiles (same as level 1)
            for (int row = 1; row <= 8; row++) {
                for (int col = 1; col <= 8; col++) {
                    Tile tile = grid[row][col];
                    animalSprites.setTextureRect(IntRect(tile.species * 49, 0, 49, 49));
                    animalSprites.setColor(Color(255, 255, 255, tile.alpha));
                    animalSprites.setPosition(tile.x, tile.y);
                    animalSprites.move(boardOffset.x - TILE_SIZE, boardOffset.y - TILE_SIZE);
                    window.draw(animalSprites);
                }
            }
            
            // Draw UI elements specific to level 2
            Text timeText, scoreText, scoreLabel;

            // Score display
            scoreText.setFont(gameFont);
            scoreText.setCharacterSize(27);
            scoreText.setString(to_string(totalScore));
            scoreText.setFillColor(Color::Yellow);
            scoreText.setPosition(600, 190);
            scoreText.setStyle(Text::Bold);

            scoreLabel.setFont(gameFont);
            scoreLabel.setCharacterSize(27);
            scoreLabel.setString("Score: ");
            scoreLabel.setFillColor(Color::Yellow);
            scoreLabel.setPosition(500, 190);
            scoreLabel.setStyle(Text::Bold);

            window.draw(scoreText);
            window.draw(scoreLabel);

            // Time display
            timeText.setFont(gameFont);
            timeText.setCharacterSize(27);
            timeText.setFillColor(Color::Yellow);
            timeText.setPosition(500, 150);
            timeText.setStyle(Text::Bold);
            timeText.setString("Time Left: " + to_string(timeLeft) + "s");
            window.draw(timeText);

            window.display();
        }
        
        // =============================================
        // Game State: 3 - Pause Screen
        // =============================================
        else if (gameState == 3) {
            window.clear();
            window.draw(pauseScreen);
            backgroundMusic.pause();
            window.display();

            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed)
                    window.close();
                    
                // Resume game
                if (event.type == Event::KeyPressed && event.key.code == Keyboard::S) {
                    gameState = previousGameState; // Resume the correct level
                    totalPausedTime += clock.getElapsedTime() - pausedTime;
                    isPaused = false;
                    backgroundMusic.play();
                }
            }
        }

        // =============================================
        // Game State: 4 - Game Over Screen
        // =============================================
        else if (gameState == 4) {
            // Play game over sound once
            if (!gameOverSoundPlayed) {
                if (gameover.getStatus() != sf::Sound::Playing) {
                    gameover.play();
                }
                gameOverSoundPlayed = true;
            }
            
            window.draw(restartScreen);
            window.display();

            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed)
                    window.close();
                    
                // Reset game
                if (event.type == Event::KeyPressed && event.key.code == Keyboard::X) {
                    gameState = 5;
                    totalScore = 0;
                    clockStarted = false;
                    hasGameStarted = false;
                    clearInitialMatches();
                    gameOverSoundPlayed = false;
                }
            }
        }

        // =============================================
        // Game State: 5 - Reset Game
        // =============================================
        else if (gameState == 5) {
            remainingMoves = 10;
            totalScore = 0;
            window.clear();
            
            srand(time(0));

            // Reinitialize grid
            for (int row = 1; row <= 8; row++) {
                for (int col = 1; col <= 8; col++) {
                    grid[row][col].species = rand() % 3;
                    grid[row][col].col = col;
                    grid[row][col].row = row;
                    grid[row][col].x = col * TILE_SIZE;
                    grid[row][col].y = row * TILE_SIZE;
                }
            }

            gameState = 0; // Return to start screen
            backgroundMusic.play();
        }
        
        // =============================================
        // Game State: 6 - Level 2 Intro Screen
        // =============================================
        else if (gameState == 6) {
            window.draw(level2Screen);
            window.display();
            gameover.play();

            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed)
                    window.close();

                else if (event.type == Event::KeyPressed) {
                    // Start level 2
                    if (event.key.code == Keyboard::E) {
                        gameState = 2;
                        clockStarted = false;
                        remainingMoves = 10;
                        totalScore = 0;
                        clearInitialMatches();
                    } 
                    // Return to main menu
                    else if (event.key.code == Keyboard::X) {
                        gameState = 0;
                    }
                }
            }

            window.display();
        }
    }

    return 0;
}
