//******************************************************************************
// Remote Node with RFM69 PacketCommand Interface and Soil Moisture sensor
//------------------------------------------------------------------------------

#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
#include <SHT1x.h>
#include <PacketCommand.h>
#include "network_config.h"
#include "RFM69Interface.h"

#define DEBUG
//******************************************************************************
// Globals
//------------------------------------------------------------------------------
// Serial interface (for debugging on remote)
#define MAX_SERIAL_COMMANDS 10
#define SERIAL_BAUD   115200
SerialCommand sCmd(MAX_SERIAL_COMMANDS);

//------------------------------------------------------------------------------
// SHT10 Soil moisture sensor
const uint8_t sht10DataPin  = 6;
const uint8_t sht10ClockPin = 7;
SHT1x sht10(sht10DataPin, sht10ClockPin);

//******************************************************************************
// misc. Arduino pins
const uint8_t arduinoLEDPin = 9;  // Moteinos have LEDs on D9

//******************************************************************************
void setup() {
  //----------------------------------------------------------------------------
  // configure Arduino pins
  pinMode(arduinoLED,OUTPUT);
  digitalWrite(arduinoLED, LOW);
  //----------------------------------------------------------------------------
  // configure the PacketCommand parser
  pCmd_RFM69.addCommand((byte*) "\x41","LED.ON",  LED_ON_pCmd_action_handler);  // Turns LED on   ("\x41" == "A")
  pCmd_RFM69.addCommand((byte*) "\x42","LED.OFF", LED_OFF_pCmd_action_handler); // Turns LED off  ("\x42" == "B")
  //----------------------------------------------------------------------------
  // configure the SerialCommand parser
  sCmd.addCommand("LED.ON", LED_ON_sCmd_action_handler);
  sCmd.addCommand("LED.OFF", LED_OFF_sCmd_action_handler);
  sCmd.addCommand("SHT.READ?", SHT_READ_sCmd_query_handler);
  // configure the serial port
  Serial.begin(SERIAL_BAUD);
  delay(10);
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
}

//******************************************************************************
// pCmd Handlers
void LED_ON_pCmd_action_handler(PacketCommand& this_pCmd) {
  #if DEBUG
  Serial.println(F("LED_ON_pCmd_action_handler"));
  #endif
  digitalWrite(arduinoLEDPin, HIGH);
}
  
void LED_OFF_pCmd_action_handler(PacketCommand& this_pCmd) {
  #if DEBUG
  Serial.println(F("LED_OFF_pCmd_action_handler"));
  #endif
  digitalWrite(arduinoLEDPin, LOW);
}

void SHT_READ_pCmd_query_handler(PacketCommand& this_pCmd) {
  #if DEBUG
  Serial.println(F("SHT_READ_pCmd_query_handler"));
  #endif
}
//******************************************************************************
// sCmd Handlers
void LED_ON_sCmd_action_handler(SerialCommand this_sCmd) {
  #if DEBUG
  this_sCmd.println(F("LED_ON_sCmd_action_handler"));
  #endif
  digitalWrite(arduinoLEDPin, HIGH);
}
  
void LED_OFF_sCmd_action_handler(SerialCommand this_sCmd) {
  #if DEBUG
  this_sCmd.println(F("LED_OFF_sCmd_action_handler"));
  #endif
  digitalWrite(arduinoLEDPin, LOW);
}

void SHT_READ_sCmd_query_handler(SerialCommand this_sCmd) {
  #if DEBUG
  this_sCmd.println(F("SHT_READ_sCmd_query_handler"));
  #endif
  float temp_C   = sht10.readTemperatureC();
  float humid_RH = sht1x.readHumidity();
  this_sCmd.print(temp_C,3);this_sCmd.print(",");
  this_sCmd.println(humid_RH,3);
}
