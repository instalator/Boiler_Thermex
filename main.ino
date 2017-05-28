#include <avr/wdt.h>

#define SIZEBUFF 40
#define MAXBUFF  20

byte data;
int strob;
int ArrayStrob[SIZEBUFF];
int ArrayData[SIZEBUFF];
int arraytemp[MAXBUFF];
int arrayind1[MAXBUFF];
int arrayind2[MAXBUFF];
int arrayind3[MAXBUFF];
//int pretemp = 99;
int Temperature = 0;
int temp = 0;
int temp1 = 0;
int temp2 = 0;
int ind1 = 0;
int ind2 = 0;
int ind3 = 0;
int power = 0;
int SetT;
bool flagSetTemp = false;
int count = 0;
//////////////////////////////////////////////////////////////////////
void setup() {
  wdt_enable(WDTO_4S);
  Serial.begin(19200);
  DDRC = B00000000;
  PORTC = B00000000;
  DDRB &= ~_BV(0);
  DDRD &= ~_BV(2);
  DDRD &= ~_BV(3);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(4, 1);
  digitalWrite(5, 1);
  digitalWrite(6, 1);
}
/////////////////////////////////////////////////////////////////////
void loop() {
  wdt_reset();
  Buffer();
  Button();
  if (flagSetTemp && count < 20){
    count++;
    setTemp(SetT);
  } else {
    flagSetTemp = false;
    count = 0;
  }
}

void Buffer() {
  for (int m = 0; m < MAXBUFF; m++) {
    for (int i = 0; i < SIZEBUFF; i++) {
      ArrayStrob[i] = (PIND & (1 << PD2));
      ArrayData[i] = PINC;
      ArrayData[i] = (ArrayData[i] << 2) | (PINB & (1 << PB0));
      delay(1);
      Button();
      //Serial.println(ArrayData[i]);
    }
    for (int x = 0; x < SIZEBUFF - 1; x++) {
      int tempbuf = ArrayData[x];
      strob = ArrayStrob[x];
      Button();
           if (tempbuf == 144 && strob) {temp1 = 0;}
      else if (tempbuf == 64  && strob) {temp1 = 0;}
      else if (tempbuf == 121 && strob) {temp1 = 1;}
      else if (tempbuf == 136 && strob) {temp1 = 2;}
      else if (tempbuf == 40  && strob) {temp1 = 3;}
      else if (tempbuf == 49  && strob) {temp1 = 4;}
      else if (tempbuf == 36  && strob) {temp1 = 5;}
      else if (tempbuf == 4   && strob) {temp1 = 6;}
      else if (tempbuf == 120 && strob) {temp1 = 7;}
      else if (tempbuf == 0   && strob) {temp1 = 8;}
      else if (tempbuf == 32  && strob) {temp1 = 9;}
      else if (tempbuf == 132) {temp1 = 9;}
      else if (tempbuf == 64  && !strob) {temp2 = 0;}
      else if (tempbuf == 121 && !strob) {temp2 = 1;}
      else if (tempbuf == 136 && !strob) {temp2 = 2;}
      else if (tempbuf == 40  && !strob) {temp2 = 3;}
      else if (tempbuf == 49  && !strob) {temp2 = 4;}
      else if (tempbuf == 36  && !strob) {temp2 = 5;}
      else if (tempbuf == 4   && !strob) {temp2 = 6;}
      else if (tempbuf == 120 && !strob) {temp2 = 7;}
      else if (tempbuf == 0   && !strob) {temp2 = 8;}
      else if (tempbuf == 32  && !strob) {temp2 = 9;}
      else if (tempbuf == 245) {
        ind1 = 1;
        ind2 = 0;
        ind3 = 0;
      }
      else if (tempbuf == 252) {
        ind1 = 0;
        ind2 = 1;
        ind3 = 0;
      }
      else if (tempbuf == 249) {
        ind1 = 0;
        ind2 = 0;
        ind3 = 1;
      }
      if (tempbuf == 253) {
        ind1 = 0;
        ind2 = 0;
        ind3 = 0;
      }
      String Strtemp =  String(temp1) + String(temp2);
      temp = Strtemp.toInt();
    }
    if (temp > 15) {
      arraytemp[m] = temp;
    }
    arrayind1[m] = ind1;
    arrayind2[m] = ind2;
    arrayind3[m] = ind3;
  }

  //////////////
  int maxtemp = arraytemp[0]; // нулевой элемент будет.
  int maxind1 = arrayind1[0];
  int maxind2 = arrayind2[0];
  int maxind3 = arrayind3[0];
  for (int mx = 1; mx < 10; mx++) {
    Button();
    // if (arraytemp[mx] >= maxtemp){maxtemp = arraytemp[mx];}
    if (arrayind1[mx] >= maxind1) {
      maxind1 = arrayind1[mx];
    }
    if (arrayind2[mx] >= maxind2) {
      maxind2 = arrayind2[mx];
    }
    if (arrayind3[mx] >= maxind3) {
      maxind3 = arrayind3[mx];
    }
  }

  wdt_reset();
  //Находим максимальные значения для отсеивания мусора
  int confidence = 0;
  int* candidate = NULL;
  for (int i = 0; i < MAXBUFF; i++) {
    Button();
    if (confidence == 0) {
      candidate = arraytemp + i;
      confidence++;
    }
    else if (*candidate == arraytemp[i]) {
      confidence++;
    }
    else {
      confidence--;
    }
  }

  if (confidence > 0 && candidate[1] > 15) {
    maxtemp = candidate[1];
  }
  Temperature = maxtemp;
  if (maxind1 || maxind2 || maxind3){
    power = 1;
  } else {
    power = 0;
  }
  Serial.print(maxtemp);
  Serial.print(";");
  Serial.print(maxind1);
  Serial.print(";");
  Serial.print(maxind2);
  Serial.print(";");
  Serial.print(maxind3);
  Serial.print(";");
  Serial.print(power);
  Serial.print(":");
}

void Button() {
  if (Serial.available()) {
    if (Serial.find("cmd")) {
      int cmd = Serial.parseInt();
      if (cmd == 1) {
        Push(4);
      } else if (cmd == 2) {
        Push(5);
      } else if (cmd == 3) {
        Push(6);
      } else if (cmd == 35 || cmd == 40 || cmd == 45 || cmd == 50 || cmd == 55 || cmd == 60 || cmd == 65 || cmd == 70 || cmd == 75){
        SetT = cmd;
        setTemp(SetT);
      }
    }
  }
}
void setTemp(int t){
  if (Temperature != t) {
        Push(6);
        delay(300);
        flagSetTemp = true;
  } else {
    flagSetTemp = false;
  }
}

void Push(int b) {
  digitalWrite(b, LOW);
  delay(50);
  digitalWrite(b, HIGH);
}
