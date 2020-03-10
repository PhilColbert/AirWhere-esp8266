/*
 * gsm.h
 *
 *  Created on: 10 Jan 2018
 *      Author: Fugazi
 */

#ifndef GSM_H_
#define GSM_H_

#include "AirWare.h"
#include "gsm/gprs.h"

#define SIM800L_SERIAL_WAIT 500
#define SIM800L_STARTUP_TIME 3000
#define SIM800L_INIT_TIME 30000
#define SIM800L_GPRS_TIME 20000
#define SIM800L_RESET_WAIT 5000

extern char gsm_access_point_name[40];
extern char gsm_username[30];
extern char gsm_password[30];

GPRS gprs;

class Gsm
{
private:

    String gsm_output="";

    char http_cmd[70] = "GET www.airwhere.co.uk/downloads/version.txt HTTP/1.0\r\n\r\n";


    void add_to_gsm_output(String gsmout)
    {
        gsm_output=gsm_output+String('\n') + gsmout;
    }

public:
    int aircraft_type;

    void reset()
    {
       Serial1.println("Resetting sim800l board");
      //  pinMode(SIM800L_BOARD_RESET_PIN, OUTPUT);
     //   digitalWrite(SIM800L_BOARD_RESET_PIN, LOW);
     //   delay(500);
      //  digitalWrite(SIM800L_BOARD_RESET_PIN, HIGH);
       Serial.flush();
       Serial.println("");
       delay(SIM800L_SERIAL_WAIT);
   //    Serial.println("AT+CFUN=1,1");
       delay(SIM800L_RESET_WAIT);
       Serial.end();
       Serial.begin(9600);
    }

    bool begin(char *_access_point_name,char *_username, char *_password)
    {
      //  gprs.preInit();

        Serial.println("reset");
        pinMode(2, OUTPUT);

        digitalWrite(2, HIGH);
        delay(500);
        digitalWrite(2, LOW);
        delay(500);
        digitalWrite(2, HIGH);
        delay(SIM800L_STARTUP_TIME);

        int sim800l_ok=-1;

        unsigned int start=0;
        start=millis()+SIM800L_INIT_TIME;

        do {
            sim800l_ok=gprs.init();
            delay(1000);
            if (sim800l_ok!=0)
            {
                Serial.println("init error");
            }
        }
        while (start>millis() && sim800l_ok==-1);

        if (sim800l_ok!=0)
        {
            return false;
        }

        delay(1000);

        start=millis()+SIM800L_GPRS_TIME;

       while(!gprs.join(_access_point_name,_username,_password)) {
           Serial.println("Error joining GPRS network");
            delay(2000);

            if (start<millis())
            {
                return false;
            }
        }
/*String output;
        Serial.print("AT+COPS?\r\n");
        output=Serial.readString();
        //Serial.println(output);

        output="";
        delay(1000);

        Serial.print("AT+CSTT=\"payandgo.o2.co.uk\",\"O2 Mobile Web\",\"password\"\r\n");
        output=Serial.readString();
        //Serial.println(output);

        delay(1000);
        output="";

        Serial.print("AT+CSTT?\r\n");
        output=Serial.readString();
      //  Serial.println(output);
        delay(1000);

        Serial.print("AT+CIICR\r\n");
        output=Serial.readString();
      //  Serial.println(output);
        delay(1000);

        Serial.print("AT+CIFSR\r\n");
        output=Serial.readString();
      //  Serial.println(output);
        delay(1000);


      bool gps_con=false;

        start=millis()+SIM800L_BOOT_TIME;
        do {
            //sim800l_ok=gprs.join(_access_point_name,_username,_password);
            gps_con=gprs.join("payandgo.o2.co.uk","O2 Mobile Web","password");
            delay(2000);
            if (!gps_con)
            {
                Serial.println("Error joining GPRS network");
            }
        }
        while (start>millis() && !gps_con);

        if (sim800l_ok!=0)
        {
            return false;
        }
*/
        Serial.print("IP Address is ");
        Serial.println(gprs.getIPAddress());

        if(0 == gprs.connectTCP("airwhere.co.uk", 80)) {
            Serial.println("connect airwhere.co.uk success");
        }else{
            Serial.println("connect error");
            return false;
        }

        Serial.println("waiting to fetch...");
        if(0 == gprs.sendTCPData(http_cmd))
        {
            String out;
            while (Serial.available())
            {
                out=out+Serial.readString();
            }

            Serial.println(out);
        }

        gprs.closeTCP();

        gprs.shutTCP();

        return true;


    }

    /*
        delay(SIM800L_SERIAL_WAIT);
        String serial_data_in="";
        bool found_sim800l=false;
        unsigned int milli_end=millis()+SIM800L_BOOT_TIME;
        size_t slen=0;

        Serial.flush();
        Serial1.print("Searching for Sim800L GSM board..");

        delay(SIM800L_SERIAL_WAIT);

        do
        {
            Serial.print("AT");
            add_to_gsm_output("AT");

            delay(SIM800L_SERIAL_WAIT);

            slen=Serial.available();
            Serial1.print(".");

            if (slen!=0)
            {
                serial_data_in=Serial.readString();
                add_to_gsm_output(serial_data_in);
                if (serial_data_in.substring(0,4)=="OK")
                {
                   found_sim800l=true;
                   Serial1.print(".");
                }
            }
        }
        while (milli_end>millis() && !found_sim800l);

        if (!found_sim800l)
        {
            Serial1.println("Output from sim800L");
            Serial1.println(gsm_output);
            Serial1.println("End Output from sim800L");
            return false;
        }


        if (flarm_board_build[0]=='\0')
        {
            return false;
        }
        delay(FLARM_SERIAL_WAIT);
    //    Serial.print("#FNA\n");
        flarmSerial.print("#FNA\n");
        delay(FLARM_SERIAL_WAIT);
        String fna=flarmSerial.readString();
     //  Serial.println(fna);
       flarmSerial.flush();

       Serial.print(".");

       fna.substring(5).toCharArray(manufacturer_code,3);
       this->manufacturer = strtol( manufacturer_code, 0, 16);

       //bodge until we sort lk for FF manufacturers.
       // skytraxx is 0x11 (17) and 0x1

       if (this->manufacturer==17)
       {
           this->manufacturer=1;
       }
    //    Serial.println(manufacturer_code);
       fna.substring(8).toCharArray(flarm_board_id,5);
       this->id = strtol( flarm_board_id, 0, 16);
     //  Serial.println(flarm_board_id);
       flarmSerial.flush();

       Serial.print(".");

     //  Serial.print("#FAX\n");
        flarmSerial.print("#FAX\n");
        delay(FLARM_SERIAL_WAIT);
        flarmSerial.readString().substring(5).toCharArray(flarm_board_expiration,9);
    //   Serial.println(flarm_board_expiration);
       flarmSerial.flush();

       Serial.print(".");

        char flarm_ok[3];
     //   Serial.print("#FNC 1,1\n");

        flarmSerial.print("#FNC 1,1\n");
        delay(FLARM_SERIAL_WAIT);
 //       Serial.println(flarmSerial.readString());

       // flarmSerial.readString().substring(5).toCharArray(flarm_ok,3);
     //   Serial.println("#fnc");
    //   Serial.println(flarm_ok);
       flarmSerial.flush();

       Serial.print(".");

        flarmSerial.print("#DGP 1\n");
        delay(FLARM_SERIAL_WAIT);
        if (flarmSerial.readString().substring(5,7)=="OK")
        {
            fanet_tracking_on=true;
        }
        else
        {
            fanet_tracking_on=false;
        }
        flarmSerial.flush();

        Serial.print(".");

    //    Serial.print("#FAP 1\n");
        flarmSerial.print("#FAP 1\n");
        delay(FLARM_SERIAL_WAIT);
        if (flarmSerial.readString().substring(5,7)=="OK")
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
*/
    bool connectUDP()
    {
        if(0 == gprs.connectUDP("airwhere.co.uk", 5555))
        {
            Serial.println("connect UDP airwhere.co.uk success");
            return true;
        }
        else
        {
            Serial.println("connect UDP error");
            return false;
        }

    }

    bool sendUDP(String data)
    {
        char cdata[150];
        data.toCharArray(cdata,150);
        if (gprs.sendUDPData(cdata)==0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool closeTCP()
    {
        gprs.closeTCP();
        delay(1000);
    }

    void sendAT()
    {
        gprs.sendATTest();
    }

    bool sendUDPDataConfirm(String data)
    {
        char cdata[150];
        data.toCharArray(cdata,150);
        if (gprs.sendUDPDataConfirm(cdata)==0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

};




#endif /* GSM_H_ */
