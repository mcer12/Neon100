/*

  IDE SETTINGS:
  UART: Enabled
  USB: Enabled (disable for low memory chips)
  Optimize: Fastest + LTO
  Upload method: SWD (or HID if you flashed HID bootloader beforehand)


  FW SIZE
  STM32F103C6T6 (32k flash):
  sketch uses 98% of memory with USB support disabled with either DISPLAY_COUNT_LIMIT set to 2 or some animations disabled
  STM32F103C8T6 (64k flash):
  sketch uses 51% of memory with USB support disabled
  sketch uses 78% of memory with USB support & flashing via HID bootloader

  COMMANDS

  Action (first byte)
  0x20 - update
  0x25 - set brightness (leds only)

  Brightness (only for leds) 0x25
  0x01 - low
  0x02 - med
  0x03 - high

  Animation 0x24

  ANIM_SLIDE_RIGHT 0x01
  ANIM_SLIDE_LEFT 0x02
  ANIM_SLIDE_FROM_TOP 0x03
  ANIM_SLIDE_FROM_BOTTOM 0x04
  ANIM_DISSOLVE 0x05
  ANIM_SLIDE_RIGHT_FORCED 0x06
  ANIM_SLIDE_LEFT_FORCED 0x07
  ANIM_SLIDE_FROM_TOP_FORCED 0x08
  ANIM_SLIDE_FROM_BOTTOM_FORCED 0x09

  Leds power 0x26
  0x11 - on
  0x10 - off

  Status response (5 bytes):

  status (ready / busy)
  speed
  brightness
  animation
  led power

*/

//#define DISPLAY_TYPE_222x7
#define DISPLAY_TYPE_111x7

//#ifndef USE_SERIAL
#define TIMER_INTERRUPT_DEBUG 0
#define _TIMERINTERRUPT_LOGLEVEL_ 0
//#endif

#include <SPI.h>
#include <Wire.h>
#include "STM32TimerInterrupt.h"

#define DATA PA_7
#define CLK PA_5
#define LATCH PA_4
#define I2C_SDA PB7
#define I2C_SCL PB6

#define TIMER_INTERVAL_uS 45 // ISR interval
//#ifdef DISPLAY_TYPE_222x7
//#define TIMER_INTERVAL_uS 45
//#endif
#define BLANK_ITERATOR 1 // use value 1 or 2. 1 = higher refresh, lower bri, lower regulator heating. 2 = lower refresh, higher bri, higher regulator heating
#define MIN_RESET_PERIOD_uS 250 // minimum time to stay at reset cathode.
//#ifdef DISPLAY_TYPE_111x7
//#define MIN_RESET_PERIOD_uS 250 // minimum time to stay at reset cathode.
//#endif
#define SPI_SPEED 20000000

#define I2C_ADDR 0x08
#define COMMAND_UPDATE 0x20
#define COMMAND_SET_BRIGHTNESS 0x25

#define BRI_LOW 0x01
#define BRI_MED 0x02
#define BRI_HIGH 0x03

SPIClass SPI_3(pinNametoDigitalPin(DATA), pinNametoDigitalPin(PA_6), pinNametoDigitalPin(CLK)); // MOSI  MISO  SCLK
STM32Timer ITimer(TIM1);

#if defined(DISPLAY_TYPE_222x7)

const uint8_t registersCount = 2;
const uint8_t columns = 222;
const uint8_t rows = 7;
const uint8_t scanCathodes = 5; // number of scan cathodes without reset cathode
uint8_t anodesPins[rows] = { // + 1 for the reset cathode
  1,
  2,
  3,
  4,
  5,
  6,
  7
};
byte cathodesData[scanCathodes + 1] = {
  B00000100, // C1
  B00001000, // C2
  B00010000, // C3
  B10000000, // C5
  B01000000, // C4
  B00100000, // RESET
};

#elif defined(DISPLAY_TYPE_111x7)

const uint8_t registersCount = 2;
const uint8_t columns = 111;
const uint8_t rows = 7;
const uint8_t scanCathodes = 3; // number of scan cathodes without reset cathode
uint8_t anodesPins[rows] = { // + 1 for the reset cathode
  3,
  2,
  1,
  4,
  5,
  6,
  7
};
byte cathodesData[scanCathodes + 1] = {
  B00010000, // A
  B00100000, // B
  B01000000, // C
  B10000000, // RESET
};

#endif

const uint8_t bytes_per_row = ((columns + 7) / 8);
const byte resetCycles = (MIN_RESET_PERIOD_uS + TIMER_INTERVAL_uS) / TIMER_INTERVAL_uS; // number of ISR cycles to stay on reset cathode

int bri = BRI_HIGH;

uint8_t *buffer;
uint8_t *wireBuffer;
byte registersData[registersCount];

volatile bool receivingData = false;
volatile bool dataReady = false;
volatile uint8_t activeColumn = 0;
volatile uint8_t blank = 0;

void setup() {

  displayInit();
  Serial.begin(115200);

  i2cInit();

}


void loop() {

  if (dataReady && activeColumn >= columns && blank == 0) {
    translateBuffer();
    dataReady = false;
  }

}
