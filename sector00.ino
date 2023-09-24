/*
 * Initial Author: ryand1011 (https://github.com/ryand1011)
 *
 * Reads data written by a program such as "rfid_write_personal_data.ino"
 *
 * See: https://github.com/miguelbalboa/rfid/tree/master/examples/rfid_write_personal_data
 *
 * Uses MIFARE RFID card using RFID-RC522 reader
 * Uses MFRC522 - Library
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
*/

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         22//9           // Configurable, see typical pin layout above
#define SS_PIN          21//10          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

//*****************************************************************************************//
void setup() {
  Serial.begin(9600);                                           // Initialize serial communications with the PC
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card
  Serial.println(F("Read personal data on a MIFARE PICC:"));    //shows in serial that it is ready to read
}

/*
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

//*****************************************************************************************//
void loop() {

  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
//  MFRC522::MIFARE_Key key = {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5};
  key.keyByte[0] = 0xa0;
  key.keyByte[1] = 0xa1;
  key.keyByte[2] = 0xa2;
  key.keyByte[3] = 0xa3;
  key.keyByte[4] = 0xa4;
  key.keyByte[5] = 0xa5;
//  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("**Card Detected:**"));
  byte buffer[18];
  block = 0;

  for (byte block = 0; block < 4; block++) { // block < 16 does not work
    // Serial.println(F("Authenticating using key A..."));
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK) {
           Serial.print(F("PCD_Authenticate() failed: "));
           Serial.println(mfrc522.GetStatusCodeName(status));
    //      return false;
      }

      // Show the whole sector as it currently is
      Serial.println(F("Current data in sector:"));
//      mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
      mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, block);
      Serial.println();
      
      // Read block
      byte byteCount = sizeof(buffer);
      status = mfrc522.MIFARE_Read(block, buffer, &byteCount);
      if (status != MFRC522::STATUS_OK) {
           Serial.print(F("MIFARE_Read() failed: "));
           Serial.println(mfrc522.GetStatusCodeName(status));
      }
      else {
          // Successful read
    //      result = true;
          Serial.print(F("Success with key:"));
    //      dump_byte_array(key.keyByte, byteCount);
          Serial.print(F("Block ")); Serial.print(block); Serial.print(F(":"));
          dump_byte_array(buffer, byteCount);
    //      dump_byte_array(buffer, 16);
          Serial.println();
  }
  }
  




  
  //-------------------------------------------

//  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

//  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

//  //-------------------------------------------
//
//  Serial.print(F("Name: "));
//
//  byte buffer1[18];
//
//  block = 4;
//  len = 18;
//
//  //------------------------------------------- GET FIRST NAME
//  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
//  if (status != MFRC522::STATUS_OK) {
//    Serial.print(F("Authentication failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
//    return;
//  }
//
//  status = mfrc522.MIFARE_Read(block, buffer1, &len);
//  if (status != MFRC522::STATUS_OK) {
//    Serial.print(F("Reading failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
//    return;
//  }
//
//  //PRINT FIRST NAME
//  for (uint8_t i = 0; i < 16; i++)
//  {
//    if (buffer1[i] != 32)
//    {
//      Serial.write(buffer1[i]);
//    }
//  }
//  Serial.print(" ");
//
//  //---------------------------------------- GET LAST NAME
//
//  byte buffer2[18];
//  block = 1;
//
//  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
//  if (status != MFRC522::STATUS_OK) {
//    Serial.print(F("Authentication failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
//    return;
//  }
//
//  status = mfrc522.MIFARE_Read(block, buffer2, &len);
//  if (status != MFRC522::STATUS_OK) {
//    Serial.print(F("Reading failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
//    return;
//  }
//
//  //PRINT LAST NAME
//  for (uint8_t i = 0; i < 16; i++) {
//    Serial.write(buffer2[i] );
//  }
//
//
//  //----------------------------------------
//
//  Serial.println(F("\n**End Reading**\n"));
//
//  delay(1000); //change value if you want to read cards faster

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
//*****************************************************************************************//
