void SSEjPair(const char* name, const char* value, const char* event = "os" ){
  unsigned short l = strlen(name) + strlen(value) + 10;
  char buffer[l];
  sprintf(buffer, "{\"%s\": \"%s\"}", name, value);
  events.send(buffer, event, evid++);
}

void setup() {
  pinMode ( led, OUTPUT );
  digitalWrite ( led, ledOff );
  Serial.begin ( serial0speed );
  Serial.println ();
  pinMode(15, OUTPUT);
  SetAPvar();
  ReIni();
  uniBuf[uniBufout] = 0;
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "null");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
}

void loop() {
  if (overmilis(lastget,10000)){
    if(ISen_ds18b20 && (getTemperatur(false) == 0)){lastget = millis();}
  }
  switch (runlevel){
  case RL_Warmstart:{
    digitalWrite ( led, ledOn );
    delay(2000);
    if(ISen_ds18b20 && (getTempStat!=0)){
      getTemperatur(true);
    }
    SetAPvar();
    ReIni();
    WiFi.disconnect();
    SPIFFS.end();
    runlevel = RL_Start;
  }
  case RL_Start:{
    //---FileSystem initalisieren/öffnen----------------------------------------------------
    //Serial.println(SERIAL_BUFFER_SIZE);
    SPIFFS.begin();  
    if (!SPIFFS.exists(FPSTR(fm_cd))){
      if (SPIFFS.format())Serial_printlnPGM(TX_Format);
        makeConf();
        runlevel = RL_NoUseablelAP;  
    }else{
        takeConf();
    }
    //---Verfügbare AP ermitteln------------------------------------------------------------
    runlevel = RL_NoUseablelAP;
    WiFi.mode(WIFI_STA);    //Scanne nach vorhandenen Netzwerken
    WiFi.disconnect();
    delay(100);
    n = WiFi.scanNetworks();
    pp_auth = "";
    for (int i = 0; i < n; ++i){
      if ( ssid == WiFi.SSID(i)){
        MyWLAN = i;
        runlevel = RL_APreachable;
      }
      pp_auth += WiFi.SSID(i);
      pp_auth += "\n";
      Serial.print(i+1);
      Serial.print(F(". "));
      Serial.print(WiFi.SSID(i));
      Serial.print(F(" Level: "));
      Serial.print(WiFi.RSSI(i));
      Serial.print(F("dBm "));
      if (WiFi.encryptionType(i) != ENC_TYPE_NONE){Serial_printPGM(TX_NeedPass);}
      Serial.println ( "" );
    }
  break;}
  case RL_APreachable:{
    //---In AP einloggen--------------------------------------------------------------------
    WiFi.begin(ssid.c_str(), passwort.c_str());
    //---Wait for connection--------
    uint32_t beginWait = millis();
    while ((millis() - beginWait < 20000) && (WiFi.status() != WL_CONNECTED)) {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() != WL_CONNECTED){
      WiFi.disconnect();
      runlevel = RL_NoUseablelAP;
    }else{
      runlevel = RL_APlogged;
      Serial.println ( "" );
      Serial.print ( TX_ConTo );
      Serial.println ( ssid );
      Serial_printPGM(TX_IpAdr);
      myIP = WiFi.localIP();
      Serial.println(myIP);
      LSIp = myIP[3];
      LSI_State = 0x0F;
      if ( mdns.begin (C(TX_DNS))) {
        Serial_printlnPGM(TX_MDNSstart);
      }
      Inet = true;
    }        
  break;}
  case RL_NoUseablelAP:{
    //---Eigener AP starten------------------------------------------------------------------
    WiFi.mode(WIFI_AP);
    myIP[0]=192;myIP[1]=168;myIP[2]=1;myIP[3]=1;
    //myIP(192, 168, 1, 1);
    IPAddress NMask(255, 255, 255, 0);
    WiFi.softAPConfig(myIP, myIP, NMask);
    WiFi.softAP(AP_SSID.c_str(), AP_Passwort.c_str(),6);
    Serial.println ( "" );
    Serial.print ( TX_ApOpenAs );
    Serial.println ( AP_SSID );
    Serial_printPGM(TX_Passwd);
    Serial.println ( AP_Passwort );
    Serial_printPGM(TX_IpAdr);
    Serial.println( myIP );
    LSIp = myIP[3];
    LSI_State = 0x0F;
    if (mdns.begin(C(TX_DNS), myIP)) {
      Serial_printlnPGM(TX_MDNSstart);
    }
    //---Configseite verfügbar machen---------------------------------------------------------
    webStart();
    Serial_printlnPGM(TX_HTTPstart);    
    runlevel = RL_TrySerialAPsel;
    Serial.println ( "" );
    Serial_printPGM (TX_TypeAPRef);
    gtString  = "";
  break;}    
  case RL_TrySerialAPsel:{
    //---Versuchen über Serielle Schnittstelle einen AP zu wählen------------------------------
    if (SerialRead()){ //liest Eingabe in webParts 
      byte count = gtString.toInt();
      for ( byte i = 1; i < count; i++){
        pp_auth = pp_auth.substring(pp_auth.indexOf('\n')+1); 
      }
      ssid = pp_auth.substring(0,pp_auth.indexOf('\n'));
      Serial.println (ssid);
      runlevel = RL_TrySerialPass;
      Serial_printPGM (TX_TypePass);
      gtString  = "";
    }
  break;}
  case RL_TrySerialPass:{
    //---Versuchen über Serielle Schnittstelle ein Passwort zu lesen---------------------------
    if (SerialRead()){
      passwort = gtString;
      Serial.println (passwort);
      makeConf();  
      runlevel = RL_StopAlltoRes;
    }
  break;}
  case RL_WaitStopAlltoRes:delay(800);
  case RL_StopAlltoRes:{
    //---Alles stoppen um neu zu starten-------------------------------------------------------
    SPIFFS.end();
    server.end();
    WiFi.disconnect();
    ReIni();
  break;}  
  case RL_APlogged:{
    webStart();
    runlevel = RL_ServerRunning;
  } //Ohne breake Ok  
  case RL_ServerRunning:{
    runlevel |= RL_NtpFlag;    
    if (timeStatus() == timeNotSet){setSyncProvider(getNtpTime);}
    if (timeStatus() == timeNotSet){
      break;
    }else{
      Serial.println(TimeStamp("No NTP"));
      runlevel = RL_AllRunning;
    }
  }
  case RL_AllRunning:{
    TE_AcktLoop = now();
    //Daten am Serielport verarbeiten

    if (SerialByteRead()){
      uniBuf[uniBufin] = 0;
      uniBufin = 0;
      if ISen_SSE {
        SSEjPair("RxT", uniBuf, "Serial0");
      }
    }
    if (uniBuf[uniBufout] != 0){
      if ISen_TXD1 {
        if (SerialByteWrite(Serial1)) SSEjPair("TxT", "Send done", "Serial1");
      }else{
        if (SerialByteWrite(Serial)) SSEjPair("TxT", "Send done", "Serial0");
      }
    }
  break;}
  }

  //---Anzeige der LED StatusInfo-----------------------------------------------------------
  if ((LSI_State != 0) && overmilis(LSI_last,LSI_towait)){LSI_out();}     
}