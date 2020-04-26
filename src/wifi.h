// ==========WIFI INIT==============
void wifiInit(void) {
#if DEBUG
  Serial.println(F("Start Wifi"));
#endif
 // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    WiFi.begin(SSID, PASSWORD);
    Serial.println("");
#if DEBUG
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif
  server.on("/", []() {
    server.send(200, "text/plain", "Hi! I am ESP8266.");
  });
  ElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
#if DEBUG
  Serial.println("Server started");
#endif
}