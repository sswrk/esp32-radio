#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "SPIFFS.h"

#define EEPROM_SIZE 2

long interval = 250;
long previousButtonMillis = 0;

int radioStation = 0;
bool connectedToStation = false;
int previousRadioStation = -1;
const int previousButton = 13;
const int nextButton = 15;

char ssid[] = "<SSID>";
char pass[] = "<PASSWORD>";

String hosts[100] = {"centova.radios.pt", "wamu-1.streamguys.com", "realfm.live24.gr", "live.slovakradio.sk"};
String paths[100] = {"/stream?type=http&nocache=3342", "/", "/realfm", "/Devin_256.mp3"};
int   ports[100] = {9496, 80, 80, 8000};

int availableStations = 3;

AsyncWebServer server(80);

int status = WL_IDLE_STATUS;
WiFiClient  client;
uint8_t mp3buff[32];

//server methods
void notFound(AsyncWebServerRequest *request){
  request->send(404, "application/json", "{\"message\":\"Not found\"}");
}

bool addRadioStation(const char* host, const char* path, int port){
  if(host==NULL || path==NULL || port==NULL){
    return false;
  }
  hosts[availableStations] = String(host);
  paths[availableStations] = String(path);
  ports[availableStations] = port;
  availableStations++;
  
  Serial.println("Added radio station! Available radio stations: ");
  for(int i=0; i<availableStations; i++){
    Serial.print(hosts[i]);
    Serial.print(":");
    Serial.print(ports[i]);
    Serial.print(paths[i]);
    Serial.println("");
  }
  return true;
}

//buttons methods
void IRAM_ATTR previousButtonInterrupt() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousButtonMillis > interval) {

    Serial.println("PREVIOUS");

    if (radioStation > 0)
      radioStation--;
    else
      radioStation = availableStations - 1;

    previousButtonMillis = currentMillis;
  }
}

void IRAM_ATTR nextButtonInterrupt() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousButtonMillis > interval) {

    Serial.println("NEXT");

    if (radioStation < availableStations - 1)
      radioStation++;
    else
      radioStation = 0;

    previousButtonMillis = currentMillis;
  }
}

void setup () {

  Serial.begin(9600);
  delay(500);

  EEPROM.begin(EEPROM_SIZE);

  pinMode(previousButton, INPUT_PULLUP);
  pinMode(nextButton, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(previousButton), previousButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(nextButton), nextButtonInterrupt, RISING);

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  connectToWIFI();

  //server requests handling
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/stations", HTTP_POST, [](AsyncWebServerRequest *request){
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, (const char*)data);
      bool success = false;
      if(!error) {
        const char* host = doc["host"];
        const char* path = doc["path"];
        int port = doc["port"];
        success = addRadioStation(host, path, port);
      }
      if(success){
        request->send(200, "text/plain", "Radio station added");
      } else {
        request->send(400, "text/plain", "Wrong parameters");
      }
  });

  server.onNotFound(notFound);

  server.begin();

  //set radio station
  radioStation = getLastStationFromEEPROM();
  if (radioStation < 0 || radioStation >= availableStations) {
    radioStation = 0;
  }
  previousRadioStation = radioStation;
}

void loop() {

  if (radioStation != previousRadioStation || (connectedToStation==false && availableStations>0)) {
    connectToStation(radioStation);
    previousRadioStation = radioStation;
    writeLastStationToEEPROM(radioStation);
  }

  if (client.available() > 0) {
    uint8_t bytesread = client.read(mp3buff, 32);
//    for(int i=0; i<bytesread; i++){
//      Serial.println(mp3buff[i]);
//    }
//    Serial.println("");
//    TODO: send data to audio module input
  }
}

void connectToStation(int stationId ) {
  Serial.println("Connecting to radio station...");
  char buf[100];
  hosts[stationId].toCharArray(buf, 100);
  if (client.connect(buf, ports[stationId])) {
    Serial.println("Connected to radio station!");
  }
  client.print(String("GET ") + paths[stationId] + " HTTP/1.1\r\n" +
               "Host: " + hosts[stationId] + "\r\n" +
               "Connection: close\r\n\r\n");
  drawRadioStationName(stationId);
  connectedToStation = true;
}

void connectToWIFI() {
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void drawRadioStationName(int id) {
  Serial.println(hosts[id]);
}

int getLastStationFromEEPROM() {
  byte byte1 = EEPROM.read(0);
  byte byte2 = EEPROM.read(1);
  int id = (byte1 << 8) + byte2;
  Serial.println("Got station ID from EEPROM: " + String(id));
  return id;
}

void writeLastStationToEEPROM(int id) {
  byte byte1 = id >> 8;
  byte byte2 = id & 0xFF;
  EEPROM.write(0, byte1);
  EEPROM.write(1, byte2);
  EEPROM.commit();
  Serial.println("Wrote station ID to EEPROM: " + String(radioStation));
}
