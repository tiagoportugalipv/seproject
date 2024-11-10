#include<ESP8266WiFi.h> 
#include <PubSubClient.h>

const char* ssid = "ferrari";
const char* password = "spothot_strokegot";
const char* mqtt_server = "20.224.16.34";
const int potenPin = A0;
WiFiClient espClient;
PubSubClient client(espClient);

int value;

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


}

void setup(){
    Serial.begin(9600);
    delay(1000);

    pinMode(potenPin,INPUT);
    setup_wifi();

}

void loop(){

  if (!client.connected()) {
    client.connect("sentinelaClient");
  }

  client.loop();

  value = analogRead(potenPin);

  int midValue = 600; // The resolution on this esp is lower
  // int midValue = 2000;

  if (value > midValue){
    Serial.printf("Warning value = %d\n",value);
    char buf[10];
    itoa(value, buf, 10);
    client.publish("test", buf );
  }

  delay(2000);

}
