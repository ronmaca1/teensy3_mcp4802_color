#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
// trailing "_L" indicates an active low signal
// there has to be a less icky looking way to do that
#define AUDIO           A9
#define ZCROSSIN_L      2
#define LEDPIN          5
#define DAC1SEL_L       7
#define DAC2SEL_L       8
#define LDACALL_L       14
#define ScopeSync       6

// mcp 48xx dacs have no buffer on vref, config bit 14 not used
// shutdown disabled, two different gain settings gives us the constants below

//#define DAC1ACFG 0x30   // gain of 1, shdn disabled
#define DAC1ACFG 0x10    // gain of 2, shdn disabled

//#define DAC1BCFG 0xB0   // gain of 1, shdn disabled
#define DAC1BCFG 0x90    // gain of 2, shdn disabled

//#define DAC2ACFG 0x30   // gain of 1, shdn disabled
#define DAC2ACFG 0x10    //  gain of 2, shdn disabled

//#define DAC2BCFG 0xB0    // gain of 1, shdn disabled
#define DAC2BCFG 0x90   // gain of 2, shdn disabled


//#define _DEBUG_
#undef _DEBUG_
volatile uint8_t sync = false;
uint8_t band1,band2,band3,band4;
uint8_t dacoutH,dacoutL;

void zcross(void);
void writedacs(void);

// this setup changes AREF to internal 1.2v bandgap reference
// GUItool: begin automatically generated code
AudioInputAnalog         adc1(AUDIO);
AudioAnalyzeFFT256       my_fft_1;
//AudioAnalyzeFFT1024       my_fft_1;
AudioConnection          patchCord1(adc1, my_fft_1);
// GUItool: end automatically generated code




void setup() {
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.begin();
    AudioMemory(8);

    pinMode(ZCROSSIN_L,INPUT);
    // I always set my expected default output level
    // *before* setting the pin to output mode here in the setup code.
    // no unexpected glitches that way ( I hope )
    digitalWrite(DAC1SEL_L,HIGH);
    pinMode(ScopeSync,OUTPUT);
    digitalWrite(ScopeSync,LOW);
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
    }

}

void zcross(void) {
    digitalWrite(ScopeSync,HIGH);
    sync = true;
    digitalWrite(ScopeSync,LOW);
}

void writedacs(void) {

    // band 1, DAC1A
    dacoutH = DAC1ACFG | ((band1>>4) & 0x0F);
    dacoutL = (band1<<4) & 0xF0;
    digitalWrite(DAC1SEL_L, LOW);

    SPI.transfer(dacoutH);
    SPI.transfer(dacoutL);

    digitalWrite(DAC1SEL_L, HIGH);

    // band 2, DAC1B
    dacoutH = DAC1BCFG | ((band2>>4) & 0x0F);
    dacoutL = (band2<<4) & 0xF0;
    digitalWrite(DAC1SEL_L, LOW);

    SPI.transfer(dacoutH);
    SPI.transfer(dacoutL);

    digitalWrite(DAC1SEL_L, HIGH);
    // done with dac1 A and B

    // band 3, DAC2A
    dacoutH = DAC2ACFG | ((band3>>4) & 0x0f);
    dacoutL = (band3<<4) & 0xF0;
    digitalWrite(DAC2SEL_L, LOW);

    SPI.transfer(dacoutH);
    SPI.transfer(dacoutL);

    digitalWrite(DAC2SEL_L, HIGH);

    // band 4 DAC2B
    dacoutH = DAC2BCFG | ((band4>>4) & 0x0f);
    dacoutL = (band4<<4) & 0xF0;
    digitalWrite(DAC2SEL_L, LOW);

    SPI.transfer(dacoutH);
    SPI.transfer(dacoutL);

    digitalWrite(DAC2SEL_L, HIGH);
    //done with dac2 A and B

    //latch all dac data to outputs
    digitalWrite(LDACALL_L,LOW);

    digitalWrite(LDACALL_L,HIGH);
    //digitalWrite(LEDPIN, !(digitalRead(LEDPIN)));

}
