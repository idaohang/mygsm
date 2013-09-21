#include "gsm.h"

	byte comm_buf[COMM_BUF_LEN+1];	// communication buffer +1 for 0x00 termination
		byte comm_buf_len;				// num. of characters in the buffer


    int _status;
    byte comm_line_status;

    // global status - bits are used for representation of states
    byte module_status;

    // variables connected with communication buffer
    
    byte *p_comm_buf;               // pointer to the communication buffer
    byte rx_state;                  // internal state of rx state machine    
    uint16_t start_reception_tmout; // max tmout for starting reception
    uint16_t interchar_tmout;       // previous time in msec.
    unsigned long prev_time;        // previous time in msec.
	HardwareSerial _cell(Serial);

	void GSM_init()
	{
		_status = IDLE;
	}

int GSM_begin(long baud_rate){
	int response=-1;
	int cont=0;
	boolean norep=true;
	boolean turnedON=false;
	GSM_SetCommLineStatus(CLS_ATCMD);
	
	pinMode(3,OUTPUT);//The default digital driver pins for the GSM and GPS mode
	  pinMode(4,OUTPUT);
	  pinMode(5,OUTPUT);
	  digitalWrite(3,LOW);//eable GSM TX?RX
    digitalWrite(4,HIGH);//disable GPS TX?RX
			//delay(10000);
	  
	_cell.begin(baud_rate);
	GSM_setStatus(IDLE); 

	
	
	for (cont=0; cont<3; cont++){
		if (AT_RESP_OK != GSM_SendATCmdWaitResp(F("AT"), 2500, 1500, "OK", 5)&&!turnedON) {		//check power
	    // there is no response => turn on the module
			#ifdef DEBUG_ON
				Serial.println("DB:NO RESP");
			#endif
			// generate turn on pulse
			digitalWrite(GSM_ON, HIGH);
			delay(2500);
			digitalWrite(GSM_ON, LOW);
			delay(15000);
			norep=true;
		}
		else{
			#ifdef DEBUG_ON
				Serial.println("DB:ELSE");
			#endif
			norep=false;
			turnedON=true;
			break;
		}
	}
	
	if (norep && AT_RESP_OK == GSM_SendATCmdWaitResp(F("AT"), 2500, 500, "OK", 5)){
		#ifdef DEBUG_ON
			Serial.println("DB:CORRECT BR");
		#endif
		turnedON=true;
		norep=false;
	}/*
	if(cont==3&&norep){
		Serial.println("Trying to force the baud-rate to 9600\n");
		for (int i=0;i<8;i++){
		switch (i) {
			case 0:
			  _cell.begin(1200);
			  _cell.print(F("AT+IPR=9600\r"));
			  break;
			  
			case 1:
			  _cell.begin(2400);
			  _cell.print(F("AT+IPR=9600\r"));
			  break;
			  
			case 2:
			  _cell.begin(4800);
			  _cell.print(F("AT+IPR=9600\r"));
			  break;
			  
			case 3:
			  _cell.begin(9600);
			  _cell.print(F("AT+IPR=9600\r"));
			  break;
			   
			case 4:
			  _cell.begin(19200);
			  _cell.print(F("AT+IPR=9600\r"));
			  break;
			  
			case 5:
			  _cell.begin(38400);
			  _cell.print(F("AT+IPR=9600\r"));
			  break;
			  
			case 6:
			  _cell.begin(57600);
			  _cell.print(F("AT+IPR=9600\r"));
			  break;
			  
			case 7:
			  _cell.begin(115200);
			  _cell.print(F("AT+IPR=9600\r"));
			  break;
			}	
		}
		Serial.println("ERROR: SIM900 doesn't answer. Check power and serial pins in GSM.cpp");
		return 0;
	}*/


	if (!turnedON && AT_RESP_ERR_DIF_RESP == GSM_SendATCmdWaitResp(F("AT"), 2500, 500, "OK", 5)){		//check OK
		#ifdef DEBUG_ON
			Serial.println("DB:DIFF RESP");
		#endif
		for (int i=0;i<8;i++){
			switch (i) {
			case 0:
			  _cell.begin(1200);
			  break;
			  
			case 1:
			  _cell.begin(2400);
			  break;
			  
			case 2:
			  _cell.begin(4800);
			  break;
			  
			case 3:
			  _cell.begin(9600);
			  break;
			   
			case 4:
			  _cell.begin(19200);
			  break;
			  
			case 5:
			  _cell.begin(38400);
			  break;
			  
			case 6:
			  _cell.begin(57600);
			  break;
			  
			case 7:
			  _cell.begin(115200);
			  _cell.print(F("AT+IPR=9600\r"));
			  _cell.begin(9600);
			  delay(500);
			  break;
  
			// if nothing else matches, do the default
			// default is optional
			}
					
			delay(100);

			#ifdef DEBUG_PRINT
				// parameter 0 - because module is off so it is not necessary 
				// to send finish AT<CR> here
				DebugPrint("DEBUG: Stringa ", 0);
				DebugPrint(buff, 0);
			#endif
				

			if (AT_RESP_OK == GSM_SendATCmdWaitResp(F("AT"), 2500, 500, "OK", 5)){
				#ifdef DEBUG_ON
					Serial.println("DB:FOUND PREV BR");
				#endif
				_cell.print(F("AT+IPR="));
				_cell.print(baud_rate);    
				_cell.print("\r"); // send <CR>
				delay(500);
				_cell.begin(baud_rate);
				delay(100);
				if (AT_RESP_OK == GSM_SendATCmdWaitResp(F("AT"), 2500, 500, "OK", 5)){
					#ifdef DEBUG_ON
						Serial.println("DB:OK BR");
					#endif
				}
				turnedON=true;
				break;					
			}
			#ifdef DEBUG_ON
				Serial.println("DB:NO BR");
			#endif			
		}
		// communication line is not used yet = free
		GSM_SetCommLineStatus(CLS_FREE);
		// pointer is initialized to the first item of comm. buffer
		p_comm_buf = &comm_buf[0];
	}

	GSM_SetCommLineStatus(CLS_FREE);

	if(turnedON){
		GSM_WaitResp2(50, 50);
		//InitParam(PARAM_SET_0);
		//InitParam(PARAM_SET_1);//configure the module  
		GSM_Echo(0);               //enable AT echo
		GSM_setStatus(READY);
		return(1);

	}
	else{
		//just to try to fix some problems with 115200 baudrate
		_cell.begin(115200);
		delay(1000);
		_cell.print(F("AT+IPR="));
		_cell.print(baud_rate);    
		_cell.print("\r"); // send <CR>		
		return(0);
	}
}

byte GSM_WaitResp1(uint16_t start_comm_tmout, uint16_t max_interchar_tmout, 
                   char const *expected_resp_string)
{
  byte status;
  byte ret_val;

  GSM_RxInit(start_comm_tmout, max_interchar_tmout); 
  // wait until response is not finished
  do {
    status = GSM_IsRxFinished();
  } while (status == RX_NOT_FINISHED);

  if (status == RX_FINISHED) {
    // something was received but what was received?
    // ---------------------------------------------
	
    if(GSM_IsStringReceived(expected_resp_string)) {
      // expected string was received
      // ----------------------------
      ret_val = RX_FINISHED_STR_RECV;      
    }
    else {
	ret_val = RX_FINISHED_STR_NOT_RECV;
	}
  }
  else {
    // nothing was received
    // --------------------
    ret_val = RX_TMOUT_ERR;
  }
  return (ret_val);
}	



/**********************************************************
Method sends AT command and waits for response

return: 
      AT_RESP_ERR_NO_RESP = -1,   // no response received
      AT_RESP_ERR_DIF_RESP = 0,   // response_string is different from the response
      AT_RESP_OK = 1,             // response_string was included in the response
**********************************************************/

char GSM_SendATCmdWaitResp(const __FlashStringHelper  *AT_cmd_string,
                uint16_t start_comm_tmout, uint16_t max_interchar_tmout,
                char const *response_string,
                byte no_of_attempts)
{
  byte status;
  char ret_val = AT_RESP_ERR_NO_RESP;
  byte i;

  for (i = 0; i < no_of_attempts; i++) {
    // delay 500 msec. before sending next repeated AT command 
    // so if we have no_of_attempts=1 tmout will not occurred
    if (i > 0) delay(500); 

    _cell.println(AT_cmd_string);
    status = GSM_WaitResp2(start_comm_tmout, max_interchar_tmout); 
    if (status == RX_FINISHED) {
      // something was received but what was received?
      // ---------------------------------------------
      if(GSM_IsStringReceived(response_string)) {
        ret_val = AT_RESP_OK;   
		
		#ifdef DEBUG_ON
		Serial.println("3333");

		#endif
        break;  // response is OK => finish
      }
      else{
		#ifdef DEBUG_ON
		Serial.println("4444");

		#endif
	  	ret_val = AT_RESP_ERR_DIF_RESP;
      }
    }
    else {
      // nothing was received
      // --------------------
      ret_val = AT_RESP_ERR_NO_RESP;
    }
    
  }

  return (ret_val);
}


byte GSM_WaitResp2(uint16_t start_comm_tmout, uint16_t max_interchar_tmout)
{
  byte status;

  GSM_RxInit(start_comm_tmout, max_interchar_tmout); 
  // wait until response is not finished
  do {
    status = GSM_IsRxFinished();
  } while (status == RX_NOT_FINISHED);
  return (status);
}

byte GSM_IsRxFinished(void)
{
  byte num_of_bytes;
  byte ret_val = RX_NOT_FINISHED;  // default not finished

  // Rx state machine
  // ----------------

  if (rx_state == RX_NOT_STARTED) {
    // Reception is not started yet - check tmout
    if (!_cell.available()) {
      // still no character received => check timeout
	/*  
	#ifdef DEBUG_GSMRX
		
			DebugPrint("\r\nDEBUG: reception timeout", 0);			
			Serial.print((unsigned long)(millis() - prev_time));	
			DebugPrint("\r\nDEBUG: start_reception_tmout\r\n", 0);			
			Serial.print(start_reception_tmout);	
			
		
	#endif
	*/
      if ((unsigned long)(millis() - prev_time) >= start_reception_tmout) {
        // timeout elapsed => GSM module didn't start with response
        // so communication is takes as finished
		/*
			#ifdef DEBUG_GSMRX		
				DebugPrint("\r\nDEBUG: RECEPTION TIMEOUT", 0);	
			#endif
		*/
        comm_buf[comm_buf_len] = 0x00;
        ret_val = RX_TMOUT_ERR;
      }
    }
    else {
      // at least one character received => so init inter-character 
      // counting process again and go to the next state
      prev_time = millis(); // init tmout for inter-character space
      rx_state = RX_ALREADY_STARTED;
    }
  }

  if (rx_state == RX_ALREADY_STARTED) {
    // Reception already started
    // check new received bytes
    // only in case we have place in the buffer
    num_of_bytes = _cell.available();
    // if there are some received bytes postpone the timeout
    if (num_of_bytes) prev_time = millis();
      
    // read all received bytes      
    while (num_of_bytes) {
      num_of_bytes--;
      if (comm_buf_len < COMM_BUF_LEN) {
        // we have still place in the GSM internal comm. buffer =>
        // move available bytes from circular buffer 
        // to the rx buffer
        *p_comm_buf = _cell.read();

        p_comm_buf++;
        comm_buf_len++;
        comm_buf[comm_buf_len] = 0x00;  // and finish currently received characters
                                        // so after each character we have
                                        // valid string finished by the 0x00
      }
      else {
        // comm buffer is full, other incoming characters
        // will be discarded 
        // but despite of we have no place for other characters 
        // we still must to wait until  
        // inter-character tmout is reached
        
        // so just readout character from circular RS232 buffer 
        // to find out when communication id finished(no more characters
        // are received in inter-char timeout)
        _cell.read();
      }
    }

    // finally check the inter-character timeout 
	/*
	#ifdef DEBUG_GSMRX
		
			DebugPrint("\r\nDEBUG: intercharacter", 0);			
<			Serial.print((unsigned long)(millis() - prev_time));	
			DebugPrint("\r\nDEBUG: interchar_tmout\r\n", 0);			
			Serial.print(interchar_tmout);	
			
		
	#endif
	*/
    if ((unsigned long)(millis() - prev_time) >= interchar_tmout) {
      // timeout between received character was reached
      // reception is finished
      // ---------------------------------------------
	  
		/*
	  	#ifdef DEBUG_GSMRX
		
			DebugPrint("\r\nDEBUG: OVER INTER TIMEOUT", 0);					
		#endif
		*/
      comm_buf[comm_buf_len] = 0x00;  // for sure finish string again
                                      // but it is not necessary
                                      
	  //_cell.println((char*)comm_buf);
	  //_cell.flush();
      ret_val = RX_FINISHED;
    }
  }
		
	
  return (ret_val);
}

byte GSM_IsStringReceived(char const *compare_string)
{
  char *ch;
  byte ret_val = 0;

  if(comm_buf_len) {
  /*
		#ifdef DEBUG_GSMRX
			DebugPrint("DEBUG: Compare the string: \r\n", 0);
			for (int i=0; i<comm_buf_len; i++){
				Serial.print(byte(comm_buf[i]));	
			}
			
			DebugPrint("\r\nDEBUG: with the string: \r\n", 0);
			Serial.print(compare_string);	
			DebugPrint("\r\n", 0);
		#endif
	*/
	//#ifdef DEBUG_ON
		//Serial.println("ATT: ");
		//Serial.print(compare_string);
		//Serial.print("RIC: ");
		//Serial.println((char *)comm_buf);
	//#endif
    ch = strstr((char *)comm_buf, compare_string);
    if (ch != NULL) {
		#ifdef DEBUG_ON
		Serial.println("1111");

		#endif
      ret_val = 1;
	  /*#ifdef DEBUG_PRINT
		DebugPrint("\r\nDEBUG: expected string was received\r\n", 0);
	  #endif
	  */
    }
	else
	{
		#ifdef DEBUG_ON
		Serial.println("2222");

		#endif
	  /*#ifdef DEBUG_PRINT
		DebugPrint("\r\nDEBUG: expected string was NOT received\r\n", 0);
	  #endif
	  */
	}
  }

  return (ret_val);
}


void GSM_RxInit(uint16_t start_comm_tmout, uint16_t max_interchar_tmout)
{
  rx_state = RX_NOT_STARTED;
  start_reception_tmout = start_comm_tmout;
  interchar_tmout = max_interchar_tmout;
  prev_time = millis();
  comm_buf[0] = 0x00; // end of string
  p_comm_buf = &comm_buf[0];
  comm_buf_len = 0;
  _cell.flush(); // erase rx circular buffer
}


void GSM_Echo(byte state)
{
	if (state == 0 or state == 1)
	{
	  GSM_SetCommLineStatus(CLS_ATCMD);

	  _cell.print("ATE");
	  _cell.print((int)state);    
	  _cell.print("\r");
	  delay(500);
	  GSM_SetCommLineStatus(CLS_FREE);
	}
}

void GSM_SetCommLineStatus(byte new_status) {comm_line_status = new_status;}
   byte GSM_GetCommLineStatus(void) {return comm_line_status;}

  void GSM_setStatus(enum GSM_st_e status) { _status = status; }  
  int GSM_getStatus(){   return _status; }
  
  
  int GSM_read2(char* res, int resultlength)
  {
	  GSM_WaitResp2(5000,1000);
	  if(comm_buf_len == 0)
		  return 0;
	  if (comm_buf_len<resultlength){
		  strncpy(res, (char*)comm_buf,comm_buf_len-1);
		  res[comm_buf_len]='\0';
		  return comm_buf_len;
	  }
	  else{
		  
		  strncpy(res, (char*)comm_buf,resultlength-1);
		  res[resultlength-1]='\0';
		  return resultlength-1;
	  }
  }

  void GSM_engmode()
  {
	if (GSM_getStatus()==IDLE)
    return;
     if(AT_RESP_OK != GSM_SendATCmdWaitResp(F("AT+CENG=3"), 2000, 1000, "OK", 2))
		  return ;
  }

  char *GSM_getAllCellInfo(char *res,int length, int flag)
{
	char *p_char; 
	char *p_char1;
	if (GSM_getStatus()==IDLE)
    return 0;
    if(AT_RESP_OK != GSM_SendATCmdWaitResp(F("AT+CENG?"), 5000, 1500, "+CENG", 2))
		  return 0;
	
    for(int j=0;j<5;j++)
    {       
	char str[10];
	sprintf(str,"%s%d,","+CENG:",j);
	p_char=strstr((char *)(comm_buf),str);//"+CENG:0,");
	//Serial.println(p_char);
	//SimpleWriteln(p_char),
	p_char1=p_char+8;	
	p_char = strchr((char *)(p_char1), ',');
	if (p_char != NULL) {
          //*p_char = 0; 
		//strcpy(cellid, (char *)(p_char1));	
		//SimpleWriteln(p_char1);
		//strcpy(res,(char *)(p_char1));
	  strncat(res,(char *)(p_char1),p_char-p_char1);
	   // return true;
	}
	for(int i=0;i<3;i++){
	    strcat(res,"-");
		p_char1=p_char+1;
		p_char = strchr((char*)p_char1,',');
		if (p_char != NULL){
			//*p_char = 0;
			//SimpleWriteln(p_char1);
			strncat(res, (char *)(p_char1),p_char-p_char1);
		}
	}
	if (!flag)
	  break;
	if (j<4)
	strcat(res,",");
    }
	//Serial.println(res);

	//SimpleWriteln(F("AT+CENG=0,0")); 
  	//gsm.WaitResp(5000, 50, "+OK");
	return res;
}
/*
  boolean GSM_getCellMccMnc(char *mcc,char *mnc)
  {
	  char *p_char; 
	  char *p_char1;
	  boolean ret = false;
	  if (GSM_getStatus()==IDLE)
	  return false;
	  // _cell.println(F("AT+CENG=3")); 
	  if(AT_RESP_OK != GSM_SendATCmdWaitResp(F("AT+CENG=3"), 2000, 1000, "OK", 2))
		  return false;
	  if(AT_RESP_OK != GSM_SendATCmdWaitResp(F("AT+CENG?"), 2000, 1000, "+CENG", 2))
		  return false;
	  p_char=strstr((char *)(comm_buf),"+CENG:0,");
	  //Serial.println(p_char);
	  //SimpleWriteln(p_char),
	  p_char1=p_char+8;   
	  p_char = strchr((char *)(p_char1), ',');
	  if (p_char != NULL) {
			*p_char = 0; 
		  strcpy(mcc,(char *)(p_char1));
		  p_char1=p_char+1;
		  p_char = strchr((char*)p_char1,',');
		  if (p_char != NULL){
			  *p_char = 0;
			  //SimpleWriteln(p_char1);
			  strcat(mnc, (char *)(p_char1));
			  ret = true;
		  }
		  else{
			  ret = false;
		  }
	  }else{
		  ret = false;
	  }
  
	  // _cell.println(F("AT+CENG=0,0")); 
	  //GSM_WaitResp1(5000, 1000, "+OK");
	  if(AT_RESP_OK != GSM_SendATCmdWaitResp(F("AT+CENG=0,0"), 5000, 1000, "OK", 2))
		  return false;
	  return ret;
  }
  */
