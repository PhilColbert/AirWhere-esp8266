 /*
   AirWhere v16
   Author - Phil Colbert - philcolbert@hotmail.com
   Please feel free to contribute to the project :)

   AVR/Arduino nRF905 Library/Driver is developed by Zak Kemble, contact@zakkemble.co.uk
   flarm_decode is developed by Stanislaw Pusep, http://github.com/creaktive

   16.1
   - changing ID and selector
   16

   - add absolute coords to tracking packet.
   - added /r for end of incoming nmea lines. ( nexus )
   - added flarm board
   - added selector for android or kobo.
   - added gsm ground station option


   15
   - removing unused vars.
   - added aircraft type
   - changed speed to ms for flarm
   14
   - tidied start up lines
   - added more nmea line checks if is all zeros
   - add fix for newer android phones.
   - added start config
   - added 868/915mhz selector
   -
   ********  ESP32 code taken from here ******
   13
   - adding majority of html formatting - needs finishing.
   - fixed heading
   - fixed problems with faulty gps lines
   - fixed startup issue with gps active
   - fixed random crash after a couple hours running - no crashes now - uptime in days now.
   12
   - adding wifi password change facility into airwhere.
   - add flyskyhy option
   11
   - Added FANET
   10
   - move to Eclipse
   9
   - Sounds and various editing
   - add wifi off after 3 mins
   - add pilot name
   - add AirWhere ID in config
   8
   - Added stand alone wifi, vertica v2, OTG
   - add reconnect to station if on hardwired

*/
#include "AirWare.h"
//#include <nRF905.h>
#include <WiFiClient.h>
#include "WiFiHelper.h"
#include "WebHelper.h"
#include "WifiTools.h"
#include "fileOps.h"
#include "timeFunctions.h"
#include <SoftwareSerial.h>
#include "fanet_stack/payload.h"
#include "fanet.h"

#include "xmodem.h"

#include <DNSServer.h>

extern "C" {
#include <user_interface.h>
#include "Esp.h"
}

//#include "airPKT.h"

#include "CalcTools.h"
#include "RFdata.h"
//#include "flarm_codec.h"

// ************************************************************************************ REmember to change back!
#ifdef LIVE_VERSION
int AwVersion = 18;
#else
int AwVersion = 0;
#endif

//#include "sx1272.h"

#include "app.h"
App app = App();

//v16
#include "flarm.h"
Flarm flarm = Flarm();

#include "gsm.h"
Gsm gsm = Gsm();

#define FLARM_BOARD_BAUD 115200
#define FLARM_BOARD_SERIAL_TIMEOUT 0

//SX1272 sx1272;

//airWarePkt airPKT;
//airWarePkt airPKTRX;


IPAddress apIP(192, 168, 4, 1);
IPAddress airWareIP(37, 128, 187, 9);
#ifdef LOGGING_ON
IPAddress loggingIP(37, 128, 187, 9);
String loggingS = "";
bool logIt = false;
#endif

bool ipFound = false;
IPAddress apFoundIP;
bool webUpload = false;
char airWareSsid[32]={'\0'};
char airWarePassword[64]={'\0'};
char airWhere_ap_password[32]="12345678";

DNSServer dnsServer;
WiFiClient airWareClient;

bool dhcp_client_connected = false;
WiFiClient LK8000Client;
bool socket_connected = false;
int current_pos = 1;
IPAddress LK8000iPAddress(192, 168, 4, 2);

const int lk8000TcpServerPort = 4353;

String GPGGAdata = "";
bool GPGGAdataReceived = false;

String GPRMCdata = "";
bool GPRMCdataReceived = false;
bool displayedclientconnected = false;

//byte TxBuffer[PKT_SIZE];
//byte RxBuffer[PKT_SIZE];

long tcp_rx_weblines_counter = 0;
long serial_rx_packets_counter = 0;
//long nrf_rx_packets_counter = 0;
long tcp_tx_weblines_counter = 0;
long serial_tx_packets_counter = 0;
int longest_loop_time = 0;
int flush_time_counter = 0;
long web_upload_counter = 0;
long bad_rx_packets = 0;

GPGGAdataLine GPGGAdataIn ("");
GPRMCdataLine GPRMCdataIn("");

bool startLoop = true;
unsigned long t = 0;
int climbRate = 0;

String TCPBuffer = "";


//ufo_t fo;

char navSofware;
// L - LK
// X - XCSOAR
char receiveFlarm; // dont receive;

int loopTime = 0;
unsigned int gsTimer=GSTIMERINTERVAL;
//**************************CHECK****************************
uint32 free_mem;
unsigned int looking_for_socket_time;
unsigned int last_client_connected_time;
bool set_Connected_Time = false;
unsigned int last_data_received;
int last_serial_send;
unsigned int webTime;

int myChipID;

char groundStationMode = 'n';
char gsLatitude[11];
char gsLongitude[12];
char gsAltitude[6];
bool populatedGSdata=false;

int PFLout=0;

char wiredDirectly = 'n';
//char wiredDirectly;


int lastUdpPacketTime=0;

int gpsBaudRate=9600;
int airWhereID=0;
char awHexID[5]="0000";
char awHexManu[3]="00";
char wifioffSelected='n';
char awPilotName[30];
char loraFrequency='8';
char web_aircraft_type='1';

String cnl;

//uint8_t loraData[RF_PACKETS][RF_PACKET_SIZE] = {0};


//bool payloadReady[RX_PACKETS] = {false};
//Payload payloadList[RX_PACKETS];

int payloadReady[RX_PACKETS] = {0};
long int payload_time_to_send[RX_PACKETS] = {0};
Payload payloadList[RX_PACKETS];

unsigned long pflauSendTime=5000;
unsigned long pflauCounter=0;
bool addpflau;
bool rfPktToSend=false;
unsigned int spacerTime=0;

String streamData="";

//int16_t _RSSIpacket;
int8_t _SNR;

unsigned int wifiDownTime=0;
bool wifiUp=true;

//int16_t _RSSIpackets[RF_PACKETS]= {0};

//v16
// Flarm board
SoftwareSerial flarmSerial(14, 12, false, 256);
char lora_or_flarm='l'; //default to lora
bool flarm_board_ok=false;
char manufacturer_code[3]={'\0'};
char flarm_board_id[5]={'\0'};
char flarm_board_expiration[14]={'\0'};
char flarm_board_build[13]={'\0'};
bool flarm_tracking_on=false;
bool fanet_tracking_on=false;
bool flarm_needs_reset=false;
int flarm_reset_time;
#define FLARM_RESET_SPACE 30000;

//v16 - gsm ground station.




#ifdef TEST_PILOT
int addPilotTime=10000;
int pilotInterval=1000;
int pilotA=12345;
double pilotARadsDiff=.05;
double pilotADistance=50;
double pilotACurrentTrack=0;
double pilotAX=0;
double pilotAY=0;
double pilotAlt=0;
double pilotAltDiff=5;
#endif

// gsm
//v16
char groundstation_type='i'; // internet based groundstation as default
char gsm_access_point_name[40]={};
char gsm_username[30]={};
char gsm_password[30]={};
bool gsm_connected=false;

char packet_repeat='n';
long int heap_out=0;

/*void rx(int length)
{
  // Need to build a multidimensional array of packets received to be processed.....
  // use a system where array is initialised to 0, check for first character in each array, if its 0, we have nothing
  // to process, if theres anything else in it, then process whats there, then initialise.
  bool foundSlot=false;
  //Serial.println("received");

  for ( int pktNumber=0;pktNumber<RF_PACKETS;pktNumber++)
  {

  //  Serial.print("Rx");
  //  Serial.println(millis());

    if (loraData[pktNumber][0]==0 && !foundSlot)
    {
    //  Serial.print("empty slot");
   //   Serial.println(pktNumber);

   //   int rx = sx1272.getFrame(loraData[pktNumber], sizeof(loraData[pktNumber]));

   //   sx1272.getSNR();

    //  sx1272.getRSSIpacket(_RSSIpackets[pktNumber]);

      Serial.print(millis());
      Serial.print(" _SNR is :- ");
      Serial.print(_SNR);
      Serial.print(" Rssi is :- ");
      Serial.println(_RSSIpackets[pktNumber]);

      foundSlot=true;
      break;
    }
  }
}
*/

void setup()
{


 noInterrupts();

//	analogWrite(2, 0);
  //system_restart();
#ifndef LIVE_VERSION
  system_set_os_print(0); // stops all output to uart
#endif
  // ESP.wdtDisable();

  // wdtFeed() feeds watchdog
/*
if ( receiveFlarm=='y')
{
  nRF905_init();
  nRF905_setFrequency(NRF905_BAND_868 , RF_FREQ);
 // nRF905_setFrequency(NRF905_BAND_433 , 433200000UL);
  nRF905_setTransmitPower(NRF905_PWR_10);
  nRF905_setCRC(NRF905_CRC_16);
  byte addr[] = RXADDR;
  nRF905_setRXAddress(addr);
  nRF905_receive();
}
*/


  awHexID[0]='\0';

  load_configFile();

  if (lora_or_flarm=='f')
  {
      Serial.begin(gpsBaudRate);
      flarmSerial.begin(115200);
      Serial1.begin(115200);
   //   wiredDirectly = 'y';
  }
  else
  {
      Serial.begin(gpsBaudRate);
  }

  delay(200);



  sprintf(awHexManu, "%02X",FANET_MANUFACTURER);

  //if in gsm mode, send all debug output to serial1 as we are using rx and tx for gsm.
  // also stop the airwhere connecting to the net as we are now on gsm.
  if ( groundStationMode == 'y' && groundstation_type=='g')
  {
      airWareSsid[0]='\0';
      Serial1.end();
  }

  if ( wiredDirectly == 'y' )
  {

    Serial1.begin(57600);

    // GPS Baud Rate
    //swSer.begin(57600);
    //swSer.begin(9600);
    Serial1.print("\nAirWhere ");
 //   Serial1.printf("%02d",FANET_MANUFACTURER);
 //   Serial1.print(awHexID);
    Serial1.print(AwVersion);
    Serial1.println(" Starting");
  }

  if ( wiredDirectly == 'y' )
  {
      Serial1.printf("\r\nAirWhere ESP866 compiled on %s at %s - Version %d\r\n", __DATE__, __TIME__, AwVersion );
  }
  else
  {
      Serial.printf("\r\nAirWhere ESP866 compiled on %s at %s - Version %d\r\n", __DATE__, __TIME__, AwVersion );
  }

  if ( wiredDirectly == 'o' || wiredDirectly == 'v' || wiredDirectly == 'f' )
  {

      if (lora_or_flarm=='f')
      {
          Serial.print("AirWhere ");
       //   Serial.printf("%02d",FANET_MANUFACTURER);
      //    Serial.print(awHexID);
          Serial.print(AwVersion);
          Serial.println(" Starting");
      }
      else
      {
          Serial.print("AirWhere ");
       //   Serial.printf("%02d",FANET_MANUFACTURER);
      //    Serial.print(awHexID);
          Serial.print(AwVersion);
          Serial.println(" Starting");
      }

  }



  webTime = 0;
  last_client_connected_time = millis();
  last_data_received = millis();
  delay(100);

  //v16
  if (lora_or_flarm=='f')
  {
      //v16
      //start flarm board.

      flarm.reset();
      flarm.do_online_tracking=true;
      switch (web_aircraft_type)
      {
      case '1':
          flarm.aircraft_type=1;
          break;
      case '2':
          flarm.aircraft_type=2;
          break;
      case '3':
          flarm.aircraft_type=3;
          break;
      case '4':
          flarm.aircraft_type=4;
          break;
      }

     // flarmSerial.begin(FLARM_BOARD_BAUD);
     // flarmSerial.setTimeout(FLARM_BOARD_SERIAL_TIMEOUT);
      flarmSerial.begin(FLARM_BOARD_BAUD);
      flarmSerial.setTimeout(FLARM_BOARD_SERIAL_TIMEOUT);


     // if ( flarm.begin(flarmSerial, manufacturer_code, flarm_board_id, flarm_board_expiration, flarm_board_build, flarm_tracking_on, fanet_tracking_on ) )
      if ( flarm.begin(flarmSerial,Serial1,  manufacturer_code, flarm_board_id, flarm_board_expiration, flarm_board_build, flarm_tracking_on, fanet_tracking_on ) )
      {
          if ( wiredDirectly == 'y')
          {
              Serial1.println("Flarm board Found");
          }
          else
          {
              Serial.println("Flarm board Found");
          }
      }
      else
      {

          if ( wiredDirectly == 'y')
          {
              Serial1.println("Error :- Flarm board selected but not Found - check wiring.");
          }
          else
          {
              Serial.println("Error :- Flarm board selected but not Found - check wiring.");
          }
      }

      if (!fanet_tracking_on || !flarm_tracking_on )
      {
          flarm_needs_reset=true;
          flarm_reset_time=millis()+FLARM_RESET_SPACE;
      }

      strcpy(awHexManu,manufacturer_code);
      strcpy(awHexID,flarm_board_id);

  }

  delay(100);

  if ( wiredDirectly == 'y'  )
  {
    Serial1.println("AirWhere attempting to connect to Internet");
  }
  if ( wiredDirectly == 'o' || wiredDirectly == 'v' || wiredDirectly == 'f' )
  {
    Serial.println("AirWhere connecting to Internet");
  }

  Serial.flush();
  Serial.end();

  //v16.1 move here.
  WiFi_setup();
  Web_setup();

  Serial.begin(gpsBaudRate);

  if (lora_or_flarm=='l')
  {


      app.begin();
      app.do_online_tracking=true;

      switch (web_aircraft_type)
      {
      case '1':
          app.aircraft_type=1;
          break;
      case '2':
          app.aircraft_type=2;
          break;
      case '3':
          app.aircraft_type=3;
          break;
      case '4':
          app.aircraft_type=4;
          break;
      }


      if (fmac.begin(app))
      {
          if ( wiredDirectly == 'y')
          {
              Serial1.println("RF LORA - attached");
          }
          else
          {
              Serial.println("RF LORA - attached");
          }
      }
      else
      {

          if ( wiredDirectly == 'y')
          {
              Serial1.println("RF LORA - Not Attached - Please check wiring");
          }
          else
          {
              Serial.println("RF LORA - Not Attached - Please check wiring");
          }
      }


  }

  // int manuint=


   if (awHexID[0]=='\0')
   {
       char * pEnd;
       fmac.my_addr=MacAddr(strtol( awHexManu, &pEnd, 16),airWhereID);
       Serial.print("Original AirWhere ID - ");
       Serial.print("PLease update your ID in config and save ");
       Serial.println(fmac.my_addr.id);

   }
   else
   {
       char * pEnd;
       fmac.my_addr=MacAddr(strtol( awHexManu, &pEnd, 16),strtol( awHexID, &pEnd, 16));
       Serial.print("New AirWhere ID ");
       Serial.println(fmac.my_addr.id,HEX);
   }





  if ( groundStationMode == 'y')
   {
       if (webUpload)
       {
           if ( wiredDirectly == 'y' )
           {
               Serial1.println("AirWhere successfully connected to the Internet");
           }
           else
           {
               Serial.println("AirWhere successfully connected to the Internet");
           }
       }
       else
       {

           if ( wiredDirectly == 'y' )
           {
               Serial1.println("AirWhere failed to connect to Internet");
           }
           else
           {
               Serial.println("AirWhere failed to connect to Internet");
           }
       }
   }
   else
   {
    if ( (wiredDirectly == 'y' && webUpload)  )
    {
      Serial1.println("AirWhere connected to Internet");
    }
    else
    {
        Serial1.println("AirWhere failed to connect to Internet");
    }

    if ( (wiredDirectly == 'o' || wiredDirectly == 'v' || wiredDirectly == 'f' ) && webUpload)
    {
      Serial.println("AirWhere connected to Internet");
    }
    else
    {
        Serial.println("AirWhere failed to connect to Internet");
    }
   }

  if ( wifioffSelected == 'y')
  {
    wifiDownTime=180000;
  }
  else
  {
    wifiDownTime=0;
  }
  delay(500);

  GPGGAdataIn.isLineValid=false;
  GPRMCdataIn.isLineValid=false;

  //v16 - gsm setup.

  if ( groundStationMode == 'y' && groundstation_type=='g')
  {

     if( !gsm.begin(gsm_access_point_name,gsm_username,gsm_password))
     {
         Serial.println("Sim800L Start Up failed - check wiring and power supply ( >2amp)");
     }
     else
     {
         gsm_connected=true;
         Serial.println("Sim800L Start Up Success - GPRS Enabled.");
     }

     if (gsm_connected)
     {
         gsm.sendAT();
         delay(1000);
         if (!gsm.connectUDP())
         {
             Serial.println("No UDP Connection to GSM - waiting for keep alive for reboot");
         }

         for (int f=0;f<2;f++)
         {
             delay(1000);
             gsm.sendUDP("UDP Test  " + String(f));
         }
     }
  }

  if (!gsm_connected)
  {
      if ( wiredDirectly == 'y' )
      {
          Serial1.print("AirWhere ");
          Serial1.print(awHexManu);
          Serial1.print(awHexID);
          Serial1.println(" configuration completed, lets go !");
      }
      if ( wiredDirectly == 'o' || wiredDirectly == 'v' || wiredDirectly == 'f')
      {
          Serial.print("AirWhere ");
          Serial.print(awHexManu);
          Serial.print(awHexID);
          Serial.println(" configuration completed, lets go !");
      }

      if (groundStationMode == 'y')
      {
          if (groundstation_type=='i')
          {
              Serial.println("Ground Station Mode Activated - Waiting for Pilots.... ");
          }
          else
          {
              Serial.println("Ground Station Mode Activated - Waiting for Pilots.... ");
          }

      }
  }

  interrupts();

/*  Serial.println(ESP.getCpuFreqMHz());
  Serial.println(ESP.getFreeSketchSpace());
  Serial.println(ESP.getVcc());
  Serial.println(ESP.getResetReason());
  Serial.println(ESP.getFreeHeap());

*/
}

void loop()
{
//delay(1100);
    yield();

    //v16
  if (lora_or_flarm=='l')
  {
    fmac.handle();
  }
  else
  {
      // give it 30 seconds to handle any web handling and then reset again.
    if (flarm_needs_reset && millis()>flarm_reset_time)
    {
        flarm.reset();
        //if (flarm.begin(flarmSerial, manufacturer_code, flarm_board_id, flarm_board_expiration, flarm_board_build, flarm_tracking_on, fanet_tracking_on ))
        if (flarm.begin(flarmSerial,Serial1, manufacturer_code, flarm_board_id, flarm_board_expiration, flarm_board_build, flarm_tracking_on, fanet_tracking_on ))
        {
            flarm_needs_reset=false;
            if ( wiredDirectly == 'y')
            {
                Serial1.println("Flarm board Found");
            }
            else
            {
                Serial.println("Flarm board Found");
            }

        }
        else
        {
            flarm_needs_reset=true;
            flarm_reset_time=millis()+FLARM_RESET_SPACE;
            if ( wiredDirectly == 'y')
            {
                Serial1.println("Error :- Flarm board selected but not Found - check wiring.");
            }
            else
            {
                Serial.println("Error :- Flarm board selected but not Found - check wiring.");
            }
        }
    }
    else
    {
      //flarm.handle(flarmSerial);
        flarm.handle(Serial1,flarmSerial);
    }
  }

  yield();

  if ( millis() > wifiDownTime && wifiDownTime!=0 && ( wiredDirectly == 'y' || wiredDirectly == 'o' || wiredDirectly == 'v' || wiredDirectly == 'f'))
  {
     WiFi.mode(WIFI_OFF);
     WiFi.forceSleepBegin();
     wifi_set_sleep_type(MODEM_SLEEP_T);
     wifiDownTime=0;
     wifiUp=false;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
   DEBUG_SERIAL_UART_MAX("[%d] - ********** WIFI DOWN - WiFi.status = %d ************** \r\n", millis(),WiFi.status());
   //Serial1.println(" ********************* WIFI DOWN  ******************** \r\n");
  }
  else
  {
   DEBUG_SERIAL_UART_MAX("[%d] - ************ WIFI UP - WiFi.status = %d :)  ************** \r\n", millis(),WiFi.status());
  //    Serial1.println(" **************************** WIFI UP :)  ********************* \r\n");
  }
  DEBUG_SERIAL_UART_MAX("[%d] - Starting Loop\r\n", millis());

//  bool startLoop = true;

  if (millis() > t)
  {
    int eT = (int)millis();
    int sT = (int)t;



    //free_mem = system_get_free_heap_size();

    loopTime = eT - sT;

    //println("Looptime" + loopTime);
#ifdef TIMINGS
    if (  loopTime > 15)
    {
      DEBUG_SERIAL_UART_MAX("[%d] - Looptime [%d]\r\n", millis(), loopTime);
      DEBUG_SERIAL_UART_MAX("[%d] - Free Memory [%d]\r\n", millis(), free_mem);
    }
#endif

    if ( loopTime > longest_loop_time)
    {
      longest_loop_time = loopTime;
      if (longest_loop_time > 1000)
      {
        longest_loop_time = 0;
      }
    }
    t = millis();
  }


  //v16

  if (lora_or_flarm=='l')
  {

      if (app.is_broadcast_ready(NUM_NEIGHBOURS))
      {
          //if ( millis() > spacerTime)
          // {
          //int tx = sx1272.sendFrame(TxBuffer, PKT_SIZE+1);

          //  app.broadcast_successful(FANET_PACKET_TYPE);
          app.get_frame();
          //  Serial.println("in Transmit");

          int tx = 0;
          // if ( wiredDirectly == 'y' )
          //  {
          if ( tx==0 )
          {

              if ( wiredDirectly == 'y' || wiredDirectly == 'o' || wiredDirectly == 'v' || wiredDirectly == 's' || wiredDirectly == 'f')
              {
                  // streamData=("$AWARE,TRANSMITTING-LOCATION\r\n"+streamData);
                  //queue stream data up , not just send - needs ordering.
                  writeDataToSerial("$AIRWHERE,TRANSMITTING-LOCATION,*");

              }
              else
              {
                  // network stuff goes straight out as we send out a line at time.
                  // need to merge standalone stuff.
                  if (navSofware == 'L')
                  {
                      writeDataToWifi( LK8000Client, "$AIRWHERE,TRANSMITTING-LOCATION,*");
                  }
                  if (navSofware == 'X' || navSofware == 'A' || navSofware == 'K')
                  {
                      sendUDPPacket ( "$AIRWHERE,TRANSMITTING-LOCATION,*", apFoundIP );
                  }
              }
              rfPktToSend=false;
          }
          else
          {
              if ( wiredDirectly == 'y' || wiredDirectly == 'o' || wiredDirectly == 'v' || wiredDirectly == 's' || wiredDirectly == 'f')
              {
                  //streamData=("$AWARE,DEVICE FAILURE - CHECK BOARD & WIRING \r\n"+streamData);
                  //queue stream data up , not just send - needs ordering.
                  writeDataToSerial("$AIRWHERE,DEVICE FAILURE - CHECK BOARD & WIRING");
              }
              else
              {
                  if (navSofware == 'L')
                  {
                      writeDataToWifi( LK8000Client, "$AIRWHERE,DEVICE FAILURE - CHECK BOARD & WIRING  ");
                  }
                  if (navSofware == 'X'|| navSofware == 'A' || navSofware == 'K' )
                  {
                      sendUDPPacket ( "$AIRWHERE,DEVICE FAILURE - CHECK BOARD & WIRING  ", apFoundIP );
                  }
              }
              rfPktToSend=false;

          }

          if ( tx==-1 )
          {
              //    streamData=("$AWARE,CANT TRANSMIT TX ONGOING\r\n"+streamData);
          }
          if ( tx==-2 )
          {
              //     streamData=("$AWARE,CANT TRANSMIT RX ONGOING\r\n"+streamData);
          }

          //   }


      }
  }


 // unsigned long startTime = millis();
  bool success = false;
  String lineout;
  unsigned char number_client;

  if ( wiredDirectly == 'n' || wiredDirectly == 's' )
  {
    number_client = wifi_softap_get_station_num();
  }
  else
  {
    number_client = 1;
    socket_connected = true;
  }

  DEBUG_SERIAL_UART_MAX("[%d] - Number of Clients =%d \r\n", millis(), number_client);

  if (groundStationMode == 'n')
  {
    if (number_client == 0)
    {
      DEBUG_SERIAL_UART("[%d] - No clients connected to AP\r\n", millis());


      ipFound = false;
      socket_connected = false;
      myDelay(100);
      //DEBUG_SERIAL_UART("[%d] - set_Connected_Time is set to FALSE : %d \r\n", millis(),client_connected_time);
      set_Connected_Time = false;

      // no one has connected in 90 secs ( 90000 millis ), restart wifi to make sure theres not something weird gone on with wifi.
      // until we find the problem
      if ( (last_client_connected_time + 45000) < millis())
      {
        DEBUG_SERIAL_UART("[%d] - Restarting WIFI - No connect on Wifi\r\n", millis());
        last_client_connected_time = millis();
        LK8000Client.stop();
        yield();
        myDelay(500);
        Serial.println("Restarting - No connect on Wifi");
        ESP.restart();
        WiFi_setup();
      }
    }
    else
    {
      // last_ is just a marker for restarting the WIFI if no clients connected.
      last_client_connected_time = millis();
      DEBUG_SERIAL_UART("[%d] - Setting Last Client_connected_time=%d \r\n", millis(), last_client_connected_time);

      // client is for restarting wifi if a client has connected and it cant find a socket.
      if (!set_Connected_Time)
      {
        // delay for the dchp server to work its magic
        yield();
        myDelay(3000);
        yield();
        looking_for_socket_time = millis();
        set_Connected_Time = true;
      }


      if (!displayedclientconnected)
      {
        DEBUG_SERIAL_UART("[%d] - Client connected on DHCP\r\n", millis());
#ifdef SERIAL_UART_OUT
        client_status();
#endif


        displayedclientconnected = true;
      }
    }

    yield();

    if (number_client > 0 && !socket_connected)
    {
      // We need to check all the connected clients for a socket.
      for (int clientNo = 1; clientNo <= number_client; clientNo++)
      {
        if (ipFound)
        {

          LK8000iPAddress = apFoundIP;
        }
        else {
          LK8000iPAddress = getClientNumber (clientNo);
        }

        DEBUG_SERIAL_UART("[%d] - Connecting to found IP\r\n", millis());
        //  DEBUG_SERIAL_UART("[%d] - Trying connected IP : %d \r\n",millis(),LK8000iPAddress);


        yield();

        bool lkConnect1 = false;

        DEBUG_SERIAL_UART("[%d] - webTime 1 = %d\r\n", millis(), webTime);

        if (millis() > webTime)
        {
          lkConnect1 = LK8000Client.connect(LK8000iPAddress, lk8000TcpServerPort);
        }

        if (!lkConnect1)
        {

          DEBUG_SERIAL_UART("[%d] - Connection failed, clients connected are :- \r\n", millis());
#ifdef SERIAL_UART_OUT
          client_status();
#endif


        }
        else
        {
          DEBUG_SERIAL_UART("[%d] - *** Connection to Flight Software on Found IP *** \r\n", millis());
          LK8000Client.setNoDelay(true);

          LK8000Client.setTimeout(1);
          socket_connected = true;
          last_data_received = millis();
          apFoundIP = LK8000iPAddress;
          ipFound = true;
          break;
        }

        Web_loop();
        yield();
        myDelay(100);

        DEBUG_SERIAL_UART("[%d] - Failed to connect to client \r\n", millis());
        // Serial.println(LK8000iPAddress);
        DEBUG_SERIAL_UART("[%d] - Kobo reconnected but not asked for IP, go looking for connection \r\n", millis());



        //   WiFi_setup();

        yield();
        //check to see if they are still connected...
        number_client = wifi_softap_get_station_num();

        int iPs[4] = {100, 101, 2, 3};

        for ( int ip = 0; ip < 4; ip++)
        {
          int t=millis()+1000;

          while (t>millis())
          {
              yield();
              Web_loop();
          }

          yield();
          // client_connected_time is set when a client connected, if either no clients connect or a client has connected and cant find a socket
          // then restart the ESP, also the ESP sometimes doesnt realise a client has disconnected so just sits there trying to find a socket
          // if so this should restart it.
          DEBUG_SERIAL_UART("[%d] - looking_for_socket_time=%d \r\n", millis(), looking_for_socket_time);

          if ( looking_for_socket_time + 45000 < millis())
          {
            DEBUG_SERIAL_UART("[%d] - Not released Client - cant find a socket - Restarting Access Point \r\n", millis());
            LK8000Client.stop();
            yield();
            myDelay(500);
            //WiFi_setup();
            Serial.println( "Not released Client - cant find a socket - Restarting");
            ESP.restart();
          }

          number_client = wifi_softap_get_station_num();

          if ( number_client == 0 )
          {
            ipFound = false;
            socket_connected = false;
            break;
          }

          DEBUG_SERIAL_UART("[%d] - Trying 192.168.4.%d \r\n", millis(), iPs[ip]);



          IPAddress LK8000iPAddress(192, 168, 4, iPs[ip]);

          //LK8000iPAddress=LK8000iPAddress(192, 168, 4, 1);


          bool lkConnect2 = false;

          DEBUG_SERIAL_UART("[%d] - webTime 2 = %d \r\n", millis(), webTime);

          if (millis() > webTime)
          {
            lkConnect2 = LK8000Client.connect(LK8000iPAddress, lk8000TcpServerPort);
          }

          if (!lkConnect2)
          {
            DEBUG_SERIAL_UART("[%d] - Connection failed on :- 192.168.4.%d \r\n", millis(), iPs[ip]);

          }
          else
          {
            DEBUG_SERIAL_UART("[%d] - *** Connection to Flight Software on IP :- 192.168.4.%d *** \r\n", millis(), iPs[ip]);
            LK8000Client.setNoDelay(true);


            LK8000Client.setTimeout(1);
            socket_connected = true;
            last_data_received = millis();
            apFoundIP = LK8000iPAddress;
            ipFound = true;
            break;
          }
          Web_loop();
          yield();
          myDelay(100);
        }
        Web_loop();
        yield();
        myDelay(100);
      }
    }


    DEBUG_SERIAL_UART_MAX("[%d] - Checking for data from TCP\r\n", millis());

    if ( socket_connected)
    {
      looking_for_socket_time = millis();



      DEBUG_SERIAL_UART_MAX("[%d] - Socket Connected\r\n", millis());

     // char c;
      String s;
     // bool lineend = false;
      bool fullLineReturned;

      if (LK8000Client.connected() || wiredDirectly=='y' || wiredDirectly=='o' || wiredDirectly == 'v' || wiredDirectly == 'f' )
      {


        if (TCPBuffer.length() > TCP_BUFFER_MAX_LENGTH)
        {
          // bad slow down somewhere, so clear out the buffer and hopefully it will clear it self up.
          DEBUG_SERIAL_UART("[%d] - +++++++++++++++++++FLUSHING TCP BUFFER++LENGTH IS %d ++++++++++++\r\n", millis(), TCPBuffer.length());
          flush_time_counter++;
          TCPBuffer = "";


        }
        DEBUG_SERIAL_UART_MAX("[%d] - Before readAllDataOnWifi \r\n", millis());

        if ( wiredDirectly=='n' || wiredDirectly == 's')
        {
          if ( wiredDirectly == 's' )
          {
            TCPBuffer = readAllDataOnWifiTxSerial(LK8000Client, TCPBuffer, fullLineReturned);
          }
          else
          {
            TCPBuffer = readAllDataOnWifi(LK8000Client, TCPBuffer, fullLineReturned);
          }

         // add for wifi tx version
        //  TCPBuffer = readAllDataOnWifiTxSerial(LK8000Client, TCPBuffer, fullLineReturned);

        }
        else
        {
          // read serial or softserial
           TCPBuffer = readAllDataOnSerial(TCPBuffer, fullLineReturned);
        }



        DEBUG_SERIAL_UART_S("[" + String(millis()) + "] - TCPBuffer length is :>" + TCPBuffer.length() + "\r\n");
        DEBUG_SERIAL_UART_S("[" + String(millis()) + "] - TCPBuffer >>" + TCPBuffer + "\r\n");
        DEBUG_SERIAL_UART_MAX("[%d] - After readAllDataOnWifi \r\n", millis());

        //if the socket is connected and we havent receieved data for 60 seconds then sometimes not very well, restart wifi
        // its high at 60 as the kobo doesnt realise its fallen off the AP and will just sit there error transmitting
        // usually rejoins after about 20-30 secs so hopefully it will be long enough

        if ( last_data_received + 200000 < millis())
        {
          DEBUG_SERIAL_UART("[%d] - AP connected:Socket connected - no data incoming - Restarting Access Point \r\n", millis());
          LK8000Client.stop();
          yield();
          myDelay(500);
          Serial.println( "AP connected:Socket connected - no data incoming - Restarting Access Point");
          ESP.restart();
        }

        //int nextDollarPos = 0;
        int newLine = 0;
        //v16
        //newLine = TCPBuffer.indexOf("\n");

        if (TCPBuffer.indexOf("\n")>0)
        {
            newLine=TCPBuffer.indexOf("\n");
        }
        else
        {
            if (TCPBuffer.indexOf("\r")>0)
            {
              newLine=TCPBuffer.indexOf("\r");
            }
        }


/*if (TCPBuffer.length()>0)
{
    Serial.println("");
    Serial.print(">>>>>>>>>>");
Serial.print(TCPBuffer);
Serial.println("<<<<<<<<<");
  Serial.println(TCPBuffer.length());
  Serial.println("<<<<<<<<<");
}*/

        DEBUG_SERIAL_UART_MAX("[%d] - Before TCPBuffer.substring(0, 6) == \"$GPGGA\"  \r\n", millis());

        if ((TCPBuffer.substring(0, 6) == "$GPGGA" || TCPBuffer.substring(0, 6) == "$GNGGA") && fullLineReturned)
        {


          String currentNMEALine = TCPBuffer.substring(0, newLine);
          cnl = currentNMEALine;
          tcp_rx_weblines_counter++;

          DEBUG_SERIAL_UART_MAX("[%d] - Into $GPGGA  \r\n", millis());
          climbRate = GPGGAdataIn.getPilotAltitude();

          GPGGAdataIn.rePopulate(currentNMEALine);

       //   if (GPGGAdataIn.isLineValid) {Serial.print("GPGGAdataIn LineValid");}else{Serial.println("GPGGAdataIn.NOT isLineValid");}
          //Serial.println("GPGGA:"+TCPBuffer);
          // GPGGAdataIn.rePopulate(TCPBuffer); - REMOVED



          climbRate = climbRate - GPGGAdataIn.getPilotAltitude();
          GPGGAdataReceived = true;



          if ( wiredDirectly == 'n' )
          {

            if (navSofware == 'L')
            {
              DEBUG_SERIAL_UART_S("[" + String(millis()) + "] - Uploading GPPGA data to LK >>" + currentNMEALine + "\r\n");

              tcp_tx_weblines_counter++;
              writeDataToWifi( LK8000Client, GPGGAdataIn.getDataWithoutCksum());
            //DEBUG_SERIAL_UART_S("[" + String(millis()) + "] - Uploaded GPPGA data to LK >>" + currentNMEALine + "\r\n");
              tcp_tx_weblines_counter++;
              writeDataToWifi( LK8000Client, "$PFLAU,6,1,2,1,0,144,0,235,446*");
            //   DEBUG_SERIAL_UART_S("["+String(millis()) + "] - Uploaded PFLAU data to LK >>" + currentNMEALine + "\r\n");
            }
            if (navSofware == 'X' || navSofware == 'A' || navSofware == 'K')
            {

              DEBUG_SERIAL_UART_S("[" + String(millis()) + "] - Uploading GPPGA data to XCS >> >>" + currentNMEALine + "\r\n");

              sendUDPPacket ( GPGGAdataIn.getDataWithoutCksum(), apFoundIP );
              tcp_tx_weblines_counter++;
              sendUDPPacket ( "$PFLAU,6,1,2,1,0,144,0,235,446*", apFoundIP );
              tcp_tx_weblines_counter++;
            }
          }
       //   Serial.println("\nRemoving a line GPPPA line found");
          TCPBuffer = TCPBuffer.substring(newLine + 1);
        }
        else
        {
          if ((TCPBuffer.substring(0, 6) == "$GPRMC" || TCPBuffer.substring(0, 6) == "$GNRMC") && fullLineReturned)
          {
//Serial.print("GPRMC:"+TCPBuffer);

            DEBUG_SERIAL_UART_MAX("[%d] - Into $GPRMC  \r\n", millis());
            tcp_rx_weblines_counter++;
          //v16
            //newLine = TCPBuffer.indexOf("\n");
            if (TCPBuffer.indexOf("\n")>0)
            {
                newLine=TCPBuffer.indexOf("\n");
            }
            else
            {
                if (TCPBuffer.indexOf("\r")>0)
                {
                  newLine=TCPBuffer.indexOf("\r");
                }
            }
            String currentNMEALine = TCPBuffer.substring(0, newLine);
            GPRMCdataIn.rePopulate(currentNMEALine);
           // if (GPRMCdataIn.isLineValid) {Serial.print("GPRMCdataIn LineValid");}else{Serial.println("GPRMCdataIn.NOT isLineValid");}

            if (lora_or_flarm=='l')
            {
                app.set_gps_lock(GPRMCdataIn.getGpsActive());
            }
            else
            {
                flarm.set_gps_lock(GPRMCdataIn.getGpsActive());
            }


            GPRMCdataReceived = true;
   //         Serial.println("\nRemoving a line RMC line found");
            TCPBuffer = TCPBuffer.substring(newLine + 1);

          }
          else
          {
            // get rid of another $ or the line is empty
            DEBUG_SERIAL_UART_MAX("[%d] - Into Empty  \r\n", millis());

            // only remove the next line if a full line is returned !

            if (fullLineReturned)
            {
       //         Serial.println("\nRemoving a line no line found");
              TCPBuffer = TCPBuffer.substring(newLine + 1);
            }
          }
        }

        /*   int iAt = TCPBuffer.indexOf('$');

           if (iAt == -1)
           {
             TCPBuffer = "";
           }
           else
           {
             TCPBuffer = TCPBuffer.substring(iAt);
           }

        */
      }
      else
      {





        DEBUG_SERIAL_UART("[%d] - Client Disconnected -  Trying to reconnect\r\n", millis());


        socket_connected = false;
        GPRMCdataReceived = false;
      }
    }

    yield();


    DEBUG_SERIAL_UART_MAX("[%d] - Into RF Section\r\n", millis());
    /*
      RF SECTION

      We have a full set of GPS data to upload to the Air if the GPS is Active

    */

   //if (GPGGAdataIn.isLineValid) {Serial.print("GPGGAdataIn LineValid");}else{Serial.println("GPGGAdataIn.NOT isLineValid");}
  // if (GPRMCdataIn.isLineValid) {Serial.print("GPRMCdataIn LineValid");}else{Serial.println("GPRMCdataIn.NOT isLineValid");}

 //   if (GPGGAdataIn.isLineValid) {}else{Serial.println("GPGGAdataIn.NOT isLineValid");}
   //  if (GPRMCdataIn.isLineValid) {}else{Serial.println("GPRMCdataIn.NOT isLineValid");}


    if (GPRMCdataReceived && GPRMCdataIn.getGpsActive() == 'A' && GPGGAdataIn.isLineValid && GPRMCdataIn.isLineValid)
    {

      DEBUG_SERIAL_UART_MAX("[%d] - Starting Sending of Serial if data received - GPS Active\r\n", millis());

    /*  airPKT.pktLatitude = GPGGAdataIn.getPilotLatitudeNoDot().toInt();
      airPKT.pktLongitude = GPGGAdataIn.getPilotLongitudeNoDot().toInt();
      airPKT.pktNSEW = isPilotNESW(GPGGAdataIn.getPilotNS(), GPGGAdataIn.getPilotEW() );
      airPKT.pktAltitude = GPGGAdataIn.getPilotAltitude();
      airPKT.pktTrack = GPRMCdataIn.getPilotTrack();
      airPKT.pktSpeed = GPRMCdataIn.getPilotSpeed();
      airPKT.pktCrc = 36;
*/

      if (!GPGGAdataIn.isLineValid) {Serial.println("WTF ? GPGGAdataIn.NOT isLineValid *****************");}
       if (!GPRMCdataIn.isLineValid) {Serial.println("WTF ?GPRMCdataIn.NOT isLineValid*****************");}

       float lat=GPGGAdataIn.getPilotLatitude().toFloat();
       if (GPGGAdataIn.getPilotNS()=='S' )
       {
         lat=lat*-1;
       }

       float lon=GPGGAdataIn.getPilotLongitude().toFloat();

       if (GPGGAdataIn.getPilotEW()=='W' )
       {
         lon=lon*-1;
       }

      // Serial1.print("GPGGAdataIn.getPilotLongitude() - ");Serial1.println(GPGGAdataIn.getPilotLongitude());
     //  Serial1.print("lon - ");Serial1.println(lon);

       if (lora_or_flarm=='l')
       {
           app.set( lat,
                   lon,
                   float(GPGGAdataIn.getPilotAltitude()),
                   float(GPRMCdataIn.getPilotSpeed()),
                   0, float(GPRMCdataIn.getPilotTrack()), 0);
       }
       else
       {
           //v16
           //update flarm app - this will know when to send packet.
            flarm.set( GPGGAdataIn.getFixTime(), GPRMCdataIn.getPilotDate() ,lat, lon,  float(GPGGAdataIn.getPilotAltitude()),
                   float(GPRMCdataIn.getPilotSpeed()), 0, float(GPRMCdataIn.getPilotTrack()));


           // blink a led here off another pin to show transmission.
       }




      String urlUp;

      // V15 remove - if (app.do_online_tracking)

 // v16.1

      char manuMe[3];
      char idMe[5];

      sprintf(manuMe,"%02X",fmac.my_addr.manufacturer);
      sprintf(idMe,"%04X",fmac.my_addr.id);

      if (app.do_online_tracking  && webUpload)
      {


    	// on wifi  - dest and src - same place, rssi = 0;
        // id is manufacturer + ID .

        urlUp = String ( GPGGAdataIn.getFixTime() ) + "," +
                String (manuMe) + String (idMe)  + "," +
                String (manuMe) + String (idMe) + "," +
				app.aircraft_type + "," +
                GPGGAdataIn.getPilotLatitude() + "," +
				GPGGAdataIn.getPilotLongitude() + "," +
                GPGGAdataIn.getPilotNS() + "," +
				GPGGAdataIn.getPilotEW() + "," +
				GPRMCdataIn.getPilotTrack() + "," +
				GPRMCdataIn.getPilotSpeed() + "," +
				GPGGAdataIn.getPilotAltitude() + "," +
				0;

        web_upload_counter++;
        upLoadtoUDP(urlUp, airWareIP, AIRWARE_UDP_PORT  );
      }


      if (flarm.do_online_tracking  && webUpload)
      {


        // on wifi  - dest and src - same place, rssi = 0;
        // id is manufacturer + ID .

        urlUp = String ( GPGGAdataIn.getFixTime().toInt() ) + "," +
                String (manuMe) + String (idMe)  + "," +
                String (manuMe) + String (idMe) + "," +
                flarm.aircraft_type + "," +
                GPGGAdataIn.getPilotLatitude() + "," +
                GPGGAdataIn.getPilotLongitude() + "," +
                GPGGAdataIn.getPilotNS() + "," +
                GPGGAdataIn.getPilotEW() + "," +
                GPRMCdataIn.getPilotTrack() + "," +
                GPRMCdataIn.getPilotSpeed() + "," +
                GPGGAdataIn.getPilotAltitude() + "," +
                0;

        web_upload_counter++;
        upLoadtoUDP(urlUp, airWareIP, AIRWARE_UDP_PORT  );
      }


/*
      unsigned char cs = 0;
      char crcCheck[23];

      memset(crcCheck, 0, 23);
      memcpy(&crcCheck, &airPKT, 23);

      cs = 0; //clear any old checksum

      for (unsigned int n = 0; n < 23; n++) {
        cs ^= crcCheck[n]; //calculates the checksum

      }

      airPKT.pktCRC = cs;



      memset(TxBuffer, 0, sizeof(airPKT));
      memcpy(&TxBuffer, &airPKT, sizeof(airPKT));

      */
      DEBUG_SERIAL_UART("[%d] - >>>>>>>>>>>>>>> Transmitting Pilot data On RF : Pilot ID %d\r\n", millis(), fmac.my_addr.id);




      /*  	byte addr[] = TXADDR;
        	nRF905_setTXAddress(addr);
        	nRF905_setData(data, NRF905_PAYLOAD_SIZE );
        	while(!nRF905_send()) { yield(); } ;
      */
      serial_tx_packets_counter++;
     // Serial.flush();
      //Serial.write(TxBuffer, PKT_SIZE);

     // int tx = sx1272.sendFrame((uint8_t *)str.c_str(), str.length()+1);
      rfPktToSend=true;
      spacerTime=millis()+ random(1, 750);

 /*     int tx = sx1272.sendFrame(TxBuffer, PKT_SIZE+1);

      if ( wiredDirectly == 'y' )
      {
        if ( tx==0 )
        {
          streamData=("$AWARE,TRANSMITTING-LOCATION\r\n"+streamData);
        }
        if ( tx==-1 )
        {
          streamData=("$AWARE,CANT TRANSMIT TX ONGOING\r\n"+streamData);
        }
        if ( tx==-2 )
        {
          streamData=("$AWARE,CANT TRANSMIT RX ONGOING\r\n"+streamData);
        }

      }
      */

      GPRMCdataReceived = false;
    //  GPGGAdataIn.isLineValid=false;
   //   GPRMCdataIn.isLineValid=false;
    }
    else
    {
      // Serial sometimes stops for some weird reason so after 3 mins just restart it to make it happy :)

      // NEEDS SORTING
      //  if ( (looking_for_socket_time+180000)<millis() && socket_connected)
      //  {
      //   DEBUG_SERIAL_UART("[%d] - Client connected but not sending serial - SERIAL Failure\r\n", millis());
      //system_restart();
      //  Serial.end();
      //  Serial.begin(9600);
      // }


    }
  }
  else
  {
      char manuMe[3];
      char idMe[5];

      sprintf(manuMe,"%02X",fmac.my_addr.manufacturer);
      sprintf(idMe,"%04X",fmac.my_addr.id);

    if (!populatedGSdata)
    {
     //v16.1

    //GPGGAdataIn.rePopulateDecimal(airWhereID, gsLatitude, gsLongitude, gsAltitude );
      GPGGAdataIn.rePopulateDecimal(fmac.my_addr.id, gsLatitude, gsLongitude, gsAltitude );



      DEBUG_SERIAL_UART_S("[" + String(millis()) + "] - GPGGAdataIn.getPilotLatitudeNoDot > " + GPGGAdataIn.getPilotLatitudeNoDot() + "\r\n");
      DEBUG_SERIAL_UART_S("[" + String(millis()) + "] - GPGGAdataIn.getPilotLongitudeNoDot > " + GPGGAdataIn.getPilotLongitudeNoDot() + "\r\n");

      float fixLat = GPGGAdataIn.getPilotLatitude().toFloat();
      float fixLong = GPGGAdataIn.getPilotLongitude().toFloat();

      if ( gsLatitude[0]=='-' )
      {
        fixLat = fixLat * -1;
      }
      if (gsLongitude[0]=='-' )
      {
        fixLong = fixLong * -1;
      }

      app.set( fixLat,
    		  fixLong,
			  GPGGAdataIn.getPilotAltitude(),
			  0, 0, 0, 0);

      populatedGSdata=true;
      GPRMCdataIn.setGpsActive();
      GPGGAdataIn.isLineValid=true;
      GPRMCdataIn.isLineValid=true;
            if (lora_or_flarm=='l')
            {
                app.set_gps_lock(GPRMCdataIn.getGpsActive());
            }
            else
            {
                flarm.set_gps_lock(GPRMCdataIn.getGpsActive());
            }

      for ( int i=0;i<10;i++)
      {
        String urlUp = ">GS Location," + String( manuMe ) + String( idMe ) + "," + String( gsLatitude ) + "," + String(gsLongitude) + "," + String(gsAltitude);
        delay(200);
        if (groundstation_type=='g')
        {
            if (gsm_connected)
            {
              gsm.sendUDP(urlUp);
            }
            else
            {
                Serial.println("gsm down - waiting for reboot");
            }
        }
        else
        {
          upLoadtoUDP(urlUp, airWareIP, AIRWARE_UDP_PORT);
        }
      }
    }
    if (millis()>gsTimer)
    {
       if ( groundStationMode =='y' && groundstation_type=='g')
       {

       //    Serial.println("Checking for GSM functionality");
           delay(500);
           Serial.flush();
           Serial.readString();
           Serial.flush();
           delay(500);
           gsm.closeTCP();
           delay(1000);

           String urlUp = ">GS Location," + String( manuMe ) + String( idMe ) + "," + String( gsLatitude ) + "," + String(gsLongitude) + "," + String(gsAltitude);
           if ( !gsm.connectUDP())
           {
               Serial.println("UDP Send Failed on GS Check - GSM Down - Restarting");
               yield();
               myDelay(500);

               ESP.restart();
           }
           else
           {
               Serial.println("GSM is Alive.");
               gsm_connected=true;
               delay(500);
               gsTimer=millis()+GSTIMERINTERVAL;
           }
       }
       else
       {
          // WiFiClient clientAWV;
           //   const int httpPort = 80;

           HTTPClient http;

                //  USE_SERIAL.print("[HTTP] begin...\n");
                  // configure traged server and url
                  //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
           http.begin("http://www.airwhere.co.uk/downloads/version.txt"); //HTTP

               //   USE_SERIAL.print("[HTTP] GET...\n");
                  // start connection and send HTTP header
           int httpCode = http.GET();

           if(httpCode == HTTP_CODE_OK)
           {
               //Serial.println("server Found");
               String urlUp = ">GS Location," + String( manuMe ) + String( idMe ) + "," + String( gsLatitude ) + "," + String(gsLongitude) + "," + String(gsAltitude);
               upLoadtoUDP(urlUp, airWareIP, AIRWARE_UDP_PORT);
               gsTimer=millis()+GSTIMERINTERVAL;
               Serial.print("sending keep alive message :-");
               Serial.println(urlUp);
           }
           else
           {
               Serial.println("Wifi Down Can not find server - restarting");
               yield();
               myDelay(500);

               ESP.restart();
           }
       }

    }

  }

//*********************************************************************
//******************* Serial Receive Section **************************
//*********************************************************************
// we need to make sure the first character in the Serial is a $




  DEBUG_SERIAL_UART("[%d] - Serial Available - %d \r\n", millis(), Serial.available());
  //int serialLen = Serial.available();


 // uint8_t data[128] = {0};
 // int rx = sx1272.getFrame(data, sizeof(data));

 /* if ( serialLen > 23 )
  {
    while ( Serial.peek() != '$' && serialLen > 23)
    {
      DEBUG_SERIAL_UART("[%d] - Reading a byte - %c - Serial Available - %d \r\n", millis(), Serial.peek(), serialLen);
      byte b[1];
      Serial.readBytes(b, 1);
      serialLen = Serial.available();
    }
    // if seriallen is less than 24, we have got some data without a $ so run around and get some more.
    if ( serialLen < 24)
    {
      success = false;
      DEBUG_SERIAL_UART_MAX("[%d] - Data has come in but we cant find a $ \r\n", millis());
    }
    else
    {
      DEBUG_SERIAL_UART("[%d] - Serial Packet - Serial Length - %d \r\n", millis(), Serial.available());
      serial_rx_packets_counter++;
      Serial.readBytes(RxBuffer, PKT_SIZE);

      DEBUG_SERIAL_UART("[%d] - Received some ARF data :- %d\r\n", millis(), airPKTRX.pktPilotID);

#ifdef LOGGING_ON_MAX
      loggingS = String(airPKT.pktPilotID) + "," + String(millis()) + "," + "Received some ARF data";
      upLoadtoUDP(loggingS, loggingIP, LOGGING_PORT);
#endif
   if (lora_rx )
    {
      serial_rx_packets_counter++;

      // hopefully by here we have a packet that starts with a $, so it should be one of ours, but check data is ok before proceeding.
      if (validatePKT(loraData[0], PKT_SIZE ))
      {
        memset(&airPKTRX, 0, sizeof(airPKT));
        memcpy(&airPKTRX, &loraData[0], sizeof(airPKT));

        success = true;
        DEBUG_SERIAL_UART_MAX("[%d] - Incoming 24 bytes is one of our Packets, processing Pilot :- %d\r\n", millis(), airPKTRX.pktPilotID);
      }
      else
      {
       success = false;
        DEBUG_SERIAL_UART("[%d] - **** FOUND BAD PACKET - Skipping ***** :- pilotID %d , CRC %d\r\n", millis(), airPKTRX.pktPilotID, airPKTRX.pktCrc );
      }
    loraData[0][RF_PACKET_SIZE] = {0};
    lora_rx=false;
  }
#ifdef TEST_PILOT
  else
  {
    DEBUG_SERIAL_UART_MAX("[%d] - Setting A Test Pilot\r\n", millis());
    int randomNo = random(1, 30);
    int PId = random(0, 10) * 100000;
    int loc = random(-100000, 100000);
    int loc1 = random(-100000, 100000);

    if  (randomNo == 15)
    {


      airPKTRX.pktPilotID = PId + 12345;
      airPKTRX.pktCrc = 36;

      airPKTRX.pktLatitude = GPGGAdataIn.getPilotLatitudeNoDot().toInt() + loc;
      airPKTRX.pktLongitude = GPGGAdataIn.getPilotLongitudeNoDot().toInt() + loc1;
      airPKTRX.pktAltitude = random(1, 1000);;
      airPKTRX.pktNSEW = 1;
      unsigned int tr = random(0, 360);
      airPKTRX.pktTrack = tr;
      DEBUG_SERIAL_UART_MAX("[%d] - Adding Test Pilot %d\r\n", millis(), airPKTRX.pktPilotID);
      //success=true;
      // Serial.println("adding");
      unsigned char cs = 0;

      char crcCheck[23];

      memset(crcCheck, 0, 23);
      memcpy(&crcCheck, &airPKTRX, 23);

      cs = 0; //clear any old checksum

      for (unsigned int n = 0; n < 23; n++) {
        cs ^= crcCheck[n]; //calculates the checksum
      }

      airPKTRX.pktCRC = cs;

      memset(TxBuffer, 0, sizeof(airPKT));
      memcpy(&TxBuffer, &airPKTRX, sizeof(airPKT));
      DEBUG_SERIAL_UART_MAX("[%d] - Writing Test pilot to Serial %d\r\n", millis(), airPKTRX.pktPilotID);
      Serial.flush();
      Serial.write(TxBuffer, PKT_SIZE);
      success = true;
    }
    else
    {
      success = false;
    }
  }
#endif

  yield();
*/
//*********************************************************************
//******************* nrf905 Receive Section **************************
//*********************************************************************
//***** We need to priorise the Nmea strings coming in and serial *****
//***** If we have serial packet to parse, sort that first, use *******
//***** loops with nothing on serial to process nrf905 packets ********
//***** as this wont halt or cause any problem waiting for them *******

/*if ( receiveFlarm=='y')
{

  if ( !success )
  {
    nRF905_receive();
 //   unsigned long sendStartTime = millis();

    airPKTRX.pktCrc = 0;


    success = nRF905_getData(RxBuffer, sizeof(RxBuffer));
    if (success) // Got data
    {
      nrf_rx_packets_counter++;
      //char *q;

      fo.raw = Bin2Hex(RxBuffer);

DEBUG_SERIAL_UART("[%d] - Received FLarm packet - Processing\r\n", millis());

      if (webUpload) {String urlUp = "Received FLarm packet";upLoadtoUDP(urlUp, airWareIP, AIRWARE_UDP_PORT);}

#ifdef LOGGING_ON
      loggingS = String(airPKT.pktPilotID) + "," + String(millis()) + "," + "Received some FLARM data : " + String(fo.raw);
      upLoadtoUDP(loggingS, loggingIP, LOGGING_PORT);
#endif

    //fill airPKTRX with flarm data - need to go capture some flarm data and check it out :)


      flarm_decodeAW(airPKTRX,
                   (flarm_packet *) RxBuffer,
                   LATITUDE, LONGTITUDE, ALTITUDE,
                   0,
                   0,
                   0
                  );

      airPKTRX.pktCrc = 36;

      success = true;
    }
  }

}
*/
  yield();


  // Received some RF data - Upload to LK.


  DEBUG_SERIAL_UART_MAX("[%d] - Processing received packet\r\n", millis());
  // DEBUG_SERIAL_UART("[%d] - airPKTRX.pktCrc %d\r\n", millis(), airPKTRX.pktCrc);

//*********************************************************************
//******************* Process Received Packets Section ****************
//*********************************************************************
  for ( int packetNumber=0; packetNumber <  RX_PACKETS; packetNumber++)
  {
    //if (loraData[packetNumber][0] != 0 && validatePKT(loraData[packetNumber], PKT_SIZE ))
     // {
     /*   Serial.print("processing packet number ");
        Serial.println(packetNumber);


     if (!GPGGAdataIn.isLineValid)
          Serial.println("!GPGGAdataIn.isLineValid");

      if (!GPRMCdataIn.isLineValid)
               Serial.println("!GPRMCdataIn.isLineValid");
*/

    //  if (!GPGGAdataIn.isLineValid || !GPRMCdataIn.isLineValid)
    //      writeDataToSerial("$AIRWHERE,PKT RECEIVED,WAITING FOR GPSLOCK,*");
      if (payloadReady[packetNumber]>0 && millis()>payload_time_to_send[packetNumber] && GPGGAdataIn.isLineValid && GPRMCdataIn.isLineValid)
            {
	 // if (payloadReady[packetNumber] && GPGGAdataIn.isLineValid && GPRMCdataIn.isLineValid)
	//  {
        serial_rx_packets_counter++;

      float fixLat = GPGGAdataIn.getPilotLatitude().toFloat();
      float fixLong = GPGGAdataIn.getPilotLongitude().toFloat();

      if ( GPGGAdataIn.getPilotNS() == 'S')
      {
        fixLat = fixLat * -1;
      }
      if ( GPGGAdataIn.getPilotEW() == 'W')
      {
        fixLong = fixLong * -1;
      }

      float pilotBearing = CalcBearingA( fixLat, fixLong, payloadList[packetNumber].latitude, payloadList[packetNumber].longitude);
      float pilotDistance = distance( fixLat, fixLong, payloadList[packetNumber].latitude, payloadList[packetNumber].longitude, 'K') ;
      float relNorth=0;
	  float relEast=0;

      float rads = deg2rad(pilotBearing);

      relEast = sin(rads) * pilotDistance * 1000;
      relNorth = cos(rads) * pilotDistance * 1000;

     // double speedKM = airPKTRX.pktSpeed * 0.5144;

      float relVert = payloadList[packetNumber].altitude - GPGGAdataIn.getPilotAltitude();

     // char moving_pilot_data[50]={};

     /* snprintf ( moving_pilot_data, 50, "$PFLAA,0,%s,%s,%s,2,%d%d,%s,0,%s,%d,%d*",
    		    relNorth,relEast,relVert,payloadList[packetNumber].manufacturer,payloadList[packetNumber].id,
				payloadList[packetNumber].heading,payloadList[packetNumber].speed,
				climbRate,payloadList[packetNumber].aircraft_type);

      Serial1.println(moving_pilot_data);*/

      //V15
      char flarm_aircraft_type='0';

      switch (payloadList[packetNumber].aircraft_type)
      {
        case 1:
            flarm_aircraft_type='7';
        break;
        case 2:
            flarm_aircraft_type='6';
        break;
        case 3:
            flarm_aircraft_type='B';
        break;
        case 4:
            flarm_aircraft_type='1';
        break;
      }

      float currentSpeed = payloadList[packetNumber].speed/KMPH_TO_MS;

      // if manufacturer is > 9 , we need to cope with this so use skytraxx 11 = 1

      //v16.1

   //   char packet_data[200]={};
  //    sprintf(packet_data,"$PFLAA,0,%f,%f,%f,2,%02d,%04X,%f,0,%f,%d,%s*",
     //                      relNorth,relEast,relVert,payloadList[packetNumber].manufacturer,payloadList[packetNumber].id,
      //                     payloadList[packetNumber].heading, currentSpeed, climbRate, flarm_aircraft_type);

      char manu[3];
      char id[5];

      sprintf(manu,"%02X",payloadList[packetNumber].manufacturer);
      sprintf(id,"%04X",payloadList[packetNumber].id);

      String movingpilotData = "$PFLAA,0," + String(relNorth) + "," + String(relEast) + "," + String(relVert) + ",2," +
              String(manu) + String(id) + "," + payloadList[packetNumber].heading + ",0,"  +
							 currentSpeed + "," + climbRate + ","+ flarm_aircraft_type +"*";
      //V15



DEBUG_SERIAL_UART_MAX("[%d] - Crash Marker 4\r\n", millis());


      tcp_tx_weblines_counter++;

      if (wiredDirectly=='n' )
      {
        if (navSofware == 'L' && groundStationMode =='n')
        {
          DEBUG_SERIAL_UART_S("[" + String(millis()) + "] - Uploading RF Pilot to LK > " + movingpilotData + "\r\n");
          writeDataToWifi( LK8000Client, movingpilotData);
        }
        if ((navSofware == 'X' || navSofware == 'A' || navSofware == 'K') && groundStationMode =='n')
        {
          DEBUG_SERIAL_UART_S("[" + String(millis()) + "] - Uploading RF Pilot to XCS > " + movingpilotData + "\r\n");
          sendUDPPacket ( movingpilotData, apFoundIP );
        }
      }
      else
      {
         if ( groundStationMode =='n')
         {
           writeDataToSerial(movingpilotData);
         }
      }

      // V15 remove - if (app.do_online_tracking)
      if (payloadList[packetNumber].online_tracking && ( webUpload || gsm_connected ) )
      {

     	char stringlat[13];
    	char stringlon[14];

    	dtostrf(payloadList[packetNumber].latitude,12, 6, stringlat);
    	dtostrf(payloadList[packetNumber].longitude,13, 6, stringlon);

        String sla=String (stringlat);
        String slo=String (stringlon);
        sla.trim();
        slo.trim();

        DEBUG_SERIAL_UART_MAX("[%d] - Crash Marker 2\r\n", millis());


        // if manufacturer is > 9 , we need to cope with this so use hex values for now - todo

   //     char url_data[100]={};
   //     sprintf(url_data,"$PFLAA,0,%f,%f,%f,2,%02d,%04X,%f,0,%f,%d,%s*",
    //                         relNorth,relEast,relVert,payloadList[packetNumber].manufacturer,payloadList[packetNumber].id,
     //                        payloadList[packetNumber].heading, currentSpeed, climbRate, flarm_aircraft_type);

        char manuMe[3];
        char idMe[5];

        sprintf(manuMe,"%02X",fmac.my_addr.manufacturer);
        sprintf(idMe,"%04X",fmac.my_addr.id);

        String urlUp = String ( GPGGAdataIn.getFixTime() ) + "," +
                                String ( manuMe ) + String ( idMe ) + "," +
                                String ( manu ) + String ( id ) + "," +
                                payloadList[packetNumber].aircraft_type + "," +
                                sla + "," + slo + "," +
                                "0" + "," + "0" + "," +
                                payloadList[packetNumber].heading + "," +
                                payloadList[packetNumber].speed  + "," +
                                payloadList[packetNumber].altitude + "," +
                                payloadList[packetNumber].rssi;


      /*  String urlUp = String ( GPGGAdataIn.getFixTime() ) + "," +
        	                  ( fmac.my_addr.manufacturer  * 100000 + fmac.my_addr.id ) + "," +
        		              ( payloadList[packetNumber].manufacturer * 100000 + payloadList[packetNumber].id) + "," +
        		              payloadList[packetNumber].aircraft_type + "," +
					          sla + "," + slo + "," +
							  "0" + "," + "0" + "," +
					          payloadList[packetNumber].heading + "," +
						      payloadList[packetNumber].speed  + "," +
							  payloadList[packetNumber].altitude + "," +
					          payloadList[packetNumber].rssi;
*/



        serial_rx_packets_counter++;
        DEBUG_SERIAL_UART_MAX("[%d] - Crash Marker 3\r\n", millis());

        if ( groundStationMode =='y' && groundstation_type=='g')
        {
            if (gsm_connected)
            {
                gsm.sendUDP(urlUp);
            }
            else
            {
                Serial.println("gms down, waiting for keep alive ( every 5 mins ) to reset");
            }
            // we shall only restart on the keep alive check.

           // if ( !gsm.sendUDP(urlUp))
           // {
            //    Serial.println("UDP Send Failed - GSM Down - Restarting");
             //   yield();
              //  myDelay(500);
               // ESP.restart();
           // }
        }
        else
        {
           upLoadtoUDP(urlUp, airWareIP, AIRWARE_UDP_PORT);
        }

        // only log to serial for internet ground stations, gsm will do automagically.
        if ( groundStationMode =='y' && groundstation_type=='i')
        {
          Serial.println(urlUp);
        }

      }
      //v16.1
      ///else
      //{
       //   Serial.println("Ground station mode selected however packet set to no online tracking, or webupload or gsm connected is down");
      //}

  //  payloadReady[packetNumber]=false;
    /*    Serial.println(millis());
        Serial.println("Sending packet :");
        Serial.println(payloadReady[packetNumber]);
        Serial.println(payloadList[packetNumber].id);
     */
      if (packet_repeat=='y')
      {
          payload_time_to_send[packetNumber]=millis()+PACKET_REPEAT_FREQ;

          // finished sending, clear ID down.
          if (payloadReady[packetNumber]==0)
          {
              payloadList[packetNumber].id=0;
          }
      }
      payloadReady[packetNumber]=payloadReady[packetNumber]-1;
  }
}


  DEBUG_SERIAL_UART_MAX("[%d] - Crash Marker 4b\r\n", millis());
  DEBUG_SERIAL_UART_MAX("[%d] - Starting Webloop\r\n", millis());
  //2.8
  if  (wifiUp)
  {
    Web_loop();
  }

  yield();

  #ifdef TEST_PILOT

  if ( millis() > addPilotTime)
  {

    pilotACurrentTrack=pilotACurrentTrack-pilotARadsDiff;
    pilotAX=pilotADistance*sin(pilotACurrentTrack);
    pilotAY=pilotADistance*cos(pilotACurrentTrack);

    addPilotTime=millis()+pilotInterval;
    pilotADistance=pilotADistance+5;

    if (pilotACurrentTrack<-6.2)
    {
      pilotACurrentTrack=0;
    }

    pilotAlt=pilotAlt+random(1,pilotAltDiff);

    String testPilotData = "$PFLAA,0," + String (-1*pilotAX) + "," + String (-1*pilotAY)+ "," + String (pilotAlt) + ",2," + String (pilotA) + "," + String (-1*(rtodA(pilotACurrentTrack+pilotARadsDiff) )) + ",0," + 25 + "," + 0 + ",7*";
    writeDataToSerial(testPilotData);
  }
  #endif

}
