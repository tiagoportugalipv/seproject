#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <MQ2.h>

// Sentinela settings

  // Properties
  const int area = 1;

  // Thresholds stuff
  constexpr int potenThresholds[] = {300, 700, 1000};
  constexpr float tempThresholds[] = {35, 40, 50};
  constexpr int humThresholds[] = {70, 80, 90};
  constexpr float lpgThresholds[] = {30000, 60000, 100000};
  constexpr float coThresholds[] = {30000, 60000, 100000};
  constexpr float smokeThresholds[] = {30000, 60000, 100000};

  enum ValueType {
    FLOAT_TYPE,
    INT_TYPE
  };

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

// Functions

void sendMessage(String topic,String message){
  client.publish( topic.c_str(), message.c_str() );
}

String createInfo(String tipo, String unit ,String value ){

  JsonDocument msg;
  char msgOutput[256];

  msg["tipo"]=tipo.c_str();
  msg["value"]=value.c_str();
  msg["unidade"]=unit.c_str();

  serializeJson(msg, msgOutput);
  sendMessage("info",msgOutput);
  return String(msgOutput);

}


String createWarning(String tipo,String value, String unit, int code){

  JsonDocument msg;
  char msgOutput[256];

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
  sendMessage("info",msgOutput);
  return String(msgOutput);

}

void checkValue(void* value, void** thresholds, ValueType type, const String& tipo, const String& unit) {
  if (type == FLOAT_TYPE) {
    float floatValue = *((float*)value);
    float* floatdic = *((float**)thresholds);

    if (floatValue > floatdic[2]) {
      createWarning(tipo, String(floatValue), unit, 3);
      return;
    }

    if (floatValue > floatdic[1]) {
      createWarning(tipo, String(floatValue), unit, 2);
      return;
    }

    if (floatValue > floatdic[0]) {
      createWarning(tipo, String(floatValue), unit, 1);
      return;
    }
  }
  else if (type == INT_TYPE) {
    int intValue = *((int*)value);
    int* intdic = *((int**)thresholds);

    if (intValue > intdic[2]) {
      createWarning(tipo, String(intValue), unit, 3);
      return;
    }

    if (intValue > intdic[1]) {
      createWarning(tipo, String(intValue), unit, 2);
      return;
    }

    if (intValue > intdic[0]) {
      createWarning(tipo, String(intValue), unit, 1);
      return;
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
  createInfo("potenciometer", "" ,String(potenValue) );
  checkValue((void *)&potenValue,(void **)potenThresholds,INT_TYPE, "test","");

  // DHT
  float tempValue = dht.readTemperature();
  createInfo("temperature", "C" ,String(tempValue) );
  checkValue((void *)&tempValue,(void **)tempThresholds,FLOAT_TYPE,"temperature","C");

  float humValue = dht.readHumidity();
  createInfo("hummidity", "%" ,String(humValue) );
  checkValue((void *)&humValue, (void **)humThresholds,FLOAT_TYPE,"humidity", "%");


  // DHT
  float lpgValue = mq2.readLPG();
  createInfo("lpg", "ppm" ,String(humValue) );
  checkValue((void *)&lpgValue, (void **)lpgThresholds,FLOAT_TYPE,"lpg" ,"ppm");

  float coValue = mq2.readCO();
  createInfo("co", "ppm" ,String(humValue) );
  checkValue((void *)&coValue, (void **)coThresholds,FLOAT_TYPE, "co","ppm");

  float smokeValue = mq2.readSmoke();
  createInfo("smoke", "ppm" ,String(humValue) );
  checkValue((void *)&smokeValue, (void **)smokeThresholds,FLOAT_TYPE, "smoke","ppm");

  delay(2000);

}
