//******************************************************************************
// System configuration speficication stored in non-volatile flash memory.
//******************************************************************************
#ifndef __FLASHCONFIG_H__
#define __FLASHCONFIG_H__

#include "RFM69Interface.h"

// Non-volatile Data Store
#ifdef __AVR_ATmega1284P__
  #define FLASH_SS      23 // and FLASH SS on D23
#else
  #define FLASH_SS      8 // and FLASH SS on D8
#endif

namespace node_flash{
  //output data
  enum datafield_e {
    NODEID     = 0,
    NETWORKID  = 1,
    GATEWAYID  = 2,
    FREQUENCY  = 3,
    IS_RFM69HW = 4,
    ENCRYPTKEY = 5, //FIXME remember skip 16 bytes after this
  };
  extern bool initialize();
  extern void erase();
  extern void get_config(struct Node_Config& nc);
  extern void set_config(struct Node_Config& nc);
  extern void readByte(datafield_e fieldnum, uint8_t& valByRef);
  extern void readBytes(datafield_e fieldnum, uint8_t* buffer, size_t len);
  extern void writeByte(datafield_e fieldnum, uint8_t val);
  extern void writeBytes(datafield_e fieldnum, uint8_t* buffer, size_t len);
}

#endif /*__FLASHCONFIG_H__*/
