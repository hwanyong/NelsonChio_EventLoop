#include <Wire.h>  
#include <SSD1306Wire.h>
#include <bitset>

#define TSKLEN 10

using byte = unsigned char;

SSD1306Wire display(0x3c, SDA, SCL);

#pragma region Global Functions HEADER
void IRAM_ATTR onTimer();
int ByteToInt(byte *_vals);
#pragma endregion
#pragma region Global Classes
class Task {
    public:
        int pw = -1;
        int rotation = -1;
        int delayTime = -1;
};
class TaskManagement {
    private:
        struct TaskData {
            int exeLoopCount = 0;
            int setLoopCount = 0;
            int exeTask = 0;
            int setTask = 0;

            Task tasks[TSKLEN];
        } taskData;

        bool isTask() {
            if (this->taskData.exeTask >= this->taskData.setTask) {
                this->taskData.exeTask = 0;
                this->taskData.exeLoopCount += 1;
            }
            if (this->taskData.exeLoopCount >= this->taskData.setLoopCount) {
                this->taskData.setLoopCount = 0;

                return false;
            }

            return true;
        }
        void activateTask() {
            Serial.println("- activateTask");
        }
        void deactivateTask() {
            Serial.println("- deactivateTask");
        }
        void clearTask() {
            for (int idx = 0; idx < this->taskData.setTask; idx += 1) {
                this->taskData.tasks[idx].pw = -1;
                this->taskData.tasks[idx].rotation = -1;
                this->taskData.tasks[idx].delayTime = -1;
            }
            
            this->taskData.exeLoopCount = 0;
            this->taskData.setLoopCount = 0;
            this->taskData.exeTask = 0;
            this->taskData.setTask = 0;
        }

		#pragma region Timer
		hw_timer_t *timer = NULL;

		void startTimer() {
			timer = timerBegin(0, 80, true);											//timer 0, div 80
			timerAttachInterrupt(timer, &onTimer, true);								//attach callback
			timerAlarmWrite(timer, this->taskData.tasks[this->taskData.exeTask].delayTime, false);	//set time in us
			timerAlarmEnable(timer);													//enable interrupt
		}
		#pragma endregion
	public:
		// TaskMng() {
		// }

        void StartTask() {
            this->startTimer();
            this->activateTask();
        }
        void StopTask() {
            this->deactivateTask();
        }
        void NextTask() {
            this->taskData.exeTask += 1;


            if (this->isTask()) {
                this->StartTask();
            }
            else {
                this->StopTask();
            }
        }
} taskMng;
#pragma endregion

void setup() {
    Serial.begin(115200);
    Serial.println(":: STANBY ::");
}

void loop() {
}

#pragma region Global Functions BODY
void IRAM_ATTR onTimer() {
	taskMng.NextTask();
}
int ByteToInt(byte *_vals) {
    int val = 0;
    // for (int idx = 3; idx < TSKLEN; idx++) {
    //     val = (val << 8) + _vals[idx];
    // }
    
    return val;
}
#pragma endregion