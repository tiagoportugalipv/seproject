// https://randomnerdtutorials.com/esp32-mqtt-publish-subscribe-arduino-ide/

#include <WiFiS3.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Adafruit_LiquidCrystal.h"


// Atuador settings

  // Pins

  #define VIBRATION_MOTOR 12
  #define BUZZER_PIN 13
  #define MUTEBUTTON 4
  #define ALERTCODERED 10  // 3
  #define ALERTCODEYELLOW 9 // 2
  #define ALERTCODEGREEN 8 // 1

  // LCD
  
  // Connect via i2c, default address #0 (A0-A2 not jumpered)
  Adafruit_LiquidCrystal lcd(0);


  // Properties
  const int area = 1;

  // Networking
  const char* ssid ="rede2024";
  const char* password = "kkkkkkkk";
  const char* mqtt_server = "135.236.153.73";
  WiFiClient espClient;
  PubSubClient client(espClient);

  // Alert variables

  unsigned long alertStartTime = 0;
  unsigned long alertDuration = 2000;  // 2 seconds on
  unsigned long alertPauseDuration = 500;  // 0.5 seconds off
  bool isAlertActive = false;
  bool isAlertOn = false;
  int currentAlertCode = 0;
  String currentAlertMessage = "";

int alertcodeToPin(int code){
      switch (code) {
        case 1:
          return ALERTCODEGREEN;
          break;
        case 2:
          return ALERTCODEYELLOW;
          break;
        default:
          return ALERTCODERED;
          break;
      }
}

void updateAlert() {
  unsigned long currentTime = millis();

  if (!isAlertActive) return;

  // Check if it's time to toggle alert state
  if (isAlertOn && (currentTime - alertStartTime >= alertDuration)) {
    // Turn off alert
    lcd.setBacklight(LOW);
    digitalWrite(VIBRATION_MOTOR, LOW);
    digitalWrite(ALERTCODERED, LOW);
    digitalWrite(ALERTCODEYELLOW , LOW);
    digitalWrite(ALERTCODEGREEN , LOW);
    noTone(BUZZER_PIN);
    isAlertOn = false;
    alertStartTime = currentTime;
  }
  else if (!isAlertOn && (currentTime - alertStartTime >= alertPauseDuration)) {
    // Check mute button for non-red alerts
    if (digitalRead(MUTEBUTTON) && currentAlertCode != 3) {
      currentAlertCode = 0;
      isAlertActive = false;
      return;
    }

    // Turn on alert
    lcd.setCursor(0, 0);
    lcd.print(currentAlertMessage);
    lcd.setBacklight(HIGH);
    digitalWrite(alertcodeToPin(currentAlertCode), HIGH);
    digitalWrite(VIBRATION_MOTOR, HIGH);

    switch(currentAlertCode) {
      case 1:
        tone(BUZZER_PIN, 2, 2000);
        break;
      case 2:
        tone(BUZZER_PIN, 3, 2000);
        break;
      default:
        tone(BUZZER_PIN, 500, 2000);
    }

    isAlertOn = true;
    alertStartTime = currentTime;
  }
}

void startAlert(Adafruit_LiquidCrystal screen, int buzzerpin, int alertcode, String message) {
  isAlertActive = true;
  isAlertOn = false;
  currentAlertCode = alertcode;
  currentAlertMessage = message;
  alertStartTime = millis();
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.println(". Message: ");
  String messageString;
  
  for (int i = 0; i < length; i++) {
    messageString += (char)message[i];
  }

  JsonDocument messageJson;
  deserializeJson(messageJson, messageString);

  const char* warning = messageJson["aviso"];
  const int warningCode = messageJson["codigo"];

  // Determine if new alert should override current alert
  bool shouldTriggerAlert =  warningCode >= currentAlertCode;
  Serial.println(warningCode);
  Serial.println(currentAlertCode);

  if (shouldTriggerAlert) {
    if (String(topic).equals(String("avisos/")+String(area))) {
      startAlert(lcd, BUZZER_PIN, warningCode, warning);
    }
  }

  if (String(topic) == "avisos/geral") {
    startAlert(lcd, BUZZER_PIN, 3, warning);
  }
}


void reconnect() {

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("avisos/geral"); 
      client.subscribe( (String("avisos/")+String(area)).c_str() );
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup_wifi(){

    WiFi.begin(ssid, password);
    Serial.println("\nConnecting to Wifi");

    while(WiFi.status() != WL_CONNECTED){
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP IP: ");
    Serial.println(WiFi.localIP());

}

void setup_lcd(){

    Serial.println("\nConfiguring LCD");

    while(!lcd.begin(16, 2)){
        delay(100);
    }

    Serial.println("\nLCD ready to be used");
}

void setup(){

  Serial.begin(115200);
  delay(1000);

  // Hardware configuration

  setup_wifi();
  setup_lcd();

  // Pin configuration

  pinMode(ALERTCODERED, OUTPUT);
  pinMode(ALERTCODEYELLOW, OUTPUT);
  pinMode(ALERTCODEGREEN, OUTPUT);
  pinMode(MUTEBUTTON, INPUT);
  pinMode(VIBRATION_MOTOR, OUTPUT);

  // MQTT configuration

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Non-blocking alert update
  updateAlert();
}
