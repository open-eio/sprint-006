//******************************************************************************
// SENSITIVE CONFIG INFO.  KEEP THIS FILE SECRET!!!
//******************************************************************************
#include <RFM69.h>
#include "network_config.h"
//------------------------------------------------------------------------------
// rfm69 
namespace rfm69config{
  const uint8_t REMOTEID  = 2;                   //unique for each node on same network
  const uint8_t GATEWAYID = 1;                   //many nodes can talk to the same gateway
  const uint8_t NETWORKID = 100;                 //the same on all nodes that talk to each other
  const uint8_t FREQUENCY = RF69_915MHZ;//RF69_433MHZ;
  const char    ENCRYPTKEY[] = "sampleEncryptKey"; //exactly the same 16 characters/bytes on all nodes!
  const bool    IS_RFM69HW = true;               //only for RFM69HW! Leave out if you have RFM69W!
}
