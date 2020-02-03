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
//========================EP Setup========================================//
const int interruptPin = 10;//GPIO10, pin labeled 'SD3' ESP8266-12e NodeMCU
long timeStart; //time stamp for setup time out
int loopLock = 2;//extra logic to set priority to functiona in main
int inWifiKey =0;//if main is connected to wifi
int wifiPort =8080;//set port
bool setupEndpointGO = false;//setupmode flag
bool timeOUT =true;//flag for timeout startpoint

WiFiServer wifiserver(wifiPort);//server made for ESP endpoint modules only
String wifiINFO; //string for JSON information of WIFI network
//=======EP CLASS
// ep be numbered as they get paired, number determined name from array and ep codes
bool EPSave = false;//bool to add/update EP device
const char *epid = "abcde";//current ep/main name array
int epcodes[5];//array to hold ep name type
String setNames[5];//changeable name given from phone

//======

//ISR function call declared before setup, prevents compiling issues
ICACHE_RAM_ATTR void setupISR() {
  noInterrupts();  //prevents interrupt while in ISR
  Serial.println("Interrupt service routine ");
  setupEndpointGO = true;//turns on setup 'flag'
    }
//=========================================================================//


//===Firebase===//
#define FIREBASE_HOST "use different host"
#define FIREBASE_AUTH "secret key"

/* Set these to your desired softAP credentials. They are not configurable at runtime */
#define APSSID "ESP_main"
#define APPSK  "12345678"
//More definition
#define ColorNum 14               //total amount of colors
#define Device1 "led1"            //we can create device here
#define Device2 "led3"
#define AllDevice "All led"

const int led = 13;           //led pin
String ColorNames[ColorNum] = { "off","on/white","blue","lightblue","red",
                            "pink","magenta","lightmagenta","green",
                            "lightgreen","cyan","lightcyan","yellow","lightyellow"};
            
const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */
const char *myHostname = "esp8266";

/* Don't set this wifi credentials. They are configurated at runtime and stored on EEPROM */
char ssid[32] = "";
char password[32] = "";

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
ESP8266WebServer server(80);

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
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
  //output setup
  pinMode(led, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(14, OUTPUT);
  digitalWrite(led, 0); //indication led
  //
  
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  //ClearCredentials();
  Serial.println("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  //WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password,false);
 
  // WiFi.softAP(softAP_ssid,softAP_password);
  delay(500); // Without delay I've seen the IP address blank
  Serial.print("Main AP IP address: ");
  Serial.println(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  
  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  //server.on("/", handleRoot);
  server.on("/", handleWifi);
  server.on("/wifi", handleWifi);
  server.on("/wifisave", handleWifiSave);
//  server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
//  server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
//new
  server.on("/getip", HTTP_GET, handleGetIP);
  server.on("/receive", HTTP_POST, handleCommand);
//
  server.onNotFound(handleNotFound);
  
  server.begin(); // Web server start
  Serial.println("HTTP server started");
  //loadCredentials(); // Load WLAN credentials from network
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
          Serial.println("mDNS responder started");
          // Add service to MDNS-SD
          MDNS.addService("http", "tcp", 80);
        }

        //====FireBase===//
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
        
        
      } else if (s == WL_NO_SSID_AVAIL) {
        WiFi.disconnect();
      }
    }
    if (s == WL_CONNECTED) {
      MDNS.update();
    }
  }
  // Do work:
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  server.handleClient();
}
