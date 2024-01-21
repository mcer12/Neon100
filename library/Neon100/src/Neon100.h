
#ifndef Neon100_h

#define Neon100_h

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>

#define SCREEN_RES_W 100
#define SCREEN_RES_H 100

#define BLACK 0   ///< Draw 'off' pixels
#define WHITE 1   ///< Draw 'on' pixels

#define ADDR_MASTER_DEFAULT 0x42
#define ADDR_SLAVE_DEFAULT 0x08

#define COMMAND_UPDATE 0x20
#define COMMAND_SET_SPEED 0x23
#define COMMAND_SET_ANIMATION 0x24
#define COMMAND_SET_BRIGHTNESS 0x25
#define COMMAND_TOGGLE_LED_POWER 0x26

#define SPEED_HIGH 0x01
#define SPEED_MED 0x02
#define SPEED_LOW 0x03
#define SPEED_VERY_LOW 0x04
#define SPEED_EXTREMELY_LOW 0x05
#define SPEED_LOWEST 0x06

#define BRI_LOW 0x01
#define BRI_MED 0x02
#define BRI_HIGH 0x03

#define ANIM_SLIDE_RIGHT 0x01
#define ANIM_SLIDE_LEFT 0x02
#define ANIM_SLIDE_FROM_TOP 0x03
#define ANIM_SLIDE_FROM_BOTTOM 0x04
#define ANIM_DISSOLVE 0x05
#define ANIM_SLIDE_RIGHT_FORCED 0x06
#define ANIM_SLIDE_LEFT_FORCED 0x07
#define ANIM_SLIDE_FROM_TOP_FORCED 0x08
#define ANIM_SLIDE_FROM_BOTTOM_FORCED 0x09

#define LED_PWR_ON 0x11
#define LED_PWR_OFF 0x10

#define STATUS_READY 0x11
#define STATUS_BUSY 0x10

#define swap(a, b) (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) ///< No-temp-var swap operation

class Neon100 : public Adafruit_GFX {

public:

Neon100(int16_t sda = -1, int16_t scl = -1, uint8_t addr = ADDR_SLAVE_DEFAULT, int res_x = -1, int res_y = -1, TwoWire *twi = &Wire);
Neon100(int16_t sda = -1, int16_t scl = -1, int res_x = -1, int res_y = -1, TwoWire *twi = &Wire);
Neon100(int res_x = -1, int res_y = -1, TwoWire *twi = &Wire);

bool begin(void);
void drawPixel(int16_t x, int16_t y, uint16_t color);
void clear(void);
void fill(void);
void invert(void);
void update(void);
void setBrightness(uint8_t b);
uint8_t *getBuffer(void);

void command(uint8_t command, uint8_t data);

protected:

TwoWire *wire;
uint8_t *buffer;
uint8_t _addr;
int16_t _sda;
int16_t _scl;
int _res_x;
int _res_y;

int _bytes_per_row;
uint8_t _pixel_skip;

bool sendBuffer(uint8_t command, uint8_t *data, bool async);

};

#endif
