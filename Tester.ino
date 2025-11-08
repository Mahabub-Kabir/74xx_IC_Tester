#include<Wire.h>
#include<LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin configuration
#define IC_TYPE_PIN 48
#define TEST_BUTTON_PIN 5
#define POWER_PIN 4
#define CONTRAST_PIN 6
#define START_BUTTON_PIN 7

int delayms = 10;
bool testInProgress = false;
bool readyForTest = false;
bool resultDisplayed = false;
String foundIC = "";
bool icOK = false;

void setup() {
  Serial.begin(9600);
  
  // Setup LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();
  
  // Setup control pins
  pinMode(CONTRAST_PIN, OUTPUT);
  analogWrite(CONTRAST_PIN, 100);
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
  pinMode(TEST_BUTTON_PIN, INPUT_PULLUP);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(IC_TYPE_PIN, INPUT_PULLUP);
  
  // Initialize all IC pins as inputs (safe state)
  for(int i = 22; i <= 37; i++) {
    pinMode(i, INPUT);
    digitalWrite(i, LOW);
  }
  
  show("IC Tester Ready");
  lcd.setCursor(0, 1);
  lcd.print("Group-01");
  Serial.println("=== IC Tester Started ===");
  delay(5000);
  showReadyMessage();
}

void loop() {
  bool testButtonState = digitalRead(TEST_BUTTON_PIN);
  bool startButtonState = digitalRead(START_BUTTON_PIN);
  
  // If result is displayed, wait for any button to continue
  if (resultDisplayed) {
    if (testButtonState == LOW || startButtonState == LOW) {
      while(digitalRead(TEST_BUTTON_PIN) == LOW || digitalRead(START_BUTTON_PIN) == LOW) {
        delay(10);
      }
      resultDisplayed = false;
      showReadyMessage();
    }
    return;
  }
  
  // Start button pressed
  if(startButtonState == LOW && !testInProgress && !readyForTest) {
    readyForTest = true;
    show("Press TEST");
    Serial.println("Ready for test - press TEST button");
    delay(500);
  }
  
  // Test button pressed
  if(testButtonState == LOW && !testInProgress && readyForTest) {
    startTest();
  }
  
  delay(50);
}

void startTest() {
  testInProgress = true;
  readyForTest = false;
  foundIC = "";
  Serial.println("\n=== Starting IC Test ===");
  
  lcd.clear();
  show("Testing...");
  delay(500);
  
  // Reset all pins before testing
  resetAllPins();
  
  bool is16pin = digitalRead(IC_TYPE_PIN);
  Serial.print("IC Type: ");
  Serial.println(is16pin ? "16-pin" : "14-pin");
  
  bool found = false;
  
  if(!is16pin) {
    found = test14PinICs();
  } else {
    found = test16PinICs();
  }
  
  // Display result
  if(found && foundIC != "") {
    show(foundIC.c_str());
    Serial.println("=== " + foundIC + " ===");
  } else {
    lcd.setCursor(0, 0);
    show("IC NOT FOUND");
    lcd.setCursor(0, 1);
    lcd.print("or IC FAULTY");
    
    Serial.println("=== IC NOT FOUND ===");
  }
  
  resultDisplayed = true;
  testInProgress = false;
  Serial.println("=== Test Complete ===\n");
}

bool test14PinICs() {
  Serial.println("Testing 14-pin ICs...");
  
  // ADD YOUR 14-PIN IC TESTS HERE IN ORDER OF PRIORITY
  if(testIC7400()) return true;
  if(testIC7402()) return true;
  if(testIC7404()) return true;
  if(testIC7408()) return true;
  if(testIC7432()) return true;
  if(testIC7486()) return true;
  if(testIC74266()) return true; 
  if(testIC7410()) return true;
  if(testIC7427()) return true;
  if(testIC7430()) return true;
  if(testIC7473()) return true;
  if(testIC7474()) return true;  
  
  return false;
}

bool test16PinICs() {
  Serial.println("Testing 16-pin ICs...");
  
  // ADD YOUR 16-PIN IC TESTS HERE
  if(testIC74151()) return true;
  if(testIC74153()) return true;
  if(testIC74157()) return true;
  if(testIC74138()) return true;
  if(testIC74139()) return true;
  if(testIC74161()) return true;
  if(testIC7447()) return true;  
  if(testIC7476()) return true;  
  if(testIC7483()) return true;
  if(testIC74283()) return true;   
  if(testIC7485()) return true; 
  if(testIC74175()) return true;
  
  return false;
}

// ==================== HELPER FUNCTIONS ====================

void showReadyMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Insert IC & Press");
  lcd.setCursor(0, 1);
  lcd.print("START Button");
}

void show(const char* text) {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(text);
  Serial.println(text);
}

void resetAllPins() {
  for(int i = 22; i <= 37; i++) {
    pinMode(i, INPUT);
    digitalWrite(i, LOW);
  }
}

// Map IC pin to Arduino pin
int pin(int n) {
  bool is16pin = digitalRead(IC_TYPE_PIN);
  
  if(is16pin) {
    return n + 21; // 16-pin mapping
  } else {
    if(n > 7) {
      return n + 23; // Pins 8-14
    } else {
      return n + 21; // Pins 1-7
    }
  }
}

void powerIC(int vccPin, int gndPin) {
  resetAllPins();
  pinMode(pin(vccPin), OUTPUT);
  pinMode(pin(gndPin), OUTPUT);
  digitalWrite(pin(gndPin), LOW);
  digitalWrite(pin(vccPin), HIGH);
  delay(100);
}

// ==================== BASIC GATE TEST FUNCTIONS ====================

bool testNOT(int inp, int out) {
  pinMode(inp, OUTPUT);
  pinMode(out, INPUT);
  delay(delayms);
  
  digitalWrite(inp, LOW);
  delay(delayms);
  bool test1 = (digitalRead(out) == HIGH);
  
  digitalWrite(inp, HIGH);
  delay(delayms);
  bool test2 = (digitalRead(out) == LOW);
  
  return (test1 && test2);
}

bool testNAND(int in1, int in2, int out) {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(out, INPUT);
  delay(delayms);
  
  int passed = 0;
  
  digitalWrite(in1, LOW); digitalWrite(in2, LOW); delay(delayms);
  if(digitalRead(out) == HIGH) passed++;
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH); delay(delayms);
  if(digitalRead(out) == HIGH) passed++;
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW); delay(delayms);
  if(digitalRead(out) == HIGH) passed++;
  digitalWrite(in1, HIGH); digitalWrite(in2, HIGH); delay(delayms);
  if(digitalRead(out) == LOW) passed++;
  
  return (passed == 4);
}

bool testAND(int in1, int in2, int out) {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(out, INPUT);
  delay(delayms);
  
  int passed = 0;
  
  digitalWrite(in1, LOW); digitalWrite(in2, LOW); delay(delayms);
  if(digitalRead(out) == LOW) passed++;
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH); delay(delayms);
  if(digitalRead(out) == LOW) passed++;
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW); delay(delayms);
  if(digitalRead(out) == LOW) passed++;
  digitalWrite(in1, HIGH); digitalWrite(in2, HIGH); delay(delayms);
  if(digitalRead(out) == HIGH) passed++;
  
  return (passed == 4);
}

bool testOR(int in1, int in2, int out) {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(out, INPUT);
  delay(delayms);
  
  int passed = 0;
  
  digitalWrite(in1, LOW); digitalWrite(in2, LOW); delay(delayms);
  if(digitalRead(out) == LOW) passed++;
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH); delay(delayms);
  if(digitalRead(out) == HIGH) passed++;
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW); delay(delayms);
  if(digitalRead(out) == HIGH) passed++;
  digitalWrite(in1, HIGH); digitalWrite(in2, HIGH); delay(delayms);
  if(digitalRead(out) == HIGH) passed++;
  
  return (passed == 4);
}

bool testNOR(int in1, int in2, int out) {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(out, INPUT);
  delay(delayms);
  
  int passed = 0;
  
  digitalWrite(in1, LOW); digitalWrite(in2, LOW); delay(delayms);
  if(digitalRead(out) == HIGH) passed++;
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH); delay(delayms);
  if(digitalRead(out) == LOW) passed++;
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW); delay(delayms);
  if(digitalRead(out) == LOW) passed++;
  digitalWrite(in1, HIGH); digitalWrite(in2, HIGH); delay(delayms);
  if(digitalRead(out) == LOW) passed++;
  
  return (passed == 4);
}

bool testXOR(int in1, int in2, int out) {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(out, INPUT);
  delay(delayms);
  
  int passed = 0;
  
  digitalWrite(in1, LOW); digitalWrite(in2, LOW); delay(delayms);
  if(digitalRead(out) == LOW) passed++;
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH); delay(delayms);
  if(digitalRead(out) == HIGH) passed++;
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW); delay(delayms);
  if(digitalRead(out) == HIGH) passed++;
  digitalWrite(in1, HIGH); digitalWrite(in2, HIGH); delay(delayms);
  if(digitalRead(out) == LOW) passed++;
  
  return (passed == 4);
}

bool testXNOR(int in1, int in2, int out) {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(out, INPUT);
  delay(delayms);
  
  int passed = 0;
  
  digitalWrite(in1, LOW); digitalWrite(in2, LOW); delay(delayms);
  if(digitalRead(out) == HIGH) passed++;
  
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH); delay(delayms);
  if(digitalRead(out) == LOW) passed++;
  
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW); delay(delayms);
  if(digitalRead(out) == LOW) passed++;
  
  digitalWrite(in1, HIGH); digitalWrite(in2, HIGH); delay(delayms);
  if(digitalRead(out) == HIGH) passed++;
  
  Serial.print("XNOR Test passed: ");
  Serial.println(passed);
  
  return (passed == 4);
}

bool testJK_FF_Neg_With_Invert_Clear(int j,int k,int clk,int clr,int q,int q_){
  pinMode(j, OUTPUT);
  pinMode(k, OUTPUT);
  pinMode(clk, OUTPUT);
  pinMode(clr, OUTPUT);
  pinMode(q, INPUT);
  pinMode(q_, INPUT);
  
  bool passed = true;
  
  digitalWrite(clr, HIGH);
  digitalWrite(clk, HIGH);
  
  // Test JK=10 (Set)
  digitalWrite(j, HIGH); digitalWrite(k, LOW);
  digitalWrite(clk, LOW); delay(delayms); digitalWrite(clk, HIGH); delay(delayms);
  if(digitalRead(q) != HIGH || digitalRead(q_) != LOW) passed = false;
  
  // Test JK=01 (Reset)
  digitalWrite(j, LOW); digitalWrite(k, HIGH);
  digitalWrite(clk, LOW); delay(delayms); digitalWrite(clk, HIGH); delay(delayms);
  if(digitalRead(q) != LOW || digitalRead(q_) != HIGH) passed = false;
  
  // Test JK=11 (Toggle)
  bool initial = digitalRead(q);
  digitalWrite(j, HIGH); digitalWrite(k, HIGH);
  digitalWrite(clk, LOW); delay(delayms); digitalWrite(clk, HIGH); delay(delayms);
  if(digitalRead(q) == initial) passed = false;
  
  // Test clear
  digitalWrite(clr, LOW); delay(delayms);
  if(digitalRead(q) != LOW || digitalRead(q_) != HIGH) passed = false;
  
  return passed;
}

bool testThreeNOR(int inp1, int inp2, int inp3, int otp){
  pinMode(inp1, OUTPUT);
  pinMode(inp2, OUTPUT);
  pinMode(inp3, OUTPUT);
  pinMode(otp, INPUT);
  int i,j,k,count=0;
  for(i=0;i<2;i++){
    digitalWrite(inp1,(bool)i);
    for(j=0;j<2;j++){
      digitalWrite(inp2,(bool)j);
      for(k=0;k<2;k++){
         digitalWrite(inp3,(bool)k);
         delay(delayms);
         if(digitalRead(otp)!=((bool)i || (bool)j || (bool)k)){
            count++;
         }
      }
    }
  }
  return (count == 8);
}

// ==================== 14-PIN IC TEST FUNCTIONS ====================

bool testIC7400() {
  Serial.println("Testing 7400 (Quad NAND)...");
  powerIC(14, 7);
  
  bool g1 = testNAND(pin(1), pin(2), pin(3));
  bool g2 = testNAND(pin(4), pin(5), pin(6));
  bool g3 = testNAND(pin(9), pin(10), pin(8));
  bool g4 = testNAND(pin(12), pin(13), pin(11));
  
  resetAllPins();
  
  if(g1 && g2 && g3 && g4) {
    foundIC = "IC 7400";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}

bool testIC7402() {
  Serial.println("Testing 7402 (Quad NOR)...");
  powerIC(14, 7);
  
  bool g1 = testNOR(pin(2), pin(3), pin(1));
  bool g2 = testNOR(pin(5), pin(6), pin(4));
  bool g3 = testNOR(pin(8), pin(9), pin(10));
  bool g4 = testNOR(pin(11), pin(12), pin(13));
  
  resetAllPins();
  
  if(g1 && g2 && g3 && g4) {
    foundIC = "IC 7402";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}

bool testIC7404() {
  Serial.println("Testing 7404 (Hex NOT)...");
  powerIC(14, 7);
  
  bool g1 = testNOT(pin(1), pin(2));
  bool g2 = testNOT(pin(3), pin(4));
  bool g3 = testNOT(pin(5), pin(6));
  bool g4 = testNOT(pin(13), pin(12));
  bool g5 = testNOT(pin(11), pin(10));
  bool g6 = testNOT(pin(9), pin(8));
  
  resetAllPins();
  
  if(g1 && g2 && g3 && g4 && g5 && g6) {
    foundIC = "IC 7404";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}

bool testIC7408() {
  Serial.println("Testing 7408 (Quad AND)...");
  powerIC(14, 7);
  
  bool g1 = testAND(pin(1), pin(2), pin(3));
  bool g2 = testAND(pin(4), pin(5), pin(6));
  bool g3 = testAND(pin(9), pin(10), pin(8));
  bool g4 = testAND(pin(12), pin(13), pin(11));
  
  resetAllPins();
  
  if(g1 && g2 && g3 && g4) {
    foundIC = "IC 7408";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}

bool testIC7432() {
  Serial.println("Testing 7432 (Quad OR)...");
  powerIC(14, 7);
  
  bool g1 = testOR(pin(1), pin(2), pin(3));
  bool g2 = testOR(pin(4), pin(5), pin(6));
  bool g3 = testOR(pin(9), pin(10), pin(8));
  bool g4 = testOR(pin(12), pin(13), pin(11));
  
  resetAllPins();
  
  if(g1 && g2 && g3 && g4) {
    foundIC = "IC 7432";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}

bool testIC7486() {
  Serial.println("Testing 7486 (Quad XOR)...");
  powerIC(14, 7);
  
  bool g1 = testXOR(pin(1), pin(2), pin(3));
  bool g2 = testXOR(pin(4), pin(5), pin(6));
  bool g3 = testXOR(pin(9), pin(10), pin(8));
  bool g4 = testXOR(pin(12), pin(13), pin(11));
  
  resetAllPins();
  
  if(g1 && g2 && g3 && g4) {
    foundIC = "IC 7486";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}

bool testIC74266() {
  Serial.println("Testing 74266 (Quad XNOR)...");
  powerIC(14, 7);
  
  // Test all 4 gates with comprehensive patterns
  bool g1 = testXNOR(pin(1), pin(2), pin(3));
  bool g2 = testXNOR(pin(6), pin(5), pin(4));
  bool g3 = testXNOR(pin(8), pin(9), pin(10));
  bool g4 = testXNOR(pin(12), pin(13), pin(11));
  
  // Additional verification: Test specific patterns that distinguish XNOR from XOR
  bool additionalTest = true;
  
  // Test pattern that clearly distinguishes XNOR from XOR
  digitalWrite(pin(1), LOW); digitalWrite(pin(2), LOW); delay(delayms);
  if(digitalRead(pin(3)) != HIGH) additionalTest = false;  // XNOR: 0,0 → 1
  
  digitalWrite(pin(1), HIGH); digitalWrite(pin(2), HIGH); delay(delayms);
  if(digitalRead(pin(3)) != HIGH) additionalTest = false;  // XNOR: 1,1 → 1
  
  resetAllPins();
  
  // Only identify as 74266 if ALL tests pass
  if(g1 && g2 && g3 && g4 && additionalTest) {
    foundIC = "IC 74266";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}
bool testIC7410() {
  powerIC(14, 7);
  
  // Gate 1: Inputs 1,2,13 -> Output 12
  pinMode(pin(1), OUTPUT); pinMode(pin(2), OUTPUT); pinMode(pin(13), OUTPUT); pinMode(pin(12), INPUT);
  
  // Test Gate 1 with mixed patterns (not just all HIGH/LOW)
  digitalWrite(pin(1), HIGH); digitalWrite(pin(2), LOW); digitalWrite(pin(13), LOW); delay(delayms);
  bool g1_test1 = (digitalRead(pin(12)) == HIGH);  // H,L,L -> HIGH
  
  digitalWrite(pin(1), HIGH); digitalWrite(pin(2), HIGH); digitalWrite(pin(13), LOW); delay(delayms);
  bool g1_test2 = (digitalRead(pin(12)) == HIGH);  // H,H,L -> HIGH
  
  digitalWrite(pin(1), HIGH); digitalWrite(pin(2), HIGH); digitalWrite(pin(13), HIGH); delay(delayms);
  bool g1_test3 = (digitalRead(pin(12)) == LOW);   // H,H,H -> LOW
  
  bool g1 = g1_test1 && g1_test2 && g1_test3;
  
  // Gate 2: Inputs 3,4,5 -> Output 6
  pinMode(pin(3), OUTPUT); pinMode(pin(4), OUTPUT); pinMode(pin(5), OUTPUT); pinMode(pin(6), INPUT);
  
  digitalWrite(pin(3), LOW); digitalWrite(pin(4), HIGH); digitalWrite(pin(5), HIGH); delay(delayms);
  bool g2_test1 = (digitalRead(pin(6)) == HIGH);  // L,H,H -> HIGH
  
  digitalWrite(pin(3), HIGH); digitalWrite(pin(4), HIGH); digitalWrite(pin(5), HIGH); delay(delayms);
  bool g2_test2 = (digitalRead(pin(6)) == LOW);   // H,H,H -> LOW
  
  bool g2 = g2_test1 && g2_test2;
  
  resetAllPins();
  
  if(g1 && g2) {
    foundIC = "7410";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}


bool testIC7427() {
  Serial.println("Testing 7427 (Triple 3-Input NOR)...");
  powerIC(14, 7);
  
  bool g1 = testThreeNOR(pin(1), pin(2), pin(13), pin(12));
  bool g2 = testThreeNOR(pin(3), pin(4), pin(5), pin(6));
  bool g3 = testThreeNOR(pin(9), pin(10), pin(11), pin(8));
  
  resetAllPins();
  
  if(g1 && g2 && g3) {
    foundIC = "IC 7427";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}
bool testIC7430() {
  powerIC(14, 7);
  
  // Setup all 8 inputs and output
  for(int i = 1; i <= 7; i++) pinMode(pin(i), OUTPUT);
  pinMode(pin(11), OUTPUT); pinMode(pin(12), OUTPUT); pinMode(pin(8), INPUT);
  
  // Test with exactly 7 HIGH inputs (should output HIGH)
  for(int i = 1; i <= 7; i++) digitalWrite(pin(i), HIGH);
  digitalWrite(pin(11), HIGH); digitalWrite(pin(12), LOW); delay(delayms);
  bool test1 = (digitalRead(pin(8)) == HIGH);  // 7 HIGH, 1 LOW -> HIGH
  
  // Test with all HIGH inputs (should output LOW)
  for(int i = 1; i <= 7; i++) digitalWrite(pin(i), HIGH);
  digitalWrite(pin(11), HIGH); digitalWrite(pin(12), HIGH); delay(delayms);
  bool test2 = (digitalRead(pin(8)) == LOW);   // All HIGH -> LOW
  
  // Test with exactly 1 HIGH input (should output HIGH)
  for(int i = 1; i <= 7; i++) digitalWrite(pin(i), LOW);
  digitalWrite(pin(11), LOW); digitalWrite(pin(12), HIGH); delay(delayms);
  bool test3 = (digitalRead(pin(8)) == HIGH);  // 1 HIGH, 7 LOW -> HIGH
  
  resetAllPins();
  
  if(test1 && test2 && test3) {
    foundIC = "7430";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}

bool testIC7473() {
  Serial.println("Testing 7473 (Dual JK FF)...");
  powerIC(4, 11);
  
  bool ff1 = testJK_FF_Neg_With_Invert_Clear(pin(14), pin(3), pin(1), pin(2), pin(12), pin(13));
  bool ff2 = testJK_FF_Neg_With_Invert_Clear(pin(7), pin(10), pin(5), pin(6), pin(9), pin(8));
  
  resetAllPins();
  
  if(ff1 && ff2) {
    foundIC = "IC 7473";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}

bool testIC7474() {
  Serial.println("Testing 7474 (Dual D Flip-Flop)...");
  powerIC(14, 7);
  
  bool passed = true;
  
  // Flip-Flop 1 pins - CORRECTED
  pinMode(pin(2), OUTPUT);   // 1D (Data)
  pinMode(pin(3), OUTPUT);   // 1CLK (Clock)
  pinMode(pin(1), OUTPUT);   // 1CLR (Clear, active LOW) - CORRECTED: Pin 1
  pinMode(pin(4), OUTPUT);   // 1PRE (Preset, active LOW) - CORRECTED: Pin 4
  pinMode(pin(5), INPUT);    // 1Q
  pinMode(pin(6), INPUT);    // 1Q'
  
  // Flip-Flop 2 pins - CORRECTED
  pinMode(pin(12), OUTPUT);  // 2D (Data)
  pinMode(pin(11), OUTPUT);  // 2CLK (Clock)
  pinMode(pin(13), OUTPUT);  // 2CLR (Clear, active LOW) - CORRECTED: Pin 13
  pinMode(pin(10), OUTPUT);  // 2PRE (Preset, active LOW) - CORRECTED: Pin 10
  pinMode(pin(9), INPUT);    // 2Q
  pinMode(pin(8), INPUT);    // 2Q'
  
  // Test Flip-Flop 1 - Clear function
  Serial.println("Testing FF1 Clear...");
  digitalWrite(pin(1), LOW);    // 1CLR active
  digitalWrite(pin(4), HIGH);   // 1PRE inactive
  digitalWrite(pin(2), HIGH);   // 1D = 1
  pulseClock(pin(3));           // Try to clock
  
  delay(delayms);
  
  // After clear, 1Q should be LOW, 1Q' should be HIGH
  if(digitalRead(pin(5)) != LOW || digitalRead(pin(6)) != HIGH) {
    Serial.println("Failed: FF1 Clear function");
    passed = false;
  }
  
  digitalWrite(pin(1), HIGH);   // 1CLR inactive
  
  // Test Flip-Flop 1 - Preset function
  Serial.println("Testing FF1 Preset...");
  digitalWrite(pin(4), LOW);    // 1PRE active
  digitalWrite(pin(1), HIGH);   // 1CLR inactive
  digitalWrite(pin(2), LOW);    // 1D = 0
  pulseClock(pin(3));           // Try to clock
  
  delay(delayms);
  
  // After preset, 1Q should be HIGH, 1Q' should be LOW
  if(digitalRead(pin(5)) != HIGH || digitalRead(pin(6)) != LOW) {
    Serial.println("Failed: FF1 Preset function");
    passed = false;
  }
  
  digitalWrite(pin(4), HIGH);   // 1PRE inactive
  
  // Test Flip-Flop 1 - Data clocking
  Serial.println("Testing FF1 data clocking...");
  digitalWrite(pin(2), HIGH);   // 1D = 1
  pulseClock(pin(3));           // Clock rising edge
  
  if(digitalRead(pin(5)) != HIGH || digitalRead(pin(6)) != LOW) {
    Serial.println("Failed: FF1 data clocking HIGH");
    passed = false;
  }
  
  digitalWrite(pin(2), LOW);    // 1D = 0
  pulseClock(pin(3));           // Clock rising edge
  
  if(digitalRead(pin(5)) != LOW || digitalRead(pin(6)) != HIGH) {
    Serial.println("Failed: FF1 data clocking LOW");
    passed = false;
  }
  
  // Test Flip-Flop 2 - Clear function
  Serial.println("Testing FF2 Clear...");
  digitalWrite(pin(13), LOW);   // 2CLR active
  digitalWrite(pin(10), HIGH);  // 2PRE inactive
  digitalWrite(pin(12), HIGH);  // 2D = 1
  pulseClock(pin(11));          // Try to clock
  
  delay(delayms);
  
  // After clear, 2Q should be LOW, 2Q' should be HIGH
  if(digitalRead(pin(9)) != LOW || digitalRead(pin(8)) != HIGH) {
    Serial.println("Failed: FF2 Clear function");
    passed = false;
  }
  
  digitalWrite(pin(13), HIGH);  // 2CLR inactive
  
  // Test Flip-Flop 2 - Preset function
  Serial.println("Testing FF2 Preset...");
  digitalWrite(pin(10), LOW);   // 2PRE active
  digitalWrite(pin(13), HIGH);  // 2CLR inactive
  digitalWrite(pin(12), LOW);   // 2D = 0
  pulseClock(pin(11));          // Try to clock
  
  delay(delayms);
  
  // After preset, 2Q should be HIGH, 2Q' should be LOW
  if(digitalRead(pin(9)) != HIGH || digitalRead(pin(8)) != LOW) {
    Serial.println("Failed: FF2 Preset function");
    passed = false;
  }
  
  digitalWrite(pin(10), HIGH);  // 2PRE inactive
  
  // Test Flip-Flop 2 - Data clocking
  Serial.println("Testing FF2 data clocking...");
  digitalWrite(pin(12), HIGH);  // 2D = 1
  pulseClock(pin(11));          // Clock rising edge
  
  if(digitalRead(pin(9)) != HIGH || digitalRead(pin(8)) != LOW) {
    Serial.println("Failed: FF2 data clocking HIGH");
    passed = false;
  }
  
  digitalWrite(pin(12), LOW);   // 2D = 0
  pulseClock(pin(11));          // Clock rising edge
  
  if(digitalRead(pin(9)) != LOW || digitalRead(pin(8)) != HIGH) {
    Serial.println("Failed: FF2 data clocking LOW");
    passed = false;
  }
  
  // Test both flip-flops simultaneously
  Serial.println("Testing both flip-flops simultaneously...");
  
  // Setup FF1: D=1, clear and preset inactive
  digitalWrite(pin(2), HIGH);   // 1D = 1
  digitalWrite(pin(1), HIGH);   // 1CLR inactive
  digitalWrite(pin(4), HIGH);   // 1PRE inactive
  
  // Setup FF2: D=0, clear and preset inactive
  digitalWrite(pin(12), LOW);   // 2D = 0
  digitalWrite(pin(13), HIGH);  // 2CLR inactive
  digitalWrite(pin(10), HIGH);  // 2PRE inactive
  
  // Clock both simultaneously
  digitalWrite(pin(3), HIGH);   // 1CLK rising edge
  digitalWrite(pin(11), HIGH);  // 2CLK rising edge
  delay(delayms);
  digitalWrite(pin(3), LOW);    // 1CLK falling edge
  digitalWrite(pin(11), LOW);   // 2CLK falling edge
  delay(delayms);
  
  // Verify both outputs
  if(digitalRead(pin(5)) != HIGH || digitalRead(pin(6)) != LOW) {
    Serial.println("Failed: Both FF - FF1 should be HIGH");
    passed = false;
  }
  if(digitalRead(pin(9)) != LOW || digitalRead(pin(8)) != HIGH) {
    Serial.println("Failed: Both FF - FF2 should be LOW");
    passed = false;
  }
  
  // Test asynchronous override
  Serial.println("Testing asynchronous override...");
  
  // Set both to known state
  digitalWrite(pin(2), HIGH);   // 1D = 1
  digitalWrite(pin(12), HIGH);  // 2D = 1
  pulseClock(pin(3));           // Clock FF1
  pulseClock(pin(11));          // Clock FF2
  
  // Now activate clear and preset asynchronously
  digitalWrite(pin(1), LOW);    // 1CLR active
  digitalWrite(pin(10), LOW);   // 2PRE active
  
  delay(delayms);
  
  // FF1 should be cleared, FF2 should be preset
  if(digitalRead(pin(5)) != LOW || digitalRead(pin(6)) != HIGH) {
    Serial.println("Failed: Async override - FF1 should be cleared");
    passed = false;
  }
  if(digitalRead(pin(9)) != HIGH || digitalRead(pin(8)) != LOW) {
    Serial.println("Failed: Async override - FF2 should be preset");
    passed = false;
  }
  
  resetAllPins();
  
  if(passed) {
    foundIC = "IC 7474";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}


// ==================== EXISTING 16-PIN IC TEST FUNCTIONS ====================
bool testIC7485() {
  Serial.println("Testing 7485 (4-bit Magnitude Comparator)...");
  powerIC(16, 8);  // VCC=16, GND=8 (standard 16-pin IC)
  
  bool passed = true;
  
  // Set up pins - CORRECTED PIN ASSIGNMENTS
  // A inputs (A0=LSB to A3=MSB)
  pinMode(pin(10), OUTPUT);  // A0 (LSB)
  pinMode(pin(12), OUTPUT);  // A1
  pinMode(pin(13), OUTPUT);  // A2
  pinMode(pin(15), OUTPUT);  // A3 (MSB)
  
  // B inputs (B0=LSB to B3=MSB)
  pinMode(pin(9), OUTPUT);   // B0 (LSB)
  pinMode(pin(11), OUTPUT);  // B1
  pinMode(pin(14), OUTPUT);  // B2
  pinMode(pin(1), OUTPUT);   // B3 (MSB)
  
  // Cascade inputs
  pinMode(pin(2), OUTPUT);   // INA<B (A < B in)
  pinMode(pin(3), OUTPUT);   // INA=B (A = B in)
  pinMode(pin(4), OUTPUT);   // INA>B (A > B in)
  
  // Outputs
  pinMode(pin(5), INPUT);    // OUTA>B (A > B out)
  pinMode(pin(6), INPUT);    // OUTA=B (A = B out)
  pinMode(pin(7), INPUT);    // OUTA<B (A < B out)
  
  // Set cascade inputs for standalone operation
  digitalWrite(pin(2), LOW);   // INA<B = 0
  digitalWrite(pin(3), HIGH);  // INA=B = 1
  digitalWrite(pin(4), LOW);   // INA>B = 0
  
  // Test 1: A = B (0000 = 0000)
  Serial.println("Testing A = B (0000 = 0000)...");
  // A = 0000
  digitalWrite(pin(10), LOW);  // A0 = 0
  digitalWrite(pin(12), LOW);  // A1 = 0
  digitalWrite(pin(13), LOW);  // A2 = 0
  digitalWrite(pin(15), LOW);  // A3 = 0
  
  // B = 0000
  digitalWrite(pin(9), LOW);   // B0 = 0
  digitalWrite(pin(11), LOW);  // B1 = 0
  digitalWrite(pin(14), LOW);  // B2 = 0
  digitalWrite(pin(1), LOW);   // B3 = 0
  
  delay(delayms);
  
  // Expected: A=B=HIGH, A>B=LOW, A<B=LOW
  if(digitalRead(pin(6)) != HIGH ||  // A=B should be HIGH
     digitalRead(pin(5)) != LOW ||   // A>B should be LOW
     digitalRead(pin(7)) != LOW) {   // A<B should be LOW
    Serial.println("Failed: A = B (0000 = 0000)");
    passed = false;
  }
  
  // Test 2: A > B (0001 > 0000)
  Serial.println("Testing A > B (0001 > 0000)...");
  // A = 0001
  digitalWrite(pin(10), HIGH); // A0 = 1
  digitalWrite(pin(12), LOW);  // A1 = 0
  digitalWrite(pin(13), LOW);  // A2 = 0
  digitalWrite(pin(15), LOW);  // A3 = 0
  
  // B = 0000
  digitalWrite(pin(9), LOW);   // B0 = 0
  digitalWrite(pin(11), LOW);  // B1 = 0
  digitalWrite(pin(14), LOW);  // B2 = 0
  digitalWrite(pin(1), LOW);   // B3 = 0
  
  delay(delayms);
  
  // Expected: A>B=HIGH, A=B=LOW, A<B=LOW
  if(digitalRead(pin(5)) != HIGH ||  // A>B should be HIGH
     digitalRead(pin(6)) != LOW ||   // A=B should be LOW
     digitalRead(pin(7)) != LOW) {   // A<B should be LOW
    Serial.println("Failed: A > B (0001 > 0000)");
    passed = false;
  }
  
  // Test 3: A < B (0000 < 0001)
  Serial.println("Testing A < B (0000 < 0001)...");
  // A = 0000
  digitalWrite(pin(10), LOW);  // A0 = 0
  digitalWrite(pin(12), LOW);  // A1 = 0
  digitalWrite(pin(13), LOW);  // A2 = 0
  digitalWrite(pin(15), LOW);  // A3 = 0
  
  // B = 0001
  digitalWrite(pin(9), HIGH);  // B0 = 1
  digitalWrite(pin(11), LOW);  // B1 = 0
  digitalWrite(pin(14), LOW);  // B2 = 0
  digitalWrite(pin(1), LOW);   // B3 = 0
  
  delay(delayms);
  
  // Expected: A<B=HIGH, A=B=LOW, A>B=LOW
  if(digitalRead(pin(7)) != HIGH ||  // A<B should be HIGH
     digitalRead(pin(6)) != LOW ||   // A=B should be LOW
     digitalRead(pin(5)) != LOW) {   // A>B should be LOW
    Serial.println("Failed: A < B (0000 < 0001)");
    passed = false;
  }
  
  // Test 4: A > B (1000 > 0111) - MSB comparison
  Serial.println("Testing A > B (1000 > 0111)...");
  // A = 1000 (8 decimal)
  digitalWrite(pin(10), LOW);  // A0 = 0
  digitalWrite(pin(12), LOW);  // A1 = 0
  digitalWrite(pin(13), LOW);  // A2 = 0
  digitalWrite(pin(15), HIGH); // A3 = 1
  
  // B = 0111 (7 decimal)
  digitalWrite(pin(9), HIGH);  // B0 = 1
  digitalWrite(pin(11), HIGH); // B1 = 1
  digitalWrite(pin(14), HIGH); // B2 = 1
  digitalWrite(pin(1), LOW);   // B3 = 0
  
  delay(delayms);
  
  // Expected: A>B=HIGH, A=B=LOW, A<B=LOW
  if(digitalRead(pin(5)) != HIGH ||  // A>B should be HIGH
     digitalRead(pin(6)) != LOW ||   // A=B should be LOW
     digitalRead(pin(7)) != LOW) {   // A<B should be LOW
    Serial.println("Failed: A > B (1000 > 0111)");
    passed = false;
  }
  
  // Test 5: A < B (0111 < 1000) - MSB comparison
  Serial.println("Testing A < B (0111 < 1000)...");
  // A = 0111 (7 decimal)
  digitalWrite(pin(10), HIGH); // A0 = 1
  digitalWrite(pin(12), HIGH); // A1 = 1
  digitalWrite(pin(13), HIGH); // A2 = 1
  digitalWrite(pin(15), LOW);  // A3 = 0
  
  // B = 1000 (8 decimal)
  digitalWrite(pin(9), LOW);   // B0 = 0
  digitalWrite(pin(11), LOW);  // B1 = 0
  digitalWrite(pin(14), LOW);  // B2 = 0
  digitalWrite(pin(1), HIGH);  // B3 = 1
  
  delay(delayms);
  
  // Expected: A<B=HIGH, A=B=LOW, A>B=LOW
  if(digitalRead(pin(7)) != HIGH ||  // A<B should be HIGH
     digitalRead(pin(6)) != LOW ||   // A=B should be LOW
     digitalRead(pin(5)) != LOW) {   // A>B should be LOW
    Serial.println("Failed: A < B (0111 < 1000)");
    passed = false;
  }
  
  // Test 6: A = B (1100 = 1100)
  Serial.println("Testing A = B (1100 = 1100)...");
  // A = 1100 (12 decimal)
  digitalWrite(pin(10), LOW);  // A0 = 0
  digitalWrite(pin(12), LOW);  // A1 = 0
  digitalWrite(pin(13), HIGH); // A2 = 1
  digitalWrite(pin(15), HIGH); // A3 = 1
  
  // B = 1100 (12 decimal)
  digitalWrite(pin(9), LOW);   // B0 = 0
  digitalWrite(pin(11), LOW);  // B1 = 0
  digitalWrite(pin(14), HIGH); // B2 = 1
  digitalWrite(pin(1), HIGH);  // B3 = 1
  
  delay(delayms);
  
  // Expected: A=B=HIGH, A>B=LOW, A<B=LOW
  if(digitalRead(pin(6)) != HIGH ||  // A=B should be HIGH
     digitalRead(pin(5)) != LOW ||   // A>B should be LOW
     digitalRead(pin(7)) != LOW) {   // A<B should be LOW
    Serial.println("Failed: A = B (1100 = 1100)");
    passed = false;
  }
  
  // Test 7: Cascade functionality - A=B with INA<B=HIGH
  Serial.println("Testing cascade - A=B with INA<B=HIGH...");
  // A = 0000, B = 0000 (equal)
  digitalWrite(pin(10), LOW);  // A0 = 0
  digitalWrite(pin(12), LOW);  // A1 = 0
  digitalWrite(pin(13), LOW);  // A2 = 0
  digitalWrite(pin(15), LOW);  // A3 = 0
  digitalWrite(pin(9), LOW);   // B0 = 0
  digitalWrite(pin(11), LOW);  // B1 = 0
  digitalWrite(pin(14), LOW);  // B2 = 0
  digitalWrite(pin(1), LOW);   // B3 = 0
  
  // Set cascade to indicate lower bits were A<B
  digitalWrite(pin(2), HIGH);  // INA<B = 1
  digitalWrite(pin(3), LOW);   // INA=B = 0
  digitalWrite(pin(4), LOW);   // INA>B = 0
  
  delay(delayms);
  
  // When A=B but INA<B=HIGH, should output A<B
  if(digitalRead(pin(7)) != HIGH ||  // A<B should be HIGH
     digitalRead(pin(6)) != LOW ||   // A=B should be LOW
     digitalRead(pin(5)) != LOW) {   // A>B should be LOW
    Serial.println("Failed: Cascade - A=B with INA<B=HIGH");
    passed = false;
  }
  
  resetAllPins();
  
  if(passed) {
    foundIC = "IC 7485";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}

bool testIC7447() {
  Serial.println("Testing 7447 (BCD to 7-Segment Decoder)...");
  powerIC(16, 8);
  
  bool passed = true;
  
  // Set up pins
  pinMode(pin(7), OUTPUT);   // A (BCD LSB)
  pinMode(pin(1), OUTPUT);   // B
  pinMode(pin(2), OUTPUT);   // C
  pinMode(pin(6), OUTPUT);   // D (BCD MSB)
  pinMode(pin(9), OUTPUT);   // Lamp Test (active LOW)
  pinMode(pin(4), OUTPUT);   // Blank Input (active LOW)
  pinMode(pin(5), OUTPUT);   // Ripple Blanking (active LOW)
  
  // 7-segment outputs (active LOW)
  pinMode(pin(13), INPUT);   // a
  pinMode(pin(12), INPUT);   // b
  pinMode(pin(11), INPUT);   // c
  pinMode(pin(10), INPUT);   // d
  pinMode(pin(9), INPUT);    // e
  pinMode(pin(15), INPUT);   // f
  pinMode(pin(14), INPUT);   // g
  
  // Disable special functions
  digitalWrite(pin(9), HIGH);  // Lamp Test OFF
  digitalWrite(pin(4), HIGH);  // Blank Input OFF
  digitalWrite(pin(5), HIGH);  // Ripple Blanking OFF
  
  // Test BCD 0 (0000)
  Serial.println("Testing BCD 0...");
  digitalWrite(pin(7), LOW);   // A=0
  digitalWrite(pin(1), LOW);   // B=0
  digitalWrite(pin(2), LOW);   // C=0
  digitalWrite(pin(6), LOW);   // D=0
  delay(delayms);
  
  // For BCD 0, segments a,b,c,d,e,f should be ON (LOW), g should be OFF (HIGH)
  if(digitalRead(pin(13)) != LOW ||  // a ON
     digitalRead(pin(12)) != LOW ||  // b ON
     digitalRead(pin(11)) != LOW ||  // c ON
     digitalRead(pin(10)) != LOW ||  // d ON
     digitalRead(pin(9)) != LOW ||   // e ON
     digitalRead(pin(15)) != LOW ||  // f ON
     digitalRead(pin(14)) != HIGH) { // g OFF
    Serial.println("Failed: BCD 0");
    passed = false;
  }
  
  // Test BCD 5 (0101)
  Serial.println("Testing BCD 5...");
  digitalWrite(pin(7), HIGH);  // A=1
  digitalWrite(pin(1), LOW);   // B=0
  digitalWrite(pin(2), HIGH);  // C=1
  digitalWrite(pin(6), LOW);   // D=0
  delay(delayms);
  
  // For BCD 5, segments a,c,d,f,g should be ON (LOW), b,e should be OFF (HIGH)
  if(digitalRead(pin(13)) != LOW ||  // a ON
     digitalRead(pin(12)) != HIGH || // b OFF
     digitalRead(pin(11)) != LOW ||  // c ON
     digitalRead(pin(10)) != LOW ||  // d ON
     digitalRead(pin(9)) != HIGH ||  // e OFF
     digitalRead(pin(15)) != LOW ||  // f ON
     digitalRead(pin(14)) != LOW) {  // g ON
    Serial.println("Failed: BCD 5");
    passed = false;
  }
  
  // Test Lamp Test function
  Serial.println("Testing Lamp Test...");
  digitalWrite(pin(9), LOW);   // Lamp Test ON
  delay(delayms);
  
  // All segments should be ON (LOW) during lamp test
  if(digitalRead(pin(13)) != LOW ||  // a ON
     digitalRead(pin(12)) != LOW ||  // b ON
     digitalRead(pin(11)) != LOW ||  // c ON
     digitalRead(pin(10)) != LOW ||  // d ON
     digitalRead(pin(9)) != LOW ||   // e ON
     digitalRead(pin(15)) != LOW ||  // f ON
     digitalRead(pin(14)) != LOW) {  // g ON
    Serial.println("Failed: Lamp Test");
    passed = false;
  }
  
  resetAllPins();
  
  if(passed) {
    foundIC = "IC 7447";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}

bool testIC7483() {
  Serial.println("Testing 7483 (4-bit Binary Adder)...");
  powerIC(5, 12);  // VCC=5, GND=12 - CORRECTED
  
  bool passed = true;
  
  // Set up pins - CORRECTED PIN ASSIGNMENTS
  // A inputs (A1=LSB to A4=MSB)
  pinMode(pin(10), OUTPUT);  // A1 (LSB)
  pinMode(pin(8), OUTPUT);   // A2
  pinMode(pin(3), OUTPUT);   // A3
  pinMode(pin(1), OUTPUT);   // A4 (MSB)
  
  // B inputs (B1=LSB to B4=MSB)
  pinMode(pin(11), OUTPUT);  // B1 (LSB)
  pinMode(pin(7), OUTPUT);   // B2
  pinMode(pin(4), OUTPUT);   // B3
  pinMode(pin(16), OUTPUT);  // B4 (MSB)
  
  // Sum outputs (S1=LSB to S4=MSB)
  pinMode(pin(9), INPUT);    // S1 (LSB)
  pinMode(pin(6), INPUT);    // S2
  pinMode(pin(2), INPUT);    // S3
  pinMode(pin(15), INPUT);   // S4 (MSB)
  
  // Carry pins
  pinMode(pin(13), OUTPUT);  // C0 (Carry in)
  pinMode(pin(14), INPUT);   // C4 (Carry out)
  
  // Test 1: 1 + 1 (with no carry in)
  Serial.println("Testing 1 + 1...");
  // A = 0001 (1 decimal)
  digitalWrite(pin(10), HIGH); // A1 = 1 (LSB)
  digitalWrite(pin(8), LOW);   // A2 = 0
  digitalWrite(pin(3), LOW);   // A3 = 0
  digitalWrite(pin(1), LOW);   // A4 = 0 (MSB)
  
  // B = 0001 (1 decimal)
  digitalWrite(pin(11), HIGH); // B1 = 1 (LSB)
  digitalWrite(pin(7), LOW);   // B2 = 0
  digitalWrite(pin(4), LOW);   // B3 = 0
  digitalWrite(pin(16), LOW);  // B4 = 0 (MSB)
  
  digitalWrite(pin(13), LOW);  // C0 = 0 (no carry in)
  
  delay(delayms);
  
  // 1 + 1 = 2 (0010 binary) with carry out 0
  // Expected: S = 0010, C4 = 0
  if(digitalRead(pin(9)) != LOW ||   // S1 = 0
     digitalRead(pin(6)) != HIGH ||  // S2 = 1
     digitalRead(pin(2)) != LOW ||   // S3 = 0
     digitalRead(pin(15)) != LOW ||  // S4 = 0
     digitalRead(pin(14)) != LOW) {  // C4 = 0
    Serial.println("Failed: 1 + 1 = 2");
    Serial.print("Got: S=");
    Serial.print(digitalRead(pin(15)));
    Serial.print(digitalRead(pin(2)));
    Serial.print(digitalRead(pin(6)));
    Serial.print(digitalRead(pin(9)));
    Serial.print(" C4=");
    Serial.println(digitalRead(pin(14)));
    passed = false;
  }
  
  // Test 2: 3 + 2 (0011 + 0010)
  Serial.println("Testing 3 + 2...");
  // A = 0011 (3 decimal)
  digitalWrite(pin(10), HIGH); // A1 = 1
  digitalWrite(pin(8), HIGH);  // A2 = 1
  digitalWrite(pin(3), LOW);   // A3 = 0
  digitalWrite(pin(1), LOW);   // A4 = 0
  
  // B = 0010 (2 decimal)
  digitalWrite(pin(11), LOW);  // B1 = 0
  digitalWrite(pin(7), HIGH);  // B2 = 1
  digitalWrite(pin(4), LOW);   // B3 = 0
  digitalWrite(pin(16), LOW);  // B4 = 0
  
  digitalWrite(pin(13), LOW);  // C0 = 0
  
  delay(delayms);
  
  // 3 + 2 = 5 (0101 binary) with carry out 0
  // Expected: S = 0101, C4 = 0
  if(digitalRead(pin(9)) != HIGH ||  // S1 = 1
     digitalRead(pin(6)) != LOW ||   // S2 = 0
     digitalRead(pin(2)) != HIGH ||  // S3 = 1
     digitalRead(pin(15)) != LOW ||  // S4 = 0
     digitalRead(pin(14)) != LOW) {  // C4 = 0
    Serial.println("Failed: 3 + 2 = 5");
    passed = false;
  }
  
  // Test 3: 7 + 1 (0111 + 0001)
  Serial.println("Testing 7 + 1...");
  // A = 0111 (7 decimal)
  digitalWrite(pin(10), HIGH); // A1 = 1
  digitalWrite(pin(8), HIGH);  // A2 = 1
  digitalWrite(pin(3), HIGH);  // A3 = 1
  digitalWrite(pin(1), LOW);   // A4 = 0
  
  // B = 0001 (1 decimal)
  digitalWrite(pin(11), HIGH); // B1 = 1
  digitalWrite(pin(7), LOW);   // B2 = 0
  digitalWrite(pin(4), LOW);   // B3 = 0
  digitalWrite(pin(16), LOW);  // B4 = 0
  
  digitalWrite(pin(13), LOW);  // C0 = 0
  
  delay(delayms);
  
  // 7 + 1 = 8 (1000 binary) with carry out 0
  // Expected: S = 1000, C4 = 0
  if(digitalRead(pin(9)) != LOW ||   // S1 = 0
     digitalRead(pin(6)) != LOW ||   // S2 = 0
     digitalRead(pin(2)) != LOW ||   // S3 = 0
     digitalRead(pin(15)) != HIGH || // S4 = 1
     digitalRead(pin(14)) != LOW) {  // C4 = 0
    Serial.println("Failed: 7 + 1 = 8");
    passed = false;
  }
  
  // Test 4: 8 + 8 (1000 + 1000) with carry
  Serial.println("Testing 8 + 8...");
  // A = 1000 (8 decimal)
  digitalWrite(pin(10), LOW);  // A1 = 0
  digitalWrite(pin(8), LOW);   // A2 = 0
  digitalWrite(pin(3), LOW);   // A3 = 0
  digitalWrite(pin(1), HIGH);  // A4 = 1
  
  // B = 1000 (8 decimal)
  digitalWrite(pin(11), LOW);  // B1 = 0
  digitalWrite(pin(7), LOW);   // B2 = 0
  digitalWrite(pin(4), LOW);   // B3 = 0
  digitalWrite(pin(16), HIGH); // B4 = 1
  
  digitalWrite(pin(13), LOW);  // C0 = 0
  
  delay(delayms);
  
  // 8 + 8 = 16 (10000 binary) - 4-bit result is 0000 with carry out
  // Expected: S = 0000, C4 = 1
  if(digitalRead(pin(9)) != LOW ||   // S1 = 0
     digitalRead(pin(6)) != LOW ||   // S2 = 0
     digitalRead(pin(2)) != LOW ||   // S3 = 0
     digitalRead(pin(15)) != LOW ||  // S4 = 0
     digitalRead(pin(14)) != HIGH) { // C4 = 1
    Serial.println("Failed: 8 + 8 = 16 (with carry)");
    passed = false;
  }
  
  // Test 5: 15 + 1 (1111 + 0001) with carry in
  Serial.println("Testing 15 + 1 with carry in...");
  // A = 1111 (15 decimal)
  digitalWrite(pin(10), HIGH); // A1 = 1
  digitalWrite(pin(8), HIGH);  // A2 = 1
  digitalWrite(pin(3), HIGH);  // A3 = 1
  digitalWrite(pin(1), HIGH);  // A4 = 1
  
  // B = 0001 (1 decimal)
  digitalWrite(pin(11), HIGH); // B1 = 1
  digitalWrite(pin(7), LOW);   // B2 = 0
  digitalWrite(pin(4), LOW);   // B3 = 0
  digitalWrite(pin(16), LOW);  // B4 = 0
  
  digitalWrite(pin(13), HIGH); // C0 = 1 (carry in)
  
  delay(delayms);
  
  // 15 + 1 + 1 = 17 (10001 binary) - 4-bit result is 0001 with carry out
  // Expected: S = 0001, C4 = 1
  if(digitalRead(pin(9)) != HIGH ||  // S1 = 1
     digitalRead(pin(6)) != LOW ||   // S2 = 0
     digitalRead(pin(2)) != LOW ||   // S3 = 0
     digitalRead(pin(15)) != LOW ||  // S4 = 0
     digitalRead(pin(14)) != HIGH) { // C4 = 1
    Serial.println("Failed: 15 + 1 + 1 = 17 (with carry in)");
    passed = false;
  }
  
  // Test 6: 5 + 10 (0101 + 1010)
  Serial.println("Testing 5 + 10...");
  // A = 0101 (5 decimal)
  digitalWrite(pin(10), HIGH); // A1 = 1
  digitalWrite(pin(8), LOW);   // A2 = 0
  digitalWrite(pin(3), HIGH);  // A3 = 1
  digitalWrite(pin(1), LOW);   // A4 = 0
  
  // B = 1010 (10 decimal)
  digitalWrite(pin(11), LOW);  // B1 = 0
  digitalWrite(pin(7), HIGH);  // B2 = 1
  digitalWrite(pin(4), LOW);   // B3 = 0
  digitalWrite(pin(16), HIGH); // B4 = 1
  
  digitalWrite(pin(13), LOW);  // C0 = 0
  
  delay(delayms);
  
  // 5 + 10 = 15 (1111 binary) with carry out 0
  // Expected: S = 1111, C4 = 0
  if(digitalRead(pin(9)) != HIGH ||  // S1 = 1
     digitalRead(pin(6)) != HIGH ||  // S2 = 1
     digitalRead(pin(2)) != HIGH ||  // S3 = 1
     digitalRead(pin(15)) != HIGH || // S4 = 1
     digitalRead(pin(14)) != LOW) {  // C4 = 0
    Serial.println("Failed: 5 + 10 = 15");
    passed = false;
  }
  
  resetAllPins();
  
  if(passed) {
    foundIC = "IC 7483";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}
bool testIC74283() {
  Serial.println("Testing 74283 (4-bit Binary Full Adder)...");
  powerIC(16, 8);  // VCC=16, GND=8
  
  bool passed = true;
  
  // Set up pins according to your assignment:
  // A inputs (A1=LSB to A4=MSB)
  pinMode(pin(5), OUTPUT);   // A1 (LSB)
  pinMode(pin(3), OUTPUT);   // A2
  pinMode(pin(14), OUTPUT);  // A3
  pinMode(pin(12), OUTPUT);  // A4 (MSB)
  
  // B inputs (B1=LSB to B4=MSB)
  pinMode(pin(6), OUTPUT);   // B1 (LSB)
  pinMode(pin(2), OUTPUT);   // B2
  pinMode(pin(15), OUTPUT);  // B3
  pinMode(pin(11), OUTPUT);  // B4 (MSB)
  
  // Sum outputs (S1=LSB to S4=MSB)
  pinMode(pin(4), INPUT);    // S1 (LSB)
  pinMode(pin(1), INPUT);    // S2
  pinMode(pin(13), INPUT);   // S3
  pinMode(pin(10), INPUT);   // S4 (MSB)
  
  // Carry pins
  pinMode(pin(7), OUTPUT);   // C0 (Carry in)
  pinMode(pin(9), INPUT);    // C4 (Carry out)
  
  // Test 1: 1 + 1 (with no carry in)
  Serial.println("Testing 1 + 1...");
  // A = 0001 (1 decimal)
  digitalWrite(pin(5), HIGH);  // A1 = 1 (LSB)
  digitalWrite(pin(3), LOW);   // A2 = 0
  digitalWrite(pin(14), LOW);  // A3 = 0
  digitalWrite(pin(12), LOW);  // A4 = 0 (MSB)
  
  // B = 0001 (1 decimal)
  digitalWrite(pin(6), HIGH);  // B1 = 1 (LSB)
  digitalWrite(pin(2), LOW);   // B2 = 0
  digitalWrite(pin(15), LOW);  // B3 = 0
  digitalWrite(pin(11), LOW);  // B4 = 0 (MSB)
  
  digitalWrite(pin(7), LOW);   // C0 = 0 (no carry in)
  
  delay(delayms);
  
  // 1 + 1 = 2 (0010 binary) with carry out 0
  // Expected: S = 0010, C4 = 0
  if(digitalRead(pin(4)) != LOW ||   // S1 = 0
     digitalRead(pin(1)) != HIGH ||  // S2 = 1
     digitalRead(pin(13)) != LOW ||  // S3 = 0
     digitalRead(pin(10)) != LOW ||  // S4 = 0
     digitalRead(pin(9)) != LOW) {   // C4 = 0
    Serial.println("Failed: 1 + 1 = 2");
    passed = false;
  }
  
  // Test 2: 3 + 2 (0011 + 0010)
  Serial.println("Testing 3 + 2...");
  // A = 0011 (3 decimal)
  digitalWrite(pin(5), HIGH);  // A1 = 1
  digitalWrite(pin(3), HIGH);  // A2 = 1
  digitalWrite(pin(14), LOW);  // A3 = 0
  digitalWrite(pin(12), LOW);  // A4 = 0
  
  // B = 0010 (2 decimal)
  digitalWrite(pin(6), LOW);   // B1 = 0
  digitalWrite(pin(2), HIGH);  // B2 = 1
  digitalWrite(pin(15), LOW);  // B3 = 0
  digitalWrite(pin(11), LOW);  // B4 = 0
  
  digitalWrite(pin(7), LOW);   // C0 = 0
  
  delay(delayms);
  
  // 3 + 2 = 5 (0101 binary) with carry out 0
  // Expected: S = 0101, C4 = 0
  if(digitalRead(pin(4)) != HIGH ||  // S1 = 1
     digitalRead(pin(1)) != LOW ||   // S2 = 0
     digitalRead(pin(13)) != HIGH || // S3 = 1
     digitalRead(pin(10)) != LOW ||  // S4 = 0
     digitalRead(pin(9)) != LOW) {   // C4 = 0
    Serial.println("Failed: 3 + 2 = 5");
    passed = false;
  }
  
  // Test 3: 7 + 1 (0111 + 0001)
  Serial.println("Testing 7 + 1...");
  // A = 0111 (7 decimal)
  digitalWrite(pin(5), HIGH);  // A1 = 1
  digitalWrite(pin(3), HIGH);  // A2 = 1
  digitalWrite(pin(14), HIGH); // A3 = 1
  digitalWrite(pin(12), LOW);  // A4 = 0
  
  // B = 0001 (1 decimal)
  digitalWrite(pin(6), HIGH);  // B1 = 1
  digitalWrite(pin(2), LOW);   // B2 = 0
  digitalWrite(pin(15), LOW);  // B3 = 0
  digitalWrite(pin(11), LOW);  // B4 = 0
  
  digitalWrite(pin(7), LOW);   // C0 = 0
  
  delay(delayms);
  
  // 7 + 1 = 8 (1000 binary) with carry out 0
  // Expected: S = 1000, C4 = 0
  if(digitalRead(pin(4)) != LOW ||   // S1 = 0
     digitalRead(pin(1)) != LOW ||   // S2 = 0
     digitalRead(pin(13)) != LOW ||  // S3 = 0
     digitalRead(pin(10)) != HIGH || // S4 = 1
     digitalRead(pin(9)) != LOW) {   // C4 = 0
    Serial.println("Failed: 7 + 1 = 8");
    passed = false;
  }
  
  // Test 4: 8 + 8 (1000 + 1000) with carry
  Serial.println("Testing 8 + 8...");
  // A = 1000 (8 decimal)
  digitalWrite(pin(5), LOW);   // A1 = 0
  digitalWrite(pin(3), LOW);   // A2 = 0
  digitalWrite(pin(14), LOW);  // A3 = 0
  digitalWrite(pin(12), HIGH); // A4 = 1
  
  // B = 1000 (8 decimal)
  digitalWrite(pin(6), LOW);   // B1 = 0
  digitalWrite(pin(2), LOW);   // B2 = 0
  digitalWrite(pin(15), LOW);  // B3 = 0
  digitalWrite(pin(11), HIGH); // B4 = 1
  
  digitalWrite(pin(7), LOW);   // C0 = 0
  
  delay(delayms);
  
  // 8 + 8 = 16 (10000 binary) - 4-bit result is 0000 with carry out
  // Expected: S = 0000, C4 = 1
  if(digitalRead(pin(4)) != LOW ||   // S1 = 0
     digitalRead(pin(1)) != LOW ||   // S2 = 0
     digitalRead(pin(13)) != LOW ||  // S3 = 0
     digitalRead(pin(10)) != LOW ||  // S4 = 0
     digitalRead(pin(9)) != HIGH) {  // C4 = 1
    Serial.println("Failed: 8 + 8 = 16 (with carry)");
    passed = false;
  }
  
  // Test 5: 15 + 1 (1111 + 0001) with carry in
  Serial.println("Testing 15 + 1 with carry in...");
  // A = 1111 (15 decimal)
  digitalWrite(pin(5), HIGH);  // A1 = 1
  digitalWrite(pin(3), HIGH);  // A2 = 1
  digitalWrite(pin(14), HIGH); // A3 = 1
  digitalWrite(pin(12), HIGH); // A4 = 1
  
  // B = 0001 (1 decimal)
  digitalWrite(pin(6), HIGH);  // B1 = 1
  digitalWrite(pin(2), LOW);   // B2 = 0
  digitalWrite(pin(15), LOW);  // B3 = 0
  digitalWrite(pin(11), LOW);  // B4 = 0
  
  digitalWrite(pin(7), HIGH);  // C0 = 1 (carry in)
  
  delay(delayms);
  
  // 15 + 1 + 1 = 17 (10001 binary) - 4-bit result is 0001 with carry out
  // Expected: S = 0001, C4 = 1
  if(digitalRead(pin(4)) != HIGH ||  // S1 = 1
     digitalRead(pin(1)) != LOW ||   // S2 = 0
     digitalRead(pin(13)) != LOW ||  // S3 = 0
     digitalRead(pin(10)) != LOW ||  // S4 = 0
     digitalRead(pin(9)) != HIGH) {  // C4 = 1
    Serial.println("Failed: 15 + 1 + 1 = 17 (with carry in)");
    passed = false;
  }
  
  resetAllPins();
  
  if(passed) {
    foundIC = "IC 74283";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}

bool testIC7476() {
  Serial.println("Testing 7476 (Dual JK Flip-Flop)...");
  powerIC(5, 13);  // VCC=5, GND=13 - CORRECTED
  
  bool passed = true;
  
  // Flip-Flop 1 pins - CORRECTED
  pinMode(pin(1), OUTPUT);   // 1CLK (Clock)
  pinMode(pin(3), OUTPUT);   // 1CLR (Clear, active LOW)
  pinMode(pin(2), OUTPUT);   // 1PRE (Preset, active LOW)
  pinMode(pin(4), OUTPUT);   // 1J
  pinMode(pin(16), OUTPUT);  // 1K
  pinMode(pin(15), INPUT);   // 1Q
  pinMode(pin(14), INPUT);   // 1Q'
  
  // Flip-Flop 2 pins - CORRECTED
  pinMode(pin(6), OUTPUT);   // 2CLK (Clock)
  pinMode(pin(8), OUTPUT);   // 2CLR (Clear, active LOW)
  pinMode(pin(7), OUTPUT);   // 2PRE (Preset, active LOW)
  pinMode(pin(9), OUTPUT);   // 2J
  pinMode(pin(12), OUTPUT);  // 2K
  pinMode(pin(10), INPUT);   // 2Q
  pinMode(pin(11), INPUT);   // 2Q'
  
  // Test Flip-Flop 1 - Clear function
  Serial.println("Testing FF1 Clear...");
  digitalWrite(pin(3), LOW);    // 1CLR active
  digitalWrite(pin(2), HIGH);   // 1PRE inactive
  digitalWrite(pin(4), HIGH);   // 1J = 1
  digitalWrite(pin(16), HIGH);  // 1K = 1
  pulseClock(pin(1));           // Try to clock
  
  delay(delayms);
  
  // After clear, 1Q should be LOW, 1Q' should be HIGH
  if(digitalRead(pin(15)) != LOW || digitalRead(pin(14)) != HIGH) {
    Serial.println("Failed: FF1 Clear function");
    passed = false;
  }
  
  digitalWrite(pin(3), HIGH);   // 1CLR inactive
  
  // Test Flip-Flop 1 - Preset function
  Serial.println("Testing FF1 Preset...");
  digitalWrite(pin(2), LOW);    // 1PRE active
  digitalWrite(pin(3), HIGH);   // 1CLR inactive
  digitalWrite(pin(4), LOW);    // 1J = 0
  digitalWrite(pin(16), LOW);   // 1K = 0
  pulseClock(pin(1));           // Try to clock
  
  delay(delayms);
  
  // After preset, 1Q should be HIGH, 1Q' should be LOW
  if(digitalRead(pin(15)) != HIGH || digitalRead(pin(14)) != LOW) {
    Serial.println("Failed: FF1 Preset function");
    passed = false;
  }
  
  digitalWrite(pin(2), HIGH);   // 1PRE inactive
  
  // Test Flip-Flop 1 - JK functionality
  Serial.println("Testing FF1 JK functionality...");
  
  // Test JK=11 (Toggle)
  Serial.println("Testing FF1 JK=11 (Toggle)...");
  bool initialQ = digitalRead(pin(15));
  digitalWrite(pin(4), HIGH);   // 1J = 1
  digitalWrite(pin(16), HIGH);  // 1K = 1
  pulseClock(pin(1));           // Clock to toggle
  
  if(digitalRead(pin(15)) == initialQ) {
    Serial.println("Failed: FF1 JK=11 toggle");
    passed = false;
  }
  
  // Test JK=10 (Set)
  Serial.println("Testing FF1 JK=10 (Set)...");
  digitalWrite(pin(4), HIGH);   // 1J = 1
  digitalWrite(pin(16), LOW);   // 1K = 0
  pulseClock(pin(1));           // Clock to set
  
  if(digitalRead(pin(15)) != HIGH) {
    Serial.println("Failed: FF1 JK=10 set");
    passed = false;
  }
  
  // Test JK=01 (Reset)
  Serial.println("Testing FF1 JK=01 (Reset)...");
  digitalWrite(pin(4), LOW);    // 1J = 0
  digitalWrite(pin(16), HIGH);  // 1K = 1
  pulseClock(pin(1));           // Clock to reset
  
  if(digitalRead(pin(15)) != LOW) {
    Serial.println("Failed: FF1 JK=01 reset");
    passed = false;
  }
  
  // Test JK=00 (No change)
  Serial.println("Testing FF1 JK=00 (No change)...");
  bool currentQ = digitalRead(pin(15));
  digitalWrite(pin(4), LOW);    // 1J = 0
  digitalWrite(pin(16), LOW);   // 1K = 0
  pulseClock(pin(1));           // Clock - should not change
  
  if(digitalRead(pin(15)) != currentQ) {
    Serial.println("Failed: FF1 JK=00 no change");
    passed = false;
  }
  
  // Test Flip-Flop 2 - Clear function
  Serial.println("Testing FF2 Clear...");
  digitalWrite(pin(8), LOW);    // 2CLR active
  digitalWrite(pin(7), HIGH);   // 2PRE inactive
  digitalWrite(pin(9), HIGH);   // 2J = 1
  digitalWrite(pin(12), HIGH);  // 2K = 1
  pulseClock(pin(6));           // Try to clock
  
  delay(delayms);
  
  // After clear, 2Q should be LOW, 2Q' should be HIGH
  if(digitalRead(pin(10)) != LOW || digitalRead(pin(11)) != HIGH) {
    Serial.println("Failed: FF2 Clear function");
    passed = false;
  }
  
  digitalWrite(pin(8), HIGH);   // 2CLR inactive
  
  // Test Flip-Flop 2 - Preset function
  Serial.println("Testing FF2 Preset...");
  digitalWrite(pin(7), LOW);    // 2PRE active
  digitalWrite(pin(8), HIGH);   // 2CLR inactive
  digitalWrite(pin(9), LOW);    // 2J = 0
  digitalWrite(pin(12), LOW);   // 2K = 0
  pulseClock(pin(6));           // Try to clock
  
  delay(delayms);
  
  // After preset, 2Q should be HIGH, 2Q' should be LOW
  if(digitalRead(pin(10)) != HIGH || digitalRead(pin(11)) != LOW) {
    Serial.println("Failed: FF2 Preset function");
    passed = false;
  }
  
  digitalWrite(pin(7), HIGH);   // 2PRE inactive
  
  // Test Flip-Flop 2 - JK functionality
  Serial.println("Testing FF2 JK functionality...");
  
  // Test JK=11 (Toggle)
  Serial.println("Testing FF2 JK=11 (Toggle)...");
  initialQ = digitalRead(pin(10));
  digitalWrite(pin(9), HIGH);   // 2J = 1
  digitalWrite(pin(12), HIGH);  // 2K = 1
  pulseClock(pin(6));           // Clock to toggle
  
  if(digitalRead(pin(10)) == initialQ) {
    Serial.println("Failed: FF2 JK=11 toggle");
    passed = false;
  }
  
  // Test JK=10 (Set)
  Serial.println("Testing FF2 JK=10 (Set)...");
  digitalWrite(pin(9), HIGH);   // 2J = 1
  digitalWrite(pin(12), LOW);   // 2K = 0
  pulseClock(pin(6));           // Clock to set
  
  if(digitalRead(pin(10)) != HIGH) {
    Serial.println("Failed: FF2 JK=10 set");
    passed = false;
  }
  
  // Test JK=01 (Reset)
  Serial.println("Testing FF2 JK=01 (Reset)...");
  digitalWrite(pin(9), LOW);    // 2J = 0
  digitalWrite(pin(12), HIGH);  // 2K = 1
  pulseClock(pin(6));           // Clock to reset
  
  if(digitalRead(pin(10)) != LOW) {
    Serial.println("Failed: FF2 JK=01 reset");
    passed = false;
  }
  
  // Test both flip-flops simultaneously
  Serial.println("Testing both flip-flops simultaneously...");
  
  // Setup FF1: J=1, K=0 (Set)
  digitalWrite(pin(4), HIGH);   // 1J = 1
  digitalWrite(pin(16), LOW);   // 1K = 0
  digitalWrite(pin(3), HIGH);   // 1CLR inactive
  digitalWrite(pin(2), HIGH);   // 1PRE inactive
  
  // Setup FF2: J=0, K=1 (Reset)
  digitalWrite(pin(9), LOW);    // 2J = 0
  digitalWrite(pin(12), HIGH);  // 2K = 1
  digitalWrite(pin(8), HIGH);   // 2CLR inactive
  digitalWrite(pin(7), HIGH);   // 2PRE inactive
  
  // Clock both simultaneously
  digitalWrite(pin(1), HIGH);   // 1CLK rising edge
  digitalWrite(pin(6), HIGH);   // 2CLK rising edge
  delay(delayms);
  digitalWrite(pin(1), LOW);    // 1CLK falling edge
  digitalWrite(pin(6), LOW);    // 2CLK falling edge
  delay(delayms);
  
  // Verify both outputs
  if(digitalRead(pin(15)) != HIGH || digitalRead(pin(14)) != LOW) {
    Serial.println("Failed: Both FF - FF1 should be HIGH (set)");
    passed = false;
  }
  if(digitalRead(pin(10)) != LOW || digitalRead(pin(11)) != HIGH) {
    Serial.println("Failed: Both FF - FF2 should be LOW (reset)");
    passed = false;
  }
  
  resetAllPins();
  
  if(passed) {
    foundIC = "IC 7476";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}


bool testIC74151() {
  Serial.println("Testing 74151 (8:1 MUX)...");
  powerIC(16, 8);
  
  bool passed = true;
  
  // Set up pins
  pinMode(pin(9), OUTPUT);   // Select C (MSB)
  pinMode(pin(10), OUTPUT);  // Select B
  pinMode(pin(11), OUTPUT);  // Select A (LSB)
  pinMode(pin(7), OUTPUT);   // Strobe (Enable, active LOW)
  
  // Data inputs
  pinMode(pin(4), OUTPUT);   // D0
  pinMode(pin(3), OUTPUT);   // D1
  pinMode(pin(2), OUTPUT);   // D2
  pinMode(pin(1), OUTPUT);   // D3
  pinMode(pin(15), OUTPUT);  // D4
  pinMode(pin(14), OUTPUT);  // D5
  pinMode(pin(13), OUTPUT);  // D6
  pinMode(pin(12), OUTPUT);  // D7
  
  // Outputs
  pinMode(pin(5), INPUT);    // Y (output)
  pinMode(pin(6), INPUT);    // W (complement output)
  
  // Enable the multiplexer (Strobe = LOW)
  digitalWrite(pin(7), LOW);
  
  // Test all 8 input selections with HIGH data
  Serial.println("Testing all inputs with HIGH data...");
  for(int i = 0; i < 8; i++) {
    // Set select lines (C, B, A)
    digitalWrite(pin(9), (i & 0x4) ? HIGH : LOW);   // C (bit 2)
    digitalWrite(pin(10), (i & 0x2) ? HIGH : LOW);  // B (bit 1)
    digitalWrite(pin(11), (i & 0x1) ? HIGH : LOW);  // A (bit 0)
    
    // Set all data inputs to LOW except the selected one
    for(int j = 0; j < 8; j++) {
      int dataPin;
      switch(j) {
        case 0: dataPin = 4; break;
        case 1: dataPin = 3; break;
        case 2: dataPin = 2; break;
        case 3: dataPin = 1; break;
        case 4: dataPin = 15; break;
        case 5: dataPin = 14; break;
        case 6: dataPin = 13; break;
        case 7: dataPin = 12; break;
      }
      digitalWrite(pin(dataPin), (j == i) ? HIGH : LOW);
    }
    
    delay(delayms);
    
    // Check outputs
    if(digitalRead(pin(5)) != HIGH) {  // Y should be HIGH
      Serial.print("Failed: Input D");
      Serial.print(i);
      Serial.println(" selection (Y should be HIGH)");
      passed = false;
    }
    if(digitalRead(pin(6)) != LOW) {   // W should be LOW (complement)
      Serial.print("Failed: Input D");
      Serial.print(i);
      Serial.println(" selection (W should be LOW)");
      passed = false;
    }
  }
  
  // Test all 8 input selections with LOW data
  Serial.println("Testing all inputs with LOW data...");
  for(int i = 0; i < 8; i++) {
    // Set select lines
    digitalWrite(pin(9), (i & 0x4) ? HIGH : LOW);
    digitalWrite(pin(10), (i & 0x2) ? HIGH : LOW);
    digitalWrite(pin(11), (i & 0x1) ? HIGH : LOW);
    
    // Set all data inputs to LOW
    digitalWrite(pin(4), LOW);
    digitalWrite(pin(3), LOW);
    digitalWrite(pin(2), LOW);
    digitalWrite(pin(1), LOW);
    digitalWrite(pin(15), LOW);
    digitalWrite(pin(14), LOW);
    digitalWrite(pin(13), LOW);
    digitalWrite(pin(12), LOW);
    
    delay(delayms);
    
    // Check outputs
    if(digitalRead(pin(5)) != LOW) {   // Y should be LOW
      Serial.print("Failed: Input D");
      Serial.print(i);
      Serial.println(" selection (Y should be LOW)");
      passed = false;
    }
    if(digitalRead(pin(6)) != HIGH) {  // W should be HIGH (complement)
      Serial.print("Failed: Input D");
      Serial.print(i);
      Serial.println(" selection (W should be HIGH)");
      passed = false;
    }
  }
  
  // Test disable function (Strobe = HIGH)
  Serial.println("Testing disable function...");
  digitalWrite(pin(7), HIGH);  // Disable the MUX
  
  // Set some inputs to HIGH
  digitalWrite(pin(4), HIGH);
  digitalWrite(pin(11), LOW);  // Select D0
  
  delay(delayms);
  
  // When disabled, both outputs should be LOW
  if(digitalRead(pin(5)) != LOW) {
    Serial.println("Failed: Disable function (Y should be LOW)");
    passed = false;
  }
  if(digitalRead(pin(6)) != LOW) {
    Serial.println("Failed: Disable function (W should be LOW)");
    passed = false;
  }
  
  resetAllPins();
  
  if(passed) {
    foundIC = "IC 74151";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}

bool testIC74153() {
  Serial.println("Testing 74153 (Dual 4:1 MUX)...");
  powerIC(16, 8);
  
  bool passed = true;
  
  // Set up pins
  pinMode(pin(1), OUTPUT);   // 1G (MUX1 Enable, active LOW)
  pinMode(pin(15), OUTPUT);  // 2G (MUX2 Enable, active LOW)
  pinMode(pin(14), OUTPUT);  // A (Select A, common to both MUX)
  pinMode(pin(2), OUTPUT);   // B (Select B, common to both MUX)
  
  // Multiplexer 1 inputs and output - CORRECTED SEQUENCE: 6,5,4,3
  pinMode(pin(6), OUTPUT);   // 1C0 (MUX1 input 0)
  pinMode(pin(5), OUTPUT);   // 1C1 (MUX1 input 1)
  pinMode(pin(4), OUTPUT);   // 1C2 (MUX1 input 2)
  pinMode(pin(3), OUTPUT);   // 1C3 (MUX1 input 3)
  pinMode(pin(7), INPUT);    // 1Y (MUX1 output)
  
  // Multiplexer 2 inputs and output
  pinMode(pin(10), OUTPUT);  // 2C0 (MUX2 input 0)
  pinMode(pin(11), OUTPUT);  // 2C1 (MUX2 input 1)
  pinMode(pin(12), OUTPUT);  // 2C2 (MUX2 input 2)
  pinMode(pin(13), OUTPUT);  // 2C3 (MUX2 input 3)
  pinMode(pin(9), INPUT);    // 2Y (MUX2 output)
  
  // Test Multiplexer 1 - All input selections
  Serial.println("Testing Multiplexer 1...");
  digitalWrite(pin(1), LOW);   // Enable MUX1 (active LOW)
  digitalWrite(pin(15), HIGH); // Disable MUX2
  
  for(int i = 0; i < 4; i++) {
    // Set select lines (B, A)
    digitalWrite(pin(2), (i & 0x2) ? HIGH : LOW);   // B (bit 1, MSB)
    digitalWrite(pin(14), (i & 0x1) ? HIGH : LOW);  // A (bit 0, LSB)
    
    // Set all MUX1 data inputs to LOW except the selected one
    // CORRECTED SEQUENCE: 6=1C0, 5=1C1, 4=1C2, 3=1C3
    digitalWrite(pin(6), (i == 0) ? HIGH : LOW);  // 1C0
    digitalWrite(pin(5), (i == 1) ? HIGH : LOW);  // 1C1
    digitalWrite(pin(4), (i == 2) ? HIGH : LOW);  // 1C2
    digitalWrite(pin(3), (i == 3) ? HIGH : LOW);  // 1C3
    
    delay(delayms);
    
    // Check MUX1 output should be HIGH
    if(digitalRead(pin(7)) != HIGH) {
      Serial.print("Failed: MUX1 - Input 1C");
      Serial.print(i);
      Serial.print(" (BA=");
      Serial.print((i & 0x2) ? 1 : 0);
      Serial.print((i & 0x1) ? 1 : 0);
      Serial.println(") - 1Y should be HIGH");
      passed = false;
    }
  }
  
  // Test Multiplexer 1 - Disable function
  Serial.println("Testing MUX1 disable...");
  digitalWrite(pin(1), HIGH);  // Disable MUX1 (active HIGH disables)
  digitalWrite(pin(2), HIGH);  // B = 1
  digitalWrite(pin(14), HIGH); // A = 1 (should select 1C3 if enabled)
  digitalWrite(pin(3), HIGH);  // 1C3 = HIGH
  
  delay(delayms);
  
  // MUX1 output should be LOW when disabled
  if(digitalRead(pin(7)) != LOW) {
    Serial.println("Failed: MUX1 disable - 1Y should be LOW");
    passed = false;
  }
  
  // Test Multiplexer 2 - All input selections
  Serial.println("Testing Multiplexer 2...");
  digitalWrite(pin(15), LOW);   // Enable MUX2 (active LOW)
  digitalWrite(pin(1), HIGH);   // Disable MUX1
  
  for(int i = 0; i < 4; i++) {
    // Set select lines (B, A)
    digitalWrite(pin(2), (i & 0x2) ? HIGH : LOW);   // B (bit 1, MSB)
    digitalWrite(pin(14), (i & 0x1) ? HIGH : LOW);  // A (bit 0, LSB)
    
    // Set all MUX2 data inputs to LOW except the selected one
    digitalWrite(pin(10), (i == 0) ? HIGH : LOW);  // 2C0
    digitalWrite(pin(11), (i == 1) ? HIGH : LOW);  // 2C1
    digitalWrite(pin(12), (i == 2) ? HIGH : LOW);  // 2C2
    digitalWrite(pin(13), (i == 3) ? HIGH : LOW);  // 2C3
    
    delay(delayms);
    
    // Check MUX2 output should be HIGH
    if(digitalRead(pin(9)) != HIGH) {
      Serial.print("Failed: MUX2 - Input 2C");
      Serial.print(i);
      Serial.print(" (BA=");
      Serial.print((i & 0x2) ? 1 : 0);
      Serial.print((i & 0x1) ? 1 : 0);
      Serial.println(") - 2Y should be HIGH");
      passed = false;
    }
  }
  
  // Test Multiplexer 2 - Disable function
  Serial.println("Testing MUX2 disable...");
  digitalWrite(pin(15), HIGH); // Disable MUX2 (active HIGH disables)
  digitalWrite(pin(2), HIGH);  // B = 1
  digitalWrite(pin(14), HIGH); // A = 1 (should select 2C3 if enabled)
  digitalWrite(pin(13), HIGH); // 2C3 = HIGH
  
  delay(delayms);
  
  // MUX2 output should be LOW when disabled
  if(digitalRead(pin(9)) != LOW) {
    Serial.println("Failed: MUX2 disable - 2Y should be LOW");
    passed = false;
  }
  
  // Test both multiplexers simultaneously with different inputs
  Serial.println("Testing both MUX simultaneously...");
  digitalWrite(pin(1), LOW);    // Enable MUX1
  digitalWrite(pin(15), LOW);   // Enable MUX2
  
  // Set MUX1 to select input 1C1 (A=1, B=0)
  digitalWrite(pin(14), HIGH);  // A = 1
  digitalWrite(pin(2), LOW);    // B = 0
  digitalWrite(pin(6), LOW);    // 1C0 = LOW
  digitalWrite(pin(5), HIGH);   // 1C1 = HIGH
  digitalWrite(pin(4), LOW);    // 1C2 = LOW
  digitalWrite(pin(3), LOW);    // 1C3 = LOW
  
  // Set MUX2 to select input 2C2 (A=0, B=1)
  digitalWrite(pin(10), LOW);   // 2C0 = LOW
  digitalWrite(pin(11), LOW);   // 2C1 = LOW
  digitalWrite(pin(12), HIGH);  // 2C2 = HIGH
  digitalWrite(pin(13), LOW);   // 2C3 = LOW
  
  delay(delayms);
  
  // Verify both outputs
  if(digitalRead(pin(7)) != HIGH) {
    Serial.println("Failed: Both MUX - MUX1 output should be HIGH");
    passed = false;
  }
  if(digitalRead(pin(9)) != HIGH) {
    Serial.println("Failed: Both MUX - MUX2 output should be HIGH");
    passed = false;
  }
  
  // Test specific pattern: MUX1 input 1C3 (A=1, B=1)
  Serial.println("Testing MUX1 specific pattern A=1,B=1 -> 1C3...");
  digitalWrite(pin(14), HIGH);  // A = 1
  digitalWrite(pin(2), HIGH);   // B = 1
  digitalWrite(pin(6), LOW);    // 1C0 = LOW
  digitalWrite(pin(5), LOW);    // 1C1 = LOW
  digitalWrite(pin(4), LOW);    // 1C2 = LOW
  digitalWrite(pin(3), HIGH);   // 1C3 = HIGH
  
  delay(delayms);
  
  if(digitalRead(pin(7)) != HIGH) {
    Serial.println("Failed: MUX1 pattern A=1,B=1 -> 1C3");
    passed = false;
  }
  
  resetAllPins();
  
  if(passed) {
    foundIC = "IC 74153";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}
bool testIC74157() {
  Serial.println("Testing 74157 (Quad 2:1 MUX)...");
  powerIC(16, 8);
  
  bool passed = true;
  
  // Set up pins
  pinMode(pin(1), OUTPUT);   // Select (common to all 4 MUX)
  pinMode(pin(15), OUTPUT);  // Strobe (Enable, active LOW)
  
  // Multiplexer 1 pins
  pinMode(pin(2), OUTPUT);   // 1A (Input A0)
  pinMode(pin(3), OUTPUT);   // 1B (Input A1)
  pinMode(pin(4), INPUT);    // 1Y (Output)
  
  // Multiplexer 2 pins
  pinMode(pin(5), OUTPUT);   // 2A (Input B0)
  pinMode(pin(6), OUTPUT);   // 2B (Input B1)
  pinMode(pin(7), INPUT);    // 2Y (Output)
  
  // Multiplexer 3 pins
  pinMode(pin(11), OUTPUT);  // 3A (Input C0)
  pinMode(pin(10), OUTPUT);  // 3B (Input C1)
  pinMode(pin(9), INPUT);    // 3Y (Output)
  
  // Multiplexer 4 pins
  pinMode(pin(14), OUTPUT);  // 4A (Input D0)
  pinMode(pin(13), OUTPUT);  // 4B (Input D1)
  pinMode(pin(12), INPUT);   // 4Y (Output)
  
  // Enable all multiplexers (Strobe = LOW)
  digitalWrite(pin(15), LOW);
  
  // Test Case 1: Select = LOW (choose A inputs)
  Serial.println("Testing Select=LOW (A inputs)...");
  digitalWrite(pin(1), LOW);
  
  // Set A inputs to HIGH, B inputs to LOW
  digitalWrite(pin(2), HIGH);   // 1A = HIGH
  digitalWrite(pin(3), LOW);    // 1B = LOW
  digitalWrite(pin(5), HIGH);   // 2A = HIGH
  digitalWrite(pin(6), LOW);    // 2B = LOW
  digitalWrite(pin(11), HIGH);  // 3A = HIGH
  digitalWrite(pin(10), LOW);   // 3B = LOW
  digitalWrite(pin(14), HIGH);  // 4A = HIGH
  digitalWrite(pin(13), LOW);   // 4B = LOW
  
  delay(delayms);
  
  // All outputs should be HIGH (selecting A inputs)
  if(digitalRead(pin(4)) != HIGH) {
    Serial.println("Failed: MUX1 output with Select=LOW");
    passed = false;
  }
  if(digitalRead(pin(7)) != HIGH) {
    Serial.println("Failed: MUX2 output with Select=LOW");
    passed = false;
  }
  if(digitalRead(pin(9)) != HIGH) {
    Serial.println("Failed: MUX3 output with Select=LOW");
    passed = false;
  }
  if(digitalRead(pin(12)) != HIGH) {
    Serial.println("Failed: MUX4 output with Select=LOW");
    passed = false;
  }
  
  // Test Case 2: Select = HIGH (choose B inputs)
  Serial.println("Testing Select=HIGH (B inputs)...");
  digitalWrite(pin(1), HIGH);
  
  // Set A inputs to LOW, B inputs to HIGH
  digitalWrite(pin(2), LOW);    // 1A = LOW
  digitalWrite(pin(3), HIGH);   // 1B = HIGH
  digitalWrite(pin(5), LOW);    // 2A = LOW
  digitalWrite(pin(6), HIGH);   // 2B = HIGH
  digitalWrite(pin(11), LOW);   // 3A = LOW
  digitalWrite(pin(10), HIGH);  // 3B = HIGH
  digitalWrite(pin(14), LOW);   // 4A = LOW
  digitalWrite(pin(13), HIGH);  // 4B = HIGH
  
  delay(delayms);
  
  // All outputs should be HIGH (selecting B inputs)
  if(digitalRead(pin(4)) != HIGH) {
    Serial.println("Failed: MUX1 output with Select=HIGH");
    passed = false;
  }
  if(digitalRead(pin(7)) != HIGH) {
    Serial.println("Failed: MUX2 output with Select=HIGH");
    passed = false;
  }
  if(digitalRead(pin(9)) != HIGH) {
    Serial.println("Failed: MUX3 output with Select=HIGH");
    passed = false;
  }
  if(digitalRead(pin(12)) != HIGH) {
    Serial.println("Failed: MUX4 output with Select=HIGH");
    passed = false;
  }
  
  // Test Case 3: Mixed inputs with Select=LOW
  Serial.println("Testing mixed inputs with Select=LOW...");
  digitalWrite(pin(1), LOW);
  
  // Set mixed pattern: A0=H, A1=L, B0=L, B1=H, C0=L, C1=H, D0=H, D1=L
  digitalWrite(pin(2), HIGH);   // 1A = HIGH
  digitalWrite(pin(3), LOW);    // 1B = LOW
  digitalWrite(pin(5), LOW);    // 2A = LOW
  digitalWrite(pin(6), HIGH);   // 2B = HIGH
  digitalWrite(pin(11), LOW);   // 3A = LOW
  digitalWrite(pin(10), HIGH);  // 3B = HIGH
  digitalWrite(pin(14), HIGH);  // 4A = HIGH
  digitalWrite(pin(13), LOW);   // 4B = LOW
  
  delay(delayms);
  
  // Outputs should match A inputs pattern: H, L, L, H
  if(digitalRead(pin(4)) != HIGH) {
    Serial.println("Failed: MUX1 mixed pattern");
    passed = false;
  }
  if(digitalRead(pin(7)) != LOW) {
    Serial.println("Failed: MUX2 mixed pattern");
    passed = false;
  }
  if(digitalRead(pin(9)) != LOW) {
    Serial.println("Failed: MUX3 mixed pattern");
    passed = false;
  }
  if(digitalRead(pin(12)) != HIGH) {
    Serial.println("Failed: MUX4 mixed pattern");
    passed = false;
  }
  
  // Test Case 4: Disable function (Strobe = HIGH)
  Serial.println("Testing disable function...");
  digitalWrite(pin(15), HIGH);  // Disable all multiplexers
  
  // Set some inputs to HIGH
  digitalWrite(pin(2), HIGH);
  digitalWrite(pin(3), HIGH);
  digitalWrite(pin(1), LOW);    // Select A inputs
  
  delay(delayms);
  
  // When disabled, all outputs should be LOW regardless of inputs
  if(digitalRead(pin(4)) != LOW) {
    Serial.println("Failed: MUX1 disable function");
    passed = false;
  }
  if(digitalRead(pin(7)) != LOW) {
    Serial.println("Failed: MUX2 disable function");
    passed = false;
  }
  if(digitalRead(pin(9)) != LOW) {
    Serial.println("Failed: MUX3 disable function");
    passed = false;
  }
  if(digitalRead(pin(12)) != LOW) {
    Serial.println("Failed: MUX4 disable function");
    passed = false;
  }
  
  resetAllPins();
  
  if(passed) {
    foundIC = "IC 74157";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    icOK = true;
    return true;
  }
  return false;
}

bool testIC74138() {
  Serial.println("Testing 74138 (3-to-8 Decoder/Demux)...");
  powerIC(16, 8);
  
  bool passed = true;
  
  // Set up pins
  pinMode(pin(1), OUTPUT);   // A (LSB)
  pinMode(pin(2), OUTPUT);   // B
  pinMode(pin(3), OUTPUT);   // C (MSB)
  
  // Enable inputs (all active LOW)
  pinMode(pin(4), OUTPUT);   // G2A (Enable 2A)
  pinMode(pin(5), OUTPUT);   // G2B (Enable 2B)
  pinMode(pin(6), OUTPUT);   // G1 (Enable 1, active HIGH)
  
  // Outputs (active LOW)
  pinMode(pin(15), INPUT);   // Y0
  pinMode(pin(14), INPUT);   // Y1
  pinMode(pin(13), INPUT);   // Y2
  pinMode(pin(12), INPUT);   // Y3
  pinMode(pin(11), INPUT);   // Y4
  pinMode(pin(10), INPUT);   // Y5
  pinMode(pin(9), INPUT);    // Y6
  pinMode(pin(7), INPUT);    // Y7
  
  // Enable the decoder (G1=HIGH, G2A=LOW, G2B=LOW)
  digitalWrite(pin(6), HIGH);  // G1 = HIGH
  digitalWrite(pin(4), LOW);   // G2A = LOW
  digitalWrite(pin(5), LOW);   // G2B = LOW
  
  // Test all 8 output selections
  Serial.println("Testing all decoder outputs...");
  for(int i = 0; i < 8; i++) {
    // Set select lines (C, B, A)
    digitalWrite(pin(3), (i & 0x4) ? HIGH : LOW);   // C (bit 2)
    digitalWrite(pin(2), (i & 0x2) ? HIGH : LOW);   // B (bit 1)
    digitalWrite(pin(1), (i & 0x1) ? HIGH : LOW);   // A (bit 0)
    
    delay(delayms);
    
    // Check that only the selected output is LOW (active), all others HIGH
    for(int j = 0; j < 8; j++) {
      int outputPin;
      switch(j) {
        case 0: outputPin = 15; break;
        case 1: outputPin = 14; break;
        case 2: outputPin = 13; break;
        case 3: outputPin = 12; break;
        case 4: outputPin = 11; break;
        case 5: outputPin = 10; break;
        case 6: outputPin = 9; break;
        case 7: outputPin = 7; break;
      }
      
      bool expectedState = (j == i) ? LOW : HIGH;
      bool actualState = digitalRead(pin(outputPin));
      
      if(actualState != expectedState) {
        Serial.print("Failed: Input ");
        Serial.print(i);
        Serial.print(" (CBA=");
        Serial.print(i, BIN);
        Serial.print(") Output Y");
        Serial.print(j);
        Serial.print(" should be ");
        Serial.print(expectedState ? "HIGH" : "LOW");
        Serial.print(" but is ");
        Serial.println(actualState ? "HIGH" : "LOW");
        passed = false;
      }
    }
  }
  
  // Test disable function - G1 = LOW
  Serial.println("Testing disable (G1=LOW)...");
  digitalWrite(pin(6), LOW);    // G1 = LOW (disable)
  digitalWrite(pin(4), LOW);    // G2A = LOW
  digitalWrite(pin(5), LOW);    // G2B = LOW
  digitalWrite(pin(1), HIGH);   // A = 1
  digitalWrite(pin(2), HIGH);   // B = 1
  digitalWrite(pin(3), HIGH);   // C = 1 (should select Y7)
  
  delay(delayms);
  
  // All outputs should be HIGH when disabled
  for(int j = 0; j < 8; j++) {
    int outputPin;
    switch(j) {
      case 0: outputPin = 15; break;
      case 1: outputPin = 14; break;
      case 2: outputPin = 13; break;
      case 3: outputPin = 12; break;
      case 4: outputPin = 11; break;
      case 5: outputPin = 10; break;
      case 6: outputPin = 9; break;
      case 7: outputPin = 7; break;
    }
    
    if(digitalRead(pin(outputPin)) != HIGH) {
      Serial.print("Failed: Disable G1 - Y");
      Serial.print(j);
      Serial.println(" should be HIGH");
      passed = false;
    }
  }
  
  // Test disable function - G2A = HIGH
  Serial.println("Testing disable (G2A=HIGH)...");
  digitalWrite(pin(6), HIGH);   // G1 = HIGH
  digitalWrite(pin(4), HIGH);   // G2A = HIGH (disable)
  digitalWrite(pin(5), LOW);    // G2B = LOW
  digitalWrite(pin(1), HIGH);   // A = 1
  digitalWrite(pin(2), HIGH);   // B = 1
  digitalWrite(pin(3), HIGH);   // C = 1 (should select Y7)
  
  delay(delayms);
  
  // All outputs should be HIGH when disabled
  for(int j = 0; j < 8; j++) {
    int outputPin;
    switch(j) {
      case 0: outputPin = 15; break;
      case 1: outputPin = 14; break;
      case 2: outputPin = 13; break;
      case 3: outputPin = 12; break;
      case 4: outputPin = 11; break;
      case 5: outputPin = 10; break;
      case 6: outputPin = 9; break;
      case 7: outputPin = 7; break;
    }
    
    if(digitalRead(pin(outputPin)) != HIGH) {
      Serial.print("Failed: Disable G2A - Y");
      Serial.print(j);
      Serial.println(" should be HIGH");
      passed = false;
    }
  }
  
  // Test disable function - G2B = HIGH
  Serial.println("Testing disable (G2B=HIGH)...");
  digitalWrite(pin(6), HIGH);   // G1 = HIGH
  digitalWrite(pin(4), LOW);    // G2A = LOW
  digitalWrite(pin(5), HIGH);   // G2B = HIGH (disable)
  digitalWrite(pin(1), HIGH);   // A = 1
  digitalWrite(pin(2), HIGH);   // B = 1
  digitalWrite(pin(3), HIGH);   // C = 1 (should select Y7)
  
  delay(delayms);
  
  // All outputs should be HIGH when disabled
  for(int j = 0; j < 8; j++) {
    int outputPin;
    switch(j) {
      case 0: outputPin = 15; break;
      case 1: outputPin = 14; break;
      case 2: outputPin = 13; break;
      case 3: outputPin = 12; break;
      case 4: outputPin = 11; break;
      case 5: outputPin = 10; break;
      case 6: outputPin = 9; break;
      case 7: outputPin = 7; break;
    }
    
    if(digitalRead(pin(outputPin)) != HIGH) {
      Serial.print("Failed: Disable G2B - Y");
      Serial.print(j);
      Serial.println(" should be HIGH");
      passed = false;
    }
  }
  
  // Test specific pattern: C=0, B=1, A=0 (input 2) should activate Y2
  Serial.println("Testing specific pattern (010 -> Y2)...");
  digitalWrite(pin(6), HIGH);   // Enable
  digitalWrite(pin(4), LOW);
  digitalWrite(pin(5), LOW);
  digitalWrite(pin(1), LOW);    // A=0
  digitalWrite(pin(2), HIGH);   // B=1
  digitalWrite(pin(3), LOW);    // C=0 (010 binary = 2 decimal)
  
  delay(delayms);
  
  // Only Y2 should be LOW, all others HIGH
  if(digitalRead(pin(15)) != HIGH ||  // Y0
     digitalRead(pin(14)) != HIGH ||  // Y1
     digitalRead(pin(13)) != LOW ||   // Y2
     digitalRead(pin(12)) != HIGH ||  // Y3
     digitalRead(pin(11)) != HIGH ||  // Y4
     digitalRead(pin(10)) != HIGH ||  // Y5
     digitalRead(pin(9)) != HIGH ||   // Y6
     digitalRead(pin(7)) != HIGH) {   // Y7
    Serial.println("Failed: Specific pattern 010 -> Y2");
    passed = false;
  }
  
  resetAllPins();
  
  if(passed) {
    foundIC = "IC 74138";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    icOK = true;
    return true;
  }
  return false;
}

bool testIC74139() {
  Serial.println("Testing 74139 (Dual 2-to-4 Decoder/Demux)...");
  powerIC(16, 8);
  
  bool passed = true;
  
  // Set up pins for Decoder 1
  pinMode(pin(1), OUTPUT);   // 1G (Enable, active LOW) - CORRECTED
  pinMode(pin(2), OUTPUT);   // 1A (LSB)
  pinMode(pin(3), OUTPUT);   // 1B (MSB)
  
  // Decoder 1 outputs (active LOW)
  pinMode(pin(4), INPUT);    // 1Y0
  pinMode(pin(5), INPUT);    // 1Y1
  pinMode(pin(6), INPUT);    // 1Y2
  pinMode(pin(7), INPUT);    // 1Y3
  
  // Set up pins for Decoder 2
  pinMode(pin(15), OUTPUT);  // 2G (Enable, active LOW) - CORRECTED
  pinMode(pin(14), OUTPUT);  // 2A (LSB)
  pinMode(pin(13), OUTPUT);  // 2B (MSB)
  
  // Decoder 2 outputs (active LOW)
  pinMode(pin(12), INPUT);   // 2Y0
  pinMode(pin(11), INPUT);   // 2Y1
  pinMode(pin(10), INPUT);   // 2Y2
  pinMode(pin(9), INPUT);    // 2Y3
  
  // Test Decoder 1 - All output selections
  Serial.println("Testing Decoder 1...");
  digitalWrite(pin(1), LOW);   // Enable Decoder 1 (active LOW)
  
  for(int i = 0; i < 4; i++) {
    // Set select lines (B, A) - Note: B is MSB, A is LSB
    digitalWrite(pin(3), (i & 0x2) ? HIGH : LOW);   // B (bit 1, MSB)
    digitalWrite(pin(2), (i & 0x1) ? HIGH : LOW);   // A (bit 0, LSB)
    
    delay(delayms);
    
    // Check that only the selected output is LOW (active), all others HIGH
    for(int j = 0; j < 4; j++) {
      int outputPin;
      switch(j) {
        case 0: outputPin = 4; break;
        case 1: outputPin = 5; break;
        case 2: outputPin = 6; break;
        case 3: outputPin = 7; break;
      }
      
      bool expectedState = (j == i) ? LOW : HIGH;
      bool actualState = digitalRead(pin(outputPin));
      
      if(actualState != expectedState) {
        Serial.print("Failed: Decoder 1 - Input ");
        Serial.print(i);
        Serial.print(" (BA=");
        Serial.print((i & 0x2) ? 1 : 0);
        Serial.print((i & 0x1) ? 1 : 0);
        Serial.print(") Output 1Y");
        Serial.print(j);
        Serial.print(" should be ");
        Serial.print(expectedState ? "HIGH" : "LOW");
        Serial.print(" but is ");
        Serial.println(actualState ? "HIGH" : "LOW");
        passed = false;
      }
    }
  }
  
  // Test Decoder 1 - Disable function
  Serial.println("Testing Decoder 1 disable...");
  digitalWrite(pin(1), HIGH);  // Disable Decoder 1 (active HIGH disables)
  digitalWrite(pin(2), HIGH);  // A = 1
  digitalWrite(pin(3), HIGH);  // B = 1 (should select 1Y3 if enabled)
  
  delay(delayms);
  
  // All Decoder 1 outputs should be HIGH when disabled
  for(int j = 0; j < 4; j++) {
    int outputPin;
    switch(j) {
      case 0: outputPin = 4; break;
      case 1: outputPin = 5; break;
      case 2: outputPin = 6; break;
      case 3: outputPin = 7; break;
    }
    
    if(digitalRead(pin(outputPin)) != HIGH) {
      Serial.print("Failed: Decoder 1 disable - 1Y");
      Serial.print(j);
      Serial.println(" should be HIGH");
      passed = false;
    }
  }
  
  // Test Decoder 2 - All output selections
  Serial.println("Testing Decoder 2...");
  digitalWrite(pin(15), LOW);   // Enable Decoder 2 (active LOW)
  
  for(int i = 0; i < 4; i++) {
    // Set select lines (B, A) - Note: B is MSB, A is LSB
    digitalWrite(pin(13), (i & 0x2) ? HIGH : LOW);   // B (bit 1, MSB)
    digitalWrite(pin(14), (i & 0x1) ? HIGH : LOW);   // A (bit 0, LSB)
    
    delay(delayms);
    
    // Check that only the selected output is LOW (active), all others HIGH
    for(int j = 0; j < 4; j++) {
      int outputPin;
      switch(j) {
        case 0: outputPin = 12; break;
        case 1: outputPin = 11; break;
        case 2: outputPin = 10; break;
        case 3: outputPin = 9; break;
      }
      
      bool expectedState = (j == i) ? LOW : HIGH;
      bool actualState = digitalRead(pin(outputPin));
      
      if(actualState != expectedState) {
        Serial.print("Failed: Decoder 2 - Input ");
        Serial.print(i);
        Serial.print(" (BA=");
        Serial.print((i & 0x2) ? 1 : 0);
        Serial.print((i & 0x1) ? 1 : 0);
        Serial.print(") Output 2Y");
        Serial.print(j);
        Serial.print(" should be ");
        Serial.print(expectedState ? "HIGH" : "LOW");
        Serial.print(" but is ");
        Serial.println(actualState ? "HIGH" : "LOW");
        passed = false;
      }
    }
  }
  
  // Test Decoder 2 - Disable function
  Serial.println("Testing Decoder 2 disable...");
  digitalWrite(pin(15), HIGH);  // Disable Decoder 2 (active HIGH disables)
  digitalWrite(pin(14), HIGH);  // A = 1
  digitalWrite(pin(13), HIGH);  // B = 1 (should select 2Y3 if enabled)
  
  delay(delayms);
  
  // All Decoder 2 outputs should be HIGH when disabled
  for(int j = 0; j < 4; j++) {
    int outputPin;
    switch(j) {
      case 0: outputPin = 12; break;
      case 1: outputPin = 11; break;
      case 2: outputPin = 10; break;
      case 3: outputPin = 9; break;
    }
    
    if(digitalRead(pin(outputPin)) != HIGH) {
      Serial.print("Failed: Decoder 2 disable - 2Y");
      Serial.print(j);
      Serial.println(" should be HIGH");
      passed = false;
    }
  }
  
  // Test both decoders simultaneously with different inputs
  Serial.println("Testing both decoders simultaneously...");
  digitalWrite(pin(1), LOW);    // Enable Decoder 1
  digitalWrite(pin(15), LOW);   // Enable Decoder 2
  
  // Set Decoder 1 to input 1 (A=1, B=0) -> should activate 1Y1
  digitalWrite(pin(2), HIGH);   // 1A = 1
  digitalWrite(pin(3), LOW);    // 1B = 0
  
  // Set Decoder 2 to input 2 (A=0, B=1) -> should activate 2Y2
  digitalWrite(pin(14), LOW);   // 2A = 0
  digitalWrite(pin(13), HIGH);  // 2B = 1
  
  delay(delayms);
  
  // Verify Decoder 1 outputs (should have 1Y1 LOW)
  if(digitalRead(pin(4)) != HIGH ||  // 1Y0
     digitalRead(pin(5)) != LOW ||   // 1Y1 (active)
     digitalRead(pin(6)) != HIGH ||  // 1Y2
     digitalRead(pin(7)) != HIGH) {  // 1Y3
    Serial.println("Failed: Both decoders - Decoder 1 outputs (1Y1 should be LOW)");
    passed = false;
  }
  
  // Verify Decoder 2 outputs (should have 2Y2 LOW)
  if(digitalRead(pin(12)) != HIGH ||  // 2Y0
     digitalRead(pin(11)) != HIGH ||  // 2Y1
     digitalRead(pin(10)) != LOW ||   // 2Y2 (active)
     digitalRead(pin(9)) != HIGH) {   // 2Y3
    Serial.println("Failed: Both decoders - Decoder 2 outputs (2Y2 should be LOW)");
    passed = false;
  }
  
  // Test specific pattern: Decoder 1 input 3 (A=1, B=1) -> 1Y3
  Serial.println("Testing specific pattern A=1,B=1 -> 1Y3...");
  digitalWrite(pin(2), HIGH);   // 1A = 1
  digitalWrite(pin(3), HIGH);   // 1B = 1
  
  delay(delayms);
  
  if(digitalRead(pin(4)) != HIGH ||  // 1Y0
     digitalRead(pin(5)) != HIGH ||  // 1Y1
     digitalRead(pin(6)) != HIGH ||  // 1Y2
     digitalRead(pin(7)) != LOW) {   // 1Y3 (active)
    Serial.println("Failed: Specific pattern A=1,B=1 -> 1Y3");
    passed = false;
  }
  
  resetAllPins();
  
  if(passed) {
    foundIC = "IC 74139";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}
bool testIC74161() {
  Serial.println("Testing 74161 (4-bit Binary Counter)...");
  powerIC(16, 8);
  
  bool passed = true;
  
  // Set up pins
  pinMode(pin(2), OUTPUT);   // Clock (positive edge triggered)
  pinMode(pin(1), OUTPUT);   // Clear (active LOW, asynchronous)
  pinMode(pin(9), OUTPUT);   // Load (active LOW, synchronous)
  pinMode(pin(7), OUTPUT);   // Enable P
  pinMode(pin(10), OUTPUT);  // Enable T
  
  // Data inputs
  pinMode(pin(3), OUTPUT);   // Data A (LSB)
  pinMode(pin(4), OUTPUT);   // Data B
  pinMode(pin(5), OUTPUT);   // Data C
  pinMode(pin(6), OUTPUT);   // Data D (MSB)
  
  // Outputs
  pinMode(pin(14), INPUT);   // QA (LSB)
  pinMode(pin(13), INPUT);   // QB
  pinMode(pin(12), INPUT);   // QC
  pinMode(pin(11), INPUT);   // QD (MSB)
  pinMode(pin(15), INPUT);   // RCO (Ripple Carry Output)
  
  // Test 1: Asynchronous Clear function
  Serial.println("Testing asynchronous clear...");
  digitalWrite(pin(1), HIGH);   // Clear inactive
  digitalWrite(pin(9), HIGH);   // Load inactive
  digitalWrite(pin(7), HIGH);   // Enable P
  digitalWrite(pin(10), HIGH);  // Enable T
  
  // Set some outputs high by clocking
  pulseClock(pin(2));
  pulseClock(pin(2));
  
  // Now clear asynchronously
  digitalWrite(pin(1), LOW);
  delay(delayms);
  
  // All outputs should be LOW after clear
  if(digitalRead(pin(14)) != LOW || digitalRead(pin(13)) != LOW || 
     digitalRead(pin(12)) != LOW || digitalRead(pin(11)) != LOW) {
    Serial.println("Failed: Asynchronous clear");
    passed = false;
  }
  
  digitalWrite(pin(1), HIGH);  // Release clear
  
  // Test 2: Synchronous Load function
  Serial.println("Testing synchronous load...");
  digitalWrite(pin(9), LOW);   // Activate load
  digitalWrite(pin(7), HIGH);  // Enable P (can be don't care for load)
  digitalWrite(pin(10), HIGH); // Enable T (can be don't care for load)
  
  // Set data inputs to specific pattern (1101 = 13)
  digitalWrite(pin(3), HIGH);  // A = 1
  digitalWrite(pin(4), LOW);   // B = 0
  digitalWrite(pin(5), HIGH);  // C = 1
  digitalWrite(pin(6), HIGH);  // D = 1  (1101 binary)
  
  // Clock to load the data
  pulseClock(pin(2));
  
  digitalWrite(pin(9), HIGH);  // Deactivate load
  
  // Outputs should match loaded data (1101)
  if(digitalRead(pin(14)) != HIGH ||  // QA should be 1
     digitalRead(pin(13)) != LOW ||   // QB should be 0
     digitalRead(pin(12)) != HIGH ||  // QC should be 1
     digitalRead(pin(11)) != HIGH) {  // QD should be 1
    Serial.println("Failed: Synchronous load");
    passed = false;
  }
  
  // Test 3: Counting function
  Serial.println("Testing counting function...");
  digitalWrite(pin(9), HIGH);   // Load inactive
  digitalWrite(pin(7), HIGH);   // Enable P
  digitalWrite(pin(10), HIGH);  // Enable T
  
  // Clear counter first
  digitalWrite(pin(1), LOW);
  delay(delayms);
  digitalWrite(pin(1), HIGH);
  
  // Count through a few values
  int expectedCount = 0;
  for(int i = 0; i < 8; i++) {
    pulseClock(pin(2));
    expectedCount = (expectedCount + 1) & 0x0F;  // 4-bit wrap-around
    
    // Verify count
    int actualCount = (digitalRead(pin(11)) << 3) |  // D
                     (digitalRead(pin(12)) << 2) |  // C
                     (digitalRead(pin(13)) << 1) |  // B
                     (digitalRead(pin(14)) << 0);   // A
    
    if(actualCount != expectedCount) {
      Serial.print("Failed: Counting at step ");
      Serial.print(i);
      Serial.print(" - Expected: ");
      Serial.print(expectedCount);
      Serial.print(" Got: ");
      Serial.println(actualCount);
      passed = false;
      break;
    }
  }
  
  // Test 4: Enable/Disable counting
  Serial.println("Testing enable/disable...");
  digitalWrite(pin(9), HIGH);   // Load inactive
  
  // Get current count
  int currentCount = (digitalRead(pin(11)) << 3) |
                    (digitalRead(pin(12)) << 2) |
                    (digitalRead(pin(13)) << 1) |
                    (digitalRead(pin(14)) << 0);
  
  // Disable counting
  digitalWrite(pin(7), LOW);    // Disable P
  digitalWrite(pin(10), HIGH);  // Enable T (but P disabled)
  
  pulseClock(pin(2));
  
  // Count should not change
  int newCount = (digitalRead(pin(11)) << 3) |
                (digitalRead(pin(12)) << 2) |
                (digitalRead(pin(13)) << 1) |
                (digitalRead(pin(14)) << 0);
  
  if(newCount != currentCount) {
    Serial.println("Failed: Disable counting");
    passed = false;
  }
  
  // Test 5: Ripple Carry Output (RCO)
  Serial.println("Testing ripple carry output...");
  digitalWrite(pin(9), HIGH);   // Load inactive
  digitalWrite(pin(7), HIGH);   // Enable P
  digitalWrite(pin(10), HIGH);  // Enable T
  
  // Load value 14 (1110) - next count should trigger RCO
  digitalWrite(pin(9), LOW);    // Activate load
  digitalWrite(pin(3), LOW);    // A = 0
  digitalWrite(pin(4), HIGH);   // B = 1
  digitalWrite(pin(5), HIGH);   // C = 1
  digitalWrite(pin(6), HIGH);   // D = 1  (1110 = 14)
  pulseClock(pin(2));
  digitalWrite(pin(9), HIGH);   // Deactivate load
  
  // RCO should be LOW before reaching 15
  if(digitalRead(pin(15)) != LOW) {
    Serial.println("Failed: RCO should be LOW before count=15");
    passed = false;
  }
  
  // Clock to count to 15 (1111)
  pulseClock(pin(2));
  
  // RCO should be HIGH at count=15 with Enable T active
  if(digitalRead(pin(15)) != HIGH) {
    Serial.println("Failed: RCO should be HIGH at count=15");
    passed = false;
  }
  
  // Clock to wrap around to 0
  pulseClock(pin(2));
  
  // RCO should go LOW again
  if(digitalRead(pin(15)) != LOW) {
    Serial.println("Failed: RCO should be LOW after wrap-around");
    passed = false;
  }
  
  resetAllPins();
  
  if(passed) {
    foundIC = "IC 74161";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}

// Helper function for clock pulses
void pulseClock(int clockPin) {
  digitalWrite(clockPin, LOW);
  delay(delayms);
  digitalWrite(clockPin, HIGH);
  delay(delayms);
  digitalWrite(clockPin, LOW);
  delay(delayms);
}

bool testIC74175() {
  Serial.println("Testing 74175 (Quad D Flip-Flop)...");
  powerIC(16, 8);
  
  bool passed = true;
  
  // Set up pins
  pinMode(pin(9), OUTPUT);   // Clock (common to all flip-flops)
  pinMode(pin(1), OUTPUT);   // Clear (active LOW, common to all)
  
  // Flip-flop 1
  pinMode(pin(4), OUTPUT);   // 1D (Data)
  pinMode(pin(2), INPUT);    // 1Q
  pinMode(pin(3), INPUT);    // 1Q' (Complement)
  
  // Flip-flop 2
  pinMode(pin(5), OUTPUT);   // 2D
  pinMode(pin(7), INPUT);    // 2Q
  pinMode(pin(6), INPUT);    // 2Q'
  
  // Flip-flop 3
  pinMode(pin(12), OUTPUT);  // 3D
  pinMode(pin(10), INPUT);   // 3Q
  pinMode(pin(11), INPUT);   // 3Q'
  
  // Flip-flop 4
  pinMode(pin(13), OUTPUT);  // 4D
  pinMode(pin(15), INPUT);   // 4Q
  pinMode(pin(14), INPUT);   // 4Q'
  
  // Test 1: Asynchronous Clear function
  Serial.println("Testing asynchronous clear...");
  digitalWrite(pin(1), HIGH);   // Clear inactive
  digitalWrite(pin(9), LOW);    // Clock LOW
  
  // Set all data inputs to HIGH
  digitalWrite(pin(4), HIGH);
  digitalWrite(pin(5), HIGH);
  digitalWrite(pin(12), HIGH);
  digitalWrite(pin(13), HIGH);
  
  // Clock to load data
  pulseClock(pin(9));
  
  // Verify data was loaded
  if(digitalRead(pin(2)) != HIGH || digitalRead(pin(7)) != HIGH || 
     digitalRead(pin(10)) != HIGH || digitalRead(pin(15)) != HIGH) {
    Serial.println("Failed: Initial data loading");
    passed = false;
  }
  
  // Now clear asynchronously
  digitalWrite(pin(1), LOW);
  delay(delayms);
  
  // All Q outputs should be LOW after clear, all Q' should be HIGH
  if(digitalRead(pin(2)) != LOW || digitalRead(pin(3)) != HIGH) {
    Serial.println("Failed: Clear - Flip-flop 1");
    passed = false;
  }
  if(digitalRead(pin(7)) != LOW || digitalRead(pin(6)) != HIGH) {
    Serial.println("Failed: Clear - Flip-flop 2");
    passed = false;
  }
  if(digitalRead(pin(10)) != LOW || digitalRead(pin(11)) != HIGH) {
    Serial.println("Failed: Clear - Flip-flop 3");
    passed = false;
  }
  if(digitalRead(pin(15)) != LOW || digitalRead(pin(14)) != HIGH) {
    Serial.println("Failed: Clear - Flip-flop 4");
    passed = false;
  }
  
  digitalWrite(pin(1), HIGH);  // Release clear
  
  // Test 2: Individual data loading
  Serial.println("Testing individual data loading...");
  
  // Test pattern: 1D=1, 2D=0, 3D=1, 4D=0
  digitalWrite(pin(4), HIGH);   // 1D = 1
  digitalWrite(pin(5), LOW);    // 2D = 0
  digitalWrite(pin(12), HIGH);  // 3D = 1
  digitalWrite(pin(13), LOW);   // 4D = 0
  
  // Clock to load data
  pulseClock(pin(9));
  
  // Verify outputs match data inputs
  if(digitalRead(pin(2)) != HIGH || digitalRead(pin(3)) != LOW) {
    Serial.println("Failed: FF1 data loading (1)");
    passed = false;
  }
  if(digitalRead(pin(7)) != LOW || digitalRead(pin(6)) != HIGH) {
    Serial.println("Failed: FF2 data loading (0)");
    passed = false;
  }
  if(digitalRead(pin(10)) != HIGH || digitalRead(pin(11)) != LOW) {
    Serial.println("Failed: FF3 data loading (1)");
    passed = false;
  }
  if(digitalRead(pin(15)) != LOW || digitalRead(pin(14)) != HIGH) {
    Serial.println("Failed: FF4 data loading (0)");
    passed = false;
  }
  
  // Test 3: Data change without clock
  Serial.println("Testing data stability without clock...");
  
  // Change data inputs without clocking
  digitalWrite(pin(4), LOW);    // Change 1D to 0
  digitalWrite(pin(5), HIGH);   // Change 2D to 1
  digitalWrite(pin(12), LOW);   // Change 3D to 0
  digitalWrite(pin(13), HIGH);  // Change 4D to 1
  
  delay(delayms);
  
  // Outputs should NOT change (no clock edge)
  if(digitalRead(pin(2)) != HIGH || digitalRead(pin(7)) != LOW || 
     digitalRead(pin(10)) != HIGH || digitalRead(pin(15)) != LOW) {
    Serial.println("Failed: Data stability without clock");
    passed = false;
  }
  
  // Test 4: Clock edge triggering
  Serial.println("Testing clock edge triggering...");
  
  // Now clock to load new data
  pulseClock(pin(9));
  
  // Outputs should match new data inputs
  if(digitalRead(pin(2)) != LOW || digitalRead(pin(3)) != HIGH) {
    Serial.println("Failed: FF1 clock edge (0)");
    passed = false;
  }
  if(digitalRead(pin(7)) != HIGH || digitalRead(pin(6)) != LOW) {
    Serial.println("Failed: FF2 clock edge (1)");
    passed = false;
  }
  if(digitalRead(pin(10)) != LOW || digitalRead(pin(11)) != HIGH) {
    Serial.println("Failed: FF3 clock edge (0)");
    passed = false;
  }
  if(digitalRead(pin(15)) != HIGH || digitalRead(pin(14)) != LOW) {
    Serial.println("Failed: FF4 clock edge (1)");
    passed = false;
  }
  
  // Test 5: Clear overrides clock
  Serial.println("Testing clear override...");
  
  // Set data to HIGH and try to clock while clear is active
  digitalWrite(pin(4), HIGH);
  digitalWrite(pin(5), HIGH);
  digitalWrite(pin(12), HIGH);
  digitalWrite(pin(13), HIGH);
  
  digitalWrite(pin(1), LOW);   // Activate clear
  pulseClock(pin(9));          // Try to clock
  
  // Outputs should remain cleared despite clock
  if(digitalRead(pin(2)) != LOW || digitalRead(pin(7)) != LOW || 
     digitalRead(pin(10)) != LOW || digitalRead(pin(15)) != LOW) {
    Serial.println("Failed: Clear override");
    passed = false;
  }
  
  // Test 6: Mixed pattern
  Serial.println("Testing mixed pattern...");
  
  digitalWrite(pin(1), HIGH);  // Release clear
  
  // Set mixed pattern: 1D=1, 2D=0, 3D=0, 4D=1
  digitalWrite(pin(4), HIGH);   // 1D = 1
  digitalWrite(pin(5), LOW);    // 2D = 0
  digitalWrite(pin(12), LOW);   // 3D = 0
  digitalWrite(pin(13), HIGH);  // 4D = 1
  
  pulseClock(pin(9));
  
  // Verify mixed pattern
  if(digitalRead(pin(2)) != HIGH || digitalRead(pin(3)) != LOW) {
    Serial.println("Failed: FF1 mixed pattern (1)");
    passed = false;
  }
  if(digitalRead(pin(7)) != LOW || digitalRead(pin(6)) != HIGH) {
    Serial.println("Failed: FF2 mixed pattern (0)");
    passed = false;
  }
  if(digitalRead(pin(10)) != LOW || digitalRead(pin(11)) != HIGH) {
    Serial.println("Failed: FF3 mixed pattern (0)");
    passed = false;
  }
  if(digitalRead(pin(15)) != HIGH || digitalRead(pin(14)) != LOW) {
    Serial.println("Failed: FF4 mixed pattern (1)");
    passed = false;
  }
  
  resetAllPins();
  
  if(passed) {
    foundIC = "IC 74175";
    lcd.setCursor(0, 1);
    lcd.print("IC IS GOOD");
    return true;
  }
  return false;
}
