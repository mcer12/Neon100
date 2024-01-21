
void transferBytes(uint8_t *data) {
  digitalWriteFast(LATCH, LOW);
  for (int i = 0; i < registersCount; i++) {
    SPI_3.transfer(data[i]); //Transfers data to the second device. The SPI_3 instance is configured with the right settings
  }
  digitalWriteFast(LATCH, HIGH);
}

void TimerHandler()
{
  registersData[1] = B00000000; // turn off all anodes / full column

  if (activeColumn >= columns) {
    registersData[0] = cathodesData[scanCathodes]; // reset cathode
    transferBytes(registersData);
    blank++;
    if (blank >= resetCycles) {
      blank = 0;
      activeColumn = 0;
    }
    return;
  }

  registersData[0] = cathodesData[activeColumn % scanCathodes]; // cathode byte

  if (blank == 0) {
    registersData[1] = B00000000;
    transferBytes(registersData); // remove ghosting at the front
    for (byte y = 0; y < rows; y++) {
      bitWrite(registersData[1], anodesPins[y], bitRead(buffer[(activeColumn / 8) + (y * bytes_per_row )], activeColumn % 8));
    }
    transferBytes(registersData);
  }
  else if (blank == BLANK_ITERATOR) {
    registersData[1] = B00000000;
    transferBytes(registersData); // remove ghosting at the front
    blank = 0;
    activeColumn++;
    return;
  }
  else {
    for (byte y = 0; y < rows; y++) {
      bitWrite(registersData[1], anodesPins[y], bitRead(buffer[(activeColumn / 8) + (y * bytes_per_row )], activeColumn % 8));
    }
    transferBytes(registersData);
  }

  blank += 1;
  if (blank > BLANK_ITERATOR) blank = 0;
}

bool displayInit() {
  pinMode(pinNametoDigitalPin(DATA), OUTPUT);
  pinMode(pinNametoDigitalPin(CLK), OUTPUT);
  pinMode(pinNametoDigitalPin(LATCH), OUTPUT);

  SPI_3.begin();
  SPI_3.beginTransaction(SPISettings(SPI_SPEED, MSBFIRST, SPI_MODE0)); //Attaches another CS pin and configure the SPI_3 instance with other settings

  if ((!buffer) && !(buffer = (uint8_t*)malloc(bytes_per_row * rows))) {
    return false;
  }
  if ((!wireBuffer) && !(wireBuffer = (uint8_t*)malloc(bytes_per_row * rows))) {
    return false;
  }

  ITimer.attachInterruptInterval(TIMER_INTERVAL_uS, TimerHandler);

  memset(buffer, 0, bytes_per_row * rows);
  memset(wireBuffer, 0, bytes_per_row * rows);

  return true;
}

void setDisplayBri(byte brightness) {
  bri = brightness;
}

void translateBuffer() {
  memcpy(buffer, wireBuffer, bytes_per_row * rows);
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
