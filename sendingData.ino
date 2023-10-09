/*
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 * 
 * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
 * 
 * Released into the public domain.
 * ----------------------------------------------------------------------------
 * Example sketch/program which will try the most used default keys listed in 
 * https://code.google.com/p/mfcuk/wiki/MifareClassicDefaultKeys to dump the
 * block 0 of a MIFARE RFID card using a RFID-RC522 reader.
 * 4
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 *
 */
#include <HTTPClient.h> 
#include <ArduinoJson.h>

#include <WiFiManager.h>
#include <strings_en.h>
#include <wm_consts_en.h>
#include <wm_strings_en.h>
#include <wm_strings_es.h>
#include <SPI.h>
#include <MFRC522.h>

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "SPIFFS.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

// Set LED GPIO
const int ledPin = 2;
// Stores LED state

String ledState;

//........................................................

#define RST_PIN         22 //9           // Configurable, see typical pin layout above
#define SS_PIN          21 //10          // Configurable, see typical pin layout above
#define TRIG_PIN 13
#define ECHO_PIN 14

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

// Number of known default keys (hard-coded)
// NOTE: Synchronize the NR_KNOWN_KEYS define with the defaultKeys[] array
#define NR_KNOWN_KEYS   8
// Known keys, see: https://code.google.com/p/mfcuk/wiki/MifareClassicDefaultKeys
byte knownKeys[NR_KNOWN_KEYS][MFRC522::MF_KEY_SIZE] =  {
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, // FF FF FF FF FF FF = factory default
    {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5}, // A0 A1 A2 A3 A4 A5
    {0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5}, // B0 B1 B2 B3 B4 B5
    {0x4d, 0x3a, 0x99, 0xc3, 0x51, 0xdd}, // 4D 3A 99 C3 51 DD
    {0x1a, 0x98, 0x2c, 0x7e, 0x45, 0x9a}, // 1A 98 2C 7E 45 9A
    {0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7}, // D3 F7 D3 F7 D3 F7
    {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}, // AA BB CC DD EE FF
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // 00 00 00 00 00 00
};


/*
 * Helper functions routine to dump a byte array as hex values to Serial.
 */

//.................start: wifi stuff..................
// Replaces placeholder with LED state value
String processor(const String& var) {
  if(var == "STATE") {
    if(digitalRead(ledPin)) {
      ledState = "ON";
    }
    else {
      ledState = "OFF";
    }
    return ledState;
  }
  return String();
}

void  initializeWifiSetup() {
  initSPIFFS();
  
  // Load values saved in SPIFFS
  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);
  ip = readFile(SPIFFS, ipPath);
  gateway = readFile (SPIFFS, gatewayPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);
    
 if(initWiFi()) {
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });
    server.serveStatic("/", SPIFFS, "/");
    
    // Route to set GPIO state to HIGH
    server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
      digitalWrite(ledPin, HIGH);
      request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });

    // Route to set ESP32 to sleep and when tag is detected, woke up
    // server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    //     request->send(200, "text/plain", "Going to sleep...");
    //     delay(100);  // Allow time for response to be sent
    //     esp_sleep_enable_ext0_wakeup((gpio_num_t)RST_PIN, 1);  // 1 = HIGH
    //     esp_deep_sleep_start();
    // });

     
    // Route to set GPIO state to LOW
    server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
      digitalWrite(ledPin, LOW);
      request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });
    server.begin();
  }
  else {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("laundry-basket", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/wifimanager.html", "text/html");
    });
    
    server.serveStatic("/", SPIFFS, "/");
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(SPIFFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(SPIFFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(SPIFFS, ipPath, ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            writeFile(SPIFFS, gatewayPath, gateway.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart();
    });
    server.begin();
  }

  
}
// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}


// Initialize WiFi
bool initWiFi() {
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());


  if (!WiFi.config(localIP, localGateway, subnet)){
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  return true;
}
//..........................................end: wifi helper functions................

void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

void dump_byte2ascii(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " " : " ");
//        Serial.print(buffer[i], DEC);
        Serial.print((char)buffer[i]);
    }
}

//distance measurement 
float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

 

//  long duration = pulseIn(ECHO_PIN, HIGH);

    // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHO_PIN, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);

  return distanceCm;

//  float distance = (float)duration * 0.0343 / 2;
//  return distance;
}



/*
 * Try using the PICC (the tag/card) with the given key to access block 0.
 * On success, it will show the key details, and dump the block data on Serial.
 *
 * @return true when the given key worked, false otherwise.
 */
bool try_key(MFRC522::MIFARE_Key *key, byte block)
{
    bool result = false;
    byte buffer[18];
//    byte block = 0;
//    byte block = block;
    MFRC522::StatusCode status;

    // Serial.println(F("Authenticating using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        // Serial.print(F("PCD_Authenticate() failed: "));
        // Serial.println(mfrc522.GetStatusCodeName(status));
        return false;
    }

    // Read block
    byte byteCount = sizeof(buffer);
    status = mfrc522.MIFARE_Read(block, buffer, &byteCount);
    if (status != MFRC522::STATUS_OK) {
        // Serial.print(F("MIFARE_Read() failed: "));
        // Serial.println(mfrc522.GetStatusCodeName(status));
    }
    else {
        // Successful read
        result = true;
        Serial.print(F("Success with key:"));
        dump_byte_array((*key).keyByte, MFRC522::MF_KEY_SIZE);
        Serial.println();
        // Dump block data
        Serial.print(F("Block ")); Serial.print(block); Serial.print(F(":"));
        dump_byte_array(buffer, 16);
        Serial.println();
        dump_byte2ascii(buffer, 16);
        Serial.println();
    }
    Serial.println();

//    mfrc522.PICC_HaltA();       // Halt PICC
//    mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
    return result;
}


void sendRfidData(String tag_id, String tag_content) {
    HTTPClient http; 
    http.begin("http://192.168.225.121:5003/rfid");  
    http.addHeader("Content-Type", "application/json");         
     
    StaticJsonDocument<200> doc;
    // Add values in the document
    //
    doc["tag_id"] = "1351824120";
    doc["tag_content"] = "Cool";
   
//    // Add an array.
//    //
//    JsonArray data = doc.createNestedArray("data");
//    data.add(48.756080);
//    data.add(2.302038);
     
    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);   //Send the actual POST request
    if(httpResponseCode>0){
    Serial.print("httpResponseCode: ");
    Serial.println(httpResponseCode); 

   }else{
  
      Serial.println("Error on sending POST");
  
     }
  
     http.end();  //Free resources
  
}
// ..................

/*
 * Initialize.
 */
void setup() {
    Serial.begin(9600);         // Initialize serial communications with the PC
//    Serial.begin(921600); 
    while (!Serial);            // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();                // Init SPI bus
    mfrc522.PCD_Init();         // Init MFRC522 card
    Serial.println(F("Try the most used default keys to print block 0 of a MIFARE PICC."));

    pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an Output
    pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an Input

    initializeWifiSetup();

    
}



/*
 * Main loop.
 */
void loop() {
  Serial.print("\nWifi Status: ");
  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
    Serial.println(WiFi.status());
    Serial.println(ip);
    Serial.println(ssid);
    Serial.println(pass);
    Serial.println(gateway);
   HTTPClient http;   


    sendRfidData("test", "test");
//   http.begin("http://192.168.1.92:8090/post");
   http.begin("http://192.168.225.121:5003/sensor");
   http.addHeader("Content-Type", "text/plain");             
   char buf[6];
   dtostrf(measureDistance(),5,2,buf);
   int httpResponseCode = http.POST(String(buf));   //Send the actual POST request


   if(httpResponseCode>0){
    Serial.print("httpResponseCode: ");
    Serial.println(httpResponseCode); 

   }else{

    Serial.println("Error on sending POST");

   }

   http.end();  //Free resources

 }else{

    Serial.println("Error in WiFi connection");   

 }

  delay(20000);  //Send a request every 10 seconds
//    if ((WiFi.status() == WL_CONNECTED)) { 
//      Serial.println(WiFi.status());
//      Serial.println(ip);
//      HTTPClient http;
//   
////      http.begin("http://192.168.1.92:8090/hello");
//      http.begin("http://192.168.95.121:5003/data");
//      int httpCode = http.GET();                                        
//   
//      if (httpCode > 0) {
//   
//          String payload = http.getString();
//          Serial.println(httpCode);
//          Serial.println(payload);
//        }
//   
//      else {
//        Serial.println("Error on HTTP request");
//      }
//   
//      http.end();
//    }
//   
//    delay(20000);
    
    Serial.print("\ndistance: ");
    Serial.println(measureDistance());


    
    delay(1000);
    
    
    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if ( ! mfrc522.PICC_IsNewCardPresent())
        return;

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;

    // Show some details of the PICC (that is: the tag/card)
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));
    
    // Try the known default keys
    MFRC522::MIFARE_Key key;
    for (byte block = 0; block < 64; block ++) {
      for (byte k = 0; k < NR_KNOWN_KEYS; k++) {
        // Copy the known key into the MIFARE_Key structure
        for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++) {
            key.keyByte[i] = knownKeys[k][i];
        }
        // Try the key
        if (try_key(&key, block)) {
            // Found and reported on the key and block,
            // no need to try other keys for this PICC
            break;
        }
        
        // http://arduino.stackexchange.com/a/14316
        if ( ! mfrc522.PICC_IsNewCardPresent())
            break;
        if ( ! mfrc522.PICC_ReadCardSerial())
            break;
      }
    }
    
}
