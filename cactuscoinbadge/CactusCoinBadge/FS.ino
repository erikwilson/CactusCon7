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

bool ifCoinExistsOnFS(uint16_t badgeID) {
  char coinPath[255];
  sprintf(coinPath, "/coins/%d", badgeID);
  Serial.print("Checking if coin exists at ");
  Serial.println(coinPath);
  return SPIFFS.exists(coinPath);
}

bool storeUnsentSignedCoinOnFS(int otherBadgeID, String json) {
  char coinPath[255];
  sprintf(coinPath, "/coins/%d", otherBadgeID);

  if (SPIFFS.exists(coinPath)) {
    Serial.print("Skipping this write, already got a coin at..");
    Serial.println(coinPath);
    return true;
  }
  
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

  if (!appendSignedCoinOnPendingTXLogOnFS(otherBadgeID)) {
    Serial.print("Failed to append coin to pending txlog, rolling back.");
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
  Serial.println("Attempting a pending coin TX log flush");
  File tmpTXLog = SPIFFS.open("/tmp/txlog.tmp", "w");
  File TXLog = SPIFFS.open(UNSUBMITTED_COIN_PATH, "r");
  
  while (TXLog.available()) {
    otherBadgeID = TXLog.readStringUntil('\n');
    otherBadgeID.trim();
    //sprintf(path, "/coins/%s", otherBadgeID);
    path = "/coins/" + otherBadgeID;
    Serial.print("Trying to submit coin at ");
    Serial.println(path);
    File coinFile = SPIFFS.open(path, "r");
    
    if (!coinFile) {
      Serial.print("Trying to read coin at ");
      Serial.print(path);
      Serial.println("... and it's GONE!!!!!!");
      continue;
    }

    if (!submitSignedCoinToAPI(coinFile.readStringUntil('\n'))) {
      tmpTXLog.println(otherBadgeID);
      coinFile.close();
    } else {
      coinFile.close();
      File coinFile = SPIFFS.open(path, "w");  // coin submitted okay truncate the json to save space
      coinFile.println();
      coinFile.close();
    }
  }

  tmpTXLog.close();
  TXLog.close();
  SPIFFS.remove(UNSUBMITTED_COIN_PATH);
  SPIFFS.rename("/tmp/txlog.tmp", UNSUBMITTED_COIN_PATH);
}
