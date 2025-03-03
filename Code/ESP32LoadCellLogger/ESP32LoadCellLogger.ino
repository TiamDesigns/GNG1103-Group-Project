#include "HX711.h"
#include "BluetoothSerial.h"

#define DT 3  // hx711 data pin
#define SCK 2 // hx711 clock pin
#define TARE_BTN 4  // tare button pin

BluetoothSerial SerialBT;  // create bluetooth serial object
HX711 scale;
bool newDataset = true;

// creep correction variables
float previousWeight = 0;
unsigned long lastCreepTime = 0;
const float creepThreshold = 10.0;  // change this if creep correction is too sensitive
const unsigned long creepInterval = 30000;  // time interval for creep correction (30 seconds)

void setup() {
    Serial.begin(9600);
    pinMode(TARE_BTN, INPUT_PULLUP);

    // start bluetooth with a pin
    SerialBT.begin("ESP32_BT_Logger", true);  // enable secure pairing
    SerialBT.setPin("1234");  // set bluetooth pin

    scale.begin(DT, SCK);
    scale.set_scale(17896.738281); // set calibration factor
    scale.tare(); // reset scale to zero

    Serial.println("Time(ms),Weight(mg)"); // print csv header
    SerialBT.println("START");  // tell bluetooth that the connection is active
}

// function to adjust for creep (small drifts in weight over time)
long correctCreep(long weight_mg) {
    unsigned long currentTime = millis();

    // check if it's time to adjust for creep
    if (currentTime - lastCreepTime >= creepInterval) {
        float creepOffset = weight_mg - previousWeight;

        // if weight change is very small, reset the scale to correct drift
        if (abs(creepOffset) < creepThreshold) {
            scale.tare();
            Serial.println("creep correction applied");
            SerialBT.println("creep correction applied");
            previousWeight = 0;
            return 0;
        }
        previousWeight = weight_mg;
        lastCreepTime = currentTime;
    }
    return weight_mg;
}

void loop() {
    static bool buttonPressed = false;

    // check if the tare button is pressed
    if (digitalRead(TARE_BTN) == LOW) {
        if (!buttonPressed) {
            scale.tare();
            Serial.println("\nnew dataset");
            SerialBT.println("\nnew dataset");
            newDataset = true;
            buttonPressed = true;
            delay(300); // simple debounce delay
        }
    } else {
        buttonPressed = false;
    }

    // get weight from the scale
    float weight_g = scale.get_units(100); // average 100 readings for stability
    long weight_mg = weight_g * 1000; // convert to mg

    // apply creep correction
    weight_mg = correctCreep(weight_mg);

    // print data to serial monitor
    Serial.print(millis());  // timestamp
    Serial.print(",");
    Serial.println(weight_mg);

    // send data over bluetooth
    SerialBT.print("DATA,");
    SerialBT.print(millis());
    SerialBT.print(",");
    SerialBT.println(weight_mg);

    delay(500);
}
