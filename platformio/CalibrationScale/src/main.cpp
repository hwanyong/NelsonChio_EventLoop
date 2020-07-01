#include <Arduino.h>

#define PINBUTTON 21
int btnState = 0;

void setup() {
	Serial.begin(115200);
	pinMode(PINBUTTON, INPUT);
}

void loop() {
	btnState = digitalRead(PINBUTTON);

	if (btnState == HIGH) {
		Serial.println("HIGH");
	}
	else {
		Serial.println("LOW");
	}
}