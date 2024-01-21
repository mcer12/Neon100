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

//#ifndef USE_SERIAL
#define TIMER_INTERRUPT_DEBUG 0
#define _TIMERINTERRUPT_LOGLEVEL_ 0
//#endif

#define USE_ISR

#include <SPI.h>
#include <Wire.h>
#include "STM32TimerInterrupt.h"

#define OE_ANODES PB_0
#define OE_CATHODES PB_1
#define DATA PA_7
#define CLK PA_5
#define LATCH PA_4
#define I2C_SDA PB11
#define I2C_SCL PB10

#define SPI_SPEED 30000000
#define ISR_TIMER_INTERVAL_uS 80

#define SCREEN_RES_W 100
#define SCREEN_RES_H 100
#define BYTES_PER_ROW ((SCREEN_RES_W + 7) / 8)
#define swap(a, b) (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) ///< No-temp-var swap operation


#define BLACK 0   ///< Draw 'off' pixels
#define WHITE 1   ///< Draw 'on' pixels

#define I2C_ADDR 0x08

#define COMMAND_UPDATE 0x20
#define COMMAND_SET_BRIGHTNESS 0x25

#define BRI_LOW 0x01
#define BRI_MED 0x02
#define BRI_HIGH 0x03


SPIClass SPI_3(pinNametoDigitalPin(DATA), pinNametoDigitalPin(PA_6), pinNametoDigitalPin(CLK)); // MOSI  MISO  SCLK

STM32Timer timer(TIM1);

const int cols = 100;
const int rows = 100;
const int registersCount = 26;

volatile uint8_t timerCounter;
volatile uint8_t timerCounterMax = 1;
volatile uint8_t timerCounterMaxDefault = 1;
volatile uint8_t timerCounterMaxFirstRow = 2;

int activeRow = 0;
int bri = BRI_HIGH;
bool scanBack;

uint8_t *buffer;
uint8_t *wireBuffer;
byte registersData[registersCount];

volatile bool receivingData = false;
volatile bool dataReady = false;

const uint8_t anodeRegisters[registersCount / 2] = {0, 1, 2, 3, 4, 5, 19, 18, 17, 16, 15, 14, 13};
const uint8_t cathodeRegisters[registersCount / 2] = {20, 21, 22, 23, 24, 25, 12, 11, 10, 9, 8, 7, 6};
const uint8_t anodes[rows] = {
  99, //99
  40, //98
  98, //97
  41, //96
  97, //95
  42, //94
  96, //93
  43, //92
  95, //91
  44, //90
  94, //89
  45, //88
  93, //87
  46, //86
  92, //85
  47, //84
  91, //83
  32, //82
  90, //81
  33, //80
  89, //79
  34, //78
  88, //77
  35, //76
  87, //75
  36, //74
  86, //73
  37, //72
  85, //71 // !!!!
  38, //70
  84, //69
  39, //68
  83, //67
  24, //66
  82, //65
  25, //64
  81, //63
  26, //62
  80, //61
  27, //60
  79, //59
  28, //58
  78, //57
  29, //56
  77, //55
  30, //54
  76, //53
  31, //52
  75, //51
  16, //50
  74, //49
  17, //48
  73, //47
  18, //46
  72, //45
  19, //44
  71, //43
  20, //42
  70, //41
  21, //40
  69, //39
  22, //38
  68, //37
  23, //36
  67, //35
  8, //34
  66, //33
  9, //32
  65, //31
  10, //30
  64, //29
  11, //28
  63, //27
  12, //26
  62, //25
  13, //24
  61, //23
  14, //22
  60, //21
  15, //20
  59, //19
  0, //18
  58, //17
  1, //16
  57, //15
  2, //14
  56, //13 !!!
  3, //12
  55, //11
  4, //10
  54, //9
  5, //8
  53, //7
  6, //6
  52, //5
  7, //4
  51, //3
  48, //2
  50, //1
  49, //0
};

const uint8_t cathodes[rows] = {
  49, //0
  50, //1
  48, //2
  51, //3
  7, //4
  52, //5
  6, //6
  53, //7
  5, //8
  54, //9
  4, //10
  55, //11
  3, //12
  56, //13
  2, //14
  57, //15
  1, //16
  58, //17
  0, //18
  59, //19
  15, //20
  60, //21
  14, //22
  61, //23
  13, //24
  62, //25
  12, //26
  63, //27
  11, //28
  64, //29
  10, //30
  65, //31
  9, //32
  66, //33
  8, //34
  67, //35
  23, //36
  68, //37
  22, //38
  69, //39
  21, //40
  70, //41
  20, //42
  71, //43
  19, //44
  72, //45
  18, //46
  73, //47
  17, //48
  74, //49
  16, //50
  75, //51
  31, //52
  76, //53
  30, //54
  77, //55
  29, //56
  78, //57
  28, //58
  79, //59
  27, //60
  80, //61
  26, //62
  81, //63
  25, //64
  82, //65
  24, //66
  83, //67
  39, //68
  84, //69
  38, //70
  85, //71
  37, //72
  86, //73
  36, //74
  87, //75
  35, //76
  88, //77
  34, //78
  89, //79
  33, //80
  90, //81
  32, //82
  91, //83
  47, //84
  92, //85
  46, //86
  93, //87
  45, //88
  94, //89
  44, //90
  95, //91
  43, //92
  96, //93
  42, //94
  97, //95
  41, //96
  98, //97
  40, //98
  99, //99
};



void setup() {

  displayInit();
  Serial.begin(115200);

  i2cInit();

  delay(500);
  /*
    for (int i = 0; i < 100; i++) {
      setPixel(i, i, 1);
      setPixel(99 - i, i, 1);
      setPixel(0, i, 1);
      setPixel(25, i, 1);
      setPixel(50, i, 1);
      setPixel(75, i, 1);
      setPixel(99, i, 1);
      setPixel(i, 0, 1);
      setPixel(i, 99, 1);
    }
  */
  int gap = 2;
  int repeat = 50;
  for (int x = 0; x < repeat; x++) {
    for (int y = 0; y < repeat; y++) {
      setPixel(x * gap, y * gap, 1);
      //setPixel(x * gap, y * gap + 1, 1);
      //setPixel(x * gap, y * gap + 2, 1);
    }
  }


  //then = millis();
  translateBuffer();
  //Serial.println(millis() - then);

  //Serial.println("BUFFER");
  //printBuffer(buffer);

  //Serial.println("BUFFER 2");
  //printBuffer(buffer2);

}


void loop() {

#ifndef USE_ISR
  for (byte i = 0; i < BYTES_PER_ROW; i++) {
    registersData[cathodeRegisters[i]] = B00000000;
    registersData[anodeRegisters[i]] = B00000000;

    if (anodes[activeRow] / 8 == i) {
      bitSet(registersData[anodeRegisters[i]], anodes[activeRow] % 8);
    }
  }

  transferBytes(registersData);

  delayMicroseconds(10);

  if (dataReady && activeRow == 0) {
    translateBuffer();
    dataReady = false;
  }

  for (byte i = 0; i < BYTES_PER_ROW; i++) {
    registersData[cathodeRegisters[i]] = buffer[(anodes[activeRow] * BYTES_PER_ROW) + i];
    registersData[anodeRegisters[i]] = B00000000;

    if (anodes[activeRow] / 8 == i) {
      bitSet(registersData[anodeRegisters[i]], anodes[activeRow] % 8);
    }
  }

  transferBytes(registersData);

  if (activeRow == 0) {
    delayMicroseconds(140);
  } else {
    delayMicroseconds(70);
  }

  activeRow++;
  if (activeRow >= rows) {
    activeRow = 0;
  }
#endif

  /*
    activeRow--;
    if (activeRow < 0) {
      activeRow = rows - 1;
    }
  */

  /*
      if (scanBack) {
        activeRow--;
        if (activeRow < 0) {
          activeRow = 0;
          scanBack = false;
        }
      } else {
        activeRow++;
        if (activeRow >= rows) {
          activeRow = rows - 1;
          scanBack = true;
        }
      }
  */


#ifdef USE_ISR
  if (dataReady && activeRow == 0) {
    translateBuffer();
    dataReady = false;
  }
#endif

}
