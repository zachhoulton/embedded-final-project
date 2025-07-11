# Starfighter

A 3D Aerial Combat Game for the MSPM0  
*Final Project for Embedded Systems (ECE319H)*

---

## Overview

**Starfighter** is a 3D space shooter inspired by *Star Wars*, built from scratch on the MSPM0 microcontroller platform.  
All rendering, controls, sound, and gameplay are implemented using low-level embedded C — no external game engine required.

---

## Hardware Used

| Component         | Function                         |
|-------------------|----------------------------------|
| MSPM0 LaunchPad   | Core game logic + peripherals    |
| ST7735 TFT Display| Real-time 3D rendering           |
| Joystick          | Pitch/yaw aiming control         |
| 3 Switches        | Fire, Boost, Recenter            |
| DAC Output        | Sound FX (shots, explosion)      |

---

## Controls

| Control           | Action                     |
|-------------------|----------------------------|
| **Joystick**      | Aim                        |
| **Left Button**   | Fire lasers                |
| **Middle Button** | Re-center aim on opponent  |
| **Right Button**  | Boost forward              |

---

## Screenshots




---

## Demo Video





---

## Features

- 3D starfield rendering
- Real-time sprite projection
- Directional aiming + boost movement
- Player selection screen (X-Wing or TIE Fighter)
- Dual-language menus (English / Español)
- Health bar + win/lose end screen
- Retro-style audio FX (DAC)
- **(Experimental)** Multiplayer using UART communication

---

## Multiplayer

The game includes a functional UART protocol for syncing position data between two MSPM0 boards.  
- Custom 8-byte float encoding
- Bidirectional sync using UART1 and UART2
- Multiplayer startup code is present but disabled

> Multiplayer logic is included in the codebase for demo/testing purposes, but is **not fully operational**.

## 👨‍💻 Authors

Zachary Houlton and Ariv Mondal 
Embedded Systems - Spring 2025  
The University of Texas at Austin
