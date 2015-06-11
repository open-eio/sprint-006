#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
#include <PacketCommand.h>
#include "network_config.h"
#include "RFM69Interface.h"

#define DEBUG

#define SERIAL_BAUD   115200

int arduinoLED = 9;  // Moteinos have LEDs on D9

void setup() {
  //----------------------------------------------------------------------------
  // configure the PacketCommand parser
  pCmd_RFM69.addCommand((byte*) "\x41","LED.ON",     LED_on);            // Turns LED on   ("\x41" == "A")
  pCmd_RFM69.addCommand((byte*) "\x42","LED.OFF",    LED_off);           // Turns LED off  ("\x42" == "B")
  //----------------------------------------------------------------------------
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

void loop() {
  //----------------------------------------------------------------------------
  // handle feeding packets to pCmd parser and dispatching to commands
  RFM69Interface_process_incoming();
}

void LED_on(PacketCommand& this_pCmd) {
  Serial.println(F("LED on"));
  digitalWrite(arduinoLED, HIGH);
}
  
void LED_off(PacketCommand& this_pCmd) {
  Serial.println(F("LED off"));
  digitalWrite(arduinoLED, LOW);
}
