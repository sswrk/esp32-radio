#include <WiFi.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "SPIFFS.h"
#include "Vector.h"
#include <ESPVS1003.h>
#include <SPI.h>

#define EEPROM_SIZE 2


/*----------------------------------
           PIN MAPPINGS
----------------------------------*/
//buttons
const int previousButton = 13; //D13
const int nextButton = 15; //D15
const int addOrRemoveFavouriteButton = 4; //D4
const int switchFavouritesModeButton = 2; //D2
const int volumeUpButton = 25; //D25
const int volumeDownButton = 22; //D22
//VS1003
uint8_t csVS = 32; //D32
uint8_t dcsVS = 33; //D33
uint8_t dreqVS = 35; //D35
uint8_t rsetVS = 3; //EN
//uint8_t sckVS = 18; //D18
//uint8_t misoVS = 19; //D19
//uint8_t mosiVS = 23; //D23


/*----------------------------------
         BUTTONS SETTINGS
----------------------------------*/
long buttonIntervalMillis = 250;
long previousButtonMillis = 0;


/*----------------------------------
         RADIO DATA LOGIC
----------------------------------*/
struct radioStationInfo{
  int id;
  String host;
  String path;
  String name;
  int port;
  bool isFavourite;
};

int radioStation = 0;
bool connectedToStation = false;
int previousRadioStation = -1;
bool paused = false;
bool favouritesMode = false;

radioStationInfo availableStationsArray[100];
bool takenIds[100] = {false};
Vector<radioStationInfo> availableStations(availableStationsArray);

long nextId = 0;


/*----------------------------------
          VS1003 SETTINGS
----------------------------------*/
uint8_t buff[32];
VS1003 player(csVS, dcsVS, dreqVS, rsetVS);
uint8_t volume = 100;
uint8_t previousVolume = 100;


/*----------------------------------
           WIFI SETTINGS
----------------------------------*/
char ssid[] = "<SSID>";
char pass[] = "<PASS>";


/*----------------------------------
        WEB SERVER SETTINGS
----------------------------------*/
AsyncWebServer server(80);

int status = WL_IDLE_STATUS;
WiFiClient  client;

void notFound(AsyncWebServerRequest *request){
  request->send(404, "application/json", "{\"message\":\"Not found\"}");
}


/*----------------------------------
       RADIO LOGIC FUNCTIONS
----------------------------------*/
void updateNextId(){
  if(availableStations.size()==100){
    nextId = NULL;
    return;
  }
  for(int i=0; i<100; i++){
    if(takenIds[i] == false){
      nextId = i;
      return;
    }
  }
}

bool addRadioStation(const char* host, const char* path, const char* name, int port){
  if(host==NULL || path==NULL || port==NULL || name==NULL || availableStations.full()){
    return false;
  }
  radioStationInfo newStation;
  newStation.id = nextId;
  takenIds[nextId] = true;
  newStation.host = String(host);
  newStation.path = String(path);
  newStation.port = port;
  newStation.isFavourite = false;
  newStation.name = String(name);
  availableStations.push_back(newStation);
  
  Serial.println("Added radio station!");
  updateNextId();
  return true;
}

bool deleteRadioStation(int id){
  int i = 0;
  while(i<availableStations.size() && availableStations.at(i).id != id){
    i++;
  }
  if(i==availableStations.size()){
    return false;
  }
  availableStations.remove(i);
  if(radioStation == i){
    if(availableStations.size() > 0 && availableStations.size() != i+1){
      connectToStation(radioStation);
      previousRadioStation = radioStation;
      writeLastStationToEEPROM(radioStation);
    }
    else if(availableStations.size() > 0){
      radioStation--;
    }
  }
  Serial.println("Removed radio station! ");
  takenIds[id] = false;
  return true;
}

bool addToFavourites(int id){
  int i=0;
  while(i<availableStations.size() && availableStations.at(i).id != id){
    i++;
  }
  if(i==availableStations.size()){
    return false;
  }
  availableStations.at(i).isFavourite = true;
  Serial.println("Added station to favourites!");
  return true;
}

bool removeFromFavourites(int id){
  int i=0;
  while(i<availableStations.size() && availableStations.at(i).id != id){
    i++;
  }
  if(i==availableStations.size()){
    return false;
  }
  availableStations.at(i).isFavourite = false;
  Serial.println("Removed station from favourites!");
  return true;
}

bool switchFavourites(int id){
  int i=0;
  while(i<availableStations.size() && availableStations.at(i).id != id){
    i++;
  }
  if(i==availableStations.size()){
    return false;
  }
  availableStations.at(i).isFavourite = !availableStations.at(i).isFavourite;
  if(availableStations.at(i).isFavourite) {
    Serial.println("Added station to favourites!");
  }
  else {
    Serial.println("Removed station from favourites!");
  }
  return true;
}

bool setStation(int id) {
  int i=0;
  while(i<availableStations.size() && availableStations.at(i).id != id){
    i++;
  }
  if(i==availableStations.size()){
    return false;
  }
  radioStation = i;
  return true;
}

void toggleVolume() {
  if(paused==true) {
    player.setVolume(volume);
    Serial.println("Playing");
  }
  else {
    player.setVolume(0);
    paused = true;
    Serial.println("Muted");
  }
}

void volumeDown() {
  if(volume<=190){
    volume += 10;
  }
  if(volume>190 && volume<200) {
    volume = 200;
  }
  volume += 10;
  Serial.print("Volume: ");
  Serial.print(volume);
  Serial.println();
}

void volumeUp() {
  if(volume>=10) {
    volume -= 10;
  }
  if(volume>0 && volume<10){
    volume = 0;
  }
  Serial.print("Volume: ");
  Serial.print(volume);
  Serial.println();
}

void switchFavouritesMode() {
  if(favouritesMode){
    favouritesMode = false;
    Serial.println("Turned favourites mode off");
    return;
  }
  int i=0;
  while(i<availableStations.size() && availableStations.at(i).isFavourite==false){
    i++;
  }
  if(i!=availableStations.size()) {
    radioStation = i;
  }
  favouritesMode = true;
  Serial.println("Turned favourites mode on");
}

void switchToNextStation() {
  if(favouritesMode){
    int i = radioStation + 1;
    while(i!=radioStation && availableStations.at(i).isFavourite!=true){
      if(i+1>=availableStations.size()){
        i = 0;
      }
      else {
        i++;
      }
    }
    radioStation = i;
  }
  else {
    if (radioStation < availableStations.size() - 1)
      radioStation++;
    else
      radioStation = 0;
  }
  Serial.println("Switched to next station");
}

void switchToPreviousStation(){
  if(favouritesMode){
    int i = radioStation - 1;
    while(i!=radioStation && availableStations.at(i).isFavourite!=true){
      if(i<=0){
        i = availableStations.size() - 1;
      }
      else {
        i--;
      }
    }
    radioStation = i;
  }
  else {
    if (radioStation > 0)
      radioStation--;
    else
      radioStation = availableStations.size() - 1;
  }
  Serial.println("Switched to previous station");
}

void drawRadioStationName(int id) {
  Serial.println(availableStations.at(id).host);
}

/*----------------------------------
         BUTTON FUNCTIONS
----------------------------------*/
void IRAM_ATTR previousButtonInterrupt() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousButtonMillis > buttonIntervalMillis) {
    Serial.println("PREVIOUS BUTTON");
    switchToPreviousStation();

    previousButtonMillis = currentMillis;
  }
}

void IRAM_ATTR nextButtonInterrupt() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousButtonMillis > buttonIntervalMillis) {
    Serial.println("NEXT BUTTON");
    switchToNextStation();

    previousButtonMillis = currentMillis;
  }
}

void IRAM_ATTR addOrRemoveFavouriteButtonInterrupt() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousButtonMillis > buttonIntervalMillis) {
    Serial.println("ADD OR REMOVE FAV BUTTON");
    switchFavourites(radioStation);

    previousButtonMillis = currentMillis;
  }
}

void IRAM_ATTR volumeUpButtonInterrupt() {
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousButtonMillis > buttonIntervalMillis) {
    Serial.println("VOLUME UP BUTTON");
    volumeUp();

    previousButtonMillis = currentMillis;
    
  }
}

void IRAM_ATTR volumeDownButtonInterrupt() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousButtonMillis > buttonIntervalMillis) {
    Serial.println("VOLUME DOWN BUTTON");
    volumeDown();

    previousButtonMillis = currentMillis;
    
  }
}

void IRAM_ATTR switchFavouritesModeButtonInterrupt() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousButtonMillis > buttonIntervalMillis) {
    Serial.println("SWITCH FAVOURITES BUTTON");
    switchFavouritesMode();

    previousButtonMillis = currentMillis;
    
  }
  
}

/*----------------------------------
    RADIO CONNECTION FUNCTIONS
----------------------------------*/
void connectToStation(int stationId ) {
  Serial.println("Connecting to radio station...");
  char buf[100];
  availableStations.at(stationId).host.toCharArray(buf, 100);
  if (client.connect(buf, availableStations.at(stationId).port)) {
    Serial.println("Connected to radio station!");
  }
  client.print(String("GET ") + availableStations.at(stationId).path + " HTTP/1.1\r\n" +
               "Host: " + availableStations.at(stationId).host + "\r\n" +
               "Connection: close\r\n\r\n");
  drawRadioStationName(stationId);
  connectedToStation = true;
  player.startSong();
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


/*----------------------------------
         EEPROM FUNCTIONS
----------------------------------*/
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


/*----------------------------------
               SETUP
----------------------------------*/
void setup () {

  Serial.begin(115200);
  delay(500);


  //EEPROM init
  EEPROM.begin(EEPROM_SIZE);


  //BUTTONS init
  pinMode(previousButton, INPUT_PULLUP);
  pinMode(nextButton, INPUT_PULLUP);
  pinMode(addOrRemoveFavouriteButton, INPUT_PULLUP);
  pinMode(volumeUpButton, INPUT_PULLUP);
  pinMode(volumeDownButton, INPUT_PULLUP);
  pinMode(switchFavouritesModeButton, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(previousButton), previousButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(nextButton), nextButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(addOrRemoveFavouriteButton), addOrRemoveFavouriteButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(volumeUpButton), volumeUpButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(volumeDownButton), volumeDownButtonInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(switchFavouritesModeButton), switchFavouritesModeButtonInterrupt, RISING);

  
  //SPIFFS init
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //WIFI init
  connectToWIFI();

  
  //SERVER requests handling
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/styles.scss", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/styles.scss", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/script.js", "text/javascript");
  });
  server.on("/img/next.svg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/next.svg", "image/svg+xml");
  });
  server.on("/img/play.svg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/play.svg", "image/svg+xml");
  });
  server.on("/img/previous.svg", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/img/previous.svg", "image/svg+xml");
  });
  server.on("/stations", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument root(1024*100);
    for(int i=0; i<availableStations.size(); i++){
      radioStationInfo currentStation = availableStations.at(i);
      root["stations"][i]["id"] = currentStation.id;
      root["stations"][i]["name"] = currentStation.name;
      root["stations"][i]["host"] = currentStation.host;
      root["stations"][i]["path"] = currentStation.path;
      root["stations"][i]["port"] = currentStation.port;
      root["stations"][i]["isFavourite"] = currentStation.isFavourite;
    }
    serializeJson(root, *response);
    request->send(response);
  });
  server.on("/stations", HTTP_POST, [](AsyncWebServerRequest *request){
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, (const char*)data);
      bool success = false;
      if(!error) {
        const char* host = doc["host"];
        const char* path = doc["path"];
        const char* name = doc["name"];
        int port = doc["port"];
        success = addRadioStation(host, path, name, port);
      }
      if(success){
        request->send(200, "text/plain", "Radio station added");
      } else {
        request->send(400, "text/plain", "Wrong parameters");
      }
  });
  server.on("/stations", HTTP_DELETE, [](AsyncWebServerRequest *request){
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, (const char*)data);
      bool success = false;
      if(!error) {
        int id = doc["id"];
        success = deleteRadioStation(id);
      }
      if(success){
        request->send(200, "text/plain", "Radio station deleted");
      } else {
        request->send(400, "text/plain", "Wrong parameters");
      }
  });
  server.on("/favourites", HTTP_POST, [](AsyncWebServerRequest *request){
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, (const char*)data);
      bool success = false;
      if(!error) {
        int id = doc["id"];
        success = addToFavourites(id);
      }
      if(success){
        request->send(200, "text/plain", "Radio station added to favourited");
      } else {
        request->send(400, "text/plain", "Wrong parameters");
      }
  });
  server.on("/favourites", HTTP_DELETE, [](AsyncWebServerRequest *request){
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, (const char*)data);
      bool success = false;
      if(!error) {
        int id = doc["id"];
        success = removeFromFavourites(id);
      }
      if(success){
        request->send(200, "text/plain", "Radio station removed from favourites");
      } else {
        request->send(400, "text/plain", "Wrong parameters");
      }
  });
  server.on("/set-station", HTTP_POST, [](AsyncWebServerRequest *request){
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, (const char*)data);
      bool success = false;
      if(!error) {
        int id = doc["id"];
        success = setStation(id);
      }
      if(success){
        request->send(200, "text/plain", "Radio station set");
      } else {
        request->send(400, "text/plain", "Wrong parameters");
      }
  });
  server.on("/toggle-play", HTTP_GET, [](AsyncWebServerRequest *request){
    toggleVolume();
    request->send(200, "text/plain", "Toggled play");
  });
  server.on("/louder", HTTP_GET, [](AsyncWebServerRequest *request){
    volumeUp();
    request->send(200, "text/plain", "Volume +10");
  });
  server.on("/more-quiet", HTTP_GET, [](AsyncWebServerRequest *request){
    volumeDown();
    request->send(200, "text/plain", "Volume -10");
  });
  server.onNotFound(notFound);


  //SERVER init
  server.begin();
  Serial.println("Server started!");


  //VS1003 init
  SPI.begin();
  //SPI.begin(sckVS, misoVS, mosiVS);
  Serial.println("SPI set up");
  player.begin();
  player.setVolume(volume);
  Serial.println("VS1003 set up");


  //RADIO init
  radioStation = getLastStationFromEEPROM();
  if (radioStation < 0 || radioStation >= availableStations.size()) {
    radioStation = 0;
  }
  previousRadioStation = radioStation;
}

void loop() {

  if (radioStation != previousRadioStation || (connectedToStation==false && availableStations.size()>0)) {
    connectToStation(radioStation);
    previousRadioStation = radioStation;
    writeLastStationToEEPROM(radioStation);
  }
  if (volume != previousVolume) {
    player.setVolume(volume);
    previousVolume = volume;
  }

  int ssize = client.available();
  if (ssize > 0) {
    int c = client.readBytes(buff, ssize > sizeof(buff) ? sizeof(buff) : ssize);
    player.playChunk(buff, c);
  }
}
