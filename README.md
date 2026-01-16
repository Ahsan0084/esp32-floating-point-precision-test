# ESP32 Floating-Point Precision Test: Cricket Over Counter

This repository demonstrates a **floating-point precision issue** encountered on ESP32 while developing a cricket scoreboard system. These code snippets are extracted from a larger project to isolate and demonstrate the problem and its solution.

---

## The Problem

Serial output showed (after debugging with extended precision using `.10f` format specifier):
```
23:12:59.848 -> Overs: 3.4999995232    <-- Should be 3.5
23:12:59.848 -> Overs: 3.50            <-- Display rounds it, hiding the bug
```

The over counter wouldn't reset at X.5 - it kept incrementing: 3.5 → 3.6 → 3.7 instead of rolling over to 4.0.

---

## v1: Problematic Approach

### The Code

```cpp
float overs = 0.0;

if (overs < (int(overs) + 0.5) && overs != (int(overs) + 0.5)) {
    overs += 0.1;
} else if (overs == (int(overs) + 0.5)) {
    overs += 0.5;
}
```

### Intended Logic
- Balls 1-5: overs increments by 0.1 (0.1, 0.2, 0.3, 0.4, 0.5)
- Ball 6: when overs reaches X.5, add 0.5 to roll over to next whole number

### Condition Explained
```cpp
overs < (int(overs) + 0.5) && overs != (int(overs) + 0.5)
```
- `overs < target` - Check if still counting balls (not yet at X.5)
- `overs != target` - **Safety check** to prevent adding 0.1 when already at reset point

This defensive programming makes sense logically.

### Flaw #1: Floating-Point Accumulation Error

Adding 0.1 repeatedly causes precision errors:
```
Decimal 0.1 in binary: 0.0001100110011001100110011... (infinite, repeating)
```

The ESP32's 32-bit FPU truncates this, introducing tiny errors. After 5 additions:
```
Expected: 0.5
Actual:   0.4999995... (undershoots!)
```

### Flaw #2: The Critical Bug - Value Undershoots

With `overs = 3.4999995232`:
```
target = int(3.4999995232) + 0.5 = 3 + 0.5 = 3.5

Condition 1: overs < target  →  3.4999995232 < 3.5  →  TRUE!
Condition 2: overs != target →  3.4999995232 != 3.5 →  TRUE!

Result: FIRST BRANCH EXECUTES! overs += 0.1
```

The value **never reaches exactly 3.5**, so it keeps adding 0.1:
```
3.4999995232 + 0.1 = 3.5999994...  (keeps going!)
3.5999994... + 0.1 = 3.6999993...  (never resets!)
```

The safety check (`!= target`) can't help because overs is always **slightly less** than target due to accumulated precision loss.

---

## v2: Fixed Approach

### The Code

```cpp
int over = 0;      // Complete overs (integer - EXACT)
int balls = 0;     // Balls in current over: 0-5 (integer - EXACT)
float overs = 0.0; // Display value ONLY (computed once, not accumulated)

if (balls < 5) {
    balls += 1;
} else if (balls == 5) {
    over += 1;
    balls = 0;
}

// Compute display value ONLY when needed
overs = over + (balls / 10.0f);
```

### Why This Works

**FIX #1: Clear, Simple Logic**
- `balls` counts 0, 1, 2, 3, 4, 5
- When balls reaches 5 (6th ball), reset to 0 and increment over
- No complex compound conditions

**FIX #2: No Floating-Point Accumulation**
- `balls` and `over` are integers - no precision loss ever
- Integer operations are ALWAYS exact on any processor

**FIX #3: No Unreachable States**
- `balls < 5` and `balls == 5` cover ALL possible states
- No "gap" where code falls through

**FIX #4: Display Value Computed Once**
- `overs = over + (balls / 10.0f)` computed only for display
- Single division operation has minimal precision impact
- Never used for comparisons, only for human-readable output

### Why This Works on ESP32
- ESP32's Xtensa LX6 handles integer operations perfectly
- Integer comparisons (`balls == 5`) are bit-for-bit exact
- Floating-point only used for final display conversion

---

## ESP32 Architecture Details

### Xtensa LX6 Processor

| Feature | Details |
|---------|---------|
| **Core** | Xtensa LX6 (dual or single core) |
| **FPU** | Single-precision (32-bit) hardware |

### 32-Bit Float Breakdown

| Component | Bits | Purpose |
|-----------|------|---------|
| Sign | 1 | +/- |
| Exponent | 8 | Scale |
| Mantissa | 23 | Precision (~7 decimal digits) |

With only **23 bits of mantissa**, you get roughly **7 significant decimal digits**. Errors become visible quickly with repeated operations.

---

## Serial Output Comparison

### v1 - Problematic

```
23:12:59.848 -> Overs: 3.4999995232    <-- Should be 3.5! Undershoots
23:12:59.848 -> Overs: 3.50            <-- Display rounds it, HIDING THE BUG
```

Since `3.4999995232 < 3.5`:
- `overs < 3.5` → TRUE (keeps adding 0.1!)
- `overs == 3.5` → FALSE (equality never triggers!)

### v2 - Fixed

```
Before: over=0, balls=5
Action: Over complete! Reset balls=0, incremented over
After: over=1, balls=0, overs(display)=1.00
```

Integer comparison `balls == 5` works perfectly every time.

---

## Key Takeaways

1. **Never use `==` with floats** - Use epsilon comparisons if you must compare floats
2. **Use integers for counting** - Convert to float only for display
3. **Understand your FPU** - ESP32 uses 32-bit single-precision 
4. **Check raw values** - Display rounding can hide precision bugs
5. **Architecture matters** - Sometimes bugs require understanding the hardware, not just the code

