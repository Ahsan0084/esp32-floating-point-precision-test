/*
 * Floating Point Precision Test v1 - PROBLEMATIC VERSION
 * =======================================================
 * 
 * This is an extracted snippet from a larger ESP32-based Cricket Scoreboard
 * system. This code demonstrates a critical floating-point precision issue
 * that caused the over counter to malfunction during development phase.
 * 
 
 * 
 * This snippet isolates the problematic over-counting logic for demonstration.
 * 
 * Hardware: ESP32 DevKit
 * Button: Connected to GPIO 7 (Active LOW with internal pull-up)
 */

#define BUTTON_PIN 7
#define DEBOUNCE_DELAY 50

// Cricket match variables
float overs = 0.0;
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
  Serial.println("Floating Point Precision Test v1");
  Serial.println("PROBLEMATIC VERSION - Demonstrating Bug");
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
        incrementOvers_Problematic();
        printCurrentValues();
      }
    }
  }
  
  lastButtonState7 = reading;
}

/*
 * PROBLEMATIC APPROACH - FLOATING-POINT FLAWS
 * 
 * Intended Logic:
 * - Balls 1-5: overs goes 0.1, 0.2, 0.3, 0.4, 0.5
 * - Ball 6: overs jumps from X.5 to (X+1).0
 * 
 * CONDITION EXPLAINED:
 * `overs < (int(overs) + 0.5) && overs != (int(overs) + 0.5)`
 * - The `< target` checks if we're still counting balls (not yet at X.5)
 * - The `!= target` is a SAFETY CHECK to ensure we don't accidentally
 *   add 0.1 when we're already exactly at the reset point (X.5)
 * - This defensive programming makes sense logically!
 * 
 * FLAW #1: FLOATING-POINT ACCUMULATION ERROR
 * Adding 0.1 repeatedly causes precision errors:
 * - 0.1 in binary is infinite: 0.0001100110011...
 * - After 5 additions: result is 0.4999995... instead of exactly 0.5
 * 
 * FLAW #2: THE CRITICAL BUG - VALUE UNDERSHOOTS!
 * Real serial output showed:
 *   Overs: 3.4999995232   <-- Should be 3.5 but undershoots!
 *   Overs: 3.50           <-- Display rounds it, hiding the bug
 * 
 * What happens with overs = 3.4999995232:
 *   target = int(3.4999995232) + 0.5 = 3 + 0.5 = 3.5
 *   
 *   Condition 1: overs < target  →  3.4999995232 < 3.5  →  TRUE!
 *   Condition 2: overs != target →  3.4999995232 != 3.5 →  TRUE!
 *   
 *   Result: FIRST BRANCH EXECUTES! overs += 0.1
 *   
 * The value NEVER reaches exactly 3.5, so it keeps adding 0.1!
 * overs goes: 3.49999... → 3.59999... → 3.69999... (never resets!)
 * 
 * The safety check (!=) can't help because overs is always slightly
 * LESS than the target due to accumulated precision loss.
 */
void incrementOvers_Problematic() {
  Serial.println("--- Button Pressed (PROBLEMATIC VERSION) ---");
  Serial.print("Before: overs = ");
  Serial.println(overs, 10);  // Print with 10 decimal places to see precision
  
  float target = int(overs) + 0.5;
  
  // THE FLAWED CONDITION
  if (overs < target && overs != target) {
    overs += 0.1;
    Serial.println("Action: Added 0.1 (ball increment)");
  } else if (overs == target) {
    overs += 0.5;
    Serial.println("Action: Added 0.5 (over complete, moving to next)");
  } 
  
  Serial.print("After: overs = ");
  Serial.println(overs, 10);
  
  // Debug: Show what the comparison values actually are
  Serial.println();
  Serial.println("Debug Comparison Analysis:");
  Serial.print("  target = int(overs) + 0.5 = ");
  Serial.println(target, 10);
  Serial.print("  overs < target ? ");
  Serial.println((overs < target) ? "TRUE" : "FALSE");
  Serial.print("  overs == target ? ");
  Serial.println((overs == target) ? "TRUE" : "FALSE");
  Serial.print("  Difference (overs - target): ");
  Serial.println(overs - target, 15);
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
  Serial.print("Overs (raw): ");
  Serial.println(overs, 10);
  Serial.print("Overs (display): ");
  Serial.println(overs, 2);
  Serial.println("========================================");
  Serial.println();
}

/*
 * SIMULATION MODE (for testing without hardware)
 * Uncomment the following function and call it from setup() 
 * to simulate multiple button presses and see the bug in action.
 * 
 * Try simulateButtonPresses(12) to see the bug occur around ball 6.
 */

/*
void simulateButtonPresses(int count) {
  Serial.println("=== SIMULATION START ===");
  Serial.println();
  
  for (int i = 0; i < count; i++) {
    Serial.print(">>> Press #");
    Serial.println(i + 1);
    incrementOvers_Problematic();
    printCurrentValues();
    delay(100);
  }
  
  Serial.println("=== SIMULATION END ===");
}
*/
