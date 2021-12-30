#include <Arduino.h>
#include "SparkFunBME280.h"
#include "VirtualSensor.h"


class VirtualBarometer : public VirtualSensor {
    public:
        void connect();             // method to connect to the sensor using BME280 library
        VirtualBarometer();         // constructor method

        float getTemperature();     // method to get temperature
        float getPressure();        // method to get pressure
        float getHumidity();        // method to get humidity
    private:
        // object to access sensor with BME280 library
        // note that this is declared as private as we do not want the user of VirtualBarometer
        // to have access to the sensor directly.
        BME280 sensor;              
};