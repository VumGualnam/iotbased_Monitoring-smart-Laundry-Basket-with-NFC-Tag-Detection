/*
 * --------------------------------------------------------------------------------------------------------------------
 * Example sketch/program showing how to read new NUID from a PICC to serial.
 * --------------------------------------------------------------------------------------------------------------------
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid
 * 
 * Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a MFRC522 based RFID
 * Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Arduino IDE
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M). When
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial output
 * will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communication" messages
 * when removing the PICC from reading distance too early.
 * 
 * @license Released into the public domain.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino  Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101      Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST         9             5         D9        RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI        11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO        12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52       D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */

#include <SPI.h>
#include <MFRC522.h>

//#define SS_PIN 5
#define SS_PIN 21
#define RST_PIN 0
#define LED_PIN 2

#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16

// Defined pins to module RC522
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

//used in authentication
MFRC522::MIFARE_Key key; 

//authentication return status code
MFRC522::StatusCode status;
// Init array that will store new NUID 
byte nuidPICC[4];

void setup() { 
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  //prepare the key - all keys are set to FFFFFFFFFFFFh
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

//  while(!Serial.available()){}; // this block everything, do not use it
  Serial.println("I'm printing just fine");
  
  Serial.println("Approach your reader card...");
  Serial.println();
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
//  Serial.print(F("Using the following key:"));
//  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);

  pinMode(LED_PIN, OUTPUT);
}

void blinkingLED() {
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
}
void loop() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent()) {
    return;
  } else {
    Serial.println(F("A new card has been detected."));
    blinkingLED();
  }

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial()) {
    //    return;
  }

  readingData();

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && 
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  //Dump debug info about the card. PICC_HaltA() is automatically called.
  //prints the technical details of the card/tag
  rfid.PICC_DumpToSerial(&(rfid.uid));
  
//  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
//    rfid.uid.uidByte[1] != nuidPICC[1] || 
//    rfid.uid.uidByte[2] != nuidPICC[2] || 
//    rfid.uid.uidByte[3] != nuidPICC[3] ) 
    
  if(true) {
//    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
//      Serial.println(F("Start printing uidbyte size"));
//      Serial.print(typeof(rfid.uid.uidByte[i]).name());
//      Serial.println(F("End printing uidbyte size"));
    }

    //Show UID on serial monitor
    Serial.print("UID tag: ");
    String content = "";
    byte letter;
    for (byte i = 0; i < rfid.uid.size; i++) 
    {
      Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(rfid.uid.uidByte[i], HEX);
      content.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0": " "));
      content.concat(String(rfid.uid.uidByte[i], HEX));
    }
    Serial.println();
    Serial.print("Start: printing content: ");
    Serial.println();
    Serial.print(content);
    Serial.println();
    Serial.print("End: printing content: ");
    Serial.println();
    
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
//  else Serial.println(F("Card read previously."));

  

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

//reads data from card/tag
void readingData()
{

   for (byte i = 0; i < 16; i++) 
   {
      //buffer for read data
      byte buffer[SIZE_BUFFER] = {100};
    
      //the block to operate
//      byte block = 2;
      byte block = i;
      byte size = SIZE_BUFFER;//</p><p>  //authenticates the block to operate
    
    
      status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid)); //line 834 of MFRC522.cpp file
    
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Authentication failed for block: "));
        Serial.print(block);
        Serial.println(" ");
        Serial.println(rfid.GetStatusCodeName(status));
    //    digitalWrite(redPin, HIGH);
    //    delay(1000);
    //    digitalWrite(redPin, LOW);
    //    return;
      }
    
      //read data from block
      status = rfid.MIFARE_Read(block, buffer, &size);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Reading failed: "));
        Serial.println(rfid.GetStatusCodeName(status));
    //    digitalWrite(redPin, HIGH);
    //    delay(1000);
    //    digitalWrite(redPin, LOW);
    //    return;
      } 
    //  else{
    //      digitalWrite(greenPin, HIGH);
    //      delay(1000);
    //      digitalWrite(greenPin, LOW);
    //  }
      Serial.print(F("\nData from block ["));
      Serial.print(block);
      Serial.print(F("]: "));
    
     //prints read data
      for (uint8_t i = 0; i < MAX_SIZE_BLOCK; i++)
      {
          Serial.write(buffer[i]);
      }
      Serial.println(" ");
   }
  
  
}
