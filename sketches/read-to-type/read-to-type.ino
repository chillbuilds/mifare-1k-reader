#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "Keyboard.h"
#define READ_AS_NDEF (1)

// define pins for SPI communication
#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

// Use this line for a breakout with a SPI connection:
Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

void setup(void) {
  Serial.begin(115200);
  delay(100); // for Leonardo/Micro/Zero

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("pn532 not found");
    while (1); // halt
  }
  Serial.println("ready to read..");

  nfc.SAMConfig();
}

void loop(void) {
  uint8_t success;                          // Flag to check if there was an error with the PN532
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t currentblock;                     // Counter to keep track of which block we're on
  bool authenticated = false;               // Flag to indicate if the sector is authenticated
  char data[16];                         // Array to store block data during reads
  char password[10];
  uint8_t keyuniversal[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
//  uint8_t keyuniversal[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {
//    int uidStore[] = {uid[0], uid[1], uid[2], uid[3]};
//    int xm1[] = {5, 37, 237, 60};
//    for(int i = 0; i < 3; i++){
//    if (uidStore[i] != xm1[i]){
//      return;
//    }
//    }
    if (uidLength == 4)
    {
      // Now we try to go through all 16 sectors (each having 4 blocks)
      // authenticating each sector, and then dumping the blocks
      for (currentblock = 4; currentblock < 8; currentblock++)
      {
        // Check if this is a new block so that we can reauthenticate
        if (nfc.mifareclassic_IsFirstBlock(currentblock)) authenticated = false;

        // If the sector hasn't been authenticated, do so first
        if (!authenticated)
        {
          // Starting of a new sector ... try to to authenticate
          if (currentblock == 0)
          {
              success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 1, keyuniversal);
          }
          else
          {
              success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 1, keyuniversal);
          }
          if (success)
          {
            authenticated = true;
          }
          else
          {
            // trigger led
          }
        }
        // If we're still not authenticated just skip the block
        if (!authenticated)
        {
          Serial.print("Block ");Serial.print(currentblock, DEC);Serial.println(" unable to authenticate");
        return;}
        else
        {
          // Authenticated ... we should be able to read the block now
          // Dump the data into the 'data' array
          success = nfc.mifareclassic_ReadDataBlock(currentblock, data);
          if (success)
          {
            //we're throwing the last 7 chars from the first block in the sector into our password array
            if(currentblock == 4){
              for(int i = 9; i < 16; i++){
                password[i - 9] = data[i];
              }
            // and the first 3 from the next block to complete the password storage
            }else if(currentblock == 5){
              for(int i = 0; i < 3; i++){
                Serial.print(data[i]);
                password[i + 7] = data[i];
              }
            }
          }
        }
      }
    }
  }
  // loop through stored password array ad type out each character over usb
  for(int i = 0; i < 10; i++){
  Keyboard.print(password[i]);
  }
  // delay required after keypresses before hitting enter
  delay(600);
  Keyboard.press(KEY_RETURN);
  Keyboard.release(KEY_RETURN);
  //wait 2 and a half seconds before reading again
  //shorten this buffer if you'd like, but I'd leave it at at least 1000 (1 sec)
  delay(2500);
  
}
