#include "WiFi.h"
#include "PubSubClient.h"
#include "DHT.h"
#include "DS18B20.h"

//WiFi credential
const char* ssid = "SSID";
const char* password = "password";

//MQTT credential
const char *mqtt_broker = "mqtt_broker";
const char topic[] = "topic";
const char mqtt_username[] = "username";
const char client_id[] = "client_id";
const char blank[] = "";
const int mqtt_port = port;

//parameters
const int timer = 1e3*60; //1min
const int pumping = 1e3*2.5;
const int samples = 128;

float temperature;
float water_temperature;
float soil_humidity;
float humidity;
bool water_level;
bool watering = false;
int wait = 0;

//pinnout
#define pump 5
#define wtemp 18
#define temp_hum 19
#define shum 34
#define water 33

#define DHTTYPE DHT22

WiFiClient esp32;
PubSubClient client(esp32);
DHT dht(temp_hum, DHTTYPE);
DS18B20 ds18(wtemp);

//avg shum
float shumAvg(int pin, int delay_ms)
{
  float value = 0;
  for(int i = 0; i < samples; i++)
  {
    delay(delay_ms);
    value += analogRead(pin);
  }
  return -0.05555*value/samples+176;
}

//water level
bool waterCheck(int pin)
{
  if(analogRead(pin) > 2850)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void setup() 
{ 
  //Serial.begin(19200);
  //Serial.println("Startup");
  dht.begin();
  
  //wifi init
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    //Serial.println("WiFi connecting");
    delay(200);
  }

  //broker init
  client.setServer(mqtt_broker, mqtt_port);

  while (!client.connect(client_id, mqtt_username, blank))
  {
    //Serial.println("MQTT connecting");
    break;
    delay(200);
  }

  //sensors init
  pinMode(shum, INPUT);
  pinMode(water, INPUT);
  pinMode(temp_hum, INPUT);
  pinMode(wtemp, INPUT);
  pinMode(pump, OUTPUT);
  digitalWrite(pump, HIGH);
}

//
void loop() 
{
  //read values
  soil_humidity = shumAvg(shum, 10);
  water_level = waterCheck(water);
  water_temperature = ds18.tempCheck();
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  //emit log
  String string = 
    "{\"Temp\": " + String(temperature)+ 
    ",\"Humidity\": " + String(humidity) + 
    ",\"Water_Temperature\": " + String(water_temperature)+
    ",\"Water_level\": " + String(water_level)+
    ",\"Soil_humidity\": " + String(soil_humidity)+"}";
    
  client.publish(topic, string.c_str());

  //watering
  if((soil_humidity < 40 && wait > 8 && water_level))
  {
    digitalWrite(pump, LOW);
    for(int i = 0 ; i < 5; i++)
    {
      delay(500);
    }
    digitalWrite(pump, HIGH);
    watering = false;
    wait = 0;
    delay(timer-pumping);
    return;
  }

  delay(timer);
  wait++;
}
