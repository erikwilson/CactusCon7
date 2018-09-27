bool submitSignedCoinToAPI(const char *json) {
  char CCAPIMessage[MAX_JSON_SIZE * 2];  
  char url[100];
  int status;

  sprintf(url, "http://%s/coin/%d", CACTUSCOINAPI_IP, myBadgeID);
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

  // wtf why doesn arduino have a max arg for strlen?!?!
  signedJsonify((unsigned char*)json, strlen(json), CCAPIMessage, MAX_JSON_SIZE * 2);

  Serial.print(F("Submitting coin to API "));
  Serial.println(url);
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  status = http.POST(CCAPIMessage);
  WiFi.mode(WIFI_OFF);

  if (status != 200 && status != 409) {
    Serial.print(F("API returned failure status code of "));
    Serial.println(status);
    return false;
  }

  if (status != 409)
    coinCounter ++;
    
  return true;
}


bool signedJsonify(unsigned char *message, int messageLen, char *output, int outputSize) {
  StaticJsonBuffer<MAX_JSON_SIZE * 2> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();
  unsigned char signature[CDP_MODULUS_SIZE];
  unsigned char signatureB64[BASE64_MAX_SIZE];
  size_t signatureLen = 0;
  size_t hashLen = 0;
  
  sign(message, messageLen, signature, &signatureLen);
  mbedtls_base64_encode(signatureB64, BASE64_MAX_SIZE, &hashLen, signature, CDP_MODULUS_SIZE);
  
  root["sig"] = signatureB64;
  root["msg"] = message;
  root.printTo(output, outputSize);
}
