#include <Wire.h>  
#include <Adafruit_SSD1306.h>
#include <bitset>

using byte = unsigned char;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(0x3c, SDA, SCL);
void IRAM_ATTR onTimer();

void setup() {
    Serial.begin(115200);
}

void loop() {
}
