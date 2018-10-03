/*
 * Obtains the name from the user using the dpad if this badge has never been registered, 
 * then registers the supplied name for this badge on a cactuscoin node over wifi.
 */
bool registerBadge(){
  // turn on wifi
  // hit badge endpoint on cactuscoin node to determine if this badge already has a registered name with it (if so bail and/or give user option to update name).
  // ask the user for a name and register with the cactuscoin node.
  int i = 0;
  int status;
  char tmp;
  char CCAPIMessage[MAX_JSON_SIZE];
  char json[MAX_JSON_SIZE];  
  char url[100];
  char jsonResponse[MAX_JSON_SIZE];
  StaticJsonBuffer<MAX_JSON_SIZE> jsonBuffer;

  if (SPIFFS.exists("/my.name")) {
    Serial.println(F("Reading name from local file"));
    File f = SPIFFS.open("/my.name", "r");
  
    if (!f) 
      Serial.println(F("That is strange, I couldn't open my own name that exists.  Will attempt to re-register."));
    else {  
      while (f.available()) {
        tmp = f.read();
        myName[i] = tmp;
        i ++;
      }
      if (digitalRead(0)) {
        return true;
      }
    }
  }
 
  getNameViaDPAD(myName, MAX_NAME_LENGTH);

  display.clear();
  display.setFont(Roboto_Light_15);
  display.drawStringMaxWidth(0, 24, 128, "Registering...");
  display.display();

  JsonObject &root = jsonBuffer.createObject();
  root["name"] = myName;
  root.printTo(json, MAX_JSON_SIZE);
  signedJsonify((unsigned char*)json, strlen(json), CCAPIMessage, MAX_JSON_SIZE);

  sprintf(url, "http://%s/badge/%d", CACTUSCOINAPI_IP, myBadgeID);
  Serial.print(F("Registering name to API "));
  Serial.println(url);
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  status = http.POST(CCAPIMessage);
  
  if (!getSignedJSONMessage(http.getString().c_str(), jsonResponse, MAX_JSON_SIZE)) {
    Serial.println("ERROR: Failed to get message from API or validate signature");
    return false;
  }
  JsonObject &rootResponse = jsonBuffer.parseObject(jsonResponse);

  if (!rootResponse.containsKey("status")) {
    Serial.println(F("ERROR: Expected a status key from API, but it wasn't there"));
    return false;
  }
  status = rootResponse["status"];
  if (rootResponse["status"] != 200) {
    Serial.print(F("ERROR: API returned unexpected status of "));
    Serial.println(status);
    return false;
  }

  File f = SPIFFS.open("/my.name", "w");

  if (!f) 
    Serial.println(F("Unable to save name on flash.  Will attempt again next reboot."));

  f.print(myName);
  f.close();
  return true;
}
