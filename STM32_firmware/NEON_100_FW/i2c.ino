void i2cInit() {
  //Wire.setClock(400000);
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);

  Wire.begin(I2C_ADDR); // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  //Wire.onRequest(requestEvent); // register event
}

/*
  unsigned char reverse(unsigned char b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
  }
*/

void receiveEvent(int howMany)
{
  uint32_t bigNum;
  uint8_t receiveBuffer[BYTES_PER_ROW + 2]; // buffer for a single received row
  int iterator = 0;

  while (Wire.available()) // loop through all but the last
  {
    receiveBuffer[iterator] = Wire.read();    // receive byte as an integer
    //Serial.println(wireBuffer[iterator]);         // print the integer
    iterator++;
  }
  //Serial.print("Action: ");
  //Serial.println(receiveBuffer[0]);

  //Serial.print("Row: ");
  //Serial.println(receiveBuffer[1]);

  if (receiveBuffer[0] == COMMAND_SET_BRIGHTNESS) {
    setDisplayBri(receiveBuffer[1]);
  }

  if (receiveBuffer[0] == COMMAND_UPDATE) {

    for (int i = 0; i < BYTES_PER_ROW; i++) {
      wireBuffer[(receiveBuffer[1] * BYTES_PER_ROW) + i] = receiveBuffer[i + 2];
    }

    /*
        Serial.print("data: ");
        Serial.print(wireBuffer[3]);
        Serial.print(" ");
        Serial.print(wireBuffer[4]);
        Serial.print(" ");
        Serial.print(wireBuffer[5]);
        Serial.print(" ");
        Serial.print(wireBuffer[6]);
        Serial.println("");
    */
    if (receiveBuffer[1] == SCREEN_RES_H - 1) {
      dataReady = true;
    }
  }
}
