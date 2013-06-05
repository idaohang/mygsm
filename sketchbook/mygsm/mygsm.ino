//#include <adxl335.h>
#include <dht11.h>
#include <MinuteTimer.h>
#include "SIM900.h"
#include "sms.h"
#include "gps.h"

#define HAVESENSOR
#define ID "00001"
#define DHT11PIN 7
#define SWITCHPIN 2
#define SETCMD "SET+"
#define SETALARMCENTERCMD "SET+AC"
#define SETALARMTEMPCMD "SET+AT"
#define SETALARMHUCMD "SET+AH"
#define SETALARMXCMD "SET+AX"
#define SETALARMYCMD "SET+AY"
#define SETALARMPERIODCMD "SET+AP"
//#define SETEREPORTPERIODCMD "SET+RP"

int tempmax=35;
int humax = 95;
int xmax=45;
int ymax=45;
int reporttime=5;  //minutes
//int alarmtime=2; //seconds

char alarmcenter[20];


char lon[15];
char lat[15];
char alt[15];
char time[20];
char vel[15];
char msg1[5];
char msg2[5];
char stat;

void initgpsdata()
{
	lon[0]=0;
	lat[0]=0;
	alt[0]=0;
	time[0]=0;
	vel[0]=0;
}


SMSGSM sms;
GPSGSM gps;
dht11 dht(DHT11PIN);

boolean started=false;
boolean reporton=false;
char result[200];

void setup() 
{ 
  pinMode(SWITCHPIN,INPUT);
  strcpy(alarmcenter,"13715125676");
  //Serial connection.
  Serial.begin(9600);
  //delay(1000);
  Serial.println("GSM Shield testing v6.");
  delay(1000);
  //Start configuration of shield with baudrate.
  //For http uses is raccomanded to use 4800 or slower.
  if (gsm.begin(9600)){
    Serial.println("\nstatus=READY");
    started=true;  
  }
  else 
    Serial.println("\nstatus=IDLE");
  delay(2000);
  //GPS attach
    if (gps.attachGPS())
      Serial.println("status=GPSREADY");
    else 
    	Serial.println("status=GPSERROR");
	
    delay(5000);	//Time for fixing
   /* stat=gps.getStat();
	if(stat==1)
		Serial.println("NOT FIXED");
	else if(stat==0)
		Serial.println("GPS OFF");
	else if(stat==2)
		Serial.println("2D FIXED");
	else if(stat==3)
		Serial.println("3D FIXED");
	delay(5000);
	//Get data from GPS
	gps.getPar(lon,lat,alt,time,vel);
	Serial.println(lon);
	Serial.println(lat);
	Serial.println(alt);
	Serial.println(time);
	Serial.println(vel);*/
  sms.DeleteAllSMS();
  mtimer.set(reporttime,timerIsr);
  mtimer.start();
};
//char cellinfo[20];
void loop() 
{
  //char cellinfo[20];
  if(started){
        //gsm.getAllCellInfo(cellinfo,20);
        
        //Serial.println(cellinfo);
        //Serial.flush();
        
  	//gsm.CellID();
  	//gps.checkPwr();
  	#ifdef HAVESENSOR
  	//ADXL335read();
  	//dht.read();
  	#endif
	checkalarm();  
        //stat=gps.getStat();
        //if (stat == 2 || stat == 3)
	  //gps.getPar(lon,lat,alt,time,vel);
        //else
        // ;// reporton = false;
        if (reporton)
        {
          reporton=false; 
           report();           
        }
        
        char position;
        char phone_num[20]; // array for the phone number string
        char sms_text[100]; // array for the SMS text string
        char ret = -5;

        position = sms.IsSMSPresent(SMS_UNREAD);
        if (position) {
          sms.GetSMS(position, phone_num, sms_text, 100);
         if (1 !=  (ret = sms.DeleteSMS(position)))
         {
             Serial.print("DeleteSMS return:");
             Serial.print(ret);
             Serial.flush();
             delay(500);
         }
         if (1 == processsms(sms_text))
          {
            strcpy(result,sms_text);
            strcat(result," OK");
            if (1!=sms.SendSMS(phone_num,result))
            {
                  debug("ERROR","SendSMS fail");
            }
          }
          else if (1!=sms.SendSMS(phone_num,getresult()))
          {
                debug("ERROR","SendSMS fail");
          }          
        }
    //delay(alarmtime*1000);
    delay(100);
    //delay(5000);
  }
}

long getvalue(const char *str)
{
	char c = '=';
	char *p=NULL,*pe;
	int v=0;
	p=strchr(str,c);
	if (p){
		v=strtol(p+1,&pe,10);
	}
	return v;
}

int processsms(char *sms)
{
        int ret=0;
	int v;
	if (0 == strncasecmp(sms, SETCMD,4))
	{
		if (0 == strncasecmp(sms, SETALARMCENTERCMD,6))
		{
			char *p;
			p=strchr(sms,'=');
			if (0!=getvalue(sms)){
				strcpy(alarmcenter, p+1);
				Serial.println(SETALARMCENTERCMD);
				Serial.println(alarmcenter);
				Serial.println("OK");                                
                                ret=1;
                                /*if (1!=sms.SendSMS(str,result))
                                {
                                      debug("ERROR","SendSMS fail");
                                }*/
			}
			
		}else if (0 == strncasecmp(sms, SETALARMTEMPCMD,6))
		{
			v=getvalue(sms);
			if (v) {
				tempmax=v;
				Serial.println(SETALARMTEMPCMD);
				Serial.println(tempmax);
				Serial.println("OK");
                                strcpy(result,sms);
                                strcat(result," OK");
                                ret=1;
			}
		}else if (0 == strncasecmp(sms, SETALARMHUCMD,6))
		{
			v=getvalue(sms);
			if (v) {
				humax=v;
				Serial.println(SETALARMHUCMD);
				Serial.println(humax);
				Serial.println("OK");
                                strcpy(result,sms);
                                strcat(result," OK");
                                ret=1;
			}
		}else if (0 == strncasecmp(sms, SETALARMXCMD,6))
		{
			v=getvalue(sms);
			if (v) {
				xmax=v;
				Serial.println(SETALARMXCMD);
				Serial.println(xmax);
				Serial.println("OK");
                                strcpy(result,sms);
                                strcat(result," OK");
                                ret=1;
			}
		}else if (0 == strncasecmp(sms, SETALARMYCMD,6))
		{
			v=getvalue(sms);
			if (v) {
				ymax=v;
				Serial.println(SETALARMYCMD);
				Serial.println(ymax);
				Serial.println("OK");
                                strcpy(result,sms);
                                strcat(result," OK");
                                ret=1;
			}
		}else if (0 == strncasecmp(sms, SETALARMPERIODCMD,6))
		{
			v=getvalue(sms);
			if (v) {
				reporttime=v;
				Serial.println(SETALARMPERIODCMD);
				Serial.println(reporttime);
				Serial.println("OK");
                                strcpy(result,sms);
                                strcat(result," OK");
                                mtimer.stop();
                                mtimer.set(reporttime,timerIsr);
                                mtimer.start();
                                ret=1;
			}
		}
	}
        return ret;
}

void checkalarm()
{
#ifdef HAVESENSOR
 char str[20];
 boolean alarm=false;

  strcpy(result,"alarm=");  
  strcat(result, "ID:");
  strcat(result,ID);
  if (digitalRead(SWITCHPIN)==HIGH){
   strcat(result,",Switch ON"); 
   alarm=true;
  }
 ADXL335read();
 int xang = ADXL335xAng();
 int yang = ADXL335yAng();
 if (xang>xmax){ 	
	 itoa(xang,str,10);
	 strcat(result, ",XAngle:");
	 strcat(result,str);
 	 alarm=true;
 }
 if (yang>ymax){
	 itoa(yang,str,10);
         //if (alarm)
          // strcat(result,",");
	 strcat(result, ",YAngle:");
	 strcat(result,str);
 	 alarm=true;
 }
 
 dht.read();
 int temp = dht.temperature;
 int humidity = dht.humidity;  
 if (temp>tempmax){	
	 itoa(temp,str,10);
         //if (alarm)
          // strcat(result,",");
	 strcat(result, ",Temp:");
	 strcat(result,str);
 	 alarm=true;
 }
 if (humidity>humax){
	 itoa(humidity,str,10);
         //if (alarm)
          // strcat(result,",");
	 strcat(result, ",Humidity:");
	 strcat(result,str);
 	 alarm=true;
 }
 //strcat(result,",cell:");
 //strcat(result,cellinfo);
 if (alarm){
 	if (1!=sms.SendSMS(alarmcenter, result))
  	{
          debug("ERROR","Send alarm fail");
  	}
 }

#endif
}

void report() 
{
  if (1!=sms.SendSMS(alarmcenter, getresult()))
  {
          debug("ERROR","Send report fail");
  }
}

char *getresult()
{
  char str[20];
  strcpy(result,"info=");
  
  strcat(result, "ID:");
  strcat(result,ID);

  /*if (true == gsm.getCellID(str,20))
  {
	strcat(result, ",Cellid:");
 	strcat(result,str);
  }
  else
  {
	strcat(result, "Cellid:");
 	strcat(result,"null");
  }*/
  char cellinfo[20];
  //gsm.CellID();
  if (gsm.getAllCellInfo(cellinfo,20)){
    strcat(result, ",Cellid:");
    strcat(result,cellinfo);
  }
  stat=gps.getStat();
  if (stat == 2 || stat == 3){
    gps.getPar(lon,lat,alt,time,vel);
    strcat(result,",Lon:");
    strcat(result,lon);
    strcat(result,",Lat:");
    strcat(result,lat);
  }
  else{
    strcat(result,",Lon:0.00");
    strcat(result,",Lat:0.00");
  }

  #ifdef HAVESENSOR
 ADXL335read();
 int xang = ADXL335xAng();
 int yang = ADXL335yAng();
 itoa(xang,str,10);
 strcat(result, ",XAngle:");
 strcat(result,str);
 strcat(result,",YAngle:");
 itoa(yang,str,10);
 strcat(result,str);
 
 dht.read();
 int temp = dht.temperature;
 int humidity = dht.humidity; 
 strcat(result, ",Temp:");
 itoa(temp,str,10);
 strcat(result,str);
 strcat(result, ",Humidity:");
 itoa(humidity,str,10);
 strcat(result,str);
 #endif
 
 return result;
}

void timerIsr()
{
    reporton = true;
}

void debug(const char *tag, const char *str)
{
       Serial.print(tag);
       Serial.print(":  ");
       Serial.println(str);
       Serial.flush();
       delay(500);
}

