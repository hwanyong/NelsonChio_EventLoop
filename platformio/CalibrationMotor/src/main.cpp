#include <Arduino.h>
#include "HX711.h"

#define PIN_BUTTON			21
#define PIN_LOADCELL_DOUT	13
#define PIN_LOADCELL_SCK	12
#define SCALE_CLBUNIT		0.035274

HX711 scale;

float calibration_factor = 1867.57;

int btnState = -1;
int bfBtnState = -1; // { HIGH, LOW}

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
		}

		String log = "";
		float units = scale.get_units();

		if (units < 0) units = 0.0;
		Serial.println((units * SCALE_CLBUNIT), 4);
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