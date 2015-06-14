//******************************************************************************
// System configuration speficication stored in non-volatile flash memory.
//******************************************************************************
#include <RFM69.h>
#include <SPIFlash.h>
#include "RFM69Interface.h"
#include "flash_config.h"


//////////////////////////////////////////
// flash(SPI_CS, MANUFACTURER_ID)
// SPI_CS          - CS pin attached to SPI flash chip (8 in case of Moteino)
// MANUFACTURER_ID - OPTIONAL, 0x1F44 for adesto(ex atmel) 4mbit flash
//                             0xEF30 for windbond 4mbit flash
//////////////////////////////////////////
SPIFlash flash_chip(FLASH_SS, 0xEF30); //EF30 for 4mbit  Windbond chip (W25X40CL)

bool node_flash::initialize(){
  return flash_chip.initialize();
}

void node_flash::erase(){
  flash_chip.chipErase();
  while(flash_chip.busy());
}

void node_flash::get_config(struct Node_Config& nc){
  node_flash::readByte(node_flash::NODEID,     nc.nodeID);
  node_flash::readByte(node_flash::NETWORKID,  nc.networkID);
  node_flash::readByte(node_flash::GATEWAYID,  nc.gatewayID);
  node_flash::readByte(node_flash::FREQUENCY,  nc.frequency);
  node_flash::readByte(node_flash::IS_RFM69HW,  (uint8_t&) nc.is_RFM69HW);                  //cast from bool
  node_flash::readBytes(node_flash::ENCRYPTKEY, (uint8_t*) nc.encryptkey, ENCRYPTKEY_LEN);  //cast from char*
}

void node_flash::set_config(struct Node_Config& nc){
  //be sure to erase before writing
  node_flash::erase();
  node_flash::writeByte(node_flash::NODEID,     nc.nodeID);
  node_flash::writeByte(node_flash::NETWORKID,  nc.networkID);
  node_flash::writeByte(node_flash::GATEWAYID,  nc.gatewayID);
  node_flash::writeByte(node_flash::FREQUENCY,  nc.frequency);
  node_flash::writeByte(node_flash::IS_RFM69HW,  (uint8_t) nc.is_RFM69HW);                   //cast from bool
  node_flash::writeBytes(node_flash::ENCRYPTKEY, (uint8_t*) nc.encryptkey, ENCRYPTKEY_LEN);  //cast from char*
}

void node_flash::readByte(node_flash::datafield_e fieldnum, uint8_t& valByRef){
  valByRef = flash_chip.readByte(fieldnum);
}

void node_flash::readBytes(node_flash::datafield_e fieldnum, uint8_t* buffer, size_t len){
  for(size_t i=0; i < len; i++){
    buffer[i] = flash_chip.readByte(fieldnum + i);
  }
}

void node_flash::writeByte(node_flash::datafield_e fieldnum, uint8_t val){
  flash_chip.writeByte(fieldnum,val);
}

void node_flash::writeBytes(node_flash::datafield_e fieldnum, uint8_t* buffer, size_t len){
  for(size_t i=0; i < len; i++){
    flash_chip.writeByte((uint8_t) fieldnum + i,buffer[i]);
  }
}
