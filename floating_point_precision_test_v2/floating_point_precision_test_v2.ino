/*
 * Floating Point Precision Test v2 - FIXED VERSION
 * =================================================
 * 
 * This is an extracted snippet from a larger ESP32-based Cricket Scoreboard
 * system. This code demonstrates the SOLUTION to the floating-point precision
 * issue that plagued v1.
 * 
 * 
 * 
 * KEY INSIGHT: The fix required understanding that the ESP32's Xtensa LX6
 * processor uses a 32-bit single-precision FPU, making floating-point
 * comparisons inherently unreliable for equality checks.
 * 
 * Hardware: ESP32 DevKit
 * Button: Connected to GPIO 7 (Active LOW with internal pull-up)
 */

#define BUTTON_PIN 7
#define DEBOUNCE_DELAY 50

// Cricket match variables - FIXED: Using integers for counting
int over = 0;      // Complete overs (integer - EXACT)
int balls = 0;     // Balls in current over: 0-5 (integer - EXACT)
float overs = 0.0; // Display value ONLY (computed once, not accumulated)

int score = 0;
int wickets = 0;
int fours = 0;
int sixes = 0;

// Button debounce variables
int buttonState7 = HIGH;
int lastButtonState7 = HIGH;
unsigned long lastDebounceTime7 = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  Serial.println("========================================");
  Serial.println("Floating Point Precision Test v2");
  Serial.println("FIXED VERSION - Integer Arithmetic");
  Serial.println("========================================");
  Serial.println();
  Serial.println("Press button to increment balls/overs");
  Serial.println();
}

void loop() {
  // Read and debounce button
  int reading = digitalRead(BUTTON_PIN);
  
  if (reading != lastButtonState7) {
    lastDebounceTime7 = millis();
  }
  
  if ((millis() - lastDebounceTime7) > DEBOUNCE_DELAY) {
    if (reading != buttonState7) {
      buttonState7 = reading;
      
      // Button pressed (active LOW)
      if (buttonState7 == LOW) {
        incrementOvers_Fixed();
        printCurrentValues();
      }
    }
  }
  
  lastButtonState7 = reading;
}

/*
 * FIXED APPROACH - INTEGER ARITHMETIC
 * 
 * The solution addresses ALL flaws from v1:
 * 
 * FIX #1: CLEAR, SIMPLE LOGIC
 * - balls counts 0, 1, 2, 3, 4, 5
 * - When balls reaches 5 (6th ball), reset to 0 and increment over
 * - No complex compound conditions
 * 
 * FIX #2: NO FLOATING-POINT ACCUMULATION
 * - balls and over are integers - no precision loss ever
 * - Integer operations are ALWAYS exact on any processor
 * 
 * FIX #3: NO UNREACHABLE STATES
 * - balls < 5 and balls == 5 cover ALL possible states
 * - No "gap" where the code falls through
 * 
 * FIX #4: DISPLAY VALUE COMPUTED ONCE
 * - overs = over + (balls / 10.0f) is computed only for display
 * - Single division operation has minimal precision impact
 * - Never used for comparisons, only for human-readable output
 * 
 * WHY THIS WORKS ON ESP32:
 * - ESP32's Xtensa LX6 handles integer operations perfectly
 * - Integer comparisons (balls == 5) are bit-for-bit exact
 * - We only touch floating-point for final display conversion
 */
void incrementOvers_Fixed() {
  Serial.println("--- Button Pressed (FIXED VERSION) ---");
  Serial.print("Before: over=");
  Serial.print(over);
  Serial.print(", balls=");
  Serial.println(balls);
  
  // CLEAN INTEGER LOGIC - No floating-point comparisons!
  if (balls < 5) {
    balls += 1;
    Serial.println("Action: Incremented balls (integer operation)");
  } else if (balls == 5) {
    // 6th ball bowled - over complete
    over += 1;
    balls = 0;
    Serial.println("Action: Over complete! Reset balls=0, incremented over");
  }
  // Note: No else needed - balls is always 0-5, fully covered
  
  // Compute display value ONLY when needed for output
  // This is a single operation, not accumulated error
  overs = over + (balls / 10.0f);
  
  Serial.print("After: over=");
  Serial.print(over);
  Serial.print(", balls=");
  Serial.print(balls);
  Serial.print(", overs(display)=");
  Serial.println(overs, 2);
  Serial.println();
}

void printCurrentValues() {
  Serial.println("Current Values:");
  Serial.println("-----------------------------");
  Serial.print("Score: ");
  Serial.println(score);
  Serial.print("Wickets: ");
  Serial.println(wickets);
  Serial.print("Fours: ");
  Serial.println(fours);
  Serial.print("Sixes: ");
  Serial.println(sixes);
  Serial.print("Overs (computed float): ");
  Serial.println(overs, 10);
  Serial.print("Overs (display): ");
  Serial.println(overs, 2);
  Serial.print("Over (int): ");
  Serial.println(over);
  Serial.print("Balls (int): ");
  Serial.println(balls);
  Serial.println("========================================");
  Serial.println();
}

/*
 * SIMULATION MODE (for testing without hardware)
 * Uncomment the following function and call it from setup() 
 * to simulate multiple button presses and verify the fix works perfectly.
 * 
 * Try simulateButtonPresses(20) to see multiple overs complete correctly.
 */

/*
void simulateButtonPresses(int count) {
  Serial.println("=== SIMULATION START ===");
  Serial.println();
  
  for (int i = 0; i < count; i++) {
    Serial.print(">>> Press #");
    Serial.println(i + 1);
    incrementOvers_Fixed();
    printCurrentValues();
    delay(100);
  }
  
  Serial.println("=== SIMULATION END ===");
}
*/
