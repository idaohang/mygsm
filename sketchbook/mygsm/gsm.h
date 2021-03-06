#ifndef GSM_H
#define GSM_H


#include <Arduino.h>
#include <inttypes.h>



#define ctrlz 26 //Ascii character for ctr+z. End of a SMS.
#define cr    13 //Ascii character for carriage return. 
#define lf    10 //Ascii character for line feed. 
#define ctrlz 26 //Ascii character for ctr+z. End of a SMS.
#define cr    13 //Ascii character for carriage return. 
#define lf    10 //Ascii character for line feed.
#define GSM_LIB_VERSION 308 // library version X.YY (e.g. 1.00)

// pins definition
#define GSM_ON              5 // connect GSM Module turn ON to pin 77 

#define GSM_PIN 3 // Tri-state buffer control pin to enable GSM serial
#define GPS_PIN 4 // Tri-state buffer control pin to enable GPS serial

#define GSM_RESET           9 // connect GSM Module RESET to pin 35
//#define DTMF_OUTPUT_ENABLE  71 // connect DTMF Output Enable not used
#define DTMF_DATA_VALID     14 // connect DTMF Data Valid to pin 14
#define DTMF_DATA0          72 // connect DTMF Data0 to pin 72
#define DTMF_DATA1          73 // connect DTMF Data1 to pin 73
#define DTMF_DATA2          74 // connect DTMF Data2 to pin 74
#define DTMF_DATA3          75 // connect DTMF Data3 to pin 75

// length for the internal communication buffer
#define COMM_BUF_LEN        200

// some constants for the IsRxFinished() method
#define RX_NOT_STARTED      0
#define RX_ALREADY_STARTED  1

// some constants for the InitParam() method
#define PARAM_SET_0   0
#define PARAM_SET_1   1

// DTMF signal is NOT valid
//#define DTMF_NOT_VALID      0x10


// status bits definition
#define STATUS_NONE                 0
#define STATUS_INITIALIZED          1
#define STATUS_REGISTERED           2
#define STATUS_USER_BUTTON_ENABLE   4

// GPRS status
#define CHECK_AND_OPEN    0
#define CLOSE_AND_REOPEN  1

enum { EXT_MODE, GSM_MODE, GPS_MODE };


// SMS type 
// use by method IsSMSPresent()
enum sms_type_enum
{
  SMS_UNREAD,
  SMS_READ,
  SMS_ALL,

  SMS_LAST_ITEM
};

enum comm_line_status_enum 
{
  // CLS like CommunicationLineStatus
  CLS_FREE,   // line is free - not used by the communication and can be used
  CLS_ATCMD,  // line is used by AT commands, includes also time for response
  CLS_DATA,   // for the future - line is used in the CSD or GPRS communication  
  CLS_LAST_ITEM
};

enum rx_state_enum 
{
  RX_NOT_FINISHED = 0,      // not finished yet
  RX_FINISHED,              // finished, some character was received
  RX_FINISHED_STR_RECV,     // finished and expected string received
  RX_FINISHED_STR_NOT_RECV, // finished, but expected string not received
  RX_TMOUT_ERR,             // finished, no character received 
                            // initial communication tmout occurred
  RX_LAST_ITEM
};


enum at_resp_enum 
{
  AT_RESP_ERR_NO_RESP = -1,   // nothing received
  AT_RESP_ERR_DIF_RESP = 0,   // response_string is different from the response
  AT_RESP_OK = 1,             // response_string was included in the response

  AT_RESP_LAST_ITEM
};

enum registration_ret_val_enum 
{
  REG_NOT_REGISTERED = 0,
  REG_REGISTERED,
  REG_NO_RESPONSE,
  REG_COMM_LINE_BUSY,
    
  REG_LAST_ITEM
};

enum call_ret_val_enum
{
  CALL_NONE = 0,
  CALL_INCOM_VOICE,
  CALL_ACTIVE_VOICE,
  CALL_INCOM_VOICE_AUTH,
  CALL_INCOM_VOICE_NOT_AUTH,
  CALL_INCOM_DATA_AUTH,
  CALL_INCOM_DATA_NOT_AUTH,
  CALL_ACTIVE_DATA,
  CALL_OTHERS,
  CALL_NO_RESPONSE,
  CALL_COMM_LINE_BUSY,

  CALL_LAST_ITEM
};


enum getsms_ret_val_enum
{
  GETSMS_NO_SMS   = 0,
  GETSMS_UNREAD_SMS,
  GETSMS_READ_SMS,
  GETSMS_OTHER_SMS,

  GETSMS_NOT_AUTH_SMS,
  GETSMS_AUTH_SMS,

  GETSMS_LAST_ITEM
};

enum GSM_st_e { ERROR, IDLE, READY, ATTACHED, TCPSERVERWAIT, TCPCONNECTEDSERVER, TCPCONNECTEDCLIENT };

extern byte comm_buf[];
//extern byte comm_buf_len;


extern   int GSM_SendSMS(char *number_str, char *message_str);
extern void GSM_SimpleWritelnF(const __FlashStringHelper *pgmstr);
extern void GSM_SimpleWriteF(const __FlashStringHelper *pgmstr);
extern void GSM_SimpleWrite(char *comm);
extern void GSM_SimpleWriteln(char *comm);
extern void GSM_WhileSimpleRead();
extern void GSM_SimpleWriteIntln(int comm);

#endif
