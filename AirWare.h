#ifndef AIRWARE_H
#define AIRWARE_H

#define LIVE_VERSION
//#define TEST_PILOT

#ifdef LIVE_VERSION
 //#define RXFLARM
 #define DEBUG_SERIAL_UART(...)
 #define DEBUG_SERIAL_UART_S(...)
 #define DEBUG_SERIAL_UART_MAX(...)
#endif

#ifndef LIVE_VERSION
  
  #define SERIAL_UART_OUT

  #ifdef SERIAL_UART_OUT
  //  #define SERIAL_UART0_OUT
 //   #define DEBUG_SERIAL_UART(...) Serial.printf( __VA_ARGS__ );
 //   #define DEBUG_SERIAL_UART_S(...) Serial.print( __VA_ARGS__ );
    #define SERIAL_UART1_OUT
    #define DEBUG_SERIAL_UART(...) Serial.printf( __VA_ARGS__ );
    #define DEBUG_SERIAL_UART_S(...) Serial.print( __VA_ARGS__ );

 // #define DEBUG_SERIAL_UART(...) Serial.printf( __VA_ARGS__ ); Serial1.printf( __VA_ARGS__ );
 // #define DEBUG_SERIAL_UART_S(...) Serial.print( __VA_ARGS__ ); Serial1.print( __VA_ARGS__ );

 // All the output
   #define DEBUG_SERIAL_UART_MAX(...) Serial.printf( __VA_ARGS__ );
 //  #define DEBUG_SERIAL_UART_MAX(...);
  #endif

  #ifndef SERIAL_UART_OUT
    #define DEBUG_SERIAL_UART(...)
    #define DEBUG_SERIAL_UART_S(...)
    #define DEBUG_SERIAL_UART_MAX(...)
  #endif

  #define TIMINGS
 // #define TEST_PILOT
#endif

//Switch to add logging facility to UDP
//#define LOGGING_ON
//#define LOGGING_ON_MAX
 
//#include <nRF905.h>

//#define RF_FREQ   NRF905_FREQ
//#define PKT_SIZE  24

//#define RXADDR {0x31, 0xfa , 0xb6} // Address of this device (4 bytes)
//#define TXADDR {0x31, 0xfa , 0xb6} // Address of device to send to (4 bytes)

//#define TIMEOUT   5 // 5 milli second time out on the nrf905 receive loop

#define XCSOAR_PORT 10110
#define DNS_PORT 53

#ifdef LIVE_VERSION
 #define AIRWARE_UDP_PORT 5555
#else
 #define AIRWARE_UDP_PORT 5556
#endif 
 
#define LOGGING_PORT 6666

#define airWareServer "http://www.airwhere.co.uk"
/*
typedef struct UFO {
    String    raw;
    time_t    timestamp;

    uint32_t  addr;
    float     latitude;
    float     longtitude;
    int32_t   altitude;
    unsigned int type;

    int32_t   vs;
    float     distance;

    bool      stealth;
    bool      no_track;

    int8_t ns[4];
    int8_t ew[4];
} ufo_t;
*/
#define PACKET_REPEAT_NUMBER 3
#define PACKET_REPEAT_FREQ 1500

#define LATITUDE    43.21
#define LONGTITUDE  5.43
#define ALTITUDE    12

//#define GSTIMERINTERVAL 300000

#define GSTIMERINTERVAL 60000

//#define FANET_MANUFACTURER 24
#define NUM_NEIGHBOURS 10
#define FANET_PACKET_TYPE 1

//#define RF_PACKETS 10
//#define RF_PACKET_SIZE 14

#define RX_PACKETS 10

#define TCP_BUFFER_MAX_LENGTH 1024

#define KMPH_TO_MS 3.61

#endif /* AIRWARE_H */

