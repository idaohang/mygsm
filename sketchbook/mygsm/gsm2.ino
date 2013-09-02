
#include "gsm.h"
  
char GSM_getBattInf(char *str_perc, char *str_vol){
	char ret_val=0;
	char *p_char; 
	char *p_char1;

	GSM_SimpleWriteln("AT+CBC");
	GSM_WaitResp1(5000, 1000, "OK");
	if(GSM_IsStringReceived("+CBC"))
		ret_val=1;
		
	//BCL
	p_char = strchr((char *)(comm_buf),',');
	p_char1 = p_char+1;  //we are on the first char of BCS
	p_char = strchr((char *)(p_char1), ',');
	if (p_char != NULL) {
          *p_char = 0; 
    }
	strcpy(str_perc, (char *)(p_char1));	
	
	//Voltage
	p_char++;
	p_char1 = strchr((char *)(p_char), '\r');
	if (p_char1 != NULL) {
          *p_char1 = 0; 
    }	
	strcpy(str_vol, (char *)(p_char));
	return ret_val;
}
  boolean GSM_getLacCi(char *lac,char *ci,unsigned char num)
  {
	char *p_char; 
	  char *p_char1;
	  GSM_WhileSimpleRead();
	  for(int i =0;i<num;i++)
	  {
	 _cell.println(F("AT+CREG?"));
	if(GSM_WaitResp1(5000, 1000, "+CREG")==RX_FINISHED_STR_RECV)
	{ 
	  p_char=strstr((char *)(comm_buf),",\"");
	  if (p_char != NULL){
		  p_char=p_char+2;	  
		  p_char1=strstr((char *)p_char,"\",\"");
		  if (p_char1!=NULL){
			  *p_char1=0;
			  strcpy(lac,p_char);
			  p_char1=p_char1+3;  
			  p_char = strchr((char *)(p_char1), '\"');
			  if (p_char != NULL) {
				  *p_char = 0; 
				  strcpy(ci,(char *)(p_char1));
				  return true;
			  }else
				  continue;
		  }
		  else	
			  continue;
	  }
	  else
		  continue;
	}
	else continue;
	}
	return false;
  }
  
  uint8_t GSM_read1()
  {
	return _cell.read();
  }
  
  void GSM_SimpleRead()
  {
	  char datain;
	  if(_cell.available()>0){
		  datain=_cell.read();
		  if(datain>0){
			  Serial.print(datain);
		  }
	  }
  }
   
  void GSM_SimpleWrite(char *comm)
  {
	  _cell.print(comm);
  }
  void GSM_SimpleWriteInt(int comm)
  {
	  _cell.print(comm);
  }
  void GSM_SimpleWriteF(const __FlashStringHelper *pgmstr)
  {
	  _cell.print(pgmstr);
  }
  void GSM_SimpleWriteln(char *comm)
  {
	  _cell.println(comm);
  }
  void GSM_SimpleWritelnF(const __FlashStringHelper *pgmstr)
  {
	  _cell.println(pgmstr);
  }
  
  
  void GSM_SimpleWriteIntln(int comm)
  {
	  _cell.println(comm);
  }

  
  void GSM_WhileSimpleRead()
  {
	  char datain;
	  while(_cell.available()>0){
		  datain=_cell.read();
		  //if(datain>0){
			  //Serial.print(datain);
		  //}
	  }
  }
 
 
