#include <Arduino.h>
#include <WiFiClient.h>
#include "AirWare.h"
#include "WifiTools.h"
#include <ESP8266WiFi.h>
#include <DNSServer.h>
//#include "sx1272.h"

extern "C" {
#include <user_interface.h>
}

String readAllDataOnWifi(WiFiClient LK8000Client, String TCPBuffer , bool &fullLine)
{

  size_t wlen=LK8000Client.available();

  // For some reason if the incoming tcp buffer is too big it crashes the system, limit it to 512, we might miss a few lines now and then
  // but better than crashing the system, seems windows CE doesnt transmit data very well and stalls, kobo is fine :)
  
  if ( wlen > TCP_BUFFER_MAX_LENGTH )
  {
    LK8000Client.flush();
    DEBUG_SERIAL_UART_MAX("[%d] - %%%%%%%% TCP data bytes available :- %d - Flushing WifiClient Buffer\r\n", millis(),wlen);
    wlen=LK8000Client.available();
    DEBUG_SERIAL_UART_MAX("[%d] - %%%%%%%% TCP data bytes available :- %d - AFTER Flushing WifiClient Buffer\r\n", millis(),wlen);
    return "";
  }

  if (wlen!=0)
  {
  
    DEBUG_SERIAL_UART_MAX("[%d] - TCP data bytes available :- %d \r\n", millis(),wlen);
     
    last_data_received=millis();
    uint8_t sbuf[wlen];
    char cbuf[wlen];
    
    LK8000Client.readBytes(sbuf, wlen);        
     
    DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnWifi Point 2 \r\n", millis());
     
    memset(cbuf, 0, wlen);

    DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnWifi Point 2a \r\n", millis());
     
    memcpy(&cbuf,&sbuf, wlen);
   
    DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnWifi Point 3 \r\n", millis());

    TCPBuffer=TCPBuffer+String(cbuf).substring(0,wlen);
  }
  else
  {
     DEBUG_SERIAL_UART_MAX("[%d] - No Data on socket at this point \r\n", millis());
  }
  
  bool requiredNMEA=false;
  int i;

   
  if ( TCPBuffer.length()<5)
  {
    return TCPBuffer;
  }

  // make 100% sure length>0

  int len=TCPBuffer.length()-4;

  if (len<0)
  {
    len=0;
  }
  
  for (i=0;i<len;i++)
  {
  
    if (TCPBuffer.charAt(i)=='$' && ( TCPBuffer.charAt(i+3)== 'G' || TCPBuffer.charAt(i+3)== 'R' ) && ( TCPBuffer.charAt(i+4)== 'G' || TCPBuffer.charAt(i+4)== 'M' ) )
    {
     /// requiredNMEA=true;
      break;
    }
  }  

  fullLine=false;
  
  for (int i=0;i<TCPBuffer.length();i++)
  { 


    if (TCPBuffer.charAt(i)=='\n'|| TCPBuffer.charAt(i)=='\r')
    {
   //   Serial.println("Found end of Line");
      fullLine=true;
      break;
    }
   
  }  
  
 // if (requiredNMEA)
 // {
    return TCPBuffer.substring(i);
//  }
//  else
//  {
 //   return "";
 // }
 }

void writeDataToWifi(WiFiClient LK8000Client, String data)
{
  char *csum_ptr;
  char UDPpacketBuffer[96];
//   char newLine='\r';
  int p=data.length();

  unsigned char cs = 0;
  data.toCharArray(UDPpacketBuffer,p+1);

  cs = 0; //clear any old checksum
  for (unsigned int n = 1; n < strlen(UDPpacketBuffer) - 1; n++)
  {
    cs ^= UDPpacketBuffer[n]; //calculates the checksum
  }

  csum_ptr = UDPpacketBuffer + strlen(UDPpacketBuffer);
  snprintf(csum_ptr, sizeof(UDPpacketBuffer) - strlen(UDPpacketBuffer), "%02X\r\n", cs);

  size_t len = strlen(UDPpacketBuffer);
  uint8_t  sbuf[len];
  
  memset(sbuf, 0, len);
  memcpy(&sbuf, &UDPpacketBuffer, len);
        
  LK8000Client.write((const uint8_t *)sbuf, len);
}

void writeDataToSerial(String data)
{
  char *csum_ptr;
  char UDPpacketBuffer[96];
//   char newLine='\r';
  int p=data.length();

  unsigned char cs = 0;
  data.toCharArray(UDPpacketBuffer,p+1);

  cs = 0; //clear any old checksum
  for (unsigned int n = 1; n < strlen(UDPpacketBuffer) - 1; n++)
  {
    cs ^= UDPpacketBuffer[n]; //calculates the checksum
  }

  csum_ptr = UDPpacketBuffer + strlen(UDPpacketBuffer);
  snprintf(csum_ptr, sizeof(UDPpacketBuffer) - strlen(UDPpacketBuffer), "%02X\r\n", cs);

  size_t len = strlen(UDPpacketBuffer);
  uint8_t  sbuf[len];
  
  memset(sbuf, 0, len);
  memcpy(&sbuf, &UDPpacketBuffer, len);

  // Queue up the packet instead.  
 // Serial1.write((const uint8_t *)sbuf, len);
 if (streamData=="")
 {
   streamData=(String) UDPpacketBuffer;
 }
 else
 {
 // streamData=(String) UDPpacketBuffer+"\r\n"+streamData;
 streamData=(String) UDPpacketBuffer+streamData;
 }
 
}



void addTestPilot(WiFiClient LK8000Client, String Pilot, int Dist, int current_pos)
{
  size_t wlen;
  current_pos=current_pos+50;  
  String cph=String(current_pos);
  String d=String(Dist);
  String movingpilot= "$PFLAA,7,"+d+","+d+",405,2,"+Pilot+"5,"+cph+",,33,0.2,1*";

  size_t lenmp = movingpilot.length();
  uint8_t  sbufmp[lenmp];
  movingpilot.getBytes (sbufmp, lenmp);
  LK8000Client.write((const uint8_t *)sbufmp, lenmp);
  LK8000Client.write("\r");
}     

bool sendUDPPacket ( String packet, IPAddress apFoundIP )
{
   char *csum_ptr;
   char UDPpacketBuffer[96];
   char newLine='\r';
   int p=packet.length();

   unsigned char cs = 0;
   packet.toCharArray(UDPpacketBuffer,p+1);
   
   WiFiUDP udp;

    cs = 0; //clear any old checksum
    for (unsigned int n = 1; n < strlen(UDPpacketBuffer) - 1; n++) {
      cs ^= UDPpacketBuffer[n]; //calculates the checksum
    }

    csum_ptr = UDPpacketBuffer + strlen(UDPpacketBuffer);
    snprintf(csum_ptr, sizeof(UDPpacketBuffer) - strlen(UDPpacketBuffer), "%02X\n", cs);

    udp.beginPacket(apFoundIP, XCSOAR_PORT);
    udp.write(UDPpacketBuffer, strlen(UDPpacketBuffer));
    udp.endPacket();

    return true;
}

bool upLoadtoUDP( String urlToLoad, IPAddress IP, int udpPort)
{
  if (WiFi.status() != WL_CONNECTED)
  {
   return false;
  }

  char UDPpacketBuffer[128]={0};
 // memset(UDPpacketBuffer,0,sizeof(UDPpacketBuffer));

 // Serial1.println ( UDPpacketBuffer);
 // Serial1.println ( urlToLoad.length());
 
  urlToLoad.toCharArray(UDPpacketBuffer,urlToLoad.length()+1);

  WiFiUDP udp;

 
  DEBUG_SERIAL_UART_MAX("[%d] - Crash Marker 3a\r\n", millis());
//  Serial1.println ( urlToLoad);
//  Serial1.println ( urlToLoad.length());
 
  yield();

  while ( lastUdpPacketTime > millis() )
  {
    DEBUG_SERIAL_UART_MAX("[%d] - Waiting for Previous UDP Packet to send ( 100 Millis at the moment ) \r\n", millis());
    //Serial1.println(" Waiting for Previous UDP Packet to send ( 100 Millis at the moment ) \r\n");
    
    myDelay(10);
  }
// ************************ NEEDS CHANGING AND LOWERING - HERE FOR TESTING ONLY *********************************
  lastUdpPacketTime=millis()+20;

  DEBUG_SERIAL_UART_MAX("[%d] - Crash Marker 3b.111\r\n", millis());

    udp.beginPacket(IP, udpPort);

  //  Serial.println("doing UDP WRITE");
  udp.write(UDPpacketBuffer, strlen(UDPpacketBuffer));

//  Serial1.println ( urlToLoad);
 // Serial1.println ( urlToLoad.length());
  
  DEBUG_SERIAL_UART_MAX("[%d] - Crash Marker 3c.111\r\n", millis());

  // need to stop udp packets being sent too quickly, system seems to crash if its done repeatively.
  // 


  
  
 // Serial1.println(millis());
  yield();
//  delay(100);
  udp.endPacket();

  DEBUG_SERIAL_UART_MAX("[%d] - Crash Marker 3d.2222\r\n", millis());

  
}



String readAllDataOnSerial( String TCPBuffer, bool &fullLine )
{


  size_t wlen=Serial.available();

  // For some reason if the incoming tcp buffer is too big it crashes the system, limit it to 512, we might miss a few lines now and then
  // but better than crashing the system, seems windows CE doesnt transmit data very well and stalls, kobo is fine :)
  
  if ( wlen > 512 )
  {
    Serial.flush();
    DEBUG_SERIAL_UART_MAX("[%d] - %%%%%%%% TCP data bytes available :- %d - Flushing readAllDataOnSerial Buffer\r\n", millis(),wlen);
    wlen=Serial.available();
    DEBUG_SERIAL_UART_MAX("[%d] - %%%%%%%% TCP data bytes available :- %d - AFTER Flushing readAllDataOnSerial Buffer\r\n", millis(),wlen);
    return "";
  }

  if (wlen!=0)
  {
  
    DEBUG_SERIAL_UART_MAX("[%d] - Serial data bytes available :- %d \r\n", millis(),wlen);
     
    last_data_received=millis();
    uint8_t sbuf[wlen];
    char cbuf[wlen];

     Serial.flush();
     Serial.readBytes(sbuf, wlen);



//    Serial.println("READ :>");
 //   Serial.write(sbuf, wlen);
 //   Serial.println("<<<<<<<");     
     
    DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 2 \r\n", millis());
     
    memset(cbuf, 0, wlen);

    DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 2a \r\n", millis());
     
    memcpy(&cbuf,&sbuf, wlen);
   
    DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 3 \r\n", millis());

   // TCPBuffer=TCPBuffer+String(cbuf).substring(0,wlen);// + "\r\n$PFLAU,6,1,2,1,0,144,0,235,446*\r\n";
   //TCPBuffer="START\r\n" + String(cbuf).substring(0,wlen) + "\r\nEND\r\n";
/*    Serial.println("\n");
    Serial.println("TCPBuffer:");
    Serial.print(">>>>>>>>>>");
Serial.print(TCPBuffer);
Serial.println("<<<<<<<<<");
Serial.println("incoming 0:");
Serial.print(">>>>>>>>>>");
Serial.print(String(cbuf).substring(0,wlen));
Serial.println("<<<<<<<<<");
*/

    TCPBuffer=TCPBuffer+String(cbuf).substring(0,wlen);

  //  delay(200);
 //   Serial1.flush();
 //   yieldDelay(100);

   //Serial1.print(String(cbuf).substring(0,wlen));

   
   String oS = String(cbuf).substring(0,wlen);
   String oSout=oS;
   
   int searchS=wlen;

   
   if ( (millis()-pflauSendTime) > pflauCounter )
   {
    addpflau=true;
    pflauCounter=millis();
   }
 

  // Serial.println("incoming string" + oS );

   
DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 4 \r\n", millis());
   
   if( addpflau || streamData!="")
   {
//    Serial.println ( "Adding FL");
 //   Serial.println("in  string > " + oS + " < " );
    
    for (int i=0;i<searchS;i++)
    {


   //   Serial.print(">>" );
    //  Serial.print(oS.charAt(i));
    //  Serial.println("<<" );
      
      if (oS.charAt(i)=='$')
      {
      //   Serial.println("Start" + oS.substring(0,i));
      //   Serial.println("End" + oS.substring(i));
         
         if( addpflau)
         {
         // pflaaPacket="\r\n\r\n" + pflaaPacket+"\r\n\r\n$PFLAU,6,1,2,1,0,144,0,235,446*55\r\n\r\n";
            streamData="$PFLAU,6,1,2,1,0,144,0,235,446*55\r\n"+streamData;
         }
         
         oSout=oS.substring(0,i)+streamData+oS.substring(i);
        // Serial.println("outgoing string out of Addition" + oSout);
      //   Serial.println("out  string > " + oSout + " < " );
         
      //   Serial1.flush();
       //  Serial1.print("\r\n"+oS.substring(0,i)+pflaaPacket+oS.substring(i));
         
         addpflau=false;
         streamData="";
         break;
         i=searchS;
      }
     }  
   }
   
//make use of both serial ports for speed.
    if ( wiredDirectly == 'y' )
    {
      DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 5 \r\n", millis());
      Serial1.flush();
      DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 5a \r\n", millis());
      Serial1.print(oSout);
    }
    
    if ( wiredDirectly == 'o' || wiredDirectly == 'v' || wiredDirectly == 'f')
    {
      DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 5 \r\n", millis());
      DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 5aaaaa \r\n", millis());
      delay(1);
      Serial.flush();
      DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 5a-1 \r\n", millis());
      delay(1);
      DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 5a \r\n", millis());
      Serial.print(oSout);
    }
    

  //  Serial.println("to kobo start");
 //  Serial.println(oSout);    
  //  Serial.println("to kobo end");

    
   //   Serial1.print(String(cbuf).substring(0,wlen));

  }
  else
  {
     DEBUG_SERIAL_UART_MAX("[%d] - No Data on socket at this point \r\n", millis());
  }

   DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 5b \r\n", millis());
  bool requiredNMEA=false;
  int i;
   DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 5c \r\n", millis());
  // Serial1.print("before with ");
   // Serial1.println(TCPBuffer.length());

// moving this here as it can come out early.

   fullLine=false;

//following if statement is because sometimes the next crashes the system when tcpbuffer=0, god knows why.....
  if ( TCPBuffer.length()==0)
  {
  //    Serial.println("out here 1");
   //   Serial.println(TCPBuffer);
    return "";
  }
    
  if ( TCPBuffer.length()<5)
  {
    //Serial1.print("returning with ");
  //  Serial1.println(TCPBuffer.length());
  //    Serial.println("out here 2 ");
   //   Serial.println(TCPBuffer);
    return TCPBuffer;
  }

DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 6 \r\n", millis());

  // make 100% sure length>0

  int len=TCPBuffer.length()-4;

  if (len<0)
  {
    len=0;
  }
  
  bool found_valid_nmea=false;

  for (i=0;i<len;i++)
  {
  
    if (TCPBuffer.charAt(i)=='$' && ( TCPBuffer.charAt(i+3)== 'G' || TCPBuffer.charAt(i+3)== 'R' ) && ( TCPBuffer.charAt(i+4)== 'G' || TCPBuffer.charAt(i+4)== 'M' ) )
    {
      found_valid_nmea=true;
      break;
    }
  }  

DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 7 \r\n", millis());

  // if it doesnt find a valid line it steps over the 4 digits we had to save for partial lines
  // coming in next time, ie $GGP - therefore we need to get from i for valid line and if its a partial
 // then i-1;

  if (found_valid_nmea && TCPBuffer.length()>0)
  {
    TCPBuffer= TCPBuffer.substring(i);
  }
  else
  {
    TCPBuffer= TCPBuffer.substring(i-1);
  }


  for (int i=0;i<TCPBuffer.length();i++)
  { 


    if (TCPBuffer.charAt(i)=='\n' && i >6)
    {
    //  Serial.println("Found end of Line" );
      fullLine=true;
      break;
    }
   
  }  
//  Serial.println("REturning :>");
 // Serial.println(TCPBuffer );
 // Serial.println("<<<<<<<");   
 /* Serial.println("\n");
  Serial.println("TCPBuffer REturning:");
  Serial.print(">>>>>>>>>>");
Serial.print(TCPBuffer);
Serial.println("<<<<<<<<<");
*/

  return TCPBuffer;
 }


String readAllDataOnWifiTxSerial( WiFiClient LK8000Client, String TCPBuffer, bool &fullLine )
{


  size_t wlen=Serial.available();

  // For some reason if the incoming tcp buffer is too big it crashes the system, limit it to 512, we might miss a few lines now and then
  // but better than crashing the system, seems windows CE doesnt transmit data very well and stalls, kobo is fine :)
  
  if ( wlen > 512 )
  {
    Serial.flush();
    DEBUG_SERIAL_UART_MAX("[%d] - %%%%%%%% TCP data bytes available :- %d - Flushing readAllDataOnSerial Buffer\r\n", millis(),wlen);
    wlen=Serial.available();
    DEBUG_SERIAL_UART_MAX("[%d] - %%%%%%%% TCP data bytes available :- %d - AFTER Flushing readAllDataOnSerial Buffer\r\n", millis(),wlen);
    return "";
  }

  if (wlen!=0)
  {
  
    DEBUG_SERIAL_UART_MAX("[%d] - Serial data bytes available :- %d \r\n", millis(),wlen);
     
    last_data_received=millis();
    uint8_t sbuf[wlen];
    char cbuf[wlen];
    

    Serial.readBytes(sbuf, wlen);        
     
    DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 2 \r\n", millis());
     
    memset(cbuf, 0, wlen);

    DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 2a \r\n", millis());
     
    memcpy(&cbuf,&sbuf, wlen);
   
    DEBUG_SERIAL_UART_MAX("[%d] - In  readAllDataOnSerial Point 3 \r\n", millis());

   // TCPBuffer=TCPBuffer+String(cbuf).substring(0,wlen);// + "\r\n$PFLAU,6,1,2,1,0,144,0,235,446*\r\n";
   //TCPBuffer="START\r\n" + String(cbuf).substring(0,wlen) + "\r\nEND\r\n";
   
    TCPBuffer=TCPBuffer+String(cbuf).substring(0,wlen);
  //  delay(200);
 //   Serial1.flush();
 //   yieldDelay(100);

   //Serial1.print(String(cbuf).substring(0,wlen));

   
   String oS = String(cbuf).substring(0,wlen);
   String oSout=oS;
   
   int searchS=wlen;

   
   if ( (millis()-pflauSendTime) > pflauCounter )
   {
    addpflau=true;
    pflauCounter=millis();
   }
 

  // Serial.println("incoming string" + oS );

   

   
   if( addpflau || streamData!="")
   {
//    Serial.println ( "Adding FL");
 //   Serial.println("in  string > " + oS + " < " );
    
    for (int i=0;i<searchS;i++)
    {


   //   Serial.print(">>" );
    //  Serial.print(oS.charAt(i));
    //  Serial.println("<<" );
      
      if (oS.charAt(i)=='$')
      {
      //   Serial.println("Start" + oS.substring(0,i));
      //   Serial.println("End" + oS.substring(i));
         
         if( addpflau)
         {
         // pflaaPacket="\r\n\r\n" + pflaaPacket+"\r\n\r\n$PFLAU,6,1,2,1,0,144,0,235,446*55\r\n\r\n";
            streamData=streamData+"$PFLAU,6,1,2,1,0,144,0,235,446*55\r\n";
         }
         
         oSout=oS.substring(0,i)+streamData+oS.substring(i);
        // Serial.println("outgoing string out of Addition" + oSout);
      //   Serial.println("out  string > " + oSout + " < " );
         
      //   Serial1.flush();
       //  Serial1.print("\r\n"+oS.substring(0,i)+pflaaPacket+oS.substring(i));
         
         addpflau=false;
         streamData="";
         break;
         i=searchS;
      }
     }  
   }
    
  //  Serial1.flush();
  //  Serial1.print(oSout);
   //   Serial1.print(String(cbuf).substring(0,wlen));
   LK8000Client.flush();
   LK8000Client.print(oSout);
 //  Serial.print(oSout);

  }
  else
  {
     DEBUG_SERIAL_UART_MAX("[%d] - No Data on socket at this point \r\n", millis());
  }

   
  bool requiredNMEA=false;
  int i;

   
  if ( TCPBuffer.length()<5)
  {
    return TCPBuffer;
  }

  // make 100% sure length>0

  int len=TCPBuffer.length()-4;

  if (len<0)
  {
    len=0;
  }
  
  for (i=0;i<len;i++)
  {
  
    if (TCPBuffer.charAt(i)=='$' && ( TCPBuffer.charAt(i+3)== 'G' || TCPBuffer.charAt(i+3)== 'R' ) && ( TCPBuffer.charAt(i+4)== 'G' || TCPBuffer.charAt(i+4)== 'M' ) )
    {
     /// requiredNMEA=true;
      break;
    }
  }  

  TCPBuffer= TCPBuffer.substring(i);

  fullLine=false;
  
  for (int i=0;i<TCPBuffer.length();i++)
  { 


    if (TCPBuffer.charAt(i)=='\n')
    {
   //   Serial.println("Found end of Line");
      fullLine=true;
      break;
    }
   
  }  
 // Serial.println("returning " + TCPBuffer );
  return TCPBuffer;
 }

 /*bool nmea_cksum(String data )
 {
  char *csum_ptr;
  char UDPpacketBuffer[96];
//   char newLine='\r';
  int p=data.length();

  unsigned char cs = 0;
  data.toCharArray(UDPpacketBuffer,p+1);

  cs = 0; //clear any old checksum
  for (unsigned int n = 1; n < strlen(UDPpacketBuffer) - 1; n++)
  {
    cs ^= UDPpacketBuffer[n]; //calculates the checksum
  }

  csum_ptr = UDPpacketBuffer + strlen(UDPpacketBuffer);
  snprintf(csum_ptr, sizeof(UDPpacketBuffer) - strlen(UDPpacketBuffer), "%02X\r\n", cs);

  Serial1.println(UDPpacketBuffer);
  Serial1.println(csum_ptr);

  return true;
 }*/

bool cksum(String data)
{


   // if theres a carriage return on the end of the line then remove.

//	Serial.print(">");Serial.print(data);Serial.println("<");
//	Serial.print(">");Serial.print(data.substring(data.length()));Serial.println("<");
//	Serial.print(">");Serial.print(data.substring(data.length(),data.length()));Serial.println("<");
//	Serial.print(">");Serial.print(data.substring(data.length()-1,data.length()));Serial.println("<");
//	Serial.print(">");Serial.print(data.substring(data.length()-2,data.length()));Serial.println("<");

   if (data.substring(data.length()-1,data.length())=="\n" || data.substring(data.length()-1,data.length())=="\r" || data.substring(data.length()-1,data.length())==" ")
   {
	// Serial.println("found a return ! ");
	 data=data.substring(0,data.length()-1);
	// Serial.print(">");Serial.print(data);Serial.println("<");
   }


   char UDPpacketBuffer[150];

   int p=data.length();

   unsigned char cs = 0;
   data.toCharArray(UDPpacketBuffer,p+1);


   //start at 1 to jump the $ and  remove the 3 digits at the end for the cksum.

   cs = 0; //clear any old checksum

   unsigned int c=0;

   char cksum_value[3];

   for (unsigned int n = 1; n < strlen(UDPpacketBuffer); n++)
   {
	   if (n < strlen(UDPpacketBuffer) - 3)
	   {
		   //Serial.print("Before:");
	//   Serial.println(UDPpacketBuffer[n]);
         cs ^= UDPpacketBuffer[n]; //calculates the checksum
	   }
	   else
	   {
		 if (n != (strlen(UDPpacketBuffer) -3 )) // on the *
	     {
			 // check its a digit - if not line is false;
		   cksum_value[c]=UDPpacketBuffer[n];

		   if (!isalnum (cksum_value[c]))
		   {
			  // Serial.println("fail digit");
			   return false;
		   }
		 //  Serial.print("After:");
		 //  Serial.println(UDPpacketBuffer[n]);
		   c++;
	     }
	   }

   }

  // Serial1.println("cs");Serial1.println(cs);

   cksum_value[2]='\0';
   //unsigned char number = (char)strtol(cksum_value, NULL, 16);

   if (cs==(char)strtol(cksum_value, NULL, 16))
   {
	   return true;
   }
   else
   {
	  // Serial1.println("fail cksum");
	   return false;
   }

}
