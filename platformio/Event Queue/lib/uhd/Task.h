// #pragma once
#ifndef UHD_TASK_H
#define UHD_TASK_H

#include <map>
#include <Arduino.h>

#define TSKLEN 20
#define EEPROM_SIZE sizeof(taskData)

using byte = unsigned char;

namespace UHD
{
	enum TASKTYPE
	{
		None = 0,
		Motor,
		Scale
	};

	class Task
	{
	public:
		TASKTYPE taskType;
		String id;
		int pwm;
		int rotation;
		int executeTime;
		bool checkScale;

		Task();
		void add(TASKTYPE, String, int, int, int, bool);
		void clear();
		void start();
		void stop();
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
			String pins[50];
			Task tasks[TSKLEN];
		} taskData;

		bool isTask();
		void activateTask();
		void deactivateTask();

		hw_timer_t *timer = NULL;
		byte eepromBuff[EEPROM_SIZE];

		void startTimer();

	public:
		TaskManagement();
		void clearTask();
		void startTask();
		void stopTask();
		void nextTask();
		void addPin(String, int);
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