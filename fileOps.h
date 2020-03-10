#include "AirWare.h"
#include "FS.h"

extern char airWareSsid[32]; 
extern char airWarePassword[64];
extern char airWhere_ap_password[32];
extern char navSofware;
extern char receiveFlarm;
extern char groundStationMode;
extern char gsLatitude[11];
extern char gsLongitude[12];
extern char gsAltitude[6];
extern char wiredDirectly;
extern int gpsBaudRate;
extern char wifioffSelected;
extern int airWhereID;
extern char awHexID [5];
extern char awPilotName[30];
extern char loraFrequency;
extern char web_aircraft_type;
//v16
extern char lora_or_flarm;
extern char groundstation_type;
extern char gsm_access_point_name[40];
extern char gsm_username[30];
extern char gsm_password[30];
extern char packet_repeat;

extern int myChipID;

void load_configFile(void);
void save_configFile(String UpSsid,String UpPass,String UpNavSW, String GS, String hwMode,
                     String gpsBaud, String WOSin, String AwIDin, String awPilotNameIn,
                     String aWappassIn, String LfreqIn, String aircraft_type_in);


void update_configFile(String flarm);
void update_configFile(char gs,char gsm_or_internet);
void update_configFile(String UpSsid,String UpPass,String UpNavSW, String GS);

//hardware settings
void update_configFile(char selector, String UpNavSW, String GS, String hwMode, String gpsBaud, String WOSout,String LfreqIn, String radio_board );

void update_configFile(String gsLatitudeSwitch, String gsLongitudeSwitch, String gsAltitudeI, String UpSsid, String UpPass, String UpNavSW, String GS);
//update_configFile(UpSsid, UpPass, GS, AwIDin,asPilotNameIn,aWappassIn,aircraft_type_in, AwIDin );
//pilot settings
void update_configFile(String UpSsid,String UpPass, String GS,  String AwIDin, String awPilotNameIn, String aWappassIn, String aircraft_type_in,String AwIDin1, String packetsel );
//void update_configFile(String UpSsid,String UpPass,String UpNavSW, String GS, String hwMode,
 //                      String gpsBaud, String WOSin, String AwIDin, String awPilotNameIn,
  //                     String aWappassIn, String LfreqIn, String aircraft_type_in);
void update_configFile(String awID, String gsLatitudeSwitch, String gsLongitudeSwitch,String gsAltitude, String UpSsid, String UpPass, String UpNavSW, String GS, char rcF);
void update_configFile(String awID, String gsLatitudeSwitch, String gsLongitudeSwitch,String gsAltitude, String gsmAPNweb, String gsmUNweb, String gsmPWweb, String UpNavSW, String GS, char rcF);

int gpsSelectToBaud(char sel);
char gpsBaudtoChar(int sel);
