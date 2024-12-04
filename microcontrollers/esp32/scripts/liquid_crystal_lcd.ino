#include "Adafruit_LiquidCrystal.h"

Adafruit_LiquidCrystal lcd(0);

void setup() {
  Serial.begin(115200);
  Serial.println("LCD Character Backpack I2C Test.");

  if (!lcd.begin(16, 2)) {
    Serial.println("Could not init backpack. Check wiring.");
    while(1);
  }
  Serial.println("Backpack init'd.");

  lcd.print("hello, world!");
}

void loop() {
  lcd.setCursor(0, 1);
  lcd.print(millis()/1000);

  lcd.setBacklight(HIGH);
  delay(500);
  lcd.setBacklight(LOW);
  delay(500);
}
