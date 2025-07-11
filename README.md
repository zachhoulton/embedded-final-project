# Starfighter

A 3D Aerial Combat Game for the MSPM0  
*Final Project for Embedded Systems (ECE319H)*

---

## Overview

**Starfighter** is a 3D space shooter inspired by *Star Wars*, built from scratch on the MSPM0 microcontroller platform.  
All rendering, controls, sound, and gameplay are implemented using low-level embedded C — no external game engine required.

---

## Demo Video

https://github.com/user-attachments/assets/56fe94dd-2f6c-4483-828e-f5687942c90d

---

## Screenshots

<img src="https://github.com/user-attachments/assets/3b6b05e1-2152-463c-9fbe-55ba1b2330a8" width="250"/>
<img src="https://github.com/user-attachments/assets/9dc55e49-64fc-403b-907c-cd5a90c6c808" width="250"/>
<img src="https://github.com/user-attachments/assets/056cf527-dd8e-4c2b-b302-01a785d71203" width="250"/>


---

## Hardware Used

| Component         | Function                         |
|-------------------|----------------------------------|
| MSPM0 LaunchPad   | Core game logic + peripherals    |
| ST7735 TFT        | Real-time 3D rendering           |
| Joystick          | Pitch/yaw aiming control         |
| 3 Switches        | Fire, Boost, Recenter            |
| DAC Audio Jack    | Sound FX (shots, explosion)      |

---

## Controls

| Control           | Action                     |
|-------------------|----------------------------|
| **Joystick**      | Aim                        |
| **Left Button**   | Fire lasers                |
| **Middle Button** | Re-center aim on opponent  |
| **Right Button**  | Boost forward              |

---

## Features

- 3D starfield rendering
- Real-time sprite projection
- Directional aiming + boost movement
- Player selection screen (X-Wing or TIE Fighter)
- Dual-language menus (English / Español)
- Health bar + win/lose end screen
- Sound effects(DAC)
- **(Experimental)** Multiplayer using UART communication

---

## Multiplayer

The game includes a functional UART protocol for syncing position data between two MSPM0 boards.  
- 8-byte float encoding for transmitting x, y, and z coordinates
- Framing and decoding logic
- A **handshake protocol** at startup to ensure both devices are ready before beginning the game

The multiplayer system is currently **commented out** due to performance constraints.

## Authors

Zachary Houlton and Ariv Mondal 
