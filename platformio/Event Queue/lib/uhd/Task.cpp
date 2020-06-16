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
void Task::setPins() {
	// Pin
	for (int i = 0; this->pins.size(); i += 1) {
		pinMode(this->pins[i], OUTPUT);
	}

	// Module
	switch (this->taskType) {
	case DC:
		// ledcSetup(PWM1Channel, PWM1FREQ, PWM1RESOLUTION);
		// ledcAttachPin(PWM1, PWM1Channel);
	break;
	case Stepper:	this->exeStepMotor(); break;
	case Scale:		this->exeScale(); break;
	case None:
	default: break;
	}
}
void Task::getPinNumber(String pins[]) {
	for (int i = 0; i < PINLEN; i += 1) {
		if (pins[i] == id) {
			this->pins.push_back(i);
			pinMode(i, OUTPUT);
			break;
		}
	}
}
void Task::exeDCMotor() {
	switch (this->rotation)
	{
	case LOW:
		digitalWrite(this->pins[0], HIGH);
		digitalWrite(this->pins[1], LOW);
		break;
	case HIGH:
		digitalWrite(this->pins[0], HIGH);
		digitalWrite(this->pins[1], LOW);
		break;
	default:
		digitalWrite(this->pins[0], LOW);
		digitalWrite(this->pins[1], LOW);
		break;
	}
	
	ledcWrite(this->pwm, this->speed);
}
void Task::exeStepMotor() {}
void Task::exeScale() {}
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
void Task::start(hw_timer_t *_timer, String pins[]) {
	Serial.println("\t(activateTask)");

	_timer = timerBegin(0, 80, true);							//timer 0, div 80
	timerAttachInterrupt(_timer, &UHD::onTimer, true);			//attach callback
	timerAlarmWrite(_timer,	this->executeTime * 1000, false);	//set time in us
	timerAlarmEnable(_timer);									//enable interrupt

	this->getPinNumber(pins);

	switch (this->taskType)
	{
	case DC:		this->exeDCMotor(); break;
	case Stepper:	this->exeStepMotor(); break;
	case Scale:		this->exeScale(); break;
	default: break;
	}
}
void Task::stop(hw_timer_t *_timer) {
	Serial.println("\t(deactivateTask)");

	_timer = NULL;
}

TaskManagement::TaskManagement()
{
	Serial.print("call TaskManagement()");
	this->clearPins();
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
void TaskManagement::startTask()
{
	Serial.println("\t(StartTask)");

	String msg = String("\tLoop: ") + (this->taskData.exeLoopCount + 1) + String("/") + this->taskData.setLoopCount + String("\n\tTask Num: ") + (this->taskData.exeTask + 1) + String("/") + this->taskData.setTask;
	Serial.println(msg);

	this->taskData.tasks[this->taskData.exeLoopCount].start(this->timer, this->taskData.pins);
}
void TaskManagement::stopTask()
{
	Serial.println("\t(StopTask)");
	this->taskData.tasks[this->taskData.exeLoopCount].stop(this->timer);
}
void TaskManagement::nextTask()
{
	Serial.println("\t(NextTask)");

	this->stopTask();
	this->taskData.exeTask += 1;

	if (this->isTask()) this->startTask();
}
bool TaskManagement::isPin(int _r, int _p, String _i) {
	for (int i = _p; i < _p + _r; i += 1) {
		if (this->taskData.pins[i] != "" && this->taskData.pins[i] != _i) {
			return false;
		}
	}

	return true;
}
void TaskManagement::addPin(String _i, int _p)
{
	Serial.print("add pin: ");
	Serial.println(_i);

	int range = getCalcPinForModule(_i.charAt(0));

	if (this->isPin(range, _p, _i)) {
		this->removePin(_i);
		for (int i = _p; i < _p + range; i += 1) {
			this->taskData.pins[i] = _i;
		}
	}
	else {
		Serial.println("!!ERROR: Already in use");
	}
}
void TaskManagement::clearPins() {
	this->taskData.pins[PINLEN] = { "" };
}
void TaskManagement::removePin(String _id)
{
	for (int idx = 0; idx < PINLEN; idx += 1) {
		if (this->taskData.pins[idx] == _id) {
			this->taskData.pins[idx] = "";
		}
	}
}
void TaskManagement::removePin(int _p) {
	this->taskData.pins[_p] = "";
}
int TaskManagement::getCalcPinForModule(char moduleID) {
	switch (moduleID)
	{
	case 'm':
	case 'M': return 2;
	case 's':
	case 'S': return 2;
	case 't':
	case 'T': return 4;
	default: return 0;
	}
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
	for (int _i = 0; _i < PINLEN; _i += 1) {
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
