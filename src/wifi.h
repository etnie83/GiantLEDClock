// ==========WIFI INIT==============
void wifiInit(void) {
#if DEBUG
  Serial.println(F("Start Wifi"));
#endif
 // Wait for connection
 WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    //WiFi.begin(SSID, PASSWORD);  
    Serial.println("");
#if DEBUG
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif
}