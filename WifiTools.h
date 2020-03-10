#include <SoftwareSerial.h>
#include "timeFunctions.h"

extern unsigned int last_data_received;
extern SoftwareSerial swSer;

extern int lastUdpPacketTime;

String readAllDataOnWifi(WiFiClient LK8000Client, String TCPBuffer, bool &fullLine );
String readAllDataOnSerial(String TCPBuffer, bool &fullLine );
String readAllDataOnWifiTxSerial( WiFiClient LK8000Client, String TCPBuffer, bool &fullLine );

void writeDataToWifi(WiFiClient LK8000Client, String data);
void writeDataToSerial( String data);

void addTestPilot(WiFiClient LK8000Client, String Pilot, int Dist, int current_pos);
bool sendUDPPacket ( String packet, IPAddress apFoundIP );
bool upLoadtoUDP( String urlToLoad, IPAddress IP, int udpPort);

bool cksum(String data);

extern unsigned long pflauSendTime;
extern unsigned long pflauCounter;
extern bool addpflau;
extern String streamData;
extern char wiredDirectly;

extern char lora_or_flarm;

extern SoftwareSerial flarmSerial;


