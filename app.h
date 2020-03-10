/*
 * app.h
 *
 *  Created on: 17 Oct 2016
 *      Author: sid
 */

#ifndef STACK_APP_H_
#define STACK_APP_H_

#include <math.h>

#include "fanet.h"
//#include "com/serial.h"
#include "fanet_stack/fmac.h"
#include "AirWare.h"
#include "fanet_stack/payload.h"

#define APP_VALID_STATE_MS			10000
//#define FANET_MANUFACTURER 4


//todo... worst case coding...
#define APP_TYPE1_AIRTIME_MS			40		//actually 20-25ms
//MINTAU is the minimum time before it tries a retransmission (in case the channel was busy)
#define	APP_TYPE1_MINTAU_MS			250
//Time between transmissions
#define	APP_TYPE1_TAU_MS			2000
//_SIZE is simply the payload size of a type 1 packet
#define APP_TYPE1_SIZE				12

//extern bool framesArrayFree[RX_PACKETS];
//extern Frame *frameData[RX_PACKETS];

//extern Payload payloadList[RX_PACKETS];
//extern int payloadReady[RX_PACKETS];


extern Payload payloadList[RX_PACKETS];
extern int payloadReady[RX_PACKETS];
extern long int payload_time_to_send[RX_PACKETS];
extern char packet_repeat;



extern IPAddress apFoundIP;
extern char groundStationMode;

class App : public Fapp
{
private:
	/* units are degrees, seconds, and meter */
	float latitude = NAN;
	float longitude = NAN;
	int altitude;
	float speed;
	float climb;
	float heading;
	float turnrate;

	//Serial_Interface *mySerialInt = NULL;

	/* ensures the broadcasted information are still valid */
	unsigned long valid_until = 0;

	/* determines the tx rate */
	unsigned long last_tx = 0;
	unsigned long next_tx = 0;

#ifdef FANET_NAME_AUTOBRDCAST
	char name[20] = "\0";
	bool brdcast_name = false;

	int serialize_name(uint8_t*& buffer)
	{
		const int namelength = strlen(name);
		buffer = new uint8_t[namelength];
		memcpy(buffer, name, namelength);
		return namelength;
	}
#endif
//v16
    void buf_absolut2coord(float &lat, float &lon, uint8_t *buf)
    {
      int32_t lat_i = 0;
      int32_t lon_i = 0;

      if(buf == NULL || lat == NULL || lon == NULL)
        return;

      ((uint8_t*)&lat_i)[0] = buf[0];
      ((uint8_t*)&lat_i)[1] = buf[1];
      ((uint8_t*)&lat_i)[2] = buf[2];

      ((uint8_t*)&lon_i)[0] = buf[3];
      ((uint8_t*)&lon_i)[1] = buf[4];
      ((uint8_t*)&lon_i)[2] = buf[5];

      lat = (float) lat_i / 93206.0f;
      lon = (float) lon_i / 46603.0f;


    }
	int serialize_tracking(uint8_t*& buffer)
	{
		buffer = new uint8_t[APP_TYPE1_SIZE];

		Frame::coord2payload_absolut(latitude, longitude, buffer);

		/* altitude set the lower 12bit */
		int alt = constrain(altitude, 0, 8190);
		if(alt > 2047)
			((uint16_t*)buffer)[3] = ((alt+2)/4) | (1<<11);				//set scale factor
		else
			((uint16_t*)buffer)[3] = alt;
		/* online tracking */
		((uint16_t*)buffer)[3] |= !!do_online_tracking<<15;
		/* aircraft type */
		((uint16_t*)buffer)[3] |= (aircraft_type&0x7)<<12;

		/* Speed */
		int speed2 = constrain((int)roundf(speed*2.0f), 0, 635);
		if(speed2 > 127)
			buffer[8] = ((speed2+2)/5) | (1<<7);					//set scale factor
		else
			buffer[8] = speed2;

		/* Climb */
		int climb10 = constrain((int)roundf(climb*10.0f), -315, 315);
	   if(abs(climb10) > 63)
	   		buffer[9] = ((climb10 + (climb10>=0?2:-2))/5) | (1<<7);			//set scale factor
		else
			buffer[9] = climb10 & 0x7F;

		/* Heading */

		buffer[10] = constrain((int)roundf(heading*256.0f)/360.0f, 0, 255);

		/* Turn rate */
		if(!isnan(turnrate))
		{
			int turnr4 = constrain((int)roundf(turnrate*4.0f), 0, 255);
			if(abs(turnr4) > 63)
				buffer[9] = ((turnr4 + (turnr4>=0?2:-2))/4) | (1<<7);			//set scale factor
			else
				buffer[9] = turnr4 & 0x7f;
			return APP_TYPE1_SIZE;
		}
		else
		{
			return APP_TYPE1_SIZE - 1;
		}
	}

public:
	int aircraft_type;
	bool do_online_tracking;
	bool gpslocked=false;

	void set(float lat, float lon, float alt, float speed, float climb, float heading, float turn)
	{

		latitude = lat;
		longitude = lon;
		altitude = roundf(alt);
		this->speed = speed;
		this->climb = climb;
		if(heading < 0.0f)
			heading += 360.0f;
		this->heading = heading;
		turnrate = turn;

		valid_until = millis() + APP_VALID_STATE_MS;

		if (longitude > -2 || longitude < -3)
			       {
		                  //Serial1.print("lat->");Serial1.print(lat);Serial1.print("<-");
				           //Serial1.print("lon->");Serial1.print(lon);Serial1.println("<-");
		//	Serial1.println("###############################################");

			       }

		//interrupts();
	}

	void set_gps_lock(char gpslockchar)
	{

      if ( gpslockchar == 'A')
      {
    	  gpslocked=true;
      }
      else
      {
    	  gpslocked=false;
	  }

	  return;
	}

	/* device -> air */
	bool is_broadcast_ready(int num_neighbors)
	{

		/* if we are a ground station then dont transmit */
	    if (groundStationMode == 'y')
        {
	    	return false;
        }


		/* is the state valid? */
		if(millis() > valid_until || isnan(latitude) || isnan(longitude) || !gpslocked)
			return false;

		/* in case of a busy channel, ensure that frames from the fifo get also a change */
		if(next_tx > millis())
			return false;

		/* determine if its time to send something (again) */
		const int tau_add = (num_neighbors/10 + 1) * APP_TYPE1_TAU_MS;
		if(last_tx + tau_add > millis())
			return false;

		return true;
	}

	void broadcast_successful(int type)
	{
		last_tx = millis();

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
		            if (navSofware == 'X' )
		            {
		              sendUDPPacket ( "$AIRWHERE,TRANSMITTING-LOCATION,*", apFoundIP );
		            }
		          }

 }


	Frame *get_frame()
	{
		/* prepare frame */
		Frame *frm = new Frame(fmac.my_addr);
#ifdef FANET_NAME_AUTOBRDCAST
		static uint32_t framecount = 0;
		if(brdcast_name && (framecount & 0x7F) == 0)
		{
			/* broadcast name */
			frm->type = FRM_TYPE_NAME;
			frm->payload_length = serialize_name(frm->payload);
		}
		else
		{
#endif
			/* broadcast tracking information */
			frm->type = FRM_TYPE_TRACKING;
			frm->payload_length = serialize_tracking(frm->payload);

#ifdef FANET_NAME_AUTOBRDCAST
		}
		framecount++;
#endif

		/* in case of a busy channel, ensure that frames from the fifo gets also a change */
		next_tx = millis() + APP_TYPE1_MINTAU_MS;

		return frm;
	}

#ifdef FANET_NAME_AUTOBRDCAST
	/* Name automation in case the host application does not know this... */
	void set_name(char *devname) { snprintf(name, sizeof(name), devname); };
	void allow_brdcast_name(boolean value)
	{
		if(value == false)
			brdcast_name = false;
		else
			brdcast_name = (strlen(name)==0?false:true);
	};
#endif

	void begin()
	{
		//mySerialInt = &si;
	}

	//void begin(Serial_Interface &si)
	//{
		//mySerialInt = &si;
	//}

	/* air -> device */
	void handle_acked(boolean ack, MacAddr &addr)
	{
		SerialDEBUG.println("ack_frame");
		//if(mySerialInt == NULL)
		//	return;

		//mySerialInt->handle_acked(ack, addr);
	}

	void handle_frame(Frame *frm)
	{
#if defined(SerialDEBUG) && MAC_debug_mode > 0

		SerialDEBUG.print(millis());
		SerialDEBUG.print ("### Handling frame :- ");
		/* src_manufacturer,src_id,broadcast,signature,type,payloadlength,payload */
		SerialDEBUG.print(frm->src.manufacturer, HEX);
		SerialDEBUG.print(',');
		// needs sending as HEX?!
		SerialDEBUG.print(frm->src.id, DEC);
		SerialDEBUG.print(',');
		SerialDEBUG.print(frm->dest == MacAddr());	//broadcast
		SerialDEBUG.print(',');
		SerialDEBUG.print(frm->signature, HEX);
		SerialDEBUG.print(',');
		SerialDEBUG.print(frm->type, HEX);
		SerialDEBUG.print(',');
		SerialDEBUG.print(frm->payload_length, HEX);
		SerialDEBUG.print(',');
		for(int i=0; i<frm->payload_length; i++)
		{
			char buf[8];
			sprintf(buf, "%02X", frm->payload[i]);
			SerialDEBUG.print(buf);
		}

		SerialDEBUG.println();
		SerialDEBUG.flush();
#endif

				switch (frm->type)
				{
				  //ACK (Type = 0)
				  case 0:
			        break;
			      case 1:
			      //Tracking (Type = 1)
			      // only build parse and process incoming message if we have lock

			     ///   if (gpslocked) {Serial.println("gpslocked"); }else {Serial.println("gps not locked"); }
			     //   if (isnan(latitude)) {Serial.println("isnan(latitude"); }else {Serial.println("isnan(latitude"); }



				    //if (gpslocked && !isnan(latitude) && !isnan(longitude))
			          if (1)
			        {
					  Payload payload(frm->src.manufacturer,
					  frm->src.id,
					  frm->dest.id,
					  frm->signature,
					  frm->type,
				      frm->rssi,
					  frm->payload_length,
					  frm->payload,
					  latitude,
					  longitude);



					/*  Serial.print("payload id ");Serial.println(payload.id);
					  Serial.print("payload manu ");Serial.println(payload.manufacturer, HEX);
					  Serial.print("payload lon x1000 ");  Serial.println(payload.longitude*1000);
					  Serial.print("payload lat x1000 ");  Serial.println(payload.latitude*1000);
					  Serial.print("payload aircraft type ");Serial.println(payload.aircraft_type);
				      Serial.print("altitude ");Serial.println(payload.altitude);
				      Serial.print("speed ");Serial.println(payload.speed);
				      Serial.print("climb ");Serial.println(payload.climb);
					  Serial.print("heading ");Serial.println(payload.heading);



				      for ( int packetNumber=0; packetNumber <  RX_PACKETS; packetNumber++)
					  {
					  // if its not ready there a free space in the array
					    if (!payloadReady[packetNumber] )
						{
						  payloadList[packetNumber]=payload;
						  payloadReady[packetNumber]=true;
						  break;
						}
					  }
*/


				      //v17


		              if ( packet_repeat=='y')
		              {

		                  // if its not ready there a free space in the array


		                  // first search array, see if we already have this pilot in the array
		                  // then if we do, update this pilots details and set to repeat
		                  // if not, start at the start again and find the first 0 and add into array.

		                  bool pilot_added=false;

		                  for ( int packetNumber=0; packetNumber <  RX_PACKETS; packetNumber++)
		                  {
		                      if (payloadList[packetNumber].manufacturer==payload.manufacturer &&
		                              payloadList[packetNumber].id==payload.id )
		                      {
		                       //   Serial.println("Adding into array - pilot found");

		                          payloadList[packetNumber]=payload;
		                          payloadReady[packetNumber]=PACKET_REPEAT_NUMBER;
		                          // send immediately.
		                          payload_time_to_send[packetNumber]=0;
		                          pilot_added=true;
		                      }
		                  }

		                  if (!pilot_added)
		                  {
		                      for ( int packetNumber=0; packetNumber <  RX_PACKETS; packetNumber++)
		                      {
		                          if (payloadReady[packetNumber]==0 )
		                          {
		                          //    Serial.println("Adding into array - new pilot");
		                              payloadList[packetNumber]=payload;
		                              payloadReady[packetNumber]=PACKET_REPEAT_NUMBER;
		                              // send immediately.
		                              payload_time_to_send[packetNumber]=0;
		                              break;
		                          }
		                      }
		                  }
		              }
		              else
		              {
		                  for ( int packetNumber=0; packetNumber <  RX_PACKETS; packetNumber++)
		                  {
		                      if (payloadReady[packetNumber]==0 )
		                      {
		                          //    Serial.println("Adding into array");
		                          payloadList[packetNumber]=payload;
		                          payloadReady[packetNumber]=1;
		                          payload_time_to_send[packetNumber]=0;
		                          break;
		                      }
		                  }
		              }
				   }

				    break;

				  case 2:
				     // SerialDEBUG.println("Received Name");
     			  //Name (Type = 2)
					  //Add Support for Name into LK&XCSoar and then complete following to suit.
					  //Payload payload(frm->src.manufacturer, frm->src.id, frm->payload_length,frm->payload);
				  	break;
				  case 3:
				  //Message (Type = 3)
				  	break;
				  case 4:
				  //Service (Type = 4)
				  	break;
				  case 5:
				  //Landmarks (Type = 5)
				  	break;
				}



	}
};


//extern App app;

#endif /* STACK_APP_H_ */
