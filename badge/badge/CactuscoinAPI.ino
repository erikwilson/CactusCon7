bool submitSignedCoinToAPI(const char *json) {
  char CCAPIMessage[MAX_JSON_SIZE * 2];  
  char jsonResponse[MAX_JSON_SIZE];
  StaticJsonBuffer<MAX_JSON_SIZE> jsonBuffer;
  char url[100];
  int status;

  sprintf(url, "http://%s/coin/%d", CACTUSCOINAPI_IP, myBadgeID);

  // wtf why doesn arduino have a max arg for strlen?!?!
  signedJsonify((unsigned char*)json, strlen(json), CCAPIMessage, MAX_JSON_SIZE * 2);

  Serial.print(F("Submitting coin to API "));
  Serial.println(url);
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.POST(CCAPIMessage);
  
  if (!getSignedJSONMessage(http.getString().c_str(), jsonResponse, MAX_JSON_SIZE)) {
    Serial.println(F("ERROR: Failed to get a signed message back from the API."));
    return false;
  }
  JsonObject &root = jsonBuffer.parseObject(jsonResponse);

  if (!root.containsKey("status")) {
    Serial.println(F("ERROR: Expected a status key from API, but it wasn't there"));
    return false;
  }\
  
  if (!root.containsKey("balance")) {
    Serial.println(F("ERROR: Expected a balance key from API, but it wasn't there"));
    return false;
  }

  coinCounter = root["balance"];
  
  if (root["status"] != 200 && root["status"] != 409) {
    Serial.print(F("ERROR: API returned failure status code of "));
    Serial.println(status);
    return false;
  }
  
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
  int status = -1;
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
      Serial.printf("WARN: Bad response (%d)", status);
      Serial.println(" from API when pulling coined badges.");
      break;
    }

    memset(jsonMessage, 0, sizeof(jsonMessage));
    if (!getSignedJSONMessage(http.getString().c_str(), jsonMessage, MAX_JSON_SIZE)) {
      Serial.println(F("ERROR: Failed to get a signed message back from the API."));
      return false;
    }
    JsonArray& badgeArray = jsonBuffer.parseArray(jsonMessage);

    for (auto value : badgeArray) {
      storeCompletedCoinOnFS(value.as<uint16_t>());
      pos += 1;
    }

    if (badgeArray.size() != badgeBatchSize)
      break;
     
    memset(jsonMessage, 0, sizeof(jsonMessage));
  }

  if (status != -1)
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
  mbedtls_base64_decode(signature, CDP_MODULUS_SIZE, &sigLen, (byte *)signatureB64, signatureSize);
  return verify((byte *)results, messageSize, signature, CDP_MODULUS_SIZE);
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
