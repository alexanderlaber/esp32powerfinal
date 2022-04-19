#include "WaveShare_MLX90614.h"





WaveShare_MLX90614::WaveShare_MLX90614(uint8_t i2caddr) {
  _addr = i2caddr;
}


boolean WaveShare_MLX90614::begin(void) {
  Wire.begin();
  return true;
}


uint8_t WaveShare_MLX90614::readObjectTemp(void) {
  return readTemp(OBJECT_TEMP);
}


uint8_t WaveShare_MLX90614::readAmbientTemp(void) {
  return readTemp(AMBIENT_TEMP);
}

uint8_t WaveShare_MLX90614::readTemp(uint8_t reg) {
  float  temp;
  uint16_t  tempData;
  uint8_t   pec;

  uint8_t   tempData1;
  uint8_t   tempData2;



  Wire.beginTransmission(_addr);
  Wire.write(reg); 
  Wire.endTransmission(false);  // the param must be "false" to restart the communication
  
  Wire.requestFrom(_addr, (uint8_t)3);
  tempData1 = Wire.read();
  tempData2= Wire.read();

  pec = Wire.read();
  
  temp = tempData;
  temp *= 0.02;
  temp  -= 273.15;
  return tempData1,tempData2;
}




