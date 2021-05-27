/*
    Edited (C) 2020 André Geiger Giant LED Clock
    v0.1 14.09.2020
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "userconfig.h"
#include <Arduino.h>
#include "wifi.h"
#include <Adafruit_NeoPixel.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// DEFINITIONS
// ===============================
Adafruit_NeoPixel led_segments(LED_COUNT, PIN_SEGMENTS, NEO_GRB + NEO_KHZ800);
WiFiUDP ntpUDP;
int secondblink = 0;
int PpSeg = 9;
int number0[] = {1,1,1,0,1,1,1};
int number1[] = {1,0,0,0,1,0,0};
int number2[] = {1,1,0,1,0,1,1};
int number3[] = {1,1,0,1,1,1,0};
int number4[] = {1,0,1,1,1,0,0};
int number5[] = {0,1,1,1,1,1,0};
int number6[] = {0,1,1,1,1,1,1};
int number7[] = {1,1,0,0,1,0,0};
int number8[] = {1,1,1,1,1,1,1};
int number9[] = {1,1,1,1,1,1,0};
int SegmentOffset[] = {0,9,27,36,45,54,72};
int SegmentOffsetLastSegment[] = {0,9,18,27,36,45,54};
unsigned long timer1 = 0;
unsigned long timer2 = 0;
unsigned long timer3 = 0;
int hours;
int hour1;
int hour2;
int minutes;
int minutes1;
int minutes2;
int seconds;
int seconds1;
int seconds2;
int color1_r;
int color1_g;
int color1_b;
int color2_r = 0;
int color2_g = 0;
int color2_b = 255;
int offset = 7200;
char msg[50];
int alarm_set;
int previous_alarm_set = 0;
int alarm_time = 0;
long now;
int soc = 0;
int percent = 0;

// ===============================
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", offset, 60000);

const char* mqtt_server = BROKERIP;

void callback(char* topic, byte* payload, int length)
 {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    }
  Serial.println();
  if (strcmp(topic,"giantclock/color1/r")==0)
    {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    color1_r = atoi((char *)payload);
    }
  if (strcmp(topic,"giantclock/color1/g")==0)
    {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    color1_g = atoi((char *)payload);
    }
  if (strcmp(topic,"giantclock/color1/b")==0)
    {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    color1_b = atoi((char *)payload);
    }
  if (strcmp(topic,"giantclock/alarm/set")==0)
    {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    alarm_set = atoi((char *)payload);
    }
  if (strcmp(topic,"giantclock/alarm/time")==0)
    {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    alarm_time = atoi((char *)payload);
    }
  if (strcmp(topic,"solar/accu/soc")==0)
    {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    soc = atoi((char *)payload);
    }
  if (strcmp(topic,"solar/accu/percent")==0)
    {
    payload[length] = '\0'; // Make payload a string by NULL terminating it.
    percent = atoi((char *)payload);
    }
  }

WiFiClient espClient;
PubSubClient client(mqtt_server, 1883, callback, espClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
  if (client.connect("giantclock", "openhab", BROKERPASSWORD)) 
    {
    client.subscribe("giantclock/#");
    client.subscribe("solar/accu/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(void) {
  Serial.begin(115200);
    // Wifi initialize
  wifiInit();
  
// START LED
// ===============================
  led_segments.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  led_segments.show();            // Turn OFF all pixels ASAP
  led_segments.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
// ===============================

  if (client.connect("giantclock", "openhab", BROKERPASSWORD)) 
    {
    client.subscribe("giantclock/#");
    client.subscribe("solar/accu/#");
    }

  ArduinoOTA.setHostname("giantLED");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  timeClient.begin();
}

void loop(void) {
  now = millis();
  if (!client.connected()) {
    reconnect();
  }
    client.loop();

  ArduinoOTA.handle();
  
// Refresh Time
// ===============================
 while(millis() >= timer1 + 100)
  {
  timeClient.update();
//    Serial.print("Actual time: ");
//    Serial.println(timeClient.getFormattedTime());
  hours = timeClient.getHours();
    hour1 = hours / 10;                   // left position hour
    hour2 = hours - (hour1 *10);          // right position hour
  minutes = timeClient.getMinutes();
    minutes1 = minutes / 10;              // left position minute
    minutes2 = minutes - (minutes1 *10);  // right position minute
  seconds = timeClient.getSeconds();
    seconds1 = seconds / 10;              // left position second
    seconds2 = seconds - (seconds1 *10);  // right position second
  timer1 = millis();
  }
// ===============================

// ===============================
// Seconds Blink
while(millis() >= timer2 + 1000)
  { 
  if (secondblink == 0)  
  {
  led_segments.setPixelColor(175, 50, 0, 0);
  led_segments.show();
  secondblink = 1;
  }
  else
  {
  led_segments.setPixelColor(175, 0, 0, 0);
  led_segments.show();
  secondblink = 0;
  }
  timer2 = millis();
  }

// ===============================

  setSegment(0, minutes2, color1_r, color1_g, color1_b);
  setSegment(90,minutes1, color1_r, color1_g, color1_b);
  setSegment(180,hour2, color1_r, color1_g, color1_b);
  setSegment2(270,hour1, color1_r, color1_g, color1_b);
 
soc_led(soc, percent);

if (alarm_set == 1 && (previous_alarm_set != alarm_set)) 
  {
    alarm(alarm_time);
  }
previous_alarm_set = alarm_set;
}

// ============================================================================================================================
// ============================================================================================================================
// Set Segments
// ===============================
void setSegment (int a, int number, int r, int g, int b)
{
   switch (number)
  {
    case 0:
      for(int y = 0; y<7; y += 1) 
      {
        for(int x = 0; x<PpSeg; x += 1)
        {
          
          if (number0[y] == 1)
          {
              led_segments.setPixelColor(SegmentOffset[y]+x+a, r, g, b);
          }
          if (number0[y] == 0)
          {
              led_segments.setPixelColor(SegmentOffset[y]+x+a, 0, 0, 0);
          }
        }
      }
      led_segments.show();
      break;
    case 1:
      for(int y = 0; y<7; y += 1) 
      {
        for(int x = 0; x<PpSeg; x += 1) 
        {
          
          if (number1[y] == 1)
          {
              led_segments.setPixelColor(SegmentOffset[y]+x+a, r, g, b);
          }
          if (number1[y] == 0)
          {
              led_segments.setPixelColor(SegmentOffset[y]+x+a, 0, 0, 0);
          }
        }
      }
      led_segments.show();
      break;
    case 2:
      for(int y = 0; y<7; y += 1) 
      {
        for(int x = 0; x<PpSeg; x += 1) 
        {
      
          if (number2[y] == 1)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, r, g, b);
          }
          if (number2[y] == 0)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, 0, 0, 0);
          }
        }
      }
      led_segments.show();
      break;
    case 3:
      for(int y = 0; y<7; y += 1) 
      {
        for(int x = 0; x<PpSeg; x += 1) 
        {
      
          if (number3[y] == 1)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, r, g, b);
          }
          if (number3[y] == 0)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, 0, 0, 0);
          }
        }
      }
      led_segments.show();
      break;
    case 4:
      for(int y = 0; y<7; y += 1) 
      {
        for(int x = 0; x<PpSeg; x += 1) 
        {
      
          if (number4[y] == 1)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, r, g, b);
          }
          if (number4[y] == 0)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, 0, 0, 0);
          }
        }
      }
      led_segments.show();
      break;
    case 5:
      for(int y = 0; y<7; y += 1) 
      {
        for(int x = 0; x<PpSeg; x += 1) 
        {
      
          if (number5[y] == 1)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, r, g, b);
          }
          if (number5[y] == 0)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, 0, 0, 0);
          }
        }
      }
      led_segments.show();
      break;
    case 6:
      for(int y = 0; y<7; y += 1) 
      {
        for(int x = 0; x<PpSeg; x += 1) 
        {
      
          if (number6[y] == 1)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, r, g, b);
          }
          if (number6[y] == 0)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, 0, 0, 0);
          }
        }
      }
      led_segments.show();
      break;
    case 7:
      for(int y = 0; y<7; y += 1) 
      {
        for(int x = 0; x<PpSeg; x += 1) 
        {
          if (number7[y] == 1)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, r, g, b);
          }
          if (number7[y] == 0)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, 0, 0, 0);
          }
        }
      }
      led_segments.show();
      break;
    case 8:
      for(int y = 0; y<7; y += 1) 
      {
        for(int x = 0; x<PpSeg; x += 1) 
        {
          if (number8[y] == 1)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, r, g, b);
          }
          if (number8[y] == 0)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, 0, 0, 0);
          }
        }
      }
      led_segments.show();
      break;
    case 9:
      for(int y = 0; y<7; y += 1) 
      {
        for(int x = 0; x<PpSeg; x += 1) 
        {
          if (number9[y] == 1)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, r, g, b);
          }
          if (number9[y] == 0)
          {
          led_segments.setPixelColor(SegmentOffset[y]+x+a, 0, 0, 0);
          }
        }
      }
      led_segments.show();
      break;
  }
}


// ============================================================================================================================
// ============================================================================================================================
// Set Segments Last
// ===============================
void setSegment2 (int a, int number, int r, int g, int b)
{
   switch (number)
  {
    case 0:
      for(int y = 0; y<7; y += 1) 
      {
        for(int x = 0; x<PpSeg; x += 1)
        {
          
          if (number0[y] == 1)
          {
              led_segments.setPixelColor(SegmentOffsetLastSegment[y]+x+a, r, g, b);
          }
          if (number0[y] == 0)
          {
              led_segments.setPixelColor(SegmentOffsetLastSegment[y]+x+a, 0, 0, 0);
          }
        }
      }
      led_segments.show();
      break;
    case 1:
      for(int y = 0; y<7; y += 1) 
      {
        for(int x = 0; x<PpSeg; x += 1) 
        {
          
          if (number1[y] == 1)
          {
              led_segments.setPixelColor(SegmentOffsetLastSegment[y]+x+a, r, g, b);
          }
          if (number1[y] == 0)
          {
              led_segments.setPixelColor(SegmentOffsetLastSegment[y]+x+a, 0, 0, 0);
          }
        }
      }
      led_segments.show();
      break;
    case 2:
      for(int y = 0; y<7; y += 1) 
      {
        for(int x = 0; x<PpSeg; x += 1) 
        {
      
          if (number2[y] == 1)
          {
          led_segments.setPixelColor(SegmentOffsetLastSegment[y]+x+a, r, g, b);
          }
          if (number2[y] == 0)
          {
          led_segments.setPixelColor(SegmentOffsetLastSegment[y]+x+a, 0, 0, 0);
          }
        }
      }
      led_segments.show();
      break;
      default:
    // Statement(s)
    break; // Wird nicht benötigt, wenn Statement(s) vorhanden sind
  }
}

void alarm(int time)
{
  Serial.println("Alarmmodus");

  long previousMillis;
  previousMillis = now;
  if (time == 0 || time > 60) { time = 5; }
  
  while(now <= previousMillis + (time * 1000))
  {
    now = millis();
    for(int i=0;i<14;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    led_segments.setPixelColor(i+333, led_segments.Color(255,0,0)); // Moderately bright green color.
    } 
    led_segments.show(); // This sends the updated pixel color to the hardware.
    delay(50); // Delay for a period of time (in milliseconds). 
    for(int i=0;i<14;i++){
    led_segments.setPixelColor(i+333, led_segments.Color(0,0,0)); // Moderately bright green color.
    } 
    led_segments.show(); // This sends the updated pixel color to the hardware.
    delay(30); // Delay for a period of time (in milliseconds).     
    for(int i=0;i<14;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    led_segments.setPixelColor(i+333, led_segments.Color(255,0,0)); // Moderately bright green color.
    } 
    led_segments.show(); // This sends the updated pixel color to the hardware.
    delay(50); // Delay for a period of time (in milliseconds). 
    for(int i=0;i<14;i++){
    led_segments.setPixelColor(i+333, led_segments.Color(0,0,0)); // Moderately bright green color.
    }
    led_segments.show(); // This sends the updated pixel color to the hardware.
    delay(250); // Delay for a period of time (in milliseconds).   
  }
alarm_set = 0;
}

void soc_led(int state, int state_percent)
{  
  while(millis() >= timer3 + 1000)
  {
    if (state_percent >= 100 && state == 4)
      {
        for(int i=0;i<2;i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        led_segments.setPixelColor(i+108, led_segments.Color(0,255,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        for(int i=0;i<2;i++){
        led_segments.setPixelColor(i+171, led_segments.Color(0,255,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        for(int i=0;i<2;i++){
        led_segments.setPixelColor(i+153, led_segments.Color(0,255,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        delay(800);
        for(int i=0;i<2;i++){
        led_segments.setPixelColor(i+108, led_segments.Color(0,0,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
      }
    else if (state == 3)
      {
        for(int i=0;i<2;i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        led_segments.setPixelColor(i+108, led_segments.Color(255,0,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        delay(300);
        for(int i=0;i<2;i++){
        led_segments.setPixelColor(i+171, led_segments.Color(255,0,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        delay(300);
        for(int i=0;i<2;i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        led_segments.setPixelColor(i+153, led_segments.Color(255,0,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        delay(300);
            for(int i=0;i<2;i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        led_segments.setPixelColor(i+108, led_segments.Color(0,0,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        delay(300);
        for(int i=0;i<2;i++){
        led_segments.setPixelColor(i+171, led_segments.Color(0,0,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        delay(300);
        for(int i=0;i<2;i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        led_segments.setPixelColor(i+153, led_segments.Color(0,0,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        delay(300);
      }
    else if (state == 4)
      {
        for(int i=0;i<2;i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        led_segments.setPixelColor(i+153, led_segments.Color(0,255,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        delay(300);
        for(int i=0;i<2;i++){
        led_segments.setPixelColor(i+171, led_segments.Color(0,255,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        delay(300);
        for(int i=0;i<2;i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        led_segments.setPixelColor(i+108, led_segments.Color(0,255,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        delay(300);
            for(int i=0;i<2;i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        led_segments.setPixelColor(i+153, led_segments.Color(0,0,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        delay(300);
        for(int i=0;i<2;i++){
        led_segments.setPixelColor(i+171, led_segments.Color(0,0,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        delay(300);
        for(int i=0;i<2;i++){
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        led_segments.setPixelColor(i+108, led_segments.Color(0,0,0));
        } 
        led_segments.show(); // This sends the updated pixel color to the hardware.
        delay(300);
      }
  timer3 = millis();
  }
}