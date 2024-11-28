#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LCD-I2C.h>

// Atuador settings

  // Properties
  const int area = 3;

  // Networking
  const char* ssid = "";
  const char* password = "";
  const char* mqtt_server = "";
  WiFiClient espClient;
  PubSubClient client(espClient);

  //LCD
  int lcdColumns = 16;
  int lcdRows = 2;
  LCD_I2C lcd(0x27, lcdColumns, lcdRows);
  //LiquidCrystal_IC2 lcd(0x27, lcdColumns, lcdRows);
  //LiquidCrystal lcd(12,11,5,4,3,2);

void setup_wifi(){

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");

    while(WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(100);
    }

    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());

    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

}

//Função que recebe as mensagens do MQTT
void callback(char* topic, byte* message, int lenght)
{
  String string_message;
  for(int i = 0; i < lenght; i++)
  {
    Serial.print((char)message[i]);
    string_message += (char)message[i];
  }

  //Deserialization
  JsonDocument doc;
  deserializeJson(doc, string_message);

  const char* tipo = doc["tipo"];
  
  if(String(topic)=="avisos\area")
  {
    if(tipo=="temperature")
    {
      lcd.setCursor(0,0);
      lcd.println("Risco: Temperatura elevada");
      lcd.setCursor(0,1);
      lcd.println("Evacuar local!!");
    }
    else if(tipo=="humidity")
    {
      lcd.setCursor(0,0);
      lcd.println("Risco: Humidade superior a 60%");
      lcd.setCursor(0,1);
      lcd.println("Evacuar local!!");
    } 
    else if(tipo=="potenciometer")
    {
      lcd.setCursor(0,0);
      lcd.println("Risco: Qualidade do ar prejudicial");
      lcd.setCursor(0,1);
      lcd.println("Evacuar local!!");
    }  
    else if(tipo == "decibel")
    { 
      lcd.setCursor(0,0);
      lcd.println("Risco: Ruído prejudicial");
      lcd.setCursor(0,1);
      lcd.println("Evacuar local!!");
    }
    else
    {
      Serial.println("Erro! Valores errados!");
    }
  }

}

void setup(){

    Serial.begin(9600);
    delay(1000);
    setup_wifi();
}

void loop(){

  if (!client.connected()) {
    client.connect("atuadorClient");
  }

  client.loop();

}
