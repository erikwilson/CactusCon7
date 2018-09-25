bool setupFS() {
  if(!SPIFFS.begin(true)) {
    Serial.println("Failed to start SPIFFS!");
    return false;
  }
  
  File f = SPIFFS.open("/my.id", "r");
  
  if (!f) {
    Serial.println("Failed to open badge ID");
    return false;
  }
  else {
    myBadgeID = f.read() - '0';
  }

  return true;
}

bool ifCoinExistsOnFS(int badgeID) {
  char coinPath[255];
  sprintf(coinPath, "/coins/%d", badgeID);
  Serial.print("Checking if coin exists at ");
  Serial.println(coinPath);
  return SPIFFS.exists(coinPath);
}

bool storeUnsentSignedCoinOnFS(int badgeID, String json) {
  char coinPath[255];
  sprintf(coinPath, "/coins/%d", badgeID);
  
  Serial.print("Writing coin out to ");
  Serial.println(coinPath);
  File f = SPIFFS.open(coinPath, "w");
  
  if (!f) {
    Serial.print("Failed to open coin for writing on FS.. ");
    Serial.println(coinPath);
    return false;
  }

  if (f.print(json)) {
    f.close();
  } else {
    Serial.print("Failed to write coin file on FS.. ");
    Serial.println(coinPath);
    return false;
  }
  return true;
}

