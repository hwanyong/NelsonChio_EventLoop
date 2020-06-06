  /*
 * 입력 메뉴얼
 * [명령어 종류]:[명령 값]
 * 
 * 명령어 종류
 *  L: LoopCount 입력
 *    ex> L:3 (LoopCount 3번으로 설정)
 *  T: Task 값 입력
 *    ex> T:1:10 (CW 방향으로 10초 동안 회)전
 *    ex> T:2:100 (CW 방향으로 10초 동안 회)전
 *  S: Start Tasks (*시작하면 중단할 수 없)
 */

 /*
  * DEMO Commands
  L:2
  T:1:3000
  T:1:3000
  T:1:3000
  S
  */

#include <EEPROM.h>

#define BUFLEN 8
#define TSKLEN 10
#define EEPROM_SIZE sizeof(Task) * TSKLEN

struct Task {
  int rotateType = -1; // 1: CW, 2: CCW
  int delayTime = -1;  // 1000ms == 1s
};

int LoopCount = 0;
Task tasks[TSKLEN];

// Task control
int exeTask = 0;
int setTask = 0;

// EEPROM variables
// int EEPROM_SIZE = sizeof(Task) * TSKLEN;
byte eepromBuff[EEPROM_SIZE];

void setup() {
  Serial.begin(115200);
  Serial.println(":: called setup()\n");
  
  EEPROM.begin(EEPROM_SIZE);
}

void loop() {
  if (Serial.available() > 0) {
    char buf[BUFLEN];
    Serial.readBytes(buf, BUFLEN);
    Serial.flush();
    
    printBuffer(buf);

    switch(buf[0]) {
      case 'L': {
        char _cnt[6] = { buf[2], buf[3], buf[4], buf[5], buf[6], buf[7] };
        sscanf(_cnt, "%d", &LoopCount);
        
        Serial.write("LoopCount: ");
        Serial.println(LoopCount);
        break;
      }
      case 'T': {
        if(setTask >= TSKLEN) {
          Serial.println("\n:: !fully tasks");
          printTasks(tasks);
          break;  
        }
        
        char _times[4] = { buf[4], buf[5], buf[6], buf[7] };
        int _time;
        sscanf(_times, "%d", &_time);

        if (buf[2] == '1') {
          tasks[setTask].rotateType = HIGH;
        }
        else if (buf[2] == '2') {
          tasks[setTask].rotateType = LOW;
        }
        tasks[setTask].delayTime = _time;

        printTasks(tasks);
        setTask += 1;
        
        break;
      }
      case 'S': {
        Serial.println("\n:: Start Tasks");
        
        for (int loopCnt = 0; loopCnt < LoopCount; loopCnt += 1) {
          Serial.write("-- LOOP ");
          Serial.print(loopCnt + 1);
          Serial.println(" --");
          
          for (; exeTask < setTask; exeTask += 1) {
            Serial.write("Task[");
            Serial.print(exeTask);
            Serial.write("]");
  
            Serial.write("\tDelay Time: ");
            Serial.print(tasks[exeTask].delayTime);
            Serial.write("ms\n");
            delay(tasks[exeTask].delayTime);
          }
  
          exeTask = 0;
        }
        
        clearTask();
        Serial.println(":: Finish Task");
        
        break;
      }
      case 'V':
        saveTask();
        break;
      case 'A':
        loadTask();
        break;
    }
  }
}

void saveTask () {
  // Serial.print("Memory size: ");
  // Serial.println(sizeof(Task) * TSKLEN);
  memcpy(eepromBuff, tasks, EEPROM_SIZE);

  for (int addr = 0; addr < EEPROM_SIZE; addr += 1) {
    EEPROM.write(addr, eepromBuff[addr]);
  }
  EEPROM.commit();
}
void loadTask () {
  for (int addr = 0; addr < EEPROM_SIZE; addr += 1) {
    eepromBuff[addr] = EEPROM.read(addr);
  }
  
  memcpy(tasks, eepromBuff, EEPROM_SIZE);
  exeTask = 0;
  setTask = 0;
}
void clearTask () {
  for (int idx = 0; idx < setTask; idx += 1) {
    tasks[idx].rotateType = -1;
    tasks[idx].delayTime = -1;
  }
  
  exeTask = 0;
  setTask = 0;
}

// 무시 하셔도 되는 소스
void printBuffer(char *buf) {
  Serial.write("buf: {");
  Serial.write(buf[0]);
  Serial.write(", ");
  Serial.write(buf[1]);
  Serial.write(", ");
  Serial.write(buf[2]);
  Serial.write(", ");
  Serial.write(buf[3]);
  Serial.write(", ");
  Serial.write(buf[4]);
  Serial.write(", ");
  Serial.write(buf[5]);
  Serial.write(", ");
  Serial.write(buf[6]);
  Serial.write(", ");
  Serial.write(buf[7]);
  Serial.write("}\n");
}
void printTasks(Task *tsk) {
  for (int idx = 0; idx < TSKLEN; idx += 1) {
    Serial.write("Task1: { rotateType: ");
    Serial.print(tsk[idx].rotateType);
    Serial.write(", delayTime: ");
    Serial.print(tsk[idx].delayTime);
    Serial.write(" }\n");
  }

  Serial.write("exeTask: ");
  Serial.println(exeTask);
  Serial.write("setTask: ");
  Serial.println(setTask);
}
