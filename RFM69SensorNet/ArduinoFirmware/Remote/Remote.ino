//******************************************************************************
// Remote Node with RFM69 PacketCommand Interface and Soil Moisture sensor
//------------------------------------------------------------------------------

#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
#include <SHT1x.h>
#include <SerialCommand.h>
#include <PacketCommand.h>
#include "network_config.h"
#include "RFM69Interface.h"

#define DEBUG
//******************************************************************************
// Globals
//------------------------------------------------------------------------------
// Serial interface (for debugging on remote)
#define MAX_SERIAL_COMMANDS 10
#define SERIAL_BAUD   9600

SerialCommand sCmd(Serial, MAX_SERIAL_COMMANDS);

// Non-volatile Data Store
#ifdef __AVR_ATmega1284P__
  #define LED           15 // Moteino MEGAs have LEDs on D15
  #define FLASH_SS      23 // and FLASH SS on D23
#else
  #define LED           9 // Moteinos have LEDs on D9
  #define FLASH_SS      8 // and FLASH SS on D8
#endif

//////////////////////////////////////////
// flash(SPI_CS, MANUFACTURER_ID)
// SPI_CS          - CS pin attached to SPI flash chip (8 in case of Moteino)
// MANUFACTURER_ID - OPTIONAL, 0x1F44 for adesto(ex atmel) 4mbit flash
//                             0xEF30 for windbond 4mbit flash
//////////////////////////////////////////
SPIFlash flash(FLASH_SS, 0xEF30);

//------------------------------------------------------------------------------
// SHT10 Soil moisture sensor
const uint8_t sht10DataPin  = 6;
const uint8_t sht10ClockPin = 7;
SHT1x sht10(sht10DataPin, sht10ClockPin);

//******************************************************************************
// misc. Arduino pins
const uint8_t arduinoLEDPin = 9;  // Moteinos have LEDs on D9

void flash_read(node_flash::datafield_e fieldnum, uint8_t& valByRef);
void flash_read(node_flash::datafield_e fieldnum, uint8_t* buffer, size_t len);

//******************************************************************************
void setup() {
  //----------------------------------------------------------------------------
  // configure Arduino pins
  pinMode(arduinoLEDPin,OUTPUT);
  digitalWrite(arduinoLEDPin, LOW);
  //----------------------------------------------------------------------------
  // configure the PacketCommand parser
  pCmd_RFM69.addCommand((byte*) "\x41","LED.ON",  LED_ON_pCmd_action_handler);  // Turns LED on   ("\x41" == "A")
  pCmd_RFM69.addCommand((byte*) "\x42","LED.OFF", LED_OFF_pCmd_action_handler); // Turns LED off  ("\x42" == "B")
  //----------------------------------------------------------------------------
  // configure the SerialCommand parser
  //sCmd.addCommand("IDN?", IDN_sCmd_query_handler);
  sCmd.addCommand("LED.ON", LED_ON_sCmd_action_handler);
  sCmd.addCommand("LED.OFF", LED_OFF_sCmd_action_handler);
  sCmd.addCommand("FLASH.DUMP?", FLASH_DUMP_sCmd_query_handler);
  sCmd.addCommand("FLASH.READ?", FLASH_READ_sCmd_query_handler);
  sCmd.addCommand("FLASH.WRITE", FLASH_WRITE_sCmd_action_handler);
  sCmd.addCommand("SHT.READ?", SHT_READ_sCmd_query_handler);
  // configure the serial port
  Serial.begin(SERIAL_BAUD);
  delay(10);
  
  //start up the flash memory
  #ifdef DEBUG
  Serial.print("Initializing flash memory...");
  #endif
  if (flash.initialize()){
    #ifdef DEBUG
    Serial.println("Init OK!");
    #endif
  }
  else{
    #ifdef DEBUG
    Serial.println("Init FAIL!");
    #endif
  }
  //----------------------------------------------------------------------------
  // configure as remote device
  RFM69Interface_remote_default_config();
  //----------------------------------------------------------------------------
  // start up the radio interface
  RFM69Interface_start();
}
//******************************************************************************
void loop() {
  //----------------------------------------------------------------------------
  // handle feeding packets to pCmd parser and dispatching to commands
  RFM69Interface_process_incoming();
  //
  sCmd.readSerial();
  
}

//******************************************************************************
// pCmd Handlers
void LED_ON_pCmd_action_handler(PacketCommand& this_pCmd) {
  #ifdef DEBUG
  Serial.println(F("LED_ON_pCmd_action_handler"));
  #endif
  digitalWrite(arduinoLEDPin, HIGH);
}
  
void LED_OFF_pCmd_action_handler(PacketCommand& this_pCmd) {
  #ifdef DEBUG
  Serial.println(F("LED_OFF_pCmd_action_handler"));
  #endif
  digitalWrite(arduinoLEDPin, LOW);
}


void SHT_READ_pCmd_query_handler(PacketCommand& this_pCmd) {
  #ifdef DEBUG
  Serial.println(F("SHT_READ_pCmd_query_handler"));
  #endif
}
//******************************************************************************
// sCmd Handlers
void IDN_sCmd_query_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("IDN_sCmd_query_handler"));
  #endif
  Serial.print(F("RFM69 Node #"));
  uint8_t idn;         //formatted to print as string
  flash_read(node_flash::IDN, idn); //updated by reference
  Serial.print(idn);
  
}

void LED_ON_sCmd_action_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("LED_ON_sCmd_action_handler"));
  #endif
  digitalWrite(arduinoLEDPin, HIGH);
}
  
void LED_OFF_sCmd_action_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("LED_OFF_sCmd_action_handler"));
  #endif
  digitalWrite(arduinoLEDPin, LOW);
}

void FLASH_DUMP_sCmd_query_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("FLASH_DUMP_sCmd_query_handler"));
  #endif
  int counter = 0;

  while(counter<=256){
    Serial.print(flash.readByte(counter++), HEX);
    Serial.print('.');
  }
  Serial.println();
}

#define FLASH_READ_BUFFER_SIZE 64
void FLASH_READ_sCmd_query_handler(SerialCommand this_sCmd) {
  uint8_t buffer[FLASH_READ_BUFFER_SIZE];
  #ifdef DEBUG
  this_sCmd.println(F("FLASH_READ_sCmd_query_handler"));
  #endif
  char* arg1 = this_sCmd.next();
  if (arg1 == NULL){
    this_sCmd.println(F("###ERROR FLASH.READ require one or two arguments (fieldnum, [len])"));
  }
  else{
    char* arg2 = this_sCmd.next();
    if (arg2 == NULL){
      //interpet value as ASCII integer
      uint32_t fieldnum = atoi(arg1);
      uint8_t valByRef;
      flash_read((node_flash::datafield_e) fieldnum, valByRef);
      Serial.print(valByRef);
    }
    else{
      size_t len = min(atoi(arg2), FLASH_READ_BUFFER_SIZE); //limit read length
      //interpret string of bytes as uint8_t array
      flash_read((node_flash::datafield_e) fieldnum, buffer, len)
      for(int i=0; i < len; i++){
        Serial.write(buffer[i]);
      }
    }
  }
}

void FLASH_WRITE_sCmd_action_handler(SerialCommand this_sCmd) {
  uint8_t buffer[FLASH_READ_BUFFER_SIZE];
  #ifdef DEBUG
  this_sCmd.println(F("FLASH_WRITE_sCmd_query_handler"));
  #endif
  char* arg1 = this_sCmd.next();
  if (arg1 == NULL){
    this_sCmd.println(F("###ERROR FLASH.WRITE require one or two arguments (fieldnum, [len])"));
  }
  else{
    char* arg2 = this_sCmd.next();
    if (arg2 == NULL){
      //interpet value as ASCII integer
      uint32_t fieldnum = atoi(arg1);
      uint8_t valByRef;
      flash_read((node_flash::datafield_e) fieldnum, valByRef);
      Serial.print(valByRef);
    }
    else{
      size_t len = min(atoi(arg2), FLASH_READ_BUFFER_SIZE); //limit read length
      //interpret string of bytes as uint8_t array
      flash_read((node_flash::datafield_e) fieldnum, buffer, len)
      for(int i=0; i < len; i++){
        Serial.write(buffer[i]);
      }
    }
  }
}

void SHT_READ_sCmd_query_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("SHT_READ_sCmd_query_handler"));
  #endif
  float temp_C   = sht10.readTemperatureC();
  float humid_RH = sht10.readHumidity();
  this_sCmd.print(temp_C,3);this_sCmd.print(",");
  this_sCmd.println(humid_RH,3);
}
//******************************************************************************
// Helper Functions
void flash_read(node_flash::datafields fieldnum, uint8_t& valByRef){
  valByRef = flash.readByte(fieldnum);
}

void flash_read(node_flash::datafields fieldnum, uint8_t* buffer, size_t len){
  for(size_t i=0; i < len; i++){
    buffer[i] = flash.readByte(fieldnum + i);
  }
}

void flash_write(node_flash::datafields fieldnum, uint8_t val){
  flash.writeByte(fieldnum,val);
}

void flash_write(node_flash::datafields fieldnum, uint8_t* buffer, size_t len){
  for(size_t i=0; i < len; i++){
    flash.writeByte((uint8_t) fieldnum + i,buffer[i]);
  }
}
