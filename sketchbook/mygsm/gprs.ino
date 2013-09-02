#include "gsm.h"
  
  int GSM_attachGPRS(char* domain, char* dom1, char* dom2)
  {
	 int i=0;
	 delay(5000);
	 
	//gsm._tf.setTimeout(_GSM_DATA_TOUT_);	  //Timeout for expecting modem responses.
	GSM_WaitResp2(1000, 250);
	GSM_SimpleWritelnF(F("AT+CIFSR"));
	if(GSM_WaitResp1(5000, 250, "ERROR")!=RX_FINISHED_STR_RECV){
	  GSM_SimpleWritelnF(F("AT+CIPCLOSE"));
	  GSM_WaitResp1(5000, 250, "ERROR");
	  delay(1000);
	  //gsm.SimpleWriteln("AT+CIPSERVER=0");
	  //gsm.WaitResp(5000, 50, "ERROR");
	  return 1;
	}
	else{  
	GSM_SimpleWritelnF(F("AT+CIPSHUT"));
	
	switch(GSM_WaitResp1(1000, 250, "SHUT OK")){
	  case RX_TMOUT_ERR: 
		  return 0;
	  break;
	  case RX_FINISHED_STR_NOT_RECV: 
		  return 0; 
	  break;
	}
	   delay(1000);
	   
	GSM_SimpleWriteF(F("AT+CSTT=\""));
	GSM_SimpleWrite(domain);
	GSM_SimpleWrite("\",\"");
	GSM_SimpleWrite(dom1);
	GSM_SimpleWrite("\",\"");
	GSM_SimpleWrite(dom2);
	GSM_SimpleWrite("\"\r");  
  
	
	switch(GSM_WaitResp1(1000, 250, "OK")){
	  case RX_TMOUT_ERR: 
		  return 0;
	  break;
	  case RX_FINISHED_STR_NOT_RECV: 
		  return 0; 
	  break;
	}
	   delay(5000);
		
	  GSM_SimpleWritelnF(F("AT+CIICR"));  
  
	switch(GSM_WaitResp1(10000, 500, "OK")){
	  case RX_TMOUT_ERR: 
		  return 0; 
	  break;
	  case RX_FINISHED_STR_NOT_RECV: 
		  return 0; 
	  break;
	}
	delay(1000);
   GSM_SimpleWritelnF(F("AT+CIFSR"));
   if(GSM_WaitResp1(5000, 500, "ERROR")!=RX_FINISHED_STR_RECV){
	  GSM_setStatus(ATTACHED);
	  return 1;
     }
   return 0;
   }
}
 
  int GSM_httpPOST(const char* server, int port, const char* path, const char* parameters, char* result, int resultlength)
  {
	boolean connected=false;
	boolean sendok = false;
	int n_of_at=0;
	char itoaBuffer[8];
	int num_char;
	char end_c[2];
	end_c[0]=0x1a;
	end_c[1]='\0';
  
	GSM_WhileSimpleRead();
  
	while(n_of_at<3){
		if(!GSM_connectTCP(server, port)){
	  	#ifdef DEBUG_ON
			  Serial.println("DB:NOT CONN");
		#endif	
			  n_of_at++;
		}
		else{
		  connected=true;
		  n_of_at=3;
	  }
	}
  
	if(!connected) return -1;
	  
	GSM_SimpleWriteF(F("POST "));
	GSM_SimpleWrite((char*)path);
	GSM_SimpleWriteF(F(" HTTP/1.1\nHost: "));
	GSM_SimpleWrite((char*)server);
	GSM_SimpleWrite("\n");
	GSM_SimpleWriteF(F("User-Agent: Arduino\n"));
	GSM_SimpleWriteF(F("Content-Type: application/x-www-form-urlencoded\n"));
	GSM_SimpleWriteF(F("Content-Length: "));
	itoa(strlen(parameters),itoaBuffer,10);
	GSM_SimpleWrite(itoaBuffer);
	GSM_SimpleWriteF(F("\n\n"));
	GSM_SimpleWrite((char*)parameters);
	GSM_SimpleWriteF(F("\n\n"));
	GSM_SimpleWrite(end_c);
   
	switch(GSM_WaitResp1(10000, 1500, "SEND OK")){
	  case RX_TMOUT_ERR: 
		  return -1;
	  break;
	  case RX_FINISHED_STR_NOT_RECV: 
		  return -1; 
	  case RX_FINISHED_STR_RECV:
		  //return 1;
		  sendok=true;
	  break;
	}
  
   delay(50);
  
	int res= GSM_read2(result, resultlength);
	GSM_disconnectTCP();
	//if (res == 0 && sendok)
	//	  res = 1;
	return res;
  }
  
  
  int GSM_connectTCP(const char* server, int port)
  {
	//Visit the remote TCP server.
	 GSM_SimpleWriteF(F("AT+CIPSTART=\"TCP\",\""));
	 GSM_SimpleWrite((char*)server);
	 GSM_SimpleWriteF(F("\","));
	 GSM_SimpleWriteIntln(port);
	
	switch(GSM_WaitResp1(15000, 1500, "OK")){
	  case RX_TMOUT_ERR: 
		  return 0;
	  break;
	  case RX_FINISHED_STR_NOT_RECV: 
		  return 0; 
	  break;
	} 
  /*
	switch(GSM_WaitResp1(15000, 1500, "OK")){
	  case RX_TMOUT_ERR: 
		  return 0;
	  break;
	  case RX_FINISHED_STR_NOT_RECV: 
		  return 0; 
	  break;
	}*/
  
	GSM_setStatus(TCPCONNECTEDCLIENT);
  
	delay(3000);
	GSM_SimpleWritelnF(F("AT+CIPSEND"));
	switch(GSM_WaitResp1(5000, 1500, ">")){
	  case RX_TMOUT_ERR: 
		  return 0;
	  break;
	  case RX_FINISHED_STR_NOT_RECV: 
		  return 0; 
	  break;
	}
  
	delay(4000);
	return 1;
  }
  
  int GSM_disconnectTCP()
  {
	GSM_SimpleWritelnF(F("AT+CIPCLOSE"));
	delay(1000);
	if(GSM_getStatus()==TCPCONNECTEDCLIENT)
		  GSM_setStatus(ATTACHED);
	 else
		  GSM_setStatus(TCPSERVERWAIT);   
	  return 1;
  }
  
  char GSM_attachGPS() 
  {
	  if(AT_RESP_OK != GSM_SendATCmdWaitResp(F("AT+CGPSPWR=1"), 2000, 500, "OK", 5))
		  return 0;
	  if(AT_RESP_OK != GSM_SendATCmdWaitResp(F("AT+CGPSRST=1"), 2000, 500, "OK", 5))
		  return 0;
	  return 1;
  }
  
  
  char GSM_getPar(char *str_long, char *str_lat, char *str_alt, char *str_time, char *str_speed) 
  {
	  char ret_val=0;
	  char *p_char; 
	  char *p_char1;
	  GSM_SimpleWritelnF(F("AT+CGPSINF=0"));
	  GSM_WaitResp1(5000, 500, "OK");
	  if(GSM_IsStringReceived("OK"))
		  ret_val=1;
		  
	  //longitude
	  p_char = strchr((char *)(comm_buf),',');
	  p_char1 = p_char+1;  //we are on the first char of longitude
	  p_char = strchr((char *)(p_char1), ',');
	  if (p_char != NULL) {
			*p_char = 0; 
	  }
	  strcpy(str_long, (char *)(p_char1));
	  
	  // latitude
	  p_char++;
	  p_char1 = strchr((char *)(p_char), ',');
	  if (p_char1 != NULL) {
			*p_char1 = 0; 
	  }   
	  strcpy(str_lat, (char *)(p_char));
	  
	  // altitude
	  p_char1++;
	  p_char = strchr((char *)(p_char1), ',');
	  if (p_char != NULL) {
			*p_char = 0; 
	  }   
	  strcpy(str_alt, (char *)(p_char1));
	  
	  // UTC time
	  p_char++;
	  p_char1 = strchr((char *)(p_char), ',');
	  if (p_char1 != NULL) {
			*p_char1 = 0; 
	  }   
	  strcpy(str_time, (char *)(p_char)); 
  
	  // TTFF
	  p_char1++;
	  p_char = strchr((char *)(p_char1), ',');
	  if (p_char != NULL) {
			*p_char = 0; 
	  }   
  
	  // num
	  p_char++;
	  p_char1 = strchr((char *)(p_char), ',');
	  if (p_char1 != NULL) {
			*p_char1 = 0; 
	  }   
  
	  // speed
	  p_char1++;
	  p_char = strchr((char *)(p_char1), ',');
	  if (p_char != NULL) {
			*p_char = 0; 
	  } 	  
	  strcpy(str_speed, (char *)(p_char1));   
	  
	  return ret_val;
  }

