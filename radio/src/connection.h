#ifndef connection_h
#define connection_h

#include "radio.h"
#include "audio.h"

#include <HTTPClient.h>

/*----------------------------------
           WIFI SETTINGS
----------------------------------*/
extern char ssid[];
extern char pass[];

extern int status;
extern WiFiClient  client;

/*----------------------------------
    RADIO CONNECTION FUNCTIONS
----------------------------------*/
void connectToStation(int stationId );
void connectToWIFI();

#endif