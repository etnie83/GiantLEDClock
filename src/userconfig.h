  
//##############################################
// USERCONFIG
//##############################################

#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

// -- Localization --------------------------------
  // If non selected the default de-DE will be used
  // ATTENTION: The Definition of the GoogleCalendar Entrys must edited in the Localization-File for autodetect the Waste Bin
//#define MY_LANGUAGE            de-DE           // German in Germany
//#define MY_LANGUAGE            he-HE           // Platt in Hessisch_Platt
#define MY_LANGUAGE            en-GB           // English in GreatBritain
#define DEBUG                 1
// -- WLAN Settings -------------------------------
#define SSID                  "YOUR_SSID"
#define PASSWORD              "YOUR_PASSWORD_MAYBE"

#define NEOPIXEL
// -- Neopixel Definitions ------------------------
  #ifdef NEOPIXEL
    #include <Adafruit_NeoPixel.h>
    #define PIN_SEGMENTS  14
    #define LED_COUNT     347
  #endif
// ------------------------------------------------

#define BROKERIP              "YOUR_IP_BROKER"
#define BROKERPASSWORD        "YOUR_BROKER_PASSWORD"

#endif // _USER_CONFIG_H_