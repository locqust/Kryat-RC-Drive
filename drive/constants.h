#ifndef CONSTANTS_H
#define CONSTANTS_H

/*****************************************************************************/
// RC Values
// - These are defined by the limits of your transmitter
/*****************************************************************************/
#define RC_MIN 174
#define RC_MID 995
#define RC_MAX 1815

/*****************************************************************************/
// RC Deadband
// - Set a range of values to be considered "off" for 3-pos switches
/*****************************************************************************/
#define RC_DEADBAND_LOW 960
#define RC_DEADBAND_HIGH 970

/*****************************************************************************/
// Button Pad resting value
// - The SBUS value when no button is pressed
/*****************************************************************************/
#define Resting 174

/*****************************************************************************/
// Hardware Pin Definitions
/*****************************************************************************/
#define DFPLAYER_TX_PIN 17 // DFPlayer RX to this pin
#define DFPLAYER_RX_PIN 5  // DFPlayer TX to this pin
#define AUDIO_BUSY_PIN 4
#define AUDIO_OUTPUT_PIN 36

#endif // CONSTANTS_H

