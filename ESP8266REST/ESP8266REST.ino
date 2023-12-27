#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <FS.h>
#include <TimeLib.h>
//---Anpassung für Hardwareaufbau------------------------------
#define ledOn false //je nach Beschaltung anpassen
#define ledOff true //je nach Beschaltung anpassen
#define DefaultLogin F("admin@admin")

//---Universal Global Variabelen-------------------------------
String gtString = "";
#define s_uniBuf  3380
char uniBuf[s_uniBuf];  //universal Buffer für Subs kann in jeder Sub benutzt werden 3380
unsigned short uniBufin = 0;
unsigned short uniBufoutstart = 1000;
unsigned short uniBufout = uniBufoutstart;
unsigned short uniBufdatastart = 2000;
unsigned long serial0speed = 115200;
JsonVariant jempty;
//StaticJsonDocument<3000> jdoc;
//JsonObject jroot = jdoc.to<JsonObject>();
StaticJsonDocument<1000> sysdoc;
JsonObject sysdict = sysdoc.to<JsonObject>();
JsonObject keydict = sysdict.createNestedObject("keydict");

#define led 0    // on GPIO0
static const char Unset[] = ";}na]{";
#define RL_NtpFlag        0x08

#define RL_Start          0x00
#define RL_NoUseablelAP   0x01
#define RL_APreachable    0x02
#define RL_APlogged       0x03
#define RL_Warmstart      0x04
#define RL_WaitStopAlltoRes 0x0E
#define RL_StopAlltoRes   0x0F
#define RL_TrySerialAPsel 0x10
#define RL_TrySerialPass  0x11
#define RL_ServerRunning  0x20
#define RL_Time           0x23
#define RL_AllRunning     0xFF
byte runlevel;

byte IOused = 0;
#define ISused_D4         ((IOused & 0x01) != 0)
#define ISused_D5         ((IOused & 0x02) != 0)
#define ISused_D12        ((IOused & 0x04) != 0)
#define ISused_D13        ((IOused & 0x08) != 0)
#define ISused_D14        ((IOused & 0x10) != 0)
#define ISused_D15        ((IOused & 0x20) != 0)
#define ISused_D16        ((IOused & 0x40) != 0)
#define ISused_D2         ((IOused & 0x80) != 0)
#define use_D4(state)    if(state) IOused |= 0x01; else IOused &= 0xFE;
#define use_D5(state)    if(state) IOused |= 0x02; else IOused &= 0XFD;
#define use_D12(state)   if(state) IOused |= 0x04; else IOused &= 0XFB;
#define use_D13(state)   if(state) IOused |= 0x08; else IOused &= 0XF7;
#define use_D14(state)   if(state) IOused |= 0x10; else IOused &= 0xEF;
#define use_D15(state)   if(state) IOused |= 0x20; else IOused &= 0xDF;
#define use_D16(state)   if(state) IOused |= 0x40; else IOused &= 0xBF;
#define use_D2(state)    if(state) IOused |= 0x80; else IOused &= 0X7F;

#define ISen_ds18b20    ((IFenabeld & 0x01) != 0) //D2 Software
#define ISen_TXD1       ((IFenabeld & 0x02) != 0) //D2
#define ISen_HSPI       ((IFenabeld & 0x04) != 0) //D12,13,14,15
#define ISen_I2S1       ((IFenabeld & 0x08) != 0) //D12,13,14 Software
#define ISen_TWI        ((IFenabeld & 0x10) != 0) //D2,14 Software
#define ISen_IR         ((IFenabeld & 0x20) != 0) //D5,14 Software
#define ISen_TOUT       ((IFenabeld & 0x40) != 0) //ADC
#define ISen_ADC        ((IFenabeld & 0x40) != 0) //ADC
#define en_ds18b20(state)    if(state) IFenabeld |= 0x01; else IFenabeld &= 0xFE;
#define en_TXD1(state)       if(state) IFenabeld |= 0x02; else IFenabeld &= 0xFD;
#define en_HSPI(state)       if(state) IFenabeld |= 0x04; else IFenabeld &= 0xFB;
#define en_I2S1(state)       if(state) IFenabeld |= 0x08; else IFenabeld &= 0xF7;
#define en_TWI(state)        if(state) IFenabeld |= 0x10; else IFenabeld &= 0xEF;
#define en_IR(state)         if(state) IFenabeld |= 0x20; else IFenabeld &= 0xDF;
#define en_TOUT(state)       if(state) IFenabeld |= 0x40; else IFenabeld &= 0xBF;
#define en_ADC(state)        if(state) IFenabeld |= 0x80; else IFenabeld &= 0x7F;
byte IFenabeld = 0;

#define ISen_xds18b20    ((SFenabeld & 0x01) != 0) //D2 Software
#define ISen_xTXD1       ((SFenabeld & 0x02) != 0) //D2
#define ISen_xHSPI       ((SFenabeld & 0x04) != 0) //D12,13,14,15
#define ISen_xI2S1       ((SFenabeld & 0x08) != 0) //D12,13,14 Software
#define ISen_xTWI        ((SFenabeld & 0x10) != 0) //D2,14 Software
#define ISen_LoopTXD     ((SFenabeld & 0x20) != 0) //D5,14 Software
#define ISen_LoopTXD1    ((SFenabeld & 0x40) != 0) //ADC
#define ISen_SSE         ((SFenabeld & 0x80) != 0) //SSE
#define en_xds18b20(state)    if(state) SFenabeld |= 0x01; else SFenabeld &= 0xFE;
#define en_xTXD1(state)       if(state) SFenabeld |= 0x02; else SFenabeld &= 0xFD;
#define en_xHSPI(state)       if(state) SFenabeld |= 0x04; else SFenabeld &= 0xFB;
#define en_xI2S1(state)       if(state) SFenabeld |= 0x08; else SFenabeld &= 0xF7;
#define en_xTWI(state)        if(state) SFenabeld |= 0x10; else SFenabeld &= 0xEF;
#define en_LoopTXD(state)     if(state) SFenabeld |= 0x20; else SFenabeld &= 0xDF;
#define en_LoopTXD1(state)    if(state) SFenabeld |= 0x40; else SFenabeld &= 0xBF;
#define en_SSE(state)         if(state) SFenabeld |= 0x80; else SFenabeld &= 0x7F;
byte SFenabeld = 0;
boolean Inet;
String ssid,passwort,AP_SSID,AP_Passwort;
String SysHash = "";
int16_t n = 0;
int16_t MyWLAN = -1;
IPAddress myIP;
String pp_auth;
uint32_t TE_AcktLoop=0;
String SerInfo = "";
uint32_t SerInfoT;
//------MDNS-------------------------------------------------------------------------
MDNSResponder mdns;
//------HTTP-Server------------------------------------------------------------------
AsyncWebServer server(80);
AsyncEventSource events("/events");
unsigned int evid = 0;
//------für LED StatusInfo-----------------------------------------------------------
byte LSIp = 0;
byte LSI_Digit,LSI_State; 
uint16_t LSI_temp;
uint16_t LSI_towait = 0;
uint32_t LSI_last = 0;
//------für Temperaturfühler---------------------------------------------------------
#define OneWirePin 2
OneWire  ds(OneWirePin);  // on GPIO2 (a 4.7K pullup resistor is necessary)
unsigned long lastget;
byte getTempStat = 0;
//------für die NTP-Abfrage-----------------------------------------------------------
WiFiUDP NTPudp;
String ntpServerName;
#define NTP_PACKET_SIZE  48 // NTP time stamp is in the first 48 bytes of the message
boolean milliesResync = true;
//---FileMan--------------------------------------------------------------------------
#define fm_count 3
static const char fm_cd[] PROGMEM = "/c.d";
static const char fm_gd[] PROGMEM = "/g.d";
static const char fm_ct[] PROGMEM = "/c.t";
static const char fm_tt[] PROGMEM = "/t.t";
const char* fm_list[] = {(const char*)&fm_cd[0],
                         (const char*)&fm_gd[0],
                         (const char*)&fm_ct[0],
                         (const char*)&fm_tt[0]};

void SetAPvar(){
  ssid = FPSTR(Unset);
  passwort = ssid;
  AP_SSID = F("ESP8266_Conf");
  AP_Passwort = F("ESP8266M");
  ntpServerName = F("ntp1.t-online.de");
  SysHash = makeMD5(DefaultLogin,false);
}
void ReIni(){
  if ISen_SSE {
    events.close();
    en_SSE(false);
  }
  serial0speed = 115200;
  Serial.end();
  Serial.begin ( serial0speed );
  LSI_State = 0x0FF;
  if (keydict.size() > 0){
    keydict.clear();
    sysdoc.garbageCollect();
  }
  runlevel = RL_Start;
  Inet = false;
}

//---Texte---------------------------------------------------------------------------
#define           TX_ConTo F("Verbunden mit: ")
#define           TX_ApOpenAs F("AP gestartet als: ")
static const char TX_DNS[] PROGMEM = "esp8266";
static const char TX_Format[] PROGMEM = "Erzeuge Filesystem";
static const char TX_NeedPass[] PROGMEM = "Passwortgeschuetzt";
static const char TX_Passwd[] PROGMEM = "Passwort: ";
static const char TX_IpAdr[] PROGMEM = "IP Adresse: ";
static const char TX_MDNSstart[] PROGMEM = "MDNS Responder gestartet";
static const char TX_HTTPstart[] PROGMEM = "HTTP Server gestartet";
static const char TX_TypeAPRef[] PROGMEM = "Bitte ein AP per Ziffer Auswählen: ";
static const char TX_TypePass[] PROGMEM = "Bitte das Passwort eingeben: ";