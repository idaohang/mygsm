#include <avr/eeprom.h>

#define WRITEEEPROM

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

//#pragma data:eeprom        
//unsigned char table[]= {35,95,45,4,0x01};//eep_read.eep
unsigned char table[]= {35,95,45,45,0x05};//eep_read.eep
unsigned char bamin = 10;
unsigned char yemin = 10;
unsigned char bachk = 0;
char *id = "00006";
//char *center = "13715125676";
//char *svr = "120.31.134.57";
//char *svrpath = "/myservlet/MyServlet";
char *center = "106575011785014643";
char *svr = "121.199.17.160";
char *svrpath = "/acc/service/dispatcher";
//#pragma data:data 
union int_uni{
unsigned int port;
unsigned char data[2];
}port = {8080};
  byte value;
  char result[200];
  
  
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

void setup()
{
  Serial.begin(9600);  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  #ifdef WRITEEEPROM
  Serial.println("Write EEPROM");
  for(int i=0;i<5;i++){
    eeprom_write_byte((unsigned char *)i,table[i]);
  }
  
  eeprom_write_byte((unsigned char *)EEPROM_SVRPORT_ADDR, port.data[0]);
  eeprom_write_byte((unsigned char *)(EEPROM_SVRPORT_ADDR+1), port.data[1]);
  
  eeprom_write_byte((unsigned char *)EEPROM_BA_ADDR, bamin);
  eeprom_write_byte((unsigned char *)EEPROM_YE_ADDR, yemin);
  eeprom_write_byte((unsigned char *)EEPROM_BACHK_ADDR, bachk);
  
  for(int i=0;i<strlen(id);i++){
    eeprom_write_byte((unsigned char *)(EEPROM_ID_ADDR+i),id[i]);
  }
  eeprom_write_byte((unsigned char *)(EEPROM_ID_ADDR+strlen(id)),'\n');
  
  for(int i=0;i<strlen(center);i++){
    eeprom_write_byte((unsigned char *)(EEPROM_AC_ADDR+i),center[i]);
  }
  eeprom_write_byte((unsigned char *)(EEPROM_AC_ADDR+strlen(center)),'\n');
  
  for(int i=0;i<strlen(svr);i++){
    eeprom_write_byte((unsigned char *)(EEPROM_SVR_ADDR+i),svr[i]);
  }
  eeprom_write_byte((unsigned char *)(EEPROM_SVR_ADDR+strlen(svr)),'\n');
  
  for(int i=0;i<strlen(svrpath);i++){
    eeprom_write_byte((unsigned char *)(EEPROM_SVRPATH_ADDR+i),svrpath[i]);
  }
  eeprom_write_byte((unsigned char *)(EEPROM_SVRPATH_ADDR+strlen(svrpath)),'\n');
  Serial.println("Write EEPROM End");
  #endif
  Serial.println("Reading EEPROM...");
  for(int i=0;i<5;i++){   
    Serial.print("addr ");
    Serial.print(i);
    Serial.print(":");
    Serial.print(eeprom_read_byte((unsigned char *)i),DEC);
    Serial.println();
  }
  
  union int_uni temp;
  temp.data[0] = eeprom_read_byte((unsigned char *)EEPROM_SVRPORT_ADDR);
  temp.data[1] = eeprom_read_byte((unsigned char *)(EEPROM_SVRPORT_ADDR+1));
  Serial.print("port:");
  Serial.print(temp.port);
  Serial.println();

  Serial.print("Battery min value:");
  Serial.print(eeprom_read_byte((unsigned char *)EEPROM_BA_ADDR));

  
  Serial.print("Balance min value:");
  Serial.print(eeprom_read_byte((unsigned char *)EEPROM_YE_ADDR));
  
  Serial.print("Battery check value:");
  Serial.print(eeprom_read_byte((unsigned char *)EEPROM_BACHK_ADDR));
    
  char *p,re[50];
  int addr=EEPROM_ID_ADDR;
  read_eeprom_str(re,addr);
  Serial.print("ID:");
  Serial.println(re);
  
  addr=EEPROM_AC_ADDR;
  read_eeprom_str(re,addr);
  Serial.print("Alarm Center:");
  Serial.println(re);
  
  addr=EEPROM_SVR_ADDR;
  read_eeprom_str(re,addr);
  Serial.print("Server Addr:");
  Serial.println(re);
  
  addr=EEPROM_SVRPATH_ADDR;
  read_eeprom_str(re,addr);
  Serial.print("Server Path:");
  Serial.println(re);
  
  sprintf(result, "T:%d,H:%d,X:%d,Y:%d,RT:%d,P:%d,SVRP:%s",table[0],table[1],table[2],table[3],table[4],temp.port,re);
  	Serial.println(result);
}
void loop()
{

  delay(5000);
}
