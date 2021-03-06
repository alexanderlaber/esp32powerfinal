#include "WaveShare_MLX90614.h"





WaveShare_MLX90614::WaveShare_MLX90614(uint8_t i2caddr) {
  _addr = i2caddr;
}


boolean WaveShare_MLX90614::begin(void) {
  Wire.begin();
  return true;
}

/*
uint8_t WaveShare_MLX90614::readObjectTemp(void) {
  return readTemp(OBJECT_TEMP);
}


uint8_t WaveShare_MLX90614::readAmbientTemp(void) {
  return readTemp(AMBIENT_TEMP);
}*/

irOutput WaveShare_MLX90614::readTemp(void) {

  uint8_t   pec;

  irOutput irres;

  Wire.beginTransmission(_addr);
  Wire.write(OBJECT_TEMP);
  Wire.endTransmission(false);  // the param must be "false" to restart the communication
  Wire.requestFrom(_addr, (uint8_t)3);
  irres.packagetemp1 = Wire.read();
  irres.packagetemp2= Wire.read();
  pec = Wire.read();

  Wire.beginTransmission(_addr);
  Wire.write(AMBIENT_TEMP);
  Wire.endTransmission(false);  // the param must be "false" to restart the communication
  Wire.requestFrom(_addr, (uint8_t)3);
  irres.ambtemp1 = Wire.read();
  irres.ambtemp2= Wire.read();
  pec = Wire.read();
  
  /*temp = tempData;
  temp *= 0.02;
  temp  -= 273.15;*/
  return irres;
}




