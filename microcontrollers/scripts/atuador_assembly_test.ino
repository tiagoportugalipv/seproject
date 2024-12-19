#include "Adafruit_LiquidCrystal.h"

#define BUZZER_PIN 4
#define MUTEBUTTON 17
#define ALERTCODERED 16
#define ALERTCODEYELLOW 2
#define ALERTCODEGREEN 15

void sendAlert(Adafruit_LiquidCrystal screen,int buzzerpin,int alertcode,String message){
  screen.setCursor(0, 0);
  screen.print(message);

  while(true){

    screen.setBacklight(HIGH);
    digitalWrite(alertcode, HIGH);

    switch(alertcode) {
      case ALERTCODERED:
        tone(buzzerpin, 500, 2000);
        break;
      case ALERTCODEYELLOW:
        tone(buzzerpin, 3, 2000);
        break;
      default:
        tone(buzzerpin, 2, 2000);
    }

    delay(2000);
    
    screen.setBacklight(LOW);
    digitalWrite(alertcode, LOW);
    noTone(buzzerpin);
    
    delay(500);

    if (digitalRead(MUTEBUTTON) && alertcode != ALERTCODERED){
      break;
    }

  }

}


bool alertdone = false;


void setup() {
  Serial.begin(115200);
  // while(!Serial);
  Serial.println("LCD Character Backpack I2C Test.");

  // set up the LCD's number of rows and columns:
  if (!lcd.begin(16, 2)) {
    Serial.println("Could not init backpack. Check wiring.");
    while(1);
  }

  pinMode(ALERTCODERED, OUTPUT);
  pinMode(ALERTCODEYELLOW, OUTPUT);
  pinMode(ALERTCODEGREEN, OUTPUT);
  pinMode(MUTEBUTTON, INPUT);

  Serial.println("Backpack init'd.");

}

void loop() {
  if(!alertdone){
    sendAlert(lcd,BUZZER_PIN,ALERTCODEGREEN,"Olha ai pá, cheira a esturro");
  }

  if(!alertdone){
    sendAlert(lcd,BUZZER_PIN,ALERTCODEYELLOW,"Alguma coisa queimou, vai ver");
  }

  if(!alertdone){
    sendAlert(lcd,BUZZER_PIN,ALERTCODERED,"Foge !!!!");
  }

  alertdone = true;

}
