# VirtualSensor
Arduino (PlatformIO) library designed to simulate inputs from sensors to run testing of scenarios that would otherwise be difficult to test.

You need to install and use [pyVirtualSensor](https://github.com/TeamSunride/pyVirtualSensor) to create Simulations that pass data to each VirtualSensor via the serial port.

Developed by [Tom Danvers](https://github.com/TomD53)

# Installation

`platformio.ini` (recommended)
```ini
[env:myenv]
lib_deps = https://github.com/TeamSunride/VirtualSensor.git
```
_alternatively_

    pio lib install https://github.com/TeamSunride/VirtualSensor.git

# Example: Virtual Barometer

## Creating our VirtualBarometer object

This example uses the [Sparkfun BME280 Library](https://github.com/sparkfun/SparkFun_BME280_Arduino_Library/blob/master/src/SparkFunBME280.h) to get data from a sensor. Refer to those docs if you need.

Start by creating a class `VirtualBarometer` which inherits from `VirtualSensor`

It is recommended that you split the code for this into `*.h` and `*.cpp` files

`VirtualBarometer.cpp`

```cpp
#include <Arduino.h>
#include "SparkFunBME280.h"
#include "VirtualSensor.h"


class VirtualBarometer : public VirtualSensor {
    public:
        bool connect();             // method to connect to the sensor using BME280 library
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
```

Now we need to implement each method defined above in the `*.h` files. We will go through them one by one, starting with the constructor, `VirtualBarometer()`

In the constructor method, we define the manufacturer and name of the sensor. This is for logging and identification purposes.

`VirtualBarometer.cpp`
```cpp
#include <Arduino.h>
#include "VirtualBarometer.h"
#include "SparkFunBME280.h"

VirtualBarometer::VirtualBarometer()
{
    manufacturer = "Sparkfun";
    name = "BME280";
}
```

Next, we need to implement the `connect()` function, according to the documentation of the `BME280` library.

Note the absence of `Wire.begin()` as this is done in the `setup()` function of our `main.cpp`

```cpp
bool VirtualBarometer::connect()
{
    println("Starting I2C communication.");
    sensor.setI2CAddress(0x76);
    sensor.beginI2C();
}
```

After this, we need to implement the methods for getting the sensor values. Starting with temperature:

```cpp
// Returns temperature in a range between -40 and 85°C
float VirtualBarometer::getTemperature()
{
    if (isTesting()) {
        return getFloatSimulated("air_temperature");
    }
    return sensor.readTempC();
};
```

As you can see, we use the `isTesting()` function to determine if the sensor is in test mode or not. If in test mode, we return a simulated value for the `air_temperature`, and if not, we return the temperature value from our `BME280` sensor.

Now, we do the same for the other two values we want to get, `air_pressure` and `humidity`:

```cpp
// Returns pressure in Pascals between a range of 30000 and 110000
float VirtualBarometer::getPressure()
{
    if (isTesting()) {
        return getFloatSimulated("air_pressure");
    }
    return sensor.readFloatPressure();
};

// Returns relative humidity as a percentage between 0 and 100
float VirtualBarometer::getHumidity()
{
    if (isTesting()) {
        return getFloatSimulated("humidity");
    }
    return sensor.readFloatHumidity();
};
```

And that's all we need to do in order to create our `VirtualBarometer`.

## Using the VirtualBarometer in a simulation

In this example we'll go over how you can use the VirtualBarometer to test an apogee detection algorithm.

We start by making the necessary imports, setting a constant for our Serial baudrate, and creating an instance of our `VirtualBarometer`.

`main.cpp`
```cpp
#include <Arduino.h>
#include <VirtualBarometer.h>
#include <Wire.h>

const unsigned long SERIAL_BAUD_RATE = 2000000;

VirtualBarometer barometer = VirtualBarometer();
```

In order to detect apogee, we'll take an air pressure reading from the sensor every loop, and check to see if it is higher than the lowest reading we've seen so far. This is because at apogee of a rocket, the air pressure will be at its minimum.

Therefore, we need to create two variables to help. 

```cpp
float lowest_pressure;
bool apogee_detected; // this will turn off apogee detection once we've detected it, so we don't detect it multiple times
```

Next, we need to define the `VirtualSensor::setup()` function. On a regular Arduino, when a Serial connection is started, the program starts from the beginning. However, on a Teensy 4.1, the program continues running. Therefore, in order to enable compatability with the Teensy, we need to have some way of telling `VirtualSensor` how to set up our sensors/simulation.

```cpp
void VirtualSensor::setup() {
    barometer.allowTesting();
    lowest_pressure = barometer.getPressure();
    apogee_detected = false;
}
```

Following this, we run our setup code:

```cpp
void setup() {
    Serial.begin(SERIAL_BAUD_RATE);

    Wire.begin();

    Serial.println("Connecting to barometer");

    barometer.connect();

    VirtualSensor::setup();
}
```

And then enter the main loop, containing our algorithm for apogee detection. When we detect any event, such as `APOGEE`, we need some way of telling `VirtualSensor` that we detected it. That is done with `VirtualSensor::broadcastEvent`

```cpp
void loop() {
    float pressure = barometer.getPressure();
    barometer.println(String("Simulated pressure: ") + pressure);

    if (pressure <= lowest_pressure) {
        lowest_pressure = pressure;
    } else if (!apogee_detected) {
        VirtualSensor::broadcastEvent("APOGEE");
        apogee_detected = true;
    }
}
```

You can view the full C++ example [here](/lib/VirtualSensor/examples/apogee-detect)

In order to pass data to the VirtualSensor, we need to use the [pyVirtualSensor](https://github.com/TeamSunride/pyVirtualSensor) library. 

Refer to [this Python example](https://github.com/TeamSunride/pyVirtualSensor#advanced-example-running-a-simulation-with-openrocket-data).