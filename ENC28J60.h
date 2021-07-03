#define CS_PIN D1
#ifndef HEADER_ENC28J60
  #define HEADER_ENC28J60

  class ENC28J60{
    public:
    struct BarePacket{
        byte dest_mac[6]; // if sending packet, this must be the destination's packet. If receiving a packet, this will be the source packet.
        byte type[2];
        byte* payload;
        unsigned int payload_length; // must match payload's size;
    };
    /**
    * Initializes the ENC28J60 chipset
    */
    void initialize();

    /**
     * Enables/Disable packet reception on the ENC28J60 (off by default)
     */
     void StartReceive();

     void StopReceive();
     

    /**
     * Get the value of specified control register.
     * @param address in the form 0b0BBAAAAA, where B specifies register bank, and A specifies the register address
     */
    byte GetControlReg(byte address);

    /**
      * Gets the next packet to be read on buffer, and clears it from buffer
    */
    BarePacket GetNextPacket();

     /**
       * Write value to specified control register
       * @param address in the form 0b0BBAAAAA, where B specifies register bank, and A specifies the register 
       * @param value value to be written
    */
    void SetControlReg(byte address, byte value);

    /**
      * Write packet to buffer and transmit
      *
      * @param bare_packet
      */
    void SendPacket(BarePacket bare_packet);

    /**
     * Sets/Clears bit in address
     * @param address in the form 0b0BBAAAAA, where B specifies register bank, and A specifies the register 
     * @param bits select which bits to control. E.g., 0x88 would mean bits 7 and 3 would be controlled
     * @param control_type true to set bit(s), false to clear bits
     */
     void ControlBit(byte address, byte bits, bool control_type);


    

    private:
    bool receiving = false; // whether interface is accepting packets
    byte src_mac_[6] = {0x87, 0x35, 0xDE, 0x95, 0x21, 0x4E}; // MAC Address of Ethernet NIC (randomly generated for now.)
    byte cur_bank_ = 0x0;
    unsigned int transactions_count = 0;

    /**
     * Helper function for GetNextPacket that gets byte in buffer pointed by ERDPT
     */
     byte GetNextByte();

    /**
     * Helper function for GetControlReg
     * @param argument address in the form 0b000AAAAA, where A specifies register address. Bank must have already been selected at this point.
     */

    byte GetControlReg_(byte argument);

    /**
     * Helper function for SetControlReg
     * @param byte argument address in the form 0b000AAAAA, where A specifies register address. Bank must have already been selected at this point.
     */
    void SetControlReg_(byte argument, byte value);     

    
    /**
     * Switches to appropriate memory bank
     */

    void SelBank(byte bank);

    /**
    * Checks if reading an Ethernet (E) register. For dummy bit logic.
    */
    bool isEthReg(byte addr);

    /**
     * Wrapper functions for SPI.begin/endTransaction to prevent conflicts when nesting functions.
     */
    void BeginTransaction();
    void EndTransaction();

    BarePacket GenerateTestPacket();
  };

#endif
