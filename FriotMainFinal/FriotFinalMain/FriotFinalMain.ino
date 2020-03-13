//FirebaseESP8266.h must be included before ESP8266WiFi.h
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>//add json library
//=================== Define =============================//
//#define FIREBASE_HOST "use different host"
//#define FIREBASE_AUTH "secret key"
#define FIREBASE_HOST "piechart-9229a.firebaseio.com"
#define FIREBASE_AUTH "EJeC73NDL7TeVwIuGWjLTjrPtFReVwzSIcMZYWjb"
/* Set these to your desired softAP credentials. They are not configurable at runtime */
#define APSSID "ESP_main"
#define APPSK  "12345678"
//More definition
#define maxfamilyplace 8
#define ColorNum 14               //total amount of colors
#define Device0 "led0"            //we can create device here
#define Device2 "led1"
#define AllDevice "All led"
#define pinForInterrupt 10 //sd3
#define interruptPreventionPin 16 //d0
#define loopLockPriority 1
#define red 14 //d5
#define blue 13 //d7
#define green 12 //d6
#define output1 2 //d4
//========================EP Setup========================================//
const int interruptPin = pinForInterrupt;//GPIO10, pin labeled 'SD3' ESP8266-12e NodeMCU
const byte antiInterrupt = interruptPreventionPin;
long timeStart; //time stamp for setup time out
int loopLock = loopLockPriority-1;//extra logic to set priority to functiona in main
int inWifiKey =0;//if main is connected to wifi
int wifiPort =8080;//set wifi server port (non-http/browser)
bool setupEndpointGO = false;//setupmode flag
bool timeOUT =true;//flag for timeout startpoint
bool ledFlag = false;//flag with incoming epip to label as led

WiFiServer wifiserver(wifiPort);//server made for ESP endpoint modules only
String wifiINFO; //string for JSON information of WIFI network
//=======EP CLASS
// ep be numbered as they get paired, number determined name from array and ep codes
const int maxfp = maxfamilyplace;//max number of devices varialble
bool EPSave = false;//add/update EP device on main flag
bool EPFPUP = false;//flag to send new FP update to an EP
int tempEPPlace=0;//intializes temp FP values as 0
int epGroup[maxfp];//byte array saving ep group type; same index as LIP
String  EPIP="";// global temp string for EP IPs
char tempName[maxfp];
char tempEPIP[maxfp];
String LIP[maxfp]="";//string array to save EP IPs
char sendFP='0';//intializes char number variable to send to EP for FP updates
char *eek = "fp=0";//creates char array to add to post string to send EP update
//======Relay to EP===///
String Gcommand="";//global string to copy commands from phone to relay to EPs
byte epGCI = 0; //intializes group command indentifier as zero to send to EPs
//==============//


   HTTPClient http;    //Declare object of class HTTPClient
//ISR function call declared before setup, prevents compiling issues
ICACHE_RAM_ATTR void setupISR() {
  digitalWrite(antiInterrupt, 1); //disables use of the setup button, by setting both sides to HIGH
  noInterrupts();  //prevents interrupt while in ISR
  Serial.println("Interrupt service routine ");
  setupEndpointGO = true;//turns on setup 'flag'
    }
//=========================================================================//

//===Firebase===//



String ColorNames[ColorNum] = { "off","on/white","blue","lightblue","red",
                            "pink","magenta","lightmagenta","green",
                            "lightgreen","cyan","lightcyan","yellow","lightyellow"};
            
const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "espmain";

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
char ssid[32] = "";
char password[32] = "";

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);//static IP on main's AP, EPs connect to this
IPAddress netMsk(255, 255, 255, 0);

/** Should I connect to WLAN asap? */
boolean connect;

/** Last time I tried to connect to WLAN */
unsigned long lastConnectTry = 0;

/** Current WLAN status */
unsigned int status = WL_IDLE_STATUS;

//Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseJson json;


//====setup===//
void setup() {
  delay(1500);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(output1, OUTPUT);
  pinMode(antiInterrupt, OUTPUT);
  digitalWrite(antiInterrupt, 1); //indication led
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  //ClearCredentials(); ////for debugging
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), setupISR, FALLING);
  
  Serial.println("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  //WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password,false);
   loadCredentials();
  // WiFi.softAP(softAP_ssid,softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("Main AP IP address: ");
  Serial.println(WiFi.softAPIP());
  
 wifiserver.begin();
   Serial.print("Started wifiserver on port: ");
   Serial.println(wifiPort);
  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  
  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  //server.on("/", handleRoot);
  server.on("/", handleWifi);
  server.on("/wifi", handleWifi);
  server.on("/wifisave", handleWifiSave);
  server.on("/getip", HTTP_GET, handleGetIP);
  server.on("/receive", HTTP_POST, handleCommand);
  server.on("/epip", HTTP_POST, handlerecieveIP);
  server.on("/setupgo", HTTP_POST, handleSetupRequest);
//
  server.onNotFound(handleNotFound);
  
  server.begin(); // Web server start
  Serial.println("HTTP server started");
  connect = strlen(ssid) > 0; // Request WLAN connect if there is a SSID
  dnsServer.start(DNS_PORT, "*", apIP);

}

void loop() {
  if (connect) {
    Serial.println("Connect requested");
    connect = false;
    connectWifi();
    lastConnectTry = millis();
  }
  {
    unsigned int s = WiFi.status();
    if (s == 0 && millis() > (lastConnectTry + 60000)) {
      /* If WLAN disconnected and idle try to connect */
      /* Don't set retry time too low as retry interfere the softAP operation */
      connect = true;
    }
    if (status != s) { // WLAN status change
      Serial.print("Status: ");
      Serial.println(s);
      status = s;
      if (s == WL_CONNECTED) {
          digitalWrite(antiInterrupt, 1); //disables use of the setup button
        /* Just connected to WLAN */
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        
        // Setup MDNS responder
        if (!MDNS.begin(myHostname)) {
          Serial.println("Error setting up MDNS responder!");
        } else {
          Serial.println("mDNS responder started\n connecting to firebase");
          // Add service to MDNS-SD
          MDNS.addService("http", "tcp", 80);
        }

        //====FireBase disabled===//
        Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
        Firebase.reconnectWiFi(true);

        //Set database read timeout to 1 minute (max 15 minutes)
        Firebase.setReadTimeout(firebaseData, 1000 * 60);
        //tiny, small, medium, large and unlimited.
        //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
        Firebase.setwriteSizeLimit(firebaseData, "tiny");
        
        //set IP with debug info in serial output
        if (Firebase.setString(firebaseData,  "/Wifi_IP" , toStringIp(WiFi.localIP())))
        {
          Serial.println("PASSED");
          Serial.println("PATH: " + firebaseData.dataPath());
          Serial.println("TYPE: " + firebaseData.dataType());
          Serial.println("ETag: " + firebaseData.ETag());
          Serial.print("VALUE: ");
          printResult(firebaseData);
          Serial.println("------------------------------------");
          Serial.println();
         }
         else
         {
            Serial.println("FAILED");
            Serial.println("REASON: " + firebaseData.errorReason());
            Serial.println("------------------------------------");
            Serial.println();
           }
        //===FireBase==//
        Serial.println("Redi2fullyparty");
          digitalWrite(antiInterrupt, 0); //enables use of the setup button
        
      } else if (s == WL_NO_SSID_AVAIL) {
        WiFi.disconnect();
      }
    }
    if (s == WL_CONNECTED) {
      MDNS.update();
    }
  }


  
    if (setupEndpointGO&loopLock<loopLockPriority){//if in setting up mode, may modify to a while loop
      if(wifiINFO=="")//fills out wifi json info only once
      loadWifiInfo();//goes to json void for wifi info
      setupMode();
    }//end setup mode if statement
    if(EPFPUP&loopLock<loopLockPriority+1) {
        endSetup();
      updateAIP();
      Serial.println("setup ended in main loop");
    }
 



  
 while (loopLock>loopLockPriority) { //doesnot handle http while outputing to mutiple EPS
   if (Gcommand=="")
      Serial.println("no global command");
   if(ledFlag &epGroup[loopLock-2]==2){
     toEP(47+loopLock, Gcommand);
     delay(500);
    }
   if(ledFlag==false){
     toEP(47+loopLock, Gcommand);
    delay(500);
   }
   Serial.println();
  loopLock--;
  Serial.print("LoopLock in main is now:");
   Serial.println(loopLock);
   if(loopLock == 1)
   {
    Serial.print("end cycle");
     Gcommand="";
     ledFlag=false;
     return;
     }
   Serial.print("next device");
   Serial.println(loopLock-1);
  }
  
  
  dnsServer.processNextRequest();
  //HTTP
  server.handleClient();
}
