//******************************************************************************
// System configuration speficication.  This file should be readable, but the
// implementation details in the .cpp should be kept secret - i.e. not in
// a public epository.
//******************************************************************************
#ifndef __NETWORKCONFIG_H__
#define __NETWORKCONFIG_H__

namespace rfm69config{
  extern const uint8_t REMOTEID;   //unique for each node on same network
  extern const uint8_t GATEWAYID;  //many nodes can talk to the same gateway
  extern const uint8_t NETWORKID;  //the same on all nodes that talk to each other
  extern const uint8_t FREQUENCY;
  extern const bool    IS_RFM69HW; //only for RFM69HW! Leave out if you have RFM69W!
  extern const char    ENCRYPTKEY[]; //exactly the same 16 characters/bytes on all nodes!
}

namespace node_flash{
  //output data
  enum datafield_e {
    IDN = 0,
    NETWORKID = 1,
    GATEWAYID = 2,
    FREQUENCY = 3,
    IS_RFM69HW = 4,
    ENCRYPTKEY = 5, //skip 16 bytes
  };
}

#endif /*__NETWORKCONFIG_H__*/
