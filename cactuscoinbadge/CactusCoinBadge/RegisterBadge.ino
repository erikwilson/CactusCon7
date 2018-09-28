/*
 * Obtains the name from the user using the dpad if this badge has never been registered, 
 * then registers the supplied name for this badge on a cactuscoin node over wifi.
 */
void registerBadge(){
  // turn on wifi
  // hit badge endpoint on cactuscoin node to determine if this badge already has a registered name with it (if so bail and/or give user option to update name).
  // ask the user for a name and register with the cactuscoin node.
  int i = 0;
  int status;
  char tmp;
  char CCAPIMessage[MAX_JSON_SIZE];
  char json[MAX_JSON_SIZE];  
  char url[100];
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
      return;
    }
  }

  //myName = getNameViaDPAD(); // TODO ERIK: replace with call to do dpad and return users name here (should be no larger than MAX_NAME_LENGTH)
  sprintf(myName, "cybaix%d", myBadgeID);
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

  File f = SPIFFS.open("/my.name", "w");

  if (!f) 
    Serial.println(F("Unable to save name on flash.  Will attempt again next reboot."));

  f.print(myName);
  f.close();
}
