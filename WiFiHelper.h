
#include "AirWare.h"
#include "timeFunctions.h"
#include "fanet.h"

void WiFi_setup(void);

#include <DNSServer.h>

extern DNSServer dnsServer;
extern IPAddress apIP;
extern bool webUpload;
extern char airWareSsid[32]; 
extern char airWarePassword[64];
extern bool set_Connected_Time;
extern int client_connected_time;
extern int myChipID;
extern char wiredDirectly;
extern int airWhereID;
//v16.1

extern char awHexID[5];
extern char awHexManu[3];

extern char airWhere_ap_password[32];

bool upLoadtoWeb( String urlToLoad);

//v17

extern char packet_repeat;




