#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <MQ2.h>

// Sentinela settings

  // Properties
  const int area = 1;

  // Networking
  const char* ssid = "ferrari";
  const char* password = "spothot_strokegot";
  const char* mqtt_server = "20.224.16.34";
  WiFiClient espClient;
  PubSubClient client(espClient);

  // Pins
  const int potenPin = A5;
  const int mq2Pin = A6;
  const int dhtPin = 4;

  // Vars
  JsonDocument msg;
  char msgOutput[256];


// Sensor setup/settings

  DHT dht(dhtPin,DHT11);
  MQ2 mq2(mq2Pin);

void sendMessage(){
  serializeJson(msg, msgOutput);
  client.publish("test", msgOutput );
}

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
    msg["area"]=area;
    pinMode(dhtPin,INPUT);
    pinMode(potenPin,INPUT);
    dht.begin();

    setup_wifi();

}

void loop(){

  if (!client.connected()) {
    client.connect("sentinelaClient");
  }

  client.loop();

  // Read Values

  Serial.print("\nMQ2 analog value: "); Serial.print(analogRead(mq2Pin));

  // Potenciometer (for testing)
  int potenValue = analogRead(potenPin);

  // DHT
  float tempValue = dht.readTemperature();
  float humValue = dht.readHumidity();


  // DHT
  float lpgValue = mq2.readLPG();
  float coValue = mq2.readCO();
  float smokeValue = mq2.readSmoke();

  msg["tipo"]="potenciometer";
  msg["value"]=potenValue;
  sendMessage();

  msg["tipo"]="temperature";
  msg["value"]=tempValue;
  sendMessage();

  msg["tipo"]="humidity";
  msg["value"]=humValue;
  sendMessage();


  msg["tipo"]="lpg";
  msg["value"]=lpgValue;


  msg["tipo"]="co";
  msg["value"]=coValue;


  msg["tipo"]="smoke";
  msg["value"]=coValue;
  sendMessage();

  delay(2000);

}
