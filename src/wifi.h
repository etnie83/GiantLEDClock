// ==========WIFI INIT==============
void wifiInit(void) {
#if DEBUG
  Serial.println(F(D_WIFI_START));
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
  Serial.print(D_WIFI_CONNECTED_TO);
  Serial.println(SSID);
  Serial.print(D_WIFI_IPADRESS);
  Serial.println(WiFi.localIP());
#endif
  server.on("/", []() {
    server.send(200, "text/plain", "Hi! I am ESP8266.");
  });
  ElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
#if DEBUG
  Serial.println(D_WIFI_HTTPSTARTED);
#endif
}