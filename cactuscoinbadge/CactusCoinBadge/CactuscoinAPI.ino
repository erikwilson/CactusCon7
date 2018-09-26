bool submitSignedCoinToAPI(String json) {
  String CCAPIMessage;  
  byte messageBytes[MAX_JSON_SIZE];  // this should be computed and moved to a header, but short on time
  WiFi.mode(WIFI_STA);
  WiFi.begin(WLAN_SSID, WLAN_PASSWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  json.getBytes(messageBytes, MAX_JSON_SIZE);
  Serial.println((char *)messageBytes);
  Serial.println("I got some bytes in json, now to sign");
  CCAPIMessage = signedJsonify((byte *)"signme", 6);
  Serial.println(CCAPIMessage);

  /*
  HTTPClient http;
  http.begin("http://www.mywebpage.com/php_page.php");
  http.addHeader("Content-Type", "application/json");
  http.POST(root.);*/
  WiFi.mode(WIFI_OFF);

  return false;
}


String signedJsonify(byte *message, int messageLen) {
  //StaticJsonBuffer<MAX_JSON_SIZE> jsonBuffer;
  //JsonObject &root = jsonBuffer.createObject();
  unsigned char signature[MBEDTLS_MPI_MAX_SIZE];
  unsigned char signatureB64[1024];
  size_t signatureLen = 0;
  size_t hashLen = 0;
  String returnJson;
  
  Serial.print("signing, ");
  Serial.println((char *)message);
  sign(message, messageLen, signature, &signatureLen);
  Serial.println("hashing");
  mbedtls_base64_encode(signatureB64, 1024, &hashLen, signature, CDP_MODULUS_SIZE);
  /*
  root["sig"] = signatureB64;
  root["msg"] = message;
  root.printTo(returnJson);
  return returnJson;*/
}
