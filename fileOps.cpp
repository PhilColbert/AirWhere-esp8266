#include "fileOps.h"


int gpsSelectToBaud(char sel)
{

  if (sel=='9')
  {
    return 9600;
  }
  if (sel=='2')
  {
    return 19200;
  }
  if (sel=='3')
  {
    return 38400;
  }
  if (sel=='5')
  {
    return 57600;
  }
  if (sel=='1')
  {
    return 115200;
  }

  return 9;

}

char gpsBaudtoChar(int sel)
{

  if (sel==9600)
  {
    return '9';
  }
  if (sel==19200)
  {
    return '2';
  }
  if (sel==38400)
  {
    return '3';
  }
  if (sel==57600)
  {
    return '5';
  }
  if (sel==115200)
  {
    return '1';
  }
  
  return '9';

}


void load_configFile()
{

  SPIFFS.begin();


/*  File testF = SPIFFS.open("/aw.txt", "w");
      testF.println("ssid=ASUS");
      testF.println("pass=davedave");
      testF.println("navS=L"); //LK
      testF.println("flrm=0"); //do not receive flarm 
  testF.close();
  */
  //DEBUG_SERIAL_UART("[%d] - SPIFFS opened:\r\n", millis(),result);
  File f = SPIFFS.open("/aw.txt", "r");  

//v16.1
  String rndPilotNo="awhid=0000";
 
  if (!f) 
  {
   // DEBUG_SERIAL_UART("[%d] - File doesn't exist yet. Creating it\r\n", millis(),result);

    File f = SPIFFS.open("/aw.txt", "w");
    if (!f)
    {
  //    DEBUG_SERIAL_UART("[%d] - File creation failed\r\n", millis(),result);
    }
    else
    {
      f.println("ssid=");
      f.println("pass=");
      f.println("navS=L"); //LK
      f.println("flrm=n"); //do not receive flarm 
      f.println("gs=n"); // ground station is not set by default
      f.println("hw=n"); // hard wired not set by default
      f.println("lat=00.0000000");    
      f.println("long=000.0000000");   
      f.println("alt=0");
      f.println("gpsB=9");       
      f.println("wo=n");  
      f.println(rndPilotNo);      
      f.println("awpn="); 
      f.println("awap=12345678");
      f.println("freq=8");
      f.println("airT=1");
      f.println("radB=l");
      //v16
      f.println("gsty=i");
      f.println("gsmap=");
      f.println("gsmun=");
      f.println("gsmpw=");
      f.println("psel=n");

    }
  }
  else
  {
    while(f.available())
    {
      delay(20);
      String line = f.readStringUntil('\n');
     // Serial.println("["+String(millis()) + "] - Line from Config File :>" +line + "\r\n");
      
      if (line.substring(0,4)=="ssid")
      {
        String ssid=line.substring(5);
        int len=line.length()-5;
        // if theres no ssid, the network wont be tried to connect and it will leave the AP in a strange state.
        // so set to N

        if (line.length()==6)
        {
            airWareSsid[0]='N';
        }
        else
        {
            ssid.toCharArray(airWareSsid,len);
        }
      }
      if (line.substring(0,4)=="pass")
      {
        String pw=line.substring(5);
        int len=line.length()-5;
        if (pw=="")
        {
            airWarePassword[0]='N';
        }
        else
        {
            pw.toCharArray(airWarePassword,len);
        }


      }
      if (line.substring(0,4)=="navS")
      {
        navSofware=line.charAt(5);
      }
      if (line.substring(0,4)=="flrm")
      {
        receiveFlarm=line.charAt(5);
      }
      if (line.substring(0,3)=="gs=")
      {
        groundStationMode=line.charAt(3);
      }
      if (line.substring(0,2)=="hw")
      {
        wiredDirectly=line.charAt(3);
      }
      if (line.substring(0,3)=="lat")
      {
        String ls=line.substring(4);
        ls.toCharArray(gsLatitude,ls.length());
      }     
      if (line.substring(0,4)=="long")
      {
        String ls=line.substring(5); 
        ls.toCharArray(gsLongitude,ls.length());
      }          
      if (line.substring(0,3)=="alt")
      {
        String ls=line.substring(4);
        ls.toCharArray(gsAltitude,ls.length());        
      }        
      if (line.substring(0,4)=="gpsB")
      {
        gpsBaudRate=gpsSelectToBaud(line.charAt(5));
      }  
      if (line.substring(0,2)=="wo")
      {
        wifioffSelected=line.charAt(3);
      }   
      if (line.substring(0,4)=="awid")
      {
        airWhereID=line.substring(5).toInt();
    //    Serial.println(airWhereID);
      }  
      if (line.substring(0,5)=="awhid")
      {
          String ls=line.substring(6);
          ls.toCharArray(awHexID,ls.length());
      }
      if (line.substring(0,4)=="awpn")
      {
        String ls=line.substring(5);
        ls.toCharArray(awPilotName,ls.length());     
    //    Serial.println(awPilotName);       
      } 
      if (line.substring(0,4)=="awap")
      {
        String ls=line.substring(5);
        ls.toCharArray(airWhere_ap_password,ls.length());
    //    Serial.println(awPilotName);
      }
      if (line.substring(0,4)=="freq")
      {
        loraFrequency=line.charAt(5);
  //    Serial.println(awPilotName);
      }
      if (line.substring(0,4)=="radB")
      {
        lora_or_flarm=line.charAt(5);
      }
      if (line.substring(0,4)=="airT")
      {
        web_aircraft_type=line.charAt(5);
      }
      //v16
      if (line.substring(0,4)=="gsty")
      {
         groundstation_type=line.charAt(5);
      }
      if (line.substring(0,5)=="gsmap")
      {
          String ls=line.substring(6);
          ls.toCharArray(gsm_access_point_name,ls.length());
      }
      if (line.substring(0,5)=="gsmun")
      {
          String ls=line.substring(6);
          ls.toCharArray(gsm_username,ls.length());
      }
      if (line.substring(0,5)=="gsmpw")
      {
          String ls=line.substring(6);
          ls.toCharArray(gsm_password,ls.length());
      }
      if (line.substring(0,4)=="psel")
      {
          packet_repeat=line.charAt(5);
      }

      // to keep current system working - replace with a value 60k-65535.
      //v16.1
   //   if (airWhereID==0)
    //  {
     //   String myID = String(ESP.getChipId());
      //  myID = myID.substring(0, 6);
       // myChipID = myID.toInt();
       // airWhereID=myChipID;
     // }

    }    
  }
}
//update_configFile(UpSsid, UpPass, GS, AwIDin,asPilotNameIn,aWappassIn,aircraft_type_in );
//pilot settings
void update_configFile(String UpSsid,String UpPass, String GS,  String AwIDin, String awPilotNameIn, String aWappassIn, String aircraft_type_in,String AwIDin1, String packetsel )
{
  String gsLa=String(gsLatitude);
  String gsLo=String(gsLongitude);
  String gsA=String(gsAltitude);
  String rcF=String(receiveFlarm);
  String Fns=String(navSofware);
  String wd=String(wiredDirectly);
  String gpsB=String(gpsBaudtoChar(gpsBaudRate));
  String WOSin=String (wifioffSelected);
  String LfreqIn=String(loraFrequency);
  //v16
  String radBIn=String(lora_or_flarm);


  File testF = SPIFFS.open("/aw.txt", "w");
      testF.println("ssid="+UpSsid);
      testF.println("pass="+UpPass);
      testF.println("navS="+Fns); //LK
      testF.println("flrm="+rcF); //do not receive flarm 
      testF.println("gs="+GS); 
      testF.println("hw="+wd);
      testF.println("lat="+gsLa);    
      testF.println("long="+gsLo);        
      testF.println("alt="+gsA);
      testF.println("gpsB="+gpsB);
      testF.println("wo="+WOSin);
      testF.println("awhid="+AwIDin);
      testF.println("awpn="+awPilotNameIn);
      testF.println("awap="+aWappassIn);
      testF.println("freq="+LfreqIn);
      testF.println("airT="+aircraft_type_in);

      //v16
      testF.println("radB="+radBIn);
      testF.println("gsty="+String (groundstation_type));
      testF.println("gsmap="+String (gsm_access_point_name));
      testF.println("gsmun="+String (gsm_username));
      testF.println("gsmpw="+String (gsm_password));

      //v17
      testF.println("psel="+packetsel);



  testF.close();
  
}

// update hardware section
void update_configFile(char selector, String UpNavSW, String GS, String hwMode, String gpsBaud, String WOSout,String LfreqIn, String radio_board )
{
// working on
    String FawS=String(airWareSsid);
    String FawP=String(airWarePassword);
   // String Fns=String(navSofware);
    String rcF=String(receiveFlarm);

    String Fgsm=String(groundStationMode);
    String Fgsla=String(gsLatitude);
    String FgsLo=String(gsLongitude);
    String FgsA=String(gsAltitude);

//    String wd=String(wiredDirectly);
    //String gpsB=String(gpsBaud);
  //  String wOS=String (wifioffSelected);

    //v16.1
    String AwhIDin=String (awHexID);

    String awPilotNameIn=String (awPilotName);
    String aWappassIn=String (airWhere_ap_password);
  //  String LfreqIn=String(LfreqIn);
    String aircraft_type_in=String(web_aircraft_type);




    File testF = SPIFFS.open("/aw.txt", "w");

        testF.println("ssid="+FawS);
        testF.println("pass="+FawP);
        testF.println("navS="+UpNavSW); //LK
        testF.println("flrm="+rcF); //do not receive flarm

        testF.println("gs="+Fgsm); //do not receive flarm
        testF.println("hw="+hwMode);
        testF.println("lat="+Fgsla);
        testF.println("long="+FgsLo);

        testF.println("alt="+FgsA);
        testF.println("gpsB="+gpsBaud);
        testF.println("wo="+WOSout);
        testF.println("awhid="+AwhIDin);

        testF.println("awpn="+awPilotNameIn);
        testF.println("awap="+aWappassIn);
        testF.println("freq="+LfreqIn);
        testF.println("airT="+aircraft_type_in);
        //v17
         testF.println("psel="+String (packet_repeat));

        //v16
        testF.println("radB="+radio_board);
        testF.println("gsty="+String (groundstation_type));
        testF.println("gsmap="+String (gsm_access_point_name));
        testF.println("gsmun="+String (gsm_username));
        testF.println("gsmpw="+String (gsm_password));



    testF.close();


}


void update_configFile(String UpSsid,String UpPass,String UpNavSW, String GS, String hwMode, String gpsBaud,
                       String WOSin, String AwhIDin, String awPilotNameIn, String aWappassIn, String LfreqIn, String aircraft_type_in )
{

  String gsLa=String(gsLatitude);
  String gsLo=String(gsLongitude);
  String gsA=String(gsAltitude);
  String rcF=String(receiveFlarm);
  String hwM=String (hwMode);
  String wd=String(wiredDirectly);
  String gpsB=String(gpsBaudtoChar(gpsBaudRate));
  String wOS=String (wifioffSelected);
 /* String AwIDin=String (airWhereID);
  String awPilotNameIn=String (awPilotName);
  String aWappassIn=String (airWhere_ap_password);
  String LfreqIn=String(loraFrequency);
  String aircraft_type_in=String(web_aircraft_type); */
  String radBIn=String(lora_or_flarm);

 // Serial.println("awid>" + AwIDin + "<");
 // Serial.println("awpn>" + awPilotNameIn + "<");

  // Serial.println("airT="+aircraft_type_in);


  File testF = SPIFFS.open("/aw.txt", "w");
      testF.println("ssid="+UpSsid);
      testF.println("pass="+UpPass);
      testF.println("navS="+UpNavSW); //LK
      testF.println("flrm="+rcF); //do not receive flarm
      testF.println("gs="+GS);
      testF.println("hw="+hwMode);
      testF.println("lat="+gsLa);
      testF.println("long="+gsLo);
      testF.println("alt="+gsA);
      testF.println("gpsB="+gpsBaud.substring(0,1));
      testF.println("wo="+WOSin);
      testF.println("awhid="+AwhIDin);
      testF.println("awpn="+awPilotNameIn);
      testF.println("awap="+aWappassIn);
      testF.println("freq="+LfreqIn);
      testF.println("airT="+aircraft_type_in);
      //v16
      testF.println("radB="+radBIn);
      testF.println("gsty="+String (groundstation_type));
      testF.println("gsmap="+String (gsm_access_point_name));
      testF.println("gsmun="+String (gsm_username));
      testF.println("gsmpw="+String (gsm_password));
     //v17
      testF.println("psel="+String (packet_repeat));
  testF.close();

}


void update_configFile(String UpSsid,String UpPass,String UpNavSW, String GS)
{

  String gsLa=String(gsLatitude);
  String gsLo=String(gsLongitude);
  String gsA=String(gsAltitude);
  String rcF=String(receiveFlarm);
  String wd=String(wiredDirectly);
  String gpsB=String(gpsBaudtoChar(gpsBaudRate));
  String wOS=String (wifioffSelected); 
  //v16.1
  String AwhIDin=String (awHexID);
  String awPilotNameIn=String (awPilotName);
  String aWappassIn=String (airWhere_ap_password);
  String LfreqIn=String(loraFrequency);
  String aircraft_type_in=String(web_aircraft_type);
  String radBIn=String(lora_or_flarm);

  File testF = SPIFFS.open("/aw.txt", "w");
      testF.println("ssid="+UpSsid);
      testF.println("pass="+UpPass);
      testF.println("navS="+UpNavSW); //LK
      testF.println("flrm="+rcF); //do not receive flarm 
      testF.println("gs="+GS); 
      testF.println("hw="+wd);       
      testF.println("lat="+gsLa);    
      testF.println("long="+gsLo);        
      testF.println("alt="+gsA);
      testF.println("gpsB="+gpsB);
      testF.println("wo="+wOS);      
      testF.println("awhid="+AwhIDin);
      testF.println("awpn="+awPilotNameIn);
      testF.println("awap="+aWappassIn);
      testF.println("freq="+LfreqIn);
      testF.println("airT="+aircraft_type_in);
      //v16
      testF.println("radB="+radBIn);
      testF.println("gsty="+String (groundstation_type));
      testF.println("gsmap="+String (gsm_access_point_name));
      testF.println("gsmun="+String (gsm_username));
      testF.println("gsmpw="+String (gsm_password));
      //v17
       testF.println("psel="+String (packet_repeat));
  testF.close();
  
}

//updating internet ground station params
void update_configFile(String AwhIDin, String gsLatitudeSwitch, String gsLongitudeSwitch, String gsAltitudeI, String UpSsid, String UpPass, String UpNavSW, String GS, char rcF)
{
  DEBUG_SERIAL_UART_S("["+String(millis()) + "] - vars are :>" + gsLatitudeSwitch + ":" + gsLongitudeSwitch + ":" + gsAltitudeI + ":" + UpSsid + ":" + UpPass + "\r\n");

 //String rcF=String(receiveFlarm);
 String gpsB=String(gpsBaudtoChar(gpsBaudRate));
 String wOS=String (wifioffSelected);
 // String AwIDin=String (airWhereID);
  String awPilotNameIn=String (awPilotName);
  String wd=String(wiredDirectly);
  String aWappassIn=String (airWhere_ap_password);
  String LfreqIn=String(loraFrequency);
  String aircraft_type_in=String(web_aircraft_type);
  String radBIn=String(lora_or_flarm);

 File testF = SPIFFS.open("/aw.txt", "w");
      testF.println("ssid="+UpSsid);
      testF.println("pass="+UpPass);
      testF.println("navS="+UpNavSW); //LK
      testF.println("flrm="+rcF); //do not receive flarm
      testF.println("gs="+GS); //do not receive flarm
      testF.println("hw="+wd);
      testF.println("lat="+gsLatitudeSwitch);
      testF.println("long="+gsLongitudeSwitch);
      testF.println("alt="+gsAltitudeI);
      testF.println("gpsB="+gpsB);
      testF.println("wo="+wOS);
      testF.println("awhid="+AwhIDin);
      testF.println("awpn="+awPilotNameIn);
      testF.println("awap="+aWappassIn);
      testF.println("freq="+LfreqIn);
      testF.println("airT="+aircraft_type_in);
      //v16
      testF.println("radB="+radBIn);
      testF.println("gsty="+String (groundstation_type));
      testF.println("gsmap="+String (gsm_access_point_name));
      testF.println("gsmun="+String (gsm_username));
      testF.println("gsmpw="+String (gsm_password));
      //v17
       testF.println("psel="+String (packet_repeat));
  testF.close();
}

//v16 updating gsm ground station params.
void update_configFile(String AwhIDin, String gsLatitudeSwitch, String gsLongitudeSwitch, String gsAltitudeI, String gsmAPNweb, String gsmUNweb, String gsmPWweb, String UpNavSW, String GS, char rcF)
{
 // DEBUG_SERIAL_UART_S("["+String(millis()) + "] - vars are :>" + gsLatitudeSwitch + ":" + gsLongitudeSwitch + ":" + gsAltitudeI + ":" + UpSsid + ":" + UpPass + "\r\n");



 String gpsB=String(gpsBaudtoChar(gpsBaudRate));
 String wOS=String (wifioffSelected); 
  String awPilotNameIn=String (awPilotName); 
  String wd=String(wiredDirectly);
  String aWappassIn=String (airWhere_ap_password);
  String LfreqIn=String(loraFrequency);
  String aircraft_type_in=String(web_aircraft_type);
  String radBIn=String(lora_or_flarm);
 
 File testF = SPIFFS.open("/aw.txt", "w");
      testF.println("ssid="+String(airWareSsid));
      testF.println("pass="+String(airWarePassword));
      testF.println("navS="+UpNavSW); //LK
      testF.println("flrm="+rcF); //do not receive flarm 
      testF.println("gs="+GS); //do not receive flarm 
      testF.println("hw="+wd);
      testF.println("lat="+gsLatitudeSwitch);    
      testF.println("long="+gsLongitudeSwitch);    
      testF.println("alt="+gsAltitudeI);   
      testF.println("gpsB="+gpsB);
      testF.println("wo="+wOS);
      testF.println("awhid="+AwhIDin);
      testF.println("awpn="+awPilotNameIn); 
      testF.println("awap="+aWappassIn);
      testF.println("freq="+LfreqIn);
      testF.println("airT="+aircraft_type_in);
      //v16
      testF.println("radB="+radBIn);
      testF.println("gsty="+String (groundstation_type));
      testF.println("gsmap="+ gsmAPNweb);
      testF.println("gsmun="+gsmUNweb);
      testF.println("gsmpw="+gsmPWweb);
      //v17
       testF.println("psel="+String (packet_repeat));
  testF.close();
}



void update_configFile(String flarm)
{
  
  String FawS=String(airWareSsid);
  String FawP=String(airWarePassword);
  String Fns=String(navSofware);
  
  String Fgsm=String(groundStationMode);
  String Fgsla=String(gsLatitude);
  String FgsLo=String(gsLongitude);
  String FgsA=String(gsAltitude);
  String wd=String(wiredDirectly);

  String gpsB=String(gpsBaudtoChar(gpsBaudRate));
  String wOS=String (wifioffSelected); 

  //v16.1
  String AwhIDin=String (awHexID);
  String awPilotNameIn=String (awPilotName);
  String aWappassIn=String (airWhere_ap_password);
  String LfreqIn=String(loraFrequency);
  String aircraft_type_in=String(web_aircraft_type);
  String radBIn=String(lora_or_flarm);
  
  File testF = SPIFFS.open("/aw.txt", "w");
      testF.println("ssid="+FawS);
      testF.println("pass="+FawP);
      testF.println("navS="+Fns); //LK
      testF.println("flrm="+flarm); //do not receive flarm 
      testF.println("gs="+Fgsm); //do not receive flarm 
      testF.println("hw="+wd);      
      testF.println("lat="+Fgsla);    
      testF.println("long="+FgsLo);        
      testF.println("alt="+FgsA);
      testF.println("gpsB="+gpsB);
      testF.println("wo="+wOS);
      testF.println("awhid="+AwhIDin);
      testF.println("awpn="+awPilotNameIn);
      testF.println("awap="+aWappassIn);
      testF.println("freq="+LfreqIn);
      testF.println("airT="+aircraft_type_in);
      //v16
      testF.println("radB="+radBIn);
      testF.println("gsty="+String (groundstation_type));
      testF.println("gsmap="+String (gsm_access_point_name));
      testF.println("gsmun="+String (gsm_username));
      testF.println("gsmpw="+String (gsm_password));
      //v17
       testF.println("psel="+String (packet_repeat));
  testF.close();
}
//v16 - switch between gsm and internet.
void update_configFile(char gs, char gsm_or_internet)
{
    String FawS=String(airWareSsid);
    String FawP=String(airWarePassword);
    String Fns=String(navSofware);

    String Fgsm=String(groundStationMode);
    String Fgsla=String(gsLatitude);
    String FgsLo=String(gsLongitude);
    String FgsA=String(gsAltitude);
    String wd=String(wiredDirectly);
    String rcF=String(receiveFlarm);
    String gpsB=String(gpsBaudtoChar(gpsBaudRate));
    String wOS=String (wifioffSelected);

    //v16.1
    String AwhIDin=String (awHexID);
    String awPilotNameIn=String (awPilotName);
    String aWappassIn=String (airWhere_ap_password);
    String LfreqIn=String(loraFrequency);
    String aircraft_type_in=String(web_aircraft_type);
    String radBIn=String(lora_or_flarm);

    File testF = SPIFFS.open("/aw.txt", "w");
        testF.println("ssid="+FawS);
        testF.println("pass="+FawP);
        testF.println("navS="+Fns); //LK
        testF.println("flrm="+rcF); //do not receive flarm
        testF.println("gs="+Fgsm); //do not receive flarm
        testF.println("hw="+wd);
        testF.println("lat="+Fgsla);
        testF.println("long="+FgsLo);
        testF.println("alt="+FgsA);
        testF.println("gpsB="+gpsB);
        testF.println("wo="+wOS);
        testF.println("awhid="+AwhIDin);
        testF.println("awpn="+awPilotNameIn);
        testF.println("awap="+aWappassIn);
        testF.println("freq="+LfreqIn);
        testF.println("airT="+aircraft_type_in);

        testF.println("radB="+radBIn);
        testF.println("gsty="+String (gsm_or_internet));
        testF.println("gsmap="+String (gsm_access_point_name));
        testF.println("gsmun="+String (gsm_username));
        testF.println("gsmpw="+String (gsm_password));
        //v17
         testF.println("psel="+String (packet_repeat));
    testF.close();
}


void update_configFile(char hw)
{
  
  String FawS=String(airWareSsid);
  String FawP=String(airWarePassword);
  String Fns=String(navSofware);
  
  String Fgsm=String(groundStationMode);
  String Fgsla=String(gsLatitude);
  String FgsLo=String(gsLongitude);
  String FgsA=String(gsAltitude);
  String gpsB=String(gpsBaudtoChar(gpsBaudRate));
  String wd=String(wiredDirectly);
  String wOS=String (wifioffSelected);

  //v16.1
  String AwhIDin=String (awHexID);
  String awPilotNameIn=String (awPilotName);
  String aWappassIn=String (airWhere_ap_password);
  String LfreqIn=String(loraFrequency);
  String aircraft_type_in=String(web_aircraft_type);
  String radBIn=String(lora_or_flarm);
  
  File testF = SPIFFS.open("/aw.txt", "w");
      testF.println("ssid="+FawS);
      testF.println("pass="+FawP);
      testF.println("navS="+Fns); //LK
      testF.println("flrm="+receiveFlarm); //do not receive flarm 
      testF.println("gs="+Fgsm); //do not receive flarm 
      testF.println("hw="+hw); //hard wired
      testF.println("lat="+Fgsla);    
      testF.println("long="+FgsLo);        
      testF.println("alt="+FgsA);
      testF.println("gpsB="+gpsB);
      testF.println("wo="+wOS);
      testF.println("awhid="+AwhIDin);
      testF.println("awpn="+awPilotNameIn);
      testF.println("awap="+aWappassIn);
      testF.println("freq="+LfreqIn);
      testF.println("airT="+aircraft_type_in);
      //v16
      testF.println("radB="+radBIn);
      testF.println("gsty="+String (groundstation_type));
      testF.println("gsmap="+String (gsm_access_point_name));
      testF.println("gsmun="+String (gsm_username));
      testF.println("gsmpw="+String (gsm_password));
      //v17
       testF.println("psel="+String (packet_repeat));
  testF.close();
}






