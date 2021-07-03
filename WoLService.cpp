#include "Arduino.h"
#include "WoLService.h"
#include "ENC28J60.h"




void WoLService::initialize(){
  eth.initialize();
}

bool WoLService::SearchTargetMAC(){
  
  if(!searching_){
    eth.StartReceive();
    searching_ = true;
  }
  
  byte packet_count = eth.GetControlReg(0x39); //EPKTCNT
  if(!packet_count){
    Serial.println("No packet received thus far.");
    return false;
  }  
  
  ENC28J60::BarePacket bp = eth.GetNextPacket();
  byte target_mac [6];
  Serial.println("Packet received. Extracting MAC address from Target:");
  for(unsigned i = 0; i<6; i++){
    target_mac[i] = bp.dest_mac[i];
    Serial.println(bp.dest_mac[i],HEX);
  }
  SetTargetMAC(target_mac);
  searching_ = false;
  eth.StopReceive();
  return true;
  
}

void WoLService::SendMagicPacket(){
  
  ENC28J60::BarePacket bp;
  bp.payload_length = 102;
  bp.payload = new byte[102];
  for(unsigned i = 0 ; i < 6; i++){ 
    bp.dest_mac[i] = 0xFF; // Destination MAC set to broadcast
    bp.payload[i] = 0xFF; // initial part of Magic Packet payload;
  }
  bp.type[0] = 0x08;
  bp.type[1] = 0x42; //WoL type

  for(unsigned j = 0; j < 16; j++){ // 16 repetitions of target mac address
    for(unsigned i = 0; i < 6; i++){
      bp.payload[6 + i + (j*6)] = target_mac_[i];
    }
  }
  eth.SendPacket(bp);
  
  
}

bool WoLService::isTargetSet(){
  return target_set_;
}

void WoLService::SetTargetMAC(byte* address){
  for(unsigned i = 0; i<6; i++){
    target_mac_[i] = address[i];
  }
  target_set_ = true;
}
  
