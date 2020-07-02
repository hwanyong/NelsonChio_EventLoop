#include <Arduino.h>
#include "HX711.h"

using namespace std;

#define PIN_BUTTON			21
#define PIN_LOADCELL_DOUT	13
#define PIN_LOADCELL_SCK	12
#define SCALE_OUNCES		0.035274

HX711 scale;

unsigned long
	firstMillis = 0,
	previousMillis = 0,
	currentMillis = 0;

int
	pwm = 0,
	rotateCount = 0,
	maxWeight = 300, // unit: ml
	btnState = -1,
	bfBtnState = -1; // { HIGH, LOW}

float calibration_factor = 1867.570000;

void setup() {
	Serial.begin(115200);
	pinMode(PIN_BUTTON, INPUT);

	scale.begin(PIN_LOADCELL_DOUT, PIN_LOADCELL_SCK);
	scale.set_scale(calibration_factor);
	scale.tare();
}

void loop() {
	btnState = digitalRead(PIN_BUTTON);

	char cmd = ' ';
	if (Serial.available()) { cmd = (char)Serial.read(); }

	// Serial.println(String(btnState) + ":" + String(bfBtnState));
	if (btnState == HIGH) {
		if (bfBtnState != btnState) {
			Serial.println("BUTTON: DOWN");
			bfBtnState = btnState;
			// scale.power_up();

			firstMillis = millis();
			previousMillis = firstMillis;

			Serial.println("pwm,rotate count,total millis, step millis, amount, average");
		}

		float units = scale.get_units();	// 10 : 1 seconds
		currentMillis = millis();

		if (units < 0) units = 0.0;
		unsigned long totalMillis = currentMillis - firstMillis;
		unsigned long stepMillis = currentMillis - previousMillis;
		double averageAmount = units / (float)totalMillis;
		previousMillis = currentMillis;

		// temporary code: motor rotation count (s)
		rotateCount = totalMillis / 1000;
		// temporary code: motor rotation count (e)

		String log = String(pwm) + "," +
					String(rotateCount) + "," +
					String(totalMillis) + "," +
					String(stepMillis) + "," +
					String(units) + ","	+
					String(averageAmount);
		Serial.println(log);
	}
	else {
		if (bfBtnState != btnState) {
			Serial.println("BUTTON: UP");
			bfBtnState = btnState;
			// scale.power_down();
		}

		switch (cmd)
		{
		case 'z': // reset (set zero factor)
			scale.tare();
			break;
		break;
			break;
		}
	}
}