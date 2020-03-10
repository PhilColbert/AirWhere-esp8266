#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

extern "C" {
#include <user_interface.h>
}

#include "WebHelper.h"

ESP8266WebServer server ( 80 );
int timeOnWeb = 15000;



void close_socket()
{
    if (wiredDirectly == 's' || wiredDirectly == 'n' )
    {
        LK8000Client.flush();
        LK8000Client.stop();
        socket_connected = false;
    }

}

void displayWeb() {

  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (displayWeb)\r\n", millis());

 // Serial.println("displaying web");
  //char Root_temp[2048];
//  flarmSerial.end();

  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  char Root_temp[1900];
  char webConnected[49] =    "<font color=#00aeef>Not Connected</font>";
  char configWire[30] = "Wifi mode On";

  // if we are in ground station mode and gsm - check whether the sim800l is online.

  if ( groundStationMode == 'y' && groundstation_type=='g' && gsm_connected)
  {
      strcpy ( webConnected, "<font color=#00aeef>GSM Connected</font>");
  }
  else
  {
      if (webUpload)
      {
        strcpy ( webConnected, "<font color=#00aeef>Connected</font>");
      }
  }
        
  if (groundStationMode == 'n')
  {
    webTime = millis() + timeOnWeb;

    char navSofwareWeb[16] = "";
    char webConnected[49] =    "<font color=#00aeef>Not Connected</font>";
    char socketConnected[60] = "<font color=#00aeef>Flight Software Not Connected</font>";
    char checkVersion[73] = "Please connect your AirWhere to the Internet to check for a new version";
    char flarmEnabled[23] = "Nrf905 is not attached";
    char flarmSubmit[47] = "y'><input type=submit value='Connect nrf905";
    
    if (webUpload)
    {
      strcpy ( webConnected, "<font color=#00aeef>Connected</font>");
      strcpy ( checkVersion, "<input type=submit class=\"bu\" value='Check for new Version'>");
    }

    if ( socket_connected )
    {
      strcpy ( socketConnected, "<font color=#00aeef>Flight Software Connected</font>");
    }

    if ( wiredDirectly=='y')
    {
      strcpy ( configWire, "Hardwired Mode");
    }
    if ( wiredDirectly=='n')
    {
      strcpy ( configWire, "Wifi Mode");
    }
    if ( wiredDirectly=='s')
    {
      strcpy ( configWire, "StandAlone Wifi Mode");
    }
    if ( wiredDirectly=='o')
    {
      strcpy ( configWire, "AirWhere OTG Mode");
    }        
    if ( wiredDirectly=='v')
    {
      strcpy ( configWire, "Vertica V2 Mode");
    }
    if ( wiredDirectly=='f')
    {
      strcpy ( configWire, "FlySkyHy Mode");
    }

    //v16
    
    if ( navSofware == 'L')
    {
      strcpy ( navSofwareWeb, "LK8000 - Kobo");
    }
    if ( navSofware == 'X')
    {
      strcpy ( navSofwareWeb, "XCSoar - Kobo");
    }
    if ( navSofware == 'A')
    {
      strcpy ( navSofwareWeb, "XCSoar - Android");
    }
    if ( navSofware == 'K')
    {
      strcpy ( navSofwareWeb, "LK8000 - Android");
    }

    if ( receiveFlarm=='y')
    {
       strcpy ( flarmEnabled, "Nrf905 is attached" );
       strcpy ( flarmSubmit, "n'><input type=submit value='Disconnect Nrf905"); 
    }
    //v16
    close_socket();

    snprintf ( Root_temp, 1900,
               "<html><head>\
<style>.bu {background-color: #d7d7d7;border: none;color: black;padding: 8px 32px;\
text-decoration: none;font-size: 18px;margin: 4px 2px;}\
body{font: normal 12px Verdana, Arial, sans-serif;background-color:#e6e7e8}</style>\
<table border=0 cellpadding=6><tr><td><h1>AirWhere ID - %s</h1></td></tr>\
<tr><td>Wifi Router Name (SSID)</td><td><b>%s</b></td></tr>\
<tr><td>Wifi Router Password( SSID )</td><td><b>%s</b></td></tr>\
<tr><td>AirWhere Access Point Password</td><td><b>%s</b></td></tr>\
<tr><td><br><font color=#00aeef>Internet Status</font></td><td><br>%s</td></tr>\
<tr><td><br><br>Flight Software Configured</td><td><br><br><b>%s</b></td></tr>\
<tr><td>Connection Mode</td><td><b>%s</b></td></tr>\
<tr><td><br><font color=#00aeef>Status</td><td><br>%s</td></tr>\
<tr><td><form method=post action=\"config\"><input type=hidden name=work1 value=please1><input type=submit class=\"bu\" value='Configure Pilot Info'></form></td>\
<td><form method=post action=\"configh\"><input type=hidden name=work2 value=please2><input type=submit class=\"bu\" value='Configure Hardware'></form></td></tr>\
<tr><td><font color=#00aeef>AirWhere Version</td><td><font color=#00aeef>%d</td></tr>\
<tr><td><form method=post action=\"version\"><input type=hidden name=work value=please>%s</form></td></tr>\
<tr><td>Uptime</td><td>%02d:%02d:%02d</td></tr>\
<tr><td><form method=post action=\"stats\"><input type=hidden name=stats1 value=stats1><input type=submit class=\"bu\" value='Statistics'></form></td>\
<tr><td><form method=post action=\"switch\"><input type=hidden name=aWssid value=%s><input type=hidden name=aWpass value=%s>\
<tr><td><input type=submit class=\"bu\" value='Switch to Ground Station Mode'></form></td></tr></table></body></body></html>",
awHexID , airWareSsid, airWarePassword,airWhere_ap_password, webConnected, navSofwareWeb, configWire,
			   socketConnected, AwVersion, checkVersion,
			   hr, min % 60, sec % 60, airWareSsid, airWarePassword);
  }
  else
  {
      if ( groundstation_type=='g')
      {
           snprintf ( Root_temp, 1900,
               "<h1 align=left>AirWhere GSM ( SIM800L )  Ground Station - %s</h1>\
<form method=post action=\"updateGSgsm\">\
<br><Br>Current Latitude, Longitude, Altitude of your Ground Station ( DD.DDDDDD, DDD.DDDDDD, M )<br><br>\
For example 51.991646,02.223554,-001.757813,132.223344, Altitude 250 ( integer ) <br><br>\
Please keep the formatting to (-)DD.DDDDDD (-)DDD.DDDDDD<br><br><br><br>\
<b>Latitude =</b> %s <input type=number step=\"0.000001\" name=gsLatitudeSwitch value=%s><br><br>\
<b>Longitude</b> = %s <input type=number step=\"0.000001\" name=gsLongitudeSwitch value=%s><br><br>\
<b>Altitude AGL (metres)</b> = %s <input type=number name=gsAltitudeSwitch value=%s><br><br><br><Br>\
Enter your Ground Station ID,Access Point Name, User name and Password for AirWhere to upload Pilots<br>\
You will find these values on the mobile phone providers (gsm) website - make sure you check if its for monthly or pay as you go\
<br><br>For example :-<br>\
O2 UK Pay as you go - payandgo.o2.co.uk, O2 Mobile Web, password<br>\
EE Pay as you go - everywhere, <leave blank>, <leave blank> ( everywhere,,,)<br>\
<br><br>Please register the Id on the website to allocate your unique ID<br><br>\
GroundStation ID - %s <input type=text class=bu required=required pattern=\"[0-9A-F]{4}\" name=aWiD value=%s><br><br>\
GSM Access Point Name - %s <input type=text name=gsmAPNweb value=\"%s\"><br><br>\
GSM User Name - %s <input type=text name=gsmUNweb value=\"%s\"><br><br>\
GSM Password - %s <input type=text name=gsmPWweb value=\"%s\"><br><br>\
%s<br><hr><br>\
<input type=submit value=Submit></form><hr>\
Uptime - %02d:%02d:%02d<br>\
AirWhere Packets Received - %ld<br><br>\
<form method=post action=\"switchIGSMode\">\
<input type=submit value='Switch to Internet Ground Station Mode'></form><br>\
<br><hr><br>\
<form method=post action=\"switchAW\">\
<input type=hidden name=aWssid value=%s><input type=hidden name=aWpass value=%s>\
<input type=hidden name=gsLatitude value=%s><input type=hidden name=gsLongitude value=%s>\
<input type=submit value='Switch to AirWhere Mode'></form>",
awHexID , gsLatitude, gsLatitude, gsLongitude, gsLongitude, gsAltitude, gsAltitude,
awHexID ,awHexID ,gsm_access_point_name,gsm_access_point_name,gsm_username,gsm_username,
               gsm_password,gsm_password,webConnected, hr, min % 60, sec % 60, serial_rx_packets_counter,
               airWareSsid, airWarePassword, gsLatitude, gsLongitude);
      }
      else
      {
          snprintf ( Root_temp, 1900,
              "<h1 align=left>AirWhere Internet Ground Station - %s</h1>\
<form method=post action=\"updateGS\">\
<br><Br>Current Latitude, Longitude, Altitude of your Ground Station ( DD.DDDDDD, DDD.DDDDDD, M )<br><br>\
For example 51.991646,02.223554,-001.757813,132.223344, Altitude 250 ( integer ) <br><br>\
Please keep the formatting to (-)DD.DDDDDD (-)DDD.DDDDDD<br><br><br><br>\
<b>Latitude =</b> %s <input type=number step=\"0.000001\" name=gsLatitudeSwitch value=%s><br><br>\
<b>Longitude</b> = %s <input type=number step=\"0.000001\" name=gsLongitudeSwitch value=%s><br><br>\
<b>Altitude AGL (metres)</b> = %s <input type=number name=gsAltitudeSwitch value=%s><br><br><br><Br>\
Enter your Ground Station ID,WiFi Routers SSID and Password for AirWhere to upload Pilots<br>\
Please register the Id on the website to allocate your unique ID<br><br>\
GroundStation ID - %s <input type=text class=bu required=required pattern=\"[0-9A-F]{4}\" name=aWiD value=%s><br><br>\
SSID - %s <input type=text name=aWssidSwitch value=%s><br><br>\
Password - %s <input type=text name=aWpassSwitch value=%s><br><br>\
%s<br><hr><br>\
<input type=submit value=Submit></form><hr>\
Uptime - %02d:%02d:%02d<br>\
AirWhere Packets Received - %ld<br>\
<form method=post action=\"switchGSMGS\">\
<input type=submit value='Switch to GSM Ground Station Mode'></form>\
<br><form method=post action=\"switchAW\">\
<br><hr><br><input type=hidden name=aWssid value=%s><input type=hidden name=aWpass value=%s>\
<input type=hidden name=gsLatitude value=%s><input type=hidden name=gsLongitude value=%s>\
<input type=submit value='Switch to AirWhere Mode'></form>",
awHexID , gsLatitude, gsLatitude, gsLongitude, gsLongitude, gsAltitude, gsAltitude,
awHexID ,awHexID ,airWareSsid, airWareSsid, airWarePassword, airWarePassword, webConnected, hr, min % 60, sec % 60, serial_rx_packets_counter,
              airWareSsid, airWarePassword, gsLatitude, gsLongitude);
      }

  }

  //const char dave[1199]="12345000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002442111111111111111111111000000006";

  Serial.println(ESP.getFreeHeap());

  server.send ( 200, "text/html", Root_temp);

 // flarmSerial.begin(112500);
}

void configureAW() {
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (configureAW)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  char Root_temp[1700];

  char type_paraglider[9]="";
  char type_hangglider[9]="";
  char type_balloon[9]="";
  char type_glider[9]="";

  switch (web_aircraft_type)
  {
    case '1':
    strcpy ( type_paraglider, "selected");
    break;
    case '2':
    strcpy ( type_hangglider, "selected");
    break;
    case '3':
    strcpy ( type_balloon, "selected");
    break;
    case '4':
    strcpy ( type_glider, "selected");
    break;
  }

  //v16.1
  char disabled[18]={};

  if (lora_or_flarm=='f')
  {
      strcpy ( disabled,"disabled=disabled");
  }

  //v17

  char packetselon[9]="";
  char packetseloff[9]="";;

  if ( packet_repeat == 'y')
  {
    strcpy ( packetselon, "selected");
  }
  else
  {
    strcpy ( packetseloff, "selected");
  }

  DEBUG_SERIAL_UART("[%d] - Point 1 (configureAW)\r\n", millis());
  snprintf ( Root_temp, 1700,"<style>.bu {background-color: #d7d7d7;border: none;color: black;padding: 8px 32px;\
text-decoration: none;font-size: 18px;margin: 4px 2px;}\
body{font: normal 12px Verdana, Arial, sans-serif;background-color:#e6e7e8}</style>\
<table border=0 cellpadding=6><tr><td><h1 align=center>Configure AirWhere %s</h1>\
<form method=post action=\"update\">\
<tr><td>Manufacturer ID</td><td>%s</td></tr>\
<tr><td>AirWhere ID</td><td><input type=text %s class=bu required=required pattern=\"[0-9A-F]{4}\" name=aWiD value=%s></td></tr>\
<tr><td colspan=2>( IMPORTANT - ( 4 digits 0000-FFFF ) ie 4d2d or 0203 - Please register your unit on the website FIRST! )</td></tr>\
<tr><td>Pilot Name</td><td><input class=bu type=text name=awPilotName value=\"%s\"></td></tr>\
<tr><td>Aircraft Type</td><td><select class=bu name=wat><option value=1 %s>Paraglider</option><option value=2 %s>Hangglider</option><option value=3 %s>Balloon</option><option value=4 %s>Glider</option></select>\
<tr><td>Wifi Router Name (SSID)</td><td><input class=bu type=text name=aWssid value=\"%s\"></td></tr>\
<tr><td>Wifi Router Password( SSID )</td><td><input class=bu type=text name=aWpass value=\"%s\"></td></tr>\
<tr><td>AirWhere Access Point Password</td><td><input class=bu type=text name=aWappass value=%s> (  8 characters or more )</td></tr>\
<tr><td>Packet Repeat ( keep xcsoar pilots on screen for longer )</td><td><select class=bu name=selectpkt><option value=yes %s>yes</option><option value=no %s>no</option></select></td></tr>\
<tr><td><input type=submit class=bu value=Submit></form></td></tr>\
<tr><td><a href=/>Return to AirWhere Home</a></td><tr></a>\</table>",
awHexID , awHexManu,disabled, awHexID , awPilotName, type_paraglider, type_hangglider, type_balloon, type_glider, airWareSsid,
                    airWarePassword,airWhere_ap_password,packetselon,packetseloff);
  DEBUG_SERIAL_UART("[%d] - Point 2 (configureAW)\r\n", millis());
  server.send ( 200, "text/html", Root_temp );
//Serial.println(navSofwareWeb);

  DEBUG_SERIAL_UART("[%d] - Sent Page Back (configureAW)\r\n", millis());

}


void configureAWHardware() {
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (configureAW)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  char Root_temp[1950];

  //v16
    char navSofwareWeb[17] = "";
    char softwareL[9] = "";
    char softwareX[9] = "";
    char softwareA[9] = "";
    char softwareK[9] = "";

    if ( navSofware == 'L')
    {
      strcpy ( navSofwareWeb, "LK8000 - Kobo");
      strcpy ( softwareL, "selected");
    }
    if ( navSofware == 'X')
    {
      strcpy ( navSofwareWeb, "XCSoar - Kobo");
      strcpy ( softwareX, "selected");
    }
    if ( navSofware == 'A')
    {
      strcpy ( navSofwareWeb, "XCSoar - Android");
      strcpy ( softwareA, "selected");
    }
    if ( navSofware == 'K')
    {
      strcpy ( navSofwareWeb, "XCSoar - Android");
      strcpy ( softwareK, "selected");
    }

  char hw[20] = "";
  char hwNo[9] = "";
  char hwV[9] = "";
  char hwYes[9] = "";
  char hwSW[9] = "";
  char hwOTG[9] = "";
  char hwF[9] = "";

  if ( wiredDirectly == 'y')
  {
    strcpy ( hw, "Hard Wired Mode");
    strcpy ( hwYes, "selected");
  }
  if ( wiredDirectly == 'n')
  {
    strcpy ( hw, "Wifi Mode");
    strcpy ( hwNo, "selected");
  }
  if ( wiredDirectly == 's')
  {
    strcpy ( hw, "StandAlone Wifi Mode");
    strcpy ( hwSW, "selected");
  }
  if ( wiredDirectly == 'o')
  {
    strcpy ( hw, "AirWhere OTG Mode");
    strcpy ( hwOTG, "selected");
  }
  if ( wiredDirectly == 'v')
  {
    strcpy ( hw, "Vertica v2 Mode");
    strcpy ( hwV, "selected");
  }
  if ( wiredDirectly == 'f')
  {
    strcpy ( hw, "FlySkyHy Mode");
    strcpy ( hwF, "selected");
  }

  char gpsBaud[7]="";
  char gps9600[9]="";
  char gps19200[9]="";
  char gps38400[9]="";
  char gps57600[9]="";
  char gps115200[9]="";


  switch (gpsBaudRate)
  {
    case 9600:
     strcpy ( gpsBaud, "9600");
     strcpy ( gps9600, "selected");
     break;
    case 19200:
     strcpy ( gpsBaud, "19200");
     strcpy ( gps19200, "selected");
     break;
    case 38400:
     strcpy ( gpsBaud, "38400");
     strcpy ( gps38400, "selected");
     break;
    case 57600:
     strcpy ( gpsBaud, "57600");
     strcpy ( gps57600, "selected");
     break;
    case 115200:
     strcpy ( gpsBaud, "115200");
     strcpy ( gps115200, "selected");
     break;
  }

  char wifioff[9]="";
  char wifion[9]="";

  if ( wifioffSelected == 'n')
  {
    strcpy ( wifion, "selected");
  }
  else
  {
    strcpy ( wifioff, "selected");
  }

  char nsw[10]="";
  strcpy ( nsw, navSofwareWeb);

  char eightsel[9]="";
  char ninesel[9]="";;

  if ( loraFrequency == '8')
  {
    strcpy ( eightsel, "selected");
  }
  else
  {
    strcpy ( ninesel, "selected");
  }
//v16
  char lorasel[9]="";
  char flarmsel[9]="";
  char flarmupdate[110]="";

  if ( lora_or_flarm == 'l')
  {
    strcpy ( lorasel, "selected");
  }
  else
  {
    strcpy ( flarmsel, "selected");
    if (webUpload)
    {
      snprintf ( flarmupdate,110, "<tr><td>Flarm Expires : <b>%s</b></td><td><a href=updateflarm>Update Flarm board</a></td></tr>", flarm_board_expiration);
    }
    else
    {
        snprintf ( flarmupdate,110, "<tr><td>Flarm Expires : <b>%s</b></td><td>Connect to Internet to Update Flarm</td></tr>", flarm_board_expiration);
    }
  }



  DEBUG_SERIAL_UART("[%d] - Point 1 (configureAW)\r\n", millis());
  snprintf ( Root_temp, 1950,"<style>.bu {background-color: #d7d7d7;border: none;color: black;padding: 8px 32px;\
text-decoration: none;font-size: 18px;margin: 4px 2px;}\
body{font: normal 12px Verdana, Arial, sans-serif;background-color:#e6e7e8}</style>\
<table border=0 cellpadding=6><tr><td colspan=2><h1 align=center>Configure AirWhere Hardware %s</h1>\
<form method=post action=\"updateh\">\
<tr><td>Radio Frequency (Europe - 868Mhz, US - 915Mhz)</td><td><select class=bu name=Lfreq><option value=8 %s>868Mhz</option><option value=9 %s>915mhz</option></select></td></tr>\
<tr><td>Radio Board</td><td><select class=bu name=rfboard><option value=l %s>RF Lora Board</option><option value=f %s>Flarm Board</option></select></td></tr>%s\
<tr><td>Flight Software</td><td><select class=bu name=navsw><option value=L %s>LK8000 - Kobo</option><option value=X %s>XCSoar - Kobo</option>\
<option value=K %s>LK8000 - Android</option><option value=A %s>XCSoar - Android</option></select></td></tr>\
<tr><td>Connection Mode</td><td><select class=bu name=hwMode><option value=y %s>Hard Wired Mode</option><option value=n %s>Wifi Mode</option>\
<option value=s %s>Wifi Standalone Mode</option><option value=o %s>AirWhere OTG Mode</option><option value=v %s>Vertica V2 Mode</option><option value=f %s>FlySkyHy Mode</option></select></td></tr>\
<tr><td>GPS Baud Rate ( hard wired only )</td><td><select class=bu name=gpsBaud><option value=9 %s>9600 bps</option><option value=2 %s>19200 bps</option><option value=3 %s>38400 bps</option>\
<option value=5 %s>57600 bps</option><option value=1 %s>115200 bps</option></select></td></tr>\
<tr><td>Wifi Off(wifi off after 3 mins to save power<br>Leave no if you dont know what this means)</td><td><select class=bu name=selectwifioff><option value=yes %s>yes</option><option value=no %s>no</option></select></td></tr>\
<tr><td><input type=submit class=bu value=Submit></form></td></tr>\
<tr><td><a href=/>Return to AirWhere Home</a></td><tr></a>\</table>",
awHexID ,eightsel,ninesel,lorasel, flarmsel,flarmupdate, softwareL, softwareX,softwareK,softwareA, hwYes,hwNo,hwSW, hwOTG,hwV,hwF, gps9600,gps19200, gps38400,gps57600,gps115200,wifioff,wifion);

  DEBUG_SERIAL_UART("[%d] - Point 2 (configureAW)\r\n", millis());
  server.send ( 200, "text/html", Root_temp );
//Serial.println(navSofwareWeb);

  DEBUG_SERIAL_UART("[%d] - Sent Page Back (configureAW)\r\n", millis());

}


void aWversion() {
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (aWversion)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();



#define USE_SERIAL Serial

    HTTPClient http;
    String payload;
        //  USE_SERIAL.print("[HTTP] begin...\n");
          // configure traged server and url
          //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
          http.begin("http://www.airwhere.co.uk/downloads/version.txt"); //HTTP

       //   USE_SERIAL.print("[HTTP] GET...\n");
          // start connection and send HTTP header
          int httpCode = http.GET();

          // httpCode will be negative on error
          if(httpCode > 0) {
              // HTTP header has been send and Server response header has been handled
           //   USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

              // file found at server
              if(httpCode == HTTP_CODE_OK) {
                   payload = http.getString();
              //    USE_SERIAL.println(payload);
              }
          } else {
              USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
          }

          http.end();


//  WiFiClient clientAWV;
  //const int httpPort = 80;
 // char Root_temp[1000];

/*  if (!clientAWV.connect(airWareServer, 80))
  {
    server.send ( 200, "text/html", "Failed to access www.air-ware.co.uk, try later" );
    Serial.print("WIFICLIENT code ");
    Serial.println(clientAWV.connect(airWareServer, 80));
    return;
  }
  else
  {


    clientAWV.write("GET /downloads/version.txt HTTP/1.1\r\nHost: www.air-ware.co.uk\r\nConnection: close\r\n\r\n", 84);
    clientAWV.write("\n", 1);
  }

  String line;
  while (clientAWV.available())
  {
    line = clientAWV.readStringUntil('\r');
  }

  clientAWV.stop();
*/
  String v = payload.substring(8);
  int vI = v.toInt();
  v.trim();

  if (vI == AwVersion)
  {
    server.send ( 200, "text/html", "<html><head>\
<style>.bu {background-color: #d7d7d7;border: none;color: black;padding: 8px 32px;\
text-decoration: none;font-size: 18px;margin: 4px 2px;}\
body{font: normal 12px Verdana, Arial, sans-serif;background-color:#e6e7e8}</style>\
<h1>AirWhere is on latest version</h1><br><Br><a href=/>\
Return to AirWhere Home</a>\
<br><Br><br><br><form method=post action=\"devVersion\"><input type=hidden name=work1 value=please1>\
<input type=submit class=bu value='Install Development AirWhere'></form>" );
  }
  else
  {
   char Root_temp[750];

    snprintf ( Root_temp, 750,

               "<html><head>\
<style>.bu {background-color: #d7d7d7;border: none;color: black;padding: 8px 32px;\
text-decoration: none;font-size: 18px;margin: 4px 2px;}\
body{font: normal 12px Verdana, Arial, sans-serif;background-color:#e6e7e8}</style>\
<h1>AirWhere %s - Please Update</h1><br>\
<form method=post action=\"updateVersion\">\
<table><tr><td>Current Version</td><td>%d</td></tr>\
<tr><td>New Version</td><td>%d</td></tr>\
<tr><td><br><br><input class=bu type=submit value=\"Update\"></form></td></tr>\
<tr><td><a href=/><br><br>AirWhere home page</a></td></tr>\
<tr><td><br><Br><form method=post action=\"devVersion\"><input type=hidden name=work1 value=please1>\
<input class=bu type=submit value='Install Development AirWhere'></form></td></tr></table>", awHexID , AwVersion, vI);

    server.send ( 200, "text/html", Root_temp );
  }

}

/*void updateAW_entered_directly()
{
  DEBUG_SERIAL_UART("[%d] - updateAW_entered_directly (updateAW)\r\n", millis());
  webTime = millis() + timeOnWeb;
  server.send ( 200, "text/html", "<h1>Entered Update page directly - Error</h1><a href=/>Please return to AirWhere home page</a></h2>" );
  yieldDelay(100);
}
*/
void switchGS()
{
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (switchGS)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  String UpSsid, UpPass, UpNavSW = "L", GS = "y";

  for ( uint8_t i = 0; i < server.args(); i++ )
  {
    if ( server.argName ( i ) == "aWssid")
    {
      UpSsid = server.arg ( i );
    }
    if ( server.argName ( i ) == "aWpass")
    {
      UpPass = server.arg ( i );
    }
    if ( server.argName ( i ) == "navsw")
    {
      UpNavSW = server.arg ( i );
    }
  }
  DEBUG_SERIAL_UART("[%d] -( configureGS - y )\r\n", millis());
  update_configFile(UpSsid, UpPass, UpNavSW, GS);

  server.send ( 200, "text/html", "<h1>AirWhere Rebooting into Ground Station Mode - please reconnect to the Access Point again<br><br></h1><br><br><a href=/>Reconnect to AirWhere home page</a></h2>" );

  DEBUG_SERIAL_UART("[%d] -!!!!! Restarting AirWhere !!!!!\r\n", millis());
  ESP.restart();
}

/*void flarmConf()
{

  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (flarmConf)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  String flarmC;

  for ( uint8_t i = 0; i < server.args(); i++ )
  {
    if ( server.argName ( i ) == "flarmC")
    {
      flarmC = server.arg ( i );
    }
  }
  DEBUG_SERIAL_UART("[%d] -( flarmConf - n )\r\n", millis());
  
  update_configFile(flarmC);

  server.send ( 200, "text/html", "<h1>AirWhere Changing Nrf905 Setting - please reconnect to the Access Point again<br><br></h1><a href=/>Reconnect to AirWhere home page</a></h2>" );


  DEBUG_SERIAL_UART("[%d] -!!!!! Restarting AirWhere !!!!!\r\n", millis());
  ESP.restart();
  
}
*/
void aw_stats()
{
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (stats)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  char Root_temp[1000];
  char reset_reason[30]="";
  ESP.getResetReason().toCharArray(reset_reason,ESP.getResetReason().length());

  snprintf ( Root_temp, 1000, "<html><head>\
<style>.bu {background-color: #d7d7d7;border: none;color: black;padding: 8px 32px;\
text-decoration: none;font-size: 18px;margin: 4px 2px;}\
body{font: normal 12px Verdana, Arial, sans-serif;background-color:#e6e7e8}</style>\
<h1>AirWhere %s - Statistics</h1><br>\
<table cellspacing=10><tr><td>Current Version</td><td>%d</td></tr>\
<tr><td>Uptime</td><td>%02d:%02d:%02d</td></tr>\
<tr><td>NMEA lines received</td><td>%ld</td></tr>\
<tr><td>AirWhere packets received</td><td>%ld</td></tr>\
<tr><td>NMEA Lines transmitted</td><td>%ld</td></tr>\
<tr><td>AirWhere packets transmitted</td><td>%ld</td></tr>\
<tr><td>CPU Speed</td><td>%d</td></tr>\
<tr><td>Free Heap</td><td>%d</td></tr>\
<tr><td>Reset Reason</td><td>%s</td></tr>\
<tr><td>Longest Loop time</td><td>%d</td></tr>\
<tr><td><a href=/>Return to AirWhere Home</a></td><tr></a></td></tr></table>",awHexID , AwVersion,hr, min % 60, sec % 60,tcp_rx_weblines_counter, serial_rx_packets_counter,
  tcp_tx_weblines_counter, serial_tx_packets_counter, (int)ESP.getCpuFreqMHz(),(int)ESP.getFreeHeap(),reset_reason,  longest_loop_time);



  server.send ( 200, "text/html", Root_temp );

}

//v16
void updateflarm()
{
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (stats)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  char Root_temp[1000];

  snprintf ( Root_temp, 1000, "<html><head>\
<style>.bu {background-color: #d7d7d7;border: none;color: black;padding: 8px 32px;\
text-decoration: none;font-size: 18px;margin: 4px 2px;}\
body{font: normal 12px Verdana, Arial, sans-serif;background-color:#e6e7e8}</style>\
<h1>AirWhere %s - Update Flarm Board</h1><br>\
<form method=post action=\"processfupdate\">\
<table cellspacing=10><tr><td>AirWhere Current Version</td><td>%d</td></tr>\
<tr><td>Uptime</td><td>%02d:%02d:%02d</td></tr>\
<tr><td>Flarm board expiration date</td><td>%s</td></tr>\
<tr><td><input type=submit class=bu value=Process Update></form></td><td></td></tr>\
<tr><td><a href=/>Return to AirWhere Home</a></td><tr></a></td></tr></table>",awHexID , AwVersion,hr, min % 60, sec % 60, flarm_board_expiration);

  server.send ( 200, "text/html", Root_temp );

}

void processfupdate()
{
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (updateVersion)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  yieldDelay(500);
  yield();

  server.send ( 200, "text/html", "<!DOCTYPE html><html><style>#myProgress{position: relative;width: 100%;height: 30px;background-color: #ddd;}\
#myBar {position: absolute;width: 1%;height: 100%;background-color: #4CAF50;}\
.bu {background-color: #d7d7d7;border: none;color: black;padding: 8px 32px;\
text-decoration: none;font-size: 18px;margin: 4px 2px;}\
body{font: normal 12px Verdana, Arial, sans-serif;background-color:#e6e7e8}</style><body onload=\"move()\" width=100%>\
<script>var width = 1;function move() {var elem = document.getElementById(\"myBar\");\
var id = setInterval(frame, 700);function frame(){width++;if (width >= 100){clearInterval(id);\
document.getElementById(\"p1\").innerHTML = \"<br><br><a href=/>AirWhere Home</a>\";}\
elem.style.width = width + '%';}}</script><div>Processing Flarm Update - please wait - if \
the update is successful the board will automatically reboot, please then reconnect , please check for new expiration date.<div id=myProgress><div id=myBar></div>\
<div align=center><p id=\"p1\"><br><br>>>>      Processing Flarm Update     >>></p></div></body></html>" );

  int ret= xmodem_transmit();

  if( ret==0)
  {

    Serial.println("Flarm Update completed - rebooting.");
  }
  else
  {
    server.send ( 200, "text/html", "Failed to access www.airwhere.co.uk, try later" );
    Serial.println("Flarm Update failed - please try again.");
  }


  yieldDelay(500);

  ESP.restart();


}


void switchAW()
{
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (switchAW)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  String UpSsid, UpPass, UpNavSW = "L", GS = "n";

  for ( uint8_t i = 0; i < server.args(); i++ )
  {
    if ( server.argName ( i ) == "aWssid")
    {
      UpSsid = server.arg ( i );
    }
    if ( server.argName ( i ) == "aWpass")
    {
      UpPass = server.arg ( i );
    }
    if ( server.argName ( i ) == "navsw")
    {
      UpNavSW = server.arg ( i );
    }
  }
  DEBUG_SERIAL_UART("[%d] -( configureGS - n )\r\n", millis());
  update_configFile(UpSsid, UpPass, UpNavSW, GS);

  server.send ( 200, "text/html", "<h1>AirWhere Rebooting into AirWhere Mode - please reconnect to the Access Point again<br><br></h1><a href=/>Reconnect to AirWhere home page</a></h2>" );


  DEBUG_SERIAL_UART("[%d] -!!!!! Restarting AirWhere !!!!!\r\n", millis());
  ESP.restart();
}

void switchIGSMode()
{
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (switchAW)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  DEBUG_SERIAL_UART("[%d] -( configureGS - n )\r\n", millis());
  update_configFile('g','i');

  server.send ( 200, "text/html", "<h1>AirWhere Rebooting into Internet Ground Station Mode - please reconnect to the Access Point again<br><br></h1><a href=/>Reconnect to AirWhere home page</a></h2>" );
  delay(1000);

  DEBUG_SERIAL_UART("[%d] -!!!!! Restarting AirWhere !!!!!\r\n", millis());
  ESP.restart();
}

void switchGSMGS()
{
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (switchAW)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  DEBUG_SERIAL_UART("[%d] -( configureGS - n )\r\n", millis());
  update_configFile('g','g');

  server.send ( 200, "text/html", "<h1>AirWhere Rebooting into GSM Ground Station Mode - please reconnect to the Access Point again<br><br></h1><a href=/>Reconnect to AirWhere home page</a></h2>" );
  delay(1000);

  DEBUG_SERIAL_UART("[%d] -!!!!! Restarting AirWhere !!!!!\r\n", millis());
  ESP.restart();
}


void updateAW()
{
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (updateAW)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  String UpSsid, UpPass, GS = "n", AwIDin, asPilotNameIn, aWappassIn,aircraft_type_in,packetsel;

  for ( uint8_t i = 0; i < server.args(); i++ )
  {
    if ( server.argName ( i ) == "aWssid")
    {
      UpSsid = server.arg ( i );
    }
    if ( server.argName ( i ) == "aWpass")
    {
      UpPass = server.arg ( i );
    }

    if ( server.argName ( i ) == "aWiD")
    {
      AwIDin = server.arg ( i );
    }    
    if ( server.argName ( i ) == "awPilotName")
    {
      asPilotNameIn = server.arg ( i );
    }
    if ( server.argName ( i ) == "aWappass")
    {
    	aWappassIn = server.arg ( i );
    }

    if ( server.argName ( i ) == "wat")
    {
        aircraft_type_in = server.arg ( i );
    }
    if ( server.argName ( i ) == "selectpkt")
    {
       packetsel = server.arg ( i );
    }
  }

  update_configFile(UpSsid, UpPass, GS, AwIDin,asPilotNameIn,aWappassIn,aircraft_type_in, AwIDin ,packetsel);

  server.send ( 200, "text/html", "<html><head>\
<style>.bu {background-color: #d7d7d7;border: none;color: black;padding: 8px 32px;\
text-decoration: none;font-size: 18px;margin: 4px 2px;}\
body{font: normal 12px Verdana, Arial, sans-serif;background-color:#e6e7e8}</style>\
<h1>AirWhere Rebooting<br><br></h1>Please reconnect to the Access Point again<br><br><a href=/>Reconnect to AirWhere home page</a></h2>" );


  DEBUG_SERIAL_UART("[%d] -!!!!! Restarting AirWhere !!!!!\r\n", millis());
  ESP.restart();
}


void updateAWHardware()
{
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (updateAW)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  //String UpSsid, UpPass, GS = "n", AwIDin, asPilotNameIn, aWappassIn,aircraft_type_in;
  String UpNavSW, GS = "n", hwMode, gpsBaud,WOSout,LfreqIn, radio_board;

  for ( uint8_t i = 0; i < server.args(); i++ )
  {
      if ( server.argName ( i ) == "navsw")
      {
         UpNavSW = server.arg ( i );
      }
      if ( server.argName ( i ) == "hwMode")
      {
         hwMode = server.arg ( i );
      }
      if ( server.argName ( i ) == "gpsBaud")
      {
         gpsBaud = server.arg ( i );
      }
      if ( server.argName ( i ) == "selectwifioff")
      {
         WOSout = server.arg ( i );
      }
      if ( server.argName ( i ) == "Lfreq")
      {
         LfreqIn = server.arg ( i );
      }
      if ( server.argName ( i ) == "rfboard")
      {
         radio_board = server.arg ( i );
      }

  }

  update_configFile('h',UpNavSW, GS = "n", hwMode, gpsBaud,WOSout,LfreqIn, radio_board );

  server.send ( 200, "text/html", "<html><head>\
<style>.bu {background-color: #d7d7d7;border: none;color: black;padding: 8px 32px;\
text-decoration: none;font-size: 18px;margin: 4px 2px;}\
body{font: normal 12px Verdana, Arial, sans-serif;background-color:#e6e7e8}</style>\
<h1>AirWhere Rebooting<br><br></h1>Please reconnect to the Access Point again<br><br><a href=/>Reconnect to AirWhere home page</a></h2>" );


  DEBUG_SERIAL_UART("[%d] -!!!!! Restarting AirWhere !!!!!\r\n", millis());
  ESP.restart();
}

void updateGSgsm()
{
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (updateGS)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  String awID, gsLatitudeSwitch, gsLongitudeSwitch, gsAltitudeS, gsmAPNweb, gsmUNweb, gsmPWweb, UpNavSW = "L" , GS = "y";

  for ( uint8_t i = 0; i < server.args(); i++ )
  {
    DEBUG_SERIAL_UART_S("[" + String(millis()) + "] - Server Arg :>" + server.argName (i) + "\r\n");

    if ( server.argName ( i ) == "aWiD")
    {
        awID = server.arg ( i );
    }
    if ( server.argName ( i ) == "gsLatitudeSwitch")
    {
      gsLatitudeSwitch = server.arg ( i );
    }
    if ( server.argName ( i ) == "gsLongitudeSwitch")
    {
      gsLongitudeSwitch = server.arg ( i );
    }
    if ( server.argName ( i ) == "gsAltitudeSwitch")
    {
      gsAltitudeS = server.arg ( i );
    }
    if ( server.argName ( i ) == "gsmAPNweb")
    {
        gsmAPNweb = server.arg ( i );
    }
    if ( server.argName ( i ) == "gsmUNweb")
    {
        gsmUNweb = server.arg ( i );
    }
    if ( server.argName ( i ) == "gsmPWweb")
    {
        gsmPWweb = server.arg ( i );
    }

  }

  update_configFile(awID, gsLatitudeSwitch, gsLongitudeSwitch, gsAltitudeS, gsmAPNweb,gsmUNweb, gsmPWweb, UpNavSW, GS, receiveFlarm);

  server.send ( 200, "text/html", "<h1>AirWhere Ground Station Rebooting with new settings  - please reconnect to the Access Point again<br><br></h1><a href=/>Reconnect to AirWhere home page</a></h2>" );


  DEBUG_SERIAL_UART("[%d] -!!!!! Restarting AirWhere !!!!!\r\n", millis());
  ESP.restart();
}



void updateGS()
{
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (updateGS)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  String awID, gsLatitudeSwitch, gsLongitudeSwitch, gsAltitudeS, UpSsid, UpPass, UpNavSW = "L" , GS = "y";

  for ( uint8_t i = 0; i < server.args(); i++ )
  {
    DEBUG_SERIAL_UART_S("[" + String(millis()) + "] - Server Arg :>" + server.argName (i) + "\r\n");

    if ( server.argName ( i ) == "aWiD")
    {
        awID = server.arg ( i );
    }
    if ( server.argName ( i ) == "gsLatitudeSwitch")
    {
      gsLatitudeSwitch = server.arg ( i );
    }
    if ( server.argName ( i ) == "gsLongitudeSwitch")
    {
      gsLongitudeSwitch = server.arg ( i );
    }
    if ( server.argName ( i ) == "gsAltitudeSwitch")
    {
      gsAltitudeS = server.arg ( i );
    }
    if ( server.argName ( i ) == "aWssidSwitch")
    {
      UpSsid = server.arg ( i );
    }
    if ( server.argName ( i ) == "aWpassSwitch")
    {
      UpPass = server.arg ( i );
    }

  }

  update_configFile(awID, gsLatitudeSwitch, gsLongitudeSwitch, gsAltitudeS, UpSsid, UpPass, UpNavSW, GS, receiveFlarm);

  server.send ( 200, "text/html", "<h1>AirWhere Ground Station Rebooting with new settings  - please reconnect to the Access Point again<br><br></h1><a href=/>Reconnect to AirWhere home page</a></h2>" );


  DEBUG_SERIAL_UART("[%d] -!!!!! Restarting AirWhere !!!!!\r\n", millis());
  ESP.restart();
}

void updateVersion()
{
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (updateVersion)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  yieldDelay(500);
  yield();

  //  auto ret = ESPhttpUpdate.update("www.air-ware.co.uk", 80, "/downloads/AirWare.bin");
  /*  server.send ( 200, "text/html", "<!DOCTYPE html><html><style>#myProgress{position: relative;width: 100%;height: 30px;background-color: #ddd;}#myBar {position: absolute;width: 1%;height: 100%;background-color: #4CAF50;}\
               </style><body>Airware updating to NEW VERSION - please wait - if the update is successful the board will automatically reboot, please then reconnect , if the update fails the update number will still be the same.\
               <div id=myProgress><div id=myBar></div></div><script>var width = 1;setInterval(move, 200);function move(){var elem = document.getElementById(\"myBar\");\
               width++;var id = setInterval(frame, 10);function frame() {if (width >= 100) {clearInterval(id);} elem.style.width = width + '%';}}</script></body></html>" );
  */
  server.send ( 200, "text/html", "<!DOCTYPE html><html><style>#myProgress{position: relative;width: 100%;height: 30px;background-color: #ddd;}\
#myBar {position: absolute;width: 1%;height: 100%;background-color: #4CAF50;}\
.bu {background-color: #d7d7d7;border: none;color: black;padding: 8px 32px;\
text-decoration: none;font-size: 18px;margin: 4px 2px;}\
body{font: normal 12px Verdana, Arial, sans-serif;background-color:#e6e7e8}</style><body onload=\"move()\" width=100%>\
<script>var width = 1;function move() {var elem = document.getElementById(\"myBar\");\
var id = setInterval(frame, 700);function frame(){width++;if (width >= 100){clearInterval(id);\
document.getElementById(\"p1\").innerHTML = \"<br><br><a href=/>AirWhere Home</a>\";}\
elem.style.width = width + '%';}}</script><div>AirWhere updating to NEW VERSION - please wait - if \
the update is successful the board will automatically reboot, please then reconnect , if the update \
fails the update number will still be the same.<div id=myProgress><div id=myBar></div>\
<div align=center><p id=\"p1\"><br><br>>>>      Installing AirWhere     >>></p></div></body></html>" );

  if (wiredDirectly=='y')
  {
      Serial1.println(ESP.getFreeHeap());
   Serial1.println("Updating AirWhere - please wait.");
  }
  else
  {Serial.println(ESP.getFreeHeap());
     Serial.println("Updating AirWhere - please wait.");
  }

  yieldDelay(1000);
  Serial.end();
 // WifiUDP::stopAll();
  yieldDelay(500);

  // t_httpUpdate_return ret = ESPhttpUpdate.update("www.air-ware.co.uk", 80 , "/downloads/AirWare.ino.bin" );
  t_httpUpdate_return ret = ESPhttpUpdate.update("www.airwhere.co.uk", 80 , "/downloads/AirWare.ino.bin" );
  //  t_httpUpdate_return ret = ESPhttpUpdate.update("192.168.1.171", 80 , "/downloads/AirWare.ino.bin" );
  Serial.begin(gpsBaudRate);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      DEBUG_SERIAL_UART("[%d] - OTA - HTTP_UPDATE_FAILED\r\n", millis());
      if (wiredDirectly=='y')
      {
       Serial1.println("HTTP_UPDATE_FAILED.");
      }
      else
      {
         Serial.println("HTTP_UPDATE_FAILED.");
      }
      break;

    case HTTP_UPDATE_OK:
      DEBUG_SERIAL_UART("[%d] - OTA - HTTP_UPDATE_OK\r\n", millis());
      if (wiredDirectly=='y')
      {
       Serial1.println("HTTP_UPDATE_OK.");
      }
      else
      {
         Serial.println("HTTP_UPDATE_OK.");
      }
      break;

    case HTTP_UPDATE_NO_UPDATES:
      DEBUG_SERIAL_UART("[%d] - OTA - HTTP_UPDATE_FAILED\r\n", millis());
      if (wiredDirectly=='y') {
       Serial1.println("HTTP_UPDATE_FAILED.");

      }
      else
      {
         Serial.println("HTTP_UPDATE_FAILED.");
      }
      break;
  }


  yieldDelay(500);
  ESP.restart();
}

void devVersion()
{
  DEBUG_SERIAL_UART("[%d] - Disconnecting Nmea (devVersion)\r\n", millis());
  webTime = millis() + timeOnWeb;
  //v16
  close_socket();

  //  auto ret = ESPhttpUpdate.update("www.air-ware.co.uk", 80, "/downloads/AirWare.bin");
  yieldDelay(500);

  /*  server.send ( 200, "text/html", "<!DOCTYPE html><html><style>#myProgress{position: relative;width: 100%;height: 30px;background-color: #ddd;}#myBar {position: absolute;width: 1%;height: 100%;background-color: #4CAF50;}\
                 </style><body>Airware updating to DEV VERSION - please wait - if the update is successful the board will automatically reboot, please then reconnect , if the update fails the update number will still be the same.\
                 <div id=myProgress><div id=myBar></div></div><script>var width = 1;setInterval(move, 200);function move(){var elem = document.getElementById(\"myBar\");\
                 width++;var id = setInterval(frame, 10);function frame() {if (width >= 100) {clearInterval(id);} elem.style.width = width + '%';}}</script></body></html>" );
  */
  server.send ( 200, "text/html", "<!DOCTYPE html><html><style>#myProgress{position: relative;width: 100%;height: 30px;background-color: #ddd;}\
#myBar {position: absolute;width: 1%;height: 100%;background-color: #4CAF50;}</style><body onload=\"move()\" width=100%>\
<script>var width = 1;function move() {var elem = document.getElementById(\"myBar\");\
var id = setInterval(frame, 700);function frame(){width++;if (width >= 100){clearInterval(id);\
document.getElementById(\"p1\").innerHTML = \"<br><br><a href=/>AirWhere Home</a>\";}\
elem.style.width = width + '%';}}</script><div>AirWhere updating to DEVELOPMENT VERSION - please wait<br><Br>\
If the update is successful the board will automatically reboot, reconnect, if the update \
fails the update number will still be the same. <br><br>Please only use this version for testing unless\
otherwise specified.<div id=myProgress><div id=myBar></div>\
<div align=center><p id=\"p1\"><br><br>>>>      Installing AirWhere     >>></p></div></body></html>" );


  if (wiredDirectly=='y')
  {
      Serial1.println(ESP.getFreeHeap());
      Serial1.println("Updating AirWhere to Dev Version - please wait.");

  }
  else
  {
      Serial.println(ESP.getFreeHeap());
      Serial.println("Updating AirWhere to Dev Version - please wait.");
  }


  yieldDelay(500);
  Serial.end();
  yieldDelay(500);
 // WifiUDP::stopAll();

  yieldDelay(1500);
  // t_httpUpdate_return ret = ESPhttpUpdate.update("www.air-ware.co.uk", 80 , "/downloads/AirWare.ino.bin" );
  t_httpUpdate_return ret = ESPhttpUpdate.update("www.airwhere.co.uk", 80 , "/downloads/AirWareDev.ino.bin" );
  //  t_httpUpdate_return ret = ESPhttpUpdate.update("192.168.1.171", 80 , "/downloads/AirWare.ino.bin" );
  //Serial.begin(gpsBaudRate);
  Serial.begin(gpsBaudRate);
  delay(500);


  switch (ret) {
    case HTTP_UPDATE_FAILED:
      DEBUG_SERIAL_UART("[%d] - OTA - HTTP_UPDATE_FAILED\r\n", millis());
      if (wiredDirectly=='y')
      {
       Serial1.println("HTTP_UPDATE_FAILED - Please try again");
      }
      else
      {
         Serial.println("HTTP_UPDATE_FAILED - Please try again.");
      }
      break;

    case HTTP_UPDATE_OK:
      DEBUG_SERIAL_UART("[%d] - OTA - HTTP_UPDATE_OK\r\n", millis());
      if (wiredDirectly=='y')
      {
       Serial1.println("HTTP_UPDATE_OK.");
      }
      else
      {
         Serial.println("HTTP_UPDATE_OK.");
      }
      break;

    case HTTP_UPDATE_NO_UPDATES:
      DEBUG_SERIAL_UART("[%d] - OTA - HTTP_UPDATE_FAILED\r\n", millis());
      if (wiredDirectly=='y') {
       Serial1.println("HTTP_UPDATE_FAILED - Please try again.");
      }
      else
      {
         Serial.println("HTTP_UPDATE_FAILED - Please try again.");
      }
      break;
  }
  delay(500);
  ESP.restart();
 }


void Web_setup()
{
  server.on ( "/", HTTP_ANY, displayWeb );

  //http://www.msftncsi.com/ncsi.txt

  server.on ( "/ncsi.txt", HTTP_ANY, []() {
    server.send ( 200, "text/html", "Microsoft NCSI" );
    DEBUG_SERIAL_UART("[%d] - Received dodgy new firmware kobo connect\n Kobo looking for NSCI site\n Returning spoofed site.\r\n", millis());
    webTime = millis() + timeOnWeb;
    dhcp_client_connected = true;
  } );


  server.on ( "/library/test/success.html", HTTP_ANY, []() {
    server.send ( 200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>" );
    DEBUG_SERIAL_UART("[%d] - Received dodgy kobo connect , looking for APPLE site, return spoofed apple site\r\n", millis());
    webTime = millis() + timeOnWeb;
  } );


  server.on ( "/config", HTTP_ANY, configureAW );
  server.on ( "/configh", HTTP_ANY, configureAWHardware );
  server.on ( "/update", HTTP_POST, updateAW );
  server.on ( "/updateh", HTTP_POST, updateAWHardware );
  server.on ( "/updateGS", HTTP_POST, updateGS );
  server.on ( "/updateGSgsm", HTTP_POST, updateGSgsm );
//  server.on ( "/update", HTTP_ANY, updateAW_entered_directly );
  server.on ( "/version", HTTP_ANY, aWversion );
  server.on ( "/updateVersion", HTTP_ANY, updateVersion );
  server.on ( "/devVersion", HTTP_ANY, devVersion );
  server.on ( "/switch", HTTP_ANY, switchGS );
  server.on ( "/switchAW", HTTP_ANY, switchAW );
  server.on ( "/switchIGSMode", HTTP_ANY, switchIGSMode );
  server.on ( "/switchGSMGS", HTTP_ANY, switchGSMGS );
  //server.on ( "/flarmConf", HTTP_ANY, flarmConf );
  //v16
  server.on ( "/updateflarm", HTTP_ANY, updateflarm );
  server.on ( "/processfupdate", HTTP_ANY, processfupdate );
  server.on ( "/stats", HTTP_ANY, aw_stats );


  server.onNotFound([]() {
    server.send(200, "text/html", "Web Page doesnt Exist - please reconnect");
    DEBUG_SERIAL_UART("[%d] - Received a Not Found request - Returning \"Success\"\r\n", millis());
    webTime = webTime + timeOnWeb;
  });

  server.begin();
  DEBUG_SERIAL_UART("[%d] - HTTP server started\r\n", millis());

}

void Web_loop()
{
  dnsServer.processNextRequest();
  yield();
  //DEBUG_SERIAL_UART_MAX("[%d] - After DNS Process\r\n", millis());
  server.handleClient();
  yield();
  // DEBUG_SERIAL_UART_MAX("[%d] - Ending Webloop\r\n", millis());
}

