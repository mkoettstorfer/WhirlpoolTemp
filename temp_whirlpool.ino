/*
 NodeMcu-Client-Aussentemperatur
 
 Projektbeschreibung und weitere Beispiele unter https://www.mikrocontroller-elektronik.de/
*/

#include <ESP8266WiFi.h>
#include <DallasTemperature.h> 
#include <Base64.h>
#include <OneWire.h>
#include "DHTesp.h"

#define ONE_WIRE_BUS 2 // D4
#define DHTPIN 5 //D1 //Der Sensor wird an PIN 2 angeschlossen    

#define DHTTYPE DHT22    // Es handelt sich um den DHT22 Sensor

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

const char* ssid = "xxx"; //Hier SSID eures WLAN Netzes eintragen
const char* password = "xxx"; //Hier euer Passwort des WLAN Netzes eintragen

//ThingSpeek Server
const char* server = "api.thingspeak.com";
const int httpPort = 80;
//ThingSpeek
String apiKey = "XXX"; 

WiFiClient client;

/** Initialize DHT sensor */
DHTesp dht;
//DHT dht(DHTPIN, DHTTYPE); //Der Sensor wird ab jetzt mit „dth“ angesprochen
/** Task handle for the light value read task */
//TaskHandle_t tempTaskHandle = NULL;

//
// Setup Serial, Wifi, ...
//
void setup() {
 Serial.begin(115200);
 Serial.setTimeout(2000);

 while(!Serial) { }  
 Serial.println("Wake Up!");
 delay(100);
 
 // Initialize temperature sensor 
 DS18B20.begin(); 
 dht.setup(DHTPIN, DHTesp::DHT22); //DHT22 Sensor starten
 delay(10);

 Serial.println();
 Serial.println();
 Serial.print("Verbinde mich mit Netz: ");
 Serial.println(ssid);
 
 WiFi.begin(ssid, password);
 
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }

 Serial.println("");
 Serial.println("WiFi Verbindung aufgebaut"); 
 Serial.print("Eigene IP des ESP-Modul: ");
 Serial.println(WiFi.localIP());
}

//Funktion um Whirlpooltemperatur zu ermitteln
float getTemperatur() {
 float temp;
 do {
   DS18B20.requestTemperatures(); 
   temp = DS18B20.getTempCByIndex(0);
   delay(100);
 } while (temp == 85.0 || temp == (-127.0));
 return temp;
}

//
//Thingspeek Connection
//
void SendThingSpeak(String temp, String tempOut, String humidity){
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(temp);
    postStr +="&field2=";
    postStr += String(tempOut);
    postStr +="&field3=";
    postStr += String(humidity);
    postStr += "\r\n\r\n";
 
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
 
    Serial.print("Temperatur Whirlpool: ");
    Serial.print(temp);
    Serial.print(" Grad Celcius: ");
    
    Serial.print("Außentemperatur: ");
    Serial.print(tempOut);
    Serial.print(" Grad Celcius: ");    
    
    Serial.print(", Luftfeuchtigkeit: ");    
    Serial.print(humidity);
    Serial.println("%.");
    Serial.println("Sende an Thingspeak.");  
}

//Hauptschleife
void loop() {

  Serial.println("Ermittle Daten");
  //Whirlpool Temp
  char temperaturStr[6];
  float temperatur = getTemperatur();
  dtostrf(temperatur, 2, 1, temperaturStr);
  Serial.print("Temperatur Whirlpool: "); 
  Serial.println(temperaturStr); 

  // Reading temperature and humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity lastValues = dht.getTempAndHumidity();
  //Luffeuchtigekit
  char humidityStr[6];
  dtostrf(lastValues.humidity, 2, 1, humidityStr);
  Serial.print("Luftfeuchtigkeit: "); 
  Serial.println(humidityStr); 

  //Außen Temp
  char temperaturOutStr[6];
  dtostrf(lastValues.temperature, 2, 1, temperaturOutStr);
  Serial.print("Temperatur Außen: "); 
  Serial.println(temperaturOutStr); 
 
  Serial.print("Verbindungsaufbau zu Server ");
  Serial.println(server);

  if (client.connect(server,80)) {   //   "184.106.153.149" or api.thingspeak.com

    SendThingSpeak(temperaturStr, temperaturOutStr, humidityStr);
  } else {
    Serial.println("\nVerbindung gescheitert");
  }
  client.stop();
 
  WiFi.disconnect(); 
  Serial.println("\nVerbindung beendet");
 
  Serial.println("Schlafe jetzt ...");
  ESP.deepSleep( 10*60000000); //Angabe in Minuten - hier 10
}
