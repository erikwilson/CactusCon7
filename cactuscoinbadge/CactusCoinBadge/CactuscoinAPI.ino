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
    Serial.print(F("ERROR: API returned failure status code of "));
    Serial.println(status);
    return false;
  }

  if (status != 409)
    coinCounter ++;
    
  return true;
}

bool refreshLocalCoinListFromAPI() {
  // run every so often to ensure local FS is in sync
  char CCAPIMessage[MAX_JSON_SIZE];
  char url[100];
  char jsonMessage[MAX_JSON_SIZE];
  StaticJsonBuffer<MAX_JSON_SIZE> jsonBuffer;
  int pos = 0;
  int badgeBatchSize = 10;
  int status;
  uint16_t coinedBadges[MAX_NUM_BADGES];

  sprintf(url, "http://%s/coin_list/%d", CACTUSCOINAPI_IP, myBadgeID);
  
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  Serial.printf("Pulling list of coined badges from API.. %s\r\r\n", url);
  while (pos < MAX_NUM_BADGES) {
    sprintf(jsonMessage, "{\"start\":%d, \"numBadges\":%d}", pos, badgeBatchSize);
    signedJsonify((unsigned char *)jsonMessage, strlen(jsonMessage), CCAPIMessage, MAX_JSON_SIZE);
    status = http.POST(CCAPIMessage);
    
    if (status != 200) {
      Serial.printf("WARN: Bad response (%d) from API when pulling coined badges.\r\r\n", status);
      break;
    }

    memset(jsonMessage, 0, sizeof(jsonMessage));
    getSignedJSONMessage(http.getString().c_str(), jsonMessage, MAX_JSON_SIZE);
    JsonArray& badgeArray = jsonBuffer.parseArray(jsonMessage);

    for (auto value : badgeArray) {
      storeCompletedCoinOnFS(value.as<uint16_t>());
      pos += 1;
    }

    if (badgeArray.size() != badgeBatchSize)
      break;
     
    memset(jsonMessage, 0, sizeof(jsonMessage));
  }

  coinCounter = pos;
}

bool getSignedJSONMessage(const char *httpResponse, char *results, int resultMaxSize) {
  unsigned char signature[CDP_MODULUS_SIZE];
  char signatureB64[BASE64_MAX_SIZE];
  StaticJsonBuffer<MAX_JSON_SIZE * 2> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(httpResponse);
  size_t sigLen = 0;
  int messageSize, signatureSize, ret;

  if (!root.containsKey("sig")) {
    Serial.println(F("ERROR: CactuscoinAPI response did not contain a signature"));
    return false;
  }

  if (!root.containsKey("msg")) {
    Serial.println(F("ERROR: CactuscoinAPI response did not contain a message"));
    return false;
  }

  messageSize = strlen(root["msg"]);  
  if (messageSize > resultMaxSize) // thou shall not overflow thine buffers
    messageSize = resultMaxSize;

  signatureSize = strlen(root["sig"]);
  if (signatureSize > BASE64_MAX_SIZE) // my buffer overflowith
    signatureSize = resultMaxSize;

  strncpy(signatureB64, root["sig"], signatureSize);
  strncpy(results, root["msg"], resultMaxSize);
  Serial.println(signatureB64);
  mbedtls_base64_decode(signature, CDP_MODULUS_SIZE, &sigLen, (byte *)signatureB64, BASE64_MAX_SIZE);
  return verify((byte *)results, messageSize, signature, sigLen);
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
