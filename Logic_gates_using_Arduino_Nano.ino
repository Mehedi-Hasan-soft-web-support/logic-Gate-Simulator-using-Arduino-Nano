#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C  // I2C address of SSD1306 display (common values are 0x3C or 0x3D)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Button pins
const int nextButtonPin = 2;   // Navigate to next mode
const int selectButtonPin = 3; // Select current gate
const int resetButtonPin = 4;  // Reset selection
const int inputAPin = 5;       // Input A button
const int inputBPin = 6;       // Input B button

// Output pin
const int outputPin = 7;       // Output LED

// Variables
int currentGate = 0;           // 0=AND, 1=OR, 2=NOT, 3=NAND, 4=NOR, 5=XOR, 6=XNOR
bool selected = false;
bool inputAState = 0;
bool inputBState = 0;
bool outputState = 0;

// Debouncing variables
unsigned long lastInputAChangeTime = 0;
unsigned long lastInputBChangeTime = 0;
unsigned long lastNextButtonTime = 0;
unsigned long lastSelectButtonTime = 0;
unsigned long lastResetButtonTime = 0;
const unsigned long debounceDelay = 50;  // Milliseconds

// Logic gate names
const String gateNames[] = {"AND", "OR", "NOT", "NAND", "NOR", "XOR", "XNOR"};

void setup() {
  // Initialize serial for debugging
  Serial.begin(9600);
  
  // Button and LED pin modes
  pinMode(nextButtonPin, INPUT_PULLUP);
  pinMode(selectButtonPin, INPUT_PULLUP);
  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(inputAPin, INPUT_PULLUP);
  pinMode(inputBPin, INPUT_PULLUP);
  pinMode(outputPin, OUTPUT);
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) { 
    Serial.println(F("SSD1306 allocation failed"));
    while (true); // Infinite loop if OLED not found
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  // Show startup screen
  display.setTextSize(2);
  display.setCursor(10, 5);
  display.println("ICT with");
  display.setCursor(20, 25);
  display.println("Mehedi");
  display.setTextSize(1);
  display.setCursor(20, 50);
  display.println("@Arduinolog");
  display.display();
  
  delay(5000);  // Show splash screen for 2 seconds
  updateDisplay();
}

void loop() {
  // Handle navigation with debouncing
  if (!digitalRead(nextButtonPin)) {  // Button pressed (active 0)
    if (millis() - lastNextButtonTime > debounceDelay) {
      if (!selected) {
        currentGate = (currentGate + 1) % 7; // Cycle 0-6
        updateDisplay();
      }
    }
    lastNextButtonTime = millis();
  }
  
  if (!digitalRead(selectButtonPin)) {  // Button pressed (active 0)
    if (millis() - lastSelectButtonTime > debounceDelay) {
      selected = true;
      updateDisplay();
    }
    lastSelectButtonTime = millis();
  }
  
  if (!digitalRead(resetButtonPin)) {  // Button pressed (active 0)
    if (millis() - lastResetButtonTime > debounceDelay) {
      selected = false;
      inputAState = 0;
      inputBState = 0;
      updateDisplay();
    }
    lastResetButtonTime = millis();
  }
  
  // Read input button states with debouncing (active 0)
  bool currentInputA = !digitalRead(inputAPin);
  if (currentInputA != inputAState && millis() - lastInputAChangeTime > debounceDelay) {
    inputAState = currentInputA;
    lastInputAChangeTime = millis();
  }
  
  bool currentInputB = !digitalRead(inputBPin);
  if (currentInputB != inputBState && millis() - lastInputBChangeTime > debounceDelay) {
    inputBState = currentInputB;
    lastInputBChangeTime = millis();
  }
  
  // Compute output
  if (selected) {
    outputState = calculateOutput(currentGate, inputAState, inputBState);
    digitalWrite(outputPin, outputState);
    updateDisplay();  // Only update when there's a change
  }
  
  delay(10);  // Small loop delay for stability
}

// Calculate output based on selected logic gate
bool calculateOutput(int gate, bool a, bool b) {
  switch (gate) {
    case 0: return a && b;        // AND
    case 1: return a || b;        // OR
    case 2: return !a;            // NOT (only input A matters)
    case 3: return !(a && b);     // NAND
    case 4: return !(a || b);     // NOR
    case 5: return a ^ b;         // XOR
    case 6: return !(a ^ b);      // XNOR
    default: return false;
  }
}

// Draw logic gate symbol
void drawGateSymbol(int gate) {
  int x = 64;
  int y = 32;
  
  // Draw gate body
  display.drawRect(x, y-10, 20, 20, WHITE);
  
  // Draw inputs
  display.drawLine(x-15, y-5, x, y-5, WHITE);  // Input A
  if (gate != 2) {  // NOT gate only has one input
    display.drawLine(x-15, y+5, x, y+5, WHITE); // Input B
  }
  
  // Draw output
  display.drawLine(x+20, y, x+35, y, WHITE);
  
  // Draw NOT bubble for inverting gates
  if (gate == 2 || gate == 3 || gate == 4 || gate == 6) {
    display.drawCircle(x+23, y, 3, WHITE);
  }
}

// Refresh OLED display with current status
void updateDisplay() {
  display.clearDisplay();
  
  // Display gate info
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Gate: ");
  display.print(gateNames[currentGate]);
  
  if (selected) {
    display.print(" [ACTIVE]");
  } else {
    display.print(" [SELECT]");
  }
  
  // Draw visual representation
  drawGateSymbol(currentGate);
  
  // Display input states
  display.setCursor(30, 20);
  display.print("A: ");
  display.print(inputAState ? "1" : "0");
  
  if (currentGate != 2) {  // NOT gate only uses input A
    display.setCursor(30, 40);
    display.print("B: ");
    display.print(inputBState ? "1" : "0");
  }
  
  // Display output state
  display.setCursor(80, 40);
  display.print("Out: ");
  display.print(outputState ? "1" : "0");
  
  // Draw IO indicators
  display.fillCircle(10, 27, 5, inputAState ? WHITE : BLACK);
  display.drawCircle(10, 27, 5, WHITE);
  
  if (currentGate != 2) {  // NOT gate only uses input A
    display.fillCircle(10, 45, 5, inputBState ? WHITE : BLACK);
    display.drawCircle(10, 45, 5, WHITE);
  }
  
  display.fillCircle(120, 32, 5, outputState ? WHITE : BLACK);
  display.drawCircle(120, 32, 5, WHITE);
  
  // Menu
  display.setCursor(0, 56);
  display.print("NEXT | SELECT | RESET");
  
  display.display();
}