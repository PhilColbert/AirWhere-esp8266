
#include "AirWare.h"
#include "fileOps.h"
#include "ESP8266httpUpdate.h"
#include "ESP8266HTTPClient.h"
#include "timeFunctions.h"
//v16
#include "fanet.h"
#include "xmodem.h"
#include <SoftwareSerial.h>

void Web_setup(void);
void Web_loop(void);

//extern byte TxBuffer[PKT_SIZE];

#include <DNSServer.h>
extern DNSServer dnsServer;
extern IPAddress apIP;
extern bool dhcp_client_connected;

extern long tcp_rx_weblines_counter;
extern long serial_rx_packets_counter;
extern long nrf_rx_packets_counter;
extern long tcp_tx_weblines_counter;
extern long serial_tx_packets_counter;
extern int longest_loop_time;
//extern ufo_t fo;
extern int flush_time_counter;
extern long web_upload_counter;
extern long bad_rx_packets;
extern uint32  free_mem;
extern int loopTime;
extern char groundStationMode;
extern char gsLatitude[11];
extern char gsLongitude[12];
extern char gsAltitude[6];
extern char wiredDirectly;
extern int gpsBaudRate;
extern int airWhereID;
//v16.1
extern char awHexManu[3];
extern char awHexID [5];


extern char wifioffSelected;
extern char awPilotName[30];

extern char airWareSsid[32]; 
extern char airWarePassword[64];
extern char airWhere_ap_password[32];
extern char navSofware;
extern char receiveFlarm;
extern char loraFrequency;
extern char web_aircraft_type;

extern bool webUpload;
extern int AwVersion;
extern WiFiClient LK8000Client;
extern bool socket_connected;
extern unsigned int webTime;
extern int myChipID;

//v16
extern char lora_or_flarm;
extern char groundstation_type;
extern char gsm_access_point_name[40];
extern char gsm_username[30];
extern char gsm_password[30];
extern bool gsm_connected;


//v16
 extern char flarm_board_expiration[14];
//extern char Root_temp[2100];

 extern SoftwareSerial flarmSerial;
extern int gpsBaudRate;


