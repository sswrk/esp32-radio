#ifndef radio_h
#define radio_h

#include <Arduino.h>
#include <ArduinoNvs.h>
#include <Vector.h>
#include <SPIFFS.h>

#include "eeprom.h"
#include "audio.h"

/*----------------------------------
         RADIO DATA LOGIC
----------------------------------*/
struct RadioStationInfo{
  int id;
  String host;
  String path;
  String name;
  int port;
  bool isFavourite;
};

extern int radioStation;
extern bool connectedToStation;
extern int previousRadioStation;
extern bool paused;
extern bool favouritesMode;

extern RadioStationInfo availableStationsArray[100];
extern bool takenIds[100];
extern Vector<RadioStationInfo> availableStations;

extern uint8_t volume;
extern uint8_t previousVolume;

extern long nextId;

extern String filePath;

/*----------------------------------
       RADIO LOGIC FUNCTIONS
----------------------------------*/
void updateNextId();
bool addRadioStation(const char* host, const char* path, const char* name, int port);
bool deleteRadioStation(int id);
bool addToFavourites(int id);
bool removeFromFavourites(int id);
bool switchFavourites(int id);
bool setStation(int id);
void toggleVolume();
void volumeDown();
void volumeUp();
void switchFavouritesMode();
void switchToNextStation();
void switchToPreviousStation();
void drawRadioStationName(int id);

void addStationToFile(RadioStationInfo station);
void deleteStationFromFile(int id);
void addStationToFavouritesToFile(RadioStationInfo station);
void removeStationFromFavouritesFromFile(RadioStationInfo station);
void readStationsFromFile();
#endif