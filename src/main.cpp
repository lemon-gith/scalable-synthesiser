#include <Arduino.h>
#include <U8g2lib.h>
#include <bitset>

//Constants
  const uint32_t interval = 100; //Display update interval
  const char keys[12] = {'c', 'C', 'd', 'D', 'e', 'f', 'F', 'g', 'G', 'a', 'A', 'b'};
  const uint32_t stepSizes [] = {54113197, 57330935, 60740010, 64351799, 68178356, 72232452, 76527617, 81078186, 85899346, 91007187, 96418756, 102152113};
  volatile uint32_t currentStepSizes[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

//Pin definitions
  //Row select and enable
  const int RA0_PIN = D3;
  const int RA1_PIN = D6;
  const int RA2_PIN = D12;
  const int REN_PIN = A5;

  //Matrix input and output
  const int C0_PIN = A2;
  const int C1_PIN = D9;
  const int C2_PIN = A6;
  const int C3_PIN = D1;
  const int OUT_PIN = D11;

  //Audio analogue out
  const int OUTL_PIN = A4;
  const int OUTR_PIN = A3;

  //Joystick analogue in
  const int JOYY_PIN = A0;
  const int JOYX_PIN = A1;

  //Output multiplexer bits
  const int DEN_BIT = 3;
  const int DRST_BIT = 4;
  const int HKOW_BIT = 5;
  const int HKOE_BIT = 6;

//Display driver object
U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2(U8G2_R0);

//Function to check key selection
std::bitset<4> readCols(){
  std::bitset<4> result;
  result[3] = digitalRead(C3_PIN);
  result[2] = digitalRead(C2_PIN);
  result[1] = digitalRead(C1_PIN);
  result[0] = digitalRead(C0_PIN);
  return result;
}

void setRow(uint8_t row){
  digitalWrite(REN_PIN, LOW);
  digitalWrite(RA2_PIN, row & 0x04 ? HIGH : LOW);
  digitalWrite(RA1_PIN, row & 0x02 ? HIGH : LOW);
  digitalWrite(RA0_PIN, row & 0x01 ? HIGH : LOW);
  digitalWrite(REN_PIN, HIGH);
}

std::bitset<4> readRow(uint8_t row){
  setRow(row);
  delayMicroseconds(3);
  return readCols();
}

std::bitset<32> readKeys() {
  std::bitset<32> keysDown;
  for (int i = 0; i < 3; i++) {
      std::bitset<4> rowVals = readRow(i);
      for (int j = 0; j < 4; j++) {
          keysDown[(i*4)+j] = rowVals[j];
      }
  }
  return keysDown;
}

void playKeys(char keyString[]) {
  std::bitset<32> keyBools;
  uint32_t localCurrentStepSizes[12];
  keyBools = readKeys();
  for (int i = 0; i < 12; i++) {
      if (keyBools[i] == 0) {
          keyString[i] = keys[i];
          localCurrentStepSizes[i] = stepSizes[i];
      }
      else{
        localCurrentStepSizes[i] = 0;
      }
  }
  // TODO: ADD MUTEX HERE
  for (int i = 0; i<12; i++) {
    __atomic_store_n(&currentStepSizes[i], localCurrentStepSizes[i], __ATOMIC_RELAXED);
  }
}

void sampleISR() {
  static uint32_t phaseAcc = 0;
  uint32_t phaseAccChange = 0;
  for(int i=0; i<12; i++){
    phaseAccChange += currentStepSizes[i];
  }
  if (phaseAccChange==0){
    phaseAcc = 0;
  }
  else{
    phaseAcc += phaseAccChange;
  }
  int32_t Vout = (phaseAcc >> 24) - 128;
  analogWrite(OUTR_PIN, Vout + 128);
}

//Function to set outputs using key matrix
void setOutMuxBit(const uint8_t bitIdx, const bool value) {
      digitalWrite(REN_PIN,LOW);
      digitalWrite(RA0_PIN, bitIdx & 0x01);
      digitalWrite(RA1_PIN, bitIdx & 0x02);
      digitalWrite(RA2_PIN, bitIdx & 0x04);
      digitalWrite(OUT_PIN,value);
      digitalWrite(REN_PIN,HIGH);
      delayMicroseconds(2);
      digitalWrite(REN_PIN,LOW);
}

void setup() {
  // put your setup code here, to run once:

  //Set pin directions
  pinMode(RA0_PIN, OUTPUT);
  pinMode(RA1_PIN, OUTPUT);
  pinMode(RA2_PIN, OUTPUT);
  pinMode(REN_PIN, OUTPUT);
  pinMode(OUT_PIN, OUTPUT);
  pinMode(OUTL_PIN, OUTPUT);
  pinMode(OUTR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(C0_PIN, INPUT);
  pinMode(C1_PIN, INPUT);
  pinMode(C2_PIN, INPUT);
  pinMode(C3_PIN, INPUT);
  pinMode(JOYX_PIN, INPUT);
  pinMode(JOYY_PIN, INPUT);

  //Initialise display
  setOutMuxBit(DRST_BIT, LOW);  //Assert display logic reset
  delayMicroseconds(2);
  setOutMuxBit(DRST_BIT, HIGH);  //Release display logic reset
  u8g2.begin();
  setOutMuxBit(DEN_BIT, HIGH);  //Enable display power supply

  //Initialise UART
  Serial.begin(9600);
  Serial.println("Hello World");

  //Initialise hardware timer
  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *sampleTimer = new HardwareTimer(Instance);
  sampleTimer->setOverflow(22000, HERTZ_FORMAT);
  sampleTimer->attachInterrupt(sampleISR);
  sampleTimer->resume();
}

void loop() {
  // put your main code here, to run repeatedly:
  static uint32_t next = millis();
  static uint32_t count = 0;

  while (millis() < next);  //Wait for next interval

  next += interval;

  //Update display
  u8g2.clearBuffer();         // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
  u8g2.drawStr(2,10,"Hello World!");  // write something to the internal memory
  u8g2.setCursor(2,20);
  //Display keys pressed
  char keys[13] = "------------"; // Initialize keyString with dashes
  playKeys(keys);
  u8g2.print(keys);
  u8g2.setCursor(2,30);
  for (int i = 0; i<12; i++){
    if (currentStepSizes[i] != 0){
      u8g2.print(currentStepSizes[i]);
    }
  }
  //
  u8g2.sendBuffer();          // transfer internal memory to the display

  //Toggle LED
  digitalToggle(LED_BUILTIN);
  
}