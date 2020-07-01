#include <Arduino.h>
#include "HX711.h"

#define PIN_BUTTON			21
#define PIN_LOADCELL_DOUT	13
#define PIN_LOADCELL_SCK	12

HX711 scale;
long calivalue = 0;

int btnState = 0;
int bfBtnState = 0; // { HIGH, LOW}

void setup() {
	Serial.begin(115200);
	pinMode(PIN_BUTTON, INPUT);

	scale.begin(PIN_LOADCELL_DOUT, PIN_LOADCELL_SCK);
	scale.power_down();
}

void loop() {
	btnState = digitalRead(PIN_BUTTON);

	char cmd;
	if (Serial.available()) {
		cmd = (char)Serial.read();
	}

	if (btnState == HIGH) {
		if (bfBtnState != btnState) {
			scale.power_up();
		}
	}
	else {
		if (bfBtnState != btnState) {
			scale.power_down();
		}

		switch (cmd)
		{
		case 'i': // initialize
			break;
		case 'r': // reset (set zero)
			scale.tare();
			break;
		default:
			break;
		}
	}
}