bool turnWiFiOnAndConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASSWD);
  Serial.print(F("Bringing up wifi please wait"));
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  IPAddress ip = WiFi.localIP();
  Serial.println(F("DONE"));
  Serial.print(F("My IP address:"));
  Serial.println(ip);
}

bool turnWiFiOff() {
  WiFi.mode(WIFI_OFF);
}
