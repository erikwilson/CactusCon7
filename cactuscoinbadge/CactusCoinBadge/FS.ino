int setupFS() {
  if(!SPIFFS.begin(true)) {
    Serial.println("Failed to start SPIFFS!");
    return -1;
  }
  
  File f = SPIFFS.open("/my.id", "r");
  
  if (!f) {
    Serial.println("Failed to open badge ID");
    return -1;
  }
  else {
    myBadgeID = f.read() - '0';
  }

  return 0;
}

String jsonifyCoin(byte *scnPtr, int packetSize) {
  SignedCoin *scn;
  String jsonString;
  StaticJsonBuffer<200> jsonBuffer;
  
  if ((packetSize - 1) != sizeof(SignedCoin)) {  // subtract 1 to account for type byte
    Serial.print("SignedCoin was an invalid length of ");
    Serial.println(packetSize);
    return "";
  }

  scn = (SignedCoin *)scnPtr;
  
  JsonObject &root = jsonBuffer.createObject();
  root["CSRID"] = scn->csr.coin.CSRID;
  root["broadcasterID"] = scn->csr.coin.broadcasterID;
  root["signatureCSR"] = scn->csr.signatureCSR;
  root["signatureBroadcaster"] = scn->signatureBroadcaster;
  root.printTo(jsonString);
  return jsonString;
}
