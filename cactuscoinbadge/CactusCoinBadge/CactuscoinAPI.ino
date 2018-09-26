bool submitSignedCoinToAPI(String json) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASSWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  /*
  HTTPClient http;
  http.begin("http://www.mywebpage.com/php_page.php");
  http.addHeader("Content-Type", "application/json");
  http.POST(root.);*/
  WiFi.mode(WIFI_OFF);

  return false;
}


