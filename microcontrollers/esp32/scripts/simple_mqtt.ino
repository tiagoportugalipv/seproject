#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
const int potenPin = A5;
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
    Serial.begin(115200);
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

  if (value > 2000){
    Serial.printf("Warning value = %d\n",value);
    char buf[10];
    itoa(value, buf, 10);
    client.publish("test", buf );
  }
  delay(2000);

}
