#include <Arduino.h>
#include "ADS131M04.h"
#include <SPI.h>
#include "soc/rtc_wdt.h"
#include "WaveShare_MLX90614.h"


static uint8_t interruptpin=27;
static uint8_t numberofsamples= 2000;//2000
static uint8_t readyword[5];
char message;
ADS131M04 adc;
adcOutput res;
irOutput irres;
WaveShare_MLX90614   MLX90614 = WaveShare_MLX90614();

volatile uint8_t readflag =0;

uint8_t spicrcerror=0;
uint8_t registerbytes[2];
uint8_t crcdatareceivebuff[9];
uint8_t irbytearray[6];//4+2crc;
uint8_t buff[12000]; //numberofsamples*6 12000
uint8_t smallsendbuff[65];//60+2 for packet counter and +2 for crc
uint16_t register_contents = 0;
uint16_t crc_topython = 0;
uint16_t crc_adc_computed;
uint16_t crc_adc_received;

uint32_t counter=0;
uint32_t bigcounter=0;
uint32_t a,o,p,x =0;

uint16_t crc16manual(uint8_t* pData, int length)
{
    uint8_t i;
    uint16_t wCrc = 0xffff;
    while (length--) {
        wCrc ^= *(uint8_t *)pData++ << 8;
        for (i=0; i < 8; i++)
            wCrc = wCrc & 0x8000 ? (wCrc << 1) ^ 0x1021 : wCrc << 1;
    }
    return wCrc & 0xffff;
}
void hw_wdt_disable(){
    *((volatile uint32_t*) 0x60000900) &= ~(1); // Hardware WDT OFF
}

void ICACHE_RAM_ATTR ISR() {
    //Serial.print(counter);
    //delayClocks( 750 );
    readflag=1;

}

void setup() {
    MLX90614.begin();
    pinMode(5, OUTPUT);
    digitalWrite(5, LOW);
    delay(200);
    digitalWrite(5, HIGH);
    delay(100);
    pinMode(interruptpin, INPUT); //dataready
    //hw_wdt_disable();
    rtc_wdt_protect_off();
    rtc_wdt_disable();
    registerbytes[1] = 0xFF;//0xFF
    registerbytes[0] = 0xFF;
    readyword[0]=0x72;
    readyword[1]=0x65;
    readyword[2]=0x61;
    readyword[3]=0x64;
    readyword[4]=0x79;
    Serial.begin(115200);

    adc.begin(18, 19, 23, 25);// cs, dataready
    adc.setOsr(3);
    //adc.set_ch0_phase(32);
    //adc.setGlobalChop(1);
    //adc.set_ch1_phase(32);
    //adc.setChannelOffsetCalibration();
    //adc.setInputChannelSelection(0, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
    //adc.setInputChannelSelection(1, INPUT_CHANNEL_MUX_AIN0P_AIN0N);
    attachInterrupt(digitalPinToInterrupt(interruptpin), ISR, FALLING);
}

void loop() {
    while (Serial.available() > 0){
        detachInterrupt(digitalPinToInterrupt(interruptpin));
        message = Serial.read();
        for(a=0;a<9;++a){
            if(message==0x30+a){
                adc.setOsr(a);
                attachInterrupt(digitalPinToInterrupt(interruptpin), ISR, FALLING);
                delay(200);
                break;
            }
        }
    }

    if (readflag){
        detachInterrupt(digitalPinToInterrupt(interruptpin));
        readflag=0;
        res = adc.readADC();
        buff[counter*6] = res.ch0x;//res.ch0x
        buff[(counter*6) +1] = res.ch0x2;;//res.ch0x2;
        buff[(counter*6) +2] = res.ch0x3;;//res.ch0x3;
        buff[(counter*6) +3] = res.ch1x;;//res.ch1x;
        buff[(counter*6) +4] = res.ch1x2;;//
        buff[(counter*6) +5] = res.ch1x3;;//
        crcdatareceivebuff[0]= res.status0;
        crcdatareceivebuff[1]= res.status1;
        crcdatareceivebuff[2]= res.status2;
        crcdatareceivebuff[3]= res.ch0x;
        crcdatareceivebuff[4]= res.ch0x2;
        crcdatareceivebuff[5]= res.ch0x3;
        crcdatareceivebuff[6]= res.ch1x;
        crcdatareceivebuff[7]= res.ch1x2;
        crcdatareceivebuff[8]= res.ch1x3;
        crc_adc_computed=crc16manual((uint8_t*)crcdatareceivebuff, 9);
        crc_adc_received=(((res.crc0 << 8) | res.crc1) & 0xFFFF);
        if(crc_adc_computed!=crc_adc_received){
            spicrcerror=1;
        }
        else{spicrcerror=0;}
/*
Serial.println("------");
Serial.print("crc_adc_computed: ");
Serial.println(crc_adc_computed);
Serial.print("crc_adc_received: ");
crc_adc_received=(((res.crc0 << 8) | res.crc1) & 0xFFFF);
Serial.print(crc_adc_received);
Serial.print(" rawbytes: ");
Serial.print(res.crc0,BIN);
Serial.print(" ");
Serial.println(res.crc1,BIN);*/
        counter+=1;
        if (counter==sizeof(buff)/6){
            irres = MLX90614.readTemp(); //read temperatures from ir sensor
            Serial.write(readyword,5);
            while (Serial.available() > 0){
                message = Serial.read();
                if(message==0x7A){
                    break;
                }
            }
            for(o=0;o<sizeof(buff)/60;++o){//sizeof(buff)/60
                for (p=0;p<60;++p){
                    smallsendbuff[p] = buff[(o*60) +p];
                }
                smallsendbuff[60] = bigcounter & 255;
                smallsendbuff[61] = (bigcounter >> 8) & 255;
                smallsendbuff[62] = spicrcerror;
                crc_topython =crc16manual((uint8_t*)smallsendbuff, 63);
                smallsendbuff[63] = crc_topython & 255;
                smallsendbuff[64] = (crc_topython >> 8) & 255;//crc.getCRC()

                Serial.write(smallsendbuff,sizeof(smallsendbuff));
                bigcounter++;
            }
            counter= 0;
            bigcounter=0;
            for(x=0;x<4;++x) {
                register_contents = adc.readRegister(x);
                registerbytes[1] = register_contents & 255;//0xFF
                registerbytes[0] = (register_contents >> 8) & 255;
                Serial.write(registerbytes, sizeof(registerbytes));
            }
            irbytearray[0]=irres.ambtemp1;
            irbytearray[1]=irres.ambtemp2;
            irbytearray[2]=irres.packagetemp1;
            irbytearray[3]=irres.packagetemp2;
            crc_topython =crc16manual((uint8_t*)irbytearray, 4);
            irbytearray[4]=crc_topython & 255;
            irbytearray[5]=(crc_topython >> 8) & 255;
            Serial.write(irbytearray, sizeof(irbytearray));

        }
        attachInterrupt(digitalPinToInterrupt(interruptpin), ISR, FALLING);

    }

}