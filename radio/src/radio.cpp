#include "radio.h"

/*----------------------------------
         RADIO DATA LOGIC
----------------------------------*/
extern int radioStation = 0;
extern bool connectedToStation = false;
extern int previousRadioStation = -1;
extern bool paused = false;
extern bool favouritesMode = false;

RadioStationInfo availableStationsArray[100];
extern bool takenIds[100] = {false};
extern Vector<RadioStationInfo> availableStations(availableStationsArray);

extern uint8_t volume = 100;
extern uint8_t previousVolume = 100;

extern long nextId = 0;

extern String filePath = "/stations.txt";

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
  RadioStationInfo newStation;
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
  addStationToFile(newStation);
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
      previousRadioStation = -1;
    }
    else if(availableStations.size() > 0){
      radioStation--;
    }
  }
  Serial.println("Removed radio station! ");
  takenIds[id] = false;
  deleteStationFromFile(id);
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
  addStationToFavouritesToFile(availableStations.at(i));
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
  removeStationFromFavouritesFromFile(availableStations.at(i));
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
    addStationToFavouritesToFile(availableStations.at(i));
    Serial.println("Added station to favourites!");
  }
  else {
    removeStationFromFavouritesFromFile(availableStations.at(i));
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
    paused = false;
    Serial.println("Playing");
  }
  else {
    player.setVolume(200);
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

void addStationToFile(RadioStationInfo station) {
  String newLine = 
    station.name + "," +
    String(station.isFavourite) + "," +
    station.host + "," +
    station.path + "," +
    String(station.port);
  NVS.setString(String(station.id), newLine);
  Serial.println(NVS.getString(String(station.id)));
}

void deleteStationFromFile(int id) {
  NVS.setString(String(id), "");
}

void addStationToFavouritesToFile(RadioStationInfo station) {
  addStationToFile(station);
}

void removeStationFromFavouritesFromFile(RadioStationInfo station) {
  addStationToFile(station);
}

void readStationsFromFile() {
  for(int i=0; i<100; i++){
    String stationString = NVS.getString(String(i));
    Serial.println(stationString);
    if(stationString!=NULL && !stationString.equals("")){
      RadioStationInfo station;
      station.id = i;
      station.name = stationString.substring(0, stationString.indexOf(","));
      stationString.remove(0, stationString.indexOf(",") + 1);
      station.isFavourite = stationString.substring(0, stationString.indexOf(",")).toInt();
      stationString.remove(0, stationString.indexOf(",") + 1);
      station.host = stationString.substring(0, stationString.indexOf(","));
      stationString.remove(0, stationString.indexOf(",") + 1);
      station.path = stationString.substring(0, stationString.indexOf(","));
      stationString.remove(0, stationString.indexOf(",") + 1);
      station.port = stationString.toInt();
      availableStations.push_back(station);
      takenIds[i] = true;
    }
  }
  updateNextId();
}

void initSPIFFS(){
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  NVS.begin();
  readStationsFromFile();
}

void initAudio(){
  SPI.begin();
  //SPI.begin(sckVS, misoVS, mosiVS);
  Serial.println("SPI set up");
  player.begin();
  uint8_t vol = getVolumeFromEEPROM();
  if (vol >= 0 && vol <= 200) {
    volume = vol;
  }
  player.setVolume(volume);
  Serial.println("VS1003 set up");
}

void checkForVolumeChange() {
  if (volume != previousVolume) {
    player.setVolume(volume);
    previousVolume = volume;
    writeVolumeToEEPROM(volume);
  }
}

void setInitRadioStation(){
  radioStation = getLastStationFromEEPROM();
  if (radioStation < 0 || radioStation >= availableStations.size()) {
    radioStation = 0;
  }
  previousRadioStation = radioStation;
}