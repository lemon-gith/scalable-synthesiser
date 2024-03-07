#include <Arduino.h>
#include <U8g2lib.h>
#include <STM32FreeRTOS.h>
#include <ES_CAN.h>
#include <bitset>

//Constants
  const uint32_t interval = 100; //Display update interval
  const char keys[12] = {'c', 'C', 'd', 'D', 'e', 'f', 'F', 'g', 'G', 'a', 'A', 'b'};
  const uint32_t stepSizes [] = {51076057, 54113197, 57330935, 60740010, 64351799, 68178356, 72232452, 76527617, 81078186, 85899346, 91007187, 96418756};
  const char* toneNames [] = {"saw", "sqr", "sin", "tri"};
  const uint32_t knobMaxes[4] = {8,(sizeof(toneNames)/sizeof(toneNames[0]))-1,5,4};

//System state variable arrays
struct {
  volatile char keyStrings[12] = {'-','-','-','-','-','-','-','-','-','-','-','-'};
  volatile uint32_t knobValues[4] = {4,0,knobMaxes[2],knobMaxes[3]}; // K0 K1 K2 K3
  volatile bool knobPushes[4] = {0,0,0,0}; //VOL TONE SETTING ECHO
  volatile short joy[3]= {0,0,0};
  volatile uint8_t octave = 4;
  volatile uint8_t TX_Message[8] = {0};
  volatile uint8_t RX_Message[8] = {0};
  volatile bool isSender = false;
  volatile uint8_t menuState= 0; //0 is met, 1 is playback, 2 is oct
  volatile bool isSelected = false;
  volatile uint8_t met = 120;
  volatile bool metMenuState = false; //false is met slider, true is ON/OFF
  volatile bool metOnState = false;
  SemaphoreHandle_t mutex;
} sysState;
QueueHandle_t msgInQ;
QueueHandle_t msgOutQ;
SemaphoreHandle_t CAN_TX_Semaphore;
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
U8G2_SSD1305_128X32_ADAFRUIT_F_HW_I2C u8g2(U8G2_R0);

//Big Endian
//Big Endian
std::bitset<4> readCols(){
  std::bitset<4> result;
  result[3] = digitalRead(C3_PIN);
  result[2] = digitalRead(C2_PIN);
  result[1] = digitalRead(C1_PIN);
  result[0] = digitalRead(C0_PIN);
  return result;
}

void navigate(char direction){
  xSemaphoreTake(sysState.mutex, portMAX_DELAY);
  uint8_t menuState = sysState.menuState;
  bool isSelected = sysState.isSelected;
  bool metMenuState = sysState.metMenuState;
  bool metOnState = sysState.metOnState;
  int metValue = sysState.met;
  uint8_t octave = sysState.octave;
  xSemaphoreGive(sysState.mutex);
  if(direction=='p'){ //joystick press changes isSelected state
    isSelected = !isSelected;
    }
  if(isSelected==false){ //if isSelected is false, it can move between the 3 menu items
    if(menuState==0){
      if(direction=='d'){
        menuState = 1;
      }
    }
    else if(menuState ==1){
      if(direction =='u'){
        menuState = 0;
      }
      else if(direction =='l'){
        menuState = 2;
      }
    }
    else if(menuState ==2){
      if(direction == 'r'){
        menuState = 1;
      }
    }
  }
  else{ //if isSelected is true (i.e. menu item is selected)
    if(menuState==0){
      if(!metMenuState){
        if(direction == 'u'){
          metValue++;
        }
        else if(direction == 'd'){
          metValue--;
        }
        else if(direction == 'r'){
          metMenuState=!metMenuState;
        }
      }
      else if(metMenuState){
        if(direction=='p'){
          metOnState = !metOnState;
        }
        else if(direction=='l'){
          metMenuState=!metMenuState;
        }
        
        
      }
    }
    else if(menuState==2){
      if(direction=='u'){
        if(octave<8){
          octave++;
        }
      }
      else if(direction=='d'){
        if(octave>0){
          octave--;
        }
      }
    }
  }
 
  xSemaphoreTake(sysState.mutex, portMAX_DELAY);
  sysState.octave = octave;
  sysState.metOnState = metOnState;
  sysState.metMenuState = metMenuState;
  sysState.met = metValue;
  sysState.menuState = menuState;
  sysState.isSelected = isSelected;
  xSemaphoreGive(sysState.mutex);
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

// Output in zero-indexed array order {3A 3B 2A 2B 1A 1B 0A 0B 3S 2S 1S 0S}
std::bitset<12> readKnobs(){
  std::bitset<12> knobVals;  
  for (int i = 3; i < 7; i++){
    std::bitset<4> rowVals = readRow(i);
    if ((3<=i)&(i<5)){ //A&B vals
      for (int j = 0; j < 4; j++) {
        knobVals[(i-3)*4+j] = rowVals[j];
      }
    }
    else{ //S vals
      for (int j = 0; j < 2; j++){
        knobVals[8+((i-5)*2)+j] = rowVals[1-j];
      }
    }
  }
  return knobVals;
}

char calcJoy(short x, short y, short p){ //calculates the direction of the joystick (goes from 0 to 1024)
  if(p==0){
    return 'p';
  }
  else if(x<300 && (y<600&&y>200)){
    return 'r';
  }
  else if (x>700&&(y<600&&y>200)){
    return 'l';
  }
  else if ((x<600&&x>200)&&y<200){
    return 'u';
  }
  else if ((x<600&&x>200)&&y>700){
    return 'd';
  }
  else{
    return 's';
  }
}

// TIMED TASKS
void updateKeysTask(void * pvParameters) {
  const TickType_t xFrequency = 20/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    std::bitset<32> keyBools;
    keyBools = readKeys();
    // Store key string locally
    static char localKeyStrings[12] = {'-','-','-','-','-','-','-','-','-','-','-','-'};
    char prevLocalKeyStrings[12];
    for (int i=0; i<12; i++){prevLocalKeyStrings[i] = localKeyStrings[i];}
    for (int i = 0; i < 12; i++) {
      if (keyBools[i] == 0) {
        localKeyStrings[i] = keys[i];
      }
      else{
        localKeyStrings[i] = '-';
      }
      //Check for changes
      if (prevLocalKeyStrings[i] != localKeyStrings[i]){
        bool isPush = (prevLocalKeyStrings[i] == '-');
        xSemaphoreTake(sysState.mutex, portMAX_DELAY);
        sysState.TX_Message[0] = isPush ? 'P' : 'R';
        sysState.TX_Message[1] = sysState.octave;
        sysState.TX_Message[2] = i;
        xSemaphoreGive(sysState.mutex);
      }
    }
    // Send changes via CAN
    xSemaphoreTake(sysState.mutex, portMAX_DELAY);
    bool localIsSender = __atomic_load_n(&sysState.isSender, __ATOMIC_RELAXED);
    if (localIsSender == true){
      uint8_t localTX_Message[8];
      for (int i=0; i<8; i++){localTX_Message[i] = __atomic_load_n(&sysState.TX_Message[i], __ATOMIC_RELAXED);}
      xQueueSend( msgOutQ, localTX_Message, portMAX_DELAY);
    }
    xSemaphoreGive(sysState.mutex);
    // Store knob values and push bools
    std::bitset<12> prevKnobBools;
    static std::bitset<12> knobBools = readKnobs();
    prevKnobBools = knobBools;
    knobBools = readKnobs();
    int localKnobDiffs[4] = {0,0,0,0};
    int lastLegalDiff[4] = {0,0,0,0};
    bool localKnobPushes[4];
    for (int i=0; i<4; i++){
      //Knob values in format {B,A}
      std::bitset<2> prevBool, currBool;
      prevBool[1] = prevKnobBools[(2*i)+1];
      prevBool[0] = prevKnobBools[2*i];
      currBool[1] = knobBools[(2*i)+1];
      currBool[0] = knobBools[2*i];
      // Read sys knob values
      uint32_t localKnobValues[4];
      xSemaphoreTake(sysState.mutex, portMAX_DELAY);
      for (int i=0; i<4; i++){localKnobValues[i] = __atomic_load_n(&sysState.knobValues[i], __ATOMIC_RELAXED);}
      xSemaphoreGive(sysState.mutex);
      if(((prevBool == 0b00)and(currBool == 0b01))or((prevBool == 0b11)and(currBool == 0b10))){
        localKnobDiffs[3-i] = (localKnobValues[3-i] < knobMaxes[3-i]) ? 1 : 0;
        lastLegalDiff[3-i] = (localKnobValues[3-i] < knobMaxes[3-i]) ? 1 : 0;
      }
      else if(((prevBool == 0b01)and(currBool == 0b00))or((prevBool == 0b10)and(currBool == 0b11))){
        localKnobDiffs[3-i] = (localKnobValues[3-i] > 0) ? -1 : 0;
        lastLegalDiff[3-i] = (localKnobValues[3-i] > 0) ? -1 : 0;
      }
      else if(((prevBool == 0b00)and(currBool == 0b11))or((prevBool == 0b11)and(currBool == 0b00))or((prevBool == 0b01)and(currBool == 0b10))or((prevBool == 0b10)and(currBool == 0b01))){
        //other legal transition
        localKnobDiffs[3-i] = 0;
      }
      else{
        //illegal transition to help w skipping
        localKnobDiffs[3-i] = 5*lastLegalDiff[3-i];
      }
      //Knob pushes
      localKnobPushes[3-i] = !knobBools[i+8];
    }
    //see joystick push
    std::bitset<4> rowVals = readRow(5);
    short joy[3];
    //Store Joystick Values
    joy[0] = analogRead(JOYX_PIN);
    joy[1] = analogRead(JOYY_PIN);
    joy[2] = rowVals[2];
    navigate(calcJoy(joy[0], joy[1], joy[2]));
    //Store to sysState
    xSemaphoreTake(sysState.mutex, portMAX_DELAY);
    // Store key string globally
    for (int i=0; i<12; i++) {
      __atomic_store_n(&sysState.keyStrings[i], localKeyStrings[i], __ATOMIC_RELAXED);
    }
    // Store knob values globally
    for (int i=0; i<4; i++){
      int tmp = sysState.knobValues[i]+localKnobDiffs[i];
      __atomic_store_n(&sysState.knobValues[i], tmp, __ATOMIC_RELAXED);
    }
    //__atomic_store_n(&sysState.octave, sysState.knobValues[2], __ATOMIC_RELAXED); //TODO: REMOVE ONCE PROPER OCTAVE CONTROL IMPLEMENTED
    // Store knob pushes globally
    for (int i=0; i<4; i++){
      __atomic_store_n(&sysState.knobPushes[i], localKnobPushes[i], __ATOMIC_RELAXED);
    }
    
    xSemaphoreGive(sysState.mutex);
    //
  }
}

// Refreshes display
void updateDisplayTask(void * pvParameters){
  const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    //DISPLAY UPDATE
    u8g2.clearBuffer();         // clear the internal memory
    u8g2.setFont(u8g2_font_u8glib_4_tf); // choose a suitable font
    //Display last sent/received CAN message
    u8g2.drawFrame(0, -2, 56, 8);
    //Display key names
    char localKeyStrings[13];
    localKeyStrings[12] = '\0'; //Termination
    xSemaphoreTake(sysState.mutex, portMAX_DELAY);
    for (int i = 0; i<12; i++){localKeyStrings[i] = __atomic_load_n(&sysState.keyStrings[i], __ATOMIC_RELAXED);}
    xSemaphoreGive(sysState.mutex);
    u8g2.drawStr(2,4,localKeyStrings);
    //Octave
    u8g2.drawStr(2, 12, "OCT:");
    u8g2.setCursor(20, 12);
    xSemaphoreTake(sysState.mutex, portMAX_DELAY);
    u8g2.print(sysState.octave);
    u8g2.setCursor(30,12);
    u8g2.print(sysState.TX_Message[0]);
    xSemaphoreGive(sysState.mutex);
    //Metronome
    u8g2.drawStr(60,4, "Met:");
    u8g2.setCursor(79, 4);
    xSemaphoreTake(sysState.mutex, portMAX_DELAY);
    u8g2.print(sysState.met);
    bool metOnState = sysState.metOnState;
    xSemaphoreGive(sysState.mutex);
    u8g2.drawStr(92, 4, "BPM");
    u8g2.setCursor(114, 4);
    u8g2.print(metOnState ? "OFF": "ON");
    //Playback
    u8g2.drawStr(60, 12, "Pb");
    u8g2.drawCircle(78, 10, 3);
    u8g2.drawBox(88, 8, 5, 5);
    u8g2.drawTriangle(100, 7, 100, 13, 103, 10);

    //Bottom Menu
    //Volume
    u8g2.drawStr(10, 20, "Vol");
    u8g2.drawFrame(1, 22, 28, 12);
    u8g2.setCursor(1+14-5, 29);
    xSemaphoreTake(sysState.mutex, portMAX_DELAY);
    u8g2.print(sysState.menuState);
    bool isClicked = sysState.isSelected;
    xSemaphoreGive(sysState.mutex);
    u8g2.setCursor(16, 29);
    u8g2.print(isClicked ? 't':'f');

    //Tone
    u8g2.drawStr(40, 20, "Tone");
    u8g2.drawFrame(33, 22, 28, 12);
    xSemaphoreTake(sysState.mutex, portMAX_DELAY);
    u8g2.drawStr(33+14-5, 29, toneNames[sysState.knobValues[1]]);
    xSemaphoreGive(sysState.mutex);
    //Setting
    u8g2.drawStr(66, 20, "Setting");
    u8g2.drawFrame(65, 22, 28, 12);
    u8g2.drawStr(65+14-5, 29, "Vib");
    //Echo
    u8g2.drawStr(104, 20, "Echo");
    u8g2.drawFrame(97, 22, 28, 12);
    u8g2.drawStr(97+14-5, 29, "Rev");
    //TODO: Add a centering function framex+(dist-2)-(floor(word/2))
    //      Word should b 3*(char len+1) - 1  
    u8g2.sendBuffer();          // transfer internal memory to the display
    //Toggle LED
    digitalToggle(LED_BUILTIN); //Toggle LED for CW requirement
  }
}

// Decodes messages
void decodeMessageTask(void * pvParameters){
  uint8_t localRX_Message[8] = {0};
  while(1){
    xQueueReceive(msgInQ, localRX_Message, portMAX_DELAY);
    xSemaphoreTake(sysState.mutex, portMAX_DELAY);
    for (int i=0; i<8; i++){sysState.RX_Message[i] = __atomic_load_n(&localRX_Message[i], __ATOMIC_RELAXED);}
    xSemaphoreGive(sysState.mutex);
  }
}

// Sends messages from queue
void CAN_TX_Task (void * pvParameters) {
	uint8_t msgOut[8];
	while (1) {
		xQueueReceive(msgOutQ, msgOut, portMAX_DELAY);
		xSemaphoreTake(CAN_TX_Semaphore, portMAX_DELAY);
		CAN_TX(0x123, msgOut);
	}
}


// Given an octave, note index, volume, and tone, plays the note
int32_t playNote(uint8_t oct, uint8_t note, uint32_t volume, uint32_t tone){
  // FUNCTION WAVES
  if((tone == 0)|(tone == 1)){ //SAWTOOTH OR SQUARE
    static uint32_t phaseAcc = 0;
    uint32_t phaseAccChange = 0;
    uint32_t phaseInc = (oct < 4) ? (stepSizes[note] >> (4-oct)) : (stepSizes[note] << (oct-4));
    phaseAccChange += phaseInc;
    if (phaseAccChange==0){
      phaseAcc = 0;
    }
    else{
      phaseAcc += phaseAccChange;
    }
    uint32_t phaseOut = (((phaseAcc >> 24) - 128) >> (8-volume)) + 128;
    if (tone == 0){ //SAWTOOTH
      return phaseOut;
    }
    else{ //SQUARE
      if (phaseOut < 128){
        return 0;
      }
      else{
        return 255;
      }
    }
  }
}

// ISR to output sound
void sampleISR() {
  if (sysState.isSender != true){ //Only receivers output sound
    int32_t Vout = 0;
    uint32_t localVolume = __atomic_load_n(&sysState.knobValues[0], __ATOMIC_RELAXED);
    uint32_t localTone = __atomic_load_n(&sysState.knobValues[1], __ATOMIC_RELAXED);
    //Play local keys
    for(int i=0; i<12; i++){
      char localCurrentKeystring = __atomic_load_n(&sysState.keyStrings[i], __ATOMIC_RELAXED);
      if(localCurrentKeystring != '-'){Vout += playNote(sysState.octave, i, localVolume, localTone);}
    }
    // Play received keys
    uint8_t localRX_Message[8];
    for (int i=0; i<8; i++){localRX_Message[i] = __atomic_load_n(&sysState.RX_Message[i], __ATOMIC_RELAXED);}
    if (localRX_Message[0] == 'P'){
      Vout += playNote(localRX_Message[1], localRX_Message[2], localVolume, localTone);
    }
    analogWrite(OUTR_PIN, Vout);
  }
}

// ISR to store incoming CAN RX messages
void CAN_RX_ISR (void) {
	uint8_t RX_Message_ISR[8];
	uint32_t ID;
	CAN_RX(ID, RX_Message_ISR);
	xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
}

// ISR to give the semaphore once mailbox available
void CAN_TX_ISR (void) {
	xSemaphoreGiveFromISR(CAN_TX_Semaphore, NULL);
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

  //Initialise CAN
  // NOTE: When the module is a sender, loopback must be set to true!
  //       This is because without an ACK, the module will try to transmit forever.
  CAN_Init(false);     //Set to true for loopback mode
  setCANFilter(0xd123,0x7ff);    //ID, mask
  CAN_RegisterRX_ISR(CAN_RX_ISR);
  CAN_RegisterTX_ISR(CAN_TX_ISR);
  CAN_Start();
  msgInQ = xQueueCreate(36,8); //No. items, bytes
  msgOutQ = xQueueCreate(36,8);
  CAN_TX_Semaphore = xSemaphoreCreateCounting(3,3);

  //Initialise freeRTOS
  TaskHandle_t updateKeysHandle = NULL;
  xTaskCreate(
  updateKeysTask,		/* Function that implements the task */
  "updateKeys",		/* Text name for the task */
  128,      		/* Stack size in words, not bytes */
  NULL,			/* Parameter passed into the task */
  4,			/* Task priority */
  &updateKeysHandle );	/* Pointer to store the task handle */

  TaskHandle_t updateDisplayHandle = NULL;
  xTaskCreate(
  updateDisplayTask,		/* Function that implements the task */
  "updateDisplay",		/* Text name for the task */
  1024,      		/* Stack size in words, not bytes */
  NULL,			/* Parameter passed into the task */
  1,			/* Task priority */
  &updateDisplayHandle );	/* Pointer to store the task handle */

  TaskHandle_t decodeMessageHandle = NULL;
  xTaskCreate(
  decodeMessageTask,		/* Function that implements the task */
  "decodeMessage",		/* Text name for the task */
  64,      		/* Stack size in words, not bytes */
  NULL,			/* Parameter passed into the task */
  2,			/* Task priority */
  &decodeMessageHandle );	/* Pointer to store the task handle */

  TaskHandle_t CAN_TXHandle = NULL;
  xTaskCreate(
  CAN_TX_Task,		/* Function that implements the task */
  "CAN_TX",		/* Text name for the task */
  64,      		/* Stack size in words, not bytes */
  NULL,			/* Parameter passed into the task */
  3,			/* Task priority */
  &CAN_TXHandle );	/* Pointer to store the task handle */

  //Initialise hardware timer
  TIM_TypeDef *Instance = TIM1;
  HardwareTimer *sampleTimer = new HardwareTimer(Instance);
  sampleTimer->setOverflow(22000, HERTZ_FORMAT);
  sampleTimer->attachInterrupt(sampleISR);
  sampleTimer->resume();

  //Set up mutexes
  sysState.mutex = xSemaphoreCreateMutex();
  vTaskStartScheduler();
}

void loop() {}