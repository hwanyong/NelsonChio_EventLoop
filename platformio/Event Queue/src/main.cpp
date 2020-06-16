#include <cctype>
#include <string>
#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include "../lib/uhd/Task.h"
#include "../lib/uhd/Utills.h"

using namespace UHD;

#define CLRTSKSW    18		// ClearTaskSW
#define STARTSW     5		// StartSW
#define PWM1FRQ     5000	// PWM1freq
#define PWM1CHN     0		// PWM1Channel
#define PWM1RSL     8		// PWM1resolution
#define BUFLEN		15

TaskManagement TaskMng;
void factoryReset();
int clrtsSW = 0;
int startSW = 0;

void setup() {
	Serial.begin(115200);
	EEPROM.begin(TaskMng.getTaskDataLength());

	Serial.println(":: SETUP - STANBY ::");

	pinMode(CLRTSKSW, INPUT);
	pinMode(STARTSW, INPUT);
}

void loop() {
	clrtsSW = digitalRead(CLRTSKSW);
	startSW = digitalRead(STARTSW);

	if (clrtsSW == HIGH) {
		Serial.println("Factory Reset with clear EEROM");
		factoryReset();
	}
	if (startSW == HIGH) {
		Serial.println(":: START TASK(button) ::");
		TaskMng.startTask();
	}

	if (Serial.available()) {
		char buf[BUFLEN];
		Serial.readBytes(buf, BUFLEN);
		Serial.flush();

		switch (buf[0])
		{
		case 'c': // Save Task Data to EEPROM
		case 'C': {
			TaskMng.saveTaskData();
		} break;
		case 'v': // Load Task Data from EEPROM
		case 'V': {
			TaskMng.loadTaskData();
		} break;
		case 'r': // Factory Reset
		case 'R': {
			Serial.println("Factory Reset with clear EEROM");
			factoryReset();
		} break;
		case 'i':
		case 'I': {
			String id;
			char _n[2] = { buf[2], buf[3] };
			int n;
			sscanf(_n, "%d", &n);
			id = String(buf[1]) + n;

			int pin;
			char _p[2] = { buf[4], buf[5] };
			sscanf(_p, "%d", &pin);

			TaskMng.addPin(id, pin);
		} break;
		case 'u':
		case 'U': {
			String id;
			char _n[2] = { buf[2], buf[3] };
			int n;
			sscanf(_n, "%d", &n);
			id = String(buf[1]) + n;

			TaskMng.removePin(id);
		} break;
		case 't': // Add Task
		case 'T': {
			TASKTYPE tp;
			String id;
			char mRotation = None;
			int pwm;
			int exeTime;

			if (buf[1] == 'm' || buf[1] == 'M') {
				tp = DC;

				char _n[2] = { buf[2], buf[3] };
				int n;
				sscanf(_n, "%d", &n);
				id = 'm' + n;

				mRotation = buf[4];

				char _pwm[3] = { buf[5], buf[6], buf[7] };
				sscanf(_pwm, "%d", &pwm);

				char _t[7] = { buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14] };
				sscanf(_t, "%d", &exeTime);

				TaskMng.addTask(tp, id, mRotation, pwm, exeTime, false);
			}
			else if (buf[1] == 's' || buf[1] == 'S') {
				tp = Scale;

				char _n[2] = { buf[2], buf[3] };
				int n;
				sscanf(_n, "%d", &n);
				id = 's' + n;

				char _t[7] = { buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10] };
				sscanf(_t, "%d", &exeTime);

				TaskMng.addTask(tp, id, None, None, exeTime, true);
			}
			else if (buf[1] == 't' || buf[1] == 'T') {}
			else {
				Serial.println("!! This command cannot be executed.");
				break;
			}
		} break;
		case 'p': // Display task(0)/memory(1)
		case 'P': {
			if (buf[1] == '0') {
				TaskMng.printTaskData();
			}
			else if (buf[1] == '1') {
				TaskMng.printEEPROM();
			}
			else { Serial.println("!! This command cannot be executed."); }
		} break;
		case 'b': // rebooting
		case 'B': {
			ESP.restart();
		} break;
		case 'd': // Clear Task data
		case 'D': {
			TaskMng.clearTask();
		} break;
		case 's': // Start Task
		case 'S': {
			Serial.println(":: START TASK ::");
			TaskMng.startTask();
		} break;
		case 'l': // Set Loop Count
		case 'L': {
			Serial.println("+ SET LOOP VALUE");
			char _cnt[11] = { buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9], buf[10], buf[11] };
			int _lpcnt;
			sscanf(_cnt, "%d", &_lpcnt);

			TaskMng.setLoopCount(_lpcnt);

			String msg = String("\tSet Loop Count: ") + TaskMng.getSetLoopCount()
						+ String("\n\tExe Loop Count: ") + TaskMng.getExeLoopCount();
			Serial.println(msg);
		} break;
		default: {
			Serial.println("!! This command cannot be executed.");
		} break;
		}
	}
}

void factoryReset() {
	TaskMng.clearTask();
	TaskMng.saveTaskData();
	ESP.restart();
}

void IRAM_ATTR UHD::onTimer() {
	TaskMng.nextTask();
}