# Snake Game on ATmega32 with 8x8 LED Matrix

This repository contains the complete firmware and simulation files for a classic Snake game, developed from scratch in C for an ATmega32 microcontroller. The project was created as a final assignment for a university Microprocessors course and demonstrates a wide range of fundamental embedded systems concepts.

The game is displayed on an 8x8 LED dot matrix, controlled by push buttons for movement, and features variable speed controlled by a potentiometer. The entire circuit was designed and validated using the Proteus Design Suite.

<img width="589" height="605" alt="image" src="https://github.com/user-attachments/assets/6aa6873e-b020-42ad-818a-1e27ebcde926" />

---

## Key Technical Features

*   **Low-Level C Programming:** The entire project is written in C for the AVR architecture, directly manipulating hardware registers for I/O, timers, and ADC control.
*   **Display Multiplexing:** A timer interrupt (`timer0_int`) is used to rapidly refresh the columns of the 8x8 LED matrix, creating a stable, persistent image from a single row of I/O pins. This is a core technique for driving matrix displays efficiently.
*   **Real-Time Control:** The system uses timer interrupts to ensure a consistent display refresh rate (~2ms) independent of the main game loop's speed.
*   **Analog Input for Speed Control:** A potentiometer is connected to the ATmega32's Analog-to-Digital Converter (ADC). The ADC value is read in `free running` mode and used to dynamically adjust the `delay_ms()` in the main game loop, allowing the player to change the snake's speed.
*   **Complete Game Logic:** All game mechanics were implemented from scratch, including snake movement, collision detection (walls and self), and the "eating" and "growing" mechanism.
*   **Circuit Simulation:** The project includes a `.pdsprj` file for the Proteus Design Suite, which contains the full circuit schematic and allows for complete simulation of the game's hardware and firmware interaction.

---

## Technologies & Tools

| Category                  | Technologies & Tools                                      |
| ------------------------- | --------------------------------------------------------- |
| **Microcontroller**         | ATmega32 (AVR Architecture)                               |
| **Language**                | C (AVR-GCC)                                               |
| **Hardware**                | 8x8 LED Dot Matrix, Push Buttons, Potentiometer           |
| **Core Concepts**           | GPIO, Timers, Interrupts, ADC, Display Multiplexing       |
| **Simulation & Design**     | Proteus Design Suite                                      |
| **Development Environment** | Atmel Studio (or equivalent AVR IDE)                      |

---
