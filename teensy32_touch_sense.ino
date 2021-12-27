/*
   FileName:    teensy32_touch_sense.ino
   
   Overview:    Test program for Teensy 3.2 non-blocking touch sense functions.
   
   Processor:   Freescale MK20DX256 ARM Cortex M4  (F_CPU = 72 MHz)
   
   Platform:    Teensy 3.2 microcontroller dev board (or functional equivalent)
   
   Toolchain:   Arduino IDE with Teensyduino support package
   
   Originated:  M.J.Bauer (alias Lalbert), 2020   [www.mjbauer.biz]
*/

#define TOUCH_SENSE_DONE  ((TSI0_GENCS & TSI_GENCS_SCNIP) == 0)

#define SUCCESS  (0)
#define FAIL    (-1)

// Function prototype declarations
int      touchSenseInit(uint8_t pin);
bool     touchSenseDone();
int      touchSenseRead();

// Array of touch inputs (pin numbers) to be read... (up to 12 pins):
uint8_t  touchPadPins[] = { 0, 1, 33, 15, 16, 17, 18, 19, 22, 23 };

// Array of touch readings corresponding to the above pins:
uint32_t touchReadings[12];

short    pindex;  // index into arrays: touchPadPins[] and touchReadings[]

//-------------------------------------------------------------------------------------

void  setup(void)
{
  uint8_t  firstPin = touchPadPins[0];
  
  touchSenseInit(firstPin);    // Initiate touch reading on first pin

  // ... etc ...
}


void  loop(void)
{
  static short numberOfTouchPins = sizeof(touchPadPins);
  
  if (touchSenseDone())   // true -> reading is ready
  {
	  touchReadings[pindex] = touchSenseRead();  // read current input
	  pindex++;
	  if (pindex >= numberOfTouchPins) pindex = 0;  // done all pins
	  touchSenseInit(touchPadPins[pindex]);  // next touch input
  }
  
  // Put below all other code to go into main loop.
  // For example, output values in array touchReadings[] via Serial port,
  // at periodic intervals.
  ;
  ;
  ;
  ;
}


//--------------------------------------------------------------------------------------
// Teensy 3.2 (MK20DX256) Touch Sense Support Functions -- Non-blocking!  
//
// These settings give approx 0.02 pF sensitivity and 1200 pF range.
// Lower current, higher number of scans, and higher prescaler increase sensitivity,
// but the trade-off is longer measurement time and decreased range.
// Time to measure 33 pF is approx 0.25 ms
// Time to measure 1000 pF is approx 4.5 ms

#define CURRENT   2  // 0 to 15 - current to use, value is 2*(current+1)
#define NSCAN     9  // number of times to scan, 0 to 31, value is nscan+1
#define PRESCALE  2  // prescaler, 0 to 7 - value is 2^(prescaler+1)

static const uint8_t pin2tsi[] = {
// 0    1    2    3    4    5    6    7    8    9
  9,   10,  255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255,  13,   0,   6,   8,   7,
  255, 255,  14,  15, 255,  12, 255, 255, 255, 255,
  255, 255,  11,   5
};

static uint32_t  tsi_ch;  // TSI channel


// Function initiates touch sense routine on the specified Teensy 3.2 pin,
// but it DOES NOT WAIT for the result. Check using touchSenseDone().
//
// Note [1]: The 10us delay may be removed if caller waits >= 10us before calling 
//           touchSenseDone(), after first calling touchSenseInit().
//
int  touchSenseInit(uint8_t pin)
{
  tsi_ch = pin2tsi[pin];
  if (tsi_ch > 15) return FAIL;

  *portConfigRegister(pin) = PORT_PCR_MUX(0);

  SIM_SCGC5 |= SIM_SCGC5_TSI;
  TSI0_GENCS = 0;
  TSI0_PEN = (1 << tsi_ch);
  TSI0_SCANC = TSI_SCANC_REFCHRG(3) | TSI_SCANC_EXTCHRG(CURRENT);
  TSI0_GENCS = TSI_GENCS_NSCN(NSCAN) | TSI_GENCS_PS(PRESCALE) | TSI_GENCS_TSIEN | TSI_GENCS_SWTS;
  delayMicroseconds(10);  // see note [1]
  return SUCCESS;
}


// Function checks touch sense status. If the routine has completed and a result is ready,
// the return value will be True (1), otherwise False (0). The function does not wait.
// Alternatively, use macro: TOUCH_SENSE_DONE
//
bool  touchSenseDone()
{
  if (TSI0_GENCS & TSI_GENCS_SCNIP) return 0;  // busy
  else  return 1;  // done
}


// Function returns the last reading available from the touch sense routine.
// Call touchSenseDone() first to check that a valid reading is ready.
//
int  touchSenseRead()
{
  return *((volatile uint16_t *)(&TSI0_CNTR1) + tsi_ch);
}

// end of file
