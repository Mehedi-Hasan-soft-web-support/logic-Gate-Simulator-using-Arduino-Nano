# Arduino Logic Gate Simulator with OLED Display

This Arduino project simulates seven basic digital logic gates: AND, OR, NOT, NAND, NOR, XOR, and XNOR. Users can interact using push buttons to simulate inputs (A and B), navigate between gates, and visualize both the logic and outputs on a 128x64 SSD1306 OLED display.

## 🔧 Hardware Requirements
- Arduino Nano (use "ATmega328P (Old Bootloader)" for most clones)
- SSD1306 128x64 I2C OLED Display
- 5x Push Buttons
- 1x LED (for output visualization)
- 220Ω resistor (for LED)
- Jumper wires and breadboard

## 📌 Pin Connections

| Component       | Arduino Pin |
|----------------|-------------|
| Next Button     | D2          |
| Select Button   | D3          |
| Reset Button    | D4          |
| Input A Button  | D5          |
| Input B Button  | D6          |
| Output LED      | D7          |
| OLED SDA        | A4          |
| OLED SCL        | A5          |

> Note: OLED I2C address is `0x3C` (commonly used).

## 🚀 Features
- Navigate through gates with a "Next" button
- Select to activate a gate and test with input buttons
- Real-time output shown via OLED and LED
- Reset functionality to start fresh

## 💻 Upload Instructions
1. Select **Board:** Arduino Nano
2. Select **Processor:** ATmega328P (Old Bootloader)
3. Select the correct **Port**
4. Click **Upload**
5. Press **Reset** on the Nano during upload if needed

## 📷 Preview
![OLED Preview](images/oled_preview.jpg)

## 🧾 License
This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
