bool setupFS() {
  char idBuf[MAX_NAME_LENGTH];
  int i = 0;
  if(!SPIFFS.begin(true)) {
    Serial.println(F("Failed to start SPIFFS!"));
    return false;
  }
  
  File f = SPIFFS.open("/my.id", "r");
  
  if (!f) {
    Serial.println(F("Failed to open badge ID"));
    return false;
  }
  else {
    while (f.available() && i < MAX_NAME_LENGTH) {
      idBuf[i] = f.read();
      i++;
    }
    Serial.print(F("My badge ID: "));
    Serial.println(idBuf);
    myBadgeID = (uint16_t)atoi(idBuf);
  }

  return true;
}

bool ifCoinExistsOnFS(uint16_t badgeID) {
  char coinPath[255];
  sprintf(coinPath, "/coins/%d", badgeID);
  Serial.print(F("Checking if coin exists at "));
  Serial.println(coinPath);
  return SPIFFS.exists(coinPath);
}

bool storeUnsentSignedCoinOnFS(int otherBadgeID, String json) {
  char coinPath[255];
  sprintf(coinPath, "/coins/%d", otherBadgeID);

  if (SPIFFS.exists(coinPath)) {
    Serial.print(F("Skipping this write, already got a coin at.."));
    Serial.println(coinPath);
    return true;
  }
  
  Serial.print(F("Writing coin out to "));
  Serial.println(coinPath);
  File f = SPIFFS.open(coinPath, "w");
  
  if (!f) {
    Serial.print(F("Failed to open coin for writing on FS.. "));
    Serial.println(coinPath);
    return false;
  }

  if (f.print(json)) {
    f.close();
  } else {
    Serial.print(F("Failed to write coin file on FS.. "));
    Serial.println(coinPath);
    return false;
  }

  if (!appendSignedCoinOnPendingTXLogOnFS(otherBadgeID)) {
    Serial.print(F("Failed to append coin to pending txlog, rolling back."));
    Serial.println(coinPath);
    SPIFFS.remove(coinPath);
  }
  
  return true;
}

bool appendSignedCoinOnPendingTXLogOnFS(uint16_t otherBadgeID) {
  char num[6];
  sprintf(num, "%d", otherBadgeID);
  File f = SPIFFS.open(UNSUBMITTED_COIN_PATH, "a");
  
  if (!f)
    return false;
    
  Serial.print("Another coin rides the bus... appended to pending txlog.. badge #");
  Serial.println(num);
  
  f.println(num);
  f.close();
  
  return true;
}

void drainSignedCoinPendingTXLogOnFS() {
  String path;
  String otherBadgeID;
  String coin;
  Serial.println(F("Attempting a pending coin TX log flush"));
  File tmpTXLog = SPIFFS.open("/tmp/txlog.tmp", "w");
  File TXLog = SPIFFS.open(UNSUBMITTED_COIN_PATH, "r");
  
  while (TXLog.available()) {
    otherBadgeID = TXLog.readStringUntil('\n');
    otherBadgeID.trim();
    //sprintf(path, "/coins/%s", otherBadgeID);
    path = "/coins/" + otherBadgeID;
    Serial.print(F("Submiting pending coin on FS at "));
    Serial.println(path);
    File coinFile = SPIFFS.open(path, "r");
    
    if (!coinFile) {
      Serial.print(F("Trying to read coin at "));
      Serial.print(path);
      Serial.println(F("... and it's GONE!!!!!!"));
      continue;
    }

    coin = coinFile.readStringUntil('\n');
    Serial.print("Coin length: ");
    Serial.println(coin.length());

    if (coin.length() == 1)
      continue;  // nothing to do here, this coin has already been submitted
    
    if (!!submitSignedCoinToAPI(coin.c_str())) {
      tmpTXLog.println(otherBadgeID);
      coinFile.close();
    } else {
      coinFile.close();
      File coinFile = SPIFFS.open(path, "w");  // coin submitted okay truncate the json to save space
      coinFile.println(" ");
      coinFile.close();
    }
  }

  tmpTXLog.close();
  TXLog.close();
  SPIFFS.remove(UNSUBMITTED_COIN_PATH);
  SPIFFS.rename("/tmp/txlog.tmp", UNSUBMITTED_COIN_PATH);
}

bool storeCompletedCoinOnFS(uint16_t otherBadgeID) {
  char coinPath[255];
  sprintf(coinPath, "/coins/%d", otherBadgeID);
  File coinFile = SPIFFS.open(coinPath, "w");  // coin submitted okay truncate the json to save space

  if (!coinFile) {
    Serial.print(F("This isn't good, couldn't write out the coin at:"));
    Serial.println(coinPath);
    return false;
  }
  coinFile.println(" ");
  coinFile.close();
}

