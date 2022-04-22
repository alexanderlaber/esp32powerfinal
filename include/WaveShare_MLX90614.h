#ifndef   __MLX90614_H
#define   __MLX90614_H


#include <Wire.h>
#include <Arduino.h>


#define   MLX90614_ADDRESS    0x00
#define   MLX90614_ADDR_WRITE   0x00	
#define   MLX90614_ADDR_READ    0x01
#define   MLX90614_RAM    0x00
#define   AMBIENT_TEMP    0x06
#define   OBJECT_TEMP     0x07

struct irOutput
{
    uint8_t ambtemp1;
    uint8_t ambtemp2;
    uint8_t packagetemp1;
    uint8_t packagetemp2;
};

class  WaveShare_MLX90614{
 public:
  WaveShare_MLX90614(uint8_t addr = MLX90614_ADDRESS);
  boolean begin();
  uint8_t readObjectTemp(void);
  uint8_t readAmbientTemp(void);
  irOutput readTemp(void);
 private:
  uint8_t   _addr;
};

#endif 
