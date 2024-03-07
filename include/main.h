#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h>
#include <ES_CAN.h>
#include <bitset>


//Constants
const uint32_t interval = 100; //Display update interval
const char keys[12] = {
  'c', 'C', 'd', 'D', 'e', 'f', 'F', 'g', 'G', 'a', 'A', 'b'
};
const std::array<char, 12> base_keys = {  // TODO: currently unused
 '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-'
};
const uint32_t stepSizes [] = {
  51076057, 54113197, 57330935, 60740010, 64351799, 68178356, 
  72232452, 76527617, 81078186, 85899346, 91007187, 96418756
};
const char* toneNames [] = {"saw", "sqr", "sin", "tri"};
const uint32_t knobMaxes[4] = {
  8, (sizeof(toneNames)/sizeof(toneNames[0]))-1, 5, 4
};

//System state variable arrays
struct {
  volatile char keyStrings[12] = {
    '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-'
  };
  
  // K0 K1 K2 K3
  volatile uint32_t knobValues[4] = {
    4, 0, knobMaxes[2], knobMaxes[3]
  }; 
  volatile bool knobPushes[4] = {0};  // VOL TONE SETTING ECHO
  volatile uint8_t octave = 4;

  volatile short joy[3] = {0};

  volatile uint8_t TX_Message[8] = {0};
  volatile uint8_t RX_Message[8] = {0};
  volatile bool isSender = false;

  volatile uint8_t menuState = 0;  // 0 is met, 1 is playback, 2 is oct
  volatile bool isSelected = false;
  volatile uint8_t met = 120;
  volatile bool metMenuState = false;  //false is met slider, true is ON/OFF
  volatile bool metOnState = false;
  volatile uint8_t dotLocation[2] = {58, 4};
  SemaphoreHandle_t mutex;
} sysState;

QueueHandle_t msgInQ;
QueueHandle_t msgOutQ;
SemaphoreHandle_t CAN_TX_Semaphore;

// an enum containing all the pin definitions, 
// since they're all just integers anyways
enum PinDefinitions{
  //Row select and enable
  RA0_PIN = D3,
  RA1_PIN = D6,
  RA2_PIN = D12,
  REN_PIN = A5,

  //Matrix input and output
  C0_PIN = A2,
  C1_PIN = D9,
  C2_PIN = A6,
  C3_PIN = D1,
  OUT_PIN = D11,

  //Audio analogue out
  OUTL_PIN = A4,
  OUTR_PIN = A3,

  //Joystick analogue in
  JOYY_PIN = A0,
  JOYX_PIN = A1,

  //Output multiplexer bits
  DEN_BIT = 3,
  DRST_BIT = 4,
  HKOW_BIT = 5,
  HKOE_BIT = 6,
};

//Display driver object
U8G2_SSD1305_128X32_ADAFRUIT_F_HW_I2C u8g2(U8G2_R0);

std::bitset<4> readCols();

void setRow(uint8_t row);

std::bitset<4> readRow(uint8_t row);

std::bitset<32> readKeys();

std::bitset<12> readKnobs();

char calcJoy(short x, short y, short p);

void updateKeysTask(void * pvParameters);

void updateDisplayTask(void * pvParameters);

void decodeMessageTask(void * pvParameters);

void CAN_TX_Task (void * pvParameters);

int32_t playNote(uint8_t oct, uint8_t note, uint32_t volume, uint32_t tone);

void sampleISR();

void CAN_RX_ISR (void);

void CAN_TX_ISR (void);

void setOutMuxBit(const uint8_t bitIdx, const bool value);

// setup and loop don't need to go here

