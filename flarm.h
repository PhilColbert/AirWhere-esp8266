/*
 * flarm.h
 *
 *  Created on: 15 Dec 2017
 *      Author: Fugazi
 */

#ifndef FLARM_H_
#define FLARM_H_

#include <math.h>
#include "AirWare.h"
#include "fanet_stack/payload.h"
#include "fanet.h"
#include "WebHelper.h"


#define FLARM_SERIAL_WAIT 250
#define FLARM_BOOT_TIME 5000
#define FLARM_ERROR 2000
//#define FANET_MANUFACTURER 4
#define FLARM_BOARD_RESET_PIN 5
#define PPS_PIN 4
#define FLARM_BOARD_DATA_FREQ 10000

//extern Payload payloadList[RX_PACKETS];
//extern bool payloadReady[RX_PACKETS];

extern Payload payloadList[RX_PACKETS];
extern int payloadReady[RX_PACKETS];
extern long int payload_time_to_send[RX_PACKETS];
extern char packet_repeat;
extern char web_aircraft_type;

extern bool flarm_needs_reset;
unsigned int last_data_time;
extern bool wifiUp;

bool pps_set=false;

class Flarm
{
private:

    String serial_buffer;
    /* units are degrees, seconds, and meter */
    float latitude = NAN;
    float longitude = NAN;
    int altitude;
    float speed;
    float climb;
    float heading;
    float turnrate;

    String year;
    String month;
    String day;
    String hour;
    String min;
    String sec;

    MacAddr src;
    MacAddr dest;


    int signature = 0;

    int type = 0;
    int payload_length = 0;
    int payload_start = 0;
    uint8_t *payload = NULL;

    unsigned int update_time = 0;



public:
    int aircraft_type;
    bool do_online_tracking;
    bool gpslocked=false;

    int manufacturer;
    int id;

    void reset()
    {
       //Serial.println("Resetting board");
        pinMode(FLARM_BOARD_RESET_PIN, OUTPUT);
        digitalWrite(FLARM_BOARD_RESET_PIN, LOW);
        delay(500);
        digitalWrite(FLARM_BOARD_RESET_PIN, HIGH);


    }


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
    }

    void set(String gps_time, String gps_date, float lat, float lon, float alt, float speed, float climb, float heading)
    {

        latitude = lat;
        longitude = lon;
        altitude = roundf(alt);
        this->speed = speed;
        this->climb = climb;
        this->heading = heading;

        day=gps_date.substring(0,2);
        month=gps_date.substring(2,4);
        year="1"+gps_date.substring(4,6);

        hour=gps_time.substring(0,2);
        min=gps_time.substring(2,4);
        sec=gps_time.substring(4,6);

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

    //void update_flarm(SoftwareSerial &flarmSerial)
    void update_flarm()
    {

        if (!gpslocked || isnan(latitude) || isnan(longitude))
        {
            return;
        }

        if (!pps_set)
        {
           return;
        }
        else
        {
            pps_set=false;
        }



        String update_message="";

        int month_tm=month.toInt();
        month_tm--;

           update_message=("#FNS "+
                          String(latitude,5) + "," +
                          String(longitude,5) + "," +
                          String(altitude) + "," +
                          String(speed) + "," +
                          String(climb) + "," +
                          String(heading) + "," +
                          String(year) + "," +
                          String(month_tm) + "," +
                          String(day) + "," +
                          String(hour) + "," +
                          String(min) + "," +
                          String(sec) +
                          "\n");

         //  Serial.println(update_message);

           Serial1.print(update_message);

        return;
    }

    static void pps_interrupt()
    {
       pps_set=true;
    }

   // bool begin(SoftwareSerial &flarmSerial, char *manufacturer_code, char *flarm_board_id, char *flarm_board_expiration, char *flarm_board_build,
     bool begin(SoftwareSerial &Readflarm,HardwareSerial &flarmSerial, char *manufacturer_code, char *flarm_board_id, char *flarm_board_expiration, char *flarm_board_build,
               bool &flarm_tracking_on, bool &fanet_tracking_on)
    {

        size_t slen=0;

        pinMode(PPS_PIN, INPUT);
        attachInterrupt(PPS_PIN, this->pps_interrupt, FALLING);

        Serial.print("Searching for Flarm radio board.");

       // delay(10000);
     //   delay(FLARM_SERIAL_WAIT);
      //  delay(FLARM_SERIAL_WAIT);
    //    delay(FLARM_SERIAL_WAIT);
      //  delay(FLARM_SERIAL_WAIT);

        // first wait for the flarm board to boot - checking for the module ID - if we cant find it then no board.
        // if we can find it then continue on gettting all the parameters from the board.
        // work as async transfer for now, assume full line coming back as we are adding a delay
        String flarm_data_in="";

        bool found_dgv=false;
        for (int i=0;i<2;i++)
        {
            delay(FLARM_SERIAL_WAIT);
            flarmSerial.print("\n");
            Readflarm.readString();
            Readflarm.flush();
            delay(FLARM_SERIAL_WAIT);
            if  (wifiUp)
            {
              Web_loop();
            }
        }

        long int wait=millis()+1;

        while ( millis() < wait)
        {
            if  (wifiUp)
            {
                Web_loop();
            }
        }

        unsigned int milli_end=millis()+FLARM_BOOT_TIME;

        do
        {

            flarmSerial.print("#DGV\n");
            delay(FLARM_SERIAL_WAIT);

            slen=Readflarm.available();

            Serial.print(".");
            if  (wifiUp)
            {
                Web_loop();
            }

            if (slen!=0)
            {
               flarm_data_in=Readflarm.readString();

               // Serial.println(flarm_data_in);

                if (flarm_data_in.substring(0,4)=="#DGV")
                {
                   flarm_data_in.substring(11).toCharArray(flarm_board_build,15);
                   found_dgv=true;
                }
                else
                {
                    // for some reason this statement below, makes it work, no idea why? look into it!
                   // SerialGPS.print(".............");
                    Readflarm.readString();
                    Readflarm.flush();
                    while(Readflarm.available()>0) Readflarm.read();
                    delay(FLARM_SERIAL_WAIT);
                    if  (wifiUp)
                    {
                        Web_loop();
                    }
                  //flarmSerial.print("\n");
                }
            }
        }
        while (milli_end>millis() && !found_dgv);


        if (flarm_board_build[0]=='\0')
        {
            snprintf(flarm_board_expiration,14,"Not Connected");
            return false;
        }
        delay(FLARM_SERIAL_WAIT);
    //    Serial.print("#FNA\n");
        flarmSerial.print("#FNA\n");
        delay(FLARM_SERIAL_WAIT);
        String fna=Readflarm.readString();
     //  Serial.println(fna);
        Readflarm.flush();

       Serial.print(".");

       fna.substring(5).toCharArray(manufacturer_code,3);
       this->manufacturer = strtol( manufacturer_code, 0, 16);

       //bodge until we sort lk for FF manufacturers.
       // skytraxx is 0x11 (17) and 0x1

    //   if (this->manufacturer==17)
    //   {
    //       this->manufacturer=1;
     //  }
    //    Serial.println(manufacturer_code);
       fna.substring(8).toCharArray(flarm_board_id,5);
       this->id = strtol( flarm_board_id, 0, 16);
     //  Serial.println(flarm_board_id);
       Readflarm.flush();

       Serial.print(".");

     //  Serial.print("#FAX\n");
        flarmSerial.print("#FAX\n");
        delay(FLARM_SERIAL_WAIT);
       // flarmSerial.readString().substring(5).toCharArray(flarm_board_expiration,9);

        int comma_pos=0;
        String fbe=Readflarm.readString().substring(5);
        Serial.print("Expires:");

        String f_year=fbe.substring(0,fbe.indexOf(',', comma_pos));
        comma_pos=fbe.indexOf(',', comma_pos);
        comma_pos++;

        String f_month=fbe.substring(comma_pos,fbe.indexOf(',', comma_pos));
        comma_pos=fbe.indexOf(',', comma_pos);
        comma_pos++;

        String f_day=fbe.substring(comma_pos,fbe.indexOf(',', comma_pos));
        comma_pos=fbe.indexOf(',', comma_pos);
        comma_pos++;

        snprintf(flarm_board_expiration,13,"%d-%d-%d",f_day.toInt(), f_month.toInt()+1, f_year.toInt()+1900);
        Serial.print(flarm_board_expiration);

      flarmSerial.flush();

      Serial.print(".");

      //  char flarm_ok[3]="";
     //   Serial.print("#FNC 1,1\n");

        String fnc="#FNC " + String(web_aircraft_type) + ",1\n";
//Serial.println(fnc);

//flarmSerial.print("#FNC "1,1\n");
        flarmSerial.print(fnc);
        delay(FLARM_SERIAL_WAIT);
 //       Serial.println(flarmSerial.readString());

       // flarmSerial.readString().substring(5).toCharArray(flarm_ok,3);
     //   Serial.println("#fnc");
    //   Serial.println(flarm_ok);
       flarmSerial.flush();

       Serial.print(".");

        flarmSerial.print("#DGP 1\n");
        delay(FLARM_SERIAL_WAIT);
        if (Readflarm.readString().substring(5,7)=="OK")
        {
            fanet_tracking_on=true;
        }
        else
        {
            fanet_tracking_on=false;
        }
        flarmSerial.flush();

        Serial.print(".");

         flarmSerial.print("#DGL 868,20\n");
         delay(FLARM_SERIAL_WAIT);
         String dgl=Readflarm.readString();
         if (dgl.substring(5,7)=="OK")
         {
           //  Serial.println("power is set to 20dbm");
           //  Serial.println(dgl);
             //fanet_tracking_on=true;
         }
         else
         {
           //  Serial.println("problem with power setting");
          //   Serial.println(dgl);
           // fanet_tracking_on=false;
         }
         flarmSerial.flush();

        Serial.print(".");

    //    Serial.print("#FAP 1\n");
        flarmSerial.print("#FAP 1\n");
        delay(FLARM_SERIAL_WAIT);
        if (Readflarm.readString().substring(5,7)=="OK")
        {
            flarm_tracking_on=true;
        }
        else
        {
            flarm_tracking_on=false;
        }
        flarmSerial.flush();
        Serial.print(".");

        last_data_time=millis()+FLARM_BOARD_DATA_FREQ;

        return true;

    }

//    void clear_input_stream(SoftwareSerial &flarmSerial, int &starting_char, int &current_char)
     void clear_input_stream(SoftwareSerial &flarmSerial, int &starting_char, int &current_char)
    {
        starting_char=0;
        current_char=0;
        serial_buffer="";
        flarmSerial.readString();
        flarmSerial.flush();
        update_time=millis()+FLARM_ERROR;
    }

  //  void clear_input_stream(SoftwareSerial &flarmSerial)
     void clear_input_stream(SoftwareSerial &flarmSerial)
    {
        serial_buffer="";
        flarmSerial.readString();
        flarmSerial.flush();
        update_time=millis()+FLARM_ERROR;
    }

    //void handle(SoftwareSerial &flarmSerial)
    void handle(HardwareSerial &flarmSerial, SoftwareSerial &Readflarm)
    {
        // first check we are getting data from the flarm board - if not reset and try again.
        // if after 10 seconds nothing as come back theres a problem so try reset and see if that fixes.
        // data only comes from the flarm board when the gps is locked.

        if (last_data_time<millis() && gpslocked)
        {
            clear_input_stream(Readflarm);
            flarm_needs_reset=true;
            return;

        }

        // first pull all data from flarm serial and add to serial_buffer - just in case serial is read in mid transaction.
        int slen=Readflarm.available();


        if (slen!=0)
        {
            last_data_time=millis()+FLARM_BOARD_DATA_FREQ;


           /*      Serial.println("serial_buffer in :- ");
                 Serial.println(serial_buffer);
                 Serial.println("slen");
                 Serial.println(slen);

*/
            serial_buffer=serial_buffer+Readflarm.readString();

       //     Serial.print("serial_buffer with flarm string :- ");
          //                Serial.println(serial_buffer);


            // next process this buffer, take every line into thepackets array which will be dealt with in the main loop.
            int current_char=0, starting_char=0;
            String wbuf;

            for (unsigned int i=0;i<serial_buffer.length();i++)
            {
                current_char++;

                if (serial_buffer.substring(i,i+1)=="\n")
                {
                 //   Serial.print("\nFound:-");
                 //   Serial.println(serial_buffer);

                    if (serial_buffer.substring(0,4)=="#FNR")
                    {
                       // Serial.print("Handling FNR:-");
                      //  Serial.println(serial_buffer.substring(starting_char+5, current_char));

                        // if its not OK, send a \n to reset.

                     //   Serial.println(serial_buffer.substring(5,9));



                        if (serial_buffer.substring(5,10)=="ERR,5")
                        {

                           // Serial.print("Flarm board problem - sending CR to reset ( output from flarm :- ");
                          //  Serial.print(serial_buffer.substring(starting_char, current_char));
                          //  Serial.println(")");
                           // flarmSerial.print("\n");
                            //clear_input_stream(flarmSerial, starting_char,current_char);
                            clear_input_stream(Readflarm, starting_char,current_char);
                            flarm_needs_reset=true;
                        }
                        if (serial_buffer.substring(5,11)=="MSG,13")
                        {

                           // Serial.print("Flarm board problem - sending CR to reset ( output from flarm :- ");
                          //  Serial.print(serial_buffer.substring(starting_char, current_char));
                          //  Serial.println(")");
                           // flarmSerial.print("\n");
                            //clear_input_stream(flarmSerial, starting_char,current_char);
                            clear_input_stream(Readflarm, starting_char,current_char);
                            flarm_needs_reset=true;
                        }
                        starting_char=i+1;
                        current_char=i+1;
                    }
                    else
                    {
                        if (serial_buffer.substring(0,4)=="#FNF")
                        {
                            // 5 digits to jump over the #FNF'
                          //  Serial.print("Handling :-");
                       //     Serial.println(serial_buffer.substring(starting_char+5, current_char));
                            bool valid_chars=true;
                            //Serial.println(serial_buffer);
                            for ( int weird=0;weird<serial_buffer.length();weird++)
                            {
                               char c=serial_buffer.charAt(weird);
                               int c_i=(int)c;

                               if ((c_i>47 && c_i<58) || (c_i>64 && c_i<91) || c_i==44 || c_i==32 || c_i==35 || c_i==10 )
                               {

                               }
                               else
                               {
                             //     Serial.print("corrupted char ");
                             //     Serial.println(c);
                              //    Serial.println(c_i);
                                  valid_chars=false;
                                  break;
                               }
                            }

                            if (valid_chars)
                            {
                                handle_frame(serial_buffer.substring(starting_char+5, current_char));
                            }
                            // jump over the /n
                            starting_char=i+1;
                            current_char=i+1;
                        }
                        else
                        { // found a CR but cant find what it is so must be weird chars so clear down.
                            Serial.print("weird  chars in stream:-");
                            Serial.println(serial_buffer.substring(starting_char, current_char));
                            starting_char=1;
                            //clear the lot down to reset.
                            clear_input_stream(Readflarm, starting_char,current_char);
                        }
                    }
                }


            }

        /*    Serial.println("");
            Serial.print("starting_char");
            Serial.println(starting_char);
            Serial.print("current_char");
            Serial.println(current_char);

            Serial.print("serial_buffer.substring(starting_char,current_char)");
            Serial.println(serial_buffer.substring(starting_char,current_char));

*/

            serial_buffer=serial_buffer.substring(starting_char,current_char);

       //     Serial.println("serial_buffer out :- ");
      //      Serial.println(serial_buffer);
        }

        // then update flarmboard with our location variables.

      //  update_flarm(flarmSerial);
        update_flarm();

    }


    void handle_frame(String buffer)
    {

        char buf[buffer.length()+1];

        // replace , with spaces to deal with later.

        buffer.replace(',', ' ');

        buffer.toCharArray(buf, buffer.length());

        long int manu, id, bcast, sig, type, len;
        char * pEnd;

        manu = strtol( buf, &pEnd, 16);

        //check for corruption - only allow manus from 01-30 for now, edit if this changes.

        if (manu<0 || manu>30)
        {
            return;
        }

        id = strtol( pEnd, &pEnd, 16);
        bcast= strtol( pEnd, &pEnd, 16);
        sig = strtol( pEnd, &pEnd, 16);
        type = strtol( pEnd, &pEnd, 16);
        len = strtol( pEnd, &pEnd, 16);

        //check for corruption - only allow len from 01-60 for now, edit if this changes.

        if (len<0 || len>60)
        {
            return;
        }

        char payload[len*2+1];
        char *cpay;

        cpay=pEnd+1;

        for (int i=0;i<len*2;i++)
        {
            payload[i]=*(cpay+i);
        }
        payload[len*2+1]='\0';

        char hex2[3];

        uint8_t upayload[len];

        for (int plen=0;plen<len;plen++)
        {
          for (int hlen=0;hlen<3;hlen++)
          {
              hex2[hlen]=payload[plen*2+hlen];
          }
          hex2[2]='\0';
          upayload[plen]=(uint8_t)strtol(hex2,0,16);
        }

        switch (type)
        {
          //ACK (Type = 0)
          case 0:
            break;
          case 1:
          //Tracking (Type = 1)
          // only build parse and process incoming message if we have lock

              // IMPORATNT PUT BACK!!!!
            //if (gpslocked)
                if(1)
            {

              Payload packet_payload(manu,
              id,
              bcast ,
              signature,
              type,
              len,
              &upayload[0]);



       /*    Serial.print("payload id ");Serial.println(packet_payload.id);
              Serial.print("payload manu ");Serial.println(packet_payload.manufacturer, HEX);
              Serial.print("payload lon x1000 ");  Serial.println(packet_payload.longitude*1000);
              Serial.print("payload lat x1000 ");  Serial.println(packet_payload.latitude*1000);
              Serial.print("payload aircraft type ");Serial.println(packet_payload.aircraft_type);
              Serial.print("altitude ");Serial.println(packet_payload.altitude);
              Serial.print("speed ");Serial.println(packet_payload.speed);
              Serial.print("climb ");Serial.println(packet_payload.climb);
              Serial.print("heading ");Serial.println(packet_payload.heading);
*/


            /*  for ( int packetNumber=0; packetNumber <  RX_PACKETS; packetNumber++)
              {
              // if its not ready there a free space in the array
                if (!payloadReady[packetNumber] )
                {
                //    Serial.println("Adding into array");
                  payloadList[packetNumber]=packet_payload;
                  payloadReady[packetNumber]=true;
                  break;
                }
              }

        */

              if ( packet_repeat=='y')
              {

                  // if its not ready there a free space in the array


                  // first search array, see if we already have this pilot in the array
                  // then if we do, update this pilots details and set to repeat
                  // if not, start at the start again and find the first 0 and add into array.

                  bool pilot_added=false;

                  for ( int packetNumber=0; packetNumber <  RX_PACKETS; packetNumber++)
                  {
                      if (payloadList[packetNumber].manufacturer==packet_payload.manufacturer &&
                              payloadList[packetNumber].id==packet_payload.id )
                      {
                       //   Serial.println("Adding into array - pilot found");

                          payloadList[packetNumber]=packet_payload;
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
                              payloadList[packetNumber]=packet_payload;
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
                          payloadList[packetNumber]=packet_payload;
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


#endif /* FLARM_H_ */
