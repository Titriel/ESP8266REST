//--- API Subrourines ---
void preresponce(AsyncWebServerRequest *httpreq, JsonObject &jroot, bool legal = true){
  String temp = makeUUID(String(now() + String(millis())));
  jroot["UUID"] = temp;
  jroot["UTC"] = TimeStamp("none");
  if(legal){
    String From = httpreq->header("From");
    temp = makeMD5(temp+SysHash, false);
    keydict[From] = temp;
    temp += "@" + From;
  }
}

void rescors(AsyncWebServerResponse *response, String allwori = "*"){
  //if(allwori != "*") response->addHeader("Access-Control-Allow-Origin", allwori);
  response->addHeader("Access-Control-Allow-Methods", "*");
  response->addHeader("Access-Control-Allow-Headers", "*");
}

void jtextsend(AsyncWebServerRequest *httpreq, String &temp, int code){
  if(httpreq->hasHeader("Origin")){
    AsyncWebServerResponse *response = httpreq->beginResponse(code, "application/json", temp);
    rescors(response, httpreq->header("Origin"));
    httpreq->send(response);
  }else{
    httpreq->send(code, "application/json", temp);
  }
}

void jsonsend(AsyncWebServerRequest *httpreq, DynamicJsonDocument &jbuf, int code){
  String temp;
  serializeJsonPretty(jbuf, temp);
  jtextsend(httpreq, temp, code);
}

void d_httprq(AsyncWebServerRequest *httpreq, JsonObject &jroot){
  JsonObject request = jroot.createNestedObject("request");
  request["uri"] = httpreq->url();
  request["method"] = httpreq->method();
  JsonObject args = request.createNestedObject("args");
  for (uint8_t i = 0; i < httpreq->params(); i++) {
    AsyncWebParameter* p = httpreq->getParam(i);
    args[p->name()] = p->value();
  }
  JsonObject headers = request.createNestedObject("headers");
  for (uint8_t i = 0; i < httpreq->headers(); i++) {
    headers[httpreq->headerName(i)] = httpreq->header(i);
  }  
}

bool postrequest(AsyncWebServerRequest *httpreq, bool ns = false){  
  if (httpreq->method() == HTTP_OPTIONS){
    AsyncWebServerResponse *response = httpreq->beginResponse(204);
    rescors(response);
    httpreq->send(response);
    return false;
  }
  DynamicJsonDocument jbuf(1536);
  JsonObject jroot = jbuf.to<JsonObject>(); 
  String temp = "";
  String From = "";
  if(httpreq->hasHeader("From")){
    From = httpreq->header("From");
  }
  if(httpreq->hasHeader("X-Auth")){
    temp = httpreq->header("X-Auth");
  }
  if(ns){
    if ((keydict.size() > 4) && !keydict.containsKey(From)){
      preresponce(httpreq, jroot, false);
      jroot["msg"] = "Max sessions (5) reatched !";
      jsonsend(httpreq, jbuf, 503);
      return false;
    }else if(temp == makeMD5(SysHash+From, false)){
      return true;
    }
  }else{
    if(keydict.containsKey(From)){
      if (temp == keydict[From].as<String>()){        
        return true;
      }else{
        keydict.remove(From);
        sysdoc.garbageCollect();
      }
    }
  }
  preresponce(httpreq, jroot, false);
  jroot["msg"] = "No Session for " + From + ".";
  d_httprq(httpreq, jroot);
  jsonsend(httpreq, jbuf, 401);
  return false;
}
//--- Default Handler for unused Mountpoints ---
void defaulthandler(AsyncWebServerRequest *httpreq, int code = 404, String msg = "None supportet call."){
  if (postrequest(httpreq)){
    DynamicJsonDocument jbuf(1536);
    JsonObject jroot = jbuf.to<JsonObject>();
    preresponce(httpreq, jroot);
    jroot["msg"] = msg;
    d_httprq(httpreq, jroot);
    jsonsend(httpreq, jbuf, code);
  }  
}
void onRequest(AsyncWebServerRequest *httpreq){
  defaulthandler(httpreq);
}
void onBody(AsyncWebServerRequest *httpreq, uint8_t *data, size_t len, size_t index, size_t total){
  defaulthandler(httpreq);
}
//-- Handler Systemcals --
//--- Handler Session begin/distroy ---
void handleSession(AsyncWebServerRequest *httpreq, JsonVariant &jvar = jempty){
  if (httpreq->method()==HTTP_GET){
    if (postrequest(httpreq, true)){
      DynamicJsonDocument jbuf(128);
      JsonObject jroot = jbuf.to<JsonObject>();
      preresponce(httpreq, jroot);
      jroot["msg"] = "Session established.";
      jsonsend(httpreq, jbuf, 201);
    }
    return;      
  }else if (postrequest(httpreq)){
    JsonObject jreq = jvar.as<JsonObject>();
    DynamicJsonDocument jbuf(128);
    JsonObject jroot = jbuf.to<JsonObject>();
    switch (httpreq->method()){
      case HTTP_POST:{
        preresponce(httpreq, jroot);
        if ISen_SSE {
          SSEjPair(jreq["name"].as<const char*>(), jreq["string"].as<const char*>(), jreq["event"].as<const char*>());
          jroot["msg"] = "SSE done.";
          jsonsend(httpreq, jbuf, 200);
        }else{
          jroot["msg"] = "SSE not running.";
          jsonsend(httpreq, jbuf, 428);
        }
        return;
      }
      case HTTP_PATCH: if ISen_SSE {events.close(); en_SSE(false); }
      case HTTP_PUT:{
        preresponce(httpreq, jroot);
        if ISen_SSE {
          jroot["msg"] = "SSE is ready.";
        }else{
          en_SSE(true)
          jroot["msg"] = "SSE established.";
          events.onConnect([](AsyncEventSourceClient *client){
            String temp = "{\"SSE\": \"established\", \"lastmsg\": ";
            if(client->lastId()){
              temp += String(client->lastId());
            }else{
              temp += "NULL";
            }
            temp += "}";
            client->send(temp.c_str(), NULL, evid++, 1000);
          });
          if (jreq.containsKey("user")){
            String temp;
            if (jreq.containsKey("pass")){
              temp = jreq["pass"].as<String>();
            }else{
              temp = keydict[httpreq->header("From")].as<String>();
            }
            events.setAuthentication(jreq["user"].as<const char*>(), temp.c_str());
            server.addHandler(&events);
          }
        }
        jsonsend(httpreq, jbuf, 201);
        return;
      }
      case HTTP_DELETE:{
        preresponce(httpreq, jroot);
        String temp = httpreq->header("From");
        if (keydict.containsKey(temp)){
          keydict.remove(temp);
          sysdoc.garbageCollect();
        }
        jroot["msg"] = "Session deleted.";        
        jsonsend(httpreq, jbuf, 202);
        return;
      }    
      default:{
        defaulthandler(httpreq);
        return;
      }
    }
  }
}
void aktualsysset(JsonObject &jroot){
  jroot["ssid"] = ssid;
  //passwort
  jroot["APssid"] = AP_SSID;
  jroot["APpass"] = AP_Passwort;
  jroot["ntp"] = ntpServerName;
  //SysHash
  JsonArray APnearBy = jroot.createNestedArray("APnearBy");
  byte i = 0;
  String temp = pp_auth;
  while ( temp != "" ){
    APnearBy.add(temp.substring(0,temp.indexOf('\n')));
    temp = temp.substring(temp.indexOf('\n')+1);
    i++;
  }
  jroot["heap"] = String(ESP.getFreeHeap());
}
void dosysset(AsyncWebServerRequest *httpreq, JsonObject &jdoc){
  if(jdoc.containsKey("ssid")) ssid = jdoc["ssid"].as<String>();
  if(jdoc.containsKey("pass")) passwort = jdoc["pass"].as<String>();
  if(jdoc.containsKey("APssid")) AP_SSID = jdoc["APssid"].as<String>();
  if(jdoc.containsKey("APpass")) AP_Passwort = jdoc["APpass"].as<String>();
  if(jdoc.containsKey("ntp")) ntpServerName = jdoc["ntp"].as<String>();
  if(jdoc.containsKey("hash")) SysHash = jdoc["hash"].as<String>();
  if(jdoc.containsKey("deltahash")) SysHash = makeMD5(jdoc["deltahash"].as<String>() + SysHash + httpreq->header("From"),false);
}
void handleConfig(AsyncWebServerRequest *httpreq, JsonVariant &jvar = jempty) {
  if (postrequest(httpreq)){
    JsonObject jreq = jvar.as<JsonObject>();
    DynamicJsonDocument jbuf(1536);
    JsonObject jroot = jbuf.to<JsonObject>();    
    String msg = "."; 
    switch (httpreq->method()){
      case HTTP_GET:{
        preresponce(httpreq, jroot);
        jroot["msg"] = "Loaded settings.";
        aktualsysset(jroot);
        jsonsend(httpreq, jbuf, 200);
        return;
      }
      case HTTP_POST:{
        preresponce(httpreq, jroot);
        jroot["msg"] = "Restarting.";        
        jsonsend(httpreq, jbuf, 200);
        runlevel = RL_Warmstart;
        return;
      }
      case HTTP_PATCH:
        msg = " and restart.";
      case HTTP_PUT:{
        dosysset(httpreq, jreq);
        preresponce(httpreq, jroot);
        jroot["msg"] = "Update config" + msg;
        aktualsysset(jroot);
        jsonsend(httpreq, jbuf, 201);
        makeConf();
        if(httpreq->method()==HTTP_PATCH) runlevel = RL_Warmstart;        
        return;
      }
      case HTTP_DELETE:{
        preresponce(httpreq, jroot);
        jroot["msg"] = "Facktory reset.";
        SetAPvar();
        aktualsysset(jroot);
        jsonsend(httpreq, jbuf, 202);
        makeConf();
        runlevel = RL_Warmstart;
        return;
      }      
      default:{
        defaulthandler(httpreq);
        return;
      }
    }
  }
}
void handleDigital(AsyncWebServerRequest *httpreq, JsonVariant jvar = jempty) {  
  if (postrequest(httpreq)){
    JsonObject jreq = jvar.as<JsonObject>();
    DynamicJsonDocument jbuf(128);
    JsonObject jroot = jbuf.to<JsonObject>();     
    switch (httpreq->method()){
      case HTTP_GET:{
        preresponce(httpreq, jroot);
        jroot["msg"] = "Loaded settings.";
        aktualsysset(jroot);
        jsonsend(httpreq, jbuf, 200);
        return;
      }
      case HTTP_PATCH:{
        if(jreq.containsKey("LSI")){
          LSIp = jreq["LSI"].as<unsigned char>();
          if(LSI_State == 0) LSI_State = 0x0F;
        }
        if(jreq.containsKey("LSIon")){
          LSI_State = (jreq["LSIon"].as<bool>())?0xF1:0xFF;
        }
        preresponce(httpreq, jroot);
        jroot["msg"] = "LSI modified.";
        jsonsend(httpreq, jbuf, 201);
        return;
      }      
      default:{
        defaulthandler(httpreq);
        return;
      }
    }
  }
}

void aktualDs18b20(JsonObject &jroot){
  jroot["enabeled"] = bool ISen_ds18b20;
  jroot["GPIO2used"] = bool ISused_D2;
}
void handleDs18b20(AsyncWebServerRequest *httpreq, JsonVariant jvar = jempty) {  
  if (postrequest(httpreq)){
    JsonObject jreq = jvar.as<JsonObject>();
    DynamicJsonDocument jbuf(256);
    JsonObject jroot = jbuf.to<JsonObject>();    
    switch (httpreq->method()){
      case HTTP_GET:{
        preresponce(httpreq, jroot);
        jroot["msg"] = "Ds18b20 settings.";
        aktualDs18b20(jroot);
        jsonsend(httpreq, jbuf, 200);
        return;
      }      
      case HTTP_POST:{
        String unit = "C";
        if(jreq.containsKey("unit")){
          unit = jreq["unit"].as<String>();
          if((toupper(unit[0]) != 'F') and (toupper(unit[0]) != 'C')) unit = "C";
        }
        preresponce(httpreq, jroot);
        if ISen_ds18b20{
          jroot["msg"] = "Read temperatures.";
          jroot["unit"] = unit;
          jroot["data"] = "@";
          String temp = "";
          serializeJsonPretty(jbuf, temp);
          temp  = temp.substring(0,temp.indexOf('@')-1) + "{\r\n";
          temp += readTemperatur("    \"" , "" , "\": " , ",\r\n" , "    \"msg\": \"", (toupper(unit[0]) == 'F')) + "\"\r\n  }\r\n}";
          jtextsend(httpreq, temp, 200);
        }else{
          jroot["msg"] = "ds18b20 is not enabeld.";
          aktualDs18b20(jroot);
          jsonsend(httpreq, jbuf, 428);
        }
        return;
      }
      case HTTP_PUT:{
        if(!ISused_D2 && jreq.containsKey("enable") && jreq["enable"].as<bool>()){
          use_D2(true);
          en_ds18b20(true);
        }
        preresponce(httpreq, jroot);
        if ISen_ds18b20{
          jroot["msg"] = "ds18b20 is enabeld.";
          aktualDs18b20(jroot);
          jsonsend(httpreq, jbuf, 201);
        }else{
          jroot["msg"] = "GPIO2 is in use.";
          aktualDs18b20(jroot);
          jsonsend(httpreq, jbuf, 409);
        }
        return;
      }
      case HTTP_DELETE:{
        preresponce(httpreq, jroot);
        if ISen_ds18b20{
          use_D2(false);
          en_ds18b20(false);
        }
        jroot["msg"] = "ds18b20 is disabeld.";
        aktualDs18b20(jroot);
        jsonsend(httpreq, jbuf, 202);
        return;
      }                          
      default:{
        defaulthandler(httpreq);
        return;
      }
    }
  }
}

void aktualSerial(JsonObject &jroot, String msg){
  jroot["msg"] = msg;
  jroot["SSE"] = ISen_SSE;
  jroot["inBufSize"] = uniBufoutstart;
  jroot["outBufSize"] = uniBufdatastart - uniBufoutstart;
  JsonObject s0 = jroot.createNestedObject("Serial0");
  s0["baud"] = Serial.baudRate();
  JsonObject s1 = jroot.createNestedObject("Serial1");   
  s1["enabeled"] = bool ISen_TXD1;
  s1["GPIO2used"] = bool ISused_D2;
  if ISen_TXD1 {
    s0["baud"] = Serial.baudRate();
  }
}
void doserialset(AsyncWebServerRequest *httpreq, JsonObject &jreq){
  if(jreq.containsKey("inBufSize")) uniBufoutstart = jreq["inBufSize"].as<unsigned short>();
  if(jreq.containsKey("outBufSize")) uniBufdatastart = uniBufoutstart + jreq["outBufSize"].as<unsigned short>();

}
void handleSerialx(AsyncWebServerRequest *httpreq, JsonVariant jvar = jempty){
  if (postrequest(httpreq)){
    JsonObject jreq = jvar.as<JsonObject>();
    DynamicJsonDocument jbuf(384);
    JsonObject jroot = jbuf.to<JsonObject>();    
    switch (httpreq->method()){
      case HTTP_GET:{
        preresponce(httpreq, jroot);
        aktualSerial(jroot, "Serial settings.");
        jsonsend(httpreq, jbuf, 200);
        return;
      }
      case HTTP_POST:{
        preresponce(httpreq, jroot);
        if (! ISen_SSE ){
          aktualSerial(jroot, "SSE not established!");
          jsonsend(httpreq, jbuf, 428);
        }else if (jreq.containsKey("useTxT1") && ! ISen_TXD1 ){
          aktualSerial(jroot, "TxT1 is disabeled!");
          jsonsend(httpreq, jbuf, 428);
        }else if (!jreq.containsKey("TxT")){
          aktualSerial(jroot, "No data to transmit!");
          jsonsend(httpreq, jbuf, 500);
        }else{
          unsigned short io = 0;
          unsigned short c = jreq["TxT"].as<String>().length();
          for ( unsigned short i = uniBufoutstart; i < uniBufdatastart; i++){
            if (((uniBuf[i] == 0) || ( i % 4 != 0 )) && (io < c)){
              uniBuf[i] = jreq["TxT"].as<String>()[io++];
            }else{
              break;
            }
          }
          jroot["msg"] = "Serial data buferd.";
          jroot["bytes"] = (io / 4) * 3;
          jsonsend(httpreq, jbuf, 200);                   
        }
        return;
      }        
      default:{
        defaulthandler(httpreq);
        return;
      }
    }
  }
}

void webStart(){
  AsyncCallbackJsonWebHandler *handler;

  server.on("/", HTTP_OPTIONS | HTTP_GET | HTTP_DELETE, [](AsyncWebServerRequest *httpreq){handleSession(httpreq);});  
  handler = new AsyncCallbackJsonWebHandler("/", [](AsyncWebServerRequest *httpreq, JsonVariant &json) {
    handleSession(httpreq, json);
  });
  server.addHandler(handler);

  server.on("/config", HTTP_OPTIONS | HTTP_GET | HTTP_DELETE, [](AsyncWebServerRequest *httpreq){handleConfig(httpreq);});  
  handler = new AsyncCallbackJsonWebHandler("/config", [](AsyncWebServerRequest *httpreq, JsonVariant &json) {
    handleConfig(httpreq, json);
  });
  server.addHandler(handler);

  server.on("/digital", HTTP_OPTIONS | HTTP_GET | HTTP_DELETE, [](AsyncWebServerRequest *httpreq){handleDigital(httpreq);});  
  handler = new AsyncCallbackJsonWebHandler("/digital", [](AsyncWebServerRequest *httpreq, JsonVariant &json) {
    handleDigital(httpreq, json);
  });
  server.addHandler(handler);

  server.on("/ds18b20", HTTP_OPTIONS | HTTP_GET | HTTP_DELETE, [](AsyncWebServerRequest *httpreq){handleDs18b20(httpreq);});  
  handler = new AsyncCallbackJsonWebHandler("/ds18b20", [](AsyncWebServerRequest *httpreq, JsonVariant &json) {
    handleDs18b20(httpreq, json);
  });
  server.addHandler(handler);

  server.on("/serial", HTTP_OPTIONS | HTTP_GET | HTTP_DELETE, [](AsyncWebServerRequest *httpreq){handleSerialx(httpreq);});  
  handler = new AsyncCallbackJsonWebHandler("/serial", [](AsyncWebServerRequest *httpreq, JsonVariant &json) {
    handleSerialx(httpreq, json);
  });
  server.addHandler(handler);

  server.onRequestBody(onBody);
  server.onNotFound(onRequest);
  server.begin();
}

/*void webStart() {
  server.on("/", handleSession);
  server.on("/config", handleConfig);
  server.on("/digital", handleDigital);
  server.on("/ds18b20", handleDs18b20);
  server.onNotFound(handleNotFound);
  server.enableCORS(true);
  server.collectHeaders("User-Agent", "From", "X-Auth");
  server.begin();
}*/