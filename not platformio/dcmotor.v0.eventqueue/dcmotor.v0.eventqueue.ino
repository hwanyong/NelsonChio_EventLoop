/*
* Timer Reference: Project WatchdogTimer
*/

#include "esp_system.h"
#include <bitset>
#include <U8g2lib.h>
#include <BluetoothSerial.h>

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define BTNAME "UHD ESP32"
#define motor1Pin1	27
#define motor1Pin2	26
#define enable1Pin	14
#define RECV_PIN	0
#define QUEUELENGTH	255
#define BTDATALENTH 7

using byte = unsigned char;

BluetoothSerial SerialBT;
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

void IRAM_ATTR onTimer();

class GUI {
	public:
		GUI() {}
		void Init() {
			u8g2.setFont(u8g2_font_5x7_tr);
			u8g2.setFontRefHeightExtendedText();
			u8g2.setDrawColor(1);
			u8g2.setFontPosTop();
			u8g2.setFontDirection(0);
			u8g2.begin();
		}
		void DrawString(char* msg) {
			u8g2.clearBuffer();
			u8g2.drawStr(0, 0, msg);
			u8g2.sendBuffer();
		}
} gui;

class DCMotor {
	// minSpeed: 170
	// maxSpeed: 255
	
	private:
		int _freq;
		int _pwmChannel;
		int _resolution;
		int _pinA;
		int _pinB;
		int _pinEnable;
		int _dutyCycle;
		bool _isCW; // true: CW, false: CCW
	public:
		DCMotor(int pwmChannel, int pinA, int pinB, int pinEnable, int dutyCycle, bool isCW) {
			this->_freq = 30000;
			this->_pwmChannel = pwmChannel;
			this->_resolution = 8;
			this->_pinA = pinA;
			this->_pinB = pinB;
			this->_pinEnable = pinEnable;
			this->_dutyCycle = dutyCycle;
			this->_isCW = isCW;
		}

		void Init() {
			pinMode(this->_pinEnable, OUTPUT);
			pinMode(this->_pinA, OUTPUT);
			pinMode(this->_pinB, OUTPUT);
			
			ledcSetup(this->_pwmChannel, this->_freq, this->_resolution);
			ledcAttachPin(this->_pinEnable, this->_pwmChannel);
		}
		void SetSpeed(int dutyCycle) {
			this->_dutyCycle = dutyCycle;
		}
		void SetRotation(bool isCW) {
			this->_isCW = isCW;
		}
		
		void Start() {
			if (this->_isCW) {
				digitalWrite(this->_pinA, HIGH);
				digitalWrite(this->_pinB, LOW);
			}
			else {
				digitalWrite(this->_pinA, LOW);
				digitalWrite(this->_pinB, HIGH);
			}
			ledcWrite(this->_pwmChannel, this->_dutyCycle);
		}
		void SlowStop() {
			this->_dutyCycle = 0;
			this->Start();
		}
		void Stop() {
			digitalWrite(this->_pinA, LOW);
			digitalWrite(this->_pinB, LOW);
		}
};
class TaskMng {
	private:
		DCMotor dcm = DCMotor(0, motor1Pin1, motor1Pin2, enable1Pin, 255, true);

		#pragma region event queue
		int pointer = 0;
		bool isTasking = false;
		byte queue[QUEUELENGTH][BTDATALENTH];

		int ByteToInt(byte *_vals) {
			int val = 0;
			for (int idx = 3; idx < BTDATALENTH; idx++) {
				val = (val << 8) + _vals[idx];
			}
			
			return val;
		}
		int getTime(byte *_vals) {
			return this->ByteToInt(_vals);
		}
		bool getRotation(int _val) {
			if (_val == 17) { // CW
				return true;
			}
			else { // CCW
				return false;
			}
		}
		void clearQueueItem(int ptr) {
			for (int idx = 0; idx < BTDATALENTH; idx++) {
				this->queue[ptr][idx] = 0;
			}
		}
		bool isTask() {
			if (this->queue[this->pointer][1] == 0) {
				return false;
			}

			return true;
		}
		void ActivateDCMotor() {
			Serial.print("Pointer: ");
			Serial.println(this->pointer);

			Serial.print("Rotation: ");
			Serial.println(this->queue[this->pointer][1]);
			dcm.SetRotation(this->getRotation(this->queue[this->pointer][1]));

			Serial.print("Speed: ");
			Serial.println(this->queue[this->pointer][2]);
			dcm.SetSpeed(this->queue[this->pointer][2]);

			Serial.print("Set Time: ");
			Serial.println(this->getTime(this->queue[this->pointer]));
			
			dcm.Start();
		}
		void DeactivateDCMotor() {
			dcm.Stop();
		}
		#pragma endregion

		#pragma region timer
		hw_timer_t *timer = NULL;

		void startTimer() {
			timer = timerBegin(0, 80, true);											//timer 0, div 80
			timerAttachInterrupt(timer, &onTimer, true);								//attach callback
			timerAlarmWrite(timer, this->getTime(this->queue[this->pointer]), false);	//set time in us
			timerAlarmEnable(timer);													//enable interrupt
		}
		#pragma endregion
	public:
		TaskMng() {
			dcm.Init();

			for(int idx = 0; idx < QUEUELENGTH; idx++) {
				this->clearQueueItem(idx);
			}
		}

		#pragma region event queue
		void AddTask(byte *_task) {
			for (int idx = 0; idx < BTDATALENTH; idx++) {
				this->queue[_task[0]][idx] = _task[idx];
			}
		}
		void NextTask() {
			this->clearQueueItem(this->pointer);
			dcm.Stop();

			this->pointer++;
			Serial.print("::NextTask: [");
			Serial.print(this->pointer);
			Serial.print("] ");
			Serial.println(this->queue[this->pointer][0]);
			Serial.println(this->queue[this->pointer][1]);
			Serial.println(this->queue[this->pointer][2]);
			Serial.println(this->queue[this->pointer][3]);
			Serial.println(this->queue[this->pointer][4]);
			Serial.println(this->queue[this->pointer][5]);
			Serial.println(this->queue[this->pointer][6]);

			if (this->isTask()) {
				this->StartTask();
			}
			else {
				this->StopTask();
			}
		}
		bool StartTask() {
			Serial.println("StartTask()");
			gui.DrawString("Operation");

			if (this->isTask()) {
				this->isTasking = true;
				this->startTimer();
				this->ActivateDCMotor();

				return true;
			}
			
			return false;
		}
		bool StopTask() {
			gui.DrawString("STANBY");
			Serial.println("StopTask()");

			this->isTasking = false;

			return false;
		}
		#pragma endregion
} taskMng;

void setup()
{
	Serial.begin(115200);
	SerialBT.begin(BTNAME); //Bluetooth device name
	
	gui.Init();
	gui.DrawString("STANBY");
	Serial.println("STANBY");
}

void loop()
{
	if (SerialBT.available()) {
		Serial.println("RECV BTMsg");
		
		byte _task[7];
		for (int idx = 0; idx < 7; idx++) {
			_task[idx] = SerialBT.read();
		}
		BTOperation(_task);
	}
}

void BTOperation(byte *_task) {
	Serial.print("task msg: ");
	Serial.println(_task[1]);

	switch (_task[1])
	{
	case 192: // Task Start
		gui.DrawString("RECV: Start Task");
		taskMng.StartTask();
		break;
	case 128: // Task Stop
		gui.DrawString("RECV: Stop Task");
		taskMng.StopTask();
		break;
	case 32: // Task CPLT
		break;
	case 16:
	case 17: // Regist Task
		gui.DrawString("RECV: Add Task");
		taskMng.AddTask(_task);
		break;
	}
}

void IRAM_ATTR onTimer() {
	taskMng.NextTask();
}