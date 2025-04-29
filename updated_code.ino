#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C  // I2C address of SSD1306 display (common values are 0x3C or 0x3D)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Button pins
const int selectDownPin = 2;   // Move selector down (D2)
const int selectEnterPin = 3;  // Enter selected gate (D3)
const int inputAPin = 5;       // Input A button
const int inputBPin = 6;       // Input B button
const int backButtonPin = 4;   // Back to menu button

// Output pin
const int outputPin = 7;       // Output LED

// State variables
int menuPosition = 0;          // Current position in menu
bool inGateMode = false;       // Whether we're in a gate function
bool inputAState = 0;
bool inputBState = 0;
bool outputState = 0;

// Debouncing variables
unsigned long lastSelectDownTime = 0;
unsigned long lastSelectEnterTime = 0;
unsigned long lastInputAChangeTime = 0;
unsigned long lastInputBChangeTime = 0;
unsigned long lastBackButtonTime = 0;
const unsigned long debounceDelay = 50;  // Milliseconds

// Logic gate names and total count
const String gateNames[] = {"AND", "XOR", "OR", "NOT", "NOR", "NAND", "XNOR", "XOR"};
const int gateCount = 8;

void setup() {
  // Initialize serial for debugging
  Serial.begin(9600);
  
  // Button and LED pin modes
  pinMode(selectDownPin, INPUT_PULLUP);
  pinMode(selectEnterPin, INPUT_PULLUP);
  pinMode(backButtonPin, INPUT_PULLUP);
  pinMode(inputAPin, INPUT_PULLUP);
  pinMode(inputBPin, INPUT_PULLUP);
  pinMode(outputPin, OUTPUT);
  
  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) { 
    Serial.println(F("SSD1306 allocation failed"));
    while (true); // Infinite loop if OLED not found
  }
  
  // Show startup screen
  showStartupScreen();
  delay(3000);  // Show splash screen for 3 seconds
  
  // Show the menu
  showMenu();
}

void loop() {
  if (inGateMode) {
    // Handle gate mode operations
    handleGateMode();
  } else {
    // Handle menu navigation
    handleMenuNavigation();
  }
  
  delay(10);  // Small loop delay for stability
}

void showStartupScreen() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  display.setTextSize(2);
  display.setCursor(5, 5);
  display.println("ICT With");
  display.setCursor(20, 25);
  display.println("Sanjoy");
  display.setTextSize(1);
  display.setCursor(20, 50);
  display.println("@Arduinolog");
  display.display();
}

void showMenu() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  // Draw title
  display.setTextSize(1);
  display.setCursor(20, 0);
  display.println("Select GATE");
  display.drawLine(0, 10, SCREEN_WIDTH, 10, WHITE);
  
  // Draw menu items in 2 columns
  for (int i = 0; i < gateCount; i++) {
    int col = i % 2;  // 0 for left column, 1 for right column
    int row = i / 2;  // Row index (0-3)
    
    int x = col * 64;  // X position (0 or 64)
    int y = 15 + row * 10;  // Y position with spacing
    
    display.setCursor(x + 15, y);
    display.print(i+1);
    display.print(". ");
    display.print(gateNames[i]);
    
    // Draw selector indicator
    if (i == menuPosition) {
      display.fillTriangle(
        x + 5, y,
        x + 10, y + 3,
        x + 5, y + 6,
        WHITE
      );
    }
  }
  
  // Draw navigation hint
  display.drawLine(0, 54, SCREEN_WIDTH, 54, WHITE);
  display.setCursor(5, 56);
  display.print("<- Next : Select ->");
  
  display.display();
}

void handleMenuNavigation() {
  // Check for select down button press (D2)
  if (!digitalRead(selectDownPin)) {  // Button pressed (active low)
    if (millis() - lastSelectDownTime > debounceDelay) {
      menuPosition = (menuPosition + 1) % gateCount;  // Move down, wrap around to top
      showMenu();
    }
    lastSelectDownTime = millis();
  }
  
  // Check for select enter button press (D3)
  if (!digitalRead(selectEnterPin)) {  // Button pressed (active low)
    if (millis() - lastSelectEnterTime > debounceDelay) {
      inGateMode = true;
      inputAState = 0;
      inputBState = 0;
      showGateScreen();
    }
    lastSelectEnterTime = millis();
  }
}

void handleGateMode() {
  // Check for back button
  if (!digitalRead(backButtonPin)) {  // Button pressed (active low)
    if (millis() - lastBackButtonTime > debounceDelay) {
      inGateMode = false;
      showMenu();
    }
    lastBackButtonTime = millis();
  }
  
  // Read input button states with debouncing (active low)
  bool currentInputA = !digitalRead(inputAPin);
  if (currentInputA != inputAState && millis() - lastInputAChangeTime > debounceDelay) {
    inputAState = currentInputA;
    lastInputAChangeTime = millis();
    outputState = calculateOutput(menuPosition, inputAState, inputBState);
    digitalWrite(outputPin, outputState);
    showGateScreen();
  }
  
  bool currentInputB = !digitalRead(inputBPin);
  if (currentInputB != inputBState && millis() - lastInputBChangeTime > debounceDelay) {
    inputBState = currentInputB;
    lastInputBChangeTime = millis();
    outputState = calculateOutput(menuPosition, inputAState, inputBState);
    digitalWrite(outputPin, outputState);
    showGateScreen();
  }
}

void showGateScreen() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  // Display gate info centered at the top
  display.setTextSize(1);
  int textWidth = gateNames[menuPosition].length() * 6; // Approximate width
  int centerX = (SCREEN_WIDTH - textWidth) / 2;
  display.setCursor(centerX, 0);
  display.print(gateNames[menuPosition]);
  display.drawLine(0, 10, SCREEN_WIDTH, 10, WHITE); // Horizontal line
  
  // Draw visual representation
  drawGateSymbol(menuPosition);
  
  // Display input states
  display.setCursor(30, 20);
  display.print("A: ");
  display.print(inputAState ? "1" : "0");
  
  if (menuPosition != 3) {  // NOT gate only uses input A (index 3 is NOT in new array)
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
  
  if (menuPosition != 3) {  // NOT gate only uses input A (index 3 is NOT in new array)
    display.fillCircle(10, 45, 5, inputBState ? WHITE : BLACK);
    display.drawCircle(10, 45, 5, WHITE);
  }
  
  display.fillCircle(120, 32, 5, outputState ? WHITE : BLACK);
  display.drawCircle(120, 32, 5, WHITE);
  
  // Menu
  display.drawLine(0, 54, SCREEN_WIDTH, 54, WHITE);
  display.setCursor(5, 56);
  display.print("Back");
  
  display.display();
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

// Calculate output based on selected logic gate
bool calculateOutput(int gate, bool a, bool b) {
  switch (gate) {
    case 0: return a && b;        // AND
    case 1: return a ^ b;         // XOR (first occurrence)
    case 2: return a || b;        // OR
    case 3: return !a;            // NOT (only input A matters)
    case 4: return !(a || b);     // NOR
    case 5: return !(a && b);     // NAND
    case 6: return !(a ^ b);      // XNOR
    case 7: return a ^ b;         // XOR (second occurrence)
    default: return false;
  }
}
