#include <HTTPClient.h>
#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <SPI.h>

#include "src/audio.h"
#include "src/eeprom.h"
#include "src/radio.h"
#include "src/buttons.h"


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
  
  readStationsFromFile();

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
      RadioStationInfo currentStation = availableStations.at(i);
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
  uint8_t vol = getVolumeFromEEPROM();
  if(vol>=0 && vol<=200){
    volume = vol;
  }
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
    writeVolumeToEEPROM(volume);
  }

  int ssize = client.available();
  if (ssize > 0) {
    int c = client.readBytes(buff, ssize > sizeof(buff) ? sizeof(buff) : ssize);
    player.playChunk(buff, c);
  }
}
