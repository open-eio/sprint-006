// Sample RFM69 receiver/gateway sketch, with ACK and optional encryption
// Passes through any wireless received messages to the serial port & responds to ACKs
// It also looks for an onboard FLASH chip, if present
// Library and code by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/

#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <SPIFlash.h> //get it here: https://www.github.com/lowpowerlab/spiflash
#include <PacketCommand.h>
#include "shared_config.h"

#define DEBUG

#define SERIAL_BAUD   115200

int arduinoLED = 9;  // Moteinos have LEDs on D9

RFM69 radio;
//SPIFlash flash(FLASH_SS, 0xEF30); //EF30 for 4mbit  Windbond chip (W25X40CL)
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network


byte ackCount=0;
uint32_t packetCount = 0;
bool RFM69_recv_callback(byte* inbuff, size_t& lenByRef){
  if (radio.receiveDone())
  {
    #ifdef DEBUG
    Serial.print(F("# radio.receiveDone(): \n"));
    Serial.print(F("# \tstart: "));
    Serial.println(start_micros);
    #endif
    lenByRef = radio.DATALEN;
    for (byte i = 0; i < lenByRef; i++){
      inbuff[i] = (byte) radio.DATA[i];
    }
    #ifdef DEBUG
    Serial.print(F("# \trecv: "));
    print_hex(inbuf, NeuroDot::RF24_PACKET_SIZE);
    Serial.println();
    #endif
    if (radio.ACKRequested())
    {
      //byte theNodeID = radio.SENDERID;
      radio.sendACK();
      #ifdef DEBUG
      Serial.print("# \tACK sent.");
      #endif
    }
    return true;
  } else {return false;}
}

// Unrecognized Packet, forward to the Serial USB interface
void pCmd_default_handler(PacketCommand this_pCmd)
{
  PacketCommand::CommandInfo current_command = this_pCmd.getCurrentCommand();
  Serial.print(F("### Error: unrecognized packet command: 0x"));
  for (int i=0; i < PacketCommand::MAX_TYPE_ID_LEN;i++){
    Serial.print(current_command.type_id[i],HEX);
  }
  Serial.println();
}

PacketCommand pCmd;         // The demo PacketCommand object, initialize with any Stream object


void setup() {
  //----------------------------------------------------------------------------
  // configure the PacketCommand parser
  pCmd.addCommand((byte*) "\x41","LED.ON",     LED_on);            // Turns LED on   ("\x41" == "A")
  pCmd.addCommand((byte*) "\x42","LED.OFF",    LED_off);           // Turns LED off  ("\x42" == "B")
  pCmd.registerRecvCallback(RFM69_recv_callback);
  pCmd.registerDefaultHandler(pCmd_default_handler);
  //----------------------------------------------------------------------------
  
  Serial.begin(SERIAL_BAUD);
  delay(10);
  
  //----------------------------------------------------------------------------
  // setup the rfm69 interface
  radio.initialize(rfm69config::FREQUENCY,rfm69config::REMOTEID,rfm69config::NETWORKID);
  if(rfm69config::IS_RFM69HW){
    radio.setHighPower(); //only for RFM69HW!
  }
  radio.encrypt(rfm69config::ENCRYPTKEY);
  radio.promiscuous(promiscuousMode);
  //radio.setFrequency(919000000);
  #ifdef DEBUG
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", rfm69config::FREQUENCY==RF69_433MHZ ? 433 : rfm69config::FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  #endif
  //----------------------------------------------------------------------------
}


void loop() {
  //----------------------------------------------------------------------------
  // handle feeding packets to pCmd parser
  // FIXME eventually this should get tucked away into a single PacketCommand API call
  PacketCommand::STATUS pcs;
  pcs = pCmd.recv();
  if (pcs == PacketCommand::SUCCESS){  //a packet was received
    pcs = pCmd.matchCommand();
    if (pcs == PacketCommand::SUCCESS){  //a command was matched
      #ifdef DEBUG
      Serial.print(F("# \tmatched command: "));
      PacketCommand::CommandInfo cmd = pCmd.getCurrentCommand();
      Serial.println(cmd.name);
      #endif
    }
    else if (pcs == PacketCommand::ERROR_NO_TYPE_ID_MATCH){  //valid ID but no command was matched
      #ifdef DEBUG
      Serial.println(F("# \tno matched command"));
      #endif
    }
    else{
      Serial.print(F("### Error: pCmd.matchCommand returned status code: "));
      Serial.println(pcs);
    }
    //dispatch to handler or default if no match
    pCmd.dispatchCommand();
  }
  else if (pcs == PacketCommand::NO_PACKET_RECEIVED){
//    #ifdef DEBUG
//    Serial.println(F("# no packet received"));
//    #endif
  }
  else {
    Serial.print(F("### Error: pCmd.recv returned status code: "));
    Serial.println(pcs);
  }
}

void LED_on(PacketCommand this_pCmd) {
  Serial.println(F("LED on"));
  digitalWrite(arduinoLED, HIGH);
}
  
void LED_off(PacketCommand this_pCmd) {
  Serial.println(F("LED off"));
  digitalWrite(arduinoLED, LOW);
}
