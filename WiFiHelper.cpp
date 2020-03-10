
#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>

#include <FS.h>
//#include <TimeLib.h>

#include "WiFiHelper.h"

WiFiClient client;

extern "C" {
#include <user_interface.h>
}


void WiFi_setup()
{

  WiFi.mode(WIFI_OFF);
  yieldDelay(500);

 // WiFi.begin();

  set_Connected_Time=false;
  //DEBUG_SERIAL_UART("[%d] - Your myChipID is :- %d\r\n", millis(),myChipID);
  //v16.1

  //String uniqueSSID="";
  char CC[20]={'\0'} ;

  if (airWhereID!=0)
  {
    //  uniqueSSID="AirWhere-"+ String(airWhereID);
      sprintf(CC,"AirWhere-%d",airWhereID);
  }
  else
  {
     // uniqueSSID="AirWhere-"+ String(FANET_MANUFACTURER) + String(awHexID);
      sprintf(CC,"AirWhere-%s%s",awHexManu,awHexID);
  }

  //String uniqueSSID="AirWhere-"+ String(airWhereID);#


 // uniqueSSID.toCharArray(CC,15);

  const char* sid = CC;

  //v16.1
 // if (airWareSsid!="")

  //snprintf("AirWhere-%02d%05d")

  if (airWareSsid[0]!='\0')
  {

   //2.8 - add reconnect if we are on hard wired as we dont need the wifi for comms. 
    if (wiredDirectly == 'y' || wiredDirectly == 'o' || wiredDirectly == 'v' )
    {
      wifi_station_set_reconnect_policy(true);
      wifi_station_set_auto_connect(true);
    }
    else
    {
      wifi_station_set_reconnect_policy(false);
      wifi_station_set_auto_connect(false);
    }

    WiFi.status();
    WiFi.mode(WIFI_AP_STA);
    wifi_set_opmode(STATIONAP_MODE);
    DEBUG_SERIAL_UART("[%d] - Connecting to :- %s\r\n", millis(),airWareSsid);

    struct station_config stationConf; 
    memset(stationConf.ssid, 0, 32);
    memset(stationConf.password, 0, 64);

    stationConf.bssid_set = 0; 
    memcpy(&stationConf.ssid, airWareSsid, 32); 
    memcpy(&stationConf.password, airWarePassword, 64); 
   
    wifi_station_set_config(&stationConf); 
    wifi_station_connect();

    int timeout=0;
    
    while (WiFi.status() != WL_CONNECTED) {
      yieldDelay(500);
      timeout++;
      DEBUG_SERIAL_UART("[%d] - Wifi connect status : %d\r\n", millis(),WiFi.status());
      if (timeout==20)
      {
        WiFi.mode(WIFI_AP);
        DEBUG_SERIAL_UART("[%d] - set mode to WIFI_AP \r\n", millis());
        break;
             
      }
    } 
    
  }
  else
  {  
     WiFi.mode(WIFI_AP);
  }
  

  
  if (WiFi.status() == WL_CONNECTED)
  {
    webUpload=true;
    DEBUG_SERIAL_UART("[%d] - WiFi connected - uploading to Web\r\n", millis());
  }
  else
  {
    webUpload=false;
    DEBUG_SERIAL_UART("[%d] - Can no connect to router, disabling Web upload\r\n", millis());
  }

 // wifi_set_phy_mode(PHY_MODE_11B);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
//  char dave[9];
 // char password[] = "12345678";

  //password=&dave;

  Serial.println(sid);
  WiFi.softAP(sid,airWhere_ap_password);

  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(DNS_PORT, "www.msftncsi.com", apIP);

  struct dhcps_lease dhcp_lease;
  IP4_ADDR(&dhcp_lease.start_ip, 192, 168, 4, 100);
  IP4_ADDR(&dhcp_lease.end_ip, 192, 168, 4, 101);
  wifi_softap_set_dhcps_lease(&dhcp_lease);
   
    
    yieldDelay(100);

}



