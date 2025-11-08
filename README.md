# IC Tester using Arduino

A comprehensive automated Integrated Circuit (IC) tester built with Arduino that can test various 14-pin and 16-pin digital logic ICs. Perfect for electronics students, hobbyists, and technicians.

## Features

- **Dual IC Support**: Tests both 14-pin and 16-pin ICs automatically
- **Wide IC Compatibility**: Supports 25+ different IC types
- **User-Friendly Interface**: Simple START → TEST button operation
- **LCD Display**: Clear results and status messages
- **Safety Features**: Automatic pin reset and protection
- **Serial Monitoring**: Real-time debugging information

## Supported ICs

### 14-Pin ICs
- **7400** - Quad 2-input NAND Gate
- **7402** - Quad 2-input NOR Gate  
- **7404** - Hex NOT Gate
- **7408** - Quad 2-input AND Gate
- **7432** - Quad 2-input OR Gate
- **7486** - Quad 2-input XOR Gate
- **74266** - Quad 2-input XNOR Gate
- **7410** - Triple 3-input NAND Gate
- **7427** - Triple 3-input NOR Gate
- **7430** - 8-input NAND Gate
- **7473** - Dual JK Flip-Flop
- **7474** - Dual D Flip-Flop

### 16-Pin ICs
- **74151** - 8:1 Multiplexer
- **74153** - Dual 4:1 Multiplexer
- **74157** - Quad 2:1 Multiplexer
- **74138** - 3-to-8 Decoder
- **74139** - Dual 2-to-4 Decoder
- **74161** - 4-bit Binary Counter
- **7447** - BCD to 7-Segment Decoder
- **7476** - Dual JK Flip-Flop
- **7483** - 4-bit Binary Adder
- **74283** - 4-bit Binary Full Adder
- **7485** - 4-bit Magnitude Comparator
- **74175** - Quad D Flip-Flop

## Hardware Requirements

### Components
- Arduino Mega 2560 or Arduino Uno
- 16x2 I2C LCD Display
- Push Buttons (2x)
- IC Sockets (14-pin and 16-pin)
- Jumper Wires
- Breadboard or PCB
- Power Supply

### Pin Configuration
```
IC_TYPE_PIN = 48       // Detects 14-pin vs 16-pin IC
TEST_BUTTON_PIN = 5    // Test button
POWER_PIN = 4          // Power control
CONTRAST_PIN = 6       // LCD contrast
START_BUTTON_PIN = 7   // Start button

// IC Testing Pins: 22-37
```

## Installation

1. **Hardware Setup**
   - Connect LCD display via I2C
   - Connect buttons to specified pins
   - Wire IC sockets according to pin mapping
   - Connect power and ground properly

2. **Software Setup**
   - Install LiquidCrystal_I2C library in Arduino IDE
   - Open the provided .ino file
   - Select correct board and port
   - Upload to Arduino

## How to Use

1. **Power On**: System displays "IC Tester Ready"
2. **Insert IC**: Place IC in correct socket
3. **Start Test**: Press START button
4. **Run Test**: Press TEST button
5. **View Results**: 
   - Good IC: Shows IC number and "IC IS GOOD"
   - Faulty IC: Shows "IC NOT FOUND or IC FAULTY"
6. **Continue**: Press any button to test another IC

## Project Structure

The main code includes:

- **Setup Function**: Initializes LCD, pins, and serial communication
- **Loop Function**: Handles button presses and test sequencing
- **Test Functions**: Individual functions for each IC type
- **Helper Functions**: Basic gate testing and utility functions

## Code Overview

### Main Functions
```cpp
void setup() - Initializes system
void loop() - Main program loop
void startTest() - Starts IC testing process
bool test14PinICs() - Tests all 14-pin ICs
bool test16PinICs() - Tests all 16-pin ICs
```

### Testing Functions
Each IC has its own test function (testIC7400(), testIC7408(), etc.) that:
- Applies power to the IC
- Tests all gates/functions
- Verifies outputs against expected results
- Returns true if IC passes all tests

## Safety Notes

- Always power off when inserting/removing ICs
- Double-check IC orientation before testing
- Ensure proper voltage levels (5V for TTL ICs)
- Use anti-static precautions

## Troubleshooting

- **IC not detected**: Check IC orientation and socket connections
- **LCD not working**: Verify I2C address and connections
- **Buttons not responding**: Check button wiring and pull-up resistors
- **False failures**: Ensure clean power supply and stable connections

## Applications

- Electronics education and labs
- IC verification before circuit assembly
- Troubleshooting digital circuits
- Quality control in electronics workshops

## Customization

You can easily add support for new ICs by:

1. Adding the IC test function
2. Adding the function call in test14PinICs() or test16PinICs()
3. Defining the pin mapping for the new IC



---

**Happy Testing!**

For questions or issues, check the code comments or create an issue in the repository.