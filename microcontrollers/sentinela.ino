#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <MQ2.h>

// Sentinela settings

  // Properties
  const int area = 1;

  // Thresholds stuff {value, code, condition to be tested} : 0 - less or equal, 1 - greater or equal, 2 equal
  // They should be ordered by priorities 3-2-1 
  constexpr int potenThresholds[][3] = {{1000,3,1},{700,2,1},{300,1,1}};
  constexpr float tempThresholds[][3] = {{45,3,1},{40,2,1},{0,2,2},{35,1,1},{10,1,0}};
  constexpr int humThresholds[][3] = {{25,3,0},{35,2,0},{40,1,0}};
  constexpr float lpgThresholds[][3] = {{100000,3,1},{60000,2,1},{30000,1,1}};
  constexpr float coThresholds[][3] = {{100000,3,1},{60000,2,1},{30000,1,1}};
  constexpr float smokeThresholds[][3] = {{100000,3,1},{60000,2,1},{30000,1,1}};

  // Networking
  const char* ssid = "ferrari";
  const char* password = "spothot_strokegot";
  const char* mqtt_server = "135.236.153.73";
  WiFiClient espClient;
  PubSubClient client(espClient);

  // Pins
  const int potenPin = A5;
  const int mq2Pin = A6;
  const int dhtPin = 4;


// Sensor setup/settings

  DHT dht(dhtPin,DHT11);
  MQ2 mq2(mq2Pin); 

// Functions

void sendMessage(String topic,String message){
  client.publish( topic.c_str(), message.c_str() );
}

String createInfo(String tipo, String unit ,String value ){

  JsonDocument msg;
  char msgOutput[256];

  msg["tipo"]=tipo.c_str();
  msg["area"]=area;
  msg["value"]=value.c_str();
  msg["unidade"]=unit.c_str();

  serializeJson(msg, msgOutput);
  sendMessage("info",msgOutput);
  return String(msgOutput);

}


String createWarning(String tipo,String value, String unit, int code){

  JsonDocument msg;
  char msgOutput[256];
  String areaChannel = "avisos/";
  areaChannel += String(area);

  msg["codigo"]=code;

  switch (code) {

    case 3:
      msg["aviso"]= "Evacuate emmediately, arduous values of"+ tipo;
      break;

    case 2:
      msg["aviso"]= "Dangerous values of "+ tipo + ", "+value+" "+unit;
      break;
    
    default:
      msg["aviso"]= "Warning: spike in "+ tipo + ", "+value+" "+unit;
      break;
  }

  serializeJson(msg, msgOutput);
  sendMessage(areaChannel.c_str(),msgOutput);
  return String(msgOutput);

}


void checkValue(float value, const float thresholds[][3], int n_thresholds, const String& tipo, const String& unit) {
  for (int i = 0; i < n_thresholds; i++) {
    float threshold_value = thresholds[i][0];
    int code = static_cast<int>(thresholds[i][1]);
  
    switch (static_cast<int>(thresholds[i][2])) {
      case 0:
        if (value <= threshold_value) {
          createWarning(tipo, String(value), unit, code);
          return;
        }
        break;

      case 1:
        if (value >= threshold_value) {
          createWarning(tipo, String(value), unit, code);
          return;
        }
        break;

      case 2:
        if (value == threshold_value) {
          createWarning(tipo, String(value), unit, code);
          return;
        }
        break;
    }
  }
}

void checkValue(int value, const int thresholds[][3], int n_thresholds, const String& tipo, const String& unit) {
  for (int i = 0; i < n_thresholds; i++) {
    int threshold_value = thresholds[i][0];
    int code = thresholds[i][1];
  
    switch (thresholds[i][2]) {
      case 0:
        if (value <= threshold_value) {
          createWarning(tipo, String(value), unit, code);
          return;
        }
        break;

      case 1:
        if (value >= threshold_value) {
          createWarning(tipo, String(value), unit, code);
          return;
        }
        break;

      case 2:
        if (value == threshold_value) {
          createWarning(tipo, String(value), unit, code);
          return;
        }
        break;
    }
  }
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
    pinMode(dhtPin,INPUT);
    pinMode(potenPin,INPUT);
    dht.begin();
    mq2.begin();
    delay(10000);

    setup_wifi();

}

void loop() {
  Serial.println("\nStarting loop iteration...");

  if (!client.connected()) {
    Serial.println("MQTT client not connected. Attempting to reconnect...");
    client.connect("sentinelaClient");
  }

  client.loop();
  Serial.println("MQTT client loop processed.");

  // Potenciometer (for testing)
  int potenValue = analogRead(potenPin);
  Serial.print("Potentiometer Value: ");
  Serial.println(potenValue);
  createInfo("potenciometer", "", String(potenValue));
  checkValue(potenValue, potenThresholds, sizeof(potenThresholds)/sizeof(potenThresholds[0]), "test", "");

  // DHT Sensor
  float tempValue = dht.readTemperature();
  Serial.print("Temperature: ");
  Serial.print(tempValue);
  Serial.println(" °C");
  createInfo("temperature", "C", String(tempValue));
  checkValue(tempValue, tempThresholds, sizeof(tempThresholds)/sizeof(tempThresholds[0]), "temperature", "C");

  float humValue = dht.readHumidity();
  Serial.print("Humidity: ");
  Serial.print(humValue);
  Serial.println(" %");
  createInfo("humidity", "%", String(humValue));
  checkValue(humValue, humThresholds, sizeof(humThresholds)/sizeof(humThresholds[0]), "humidity", "%");

  // MQ2 Sensor
  float lpgValue = mq2.readLPG();
  Serial.print("LPG: ");
  Serial.print(lpgValue);
  Serial.println(" ppm");
  createInfo("lpg", "ppm", String(lpgValue));
  checkValue(lpgValue, lpgThresholds, sizeof(lpgThresholds)/sizeof(lpgThresholds[0]), "lpg", "ppm");

  float coValue = mq2.readCO();
  Serial.print("CO: ");
  Serial.print(coValue);
  Serial.println(" ppm");
  createInfo("co", "ppm", String(coValue));
  checkValue(coValue, coThresholds, sizeof(coThresholds)/sizeof(coThresholds[0]), "co", "ppm");

  float smokeValue = mq2.readSmoke();
  Serial.print("Smoke: ");
  Serial.print(smokeValue);
  Serial.println(" ppm");
  createInfo("smoke", "ppm", String(smokeValue));
  checkValue(smokeValue, smokeThresholds, sizeof(smokeThresholds)/sizeof(smokeThresholds[0]), "smoke", "ppm");

  Serial.println("End of loop iteration.\n");
  delay(2000);
}

