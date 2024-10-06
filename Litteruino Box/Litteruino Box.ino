#include <Servo.h>
#include <LiquidCrystal_I2C.h>

Servo Move1;
Servo Move2;
Servo Scoop1;
Servo Scoop2;
Servo LID;

const int buttonPin = 2;
int buttonState = 0;
int lastButtonState = 0;
int timeDelayTillClean = 10;
bool cleaningInProgress = false;
bool setTimeMode = true;
unsigned long buttonPressStartTime = 0;
const unsigned long longPressDuration = 2000;
bool returnToSetTimeMode = false;

LiquidCrystal_I2C lcd_1(0x27, 20, 4);

void setup() {
  pinMode(A0, INPUT);  
  LID.attach(8); 
  Move1.attach(9);
  Move2.attach(10); 
  Scoop1.attach(11); 
  Scoop2.attach(12);
  lcd_1.init();
  lcd_1.backlight();
  lcd_1.setCursor(0, 1);
  lcd_1.print("LCD Test");

  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  if (setTimeMode) {
    setTime();
  } else {
    performCleaning();
  }

  handleButtonPress();
}

void handleButtonPress() {
  buttonState = digitalRead(buttonPin);

  if (buttonState == LOW && lastButtonState == HIGH) {
    buttonPressStartTime = millis();
  } else if (buttonState == HIGH && lastButtonState == LOW) {
    unsigned long pressDuration = millis() - buttonPressStartTime;
    
    if (pressDuration >= longPressDuration && !setTimeMode) {
      returnToSetTimeMode = true;
    } else if (setTimeMode && pressDuration < longPressDuration) {
      setTimeMode = false;
      cleaningInProgress = true;
    }
  }
  
  lastButtonState = buttonState;
}

void setTime() {
  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("Set Time:");

  int timeDelayMapped = map(analogRead(A0), 0, 1023, 1, 5);
  lcd_1.setCursor(0, 1);
  lcd_1.print(timeDelayMapped);
  delay(300);

  timeDelayTillClean = timeDelayMapped;
}

void performCleaning() {

  for (int countdown = timeDelayTillClean; countdown > 0; countdown--) {
    lcd_1.setCursor(0, 1);
    lcd_1.print("Countdown: ");
    lcd_1.print(countdown);
    delay(1000);
    handleButtonPress();
  }

  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("Cleaning...");

  Move1.write(180);
  Move2.write(0);
  delay(5000);
  handleButtonPress();

  Move1.write(90);
  Move2.write(90);
  delay(1000);
  handleButtonPress();

  Scoop1.write(180); 
  Scoop2.write(0);
  delay(1000);
  handleButtonPress();

  Scoop1.write(90);
  Scoop2.write(90);
  delay(1000);
  handleButtonPress();

  Move1.write(0); 
  Move2.write(180); 
  delay(5000);
  handleButtonPress();

  Move1.write(90); 
  Move2.write(90); 
  delay(1000);
  handleButtonPress();

  LID.write(180); 
  delay(2000);
  handleButtonPress();

  Scoop1.write(180);
  Scoop2.write(0);
  delay(1000);
  handleButtonPress();

  Scoop1.write(90);
  Scoop2.write(90);
  delay(1000);
  handleButtonPress();

  Scoop1.write(0); 
  Scoop2.write(180);
  delay(1000);
  handleButtonPress();

  Scoop1.write(90);
  Scoop2.write(90);
  delay(1000);
  handleButtonPress();

  LID.write(90); 
  delay(1000);
  handleButtonPress();

  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("Cleaning Complete");
  delay(2000);

  if (returnToSetTimeMode) {
    setTimeMode = true;
    cleaningInProgress = false;
    returnToSetTimeMode = false;
    lcd_1.clear();
    lcd_1.print("Returning to");
    lcd_1.setCursor(0, 1);
    lcd_1.print("Set Time Mode");
    delay(2000);
  } else {
    cleaningInProgress = true;
  }
}