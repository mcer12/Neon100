#include <Wire.h>    // I2C library
#include <Adafruit_GFX.h>    // Core graphics library
#include "Neon100.h"
#include "5x7_cz_square_thin.h"

/*
  Default pinout (changeable on some platforms)
  Platform: SDA, SCL
  ESP8266: 4, 5 (don't mistake with D4, D5)
*/

int width = 222;
int height = 7;
int rotation = 2; // 2 for 111 display, 0 for 222 display

Neon100 display(4, 5, width, height); // Explicitly set I2C pins, doesn't work on atmega

void setup() {
  // Enable serial
  Serial.begin(115200);

  display.begin();

  display.setRotation(rotation);
  display.setTextColor(WHITE);
}

void loop() {

  display.clear();
  display.drawLine(0, 0, width - 1, 0, WHITE);
  display.drawLine(0, height - 1, width - 1, height - 1, WHITE);
  display.drawLine(0, 0, 0, height - 1, WHITE);
  display.drawLine(width - 1, 0, width - 1, height - 1, WHITE);

  display.update();

  delay(3000);

  display.clear();
  display.setFont(&Font5x7Thin);
  display.setCursor(0, 7);
  display.println(F("Testing the display..."));
  display.update();

  for (int i = 0; i < width - 1; i++) {
    display.clear();
    display.setCursor(i, 7);
    display.println("weee");
    display.update();
    delay(30);
  }
  display.clear();
  display.update();

  display.setCursor(0, 7);
  display.println(F("Display seems to work!"));
  display.update();
  Serial.println("text set");

  delay(3000);

  /*
    display.fill();
    display.update();
    delay(delayMs);

    display.clear();
    display.update();
    delay(delayMs);

    display.fill();
    display.update();
    delay(delayMs);

    display.clear();
    display.update();
    delay(delayMs);

    display.fill();
    display.update();
    delay(delayMs);

    display.clear();
    display.update();
    delay(delayMs);

    display.fill();
    display.update();
    delay(delayMs);

    display.clear();
    display.update();
    delay(delayMs);
  */
}