#include <SPI.h>
#include "Arduino.h"
#include "ENC28J60.h"


void ENC28J60::initialize(){
  SPI.begin();

  // Reset to clear previous settings
  pinMode(CS_PIN, OUTPUT);
  BeginTransaction();
  digitalWrite(CS_PIN,LOW);
  SPI.transfer(0xFF); // System reset
  digitalWrite(CS_PIN,HIGH);
  EndTransaction();
  delay(100);

  // Receive Buffer ERX ST, ND, RDPT
  SetControlReg(0x08,0x00); // ERXSTL
  SetControlReg(0x09,0x18); // ERXSTH, start address of receive buffer set to 0x1800; (6144 bytes from top)
  SetControlReg(0x0C,0x00); // ERXRDPTL
  SetControlReg(0x0D,0x18); // ERXRDPTH, start address of read pointer set to start address of receive buffer
  SetControlReg(0x00,0x00); // ERDPTL
  SetControlReg(0x01,0x18); // ERDPTH, SPI read pointer set to start address of receive buffer.
  SetControlReg(0x0A,0xFF); // ERXNDL
  SetControlReg(0x0B,0x1F); // ERXNDH, end address of receive buffer set to 0x1FFF, total 2KB;

  // Transmit Buffer (no action required according to data sheet)

  // ECON Registers
  SetControlReg(0x1E,0x80); // ECON2 set to 10000000. AUTOINC enabled.

  // Receive Filter ERXFCON 
  SetControlReg(0x38, 0x00); // No filter.

  

  //Poll for OST (ESTAT.CLKRDY set) before modifying MAC and PHY registers
  while(!(GetControlReg(0x1D) & 0x01)){ 
    delay(10); // Exception thrown if delay isn't there to slow down poll requests.
    }

  if(GetControlReg(0x1D) & 0x01){
    Serial.println(GetControlReg(0x0F),BIN); //ERXWRPTH
    Serial.println(GetControlReg(0x0E),BIN); //ERXWRPTL
    Serial.println("Working as per normal");
  }

  // MAC Initialization MACON 1, 3, 4
  SetControlReg(0x40,0x0D); // MACON1 set to 00001101
  SetControlReg(0x42,0xF3); // MACON3 set to 11110011
  SetControlReg(0x43,0x40); // MACON4 set to 01000000

  // MAC Initialization pt. 2  MABBIPG (0x15), MAIPGL (0x12)
  SetControlReg(0x44,0x15); // MABBIPG set to 0x15 (according to datasheet)
  SetControlReg(0x46,0x12); // MAIPGL set to 0x12

  // MAC Address Initialization MAADR1:MAADR6
  for(byte i = 0; i<6; i++){
    SetControlReg(0x60|i,src_mac_[i]); //MAADRi set to src_mac_[i]
  }

  // Phy registers (No action required according to data sheet)


  // Test send packet
  
  SendPacket(GenerateTestPacket());
  
}

void ENC28J60::StartReceive(){
  ControlBit(0x1F,0x04,1); // Bit Field Set to ECON1.RXEN 
  receiving = true;
}

void ENC28J60::StopReceive(){
  ControlBit(0x1F,0x04,0); // Bit Field clear to ECON1.RXEN 
  receiving = false;
}

byte ENC28J60::GetControlReg(byte address){
  SelBank(address >> 5);
  return GetControlReg_(address & 0x1f);
}


byte ENC28J60::GetControlReg_(byte argument){
  byte instr = argument | 0x00;
  BeginTransaction();
  
  byte result;
  if(isEthReg(argument)){
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(instr);
    result = SPI.transfer(0x0);
    digitalWrite(CS_PIN, HIGH);
  }
  else{
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(instr);
    SPI.transfer(0x0);
    result = SPI.transfer(0x0);
    digitalWrite(CS_PIN, HIGH);
  }
  EndTransaction();
  return result;
}

ENC28J60::BarePacket ENC28J60::GetNextPacket(){
  byte instr = 0x3A; // Read Buffer Memory command
  BarePacket bp;
  byte next_packet_low = GetNextByte();
  byte next_packet_high = GetNextByte();

  byte packet_size_low = GetNextByte();
  byte packet_size_high = GetNextByte();
  short packet_size = (short)(packet_size_high << 8 | packet_size_low);
  Serial.print("Incoming packet size: " );
  Serial.println(packet_size,DEC);
  GetNextByte();
  GetNextByte(); // receive and discard last two bytes of receive status vectors
  
  //READ PACKET LOGIC
  byte packet[packet_size];
  short i = 0;
  BeginTransaction();
  digitalWrite(CS_PIN,LOW);
  SPI.transfer(instr);
  for(;i < packet_size;i++){
    packet[i] = SPI.transfer(0x00);
  }
  
  digitalWrite(CS_PIN,HIGH);
  EndTransaction();

  //parsing packet to BarePacket object
  for(unsigned i = 0; i < 6; i++) bp.dest_mac[i] = packet[i+6]; //recording source mac address
  bp.type[0] = packet[12];
  bp.type[1] = packet[13];
  bp.payload_length = packet_size - 14;
  bp.payload = new byte[bp.payload_length];
  for(unsigned i = 0; i < bp.payload_length;i++){
    bp.payload[i] = packet[14+i];
  }

  //clear buffer
  SetControlReg(0x0C, next_packet_low);
  SetControlReg(0x0D, next_packet_high); // set ERXRDPT to next packet.
  
  ControlBit(0x1E,0x40,1); // Bit Field Set to ECON2.PKTDEC.Decrements packet count by 1. 
  return bp;
  
}

byte ENC28J60::GetNextByte(){
  byte instr = 0x3A; // Read Buffer Memory command
  BeginTransaction();
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(instr);
  byte result = SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);//Immediately raise CS signal, otherwise chipset will keep feeding bytes
  EndTransaction();
  return result;
}

void ENC28J60::SetControlReg(byte address, byte value){
  SelBank(address >> 5);
//  byte argument = address & 0x1f;
  return SetControlReg_(address & 0x1f,value); 
}

void ENC28J60::SetControlReg_(byte argument, byte value){
  byte instr = argument | 0x40; // 0x010AAAAA
  BeginTransaction();
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(instr);
  SPI.transfer(value);
  digitalWrite(CS_PIN, HIGH);
  EndTransaction();
}


void ENC28J60::SendPacket(BarePacket bare_packet){
  unsigned packet_length = bare_packet.payload_length + 15;
  byte packet_final[packet_length];
  packet_final[0] = 0x00; // No overrides set by PPCB
  for(unsigned i = 0; i < 6; i++){
      packet_final[i+1] = bare_packet.dest_mac[i]; // append dest mac
      packet_final[i+7] = src_mac_[i];
  }
  packet_final[13] = bare_packet.type[0];
  packet_final[14] = bare_packet.type[1];
  for(int i = 0; i < bare_packet.payload_length; i++){
    packet_final[i+15] = bare_packet.payload[i];
  }
  delete [] bare_packet.payload;

  int i = 0;
  BeginTransaction();
  digitalWrite(CS_PIN,LOW);
  SPI.transfer(0x7A);

  for(;i<packet_length+1;i++){

    SPI.transfer(packet_final[i]);

  }
  digitalWrite(CS_PIN, HIGH);
  EndTransaction();
  byte ETXNDL = (byte)(packet_length & 0xff);
  byte ETXNDH = (byte)(packet_length >> 8 & 0xff);
  SetControlReg(0x06,ETXNDL);
  SetControlReg(0x08,ETXNDH);//Set End pointer of packet.

  ControlBit(0x1f,0x08,1); // Bit Field set to ECON1.TXRTS
  EndTransaction();

  //wait until ECON1.TXRTS is cleared
  while(GetControlReg(0x1F) & 0x08){
    delay(1);
  }

  if(GetControlReg(0x1D) & 0x02){
    Serial.println("Transmit Abort Error");
  }
  else{
    Serial.println("Transmit Success");
  }

  // Reset EWRPT
  SetControlReg(0x02,0x00);
  SetControlReg(0x03,0x00);
}

void ENC28J60::ControlBit(byte address, byte bits, bool control_type){
  SelBank(address >> 5);
  byte argument = address & 0x1f;
  byte instr;
  
  if(control_type){ //Bit Field Set operation
    instr = 0x80 | argument;
  }else{ // Bit Field Clear operation.
    instr = 0xA0 | argument;
  }
  BeginTransaction();
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(instr);
  SPI.transfer(bits);    
  digitalWrite(CS_PIN, HIGH);
  EndTransaction();
  
}

void ENC28J60::BeginTransaction(){
  if(!transactions_count){
    SPI.beginTransaction(SPISettings(14000000,MSBFIRST, SPI_MODE0));
    
  }
  ++transactions_count;
}

void ENC28J60::EndTransaction(){
  if(transactions_count == 1){
    SPI.endTransaction();
    digitalWrite(CS_PIN, HIGH);
  }
  if(transactions_count) --transactions_count; // underflow protection
}

void ENC28J60::SelBank(byte bank){
  if(bank != cur_bank_){
    byte old_econ1 = GetControlReg_(0x1F);
    old_econ1 = old_econ1 | 0x03; // set last two bits (B_SEL) to 11;
    byte new_econ1 = old_econ1 & (0xFC | bank); // bitwise AND with 0b111111BB
    SetControlReg_(0x1F,new_econ1);
    cur_bank_ = bank;
  }
}

bool ENC28J60::isEthReg(byte addr){
  if(cur_bank_ < 2) return true;
  if (addr > 0x1A) return true;
  if (cur_bank_ == 3 && (addr > 0x05 && addr != 0x0A)) return true;
  else return false;
}

ENC28J60::BarePacket ENC28J60::GenerateTestPacket(){
//   static byte test_packet [64];
  BarePacket bp;
  for(unsigned i = 0 ; i < 6; i++){ 
    bp.dest_mac[i] = 0xFF; // Destination MAC set to broadcast
    // test_packet[i+6] = src_mac_[i]; // Source MAC set  (deprecated, SendPacket will generate for you.)
  }
  bp.type[0] = 0x32;
  bp.type[1] = 0x00; // length field
  bp.payload_length = 50;
  bp.payload = new byte[50];
  for(unsigned i = 0; i <50; i++){
    bp.payload[i] = 0;
  }

  return bp;
}
