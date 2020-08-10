#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

const char * writeData = " ";
uint8_t ndefprefix = 0;

#define NR_SHORTSECTOR          (32)    // Number of short sectors on Mifare 1K/4K
#define NR_LONGSECTOR           (8)     // Number of long sectors on Mifare 4K
#define NR_BLOCK_OF_SHORTSECTOR (4)     // Number of blocks in a short sector
#define NR_BLOCK_OF_LONGSECTOR  (16)    // Number of blocks in a long sector

// Determine the sector trailer block based on sector number
#define BLOCK_NUMBER_OF_SECTOR_TRAILER(sector) (((sector)<NR_SHORTSECTOR)? \
  ((sector)*NR_BLOCK_OF_SHORTSECTOR + NR_BLOCK_OF_SHORTSECTOR-1):\
  (NR_SHORTSECTOR*NR_BLOCK_OF_SHORTSECTOR + (sector-NR_SHORTSECTOR)*NR_BLOCK_OF_LONGSECTOR + NR_BLOCK_OF_LONGSECTOR-1))

// Determine the sector's first block based on the sector number
#define BLOCK_NUMBER_OF_SECTOR_1ST_BLOCK(sector) (((sector)<NR_SHORTSECTOR)? \
  ((sector)*NR_BLOCK_OF_SHORTSECTOR):\
  (NR_SHORTSECTOR*NR_BLOCK_OF_SHORTSECTOR + (sector-NR_SHORTSECTOR)*NR_BLOCK_OF_LONGSECTOR))

// The default Mifare Classic key
static const uint8_t keyb[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const uint8_t keyc[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };


void setup(void) {
  Serial.begin(115200);
  delay(10); // buffer

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    //action if board isn't found
    while (1); // halt
  }

  nfc.SAMConfig();
}

void loop(void) {
  uint8_t success;                          // Flag to check if there was an error with the PN532
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  bool authenticated = false;               // Flag to indicate if the sector is authenticated
  uint8_t blockBuffer[16];                  // Buffer to store block contents
  uint8_t blankAccessBits[3] = { 0xff, 0x07, 0x80 };
  uint8_t idx = 0;
  uint8_t numOfSector = 15;                 // mifare classic 1k
  
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) 
  { 
    Serial.println("Reformatting in progress..");

    for (idx = 0; idx < numOfSector; idx++)
    {
      success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, BLOCK_NUMBER_OF_SECTOR_TRAILER(idx), 1, (uint8_t *)keyb);
      if (!success)
      {
        Serial.print("Authentication failed for sector "); Serial.println(numOfSector);
        return;
      }
      if (idx == 16)
      {
        memset(blockBuffer, 0, sizeof(blockBuffer));
        if (!(nfc.mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 3, blockBuffer)))
        {
          Serial.print("Unable to write to sector "); Serial.println(numOfSector);
          return;
        }
      }
      if ((idx == 0) || (idx == 16))
      {
        memset(blockBuffer, 0, sizeof(blockBuffer));
        if (!(nfc.mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, blockBuffer)))
        {
          Serial.print("Unable to write to sector "); Serial.println(numOfSector);
          return;
        }
      }
      else
      {
        memset(blockBuffer, 0, sizeof(blockBuffer));
        if (!(nfc.mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 3, blockBuffer)))
        {
          Serial.print("Unable to write to sector "); Serial.println(numOfSector);
          return;
        }
        if (!(nfc.mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, blockBuffer)))
        {
          Serial.print("Unable to write to sector "); Serial.println(numOfSector);
          return;
        }
      }
      memset(blockBuffer, 0, sizeof(blockBuffer));
      if (!(nfc.mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 1, blockBuffer)))
      {
        Serial.print("Unable to write to sector "); Serial.println(numOfSector);
        return;
      }
      
      // Step 3: Reset both keys to 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      memcpy(blockBuffer, keyb, sizeof(keyb));
      memcpy(blockBuffer + 6, blankAccessBits, sizeof(blankAccessBits));
      blockBuffer[9] = 0x69;
      memcpy(blockBuffer + 10, keyb, sizeof(keyb));

      // Step 4: Write the trailer block
      if (!(nfc.mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)), blockBuffer)))
      {
        Serial.print("Unable to write trailer block of sector "); Serial.println(numOfSector);
        return;
      }
    }
    delay(50);
        // Try to format the card for NDEF data
    success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, 0, 0, keyb);
    if (!success)
    {
      Serial.println("Unable to authenticate block 0 to enable card formatting!");
      return;
    }
    success = nfc.mifareclassic_FormatNDEF();
    if (!success)
    {
      Serial.println("Unable to format the card for NDEF");
      return;
    }
      Serial.println("Card has been formatted for NDEF data using MAD1");
  }
  delay(500);
  for(int i = 2; i < 16; i++){
      success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, (i*4), 0, keyc);
    if (!success)
    {
      Serial.println("Unable to authenticate block...");
      return;
    }

    success = nfc.mifareclassic_WriteNDEFURI(i, ndefprefix, writeData);
    if (success){
      Serial.print("Format successful at sector ");Serial.print(i-1);Serial.println("");
    }else{
      Serial.println("Write failed \n");
    }
    delay(50); // buffer to prevent corruption 
  }
  
  delay(10000);
  Serial.flush();
}
