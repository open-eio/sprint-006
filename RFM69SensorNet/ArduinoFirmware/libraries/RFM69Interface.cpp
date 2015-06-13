// Sample RFM69 receiver/gateway sketch, with ACK and optional encryption
// Passes through any wireless received messages to the serial port & responds to ACKs
// It also looks for an onboard FLASH chip, if present
// Library and code by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/

#include <Arduino.h>
#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>
#include <SPIFlash.h> //get it here: https://www.github.com/lowpowerlab/spiflash
#include <PacketCommand.h>
#include "network_config.h"
#include "RFM69Interface.h"


#define PCMD_MAX_COMMANDS 10
#define PCMD_INPUT_BUFFER_SIZE 64
#define PCMD_OUTPUT_BUFFER_SIZE 64

PacketCommand pCmd_RFM69(PCMD_MAX_COMMANDS,  // size_t maxCommands
                         PCMD_INPUT_BUFFER_SIZE,  //size_t inputBufferSize
                         PCMD_OUTPUT_BUFFER_SIZE   //size_t outputBufferSize
                         );

RFM69 Radio;


//SPIFlash flash(FLASH_SS, 0xEF30); //EF30 for 4mbit  Windbond chip (W25X40CL)


bool RFM69_recv_callback(PacketCommand& this_pCmd){
  if (Radio.receiveDone())
  {
    uint32_t start_micros = micros();
    #ifdef RFM69INTERFACE_DEBUG
    Serial.print(F("# Radio.receiveDone(): \n"));
    Serial.print(F("# \tstart: "));
    Serial.println(start_micros);
    #endif
    this_pCmd.assignInputBuffer((byte*) Radio.DATA, Radio.DATALEN);
    #ifdef RFM69INTERFACE_DEBUG
    Serial.print(F("# \trecv: "));
    print_hex((byte*) Radio.DATA, Radio.DATALEN);
    Serial.println();
    #endif
    if (Radio.ACKRequested())
    {
      //byte theNodeID = Radio.SENDERID;
      Radio.sendACK();
      #ifdef RFM69INTERFACE_DEBUG
      Serial.print("# \tACK sent.");
      #endif
    }
    return true;
  } else {return false;}
}

// Unrecognized Packet, forward to the Serial USB interface
void pCmd_default_handler(PacketCommand& this_pCmd)
{
  PacketCommand::CommandInfo current_command = this_pCmd.getCurrentCommand();
  Serial.print(F("### Error: unrecognized packet command: 0x"));
  for (int i=0; i < PacketCommand::MAX_TYPE_ID_LEN;i++){
    Serial.print(current_command.type_id[i],HEX);
  }
  Serial.println();
}


void RFM69Interface_node_default_config() {
  pCmd_RFM69.registerRecvCallback(RFM69_recv_callback);
  pCmd_RFM69.registerDefaultHandler(pCmd_default_handler);
  pCmd_RFM69.resetInputBuffer();
  //----------------------------------------------------------------------------
  // setup the rfm69 interface
  Radio.initialize(rfm69config::FREQUENCY,rfm69config::REMOTEID,rfm69config::NETWORKID);
  if(rfm69config::IS_RFM69HW){
    Radio.setHighPower(); //only for RFM69HW!
  }
  Radio.encrypt(rfm69config::ENCRYPTKEY);
  Radio.promiscuous(promiscuousMode);
  //Radio.setFrequency(919000000);
  #ifdef RFM69INTERFACE_DEBUG
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", rfm69config::FREQUENCY==RF69_433MHZ ? 433 : rfm69config::FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  #endif
  //----------------------------------------------------------------------------
}

void RFM69Interface_gateway_default_config() {
  //----------------------------------------------------------------------------
  // setup the rfm69 interface
  Radio.initialize(rfm69config::FREQUENCY,rfm69config::GATEWAYID,rfm69config::NETWORKID);
  if(rfm69config::IS_RFM69HW){
    Radio.setHighPower(); //only for RFM69HW!
  }
  Radio.encrypt(rfm69config::ENCRYPTKEY);
  Radio.promiscuous(promiscuousMode);
  //------------------------------------------------------
}

void RFM69Interface_start() {
  pCmd_RFM69.registerRecvCallback(RFM69_recv_callback);
  pCmd_RFM69.registerDefaultHandler(pCmd_default_handler);
  pCmd_RFM69.resetInputBuffer();
  #ifdef RFM69INTERFACE_DEBUG
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", rfm69config::FREQUENCY==RF69_433MHZ ? 433 : rfm69config::FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  #endif
  //----------------------------------------------------------------------------
}

void RFM69Interface_process_incoming(){
  //----------------------------------------------------------------------------
  // handle feeding packets to pCmd parser and dispatching to commands
  bool gotPacket;
  pCmd_RFM69.recv(gotPacket); //gets updated by reference
  if(gotPacket){
    pCmd_RFM69.processInput();
  }
}

void print_hex(byte* pkt, size_t len){
  Serial.print("0x");
  for (size_t i=0; i<len;i++){
    if(pkt[i] < 16){Serial.print(0x00,HEX);}
    Serial.print(pkt[i],HEX);
    Serial.print(" ");
  }
}
