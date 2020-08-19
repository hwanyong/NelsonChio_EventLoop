#include <Arduino.h>
#include "HX711.h"

using namespace std;

#define PIN_BTNRCD			21
#define PIN_BTNPSH			22
#define PIN_LOADCELL_DOUT	13
#define PIN_LOADCELL_SCK	12
#define SCALE_OUNCES		0.035274

#pragma region Motor output 1/2
#define PIN_MTPIN1			27
#define PIN_MTPIN2			26
#define PIN_ENAMBLE			14

#define FREQ				30000
#define PWMCHNNEL			0
#define RESOLUTION			8
#define MAXWEIGHT			300 // ml
int dutycycle = 255;
#pragma endregion

HX711 scale;

unsigned long
	firstMillis = 0,
	previousMillis = 0,
	currentMillis = 0;

int
	rotateCount = 0,
	crtWeight = 0,
	btnStateRecord = -1,
	btnStatePush = -1,
	bfBtnStateRecord = -1, // { HIGH, LOW}
	bfBtnStatePush = -1;

float calibration_factor = 1867.570000;

void setup() {
	Serial.begin(115200);
	pinMode(PIN_BTNRCD, INPUT);
	pinMode(PIN_BTNPSH, INPUT);
	pinMode(PIN_MTPIN1, OUTPUT);
	pinMode(PIN_MTPIN2, OUTPUT);
	pinMode(PIN_ENAMBLE, OUTPUT);

	ledcSetup(PWMCHNNEL, FREQ, RESOLUTION);
	ledcAttachPin(PIN_ENAMBLE, PWMCHNNEL);

	scale.begin(PIN_LOADCELL_DOUT, PIN_LOADCELL_SCK);
	scale.set_scale(calibration_factor);
	scale.tare();
}

void loop() {
	btnStateRecord = digitalRead(PIN_BTNRCD);
	btnStatePush = digitalRead(PIN_BTNPSH);

	char cmd = ' ';
	if (Serial.available()) { cmd = (char)Serial.read(); }

	// Serial.println(String(btnStateRecord) + ":" + String(bfBtnStateRecord));
	if (btnStatePush == HIGH) {
		if (bfBtnStatePush != btnStatePush) {
			Serial.println("btnStatePush: HIGH");
			bfBtnStatePush = btnStatePush;
		}
	}
	else if (btnStatePush == LOW) {
		if (bfBtnStatePush != btnStatePush) {
			Serial.println("btnStatePush: LOW");
			bfBtnStatePush = btnStatePush;
			crtWeight = 0;
		}
		
		digitalWrite(PIN_MTPIN1, LOW);
		digitalWrite(PIN_MTPIN2, LOW);
		ledcWrite(PWMCHNNEL, 0);
	}
	
	if (btnStateRecord == HIGH && crtWeight < MAXWEIGHT) {
		if (bfBtnStateRecord != btnStateRecord) {
			Serial.println("BUTTON: DOWN");
			bfBtnStateRecord = btnStateRecord;
			// scale.power_up();

			firstMillis = millis();
			previousMillis = firstMillis;

			Serial.println("pwm,rotate count,total millis, step millis, amount");
		}

		digitalWrite(PIN_MTPIN1, LOW);
		digitalWrite(PIN_MTPIN2, HIGH);
		ledcWrite(PWMCHNNEL, dutycycle);

		float units = scale.get_units();	// 10 : 1 seconds
		currentMillis = millis();

		// if (units < 0) units = 0.0;
		crtWeight = units;
		unsigned long totalMillis = currentMillis - firstMillis;
		unsigned long stepMillis = currentMillis - previousMillis;
		previousMillis = currentMillis;

		// temporary code: motor rotation count (s)
		rotateCount = totalMillis / 1000;
		// temporary code: motor rotation count (e)

		String log = String(dutycycle) + "," +
					String(rotateCount) + "," +
					String(totalMillis) + "," +
					String(stepMillis) + "," +
					String(units);
		Serial.println(log);
	}
	else {
		if (bfBtnStateRecord != btnStateRecord) {
			Serial.println("BUTTON: UP");
			bfBtnStateRecord = btnStateRecord;
			// scale.power_down();
		}

		switch (cmd)
		{
		case 'z': // reset (set zero factor)
			scale.tare();
			break;
		case 'd': // Set dutycycle
			break;
		break;
			break;
		}
	}
}