#include "Task.h"
#include <string>
#include <Arduino.h>
#include <EEPROM.h>

using namespace UHD;

Task::Task()
{
	this->taskType = None;
	this->pwm = None;
	this->rotation = None;
	this->executeTime = None;
	this->checkScale = None;
}
void Task::add(TASKTYPE _tt, String _id, int _p, int _r, int _d, bool _c)
{
	this->taskType = _tt;
	this->id = _id;
	this->pwm = _p;
	this->rotation = _r;
	this->executeTime = _d;
	this->checkScale = _c;
}
void Task::clear()
{
	this->taskType = None;
	this->pwm = None;
	this->rotation = None;
	this->executeTime = None;
	this->checkScale = None;
}
void Task::start() {
	
}
void Task::stop() {}

TaskManagement::TaskManagement()
{
	Serial.print("call TaskManagement()");
	this->taskData.exeLoopCount = 0;
	this->taskData.setLoopCount = 0;
	this->taskData.exeTask = 0;
	this->taskData.setTask = 0;
}
bool TaskManagement::isTask()
{
	if (this->taskData.exeTask >= this->taskData.setTask)
	{
		this->taskData.exeTask = 0;
		this->taskData.exeLoopCount += 1;
	}
	if (this->taskData.exeLoopCount >= this->taskData.setLoopCount)
	{
		this->taskData.setLoopCount = 0;

		return false;
	}

	return true;
}
void TaskManagement::clearTask()
{
	Serial.println("(clearTask)");

	for (int idx = 0; idx < this->taskData.setTask; idx += 1)
	{
		this->taskData.tasks[idx].clear();
	}

	this->taskData.exeLoopCount = 0;
	this->taskData.setLoopCount = 0;
	this->taskData.exeTask = 0;
	this->taskData.setTask = 0;
}
void TaskManagement::activateTask()
{
	Serial.println("\t(activateTask)");
}
void TaskManagement::deactivateTask()
{
	Serial.println("\t(deactivateTask)");
	this->timer = NULL;
}
void TaskManagement::startTimer()
{
	Serial.println("(startTimer)");

	this->timer = timerBegin(0, 80, true);																  //timer 0, div 80
	timerAttachInterrupt(this->timer, &UHD::onTimer, true);												  //attach callback
	timerAlarmWrite(this->timer, this->taskData.tasks[this->taskData.exeTask].executeTime * 1000, false); //set time in us
	timerAlarmEnable(this->timer);																		  //enable interrupt
}
void TaskManagement::startTask()
{
	Serial.println("\t(StartTask)");

	String msg = String("\tLoop: ") + (this->taskData.exeLoopCount + 1) + String("/") + this->taskData.setLoopCount + String("\n\tTask Num: ") + (this->taskData.exeTask + 1) + String("/") + this->taskData.setTask;
	Serial.println(msg);

	this->startTimer();
	this->activateTask();
}
void TaskManagement::stopTask()
{
	Serial.println("\t(StopTask)");
	this->deactivateTask();
}
void TaskManagement::nextTask()
{
	Serial.println("\t(NextTask)");
	this->taskData.exeTask += 1;

	if (this->isTask())
	{
		this->startTask();
	}
	else
	{
		this->stopTask();
	}
}
void TaskManagement::addPin(String _n, int _p)
{
	Serial.print("add pin: ");
	Serial.println(_n);

	this->taskData.pins[_p] = _n;
}
void TaskManagement::removePin(String _n)
{
	for (int _i = 0; _i < sizeof(this->taskData.pins)/sizeof(String); _i += 1) {
		if (this->taskData.pins[_i] == _n) {
			this->taskData.pins[_i] = "";
			break;
		}
	}
}
void TaskManagement::removePin(int _p) {
	this->taskData.pins[_p] = "";
}
void TaskManagement::addTask(TASKTYPE _tp, String _id, char _mr, int _pwm, int _tm, bool _c)
{
	switch (_mr)
	{
	case 's': // Stope
	case 'S': {
		taskData.tasks[taskData.setTask].add(None, _id, _pwm, None, _tm, _c);
	} break;
	case 'c': // CW
	case 'C': {
		taskData.tasks[taskData.setTask].add(_tp, _id, _pwm, LOW, _tm, _c);
	} break;
	case 'w': // CCW
	case 'W': {
		taskData.tasks[taskData.setTask].add(_tp, _id, _pwm, HIGH, _tm, _c);
	} break;
	default: // command for scale
		taskData.tasks[taskData.setTask].add(_tp, _id, _pwm, _mr, _tm, _c);
		break;
	}

	taskData.setTask += 1;
}
void TaskManagement::setLoopCount(int _cnt)
{
	this->taskData.setLoopCount = _cnt;
}
void TaskManagement::printFormatTask(TaskData *_td)
{
	String msg = "\n:: Display Task Data\n" +
				 String("exeLoopCount: ") + _td->exeLoopCount + "\n" +
				 "setLoopCount: " + _td->setLoopCount + "\n" +
				 "exeTask: " + _td->exeTask + "\n" +
				 "setTask: " + _td->setTask + "\n";
	
	msg += "\n----------- Pins -----------\n";
	for (int _i = 0; _i < sizeof(_td->pins)/sizeof(String); _i += 1) {
		msg += "[" + String(_i)  + "]:" + _td->pins[_i] + "\n";
	}
	msg += "------------------------------\n\n";

	msg += "---------- Task Data ---------\n";
	for (int idx = 0; idx < TSKLEN; idx += 1)
	{
		msg += "[" + String(idx) + "] taskType: " + _td->tasks[idx].taskType +
			   ", delayTime: " + _td->tasks[idx].executeTime +
			   ", pw: " + _td->tasks[idx].pwm +
			   ", rotation: " + _td->tasks[idx].rotation +
			   ", checkScale: " + _td->tasks[idx].checkScale + "\n";
	}
	msg += "------------------------------\n";
	Serial.println(msg);
}
void TaskManagement::printTaskData()
{
	this->printFormatTask(&taskData);
}
void TaskManagement::printEEPROM()
{
	TaskData dummy;

	for (int addr = 0; addr < EEPROM_SIZE; addr += 1)
	{
		eepromBuff[addr] = EEPROM.read(addr);
	}

	memcpy(&dummy, eepromBuff, EEPROM_SIZE);
	this->printFormatTask(&dummy);
}
void TaskManagement::saveTaskData()
{
	Serial.println("(saveTaskData)");
	Serial.print(EEPROM_SIZE);
	Serial.println("");

	memcpy(eepromBuff, &taskData, EEPROM_SIZE);
	for (int addr = 0; addr < EEPROM_SIZE; addr += 1)
	{
		EEPROM.write(addr, eepromBuff[addr]);
	}
	EEPROM.commit();
}
void TaskManagement::loadTaskData()
{
	Serial.println("(loadTaskData)");

	for (int addr = 0; addr < EEPROM_SIZE; addr += 1)
	{
		eepromBuff[addr] = EEPROM.read(addr);
	}

	memcpy(&taskData, eepromBuff, EEPROM_SIZE);
}
int TaskManagement::getTaskDataLength()
{
	return EEPROM_SIZE;
}
int TaskManagement::getSetLoopCount()
{
	return this->taskData.setLoopCount;
}
int TaskManagement::getExeLoopCount()
{
	return this->taskData.exeLoopCount;
}
// hidden methods
void exeTaskDCMotor() {

}
void exeTaskStepMotor() {

}
void exeTaskScale() {
	
}