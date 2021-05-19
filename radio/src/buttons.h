#ifndef buttons_h
#define buttons_h

#include <Arduino.h>
#include "radio.h"

/*----------------------------------
           PIN MAPPINGS
----------------------------------*/
const int previousButton = 13; //D13
const int nextButton = 15; //D15
const int addOrRemoveFavouriteButton = 4; //D4
const int switchFavouritesModeButton = 2; //D2
const int volumeUpButton = 25; //D25
const int volumeDownButton = 22; //D22

/*----------------------------------
         BUTTONS SETTINGS
----------------------------------*/
extern long buttonIntervalMillis;
extern long previousButtonMillis;

/*----------------------------------
         BUTTON FUNCTIONS
----------------------------------*/
void IRAM_ATTR previousButtonInterrupt();
void IRAM_ATTR nextButtonInterrupt();
void IRAM_ATTR addOrRemoveFavouriteButtonInterrupt();
void IRAM_ATTR volumeUpButtonInterrupt();
void IRAM_ATTR volumeDownButtonInterrupt();
void IRAM_ATTR switchFavouritesModeButtonInterrupt();

void initButtons();
#endif