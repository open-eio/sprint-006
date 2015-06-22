//******************************************************************************
// Remote Node with RFM69 PacketCommand Interface and Soil Moisture sensor
//------------------------------------------------------------------------------

#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
#include <SHT1x.h>
#include <SerialCommand.h>
#include <PacketCommand.h>
#include "flash_config.h"
#include "RFM69Interface.h"

#define DEBUG
//******************************************************************************
// Globals
//------------------------------------------------------------------------------
// Current Radio Configuration
struct Node_Config node_config;

// Serial interface (for debugging on remote)
#define MAX_SERIAL_COMMANDS 10
#define SERIAL_BAUD   115200

SerialCommand sCmd(Serial, MAX_SERIAL_COMMANDS);

// Board specific pins
#ifdef __AVR_ATmega1284P__
  #define LED           15 // Moteino MEGAs have LEDs on D15
#else
  #define LED           9  // Moteinos have LEDs on D9
#endif

//------------------------------------------------------------------------------
// SHT10 Soil moisture sensor
// Blue Wire   -> Data
// Yellow Wire -> Clock
// Red Wire    -> 3-5 V
// Green Wire  -> Ground

const uint8_t sht10DataPin  = 3; //blue wire
const uint8_t sht10ClockPin = 4; //yellow wire

SHT1x sht10(sht10DataPin, sht10ClockPin);

//******************************************************************************
// misc. Arduino pins
const uint8_t arduinoLEDPin = 9;  // Moteinos have LEDs on D9

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
  sCmd.addCommand("IDN?", IDN_sCmd_query_handler);
  sCmd.addCommand("LED.ON",  LED_ON_sCmd_action_handler);
  sCmd.addCommand("LED.OFF", LED_OFF_sCmd_action_handler);
  sCmd.addCommand("FLASH.DUMP?", FLASH_DUMP_sCmd_query_handler);
  sCmd.addCommand("FLASH.ERASE", FLASH_ERASE_sCmd_action_handler);
  sCmd.addCommand("FLASH.READ?", FLASH_READ_sCmd_query_handler);
  sCmd.addCommand("FLASH.WRITE", FLASH_WRITE_sCmd_action_handler);
  sCmd.addCommand("RFM69.CONFIG?",    RFM69_CONFIG_sCmd_query_handler);
  sCmd.addCommand("RFM69.SET_CONFIG", RFM69_SET_CONFIG_sCmd_action_handler);
  sCmd.addCommand("RFM69.NODEID",     RFM69_NODEID_sCmd_action_handler);
  //sCmd.addCommand("RFM69.NETWORKID", RFM69_NETWORKID_sCmd_action_handler);
  sCmd.addCommand("SHT.READ?", SHT_READ_sCmd_query_handler);
  sCmd.setDefaultHandler(UNRECOGNIZED_sCmd_default_handler);
  // configure the serial port
  Serial.begin(SERIAL_BAUD);
  delay(10);
  
  //start up the flash memory
  #ifdef DEBUG
  Serial.print("# Initializing flash memory...");
  #endif
  if (node_flash::initialize()){
    #ifdef DEBUG
    Serial.println("Init OK!");
    #endif
  }
  else{
    #ifdef DEBUG
    Serial.println("Init FAIL!");
    #endif
  }
  #ifdef DEBUG
  Serial.print(F("# Reading config from flash...\n"));
  #endif
  node_flash::get_config(node_config);
  #ifdef DEBUG
  Serial.print(F("# nodeID: "));    Serial.println(node_config.nodeID);
  Serial.print(F("# networkID: ")); Serial.println(node_config.networkID);
  Serial.print(F("# gatewayID: ")); Serial.println(node_config.gatewayID);
  Serial.print(F("# frequency: ")); Serial.println(node_config.frequency);
  Serial.print(F("# is_RFM69HW: "));Serial.println(node_config.is_RFM69HW);
  Serial.print(F("# encryptkey: "));
  for(size_t i=0; i < ENCRYPTKEY_LEN; i++){
    Serial.print(node_config.encryptkey[i]);
  }
  Serial.println();
  #endif
  //----------------------------------------------------------------------------
  // configure as remote device
  RFM69Interface_node_configure(node_config);
  //----------------------------------------------------------------------------
  // start up the radio interface
  RFM69Interface_start();
  
  #ifdef DEBUG
  char buff[50];
  uint32_t freq_val;
  switch(node_config.frequency){
    case RF69_433MHZ:
      freq_val = 433;
      break;
    case RF69_868MHZ:
      freq_val = 868;
      break;
    case RF69_915MHZ:
      freq_val = 915;
      break;
    default:
      freq_val = 0;
  }
  sprintf(buff, "\n#Listening at %d Mhz...", freq_val);
  Serial.println(buff);
  #endif
  
  //Flash LED to signify that we are ready!
  digitalWrite(arduinoLEDPin, HIGH);
  delay(500);
  digitalWrite(arduinoLEDPin, LOW);
}

//******************************************************************************
void loop() {
  //----------------------------------------------------------------------------
  // handle feeding packets to pCmd parser and dispatching to commands
  RFM69Interface_process_incoming();
  // hanlder commands from serial port
  sCmd.readSerial();
}

//******************************************************************************
// pCmd Handlers
void LED_ON_pCmd_action_handler(PacketCommand& this_pCmd) {
  #ifdef DEBUG
  Serial.println(F("#LED_ON_pCmd_action_handler"));
  #endif
  digitalWrite(arduinoLEDPin, HIGH);
}
  
void LED_OFF_pCmd_action_handler(PacketCommand& this_pCmd) {
  #ifdef DEBUG
  Serial.println(F("#LED_OFF_pCmd_action_handler"));
  #endif
  digitalWrite(arduinoLEDPin, LOW);
}


void SHT_READ_pCmd_query_handler(PacketCommand& this_pCmd) {
  #ifdef DEBUG
  Serial.println(F("#SHT_READ_pCmd_query_handler"));
  #endif
}
//******************************************************************************
// sCmd Handlers
void IDN_sCmd_query_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("#IDN_sCmd_query_handler"));
  #endif
  Serial.print(F("RFM69 Node #"));
  uint8_t idn;         //formatted to print as string
  node_flash::readByte(node_flash::NODEID, idn); //updated by reference
  Serial.println(idn);
}

void LED_ON_sCmd_action_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("#LED_ON_sCmd_action_handler"));
  #endif
  digitalWrite(arduinoLEDPin, HIGH);
}
  
void LED_OFF_sCmd_action_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("#LED_OFF_sCmd_action_handler"));
  #endif
  digitalWrite(arduinoLEDPin, LOW);
}


void FLASH_ERASE_sCmd_action_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("FLASH_ERASE_sCmd_action_handler"));
  Serial.print("#Erasing Flash chip ... ");
  #endif
  node_flash::erase();
  #ifdef DEBUG
  Serial.println("DONE");
  #endif
}

void FLASH_DUMP_sCmd_query_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("#FLASH_DUMP_sCmd_query_handler"));
  #endif
  int counter = 0;
  while(counter<=256){
    uint8_t valByRef;
    node_flash::readByte((node_flash::datafield_e) counter++, valByRef);
    Serial.print(valByRef, HEX);
    Serial.print('.');
  }
  Serial.println();
}

#define FLASH_READ_BUFFER_SIZE 64
void FLASH_READ_sCmd_query_handler(SerialCommand this_sCmd) {
  uint8_t buffer[FLASH_READ_BUFFER_SIZE];
  #ifdef DEBUG
  this_sCmd.println(F("#FLASH_READ_sCmd_query_handler"));
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
      node_flash::readByte((node_flash::datafield_e) fieldnum, valByRef);
      Serial.println(valByRef);
    }
    else{
      size_t len = min(atoi(arg2), FLASH_READ_BUFFER_SIZE); //limit read length
      uint32_t fieldnum = atoi(arg1);
      //interpret string of bytes as uint8_t array
      node_flash::readBytes((node_flash::datafield_e) fieldnum, buffer, len);
      for(int i=0; i < len; i++){
        Serial.write(buffer[i]);
      }
    }
  }
}

void FLASH_WRITE_sCmd_action_handler(SerialCommand this_sCmd) {
  uint8_t buffer[FLASH_READ_BUFFER_SIZE];
  #ifdef DEBUG
  this_sCmd.println(F("#FLASH_WRITE_sCmd_query_handler"));
  #endif
  char* arg1 = this_sCmd.next();
  if (arg1 == NULL){
    this_sCmd.println(F("###ERROR FLASH.WRITE requires two arguments (fieldnum, value), 0 given"));
  }
  else{
    char* arg2 = this_sCmd.next();
    if (arg2 == NULL){
      this_sCmd.println(F("###ERROR FLASH.WRITE requires two arguments (fieldnum, value), 1 given"));
    }
    else{
      //interpet value as ASCII integer
      uint32_t fieldnum = atoi(arg1);
      uint8_t  value    = atoi(arg2);
      node_flash::writeByte((node_flash::datafield_e) fieldnum,value);//fieldnum, value);
    }
  }
}


void RFM69_CONFIG_sCmd_query_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("#RFM69_CONFIG_sCmd_query_handler"));
  #endif
  //node_flash::get_config(node_config);
  //print out the config in JSON format
  this_sCmd.print(F("{"));
  this_sCmd.print(F( "\n  \"nodeID\":"));      this_sCmd.print(node_config.nodeID);
  this_sCmd.print(F(",\n  \"networkID\": "));  this_sCmd.print(node_config.networkID);
  this_sCmd.print(F(",\n  \"gatewayID\": "));  this_sCmd.print(node_config.gatewayID);
  this_sCmd.print(F(",\n  \"frequency\": "));  this_sCmd.print(node_config.frequency);
  this_sCmd.print(F(",\n  \"is_RFM69HW\": ")); this_sCmd.print(node_config.is_RFM69HW);
  this_sCmd.print(F(",\n  \"encryptkey: \""));
  for(size_t i=0; i < ENCRYPTKEY_LEN; i++){
    this_sCmd.print(node_config.encryptkey[i]);
  }
  this_sCmd.print(F("\""));
  this_sCmd.println(F("\n}"));
}

void RFM69_SET_CONFIG_sCmd_action_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("#RFM69_SET_CONFIG_sCmd_action_handler"));
  #endif
  //write the global config to flash NOTE: will erase ALL previous flash contents
  node_flash::set_config(node_config);
}

void RFM69_NODEID_sCmd_action_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("#RFM69_NODEID_sCmd_action_handler"));
  #endif
  char* arg = this_sCmd.next();
  if (arg == NULL){
    this_sCmd.println(F("###ERROR RFM69.NODEID requires one arguments (value), 0 given"));
  }
  else{
    //interpet value as ASCII integer
    uint8_t value = atoi(arg);
    node_config.nodeID = value;
  }
}

void SHT_READ_sCmd_query_handler(SerialCommand this_sCmd) {
  #ifdef DEBUG
  this_sCmd.println(F("#SHT_READ_sCmd_query_handler"));
  #endif
  float temp_C   = sht10.readTemperatureC();
  float humid_RH = sht10.readHumidity();
  this_sCmd.print(temp_C,3);this_sCmd.print(",");
  this_sCmd.println(humid_RH,3);
}

// Unrecognized command
void UNRECOGNIZED_sCmd_default_handler(const char* command, SerialCommand this_sCmd){
  this_sCmd.print(F("### Error: command '"));
  this_sCmd.print(command);
  this_sCmd.print(F("' not recognized ###\n"));
}
//******************************************************************************
// Helper Functions

