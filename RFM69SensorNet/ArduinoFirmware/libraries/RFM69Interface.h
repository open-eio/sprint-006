/******************************************************************************/
#ifndef RFM69INTERFACE_H
#define RFM69INTERFACE_H

#include <PacketCommand.h>
#include <RFM69.h>
//uncomment for debugging

#define RFM69INTERFACE_DEBUG



/******************************************************************************/
// PacketCommand
//------------------------------------------------------------------------------
extern PacketCommand pCmd_RFM69;
// callbacks
bool RFM69_recv_callback(PacketCommand& this_pCmd);
void pCmd_default_handler(PacketCommand& this_pCmd);
/******************************************************************************/
// RFM69
//------------------------------------------------------------------------------
extern RFM69 Radio;

#define ENCRYPTKEY_LEN 16
struct Node_Config{
    uint8_t nodeID;
    uint8_t networkID;
    uint8_t gatewayID;
    uint8_t frequency;
    bool    is_RFM69HW;
    char    encryptkey[ENCRYPTKEY_LEN];
};

static bool     promiscuousMode = false;; //set to 'true' to sniff all packets on the same network
static uint8_t  ackCount = 0;
static uint32_t packetCount = 0;
//------------------------------------------------------------------------------
// Routines
extern void RFM69Interface_node_configure(struct Node_Config nc);
extern void RFM69Interface_gateway_configure();
extern void RFM69Interface_start();
extern void RFM69Interface_process_incoming();

//helper functions
void print_hex(byte* pkt, size_t len);

#endif /*RFM69INTERFACE_H*/
