#include "HX711.h"
#define RST 19
#define DOUT  2
#define CLK  0

//HX711 scale(DOUT, CLK);
HX711 scale;
float calibration_factor = 35800.00; //-7050 worked for my 440lb max scale setup
float oldCal_factor;
float oldWeight;

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 calibration sketch");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press + or a to increase calibration factor");
  Serial.println("Press - or z to decrease calibration factor");
  scale.begin(DOUT,CLK); // removed "//HX711 scale(DOUT, CLK);" line and add this new line; Recovered error
  scale.set_scale();
  scale.tare();  //Reset the scale to 0

  long zero_factor = scale.read_average();
  Serial.print("Zero factor: ");
  Serial.println(zero_factor);

  pinMode(RST, INPUT_PULLUP);
}

void loop() {
  scale.set_scale(calibration_factor);
  float weight = scale.get_units(10);

  if (calibration_factor != oldCal_factor || weight != oldWeight) {
    oldCal_factor = calibration_factor;
    oldWeight = weight;
    Serial.print("Reading: ");
    Serial.print(weight, 3);
    Serial.print(" g");
    Serial.print(" calibration_factor: ");
    Serial.print(calibration_factor);
    Serial.println();
  }

  // User entered +, -, a or z in the Serial Monitor window? Adjust cal. factor
  if (Serial.available())
  {
    char temp = Serial.read();
    if (temp == '+' || temp == 'a')
      calibration_factor += 200;
    else if (temp == '-' || temp == 'z')
      calibration_factor -= 200;
  }

  // Has user requested a Zero Reset?
  int rstRequest = digitalRead(RST);
  if (rstRequest == LOW) {
    Serial.println("Resetting ZERO value");
    scale.set_scale();
    scale.tare();  //Reset the scale to 0
  }
}
