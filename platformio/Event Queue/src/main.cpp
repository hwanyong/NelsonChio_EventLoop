#include <Arduino.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include <bitset>
#include "../lib/uhd/Task.h"

using namespace UHD;

#define CLRTSKSW    18
#define STARTSW     5
#define PWM1        17
#define INA1        4
#define INA2        16
#define PWM1FRQ     5000
#define PWM1CHN     0
#define PWM1RSL     8

void setup() {
	Serial.begin(115200);
}

void loop() {
	Serial.println("STANBY");
	delay(10000);
}