#include <Arduino.h>
// the IR tx is generated by spi data formatted correctly
#include <SPI.h>
#include "TimerInterrupt_Generic.h"

#define sensorPin PA0
#define yLedC     PA6
#define irLedC    PA7
#define yLedA     PA5
#define irLedA    PB0

/* Sony protocol: from:
 * Investigating the cross-compatibility of IR-controlled active shutter glasses
 * Andrew J. Woods and Jesse Helliwell
 *
 * Close right in us
 *   |--20--|      |--20--|      |--20--|            |--20--|      |--20--|     
 *___|      |__20__|      |__20__|      |_____220____|      |__20__|      |_____
 *
 * Open left in us
 *   |--20--|      |--20--|      |--20--|            |--20--|      |--20--|     
 *___|      |__20__|      |__20__|      |_____140____|      |__20__|      |_____
 *
 * Close left in us
 *   |--20--|      |--20--|      |--20--|            |--20--|      |--20--|     
 *___|      |__20__|      |__20__|      |_____380____|      |__20__|      |_____
 *
 * Open right in us
 *   |--20--|      |--20--|      |--20--|            |--20--|      |--20--|     
 *___|      |__20__|      |__20__|      |_____300____|      |__20__|      |_____
 */

STM32Timer ITimer0(TIM1);

const uint8_t close_Right_bArray[3] = {0xA8,0x00,0xA0};
const uint8_t open_Left_bArray[2]  = {0xA8,0x0A};
const uint8_t close_Left_bArray[4]  = {0xA8,0x00,0x00,0xA0};
const uint8_t open_Right_bArray[3]   = {0xA8,0x00,0x0A};

enum eEye_State {
  Uninitialized = 0,
  StateA_OpenRight,
  StateB_BothClosed,
  StateC_OpenLeft,
  StateD_BothClosed,
};

eEye_State Eye_State;

void Transmit_Eye_Update(){
  switch (Eye_State){
  case StateA_OpenRight:
    SPI.transfer((uint8_t*)close_Right_bArray,sizeof(close_Right_bArray));
    Eye_State = StateB_BothClosed;
    break;
  case StateB_BothClosed:
    SPI.transfer((uint8_t*)open_Left_bArray,sizeof(open_Left_bArray));
    Eye_State = StateC_OpenLeft;
    break;
  case StateC_OpenLeft:
    SPI.transfer((uint8_t*)close_Left_bArray,sizeof(close_Left_bArray));
    Eye_State = StateD_BothClosed;
    break;
  case StateD_BothClosed:
    SPI.transfer((uint8_t*)open_Right_bArray,sizeof(open_Right_bArray));
    Eye_State = StateA_OpenRight;
    break;
  default:
    break;
  }
}

void  TimerHandlerIR()
{
  static bool toggle0 = false;
  static bool started = false;

  if (!started)
  {
    started = true;
    pinMode(LED_BUILTIN, OUTPUT);
  }

  digitalWrite(LED_BUILTIN, toggle0);
  toggle0 = !toggle0;
}

void setup() {
  SerialUSB.begin(115200);
  // start the SPI library:
  SPI.begin();
  SPI.beginTransaction(SPISettings(50000, MSBFIRST, SPI_MODE0));
  Eye_State = StateA_OpenRight;

  while (!SerialUSB) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  SerialUSB.println("Goodnight moon!");
  // put your setup code here, to run once:
    pinMode(sensorPin, INPUT_ANALOG);
    pinMode(yLedC, OUTPUT);
    digitalWrite(yLedC,0);
    pinMode(irLedC, OUTPUT);
    digitalWrite(irLedC,0);

    pinMode(PC13, OUTPUT);
}



void loop() {
  // SerialUSB.println(analogRead(sensorPin));
  // put your main code here, to run repeatedly:
  if (SerialUSB.available()){
  SerialUSB.println(SerialUSB.read());
  }

  delay(5);
}