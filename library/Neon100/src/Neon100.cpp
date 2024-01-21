/*
#ifdef __AVR__
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_MBED_RP2040)|| defined(ARDUINO_ARCH_RP2040)
#include <pgmspace.h>
#else
#define pgm_read_byte(addr)                                                    \
  (*(const unsigned char *)(addr)) ///< PROGMEM workaround for non-AVR
#endif
*/

#if !defined(__ARM_ARCH) && !defined(ENERGIA) && !defined(ESP8266) &&          \
    !defined(ESP32) && !defined(__arc__)
#include <util/delay.h>
#endif

#include "Neon100.h"

Neon100::Neon100(int16_t sda, int16_t scl, uint8_t addr, int res_x, int res_y, TwoWire *twi) : Adafruit_GFX((res_x > 0) ? res_x : SCREEN_RES_W, (res_y > 0) ? res_y : SCREEN_RES_H), wire(twi ? twi : &Wire)
{

	_addr = addr;
	_sda = sda;
	_scl = scl;
	_res_x = res_x;
	_res_y = res_y;
}

Neon100::Neon100(int16_t sda, int16_t scl, int res_x, int res_y, TwoWire *twi) : Adafruit_GFX((res_x > 0) ? res_x : SCREEN_RES_W, (res_y > 0) ? res_y : SCREEN_RES_H), wire(twi ? twi : &Wire)
{

	_addr = ADDR_SLAVE_DEFAULT;
	_sda = sda;
	_scl = scl;
	_res_x = res_x;
	_res_y = res_y;
}

Neon100::Neon100(int res_x, int res_y, TwoWire *twi) : Adafruit_GFX((res_x > 0) ? res_x : SCREEN_RES_W, (res_y > 0) ? res_y : SCREEN_RES_H), wire(twi ? twi : &Wire)
{

	_addr = ADDR_SLAVE_DEFAULT;
	_sda = -1;
	_scl = -1;
	_res_x = res_x;
	_res_y = res_y;
}

bool Neon100::begin() {

if(_res_x <= 0) _res_x = SCREEN_RES_W;
if(_res_y <= 0) _res_y = SCREEN_RES_H;

_bytes_per_row =  (_res_x + 7) / 8;
_pixel_skip = (_bytes_per_row * 8) - _res_y;

 if ((!buffer) && !(buffer = (uint8_t*)malloc(_bytes_per_row * _res_y))){
 return false;
 }

if(_sda >= 0 && _scl >= 0){
#if defined(ARDUINO_ARDUINO_NANO33BLE) || defined(ARDUINO_ARCH_MBED_RP2040) || defined(ARDUINO_ARCH_RP2040) || defined(STM32F1xx)
    wire->setSDA(_sda);
    wire->setSCL(_scl);
#elif defined(ESP32)
    wire->setPins(_sda, _scl);
#elif defined(ESP8266)
    wire->begin(_sda, _scl);
#endif

} else{
#if defined(ESP8266)
    wire->begin();
#endif
}
#if !defined(ESP8266)
  wire->begin();
#endif

  wire->setClock(400000);

    clear();

    return true;
}

void Neon100::drawPixel(int16_t x, int16_t y, uint16_t color) {

if(x >= width() || y >= height()) return;

bool val = color;
    switch (getRotation()) {
    case 0:
      //swap(x,y);

      break;
    case 1:
        y = height() - y - 1;
        swap(x,y);
      break;
    case 2:
      x = width() - x - 1;
      y = height() - y - 1;
      //swap(x,y);
      break;
    case 3:
        x = width() - x - 1;
        swap(x,y);
      break;
    }

    x = x + (_pixel_skip * (x / _res_x));
    bitWrite(buffer[(x / 8) + (y * _bytes_per_row )], x % 8 , val);

}


void Neon100::clear() {
  for (int i = 0; i < _bytes_per_row * _res_y; i++) {
        buffer[i] = 0;
  }
}

void Neon100::fill() {
  for (int i = 0; i < _bytes_per_row * _res_y; i++) {
        buffer[i] = 0xff;
  }
}

void Neon100::invert() {
      for (int i = 0; i < _bytes_per_row * _res_y; i++) {
            buffer[i] = ~buffer[i];
      }
}


void Neon100::setBrightness(uint8_t bri) {
  command(COMMAND_SET_BRIGHTNESS, bri);
}

void Neon100::update() {
  sendBuffer(COMMAND_UPDATE, buffer, false);
}



void Neon100::command(uint8_t command, uint8_t data) {
  wire->beginTransmission(_addr); // transmit to device #8
  wire->write(command); // sending the designator
  wire->write(data); // sending the buffer in 4 chunks
  wire->endTransmission();    // stop transmitting
}

bool Neon100::sendBuffer(uint8_t command, uint8_t *data, bool async) {

  uint8_t payload[_bytes_per_row];

    for (int row = 0; row < _res_y; row++) {
            int startByte = row * _bytes_per_row;

              wire->beginTransmission(_addr); // transmit to device #8
              wire->write(command); // sending the designator
              wire->write(row); // row number

            for (int b = 0; b < _bytes_per_row; b++) {
                  payload[b] = buffer[startByte + b];
            }

          wire->write(payload, _bytes_per_row); // sending the designator
          wire->endTransmission();    // stop transmitting
        
    }
  return true;
}