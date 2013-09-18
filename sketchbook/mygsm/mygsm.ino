#include <dht11.h>
#include <MinuteTimer.h>
#include "gsm.h"

#define HAVESENSOR
#define READEEPROM
//#define SMSTEST

#define DHT11PIN 7
#define SWITCHPIN 2
#define SETCMD "SET+"
#define SETALARMCENTERCMD "SET+AC"
#define SETALARMTEMPCMD "SET+AT"
#define SETALARMHUCMD "SET+AH"
#define SETALARMXCMD "SET+AX"
#define SETALARMYCMD "SET+AY"
#define SETALARMPERIODCMD "SET+AP"
#define SETPERMITSWITCH "SET+PS"
//#define SETEREPORTPERIODCMD "SET+RP"

#define CHK_SWH_BIT 0x01
#define CHK_TEMP_BIT 0x02
#define CHK_HU_BIT 0x04
#define CHK_X_BIT 0x08
#define CHK_Y_BIT 0x10
#define CHK_YE_BIT 0x20
#define CHK_BA_BIT 0x40


#ifdef READEEPROM
#include <avr/eeprom.h>
#define EEPROM_TEMP_ADDR 0
#define EEPROM_HU_ADDR 1
#define EEPROM_X_ADDR 2
#define EEPROM_Y_ADDR 3
#define EEPROM_RT_ADDR 4
#define EEPROM_SVRPORT_ADDR 5
#define EEPROM_BA_ADDR 7
#define EEPROM_YE_ADDR 8
#define EEPROM_BACHK_ADDR 9
#define EEPROM_ID_ADDR 10
#define EEPROM_AC_ADDR 20
#define EEPROM_SVR_ADDR 40
#define EEPROM_SVRPATH_ADDR 60
#endif

char ID[6];
char DEFAULTAC[20];
char DEFAULTSVRADDR[20];
char DEFAULTSVRPATH[50];

union int_uni{
	unsigned int port;
	unsigned char data[2];
	}uni_port;
#define DEFAULTSVRPORT (uni_port.port)


boolean check_permit_switch = false;
unsigned long check_permit_switch_timeout = 0;
unsigned long check_permit_switch_prevtime = 0;

unsigned long check_balance_timeout = 0;
unsigned long check_balance_prev = 0;

unsigned long check_battery_timeout = 0;
unsigned long check_battery_prev = 0;
boolean first_check_battery = false;


byte check = 0;
byte bakcheck = 0;
unsigned char bachk = 0;

int tempmax=35;
int humax = 95;
int xmax=45;
int ymax=45;
byte yemin=10;
byte bamin=10;
int reporttime=1;  //minutes
//int alarmtime=2; //seconds

int balance = -1;
int battery = -1;



dht11 dht(DHT11PIN);

boolean started=false;
boolean reporton=false;
//char mcc[5];
//char mnc[3];
char result[200];


#ifdef READEEPROM
void read_eeprom_str(char *re, unsigned char eeprom_addr)
{
	  char *p;
	  int addr=eeprom_addr;
	  p=re;
	  while(1){
	    *p = eeprom_read_byte((unsigned char *)addr++);
	    if (*p == '\n'){
	      *p=0;
	      break;
	    }
	    p++;
	  }
}
#endif

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

int getvalue1(char *s)
{
  int v=0;
  for(int i=0;i<strlen(s);i++)
  {
    int y=i%4;
    int a,b;
    if (y==0 ||y==1)
    {
      if (s[i] != '0')
	return -1;
      continue;
    }
    if (y==2)
    {
      if (s[i]!= '3')
	return -1;
    }
    if (y==3)
    {
      if (s[i]>0x39 || s[i]<0x30)
	return -1;
      v=s[i]-0x30+v*10;
    }
  }
  return v;
}

int getbalance(char *s)
{
  char *p1=NULL,*p2=NULL;
  p1 = strstr_P(s, PSTR("5F53524D5E1062374F59989D"));
  if (p1==NULL)
    return -1;
  p2 = strstr_P(p1, PSTR("002E"));
  if (p2 == NULL)
    p2=strstr_P(p1, PSTR("5143"));
  if (p2 == NULL)
    return -1;
  *p2=0;
  //printf("%s\n",p1);
  return getvalue1(p1+strlen_P(PSTR("5F53524D5E1062374F59989D")));
}

int towebcenter(char *server, int port,char *path, char *res)
{
	int numdata;
	char msg[150];
	numdata=GSM_httpPOST(server, port, path, res,msg, 150);
	if(numdata == -1){
		GSM_attachGPRS("cmnet", "", "");
		return 0;
	}
	//if (strstr(msg,"OK")!= NULL){
	//	return 1;
	//}
	return 1;
}

void report() 
{
  char *res = getresult("report ");
  if(towebcenter(DEFAULTSVRADDR,DEFAULTSVRPORT,DEFAULTSVRPATH,res) == 1)
  	return;
  if (1!=GSM_SendSMS(DEFAULTAC, res))
  {
          //debug("ERROR","Send report fail");
  }
}

char *getresult(char *tag)
{
  char str[20];
  if (tag != NULL){
	strcpy(result, tag);
	strcat(result, "info=");
  }
  else{
  	strcpy(result,"info=");
  }
  
  strcat(result, "ID:");
  strcat(result,ID);

 
  
  char cellinfo[20];
  char lon[15];
char lat[15];
char alt[15];
char time[20];
char vel[15];
char stat;
  //gsm.CellID();
  cellinfo[0]=0;
  if (GSM_getAllCellInfo(cellinfo,20,0)){
    strcat(result, ",Cellid:");
    strcat(result,cellinfo);
  }
  //stat=getStat();
  //if (stat == 2 || stat == 3){
    GSM_getPar(lon,lat,alt,time,vel);
    strcat(result,",Lon:");
    strcat(result,lon);
    strcat(result,",Lat:");
    strcat(result,lat);

    strcat(result, ",Balance:");
    str[0]=0;
    itoa(balance,str,10);
    strcat(result, str);

    strcat(result, ",Battery:");
    str[0]=0;
    itoa(battery,str,10);
    strcat(result,str);

  #ifdef HAVESENSOR
 ADXL335read();
 int xang = abs(ADXL335xAng());
 int yang = abs(ADXL335yAng());
 str[0]=0;
 itoa(xang,str,10);
 strcat(result, ",XAngle:");
 strcat(result,str);
 strcat(result,",YAngle:");
 str[0]=0;
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
				strcpy(DEFAULTAC, p+1);     
#ifdef READEEPROM
				for(int i=0;i<strlen(DEFAULTAC);i++){
				    eeprom_write_byte((unsigned char *)(EEPROM_AC_ADDR+i),DEFAULTAC[i]);
				}
				eeprom_write_byte((unsigned char *)(EEPROM_ID_ADDR+strlen(DEFAULTAC)),'\n');
#endif
                                ret=1;                                
			}
			
		}else if (0 == strncasecmp(sms, SETALARMTEMPCMD,6))
		{
			v=getvalue(sms);
			if (v) {
				tempmax=v;   
#ifdef READEEPROM
				eeprom_write_byte((unsigned char *)EEPROM_TEMP_ADDR,tempmax);
#endif
                                ret=1;
			}
		}else if (0 == strncasecmp(sms, SETALARMHUCMD,6))
		{
			v=getvalue(sms);
			if (v) {
				humax=v;
#ifdef READEEPROM
				eeprom_write_byte((unsigned char *)EEPROM_HU_ADDR,humax);
#endif
                                ret=1;
			}
		}else if (0 == strncasecmp(sms, SETALARMXCMD,6))
		{
			v=getvalue(sms);
			if (v) {
				xmax=v;
#ifdef READEEPROM
				eeprom_write_byte((unsigned char *)EEPROM_X_ADDR,xmax);
#endif
                                ret=1;
			}
		}else if (0 == strncasecmp(sms, SETALARMYCMD,6))
		{
			v=getvalue(sms);
			if (v) {
				ymax=v;
#ifdef READEEPROM
				eeprom_write_byte((unsigned char *)EEPROM_Y_ADDR,ymax);
#endif
                                ret=1;
			}
		}else if (0 == strncasecmp(sms, SETALARMPERIODCMD,6))
		{
			v=getvalue(sms);
			if (v) {
				reporttime=v;
                                mtimer.stop();
                                mtimer.set(reporttime,timerIsr);
                                mtimer.start();
#ifdef READEEPROM
				eeprom_write_byte((unsigned char *)EEPROM_RT_ADDR,reporttime);
#endif
                                ret=1;
			}
		}else if (0 == strncasecmp(sms, SETPERMITSWITCH,6))
		{
              	v=getvalue(sms);
              	check_permit_switch_prevtime = millis();
              	check_permit_switch_timeout = (unsigned long)v*60*1000;
              	check_permit_switch = true;
              	ret = 1;
		}
	}
	else if(0 == strncmp_P(sms, PSTR("007300650074002B00700073003D"),28))
	{
                v=getvalue1(sms+28);
                if (v != -1){
                	sprintf(sms, "%s%s%d",SETALARMPERIODCMD,"=",v);
              	check_permit_switch_prevtime = millis();
              	check_permit_switch_timeout = (unsigned long)v*60*1000;
              	check_permit_switch = true;
              	ret = 1;
		}
	}
        return ret;
}

void checkalarm(boolean checkbalance)
{
 boolean alarm=false;
  char perc[5];
 char str[20];
// int balance = -1;

 bakcheck = check;

 if (checkbalance){
 	balance = getbalance(result);
  }
 
  strcpy(result,"ism#alarm=");  
  strcat(result, "ID:");
  strcat(result,ID);

  if(checkbalance && balance != -1){
	if (balance<yemin){
		if (!(check & CHK_YE_BIT)){
			itoa(balance,str,10);
			 strcat(result, ",Balance:");
			 strcat(result,str);
			 check |= CHK_YE_BIT;
		 	 alarm=true;
		}
	}
	else{
		if (check & CHK_YE_BIT){
			strcat(result, ",Balance:clear");
			check &= (~CHK_YE_BIT);
			alarm=true;
		}
	}
  }

  if ((unsigned long)(millis() - check_battery_prev) >= check_battery_timeout||first_check_battery){
  		if (first_check_battery)
  			first_check_battery = false;
        	check_battery_prev = millis();
		if (GSM_getBattInf(perc,str)==1){
			char *pe;
			int v=0;
			int v1;
			v=strtol(perc,&pe,10);
			if(v!=0){
				v1=map(v, 25,100, 0,100);
				battery = v1;
				if (v1<bamin){
					if (!(check & CHK_BA_BIT)){
						itoa(v1,str,10);
						 strcat(result, ",Battery:");
						 strcat(result,str);
						 check |= CHK_BA_BIT;
#ifdef READEEPROM
						eeprom_write_byte((unsigned char *)EEPROM_BACHK_ADDR,1);
#endif
					 	 alarm=true;
					}
				}
				else{
					if (check & CHK_BA_BIT){
						strcat(result, ",Battery:clear");
						check &= (~CHK_BA_BIT);
#ifdef READEEPROM
						eeprom_write_byte((unsigned char *)EEPROM_BACHK_ADDR,0);
#endif
						alarm=true;
					}
				}
			}
		  }
		 
        }
  
  
#ifdef HAVESENSOR
 if (check_permit_switch ==  true && 
 	(unsigned long)(millis() - check_permit_switch_prevtime) >= check_permit_switch_timeout){
	check_permit_switch = false;
 }

  //bakcheck = check;
  if (digitalRead(SWITCHPIN)==HIGH){
	  if (check_permit_switch == false ){
	  	if (!(check & CHK_SWH_BIT)){
			   strcat(result,",Switch:ON"); 
			   check |= CHK_SWH_BIT;
			   alarm=true;
		}
	   }
  }
  else{
	if (check_permit_switch == false && (check & CHK_SWH_BIT)){
		strcat(result,",Switch:clear"); 
		check &= (~CHK_SWH_BIT);
		alarm=true;
	}
  }
 ADXL335read();
 int xang = abs(ADXL335xAng());
 int yang = abs(ADXL335yAng());
 if (xang>xmax){ 
 	if (!(check & CHK_X_BIT)){
		 itoa(xang,str,10);
		 strcat(result, ",XAngle:");
		 strcat(result,str);
		 check |= CHK_X_BIT;
	 	 alarm=true;
 	 }
 }else{
	if (check & CHK_X_BIT){
		strcat(result, ",XAngle:clear");
		check &= (~CHK_X_BIT);
		alarm=true;
	}
 }
  if (yang>ymax){ 
 	if (!(check & CHK_Y_BIT)){
		 itoa(yang,str,10);
		 strcat(result, ",YAngle:");
		 strcat(result,str);
		 check |= CHK_Y_BIT;
	 	 alarm=true;
 	 }
 }else{
	if (check & CHK_Y_BIT){
		strcat(result, ",YAngle:clear");
		check &= (~CHK_Y_BIT);
		alarm=true;
	}
 }
 
 dht.read();
 int temp = dht.temperature;
 int humidity = dht.humidity;  
 if (temp>tempmax){	
 	if (!(check & CHK_TEMP_BIT)){
		 itoa(temp,str,10);
		 strcat(result, ",Temp:");
		 strcat(result,str);
		 check |= CHK_TEMP_BIT;
	 	 alarm=true;
 	 }
 }else{
	if (check & CHK_TEMP_BIT){
		strcat(result, ",Temp:clear");
		check &= (~CHK_TEMP_BIT);
		alarm=true;
	}
 }
 if (humidity>humax){
 	if (!(check & CHK_HU_BIT)){
		 itoa(humidity,str,10);
		 strcat(result, ",Humidity:");
		 strcat(result,str);
		 check |= CHK_HU_BIT;
	 	 alarm=true;
 	 }
 }else{
	if (check & CHK_HU_BIT){
		strcat(result, ",Humidity:clear");
		check &= (~CHK_HU_BIT);
		alarm=true;
	}
 }
#endif
 
 if (alarm){
 	if (1!=GSM_SendSMS(DEFAULTAC, result))
  	{
          //debug("ERROR","Send alarm fail");
          //GSM_WhileSimpleRead();
          check = bakcheck;
  	}
  	 result[0]=0;
 }
}

void setup() 
{ 
  pinMode(SWITCHPIN,INPUT);
  //Serial connection.
  Serial.begin(9600);
  //delay(1000);
  Serial.println("GSM Shield testing v18");
  delay(1000);
#ifdef READEEPROM
  	Serial.println(F("Read EEPROM:"));
	tempmax = eeprom_read_byte((unsigned char *)EEPROM_TEMP_ADDR);	
	humax = eeprom_read_byte((unsigned char *)EEPROM_HU_ADDR);	
	xmax = eeprom_read_byte((unsigned char *)EEPROM_X_ADDR);	
	ymax = eeprom_read_byte((unsigned char *)EEPROM_Y_ADDR);	
	reporttime = eeprom_read_byte((unsigned char *)EEPROM_RT_ADDR);
	uni_port.data[0] = eeprom_read_byte((unsigned char *)EEPROM_SVRPORT_ADDR);
  	uni_port.data[1] = eeprom_read_byte((unsigned char *)(EEPROM_SVRPORT_ADDR+1));
  	sprintf(result, "T:%d,H:%d,X:%d,Y:%d,RT:%d,P:%d",tempmax,humax,xmax,ymax,reporttime,DEFAULTSVRPORT);	
  	Serial.println(result);
	bamin = eeprom_read_byte((unsigned char *)EEPROM_BA_ADDR);	
	yemin = eeprom_read_byte((unsigned char *)EEPROM_YE_ADDR);	
	bachk = eeprom_read_byte((unsigned char *)EEPROM_BACHK_ADDR);	
	sprintf(result, "Battery min:%d,Balance min:%d,Battery check value:%d",bamin,yemin,bachk);
	Serial.println(result);
	 result[0]=0;
	if(bachk == 1){
		 check |= CHK_BA_BIT;
	}
  	read_eeprom_str(ID,EEPROM_ID_ADDR);
  	Serial.print(F("ID:"));
  	Serial.println(ID);
  	read_eeprom_str(DEFAULTAC,EEPROM_AC_ADDR);
  	Serial.print(F("Alarm Center:"));
  	Serial.println(DEFAULTAC);
  	read_eeprom_str(DEFAULTSVRADDR,EEPROM_SVR_ADDR);
  	Serial.print(F("Server addr:"));
  	Serial.println(DEFAULTSVRADDR);
  	read_eeprom_str(DEFAULTSVRPATH,EEPROM_SVRPATH_ADDR);
  	Serial.print(F("Server path:"));
  	Serial.println(DEFAULTSVRPATH);
  	delay(1000);
#else
	strcpy(ID, "00001");
	strcpy(DEFAULTAC, "13715125676");
	strcpy(DEFAULTSVRADDR, "120.31.134.57");
	strcpy(DEFAULTSVRPATH, "/myservlet/MyServlet");
	DEFAULTSVRPORT = 8080;
#endif
  //Start configuration of shield with baudrate.
  //For http uses is raccomanded to use 4800 or slower.
  GSM_init();
  if (GSM_begin(9600)){
    Serial.println(F("\nstatus=GSM READY"));
    started=true;  
  }
  else 
    Serial.println(F("\nstatus=IDLE"));
  delay(2000);
  
  //GPRS attach
  if (GSM_attachGPRS("cmnet", "", ""))
      Serial.println(F("status=GPRS READY"));
    else 
    Serial.println(F("status=GPRS ERROR"));
    delay(1000);
    
  //GPS attach
    if (GSM_attachGPS())
      Serial.println(F("status=GPS READY"));
    else 
    	Serial.println(F("status=GPS ERROR"));
	
    delay(2000);	
  mtimer.set(reporttime,timerIsr);
  mtimer.start();
  GSM_SimpleWriteln("AT+CMGF=1");
  GSM_DeleteAllSMS();
  //GSM_getCellMccMnc( mcc,mnc);
  GSM_engmode();
  delay(1000);
  GSM_SendSMS("10086","YE");
  check_balance_prev = millis();
  check_balance_timeout = 60*60*1000;
  check_battery_prev = millis();
  check_battery_timeout = 60*60*1000;
  first_check_battery = true;
  delay(1000);
}
#if 1
void loop() 
{
  if(started){
	checkalarm(false);  
        if (reporton)
        {
          reporton=false; 
           report();           
        }

        if ((unsigned long)(millis() - check_balance_prev) >= check_balance_timeout){
        	 check_balance_prev = millis();
		GSM_SendSMS("10086","YE");
        }
        
        char position;
        char phone_num[20]; // array for the phone number string
        char sms_text[101]; // array for the SMS text string
        char ret = -5;

        position = GSM_IsSMSPresent(SMS_UNREAD);
        if (position) {
          GSM_GetSMS(position, phone_num, sms_text, 100);
         if (1 !=  (ret = GSM_DeleteSMS(position)))
         {
             //Serial.print(F("DeleteSMS return:"));
             //Serial.print(ret);
             //Serial.flush();
             delay(500);
         }
#ifdef SMSTEST
         Serial.println(sms_text);
         Serial.println(strlen(sms_text));
         byte *p = (byte*)sms_text;
         for(int i =0;i<strlen(sms_text);i++,p++)
        { Serial.print(*p);Serial.print("-");}
         Serial.println("end");
         GSM_WhileSimpleRead();
#endif
         if (1 == processsms(sms_text))
          {
            strcpy(result,sms_text);
            strcat(result," OK");
            if (1!=GSM_SendSMS(phone_num,result))
            {
                  //debug("ERROR","SendSMS fail");
            }
             result[0]=0;
          }
          else{
          	   if (0==strcmp_P(phone_num, PSTR("10086"))){
          	   	if ( NULL != strstr_P(sms_text, PSTR("5F53524D5E1062374F59989D"))){
				strcpy(result, sms_text);
				checkalarm(true);
			}
          	   }
          	   if (0!=strncmp(phone_num, "10",2) ||(0==strcmp(phone_num,DEFAULTAC)))
	          {
	          	char tag[10];
	          	if (0==strcmp(phone_num,DEFAULTAC))
	          	{
	          		if (0 == strncasecmp_P(sms_text, PSTR("get ncell"), 9)){
	          			strcpy_P(result, PSTR("ism#get ncell="));
					if (0 == GSM_getAllCellInfo(result, 200, 1)){
						result[0]=0;
					}
	          		}else{
					strcpy_P(tag, PSTR("ism#get "));
					getresult(tag);
				}
	          	}
	          	else{
	          		if (0 == strncasecmp_P(sms_text, PSTR("get ncell"), 9)){
	          			strcpy_P(result, PSTR("get ncell="));
					if (0 == GSM_getAllCellInfo(result, 200, 1)){
						result[0]=0;
					}
	          		}else{
		          		strcpy_P(tag, PSTR("get "));
		          		getresult(tag);
	          		}
	          	}
	          	 if (strlen(result))
	          	 {
	                	GSM_SendSMS(phone_num,result);
	                }
	                result[0]=0;
	          } 
          }
        }
    //delay(alarmtime*1000);
    delay(200);
    //delay(5000);
  }
}
#else
void loop()
{
	char position;
        char phone_num[20]; // array for the phone number string
        char sms_text[101]; // array for the SMS text string
        char ret = -5;

        position = GSM_IsSMSPresent(SMS_UNREAD);
        if (position) {
          GSM_GetSMS(position, phone_num, sms_text, 100);
         if (1 !=  (ret = GSM_DeleteSMS(position)))
         {
             Serial.print(F("DeleteSMS return:"));
             Serial.print(ret);
             Serial.flush();
             delay(500);
         }
         Serial.println(sms_text);
         Serial.println(strlen(sms_text));
         byte *p = (byte*)sms_text;
         for(int i =0;i<strlen(sms_text);i++,p++)
        { Serial.print(*p);Serial.print("-");}
         Serial.println("end");
         GSM_WhileSimpleRead();
          if (0==strcmp_P(phone_num, PSTR("10086")) ){
          Serial.println("1111111111111111");
          	if(NULL != strstr_P(sms_text, PSTR("5F53524D5E1062374F59989D"))){
          		strcpy(result, sms_text);
			int balance = getbalance(result);
			Serial.println("2222222222");
			Serial.println(balance);
          	   }
         }
         GSM_WhileSimpleRead();
         delay(1000);
         }
}

#endif
/*

void loop()
{
  char perc[10];
  char vol[10];
  
  if (GSM_getBattInf(perc,vol)==1){
    Serial.print("perc:");
    Serial.println(perc);
    GSM_WhileSimpleRead();
  }
  else{
    Serial.println("perc error");
    GSM_WhileSimpleRead();
  }
  
  delay(3000);
}*/
