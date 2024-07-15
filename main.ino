#define _LCD_TYPE 1
#include <LCD_1602_RUS_ALL.h>
LCD_1602_RUS lcd(0x27, 16, 2);

#define dataPin 3
#define latchPin 2
#define clockPin 4

#define InPin A7
#define OutPin 5
#define sound 6

#define CLK 7
#define DT 8

#include "GyverEncoder.h"
Encoder enc(CLK, DT, ENC_NO_BUTTON, TYPE2);

byte outPort, inPort, regData;

int usedRow[9];
uint32_t timeTest;
int num,
    mode = 0,
    currentCLK,
    lastCLK,
    count = 0,
    menuIndex = 0;

bool isPass, flag;

String menus[] = {
  "таймер:",
  "интервал:",
  "режим:"
};

String modes[] = {
  "полный",
  "частичный"
};

String config[] = {"1", "1", modes[mode]};

uint64_t timer;

void setup() {
  clearArr();
  //enc.setTickMode(AUTO);
  for(int i = 2; i <= 6; i++){
    pinMode(i, OUTPUT);
  }
  pinMode(InPin, INPUT);
  lcd.init();
  lcd.clear();
  lcd.backlight();
  Serial.begin(9600);
  menu();
}

void loop(){
  delay(100);
  if(enc.isTurn()){
    if(enc.isRight()){
    if(menuIndex + 1 < sizeof(menus)/sizeof(String)){
      menuIndex ++;
    }else{
      menuIndex = 0;
    }
  }
  if(enc.isLeft()){
    if(menuIndex > 0){
      menuIndex --;
    }else{
      menuIndex = sizeof(menus)/sizeof(String) - 1;
    }
  }
  menu();
  }
  for(int inEvent = 14; inEvent <= 15; inEvent++){
    inPort = decToBit(inEvent);
    regWrite();
    if (analogRead(InPin) > 600){
      switch (inEvent){
        case 14:{
          lcd.clear();
          lcd.noBacklight();
          if(mode == 0) allMode();
          if(mode == 1) oneMode();
          lcd.backlight();
          menu();
          break;
        }
        case 15:{
          setConfig();
          break;
        }
      }
    }
  }
}


void yield(){
  enc.tick();
}

void setConfig(){
  showConfig();
  byte temp = menuIndex;
  while(true){
    delay(100);
    if(analogRead(InPin) > 600) break;
    if(menuIndex == 2){
      if(enc.isTurn()){
        mode = (mode == 0) ? 1 : 0;
        config[menuIndex] = modes[mode];
        showConfig();
      }
    }else{
      if(enc.isRight()){
        if(config[menuIndex].toInt() < 10.0){
          config[menuIndex] = String(config[menuIndex].toFloat() + 0.1);
        }
        showConfig();
      }
      if(enc.isLeft()){
        if(String(config[menuIndex]).toFloat() > float(1)){
          config[menuIndex] = String(config[menuIndex].toFloat() - 0.1);
        }
        showConfig();
      }
    }
  }
  menu();
}

void showConfig(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("" + menus[menuIndex]);
  lcd.print(String(config[menuIndex]));
  lcd.setCursor(15, 0);
  lcd.print("<");
}

void menu(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(">");
  lcd.print(menus[menuIndex]);
  lcd.print(String(config[menuIndex]));
}

byte decToBit(int num){
  byte rez = 0;
  int temp;
  for(int i = 7; i >= 0; i--){
    temp = num - bit(i);
    if(temp >= 0){
      bitSet(rez, i);
      num = temp;
    }else{
      bitClear(rez,i);
    }
  }
  return rez;
}

void regWrite(){
  digitalWrite(latchPin, LOW);
  regData = inPort | outPort << 4;
  shiftOut(dataPin, clockPin, LSBFIRST, regData);
  digitalWrite(latchPin, HIGH);
  delay(10);
}

void clearArr(){
  for(int i = 0; i <= sizeof(usedRow)/sizeof(int); i++){
    usedRow[i] = 0;
  }
}

int numRow(){
  return random(0, 9);
}

void oneMode(){
  for(int round = 0; round < 9; round++){
    randomSeed(timeTest);
    num = numRow();
    test();
  }
}

void allMode(){
  for(int round = 0; round < 9; round++){
    while(true){
      num = numRow();
      if(usedRow[num] == 0){
        usedRow[num] = 1;
        break;
      }
    }
    test();
  }
  clearArr();
}

void test(){
  isPass = false;
  outPort = decToBit(num);
  regWrite();
  tone(sound, 250, 250*10);
  for(int blink = 0; blink <= 10; blink++){
    digitalWrite(OutPin, !digitalRead(OutPin));
    delay(250);
  }
  digitalWrite(OutPin, 1);
  timeTest = millis();
  while(millis() - timeTest <= config[0].toFloat() * 1000){
    for(int sensor = 0; sensor < 9; sensor++){
      inPort = decToBit(sensor);
      regWrite();
      Serial.print(num); Serial.print("---------------");Serial.println(analogRead(InPin));
      if(analogRead(InPin) < 200 && (sensor == num)){
        win();
        isPass = true;
        break;
      }
    }
  }
  if(isPass == false){
    lose();
  }
  digitalWrite(OutPin, 0);
  delay(config[1].toFloat() * 1000);
}

void win(){
  for(int j = 0; j <= 10; j++){
    digitalWrite(OutPin, !digitalRead(OutPin));
    tone(sound, (j % 2) * 100);
    delay(500);
  }
  noTone(sound);
  digitalWrite(OutPin, 0);
}

void lose(){
  for(int j = 0; j <= 10; j++){
    digitalWrite(OutPin, !digitalRead(OutPin));
    tone(sound, (j % 2) * 1000, 500);
    delay(250);
  }
  noTone(sound);
  digitalWrite(OutPin, 0);
}
