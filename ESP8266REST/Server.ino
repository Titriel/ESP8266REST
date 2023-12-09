//--- API Subrourines ---
void preresponce(bool legal = true){
  jroot.clear();
  jdoc.set(jroot);
  gtString = makeUUID(String(now() + String(millis())));
  jroot["UUID"] = gtString;
  jroot["UTC"] = TimeStamp("none");
  if(legal){
    String From = headerdict["From"].as<String>();
    gtString = makeMD5(gtString+SysHash, false);
    keydict[From] = gtString;
    gtString += "@" + From;
    // Serial.println ( gtString );
  }
}

void jsonsend(int code){
  gtString = "";
  serializeJsonPretty(jdoc, gtString);
  server.send(code, "application/json", gtString);  
}

void d_httprq(){
  JsonObject request = jroot.createNestedObject("request");
  request["uri"] = server.uri();
  request["method"] = server.method();
  JsonObject args = request.createNestedObject("args");
  for (uint8_t i = 0; i < server.args(); i++) {
    args[server.argName(i)] = server.arg(i);
  }
  JsonObject headers = request.createNestedObject("headers");
  for (uint8_t i = 2; i < server.headers(); i++) {
    headers[server.headerName(i)] = server.header(i);
  }  
}

bool postrequest(bool ns = false){  
  if (server.method() == HTTP_OPTIONS){
    server.sendHeader("Access-Control-Allow-Methods", "*");
    server.sendHeader("Access-Control-Allow-Headers", "*");
    server.send(204);
    return false;
  }
  gtString = "";
  String From = "";   
  for (byte i = 2; i < server.headers(); i++) {
    headerdict[server.headerName(i)] = server.header(i);
  } 
  gtString = headerdict["X-Auth"].as<String>();;
  From = headerdict["From"].as<String>();;
  if(ns){
    String temp = makeMD5(SysHash+From, false);
    //Serial.println(temp);
    if ((keydict.size() > 4) && !keydict.containsKey(From)){
      preresponce(false);
      jroot["msg"] = "Max sessions (5) reatched !";
      jsonsend(503);
      return false;
    }else if(gtString == temp){
      return true;
    }
  }else{
    if(keydict.containsKey(From)){
      String temp = keydict[From].as<String>();
      if (temp == gtString){
        for (byte i = 0; i < server.args(); i++) {
          if (server.argName(i) == "plain") deserializeJson(jdoc, server.arg(i));
        }        
        return true;
      }else{
        keydict.remove(From);
        sysdoc.garbageCollect();
      }
    }
  }
  preresponce(false);
  jroot["msg"] = "No Session for " + From + ".";
  d_httprq();
  jsonsend(401);
  return false;
}
//-- Handler Systemcals --
//--- Default Handler for unused Mountpoints ---
void handleNotFound() {
  if (postrequest()){
    preresponce();
    jroot["msg"] = "None supportet call.";
    d_httprq();
    jsonsend(404);
  }
}
//--- Handler Session begin/distroy ---
void handleSession() {
  switch (server.method()){
    case HTTP_GET:{
      if (postrequest(true)){
        preresponce();
        jroot["msg"] = "Session established.";
        jsonsend(201);
      }
      return;
    }
    case HTTP_DELETE:{
      if (postrequest()){
        preresponce();
        gtString = headerdict["From"].as<String>();
        if (keydict.containsKey(gtString)){
          keydict.remove(gtString);
          sysdoc.garbageCollect();
        }
        jroot["msg"] = "Session deleted.";        
        jsonsend(202);
      }
      return;
    }    
    default:{
      handleNotFound();
      return;
    }
  }
}
void aktualsysset(){
  jroot["ssid"] = ssid;
  //passwort
  jroot["APssid"] = AP_SSID;
  jroot["APpass"] = AP_Passwort;
  jroot["ntp"] = ntpServerName;
  //SysHash
  JsonArray APnearBy = jroot.createNestedArray("APnearBy");
  byte i = 0;
  gtString = pp_auth;
  while ( gtString != "" ){
    APnearBy.add(gtString.substring(0,gtString.indexOf('\n')));
    gtString = gtString.substring(gtString.indexOf('\n')+1);
    i++;
  }
}
void dosysset(){
  if(jdoc.containsKey("ssid")) ssid = jdoc["ssid"].as<String>();
  if(jdoc.containsKey("pass")) passwort = jdoc["pass"].as<String>();
  if(jdoc.containsKey("APssid")) AP_SSID = jdoc["APssid"].as<String>();
  if(jdoc.containsKey("APpass")) AP_Passwort = jdoc["APpass"].as<String>();
  if(jdoc.containsKey("ntp")) ntpServerName = jdoc["ntp"].as<String>();
  if(jdoc.containsKey("hash")) SysHash = jdoc["hash"].as<String>();
  if(jdoc.containsKey("deltahash")) SysHash = makeMD5(jdoc["deltahash"].as<String>() + SysHash + headerdict["From"].as<String>(),false);
}
void handleConfig() {
  if (postrequest()){
    String msg = "."; 
    switch (server.method()){
      case HTTP_GET:{
        preresponce();
        jroot["msg"] = "Loaded settings.";
        aktualsysset();
        jsonsend(200);
        return;
      }
      case HTTP_POST:{
        preresponce();
        jroot["msg"] = "Restarting.";        
        jsonsend(200);
        runlevel = RL_Warmstart;
        return;
      }
      case HTTP_PATCH:
        msg = " and restart.";
      case HTTP_PUT:{
        dosysset();
        preresponce();
        jroot["msg"] = "Update config" + msg;
        aktualsysset();
        jsonsend(201);
        makeConf();
        if(server.method()==HTTP_PATCH) runlevel = RL_Warmstart;        
        return;
      }
      case HTTP_DELETE:{
        preresponce();
        jroot["msg"] = "Facktory reset.";
        SetAPvar();
        aktualsysset();
        jsonsend(202);
        makeConf();
        runlevel = RL_Warmstart;
        return;
      }      
      default:{
        handleNotFound();
        return;
      }
    }
  }
}
void handleDigital() {
  if (postrequest()){ 
    switch (server.method()){
      case HTTP_GET:{
        preresponce();
        jroot["msg"] = "Loaded settings.";
        aktualsysset();
        jsonsend(200);
        return;
      }
      case HTTP_PATCH:{
        if(jdoc.containsKey("LSI")){
          LSIp = jdoc["LSI"].as<unsigned char>();
          if(LSI_State == 0) LSI_State = 0x0F;
        }
        if(jdoc.containsKey("LSIon")){
          LSI_State = (jdoc["LSIon"].as<bool>())?0xF1:0xFF;
        }
        preresponce();
        jroot["msg"] = "LSI modified.";
        jsonsend(201);
        return;
      }      
      default:{
        handleNotFound();
        return;
      }
    }
  }
}

void aktualDs18b20(){
  jroot["enabeled"] = bool ISen_ds18b20;
  jroot["GPIO2used"] = bool ISused_D2;
}
void handleDs18b20() {
  if (postrequest()){ 
    switch (server.method()){
      case HTTP_GET:{
        preresponce();
        jroot["msg"] = "Ds18b20 settings.";
        aktualDs18b20();
        jsonsend(200);
        return;
      }      
      case HTTP_POST:{
        String unit = "C";
        if(jdoc.containsKey("unit")){
          unit = jdoc["unit"].as<String>();
          if((toupper(unit[0]) != 'F') and (toupper(unit[0]) != 'C')) unit = "C";
        }
        preresponce();
        if ISen_ds18b20{
          jroot["msg"] = "Read temperatures.";
          jroot["unit"] = unit;
          jroot["data"] = "@";
          gtString = "";
          serializeJsonPretty(jdoc, gtString);
          gtString  = gtString.substring(0,gtString.indexOf('@')-1) + "{\r\n";
          gtString += readTemperatur("    \"" , "" , "\": " , ",\r\n" , "    \"msg\": \"", (toupper(unit[0]) == 'F')) + "\"\r\n  }\r\n}";
          server.send(200, "application/json", gtString);
        }else{
          jroot["msg"] = "ds18b20 is not enabeld.";
          aktualDs18b20();
          jsonsend(428);
        }
        return;
      }
      case HTTP_PUT:{
        if(!ISused_D2 && jdoc.containsKey("enable") && jdoc["enable"].as<bool>()){
          use_D2(true);
          en_ds18b20(true);
        }
        preresponce();
        if ISen_ds18b20{
          jroot["msg"] = "ds18b20 is enabeld.";
          aktualDs18b20();
          jsonsend(201);
        }else{
          jroot["msg"] = "GPIO2 is in use.";
          aktualDs18b20();
          jsonsend(409);
        }
        return;
      }
      case HTTP_DELETE:{
        preresponce();
        if ISen_ds18b20{
          use_D2(false);
          en_ds18b20(false);
        }
        jroot["msg"] = "ds18b20 is disabeld.";
        aktualDs18b20();
        jsonsend(202);
        return;
      }                          
      default:{
        handleNotFound();
        return;
      }
    }
  }
}    
void webStart() {
  server.on("/", handleSession);
  server.on("/config", handleConfig);
  server.on("/digital", handleDigital);
  server.on("/ds18b20", handleDs18b20);
  server.onNotFound(handleNotFound);
  server.enableCORS(true);
  server.collectHeaders("User-Agent", "From", "X-Auth");
  headerdict["X-Auth"] = "";
  headerdict["From"] = "";  
  gtString = "{\"hello\":\"world\"}";
  deserializeJson(jdoc, gtString);
  server.begin();
}