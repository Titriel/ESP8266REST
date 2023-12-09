//---Systemzeiten--------------------------------------------------------------
boolean overmilis(unsigned long lastpoint, long towait){ //wird true wenn die Zeit im towait vergangen ist
  if (towait > MillisSince(lastpoint)) return false; else return true;
}

unsigned long MillisSince(unsigned long lastpoint){
  unsigned long nowmillis = millis();
  if (nowmillis < lastpoint) {
    unsigned long help = 0xFFFFFFFF-lastpoint;
    return ++help + nowmillis;
  }else{
    return nowmillis - lastpoint;    
  }
}
    
boolean overmicros(unsigned long lastpoint, long towait){
  if (towait > MicrosSince(lastpoint)) return false; else return true;
}

unsigned long MicrosSince(unsigned long lastpoint){
  unsigned long nowmicros = micros();
  if (nowmicros < lastpoint) {
    unsigned long help = 0xFFFFFFFF-lastpoint;
    return ++help + nowmicros;
  }else{
    return nowmicros - lastpoint;    
  }
}
//---Konvertierungen----------------------------------------------------
//---byte2hex-----------------------------------------------------------
String byte2hex(byte Zahl, byte upper = 0){
  char tBuf[2];
  const char* lu[] = {"%x","%X"};
  if (Zahl < 16){
    tBuf[0] = '0';
    sprintf((char*)&tBuf[1],lu[upper],Zahl);  
  }else{
    sprintf(tBuf,lu[upper],Zahl);  
  }
  return String(tBuf);
}
//---Const Char nach Char-----------------------------------------------
char* C(const char* x){
  uint16_t len = strlen_P(x); 
  char* r = new char[len+1];
  memcpy_P(r, x, len);
  r[len]=0;
  return r;
}
//---Char zu Int16-----------------------------------------------
int16_t Char2Int(char Zahl){
  int16_t Ergebnis = Zahl;
  if ((Zahl & 0x80) != 0) Ergebnis |= 0xFF00;
  return Ergebnis;
}
//---String to Float-----------------------------------------------
double transFloat(String Zahl){
  int16_t Komma = Zahl.indexOf(',');
  if (Komma > -1)Zahl[Komma]='.';
  return (double)Zahl.toFloat();
}

//---Seriel I/O-----------------------------------------------
void Serial_printPGM(const char* x){
  Serial.print(C(x));
}

void Serial_printlnPGM(const char* x){
  Serial.println(C(x));
}

boolean SerialRead(){
  char g;
  if (Serial.available() > 0) {
    g = Serial.read();
    Serial.print(g);
    if (g != '\n'){gtString += String(g);}
  }
  if (g != '\n'){return false;}else{return true;}
}
//---Zeit/Datum-----------------------------------------------------------
String TimeStamp(String failt){
  String help;
  if (timeStatus() == timeNotSet){
    return failt;
  }else{
    return utc(now());
  }
}

String utc(time_t t){
  String help;
  help = String(day(t))+"."+String(month(t))+"."+String(year(t))+" ";
  help += zeitT(t);
  return help;  
}

String zeitT(time_t t){
  String help;
  if (hour(t)<10) {help += "0";}
  help += String(hour(t))+":";
  if (minute(t)<10) {help += "0";}
  help += String(minute(t))+":";
  if (second(t)<10) {help += "0";}
  help += String(second(t))+" ";
  return help;        
}
//---Anzeige der LED StatusInfo-----------------------------------------------------------
void LSI_out(){
  LSI_last = millis();
  switch (LSI_State){
  case 0xF: {LSI_Digit = 100; LSI_temp = LSIp;} //kein breake ist richtig !!!
  case 0x1: {digitalWrite(led, ledOn);
             LSI_State = 0x10;
             if ((uint16_t)LSI_Digit*5 > LSI_temp){
               if (LSI_Digit > LSI_temp){
                 LSI_temp += (uint16_t)LSI_Digit*5;
                 LSI_towait = 800;
               }else{
                 LSI_temp -= LSI_Digit;
                 LSI_towait = 200;
               }
             }else{
               LSI_temp -= (uint16_t)LSI_Digit*5;
               LSI_towait = 800;
             }
            break;}
  case 0x10:{digitalWrite(led, ledOff);
             LSI_State = 0x1;
             if (LSI_Digit > LSI_temp){
               LSI_Digit /=10;
               if (LSI_Digit > 0){
                 LSI_towait = 1600;          
               }else{
                 LSI_State = 0xF;
                 LSI_towait = 5000;
               }
             }else{
               LSI_towait = 600;
             }
            break;}
   case 0xFF:{digitalWrite(led, ledOff);LSI_State = 0x0;break;}
   case 0xF1:{digitalWrite(led, ledOn);LSI_State = 0x0;break;}          
  }
}
//---MD5 Code erzeugen-----------------------------------------------------------
void md5 (String text, byte* bbuff){
  MD5Builder nonce_md5;
  nonce_md5.begin();
  nonce_md5.add(text);  
  nonce_md5.calculate();  
  nonce_md5.getBytes(bbuff);
}
String makeMD5 (String text, boolean authkey){
  byte bbuff[16];
  md5(text, bbuff);
  String hash = "";
  if (authkey){
    unsigned long help = bbuff[3] << 24 | bbuff[11] << 16 | bbuff[6] << 8 | bbuff[9];
    if (help == 0) help = 158745255;
    hash = String(help);
  }else{
    for ( byte i = 0; i < 16; i++) hash += byte2hex(bbuff[i]);
  }  
  return hash;   
}
String makeUUID (String text){
  byte bbuff[16];
  byte minus[] = {4,2,2,2,6};
  byte start = 0;
  md5(text, bbuff);
  String UUID = "";
  for ( byte j = 0; j < 5; j++){
    for ( byte i = start; i < (start + minus[j]); i++){
      UUID += byte2hex(bbuff[i], 1);
    }
    if (j < 4){UUID += "-";}
    start += minus[j];
  }
  return UUID;   
}
//---Temperaturfühler-----------------------------------------------------------
String PartOfDataString(String* Data, char Trenner, byte Index){
  int16_t nstart = 0;
  int16_t ende,start;
  for ( int16_t i = -1; i < Index; i++){
    start = nstart;
    ende = (*Data).indexOf(Trenner,start);
    if (ende == -1) return "";
    nstart = ende + 1; 
  }
  return (*Data).substring(start,ende);
}    
byte getTemperatur(boolean finish) {
  do{
    switch (getTempStat) {
      case 0:
        ds.reset();
        getTempStat++;
        break;
      case 1:    
        ds.skip();          //Alle Sensoren auswählen
        getTempStat++;
        break;
      case 2:    
        ds.write(0x44, 0);  // start conversion
        getTempStat=0;
        break;
      default:
        break;
    }
  }while (finish && !(getTempStat==0));  
  return getTempStat;
}
String readTemperatur(String PreString, String Space, String Tab, String Lineend, String PreMsg, bool Fahrenheit) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  char calib;
  float celsius, fahrenheit;
  String Hstring = "";

  if (getTempStat > 0){
    getTemperatur(true);
    lastget = millis();    
  }
  #define dboffset 4
  uint16_t dbbytes = dboffset;
  File FHdl;
  if(SPIFFS.exists(FPSTR(fm_ct))){
    FHdl = SPIFFS.open(FPSTR(fm_ct), "r");
    dbbytes += FHdl.size();
  }
  if (dbbytes > dboffset){
    FHdl.readBytes(&uniBuf[dboffset], dbbytes-dboffset);
    FHdl.close();
  }
  uint16_t* DataPointer = (uint16_t*)&uniBuf[0];
  uint16_t* DataLength = (uint16_t*)&uniBuf[2];
  *DataPointer = dbbytes;
  *DataLength = 0;    
  while (!overmilis(lastget,750)){delay(1);}    
while(ds.search(addr)) {
  Hstring += PreString;
  for( i = 0; i < 8; i++) {
    Hstring += Space+byte2hex(addr[i]);
  }
  
  if (OneWire::crc8(addr, 7) != addr[7]) {
      Hstring = PreMsg + F("CRC - Error");
      return Hstring;
  }
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:{
      calib = 0;
      type_s = 0;      
      char* calibh;      
      for (uint16_t i = dboffset; i < *DataPointer; i+=8){
        if(*(byte*)&uniBuf[i+7] == addr[7]){
          calibh = (char*)&uniBuf[i];
          calib = *calibh;
        }
      }      
      memcpy(&uniBuf[dbbytes],&addr[0],8);                 
      calibh = (char*)&uniBuf[dbbytes];
      *calibh = calib;
      dbbytes += 8;
      *DataLength += 8;      
      break;}
    case 0x22:
      type_s = 0;
      break;
    default:
      Hstring=PreMsg + F("Unknowen Device");
      return Hstring;
  } 
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  raw += Char2Int(calib);
  celsius = (float)raw / 16.0;
//  fahrenheit = celsius * 1.8 + 32.0;
  if(Fahrenheit){
    Hstring +=Tab+String((float)(celsius * 1.8 + 32.0))+Lineend;
  }else{
    Hstring +=Tab+String((float)celsius)+Lineend;
  }
}
  Hstring += PreMsg + F("No more devices.");
  ds.reset_search();
  return Hstring;
}
//---FileMan--------------------------------------------------------------------------
//Config DB c.d
void makeConf(){
    File Setup = SPIFFS.open(FPSTR(fm_cd), "w");
    Setup.print(ssid + '\n');
    Setup.print(passwort + '\n');
    Setup.print(AP_SSID + '\n');
    Setup.print(AP_Passwort + '\n');
    Setup.print(ntpServerName + '\n');
    Setup.print(SysHash + '\n');       
    Setup.close();  
}

void takeConf(){
    File Setup = SPIFFS.open(FPSTR(fm_cd), "r");
    if (Setup.size()>3){
      ssid = Setup.readStringUntil('\n');
      passwort = Setup.readStringUntil('\n');
      AP_SSID = Setup.readStringUntil('\n');
      AP_Passwort = Setup.readStringUntil('\n');
      ntpServerName = Setup.readStringUntil('\n');
      SysHash = Setup.readStringUntil('\n');
    }
    Setup.close();
}

//---Holt die atuelle Zeit von dem angegebenen Zeitserver-----------------------------
time_t getNtpTime(){
  byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  IPAddress timeServerIP; // NTP server address
  
  NTPudp.begin(2390); //NTP responce are to port 2390
  NTPudp.setTimeout(5);
  WiFi.hostByName(ntpServerName.c_str(), timeServerIP);
  // send an NTP request to the time server at the given address
  Serial.println("sending NTP packet...");
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  //packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  NTPudp.beginPacket(timeServerIP, 123); //NTP requests are to port 123
  NTPudp.write(packetBuffer, NTP_PACKET_SIZE);
  NTPudp.endPacket();  
  unsigned long beginWait = millis();
  while (!overmilis(beginWait,1500)) {
    if (NTPudp.parsePacket() >= NTP_PACKET_SIZE) {
      byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 
      NTPudp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
      NTPudp.stop();
      unsigned long secsSince1900 = packetBuffer[40] << 24 | packetBuffer[41] << 16 | packetBuffer[42] << 8 | packetBuffer[43];
      milliesResync = true;
      return secsSince1900 - 2208988800UL; 
//      return secsSince1900 - 2208981600UL;            
    }
  }
  NTPudp.stop();
  return 0;
}