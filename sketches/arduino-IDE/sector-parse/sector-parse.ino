#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include "Keyboard.h"
#define READ_AS_NDEF (1)

#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

char sectorArr[64];

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

void setup(void) {
  Serial.begin(115200);
  delay(50); // for Leonardo/Micro/Zero

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("pn532 not found");
    while (1); // halt
  }else{Serial.println("ready to read..");}

  nfc.SAMConfig();
}

void loop(void) {
  uint8_t success;                          // Flag to check if there was an error with the PN532
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t currentblock;                     // Counter to keep track of which block we're on
  bool authenticated = false;               // Flag to indicate if the sector is authenticated
  char data[16];                         // Array to store block data during reads
  int sector = 1;
  int multiplier = 0;
  uint8_t key[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
//  uint8_t key[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  if (success) {

    if (uidLength == 4)
    {
      for (currentblock = sector*4; currentblock < (sector*4)+4; currentblock++)
      {
        if (nfc.mifareclassic_IsFirstBlock(currentblock)) authenticated = false;
        
        if (!authenticated)
        {
          if (currentblock == 0)
          {success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 1, key);}
          else
          {success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, currentblock, 1, key);}
          if (success){
            authenticated = true;
          }else{
            // trigger led
          }
        }
        if (!authenticated){
          return;}
        else{
          success = nfc.mifareclassic_ReadDataBlock(currentblock, data);
          if (success)
          {
            for(int i = 0; i < 16; i++){
              sectorArr[i+(multiplier*16)] = data[i];
            }
            multiplier++;
          }
        }
      }
    }
  }
  bool active = false;
  for(int i = 0; i < 64; i++){
    int holder = sectorArr[i];
    if(holder == -2 && active == true){
      delay(500);
      Keyboard.press(KEY_RETURN);
      Keyboard.release(KEY_RETURN);
      delay(2500);
      return;
    }else
    if(holder == 59 && active == false){
      active = true;
    }else
    if(active == true && holder != 59){
    Keyboard.print(sectorArr[i]);
    }
  }
} 
