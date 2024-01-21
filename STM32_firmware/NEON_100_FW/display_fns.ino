
void setPixel(uint8_t x, uint8_t y, bool val) {
  bitWrite(wireBuffer[(x / 8) + (y * BYTES_PER_ROW)], x % 8 , val);
}

uint8_t flipByte(uint8_t c) {
  c = ((c >> 1) & 0x55) | ((c << 1) & 0xAA);
  c = ((c >> 2) & 0x33) | ((c << 2) & 0xCC);
  c = (c >> 4) | (c << 4) ;

  return c;
}

void transferBytes(uint8_t *data) {
  digitalWriteFast(LATCH, LOW);
  //  SPI_3.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0)); //Attaches another CS pin and configure the SPI_3 instance with other settings
  for (int i = 0; i < registersCount; i++) {
    SPI_3.transfer(data[i]); //Transfers data to the second device. The SPI_3 instance is configured with the right settings
  }
  //SPI_3.end(); //SPI_3 instance is disabled
  digitalWriteFast(LATCH, HIGH);
}


void TimerHandler()
{
  if (timerCounter == 0) {
    for (byte i = 0; i < BYTES_PER_ROW; i++) {
      registersData[cathodeRegisters[i]] = B00000000;
      registersData[anodeRegisters[i]] = B00000000;

      if (anodes[activeRow] / 8 == i) {
        bitSet(registersData[anodeRegisters[i]], anodes[activeRow] % 8);
      }
    }

    transferBytes(registersData);

    //digitalWriteFast(OE_ANODES, LOW);
    //digitalWriteFast(OE_CATHODES, LOW);
  }

  if (timerCounter == 1) {
    for (byte i = 0; i < BYTES_PER_ROW; i++) {
      registersData[cathodeRegisters[i]] = buffer[(anodes[activeRow] * BYTES_PER_ROW) + i];
      registersData[anodeRegisters[i]] = B00000000;

      if (anodes[activeRow] / 8 == i) {
        bitSet(registersData[anodeRegisters[i]], anodes[activeRow] % 8);
      }
    }

    transferBytes(registersData);

    //digitalWriteFast(OE_ANODES, LOW);
    //digitalWriteFast(OE_CATHODES, LOW);


    if (activeRow == 0) {
      timerCounterMax = timerCounterMaxFirstRow;
    } else {
      timerCounterMax = timerCounterMaxDefault;
    }

  }


  timerCounter++;
  if (timerCounter > timerCounterMax) {
    timerCounter = 0;

    activeRow++;
    if (activeRow >= rows) {
      activeRow = 0;
    }

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
  }


}



bool displayInit() {
  pinMode(pinNametoDigitalPin(OE_ANODES), OUTPUT);
  pinMode(pinNametoDigitalPin(OE_CATHODES), OUTPUT);
  pinMode(pinNametoDigitalPin(DATA), OUTPUT);
  pinMode(pinNametoDigitalPin(CLK), OUTPUT);
  pinMode(pinNametoDigitalPin(LATCH), OUTPUT);

  SPI_3.begin();
  SPI_3.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0)); //Attaches another CS pin and configure the SPI_3 instance with other settings

  // 74HC595 OE is inverted, HIGH disables the pins
  digitalWriteFast(OE_ANODES, HIGH);
  digitalWriteFast(OE_CATHODES, HIGH);

  blank();

  digitalWriteFast(OE_ANODES, LOW);
  digitalWriteFast(OE_CATHODES, LOW);

  if ((!buffer) && !(buffer = (uint8_t*)malloc(BYTES_PER_ROW * SCREEN_RES_H))) {
    return false;
  }
  if ((!wireBuffer) && !(wireBuffer = (uint8_t*)malloc(BYTES_PER_ROW * SCREEN_RES_H))) {
    return false;
  }

  memset(buffer, 0, BYTES_PER_ROW * SCREEN_RES_H);
  memset(wireBuffer, 0, BYTES_PER_ROW * SCREEN_RES_H);

#ifdef USE_ISR
  timer.attachInterruptInterval(ISR_TIMER_INTERVAL_uS, TimerHandler);
#endif

  return true;
}

void translateBuffer() {
  uint8_t val = 0;
  for (int x = 0; x < SCREEN_RES_W; x++) {
    for (int y = 0; y < SCREEN_RES_H; y++) {
      val = bitRead(wireBuffer[(x / 8) + (y * BYTES_PER_ROW)], x % 8);
      bitWrite(buffer[(cathodes[x] / 8) + (anodes[y] * BYTES_PER_ROW)], cathodes[x] % 8, val);
    }
  }
}

void setDisplayBri(byte brightness) {
  bri = brightness;
}

/*
  void printBuffer(uint8_t *data) {
  Serial.println("");
  Serial.print("     ");
  for (int i = 0; i < BYTES_PER_ROW * 8; i++) {
    Serial.print(" ");
    Serial.print(i);
    if (i < 10) Serial.print(" ");
    Serial.print("");
  }

  Serial.println("");
  Serial.print("     ");
  for (int i = 0; i < BYTES_PER_ROW * 8; i++) {
    Serial.print(" ");
    Serial.print("|");
    Serial.print(" ");
  }

  int row = 0;
  for (int i = 0; i < BYTES_PER_ROW * SCREEN_RES_H; i++) {
    if (i % (BYTES_PER_ROW) == 0) {
      row++;
      Serial.println("");
      Serial.print(row - 1);
      if ((row - 1) < 10) Serial.print(" ");
      Serial.print(" - ");
    }
    for (int b = 0; b < 8; b++) {
      Serial.print(" ");
      Serial.print(bitRead(data[i], b));
      Serial.print(" ");
    }
  }
  }
*/

void blank() {
  digitalWriteFast(LATCH, LOW);
  //  SPI_3.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0)); //Attaches another CS pin and configure the SPI_3 instance with other settings
  for (int i = 0; i < registersCount; i++) {
    SPI_3.transfer(B00000000); // Transfer zero to set all outputs low.
  }
  //SPI_3.end(); //SPI_3 instance is disabled
  digitalWriteFast(LATCH, HIGH);
}
