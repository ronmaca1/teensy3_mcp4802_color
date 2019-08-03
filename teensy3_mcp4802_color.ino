#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
//#include <SD.h>
//#include <SerialFlash.h>

#define AUDIO A9
#define RAMPRESET 2
#define DAC1SEL 3
#define DAC2SEL 4
#define LDACALL 14

// no buffer on vref, gain of 2, shdn disabled
// gives the following nybbles for dac a , b
// channel A , VREF gain 2(4.096v)
// (use for MCP4801 single channel, too)
#define MCP4802ACONFIGBITS 0x10
// channel B , VREF gain 2(4.096v)
#define MCP4802BCONFIGBITS 0x90

#define _DEBUG_
//#undef _DEBUG_

uint8_t band1, band2, band3, band4;
uint8_t dacoutH, dacoutL;

// this setup changes AREF to internal 1.2v bandgap reference
// GUItool: begin automatically generated code
AudioInputAnalog adc1(AUDIO);
AudioAnalyzeFFT256 fft256_1;
AudioConnection patchCord1(adc1, fft256_1);
// GUItool: end automatically generated code

void setup()
{
#ifdef _DEBUG_
    Serial.begin(115200);
    Serial.print("TEST");
#endif

    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.begin();
    AudioMemory(8);

    digitalWrite(RAMPRESET, HIGH);
    pinMode(RAMPRESET, OUTPUT);
    digitalWrite(DAC1SEL, HIGH);
    pinMode(DAC1SEL, OUTPUT);
    digitalWrite(DAC2SEL, HIGH);
    pinMode(DAC2SEL, OUTPUT);

    // digitalWrite(DAC5SEL,HIGH);
    // pinMode(DAC5SEL,OUTPUT);
    digitalWrite(LDACALL, HIGH);
    pinMode(LDACALL, OUTPUT);

    //pinMode(13,OUTPUT);
    //digitalWrite(13,LOW);
    pinMode(AUDIO, INPUT);
    //analogReference(INTERNAL);
    //analogReference(DEFAULT);
}

void loop()
{
    // put your main code here, to run repeatedly:
    //digitalWrite(13, !(digitalRead(13)));
    if (fft256_1.available())
    {

        band1 = (uint8_t)(fft256_1.read(0, 7) * 256);
        band2 = (uint8_t)(fft256_1.read(8, 23) * 256);
        band3 = (uint8_t)(fft256_1.read(24, 55) * 256);
        band4 = (uint8_t)(fft256_1.read(56, 127) * 256);

#ifdef _DEBUG_
        Serial.print("\n\r");
        Serial.print(band1);
        Serial.print(',');
        Serial.print(band2);
        Serial.print(',');
        Serial.print(band3);
        Serial.print(',');
        Serial.print(band4);
        Serial.print("\n\r");
        Serial.print(AudioMemoryUsageMax());

#endif
        // see datasheet for MCP4802 to decipher data transfer formatting that follows

        // band 1, DAC1A        
        dacoutH = MCP4802ACONFIGBITS | ((band1 >> 4) & 0x0F);
        dacoutL = (band1 << 4) & 0xF0;
        digitalWrite(DAC1SEL, LOW);
        delayMicroseconds(1); // let the DAC get ready
        SPI.transfer(dacoutH);
        SPI.transfer(dacoutL);
        delayMicroseconds(1); // let the DAC settle
        digitalWrite(DAC1SEL, HIGH);

        // wait a little before the B dac
        delayMicroseconds(1);

        // band 2, DAC1B
        dacoutH = MCP4802BCONFIGBITS | ((band2 >> 4) & 0x0F);
        dacoutL = (band2 << 4) & 0xF0;
        digitalWrite(DAC1SEL, LOW);
        delayMicroseconds(1); // let the DAC get ready
        SPI.transfer(dacoutH);
        SPI.transfer(dacoutL);
        delayMicroseconds(1); // let the DAC settle
        digitalWrite(DAC1SEL, HIGH);
        // done with dac1 A and B

        // band 3, DAC2A
        dacoutH = MCP4802ACONFIGBITS | ((band3 >> 4) & 0x0F);
        dacoutL = (band3 << 4) & 0xF0;
        digitalWrite(DAC2SEL, LOW);
        delayMicroseconds(1); // let the DAC get ready
        SPI.transfer(dacoutH);
        SPI.transfer(dacoutL);
        delayMicroseconds(1); // let the DAC settle
        digitalWrite(DAC2SEL, HIGH);

        // wait a little before the B dac
        delayMicroseconds(1);

        // band 4 DAC2B
        dacoutH = MCP4802BCONFIGBITS | ((band4 >> 4) & 0x0F);
        dacoutL = (band4 << 4) & 0xF0;
        digitalWrite(DAC2SEL, LOW);
        delayMicroseconds(1); // let the DAC get ready
        SPI.transfer(dacoutH);
        SPI.transfer(dacoutL);
        delayMicroseconds(1); // let the DAC settle
        digitalWrite(DAC2SEL, HIGH);
    }

    digitalWrite(RAMPRESET, LOW);
    delayMicroseconds(100);
    digitalWrite(RAMPRESET, HIGH);
    delay(8); // about 120 hz for testing, replace with zero cross detection
}
