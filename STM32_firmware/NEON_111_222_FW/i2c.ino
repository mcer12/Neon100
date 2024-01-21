void i2cInit() {
  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);

  Wire.begin(I2C_ADDR); // join i2c bus with address #4
  Wire.setClock(400000);
  Wire.onReceive(receiveEvent); // register event
  //Wire.onRequest(requestEvent); // register event
}

void receiveEvent(int howMany)
{
  uint8_t receiveBuffer[bytes_per_row + 2]; // buffer for a single received row
  int iterator = 0;

  while (Wire.available()) // loop through all but the last
  {
    receiveBuffer[iterator] = Wire.read();    // receive byte as an integer
    iterator++;
  }

  if (receiveBuffer[0] == COMMAND_SET_BRIGHTNESS) {
    setDisplayBri(receiveBuffer[1]);
  }

  if (receiveBuffer[0] == COMMAND_UPDATE) {

    for (int i = 0; i < bytes_per_row; i++) {
      wireBuffer[(receiveBuffer[1] * bytes_per_row) + i] = receiveBuffer[i + 2];
    }

    if (receiveBuffer[1] == rows - 1) {
      dataReady = true;
    }
  }
}
