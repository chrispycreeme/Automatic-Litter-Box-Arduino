#include <Servo.h>
#include <LiquidCrystal_I2C.h>

// Pin Definitions
#define POT_PIN A0
#define BUTTON_PIN 2
#define IR_SENSOR_PIN 3
#define SERVO_ML_PIN 9    // Left movement motor
#define SERVO_MR_PIN 10   // Right movement motor
#define SCOOP_L_PIN 11    // Left scooper
#define SCOOP_R_PIN 12    // Right scooper

// Constants
const unsigned long LONG_PRESS_DURATION = 2000;
const unsigned long DISPLAY_UPDATE_INTERVAL = 300;
const int MIN_DELAY = 1;
const int MAX_DELAY = 5;
const int SERVO_STOP = 90;
const int SERVO_FORWARD = 0;
const int SERVO_BACKWARD = 180;

// IR Sensor settings
const int IR_THRESHOLD = 500;  // Adjust based on sensor characteristics
const unsigned long IR_DEBOUNCE_TIME = 1000;

// Scooper positions
const int SCOOP_CENTER = 90;
const int SCOOP_OPEN = 0;
const int SCOOP_CLOSE = 180;

// Movement durations
const unsigned long MOVEMENT_DURATION = 12000;   // 12 seconds for movement
const unsigned long SCOOP_DURATION = 1000;       // 1 second for scooping
const unsigned long DUMP_DURATION = 2000;        // 2 seconds for dumping

// Servo objects
class ServoControl {
private:
  Servo servo;
  int currentPos;
  int pin;

public:
  ServoControl(int servoPin) : pin(servoPin), currentPos(SERVO_STOP) {}
  
  void begin() {
    servo.attach(pin);
    servo.write(SERVO_STOP);
  }
  
  void setPosition(int pos) {
    currentPos = pos;
    servo.write(pos);
  }
  
  int getPosition() {
    return currentPos;
  }
  
  void stop() {
    setPosition(SERVO_STOP);
  }
};

// Global objects
ServoControl motorLeft(SERVO_ML_PIN);
ServoControl motorRight(SERVO_MR_PIN);
ServoControl scoopLeft(SCOOP_L_PIN);
ServoControl scoopRight(SCOOP_R_PIN);
LiquidCrystal_I2C lcd(0x27, 20, 4);

// State variables
struct SystemState {
  bool setTimeMode;
  bool cleaningInProgress;
  bool returnToSetTimeMode;
  bool autoMode;
  int timeDelayTillClean;
  unsigned long lastDisplayUpdate;
  unsigned long buttonPressStartTime;
  unsigned long lastIRCheckTime;
  unsigned long lastDetectionTime;
  int buttonState;
  int lastButtonState;
  bool objectDetected;
} state;

void initializeSystem() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(POT_PIN, INPUT);
  pinMode(IR_SENSOR_PIN, INPUT);
  
  motorLeft.begin();
  motorRight.begin();
  scoopLeft.begin();
  scoopRight.begin();
  
  lcd.init();
  lcd.backlight();
  updateDisplay("System Ready", "Auto Mode: OFF");
  
  state = {
    true,    // setTimeMode
    false,   // cleaningInProgress
    false,   // returnToSetTimeMode
    false,   // autoMode
    MIN_DELAY, // timeDelayTillClean
    0,      // lastDisplayUpdate
    0,      // buttonPressStartTime
    0,      // lastIRCheckTime
    0,      // lastDetectionTime
    HIGH,   // buttonState
    HIGH,   // lastButtonState
    false   // objectDetected
  };
}

void updateDisplay(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  if (line2) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}

bool checkIRSensor() {
  int sensorValue = analogRead(IR_SENSOR_PIN);
  return (sensorValue > IR_THRESHOLD);
}

void handleIRDetection() {
  if (millis() - state.lastIRCheckTime >= IR_DEBOUNCE_TIME) {
    bool currentDetection = checkIRSensor();
    
    if (currentDetection && !state.objectDetected) {
      state.objectDetected = true;
      state.lastDetectionTime = millis();
      
      if (state.autoMode && !state.cleaningInProgress) {
        updateDisplay("Object Detected", "Starting Clean");
        delay(1000);
        state.cleaningInProgress = true;
        state.setTimeMode = false;
      }
    } else if (!currentDetection) {
      state.objectDetected = false;
    }
    
    state.lastIRCheckTime = millis();
  }
}

void handleButton() {
  state.buttonState = digitalRead(BUTTON_PIN);
  
  if (state.buttonState == LOW && state.lastButtonState == HIGH) {
    state.buttonPressStartTime = millis();
  }
  else if (state.buttonState == HIGH && state.lastButtonState == LOW) {
    unsigned long pressDuration = millis() - state.buttonPressStartTime;
    
    if (pressDuration >= LONG_PRESS_DURATION) {
      state.autoMode = !state.autoMode;
      updateDisplay(state.autoMode ? "Auto Mode: ON" : "Auto Mode: OFF", 
                   "Press to Start");
      delay(1000);
    } else {
      if (!state.cleaningInProgress && !state.autoMode) {
        state.setTimeMode = false;
        state.cleaningInProgress = true;
      }
    }
  }
  
  state.lastButtonState = state.buttonState;
}

void performCleaningSequence() {
  static unsigned long sequenceStartTime = 0;
  static int sequenceStep = 0;
  
  if (sequenceStep == 0) {
    sequenceStartTime = millis();
    updateDisplay("Cleaning", "Forward");
    sequenceStep++;
  }
  
  unsigned long currentTime = millis() - sequenceStartTime;
  
  switch (sequenceStep) {
    case 1:  // Move forward
      motorLeft.setPosition(SERVO_FORWARD);
      motorRight.setPosition(SERVO_BACKWARD);
      if (currentTime >= MOVEMENT_DURATION) {
        stopMotors();
        sequenceStep++;
        updateDisplay("Cleaning", "Scooping");
      }
      break;
      
    case 2:  // Open scoops
      scoopLeft.setPosition(SCOOP_OPEN);
      scoopRight.setPosition(SCOOP_OPEN);
      if (currentTime >= MOVEMENT_DURATION + SCOOP_DURATION) {
        sequenceStep++;
      }
      break;
      
    case 3:  // Close scoops
      scoopLeft.setPosition(SCOOP_CLOSE);
      scoopRight.setPosition(SCOOP_CLOSE);
      if (currentTime >= MOVEMENT_DURATION + (2 * SCOOP_DURATION)) {
        sequenceStep++;
        updateDisplay("Cleaning", "Returning");
      }
      break;
      
    case 4:  // Return movement
      motorLeft.setPosition(SERVO_BACKWARD);
      motorRight.setPosition(SERVO_FORWARD);
      if (currentTime >= (2 * MOVEMENT_DURATION)) {
        stopMotors();
        sequenceStep++;
        updateDisplay("Cleaning", "Dumping");
      }
      break;
      
    case 5:  // Open scoops to dump
      scoopLeft.setPosition(SCOOP_OPEN);
      scoopRight.setPosition(SCOOP_OPEN);
      if (currentTime >= (2 * MOVEMENT_DURATION) + DUMP_DURATION) {
        sequenceStep++;
      }
      break;
      
    case 6:  // Reset scoops
      scoopLeft.setPosition(SCOOP_CENTER);
      scoopRight.setPosition(SCOOP_CENTER);
      updateDisplay("Cleaning", "Complete");
      delay(2000);
      
      if (state.returnToSetTimeMode) {
        state.setTimeMode = true;
        state.cleaningInProgress = false;
        state.returnToSetTimeMode = false;
        updateDisplay("Returning to", "Set Time Mode");
        delay(2000);
      } else {
        state.setTimeMode = true;
        state.cleaningInProgress = false;
      }
      sequenceStep = 0;
      break;
  }
}

void stopMotors() {
  motorLeft.stop();
  motorRight.stop();
}

void setup() {
  initializeSystem();
}

void loop() {
  handleButton();
  
  if (state.autoMode) {
    handleIRDetection();
  }
  
  if (state.setTimeMode) {
    if (millis() - state.lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
      state.timeDelayTillClean = map(analogRead(POT_PIN), 0, 1023, MIN_DELAY, MAX_DELAY);
      char buffer[20];
      sprintf(buffer, "Delay: %d min", state.timeDelayTillClean);
      updateDisplay(state.autoMode ? "Auto Mode: ON" : "Manual Mode", buffer);
      state.lastDisplayUpdate = millis();
    }
  } else if (state.cleaningInProgress) {
    performCleaningSequence();
  }
}
