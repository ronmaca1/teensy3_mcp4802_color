#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#define AUDIO           A9
#define ZCROSSIN_L      2
#define LEDPIN          5
#define DAC1SEL_L       7
#define DAC2SEL_L       8
#define LDACALL_L       14

// mcp 48xx dacs have no buffer on vref, config bit 14 not used
// shutdown disabled, two different gain settings gives us the constants below

#define DAC1ACFG 0x30   // gain of 1, shdn disabled
// #define DAC1ACFG 0x10    // gain of 2, shdn disabled

#define DAC1BCFG 0xB0   // gain of 1, shdn disabled
// #define DAC1BCFG 0x90    // gain of 2, shdn disabled


#define DAC2ACFG 0x30   // gain of 1, shdn disabled
// #define DAC2ACFG 0x10    //  gain of 2, shdn disabled

#define DAC2BCFG 0xB0    // gain of 1, shdn disabled
// #define DAC2BCFG 0x90   // gain of 2, shdn disabled


//#define _DEBUG_
#undef _DEBUG_
volatile uint8_t sync = false;
uint8_t band1,band2,band3,band4;
uint8_t dacoutH,dacoutL;

// this setup changes AREF to internal 1.2v bandgap reference
// GUItool: begin automatically generated code
AudioInputAnalog         adc1(AUDIO);
AudioAnalyzeFFT256       my_fft_1;
//AudioAnalyzeFFT1024       my_fft_1;
AudioConnection          patchCord1(adc1, my_fft_1);
// GUItool: end automatically generated code

void zcross(void) {
    sync = true;
}

void writedacs(void) {
    // band 1, DAC1A
    dacoutH = DAC1ACFG | ((band1>>4) & 0x0F);
    dacoutL = (band1<<4) & 0xF0;
    digitalWrite(DAC1SEL_L, LOW);
    delayMicroseconds(1);            // let the DAC get ready
    SPI.transfer(dacoutH);
    SPI.transfer(dacoutL);
    delayMicroseconds(1);
    digitalWrite(DAC1SEL_L, HIGH);
    // wait a little before the B dac
    delayMicroseconds(1);

    // band 2, DAC1B
    dacoutH = DAC1BCFG | ((band2>>4) & 0x0F);
    dacoutL = (band2<<4) & 0xF0;
    digitalWrite(DAC1SEL_L, LOW);
    delayMicroseconds(1);
    SPI.transfer(dacoutH);
    SPI.transfer(dacoutL);
    delayMicroseconds(1);            // let the DAC settle
    digitalWrite(DAC1SEL_L, HIGH);
    // done with dac1 A and B


    // band 3, DAC2A
    dacoutH = DAC2ACFG | ((band3>>4) & 0x0f);
    dacoutL = (band3<<4) & 0xF0;
    digitalWrite(DAC2SEL_L, LOW);
    delayMicroseconds(1);            // let the DAC get ready
    SPI.transfer(dacoutH);
    SPI.transfer(dacoutL);
    delayMicroseconds(1);
    digitalWrite(DAC2SEL_L, HIGH);
    // wait a little before the B dac
    delayMicroseconds(1);

    // band 4 DAC2B
    dacoutH = DAC2BCFG | ((band4>>4) & 0x0f);
    dacoutL = (band4<<4) & 0xF0;
    digitalWrite(DAC2SEL_L, LOW);
    delayMicroseconds(1);
    SPI.transfer(dacoutH);
    SPI.transfer(dacoutL);
    delayMicroseconds(1);            // let the DAC settle
    digitalWrite(DAC2SEL_L, HIGH);
    //done with dac2 A and B

    // wait a little
    delayMicroseconds(1);
    // then latch all dac data to outputs
    digitalWrite(LDACALL_L,LOW);
    delayMicroseconds(1);
    digitalWrite(LDACALL_L,HIGH);
    digitalWrite(LEDPIN, !(digitalRead(LEDPIN)));
}

void setup() {
#ifdef _DEBUG_
    Serial.begin(115200);
    Serial.print("TEST");
#endif

    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.begin();
    AudioMemory(8);
    pinMode(ZCROSSIN_L,INPUT);
    // I always set my expected default output level
    // *before* setting the pin to output mode here in the setup code.
    // no unexpected glitches that way ( I hope )
    digitalWrite(DAC1SEL_L,HIGH);
    pinMode(DAC1SEL_L,OUTPUT);
    digitalWrite(DAC2SEL_L,HIGH);
    pinMode(DAC2SEL_L,OUTPUT);
    digitalWrite(LDACALL_L,HIGH);
    pinMode(LDACALL_L,OUTPUT);
    digitalWrite(LEDPIN,HIGH);
    pinMode(LEDPIN,OUTPUT);
    attachInterrupt(digitalPinToInterrupt(ZCROSSIN_L), zcross, FALLING);
}

void loop() {
    // put your main code here, to run repeatedly:

    if(sync == true) {
        sync = false;
        writedacs();
    }

    if (my_fft_1.available()) {
        // band allocation finished. you no touch no more!
        band1=(uint8_t) (my_fft_1.read(1,8)*256);
        band2=(uint8_t) (my_fft_1.read(9,16)*256);
        band3=(uint8_t) (my_fft_1.read(16,32)*256);
        band4=(uint8_t) (my_fft_1.read(33,127)*256);

#ifdef _DEBUG_
        Serial.print("\n\r");
        Serial.print(band1);
        Serial.print(',');
        Serial.print(band2);
        Serial.print(',');
        Serial.print(band3);
        Serial.print(',');
        Serial.print(band4);/*
        Serial.print("\n\r");
        Serial.print((my_fft_1.read(0,7)*256));
        Serial.print(',');
        Serial.print((my_fft_1.read(8,23)*256));
        Serial.print(',');
        Serial.print((my_fft_1.read(24,55)*256));
        Serial.print(',');
        Serial.print((my_fft_1.read(56,127)*256));*/
#endif
    }

}
