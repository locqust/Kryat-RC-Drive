/*****************************************************************************/
// RC Values
// - These are defined by the limits of your transmitter
/*****************************************************************************/
#define RC_MIN 174
#define RC_MID 995
#define RC_MAX 1815

/*****************************************************************************/
// RC Deadband
// - Set a range of values to be considered "off"
/*****************************************************************************/
#define RC_DEADBAND_LOW 960
#define RC_DEADBAND_HIGH 970

/*****************************************************************************/
// RC Channel Mapping
// - The channels are N - 1 where N is the defined channel number on the transmitter
// - Ex: CH1 on Tx is 0 here, CH2 is 1
/*****************************************************************************/

#define CH_Button_Pad 8
#define CH_Button_Toggle 7
#define CH_Buttons_1_2 13
#define CH_Buttons_3_4 14
#define CH_Buttons_5_6 15
#define CH_Dome_servo 3


uint8_t dome_mac[] = {0xBC,0xFF,0x4D,0x19,0xF8,0x9F}; //Dome ESP Mac address
//startedMac Address: BC:FF:4D:19:F8:9F


/*****************************************************************************/
// RC button pad values
//These are the recorded values of the 15 buttons on my tx
/*****************************************************************************/

#define Button1 1685
#define Button2 751
#define Button3 657
#define Button4 1556
#define Button5 844
#define Button6 562
#define Button7 1437
#define Button8 940
#define Button9 468
#define Button10 1326
#define Button11 1033
#define Button12 371
#define Button13 1223
#define Button14 1126
#define Button15 268
#define Resting 174


/*****************************************************************************/
// Wifi
/*****************************************************************************/

//#define ENABLE_WIFI
const char* ssid = "kryat-rc";
const char* password = "12345678";

/*****************************************************************************/
// Audio
/*****************************************************************************/
#define AUDIO_ENABLED 1
#define AUDIO_OUTPUT_PIN 36
//#define AUDIO_BUSY_PIN 39
#define Vol 30

