/*
  ------------- Manual ------------------
   L: Loop Counts
     ex> L:3 (LoopCount run 3 times) or L,2
   T: Task
  T:1:0001000:255 // Run CW during 1sec at PWM 255
  T:0:0001000:0   // Stop during 1sec at PWM 0
  T:2:0001000:255 // Run CCW during 1sec at PWM 255
   "S:"or hardware "SW1" // Start Tasks and Loop
   "D:"or hardware "SW2" // Delete current Tasks
  DEMO Commands
  L:2 / L,2
  T:1:0003000:255 / T,1,0003000,255
  T:0:0002000:0 / T,0,0002000,0
  T:2:0002000:255 / T,2,0002000,255
  T:0:0001000:0 / T,0,0001000,0
  S
  
  ========================================== */
#include <EEPROM.h>

#define BUFLEN 15
#define TSKLEN 10
#define EEPROM_SIZE sizeof(TaskMng)
// #define PWM1 17

const int ClearTaskSW = 18;
const int StartSW = 5;
const int PWM1 = 17;
const int INA1 = 4;
const int INA2 = 16;

const int PWM1freq = 5000;
const int PWM1Channel = 0;
const int PWM1resolution = 8;

int StartSWstate = 0;
int ClearTaskSWstate = 0;

struct Task {
  int pw = -1;  // pwm
  int rotateType = -1; // 1: CW, 2: CCW // T:3000
  int delayTime = -1;  // 1000ms == 1s
};
struct TaskMng {
  int LoopCount = 0;
  Task tasks[TSKLEN];
  
  // Task control
  int exeTask = 0;
  int setTask = 0;
} taskMng;


// EEPROM variables
// int EEPROM_SIZE = sizeof(Task) * TSKLEN;
byte eepromBuff[EEPROM_SIZE];

void setup() {
  Serial.begin(115200);
  
  EEPROM.begin(EEPROM_SIZE);
  
  pinMode(StartSW, INPUT);
  pinMode(ClearTaskSW, INPUT);
  pinMode(PWM1, OUTPUT);
  pinMode(INA1, OUTPUT);
  pinMode(INA2, OUTPUT);

  ledcSetup(PWM1Channel, PWM1freq, PWM1resolution);
  ledcAttachPin(PWM1, PWM1Channel);

  Serial.println(":: called setup()\n");

  loadTask ();
}

void loop() {
  StartSWstate = digitalRead(StartSW);
  int pw = -1;  // pwm
  if (StartSWstate == HIGH) {
    startTask();
  }

  ClearTaskSWstate = digitalRead(ClearTaskSW);
  if (ClearTaskSWstate == HIGH) {
    clearTask();
    printTasks(taskMng.tasks);
  }

  if (Serial.available() > 0) {
    char buf[BUFLEN];
    Serial.readBytes(buf, BUFLEN);
    Serial.flush();

    // printBuffer(buf);

    switch (buf[0]) {
      case 'L': { // Loop Count 설정
          char _cnt[6] = { buf[2], buf[3], buf[4], buf[5], buf[6], buf[7] };
          int _lpcnt = 0;
          sscanf(_cnt, "%d", &_lpcnt);
          taskMng.LoopCount = _lpcnt;

          Serial.write("LoopCount: ");
          Serial.println(taskMng.LoopCount);
          break;
        }
      case 'T': { // Task 입력
          if (taskMng.setTask >= TSKLEN) {
            Serial.println("\n:: !fully tasks");
            printTasks(taskMng.tasks);
            break;
          }

          // set PWM
          char _pwm[3] = { buf[12], buf[13], buf[14]};
          int _pw;
          sscanf(_pwm, "%d", &_pw);
          taskMng.tasks[taskMng.setTask].pw = _pw;

          // set rotation
          switch (buf[2]) {
            case '0': taskMng.tasks[taskMng.setTask].rotateType = 0; break;
            case '1': taskMng.tasks[taskMng.setTask].rotateType = 1; break;
            case '2': taskMng.tasks[taskMng.setTask].rotateType = 2; break;
          }

          // set delay
          char _times[7] = { buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10] };
          int _time;
          sscanf(_times, "%d", &_time);
          taskMng.tasks[taskMng.setTask].delayTime = _time;

          printTasks(taskMng.tasks);
          taskMng.setTask += 1;

          break;
        }
      case 'S': startTask(); break;
      case 'P': printTasks(taskMng.tasks); break;
      case 'D': {
          clearTask();
          printTasks(taskMng.tasks);
          break;
        }
      case 'C': saveTask(); break;
      case 'V': loadTask(); break;
    }
  }
}

void startTask() {
  Serial.println("\n:: Start Tasks");

  for (int loopCnt = 0; loopCnt < taskMng.LoopCount; loopCnt += 1) {
    Serial.write("-- LOOP ");
    Serial.print(loopCnt + 1);
    Serial.println(" --");

    for (; taskMng.exeTask < taskMng.setTask; taskMng.exeTask += 1) {
      Serial.write("Task[");
      Serial.print(taskMng.exeTask);
      Serial.write("]");
      Serial.println(" >>> Running"); // Add

      Serial.write("\tDelay Time: ");
      Serial.print(taskMng.tasks[taskMng.exeTask].delayTime);
      Serial.println("ms");

      ledcWrite(PWM1Channel, taskMng.tasks[taskMng.exeTask].pw);
      Serial.print("\ttasks[exeTask].pw: ");
      Serial.println(taskMng.tasks[taskMng.exeTask].pw);
      if (taskMng.tasks[taskMng.exeTask].rotateType == 0) {
        digitalWrite(INA1, LOW);
        digitalWrite(INA2, LOW);
      }
      else if (taskMng.tasks[taskMng.exeTask].rotateType == 1) {
        digitalWrite(INA1, HIGH);
        digitalWrite(INA2, LOW);
      }
      else if (taskMng.tasks[taskMng.exeTask].rotateType == 2) {
        digitalWrite(INA1, LOW);
        digitalWrite(INA2, HIGH);
      }
      delay(taskMng.tasks[taskMng.exeTask].delayTime);
    }

    taskMng.exeTask = 0;
  }

  Serial.println(":: Finish Task");
}
void saveTask () {
  // Serial.print("Memory size: ");
  // Serial.println(sizeof(Task) * TSKLEN);
  memcpy(eepromBuff, &taskMng, EEPROM_SIZE);

  for (int addr = 0; addr < EEPROM_SIZE; addr += 1) {
    EEPROM.write(addr, eepromBuff[addr]);
  }
  EEPROM.commit();
  Serial.println("Data Save");
}
void loadTask () {
  for (int addr = 0; addr < EEPROM_SIZE; addr += 1) {
    eepromBuff[addr] = EEPROM.read(addr);
  }
  
  memcpy(&taskMng, eepromBuff, EEPROM_SIZE);
  Serial.println("Data Load");
}
void clearTask () {
  for (int idx = 0; idx < taskMng.setTask; idx += 1) {
    taskMng.tasks[idx].rotateType = -1;
    taskMng.tasks[idx].delayTime = -1;
    taskMng.tasks[idx].pw = -1;
  }
  taskMng.LoopCount = 0;
  taskMng.exeTask = 0;
  taskMng.setTask = 0;
  Serial.println("Clear Task list");
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
  Serial.write(", ");
  Serial.write(buf[8]);
  Serial.write(", ");
  Serial.write(buf[9]);
  Serial.write(", ");
  Serial.write(buf[10]);
  Serial.write(", ");
  Serial.write(buf[11]);
  Serial.write(", ");
  Serial.write(buf[12]);
  Serial.write(", ");
  Serial.write(buf[13]);
  Serial.write(", ");
  Serial.write(buf[14]);
  Serial.write("}\n");
}
void printTasks(Task *tsk) {
  for (int idx = 0; idx < TSKLEN; idx += 1) {
    Serial.write("Task[");
    Serial.print(idx);
    Serial.write("]: ( rotateType: ");
    Serial.print(tsk[idx].rotateType);
    Serial.write(", delayTime: ");
    Serial.print(tsk[idx].delayTime);
    Serial.write(", PWM: ");
    Serial.print(tsk[idx].pw);
    Serial.write(" )\n");
  }

  Serial.write("LoopCount: ");
  Serial.println(taskMng.LoopCount);
  Serial.write("exeTask: ");
  Serial.println(taskMng.exeTask);
  Serial.write("setTask: ");
  Serial.println(taskMng.setTask);
}
