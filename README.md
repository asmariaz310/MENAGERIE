# MENAGERIE — A Tile-Matching Animal Puzzle Game

**MENAGERIE** is a colorful and engaging tile-matching puzzle game developed using **C++** and **SFML**, inspired by classics like *Bejeweled* and *Candy Crush*. 
Your goal? Match animal tiles, fill species repositories, and climb through exciting levels with strategic swaps and quick thinking.


## Game Objective

- Match 3 or more animals of the same species in a line to clear them from the board.
- Chains cause tiles to fall and refill the board.
- Track how many of each species you've collected.
- Progress to the next level by completing species-specific goals or maintaining a full progress bar under time pressure.


## Key Features

### Game Modes
- **Normal Mode:** Progress through levels by filling species-specific progress bars using chains. More points per chain as levels increase.
- **Time Trial Mode:** A more challenging mode where the progress bar depletes unless you continuously match tiles.

### Gameplay Mechanics
- **Grid-Based Matching:** Match only in horizontal or vertical lines. Diagonal matches are not allowed.
- **Tile Swapping:** Click on two adjacent animals to swap them.
- **Auto-Fill:** After clearing a chain, tiles fall to fill in the gaps, and new animals drop from above.

### Scoring
- Chains of 3 = base points
- Chains of 4 = bonus (x4)
- Chains of 5 = mega bonus (x5)
- Combo chains (formed in a single move) receive additional bonus points.

### Progress Tracking
- Real-time progress bars track each species collected.
- Scores and moves remaining are displayed.
- Time limit bar is visible during timed levels.


## Screenshots
- Main Menu Screen
  ![start](https://github.com/user-attachments/assets/2c5a5bbb-a4c8-4a94-8b13-5a3c1fef1aa7)

- Level 01 Gameplay Screen
  ![level1](https://github.com/user-attachments/assets/1773db89-b432-4cfa-9683-64e2d0b80f23)

- Level 02 Gameplay Screen
  ![level2](https://github.com/user-attachments/assets/e0a08042-9af9-4ad6-b26a-d7614330934b)

- Pause Screen
  ![pause](https://github.com/user-attachments/assets/7cca2b19-f7a5-44b6-adf8-f98adc7899e5)

- Level 2 Transition Screen
  ![level2](https://github.com/user-attachments/assets/8228df97-3f85-4cda-b146-ba7b553ed4de)

- Game Over Screen
  ![restart](https://github.com/user-attachments/assets/37f1c756-c211-49a4-8f70-095ef8e63bbd)

---

## How to Run the Game

### Requirements
- **C++ compiler** supporting C++17
- **SFML 2.5+** (Graphics, Window, Audio, and System modules)

### Build Instructions (Linux/Mac)
```bash
git clone https://github.com/your-username/MENAGERIE.git
cd MENAGERIE
g++ game.cpp -o sfml-app -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
./game
