#include "ENC28J60.h"

#define D1 CS_PIN
#ifndef HEADER_WOLSERVICE
  #define HEADER_WOLSERVICE
   
  class WoLService{
    public:
      /**
      * Initializes WoLService. Instantiates ENC28J60 object and gets target's MAC address if not already found.
      */
      void initialize();

      /**
       * Sends a magic packet with target_mac as WoL Target
       */
       void SendMagicPacket();

       /**
        * Listens for a packet from the ethernet port, assumes it to be the target desktop, and sets the Target's MAC address as WoL target. Return true if operation is successful.
        */

       bool SearchTargetMAC();

       /**
        * Sets Target MAC address
        * @param address 6 byte array containing target's MAC address
        */
        void SetTargetMAC(byte* address);

       /**
        * Check if target has been set.
        */
       bool isTargetSet();
    
    private:
      byte target_mac_[6]; // MAC Address of WoL target
      bool target_set_=false;
      bool searching_ = false; //If MAC address search is in progress
      ENC28J60 eth; // Ethernet interface.
      
//      SPISettings default_settings(20000000,MSBFIRST, SPI_MODE0);

      
    

     /**
      * Generates a bare (length and payload only) magic packet to be sent. Returns a byte array of length 102.
      * SecureWoL not supported currently
      */
      byte* GenerateBareMagicPacket();

      /**
       * Sends a broadcast packet to get the MAC address of target computer's ethernet NIC.
       * @return byte[] byte array of length 6 containing target's MAC address
       */
      byte* RetrieveTargetMAC();

      

      byte* GenerateMagicPacket();
     
       
  };
   
#endif
