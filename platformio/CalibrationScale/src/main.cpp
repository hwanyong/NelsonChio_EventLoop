/*
 Setup your scale and start the sketch WITHOUT a weight on the scale
 Once readings are displayed place the weight on the scale
 Press +/- or a/z to adjust the calibration_factor until the output readings match the known weight
 Arduino pin 5 -> HX711 CLK
 Arduino pin 6 -> HX711 DOUT
 Arduino pin 5V -> HX711 VCC
 Arduino pin GND -> HX711 GND 
*/
#include <Arduino.h>
#include "HX711.h"

#define PIN_LOADCELL_DOUT	13
#define PIN_LOADCELL_SCK	12
HX711 scale;

float calibration_factor = 1867.57; // this calibration factor is adjusted according to my load cell
float units;
float ounces;

void setup() {
	Serial.begin(115200);
	scale.begin(PIN_LOADCELL_DOUT, PIN_LOADCELL_SCK);

	scale.set_scale(calibration_factor);
	scale.tare(); //Reset the scale to 0

	// long zero_factor = scale.read_average(); //Get a baseline reading
	// Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
	// Serial.println(zero_factor);
}

void loop() {
	scale.set_scale(calibration_factor); //Adjust to this calibration factor

	Serial.print("Reading: ");
	units = scale.get_units();
	// units = scale.get_units(5);
	// units = scale.read_average(4);
	// units = scale.get_value(4);
	// if (units < 0) units = 0.00;

	ounces = units * 0.035274;
	Serial.print(units, 4);
	Serial.print(" grams"); 
	Serial.print(" calibration_factor: ");
	Serial.print(calibration_factor, 6);
	Serial.println();

	if(Serial.available())
	{
		char temp = Serial.read();

		switch (temp)
		{
		case 'a': calibration_factor += 0.01; break;
		case 's': calibration_factor += 0.1; break;
		case 'd': calibration_factor += 1; break;
		case 'f': calibration_factor += 10; break;

		case 'z': calibration_factor -= 0.01; break;
		case 'x': calibration_factor -= 0.1; break;
		case 'c': calibration_factor -= 1; break;
		case 'v': calibration_factor -= 10; break;
		}
	}
}