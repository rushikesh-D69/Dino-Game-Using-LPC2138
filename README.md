# Dino Jump Game on LPC2148 with Character LCD

This project implements a simple Dino Jump game using the LPC2148 ARM7 microcontroller and a 16x2 character LCD. The game mimics the classic side-scrolling dinosaur game, with jump controls, scoring, and obstacle collision logic.

## Features

- Real-time game loop
- Jumping dinosaur using custom LCD characters
- Moving cactus obstacles with randomized size
- Collision detection and game-over screen
- Score increment on successful jumps

## System Overview

- LCD used in 8-bit mode with custom character support
- Dino and cactus rendered using CGRAM patterns
- A button press triggers the jump sequence
- Game resets or halts on collision

## Hardware Requirements

- LPC2148 Development Board
- 16x2 Character LCD
- Push Button for jump
- Resistors, wires, and breadboard

## Software Requirements

- Keil µVision (or compatible ARM IDE)
- Proteus for simulation (optional)
- ARM7 GCC Toolchain (if using CLI)

## File Structure

- `src/` – Source code
- `project/` – Simulation/project files
- `assets/` – Images or schematics (optional)

## Getting Started

1. Open the `.pdsprj` file in Proteus or the project in Keil.
2. Wire the LCD and button to the LPC2148 as per schematic.
3. Compile and flash the code to the board.
4. Press the jump button to avoid cactus and score points.

## Authors

- Rushikesh D. – ECE Department, ASEB
- Chandan Sai Pavan – ECE Department, ASEB

## License

This project is licensed under the MIT License.

