#include "main.h"


// - - - - - - - - - - - - - - READING INPUTS - - - - - - - - - - - - - - 

std::bitset<4> readCols(){
  // Big Endian
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

// Output in zero-indexed array order {3A 3B 2A 2B 1A 1B 0A 0B 3S 2S 1S 0S}
std::bitset<12> readKnobs(){
  std::bitset<12> knobVals;  
  for (int i = 3; i < 7; i++){
    std::bitset<4> rowVals = readRow(i);
    if ((3<=i)&&(i<5)){ //A&B vals
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

char calcJoy(short x, short y, short p){ 
  if(p == 0){  // TODO: engage state machine
    return 'p';
  }
  else if(x < 300 &&  (y < 600 && y > 200)){
    return 'r';
  }
  else if (x > 700 && (y < 600 && y > 200)){
    return 'l';
  }
  else if ((x < 600 && x > 200) && y < 200){
    return 'u';
  }
  else if ((x < 600 && x > 200) && y > 700){
    return 'd';
  }
  else{
    return 's';
  }
}

void navigate(char direction){
  uint8_t localDotLoc[2] = {0};
  xSemaphoreTake(sysState.mutex, portMAX_DELAY);
  
  uint8_t menuState = sysState.menuState;
  bool isSelected = sysState.isSelected;
  // bool metMenuState = sysState.metMenuState;
  bool metOnState = sysState.metOnState;
  int metValue = sysState.met;
  for(int i = 0; i < sizeof(sysState.dotLocation); i++){
    localDotLoc[i] = sysState.dotLocation[i];
  }
  uint8_t octave = sysState.octave;
  xSemaphoreGive(sysState.mutex);

  if(direction=='p') // joystick press changes isSelected state
    isSelected = !isSelected;

  if(isSelected){  // if isSelected is true (i.e. menu item is selected)
    switch(menuState){
      case 0: // metronome menu
        switch(direction){
          case 'u':
            if(metValue == 250)
              metValue = 12;
            else
              metValue++;
            break;
          case 'd':
            if(metValue == 12)
              metValue = 250;
            else
              metValue--;
            break;
          case 'l':
            metOnState = false;
            break;
          case 'r':
            metOnState = true;
            break;
          default:
            break;
        }
        break;
      case 1:  // playback menu
        break;  // TODO: implement REC playback menu
      case 2:  // octave menu
        if(direction == 'u'){
          if(octave < 8){
            octave++;
          }
          else if (octave == 8){
            octave = 1;
          }
        }
        else if(direction == 'd'){
          if(octave > 1){
            octave--;
          }
          else if(octave == 1){
            octave = 8;
          }
        }
      default:
        // undefined menu state
        break;
    }
  }
  else{  // if isSelected is false, it can move between the 3 menu items
    switch (menuState){
      case 0:  // metronome menu
        if(direction=='d'){
          menuState = 1;
          localDotLoc[0] = 58;
          localDotLoc[1] = 12;
        }
        break;
      case 1:  // playback menu
        if(direction =='u'){
          menuState = 0;
          localDotLoc[0] = 58;
          localDotLoc[1] = 4;
        }
        else if(direction =='l'){
          menuState = 2;
          localDotLoc[0] = 2;
          localDotLoc[1] = 12;
        }
        break;
      case 2:  // octave menu
        if(direction == 'r'){
          menuState = 1;
          localDotLoc[0] = 58;
          localDotLoc[1] = 12;
        }
        break;
      default:
        // undefined menu state
        break;
    }
  }
 
  xSemaphoreTake(sysState.mutex, portMAX_DELAY);
  sysState.octave = octave;
  sysState.metOnState = metOnState;
  // sysState.metMenuState = metMenuState; 
  sysState.met = metValue;
  sysState.menuState = menuState;
  sysState.isSelected = isSelected;
  for (int i=0; i<sizeof(localDotLoc); i++){
    sysState.dotLocation[i] = localDotLoc[i];
  }
  xSemaphoreGive(sysState.mutex);
}

// - - - - - - - - - - - - - - TIMED TASKS - - - - - - - - - - - - - - 

void updateKeysTask(void * pvParameters){
  const TickType_t xFrequency = 20/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (1){
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    std::bitset<32> keyBools = readKeys();

    // Store key string locally
    static char localKeyStrings[12] = {
      '-','-','-','-','-','-','-','-','-','-','-','-'
    };
    char prevLocalKeyStrings[12];
    for (int i = 0; i < 12; i++) {
      prevLocalKeyStrings[i] = localKeyStrings[i];
      if (keyBools[i] == 0)
        localKeyStrings[i] = keys[i];
      else
        localKeyStrings[i] = '-';

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
      if(((prevBool == 0b00) && (currBool == 0b01)) || ((prevBool == 0b11) && (currBool == 0b10))){
        localKnobDiffs[3-i] = (localKnobValues[3-i] < knobMaxes[3-i]) ? 1 : 0;
        lastLegalDiff[3-i] = (localKnobValues[3-i] < knobMaxes[3-i]) ? 1 : 0;
      }
      else if(((prevBool == 0b01) && (currBool == 0b00)) || ((prevBool == 0b10) && (currBool == 0b11))){
        localKnobDiffs[3-i] = (localKnobValues[3-i] > 0) ? -1 : 0;
        lastLegalDiff[3-i] = (localKnobValues[3-i] > 0) ? -1 : 0;
      }
      else if(((prevBool == 0b00) && (currBool == 0b11)) || ((prevBool == 0b11) && (currBool == 0b00)) || ((prevBool == 0b01) && (currBool == 0b10)) || ((prevBool == 0b10) && (currBool == 0b01))){
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

    // Store Joystick Values
    joy[0] = analogRead(JOYX_PIN);  // maybe average a few readings?
    joy[1] = analogRead(JOYY_PIN);
    joy[2] = rowVals[2];
    navigate(calcJoy(joy[0], joy[1], joy[2]));

    // Store to sysState
    xSemaphoreTake(sysState.mutex, portMAX_DELAY);
    
    // Store key string globally
    uint8_t localOctave = __atomic_load_n(&sysState.octave, __ATOMIC_RELAXED);
    int idx = (localOctave) * 12;
    for (int i = 0; i < 12; i++){
      __atomic_store_n(&sysState.keyStrings[i], localKeyStrings[i], 
        __ATOMIC_RELAXED);
      __atomic_store_n(&sysState.keys_down[idx++], localKeyStrings[i] != '-',
        __ATOMIC_RELAXED);  // store whether or not the key has been pressed :)
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
  }
}

void updateDisplayTask(void * pvParameters){
  const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (1){
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    // DISPLAY UPDATE
    u8g2.clearBuffer();  // clear internal write memory
    u8g2.setFont(u8g2_font_u8glib_4_tf);  // choose a suitable font
    // Display last sent/received CAN message
    
    // Take all the necessary variables
    xSemaphoreTake(sysState.mutex, portMAX_DELAY); 
    // holds local keypress info
    char localKeyStrings[13] = {0};
    localKeyStrings[12] = '\0';  // 12 keys + null terminator
    for (int i = 0; i < 12; i++){
      localKeyStrings[i] = 
        __atomic_load_n(&sysState.keyStrings[i], __ATOMIC_RELAXED);
    }
    uint8_t localDotLoc[2] = {0};
    uint8_t localOctave = sysState.octave; 
    char localSendState = sysState.TX_Message[0];
    uint8_t localMetValue = sysState.met;
    bool metOnState = sysState.metOnState;
    uint8_t localMenuState = sysState.menuState;
    char cursorSelectState = sysState.isSelected ? '>' : '-';
    const char* localToneNames = toneNames[sysState.knobValues[1]];
    uint32_t localVolValue = sysState.knobValues[0]; 
    for(int i = 0; i < sizeof(sysState.dotLocation); i++){
      localDotLoc[i] = sysState.dotLocation[i];
    }
    xSemaphoreGive(sysState.mutex);

    // TODO: switch menu frames to make this fit*
    // then modify pressed key drawing to all be evenly spaced
    // 61 - 13 (spaces) = 48, 48/12 = 4, so 4 pixels for each key
    // *switch Oct and Met, Rec can be shifted right to match Oct
    // u8g2.drawFrame(0, -2, 61, 8);
    u8g2.drawFrame(0, -2, 56, 8);
    //Display key names
    u8g2.drawStr(2, 4, localKeyStrings);

    // Cursor Shape
    u8g2.setCursor(localDotLoc[0], localDotLoc[1]);
    u8g2.print(cursorSelectState);

    // Octave
    u8g2.drawStr(6, 12, "OCT:");
    u8g2.setCursor(22, 12);
    u8g2.print(localOctave);
    u8g2.setCursor(30,12);
    u8g2.print(localSendState);

    // Metronome
    u8g2.drawStr(62,4, "MET:");
    u8g2.setCursor(79, 4);
    u8g2.print(localMetValue);
    u8g2.drawStr(92, 4, "BPM");
    u8g2.setCursor(114, 4);
    u8g2.print(metOnState ? "ON": "OFF");

    // Playback
    u8g2.drawStr(62, 12, "REC:");
    u8g2.drawCircle(86, 10, 2);
    u8g2.drawBox(97, 8, 5, 5);
    u8g2.drawTriangle(110, 7, 110, 13, 114, 10);

    // - - - Knob Stuff - - -

    // Volume
    u8g2.drawStr(10, 20, "Vol");
    u8g2.drawFrame(1, 22, 28, 12);
    u8g2.setCursor(1+18-5, 29);
    u8g2.print(localVolValue);
    // u8g2.setCursor(16, 29);

    // Tone
    u8g2.drawStr(40, 20, "Tone");
    u8g2.drawFrame(33, 22, 28, 12);
    u8g2.drawStr(33+14-5, 29, localToneNames);

    // Setting
    u8g2.drawStr(66, 20, "Setting");
    u8g2.drawFrame(65, 22, 28, 12);
    u8g2.drawStr(65+14-5, 29, "Vib");

    // Echo
    u8g2.drawStr(104, 20, "Echo");
    u8g2.drawFrame(97, 22, 28, 12);
    u8g2.drawStr(97+14-5, 29, "Rev");

    //TODO: Add a centering function framex+(dist-2)-(floor(word/2))
    //      Word should b 3*(char len+1) - 1  
    u8g2.sendBuffer();  // transfer internal memory to the display
    
    // Toggle LED
    digitalToggle(LED_BUILTIN);  //Toggle LED for CW requirement
  }
}

// - - - - - - - - - - - - - - - - - CAN TASKS - - - - - - - - - - - - - - - - -

void decodeMessageTask(void * pvParameters){
  uint8_t localRX_Message[8] = {0};
  while(1){
    xQueueReceive(msgInQ, localRX_Message, portMAX_DELAY);
    xSemaphoreTake(sysState.mutex, portMAX_DELAY);
    for (int i = 0; i < 8; i++){
      sysState.RX_Message[i] = 
        __atomic_load_n(&localRX_Message[i], __ATOMIC_RELAXED);
    }
    xSemaphoreGive(sysState.mutex);
  }
}

void CAN_RX_ISR (void) {
  uint8_t RX_Message_ISR[8];
  uint32_t ID;
  CAN_RX(ID, RX_Message_ISR);
  xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
}

void CAN_TX_ISR (void) {
  xSemaphoreGiveFromISR(CAN_TX_Semaphore, NULL);
}

void CAN_TX_Task (void * pvParameters) {
  uint8_t msgOut[8];
  while (1) {
    xQueueReceive(msgOutQ, msgOut, portMAX_DELAY);
    xSemaphoreTake(CAN_TX_Semaphore, portMAX_DELAY);
    CAN_TX(0x123, msgOut);
  }
}

// - - - - - - - - - - - - - - - - - NOISE GEN - - - - - - - - - - - - - - - - -

int32_t playNote(uint8_t oct, uint8_t note, uint32_t volume, uint32_t tone){
  // FUNCTION WAVES (based on stepSizes)
  if(tone < 3){
    static uint32_t phaseAcc = 0;
    uint32_t phaseAccChange = 0;
    uint32_t phaseInc = (oct < 4) ? 
      (stepSizes[note] >> (4 - oct)) : 
      (stepSizes[note] << (oct - 4));
    
    phaseAcc += phaseInc;

    uint32_t phaseOut = phaseAcc >> 24;
    switch (tone){
      case 0:  // SAWTOOTH
        return phaseOut;
      case 1:  // SQUARE
        return (phaseOut < 128) ? 128 : 256;  // thresholding phaseAcc
      case 2:  // TRIANGLE
        if (phaseOut > 128)
          return 256 - phaseOut;
        else
          return 2 * phaseOut;
      default:  // shouldn't happen
        return 0;
    }
  }
  else  // this shouldn't happen
    return 0;
}


int32_t playNotes(const uint32_t &vol, const uint32_t &tone){
  uint8_t localOctave = __atomic_load_n(&sysState.octave, __ATOMIC_RELAXED);
  int32_t Vout = 0;

  //Play local keys
  int idx = (localOctave - 1) * 12;
  for(int i = 0; i < 12; i++){
    bool local_key_down = 
      __atomic_load_n(&sysState.keys_down[idx++], __ATOMIC_RELAXED);
    //if(local_key_down){
    char localCurrentKeystring = 
      __atomic_load_n(&sysState.keyStrings[i], __ATOMIC_RELAXED);
    if(localCurrentKeystring != '-'){
      Vout += playNote(localOctave, i, vol, tone);
    }//}
  }

  return Vout >> (8 - vol);
}

void sampleISR() {
  if (!sysState.isSender){  //Only receivers output sound
    const uint32_t localVolume = __atomic_load_n(&sysState.knobValues[0], __ATOMIC_RELAXED);
    const uint32_t localTone = __atomic_load_n(&sysState.knobValues[1], __ATOMIC_RELAXED);

    uint8_t localRX_Info[3];
    for (int i = 0; i < 3; i++)
      localRX_Info[i] = 
        __atomic_load_n(&sysState.RX_Message[i], __ATOMIC_RELAXED);

    // stores transmitted data into sysState
    if (localRX_Info[0] == 'P'){
      int note_index = localRX_Info[1]*12 + localRX_Info[2];
      sysState.keys_down[note_index] = 1;
      // TODO: ^ should this be atomic?
    }
    // TODO: ^ should this be a for loop or sth?
    
    uint8_t Vout = playNotes(localVolume, localTone);

    analogWrite(OUTR_PIN, Vout);
  }
}

// - - - - - - - - - - - - - - - - - - SETUP - - - - - - - - - - - - - - - - - -

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
  msgInQ = xQueueCreate(36,8);  //No. items, bytes
  msgOutQ = xQueueCreate(36,8);
  CAN_TX_Semaphore = xSemaphoreCreateCounting(3,3);

  //Initialise freeRTOS
  TaskHandle_t updateKeysHandle = NULL;
  xTaskCreate(
    updateKeysTask,     /* Function that implements the task */
    "updateKeys",       /* Text name for the task */
    128,                /* Stack size in words, not bytes */
    NULL,               /* Parameter passed into the task */
    4,                  /* Task priority */
    &updateKeysHandle   /* Pointer to store the task handle */
  );

  TaskHandle_t updateDisplayHandle = NULL;
  xTaskCreate(
    updateDisplayTask,    // Function name
    "updateDisplay",      // Text name
    1024,                 // Stack size (words)
    NULL,                 // Parameter for task
    1,                    // Priority
    &updateDisplayHandle  // Pointer
  );

  TaskHandle_t decodeMessageHandle = NULL;
  xTaskCreate(
    decodeMessageTask,    // Function name
    "decodeMessage",      // Text name
    64,                   // Stack size (words)
    NULL,                 // Parameter for task
    2,                    // Priority
    &decodeMessageHandle  // Pointer
  );

  TaskHandle_t CAN_TXHandle = NULL;
  xTaskCreate(
    CAN_TX_Task,    // Function name
    "CAN_TX",       // Text name
    64,             // Stack size (words)
    NULL,           // Parameter for task
    3,              // Priority
    &CAN_TXHandle   // Pointer
  );

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

// all tasks handled by TaskHandlers set up in setup function
void loop() {}