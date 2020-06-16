// #pragma once
#ifndef UHD_TASK_H
#define UHD_TASK_H

#include <Arduino.h>

#define TSKLEN 20
#define PINLEN 50
#define PWM1FREQ 5000
#define PWM1RESOLUTION 8
#define EEPROM_SIZE sizeof(taskData)

using namespace std;
using byte = unsigned char;

namespace UHD
{
	enum TASKTYPE
	{
		None = 0,
		DC,
		Stepper,
		Scale
	};

	class Task
	{
	private:
		void setPins();
		void getPinNumber(String[]);
		void exeDCMotor();
		void exeStepMotor();
		void exeScale();
	public:
		TASKTYPE taskType;
		String id;
		vector<int> pins;
		
		// DCMotor
		int pwm;
		int rotation;
		int speed;
		int stepsPerRevolution;
		int executeTime;
		bool checkScale;

		Task();
		void add(TASKTYPE, String, int, int, int, bool);
		void clear();
		void start(hw_timer_t *, String[]);
		void stop(hw_timer_t *);
	};

	class TaskManagement
	{
	private:
		struct TaskData
		{
			int exeLoopCount;
			int setLoopCount;
			int exeTask;
			int setTask;

			// std::map<String, int> pins;
			String pins[PINLEN];
			Task tasks[TSKLEN];
		} taskData;

		bool isTask();
		bool isPin(int, int, String);
		int getCalcPinForModule(char);

		hw_timer_t *timer = NULL;
		byte eepromBuff[EEPROM_SIZE];

	public:
		TaskManagement();
		void clearTask();
		void startTask();
		void stopTask();
		void nextTask();
		void addPin(String, int);
		void clearPins();
		void removePin(String);
		void removePin(int);
		void addTask(TASKTYPE, String, char, int, int, bool);
		void editTask(int, TASKTYPE, int, char, int, int, bool);
		void setLoopCount(int);
		void printFormatTask(TaskData *);
		void printTaskData();
		void printEEPROM();
		void saveTaskData();
		void loadTaskData();
		int getTaskDataLength();
		int getSetLoopCount();
		int getExeLoopCount();
	};

	extern void IRAM_ATTR onTimer();
} // namespace UHD
#endif