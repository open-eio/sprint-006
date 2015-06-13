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
//SPIFlash flash(FLASH_SS, 0xEF30); //EF30 for 4mbit  Windbond chip (W25X40CL)
static bool     promiscuousMode = false;; //set to 'true' to sniff all packets on the same network
static uint8_t  ackCount = 0;
static uint32_t packetCount = 0;
//------------------------------------------------------------------------------
// Routines
extern void RFM69Interface_node_default_config();
extern void RFM69Interface_gateway_default_config();
extern void RFM69Interface_start();
extern void RFM69Interface_process_incoming();

//helper functions
void print_hex(byte* pkt, size_t len);

#endif /*RFM69INTERFACE_H*/
