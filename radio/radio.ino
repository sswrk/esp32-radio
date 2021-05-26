#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "src/audio.h"
#include "src/eeprom.h"
#include "src/connection.h"
#include "src/radio.h"
#include "src/buttons.h"

void setup () {
  Serial.begin(115200);
  delay(500);

  EEPROM.begin(EEPROM_SIZE);
  initButtons();
  initSPIFFS();
  connectToWIFI();
  initServer();
  initAudio();
  setInitRadioStation();
}

void playChunk(){
  int ssize = client.available();
  if (ssize > 0) {
    int c = client.readBytes(buff, ssize > sizeof(buff) ? sizeof(buff) : ssize);
    player.playChunk(buff, c);
  }
}

void checkForStationChange() {
  if (radioStation != previousRadioStation || (connectedToStation == false && availableStations.size() > 0)) {
    connectToStation(radioStation);
    previousRadioStation = radioStation;
    writeLastStationToEEPROM(radioStation);
    notifyClients();
  }
}

void loop() {
  checkForStationChange();
  checkForVolumeChange();

  playChunk();
}
