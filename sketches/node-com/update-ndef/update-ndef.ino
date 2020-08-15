#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)
#define READ_AS_NDEF (1)

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

char recByte;
char writeArr[37];
uint8_t sectArr[2];
int led = 6;
int sector = 12;
bool receiving = false;
bool recSec = false;
byte index = 0;
byte sectIndex = 0;
char sectorArr[960];

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);
  pinMode(led, OUTPUT);
  nfc.begin();

  nfc.SAMConfig();
}

void writeUpdate() {
//  int x = sectArr[0];
//  Serial.println(x);
  const char * writeData = writeArr;
uint8_t ndefprefix = 0;
uint8_t success;                          // Flag to check if there was an error with the PN532
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
bool authenticated = false;               // Flag to indicate if the sector is authenticated

//uint8_t key[6] = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 };
  uint8_t key[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };
//uint8_t key[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

      // check length to prevent corruption
    if (strlen(writeData) > 37){
      Serial.println("Write data is limited to 37 characters, so hurry tf up and sort that out");
      return;}
    
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success && sector != 0) 
  {
    nfc.PrintHex(uid, uidLength);
    Serial.println("");

    success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, (sector*4), 0, key);
    if (!success)
    {
      Serial.println("Unable to authenticate block...");
      delay(3000);
      return;
    }

    success = nfc.mifareclassic_WriteNDEFURI(sector, ndefprefix, writeData);
    if (success){
      Serial.println("Data written to sector");
    }else{
      Serial.println("Write failed \n");
    }
  }
  delay(1000);
  for(int i = 0; i < 37; i++){
  writeArr[0] = '\0';}
  for(int i = 0; i < 37; i++){
  writeArr[0] = '\0';}
  index = 0;
}

void loop(void) {
    if (Serial.available() > 0) {
    recByte = Serial.read();
    int byteCon = recByte;
//    Serial.println(byteCon);
    if(byteCon == -67 && recSec == false && receiving == false){
        recSec = true;
    }
    if(recSec == true && byteCon != -62 && byteCon != -67 && byteCon != -89 && receiving == false){
      char y = recByte;
      Serial.println(y);
    sectArr[sectIndex] = recByte;
    sectIndex++;
    sectArr[sectIndex] = '\0'; // NULL terminate the array;}
    if(byteCon == -89){
        sectArr[sectIndex] == '\0';
        recSec = false;
        sectIndex = 0;
        }
    }
    if(byteCon == 59 && receiving == false){
        receiving = true;
    }
    if(receiving == true){
    writeArr[index] = recByte;
    index++;
    writeArr[index] = '\0'; // NULL terminate the array;}
    if(byteCon == -30){
        writeArr[index] == '\0';
        receiving = false;
        writeUpdate();
        index = 0;
        }
    }
    }}
