#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

// Game States
enum GameState {
    MENU,
    MODE_SELECT,
    PLAYING,
    GAME_OVER,
    SETTINGS
};

// Game Modes
enum GameMode {
    PLAYER_VS_PLAYER,
    PLAYER_VS_AI
};

// Cell states
enum CellState {
    EMPTY,
    X_PLAYER,
    O_PLAYER
};

// Button class (unchanged)
class Button {
private:
    sf::RectangleShape shape;
    sf::Text text;
    sf::Font* font;
    sf::Color baseColor;
    sf::Color hoverColor;
    sf::Color clickColor;
    sf::VertexArray gradient;
    bool isPressed;
    bool wasPressed;
    float scale;
    float animationTime;

public:
// Constructor
    Button(float x, float y, float width, float height, const  string& buttonText, sf::Font* f) {
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(width, height));
        font = f;
        // Set colors
        baseColor = sf::Color(60, 60, 120);
        hoverColor = sf::Color(100, 100, 180);
        clickColor = sf::Color(80, 80, 160);
        // Set shape properties
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineThickness(2);
        shape.setOutlineColor(sf::Color(200, 200, 255, 200));
        // Create gradient
        gradient = sf::VertexArray(sf::Quads, 4);
        gradient[0].position = sf::Vector2f(x, y);
        gradient[1].position = sf::Vector2f(x + width, y);
        gradient[2].position = sf::Vector2f(x + width, y + height);
        gradient[3].position = sf::Vector2f(x, y + height);
        // Update gradient colors
        updateGradient(baseColor);
        // Set text properties
        text.setFont(*font);
        text.setString(buttonText);
        text.setCharacterSize(28);
        text.setFillColor(sf::Color(220, 220, 255));
        // Center text in button
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setPosition(
            x + (width - textBounds.width) / 2 - textBounds.left,
            y + (height - textBounds.height) / 2 - 5
        );
        // Initialize state variables
        isPressed = false;
        wasPressed = false;
        scale = 1.0f;
        animationTime = 0.0f;
    }
    // function to update the gradient colors based on the button state
    void updateGradient(sf::Color color) {
        gradient[0].color = color;
        gradient[1].color = sf::Color(color.r * 0.8f, color.g * 0.8f, color.b * 0.8f);
        gradient[2].color = sf::Color(color.r * 0.6f, color.g * 0.6f, color.b * 0.6f);
        gradient[3].color = sf::Color(color.r * 0.8f, color.g * 0.8f, color.b * 0.8f);
    }
    // function to update the button state based on mouse position and click
    void update(sf::Vector2i mousePos, bool mousePressed, float deltaTime) {
        // Check if mouse is over the button
        sf::FloatRect bounds = shape.getGlobalBounds();
        bool mouseOver = bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
        // Update shape color based on mouse state
        wasPressed = isPressed;
        isPressed = mouseOver && mousePressed;
        // Update shape fill color
        animationTime += deltaTime;
        
        float targetScale = mouseOver ? 1.05f : 1.0f;
        scale += (targetScale - scale) * deltaTime * 5.0f;
        shape.setScale(scale, scale);
        text.setScale(scale, scale);
        // Update gradient color based on button state
        sf::Color targetColor = isPressed ? clickColor : (mouseOver ? hoverColor : baseColor);
        updateGradient(targetColor);
    }
    // function to check if the button was clicked
    bool isClicked() {
        return wasPressed && !isPressed;
    }

    // function to draw the button on the window
    void draw(sf::RenderWindow& window) {
        window.draw(gradient);
        window.draw(shape);
        window.draw(text);
    }
};

// Particle class for visual effects
class Particle {
public:

    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float life;
    float maxLife;
    // Constructor to initialize particle properties    
    Particle() {
        position = sf::Vector2f(0, 0);
        velocity = sf::Vector2f(0, 0);
        color = sf::Color::White;
        life = 0;
        maxLife = 0;
    }
    // Constructor to initialize particle with specific properties
    Particle(sf::Vector2f pos, sf::Vector2f vel, sf::Color col, float lifetime) {
        position = pos;
        velocity = vel;
        color = col;
        life = lifetime;
        maxLife = lifetime;
    }
    //  function to update particle position and life
    void update(float deltaTime) {
        position += velocity * deltaTime;
        life -= deltaTime;
        float alpha = (life / maxLife) * 255;
        color.a = static_cast<sf::Uint8>(alpha);
    }
    //  function to check if the particle is still alive
    bool isAlive() {
        return life > 0;
    }
};
// Game statistics structure to keep track of wins, losses, and draws
struct GameStats {
    int playerWins;
    int aiWins;
    int draws;
    int totalGames;
    
    GameStats() : playerWins(0), aiWins(0), draws(0), totalGames(0) {}
};

// Tic-Tac-Toe Game Class
class TicTacToeGame {
private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Font titleFont;
    
    GameState currentState;
    GameMode currentMode;
    // Game board represented as a 2D array
    CellState board[3][3];
    int currentPlayer;
    int winner;
    bool gameEnded;
    // UI elements
    Button* menuButtons[4];
    Button* modeButtons[2];
    Button* gameOverButtons[2];
    // UI elements for grid lines, cells, and texts
    sf::RectangleShape gridLines[4];
    sf::RectangleShape cells[3][3];
    sf::Text cellTexts[3][3];
    sf::Text titleText;
    sf::Text statusText;
    sf::Text statsText;
    sf::VertexArray backgroundGradient;
    
    Particle particles[100];
    int particleCount;
    // Game statistics
    GameStats stats;
    
    float animationTime;
    sf::Color backgroundColor;
    
    int aiDifficulty;
    
public:
    // Constructor to initialize the game
    TicTacToeGame() : window(sf::VideoMode(800, 600), "Advanced Tic-Tac-Toe", sf::Style::Titlebar | sf::Style::Close) {
        window.setFramerateLimit(60);
        
        if (!font.loadFromFile("ARIAL.TTF")) {              
             cout << "Warning: Could not load arial.ttf, using default font" <<  endl;            
        }
        titleFont = font;
        // Set the title font to a larger size
        currentState = MENU;
        currentMode = PLAYER_VS_PLAYER;
        aiDifficulty = 2;
        // Initialize game state
        initializeGame();
        initializeUI();
        loadStats();
        
        particleCount = 0;
        animationTime = 0;
        backgroundColor = sf::Color(20, 20, 30);
        // Create a gradient for the background
        backgroundGradient = sf::VertexArray(sf::Quads, 4);
        backgroundGradient[0].position = sf::Vector2f(0, 0);
        backgroundGradient[1].position = sf::Vector2f(800, 0);
        backgroundGradient[2].position = sf::Vector2f(800, 600);
        backgroundGradient[3].position = sf::Vector2f(0, 600);
        updateBackgroundGradient();
        
        srand(static_cast<unsigned>(time(nullptr)));
    }
    // Destructor to clean up resources
    ~TicTacToeGame() {
        for (int i = 0; i < 4; i++) delete menuButtons[i];
        for (int i = 0; i < 2; i++) {
            delete modeButtons[i];
            delete gameOverButtons[i];
        }
        saveStats();
    }
    
    // Function to update the background gradient based on animation time
    void updateBackgroundGradient() {
        float t = sin(animationTime * 0.5f) * 0.5f + 0.5f;
        sf::Color topColor(20 + t * 10, 20 + t * 10, 30 + t * 20);
        sf::Color bottomColor(10 + t * 5, 10 + t * 5, 20 + t * 10);
        
        backgroundGradient[0].color = topColor;
        backgroundGradient[1].color = topColor;
        backgroundGradient[2].color = bottomColor;
        backgroundGradient[3].color = bottomColor;
    }
    //  Function to initialize the game state
    void initializeGame() {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                board[i][j] = EMPTY;
            }
        }
        currentPlayer = 1;
        winner = 0;
        gameEnded = false;
    }
    // Function to initialize the UI elements
    void initializeUI() {
        menuButtons[0] = new Button(300, 200, 200, 60, "Play Game", &font);
        menuButtons[1] = new Button(300, 280, 200, 60, "Statistics", &font);
        menuButtons[2] = new Button(300, 360, 200, 60, "Settings", &font);
        menuButtons[3] = new Button(300, 440, 200, 60, "Exit", &font);
        
        // Mode buttons stacked vertically
        modeButtons[0] = new Button(300, 250, 200, 60, "Player vs Player", &font);
        modeButtons[1] = new Button(300, 320, 200, 60, "Player vs AI", &font);
        
        // Game over buttons stacked vertically
        gameOverButtons[0] = new Button(450, 450, 200, 60, "Play Again", &font);
        gameOverButtons[1] = new Button(450, 520, 200, 60, "Main Menu", &font);
        
        titleText.setFont(titleFont);
        titleText.setString("TIC-TAC-TOE");
        titleText.setCharacterSize(60);
        titleText.setFillColor(sf::Color(200, 200, 255));
        titleText.setPosition(150, 80);
        titleText.setStyle(sf::Text::Bold);
        // Set the title text to pulse
        statusText.setFont(font);
        statusText.setCharacterSize(24);
        statusText.setFillColor(sf::Color::White);
        statusText.setPosition(50, 50);
        // Set the status text to pulse
        statsText.setFont(font);
        statsText.setCharacterSize(18);
        statsText.setFillColor(sf::Color(200, 200, 255));
        statsText.setPosition(50, 520);
        // Set the stats text to pulse
        setupGrid();
    }
    // Function to set up the grid lines and cells
    void setupGrid() {
        for (int i = 0; i < 4; i++) {
            gridLines[i].setFillColor(sf::Color(200, 200, 255, 200));
        }
        // Vertical lines
        gridLines[0].setPosition(350, 150);
        gridLines[0].setSize(sf::Vector2f(5, 300));
        gridLines[1].setPosition(450, 150);
        gridLines[1].setSize(sf::Vector2f(5, 300));
        // Horizontal lines
        gridLines[2].setPosition(250, 250);
        gridLines[2].setSize(sf::Vector2f(300, 5));
        gridLines[3].setPosition(250, 350);
        gridLines[3].setSize(sf::Vector2f(300, 5));
        // Initialize cells and texts
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                cells[i][j].setSize(sf::Vector2f(95, 95));
                cells[i][j].setPosition(255 + j * 100, 155 + i * 100);
                cells[i][j].setFillColor(sf::Color(30, 30, 50));
                cells[i][j].setOutlineThickness(2);
                cells[i][j].setOutlineColor(sf::Color(200, 200, 255, 200));
                
                cellTexts[i][j].setFont(font);
                cellTexts[i][j].setCharacterSize(48);
                cellTexts[i][j].setFillColor(sf::Color(200, 200, 255));
                cellTexts[i][j].setPosition(280 + j * 100, 170 + i * 100);
            }
        }
    }
    // Function to handle user input
    void handleInput() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            // Handle keyboard input for menu navigation
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                if (currentState == PLAYING && !gameEnded) {
                    handleGameClick(mousePos);
                }
            }
        }
    }
    //  Function to handle mouse clicks in the game
    void handleGameClick(sf::Vector2i mousePos) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (cells[i][j].getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y))) {
                    if (board[i][j] == EMPTY) {
                        makeMove(i, j);
                        return;
                    }
                }
            }
        }
    }
    // Function to make a move in the game
    void makeMove(int row, int col) {
        if (board[row][col] != EMPTY || gameEnded) return;
        
        board[row][col] = (currentPlayer == 1) ? X_PLAYER : O_PLAYER;
        // Create particles at the cell position
        createParticles(sf::Vector2f(255 + col * 100 + 47, 155 + row * 100 + 47));
        // Check for win or draw conditions
        if (checkWin()) {
            winner = currentPlayer;
            gameEnded = true;
            updateStats();
        } else if (checkDraw()) {
            winner = 0;
            gameEnded = true;
            updateStats();
        } else {
            currentPlayer = (currentPlayer == 1) ? 2 : 1;
            if (currentMode == PLAYER_VS_AI && currentPlayer == 2 && !gameEnded) {
                makeAIMove();
            }
        }
    }
    // Function to make an AI move based on difficulty level
    void makeAIMove() {
        int bestMove[2];
        
        if (aiDifficulty == 1) {
            makeRandomMove(bestMove);
        } else if (aiDifficulty == 2) {
            if (rand() % 2 == 0) {
                if (!makeStrategicMove(bestMove)) {
                    makeRandomMove(bestMove);
                }
            } else {
                makeRandomMove(bestMove);
            }
        } else {
            bestMove[0] = -1;
            bestMove[1] = -1;
            int bestScore = -1000;
            
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (board[i][j] == EMPTY) {
                        board[i][j] = O_PLAYER;
                        int score = minimax(false);
                        board[i][j] = EMPTY;
                        if (score > bestScore) {
                            bestScore = score;
                            bestMove[0] = i;
                            bestMove[1] = j;
                        }
                    }
                }
            }
        }
        // Make the best move if found
        if (bestMove[0] != -1 && bestMove[1] != -1) {
            makeMove(bestMove[0], bestMove[1]);
        }
    }
    // Function to make a random move for the AI
    void makeRandomMove(int move[2]) {
        int availableMoves[9][2];
        int count = 0;
        
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == EMPTY) {
                    availableMoves[count][0] = i;
                    availableMoves[count][1] = j;
                    count++;
                }
            }
        }
        
        if (count > 0) {
            int randomIndex = rand() % count;
            move[0] = availableMoves[randomIndex][0];
            move[1] = availableMoves[randomIndex][1];
        }
    }
    // Function to make a strategic move for the AI
    bool makeStrategicMove(int move[2]) {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == EMPTY) {
                    board[i][j] = O_PLAYER;
                    if (checkWin()) {
                        board[i][j] = EMPTY;
                        move[0] = i;
                        move[1] = j;
                        return true;
                    }
                    board[i][j] = EMPTY;
                }
            }
        }
        
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == EMPTY) {
                    board[i][j] = X_PLAYER;
                    if (checkWin()) {
                        board[i][j] = EMPTY;
                        move[0] = i;
                        move[1] = j;
                        return true;
                    }
                    board[i][j] = EMPTY;
                }
            }
        }
        
        return false;
    }
    // Minimax algorithm to evaluate the best move for the AI
    int minimax(bool isMaximizing) {
        if (checkWin()) {
            return isMaximizing ? -1 : 1;
        }
        if (checkDraw()) {
            return 0;
        }
        
        int bestScore = isMaximizing ? -1000 : 1000;
        
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == EMPTY) {
                    board[i][j] = isMaximizing ? O_PLAYER : X_PLAYER;
                    int score = minimax(!isMaximizing);
                    board[i][j] = EMPTY;
                    bestScore = isMaximizing ?  max(bestScore, score) :  min(bestScore, score);
                }
            }
        }
        
        return bestScore;
    }
    // Function to check for a win condition
    bool checkWin() {
        for (int i = 0; i < 3; i++) {
            if (board[i][0] != EMPTY && board[i][0] == board[i][1] && board[i][1] == board[i][2]) {
                return true;
            }
        }
        
        for (int j = 0; j < 3; j++) {
            if (board[0][j] != EMPTY && board[0][j] == board[1][j] && board[1][j] == board[2][j]) {
                return true;
            }
        }
        
        if (board[0][0] != EMPTY && board[0][0] == board[1][1] && board[1][1] == board[2][2]) {
            return true;
        }
        if (board[0][2] != EMPTY && board[0][2] == board[1][1] && board[1][1] == board[2][0]) {
            return true;
        }
        
        return false;
    }
    // Function to check for a draw condition
    bool checkDraw() {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == EMPTY) {
                    return false;
                }
            }
        }
        return true;
    }
    // Function to create particles for visual effects
    void createParticles(sf::Vector2f position) {
        for (int i = 0; i < 20; i++) {
            if (particleCount < 100) {
                float angle = static_cast<float>(rand() % 360) * 3.14159f / 180.0f;
                float speed = static_cast<float>(rand() % 100 + 50);
                sf::Vector2f velocity(cos(angle) * speed, sin(angle) * speed);
                sf::Color color = (currentPlayer == 1) ? sf::Color(255, 100, 100) : sf::Color(100, 100, 255);
                particles[particleCount] = Particle(position, velocity, color, 2.0f);
                particleCount++;
            }
        }
    }
    // Function to update game statistics
    void updateStats() {
        stats.totalGames++;
        if (winner == 1) {
            stats.playerWins++;
        } else if (winner == 2) {
            if (currentMode == PLAYER_VS_AI) {
                stats.aiWins++;
            } else {
                stats.playerWins++;
            }
        } else {
            stats.draws++;
        }
    }
    // Function to save game statistics to a file
    void saveStats() {
         ofstream file("game_stats.txt");
        if (file.is_open()) {
            file << stats.playerWins << " " << stats.aiWins << " " << stats.draws << " " << stats.totalGames;
            file.close();
        }
    }
    // Function to load game statistics from a file
    void loadStats() {
         ifstream file("game_stats.txt");
        if (file.is_open()) {
            file >> stats.playerWins >> stats.aiWins >> stats.draws >> stats.totalGames;
            file.close();
        }
    }
    // Function to update the game state
    void update(float deltaTime) {
        animationTime += deltaTime;
        updateBackgroundGradient();
        // Update particles
        for (int i = 0; i < particleCount; i++) {
            particles[i].update(deltaTime);
            if (!particles[i].isAlive()) {
                particles[i] = particles[particleCount - 1];
                particleCount--;
                i--;
            }
        }
        
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        
        if (currentState == MENU) {
            for (int i = 0; i < 4; i++) {
                menuButtons[i]->update(mousePos, mousePressed, deltaTime);
                if (menuButtons[i]->isClicked()) {
                    switch (i) {
                        case 0: currentState = MODE_SELECT; break;
                        case 1: currentState = SETTINGS; break;
                        case 2: aiDifficulty = (aiDifficulty % 3) + 1; break;
                        case 3: window.close(); break;
                    }
                }
            }
        } else if (currentState == MODE_SELECT) {
            for (int i = 0; i < 2; i++) {
                modeButtons[i]->update(mousePos, mousePressed, deltaTime);
                if (modeButtons[i]->isClicked()) {
                    currentMode = (i == 0) ? PLAYER_VS_PLAYER : PLAYER_VS_AI;
                    currentState = PLAYING;
                    initializeGame();
                }
            }
        } else if (currentState == PLAYING && gameEnded) {
            for (int i = 0; i < 2; i++) {
                gameOverButtons[i]->update(mousePos, mousePressed, deltaTime);
                if (gameOverButtons[i]->isClicked()) {
                    if (i == 0) {
                        initializeGame();
                    } else {
                        currentState = MENU;
                    }
                }
            }
        }
    }
    // Function to render the game on the window
    void render() {
        window.clear();
        window.draw(backgroundGradient);
        // Draw the grid lines
        if (currentState == MENU) {
            renderMenu();
        } else if (currentState == MODE_SELECT) {
            renderModeSelect();
        } else if (currentState == PLAYING) {
            renderGame();
        } else if (currentState == SETTINGS) {
            renderSettings();
        }
        
        for (int i = 0; i < particleCount; i++) {
            sf::CircleShape particle(3);
            particle.setPosition(particles[i].position);
            particle.setFillColor(particles[i].color);
            window.draw(particle);
        }
        
        window.display();
    }
    // Function to render the menu
    void renderMenu() {
        float scale = 1.0f + sin(animationTime * 2.0f) * 0.05f;
        titleText.setScale(scale, scale);
        titleText.setFillColor(sf::Color(200 + sin(animationTime) * 55, 200 + sin(animationTime * 0.7f) * 55,255));
        window.draw(titleText);
        
        sf::Text subtitle;
        subtitle.setFont(font);
        subtitle.setString("MASTER EDITION");
        subtitle.setCharacterSize(24);
        subtitle.setFillColor(sf::Color(150, 150, 255, 200));
        subtitle.setPosition(250, 140);
        window.draw(subtitle);
        
        for (int i = 0; i < 4; i++) {
            menuButtons[i]->draw(window);
        }
        // Draw the AI difficulty text
        sf::Text difficultyText;
        difficultyText.setFont(font);
        difficultyText.setCharacterSize(20);
        difficultyText.setFillColor(sf::Color(150, 150, 255));
        difficultyText.setPosition(50, 550);
         string difficulty = (aiDifficulty == 1) ? "Easy" : (aiDifficulty == 2) ? "Medium" : "Hard";
        difficultyText.setString("AI Difficulty: " + difficulty);
        window.draw(difficultyText);
    }
    // Function to render the mode selection screen
    void renderModeSelect() {
        sf::Text modeTitle;
        modeTitle.setFont(titleFont);
        modeTitle.setString("Select Game Mode");
        modeTitle.setCharacterSize(36);
        modeTitle.setFillColor(sf::Color(200, 200, 255));
        modeTitle.setPosition(250, 150);
        window.draw(modeTitle);
        
        // Draw mode buttons vertically
        modeButtons[0]->draw(window);
        modeButtons[1]->draw(window);
    }
    // Function to render the game
    void renderGame() {
        for (int i = 0; i < 4; i++) {
            window.draw(gridLines[i]);
        }
        // Draw the cells and texts
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                if (cells[i][j].getGlobalBounds().contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)) && 
                    board[i][j] == EMPTY && !gameEnded) {
                    cells[i][j].setFillColor(sf::Color(50, 50, 80));
                } else {
                    cells[i][j].setFillColor(sf::Color(30, 30, 50));
                }
                
                window.draw(cells[i][j]);
                
                if (board[i][j] == X_PLAYER) {
                    cellTexts[i][j].setString("X");
                    cellTexts[i][j].setFillColor(sf::Color(255, 100, 100));
                    window.draw(cellTexts[i][j]);
                } else if (board[i][j] == O_PLAYER) {
                    cellTexts[i][j].setString("O");
                    cellTexts[i][j].setFillColor(sf::Color(100, 100, 255));
                    window.draw(cellTexts[i][j]);
                }
            }
        }
        // Draw the title text
        if (gameEnded) {
            if (winner == 1) {
                statusText.setString("Player X Wins!");
                statusText.setFillColor(sf::Color(255, 100, 100));
            } else if (winner == 2) {
                if (currentMode == PLAYER_VS_AI) {
                    statusText.setString("AI Wins!");
                    statusText.setFillColor(sf::Color(100, 100, 255));
                } else {
                    statusText.setString("Player O Wins!");
                    statusText.setFillColor(sf::Color(100, 100, 255));
                }
            } else {
                statusText.setString("It's a Draw!");
                statusText.setFillColor(sf::Color(150, 150, 255));
            }
            
            // Draw game over buttons vertically
            gameOverButtons[0]->draw(window);
            gameOverButtons[1]->draw(window);
        } else {
            if (currentMode == PLAYER_VS_AI) {
                statusText.setString((currentPlayer == 1) ? "Your Turn (X)" : "AI Thinking...");
            } else {
                statusText.setString((currentPlayer == 1) ? "Player X Turn" : "Player O Turn");
            }
            statusText.setFillColor(sf::Color(200, 200, 255));
        }
        // Draw the status text
        window.draw(statusText);
        
         string statsString = "Games: " +  to_string(stats.totalGames) + 
                                 " | Wins: " +  to_string(stats.playerWins) + 
                                 " | AI Wins: " +  to_string(stats.aiWins) + 
                                 " | Draws: " +  to_string(stats.draws);
        statsText.setString(statsString);
        window.draw(statsText);
    }
    // Function to render the settings screen
    void renderSettings() {
        sf::Text settingsTitle;
        settingsTitle.setFont(titleFont);
        settingsTitle.setString("Statistics");
        settingsTitle.setCharacterSize(36);
        settingsTitle.setFillColor(sf::Color(200, 200, 255));
        settingsTitle.setPosition(300, 150);
        window.draw(settingsTitle);
        // Draw statistics
        sf::Text statsDisplay;
        statsDisplay.setFont(font);
        statsDisplay.setCharacterSize(24);
        statsDisplay.setFillColor(sf::Color(200, 200, 255));
        statsDisplay.setPosition(250, 250);
        
         string statsText = "Total Games: " +  to_string(stats.totalGames) + "\n" +
                               "Player Wins: " +  to_string(stats.playerWins) + "\n" +
                               "AI Wins: " +  to_string(stats.aiWins) + "\n" +
                               "Draws: " +  to_string(stats.draws);
        
        if (stats.totalGames > 0) {
            float winRate = (float)stats.playerWins / stats.totalGames * 100;
            statsText += "\nWin Rate: " +  to_string(static_cast<int>(winRate)) + "%";
        }
        
        statsDisplay.setString(statsText);
        window.draw(statsDisplay);
        // Draw back button text
        sf::Text backText;
        backText.setFont(font);
        backText.setString("Press ESC to return to menu");
        backText.setCharacterSize(18);
        backText.setFillColor(sf::Color(150, 150, 255));
        backText.setPosition(250, 450);
        window.draw(backText);
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            currentState = MENU;
        }
    }
    // Function to run the game loop
    void run() {
        sf::Clock clock;
        while (window.isOpen()) {
            float deltaTime = clock.restart().asSeconds();
            handleInput();
            update(deltaTime);
            render();
        }
    }
};

int main() {
    // Create and run the TicTacToe game
    TicTacToeGame game;
    game.run();
    return 0;
}