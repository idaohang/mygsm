#include "gsm.h"


  
  int GSM_SendSMS(char *number_str, char *message_str) 
  {
	int ret_val = -1;
	byte i;
	char end[2];
	end[0]=0x1a;
	end[1]='\0';
  /*
	if (CLS_FREE != gsm.GetCommLineStatus()) return (ret_val);
	gsm.SetCommLineStatus(CLS_ATCMD);  
	ret_val = 0; // still not send
  */
	GSM_WhileSimpleRead();
	// try to send SMS 3 times in case there is some problem
	for (i = 0; i < 3; i++) {
	  // send  AT+CMGS="number_str"
	  
	  GSM_SimpleWriteF(F("AT+CMGS=\""));
	  GSM_SimpleWrite(number_str);	
	  GSM_SimpleWriteln("\"");
	  
	#ifdef DEBUG_ON
		  Serial.println("DEBUG:SMS TEST");
	#endif
	  // 1000 msec. for initial comm tmout
	  // 50 msec. for inter character timeout
	  if (RX_FINISHED_STR_RECV == GSM_WaitResp1(5000, 1000, ">")) {
		#ifdef DEBUG_ON
			  Serial.println("DEBUG:>");
		#endif
		// send SMS text
		GSM_SimpleWrite(message_str); 
		GSM_SimpleWrite(end);
		_cell.flush(); // erase rx circular buffer
		//GSM_SimpleFlush();
		if (RX_FINISHED_STR_RECV == GSM_WaitResp1(15000, 1500, "+CMGS")) {
		  // SMS was send correctly 
		  ret_val = 1;
  
		  break;
		}
		else continue;
	  }
	  else {
		// try again
		continue;
  
	  }
	}
  
	GSM_SetCommLineStatus(CLS_FREE);
	return (ret_val);
  }
  
  char GSM_IsSMSPresent(byte required_status) 
	  {
		char ret_val = -1;
		char *p_char;
		byte status;
	  
		GSM_WhileSimpleRead();
		if (CLS_FREE != GSM_GetCommLineStatus()) return (ret_val);
		GSM_SetCommLineStatus(CLS_ATCMD);
		ret_val = 0; // still not present
	  
		switch (required_status) {
		  case SMS_UNREAD:
			GSM_SimpleWritelnF(F("AT+CMGL=\"REC UNREAD\",1"));
			break;
		  case SMS_READ:
			GSM_SimpleWritelnF(F("AT+CMGL=\"REC READ\""));
			break;
		  case SMS_ALL:
			GSM_SimpleWritelnF(F("AT+CMGL=\"ALL\""));
			break;
		}
	  
		// 5 sec. for initial comm tmout
		// and max. 1500 msec. for inter character timeout
		GSM_RxInit(5000, 1500); 
		// wait response is finished
		do {
		  if (GSM_IsStringReceived("OK")) { 
			// perfect - we have some response, but what:
	  
			// there is either NO SMS:
			// <CR><LF>OK<CR><LF>
	  
			// or there is at least 1 SMS
			// +CMGL: <index>,<stat>,<oa/da>,,[,<tooa/toda>,<length>]
			// <CR><LF> <data> <CR><LF>OK<CR><LF>
			status = RX_FINISHED;
			break; // so finish receiving immediately and let's go to 
				   // to check response 
		  }
		  status = GSM_IsRxFinished();
		} while (status == RX_NOT_FINISHED);
	  
		
	  
	  
		switch (status) {
		  case RX_TMOUT_ERR:
			// response was not received in specific time
			ret_val = -2;
			break;
	  
		  case RX_FINISHED:
			// something was received but what was received?
			// ---------------------------------------------
			if(GSM_IsStringReceived("+CMGL:")) { 
			  // there is some SMS with status => get its position
			  // response is:
			  // +CMGL: <index>,<stat>,<oa/da>,,[,<tooa/toda>,<length>]
			  // <CR><LF> <data> <CR><LF>OK<CR><LF>
			  p_char = strchr((char *)comm_buf,':');
			  if (p_char != NULL) {
				ret_val = atoi(p_char+1);
			  }
			}
			else {
			  // other response like OK or ERROR
			  ret_val = 0;
			}
	  
			// here we have gsm.WaitResp() just for generation tmout 20msec. in case OK was detected
			// not due to receiving
			GSM_WaitResp2(20, 20); 
			break;
		}
	  
		GSM_SetCommLineStatus(CLS_FREE);
		return (ret_val);
}


  char GSM_GetSMS(byte position, char *phone_number, char *SMS_text, byte max_SMS_len) 
  {
	char ret_val = -1;
	char *p_char; 
	char *p_char1;
	byte len;
  
	GSM_WhileSimpleRead();
  
	if (position == 0) return (-3);
	if (CLS_FREE != GSM_GetCommLineStatus()) return (ret_val);
	GSM_SetCommLineStatus(CLS_ATCMD);
	phone_number[0] = 0;  // end of string for now
	ret_val = GETSMS_NO_SMS; // still no SMS
	
	//send "AT+CMGR=X" - where X = position
	GSM_SimpleWriteF(F("AT+CMGR="));
	GSM_SimpleWriteIntln((int)position);  
  
	// 5000 msec. for initial comm tmout
	// 100 msec. for inter character tmout
	switch (GSM_WaitResp1(5000, 500, "+CMGR")) {
	  case RX_TMOUT_ERR:
		// response was not received in specific time
		ret_val = -2;
		break;
  
	  case RX_FINISHED_STR_NOT_RECV:
		// OK was received => there is NO SMS stored in this position
		if(GSM_IsStringReceived("OK")) {
		  // there is only response <CR><LF>OK<CR><LF> 
		  // => there is NO SMS
		  ret_val = GETSMS_NO_SMS;
		}
		else if(GSM_IsStringReceived("ERROR")) {
		  // error should not be here but for sure
		  ret_val = GETSMS_NO_SMS;
		}
		break;
  
	  case RX_FINISHED_STR_RECV:
		// find out what was received exactly
  
		//response for new SMS:
		//<CR><LF>+CMGR: "REC UNREAD","+XXXXXXXXXXXX",,"02/03/18,09:54:28+40"<CR><LF>
			//There is SMS text<CR><LF>OK<CR><LF>
		if(GSM_IsStringReceived("\"REC UNREAD\"")) { 
		  // get phone number of received SMS: parse phone number string 
		  // +XXXXXXXXXXXX
		  // -------------------------------------------------------
		  ret_val = GETSMS_UNREAD_SMS;
		}
		//response for already read SMS = old SMS:
		//<CR><LF>+CMGR: "REC READ","+XXXXXXXXXXXX",,"02/03/18,09:54:28+40"<CR><LF>
			//There is SMS text<CR><LF>
		else if(GSM_IsStringReceived("\"REC READ\"")) {
		  // get phone number of received SMS
		  // --------------------------------
		  ret_val = GETSMS_READ_SMS;
		}
		else {
		  // other type like stored for sending.. 
		  ret_val = GETSMS_OTHER_SMS;
		}
  
		// extract phone number string
		// ---------------------------
		p_char = strchr((char *)(comm_buf),',');
		p_char1 = p_char+2; // we are on the first phone number character
		p_char = strchr((char *)(p_char1),'"');
		if (p_char != NULL) {
		  *p_char = 0; // end of string
		  strcpy(phone_number, (char *)(p_char1));
		}
  
  
		// get SMS text and copy this text to the SMS_text buffer
		// ------------------------------------------------------
		p_char = strchr(p_char+1, 0x0a);  // find <LF>
		if (p_char != NULL) {
		  // next character after <LF> is the first SMS character
		  p_char++; // now we are on the first SMS character 
  
		  // find <CR> as the end of SMS string
		  p_char1 = strchr((char *)(p_char), 0x0d);  
		  if (p_char1 != NULL) {
			// finish the SMS text string 
			// because string must be finished for right behaviour 
			// of next strcpy() function
			*p_char1 = 0; 
		  }
		  // in case there is not finish sequence <CR><LF> because the SMS is
		  // too long (more then 130 characters) sms text is finished by the 0x00
		  // directly in the gsm.WaitResp() routine
  
		  // find out length of the SMS (excluding 0x00 termination character)
		  len = strlen(p_char);
  
		  if (len < max_SMS_len) {
			// buffer SMS_text has enough place for copying all SMS text
			// so copy whole SMS text
			// from the beginning of the text(=p_char position) 
			// to the end of the string(= p_char1 position)
			strcpy(SMS_text, (char *)(p_char));
		  }
		  else {
			// buffer SMS_text doesn't have enough place for copying all SMS text
			// so cut SMS text to the (max_SMS_len-1)
			// (max_SMS_len-1) because we need 1 position for the 0x00 as finish 
			// string character
			memcpy(SMS_text, (char *)(p_char), (max_SMS_len-1));
			SMS_text[max_SMS_len] = 0; // finish string
		  }
		}
		break;
	}
  
	GSM_SetCommLineStatus(CLS_FREE);
	return (ret_val);
  }
 
 
  char GSM_DeleteSMS(byte position) 
  {
	char ret_val = -1;
  
	GSM_WhileSimpleRead();
	if (position == 0) return (-3);
	if (CLS_FREE != GSM_GetCommLineStatus()) return (ret_val);
	GSM_SetCommLineStatus(CLS_ATCMD);
	ret_val = 0; // not deleted yet
	
	//send "AT+CMGD=XY" - where XY = position
	GSM_SimpleWriteF(F("AT+CMGD="));
	GSM_SimpleWriteIntln((int)position);  
  
  
	// 5000 msec. for initial comm tmout
	// 20 msec. for inter character timeout
	switch (GSM_WaitResp1(5000, 500, "OK")) {
	  case RX_TMOUT_ERR:
		// response was not received in specific time
		ret_val = -2;
		break;
  
	  case RX_FINISHED_STR_RECV:
		// OK was received => SMS deleted
		ret_val = 1;
		break;
  
	  case RX_FINISHED_STR_NOT_RECV:
		// other response: e.g. ERROR => SMS was not deleted
		ret_val = 0; 
		break;
	}
  
	GSM_SetCommLineStatus(CLS_FREE);
	return (ret_val);
  }
  
  void GSM_DeleteAllSMS(){
  	GSM_WhileSimpleRead();
	  GSM_SendATCmdWaitResp(F("AT+CMGDA=\"DEL ALL\""), 10000, 500, "OK", 5);
  }
  
