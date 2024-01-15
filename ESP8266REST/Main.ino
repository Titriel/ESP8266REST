void SSEjPair(const char* name, const char* value, const char* event = "os" ){
  unsigned short l = strlen(name) + strlen(value) + 10;
  char buffer[l];
  sprintf(buffer, "{\"%s\": \"%s\"}", name, value);
  events.send(buffer, event, evid++);
}
void SendMsg(const char* text, bool term = true){
  if ISen_SSE {
    sysdict["msg"] = sysdict["msg"].as<String>() + String(text);
    if(term){
      events.send(sysdict["msg"].as<const char*>(), "os", evid++);
      sysdict["msg"] = "";
    }
  }else if(term){
    Serial.println(text);
  }else{
    Serial.print(text);
  }
}

void setup() {
  pinMode ( led, OUTPUT );
  digitalWrite ( led, ledOff );
  Serial.begin ( serial0speed );
  SendMsg ("");
  pinMode(15, OUTPUT);
  SetAPvar();
  ReIni();
  sysdict["msg"] = "";
  uniBuf[uniBufout] = 0;
  uniBuf[uniBufin] = 0;
  DefaultHeaders::Instance().addHeader(PSTR("Access-Control-Allow-Origin"), "null");
  DefaultHeaders::Instance().addHeader(PSTR("Access-Control-Allow-Credentials"), "true");
  delay(100);
  while (Serial.available() > 0) Serial.read();
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
    SPIFFS.begin();  
    if (!SPIFFS.exists(FPSTR(fm_cd))){
      if (SPIFFS.format()) SendMsg(TX_Format);
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
      unsigned short l = WiFi.SSID(i).length() + 20;
      char txtbuf[l];
      sprintf(txtbuf, PSTR("%d. %s Level: %ddBm"), (i+1), WiFi.SSID(i).c_str(), WiFi.RSSI(i));
      bool nopw = (WiFi.encryptionType(i) == ENC_TYPE_NONE);
      SendMsg(txtbuf, nopw);
      if (!nopw) SendMsg(TX_NeedPass); 
    }
  break;}
  case RL_APreachable:{
    //---In AP einloggen--------------------------------------------------------------------
    WiFi.begin(ssid.c_str(), passwort.c_str());
    //---Wait for connection--------
    uint32_t beginWait = millis();
    while ((millis() - beginWait < 20000) && (WiFi.status() != WL_CONNECTED)) {
      delay(500);
      SendMsg(".", false);
    }
    if (WiFi.status() != WL_CONNECTED){
      WiFi.disconnect();
      runlevel = RL_NoUseablelAP;
    }else{
      runlevel = RL_APlogged;
      unsigned short l = ssid.length() + 46;
      char txtbuf[l];
      myIP = WiFi.localIP();
      sprintf(txtbuf, PSTR("\n%s%s %s%d.%d.%d.%d"), TX_ConTo, ssid.c_str(),TX_IpAdr, myIP[0], myIP[1],  myIP[2], myIP[3]);
      SendMsg(txtbuf);
      LSIp = myIP[3];
      LSI_State = 0x0F;
      if ( mdns.begin (C(TX_DNS))) {
        SendMsg(TX_MDNSstart);
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
    unsigned short l = AP_SSID.length()+ AP_Passwort.length() + 56;
    char txtbuf[l];
    sprintf(txtbuf, PSTR("\n%s%s %s%s %s%d.%d.%d.%d"), TX_ApOpenAs, AP_SSID.c_str(), TX_Passwd, AP_Passwort.c_str(), TX_IpAdr, myIP[0], myIP[1],  myIP[2], myIP[3]);
    SendMsg(txtbuf);
    LSIp = myIP[3];
    LSI_State = 0x0F;
    if (mdns.begin(C(TX_DNS), myIP)) {
      SendMsg(TX_MDNSstart);
    }
    //---Configseite verfügbar machen---------------------------------------------------------
    webStart();
    SendMsg(TX_HTTPstart);    
    runlevel = RL_TrySerialAPsel;
    SendMsg("");
    SendMsg(TX_TypeAPRef, false);
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
      SendMsg(ssid.c_str());
      runlevel = RL_TrySerialPass;
      SendMsg(TX_TypePass, false);
      gtString  = "";
    }
  break;}
  case RL_TrySerialPass:{
    //---Versuchen über Serielle Schnittstelle ein Passwort zu lesen---------------------------
    if (SerialRead()){
      passwort = gtString;
      SendMsg(passwort.c_str());
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
    setSyncProvider(getNtpTime);
    setSyncInterval(120);
    runlevel = RL_TimeNTP;
  }
  case RL_TimeNTP:{
    if (milliesResync && (timeStatus() == timeSet)){runlevel = RL_AllRunning;}
  }
  case RL_AllRunning:{
    TE_AcktLoop = now();
    if (milliesResync){
      SendMsg("Updated: ", false);
      SendMsg(TimeStamp("No NTP").c_str());
      if (timeStatus() != timeSet){runlevel = RL_TimeNTP;}
      milliesResync = false;
    }    
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