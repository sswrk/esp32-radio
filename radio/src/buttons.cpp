#include "buttons.h"


/*----------------------------------
         BUTTONS SETTINGS
----------------------------------*/
extern long buttonIntervalMillis = 250;
extern long previousButtonMillis = 0;


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