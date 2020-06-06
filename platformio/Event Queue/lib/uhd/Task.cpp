#include "Task.h"

namespace UHD {
	enum TASKTYPE { None = 0, Motor, Scale };

	class Task {
		public:
			TASKTYPE taskType = None;
			int pw = None;
			int rotation = None;
			int delayTime = None;
			bool checkScale = None;

			void add (TASKTYPE _tt, int _p, int _r, int _d, bool _c) {
				this->taskType = _tt;
				this->pw = _p;
				this->rotation = _r;
				this->delayTime = _d;
				this->checkScale = _c;
			}
			void clear() {
				taskType = None;
				pw = None;
				rotation = None;
				delayTime = None;
				checkScale = None;
			}
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

			hw_timer_t *timer = NULL;

			void startTimer() {
				timer = timerBegin(0, 80, true);											//timer 0, div 80
				timerAttachInterrupt(timer, &onTimer, true);								//attach callback
				timerAlarmWrite(timer, this->taskData.tasks[this->taskData.exeTask].delayTime, false);	//set time in us
				timerAlarmEnable(timer);													//enable interrupt
			}
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
	} TaskMng;

	void IRAM_ATTR onTimer() {
		TaskMng.NextTask();
	}
}