[Spanish version here](https://github.com/luisllamasbinaburo/Arduino-AsyncSonar/blob/master/readme.md)

# Arduino AsyncSonar Library

The AsyncSonar library allows to control an ultrasonic sensor such as the HC-SR04 in an "asynchronous" (non-blocking) way, i.e. it allows the main loop to perform additional actions while waiting for the Echo to be received.

Additionally the AsyncSonar library allows the sensor to be used using a single pin for Trigger and Echo. Measurements can be obtained in both microseconds and millimeters. A timeout period or distance can also be defined.

On the other hand, it incorporates functions for sound velocity correction with temperature and an extremely fast median filter with a window size of 5 elements.

AsyncSonar objects can be chained, so that several ultrasound sensors are triggered sequentially when the previous ping is finished, without requiring user intervention.

More information https://www.luisllamas.es/libreria-arduino-asyncsonar/

## Requisitos

The AsyncSonar library uses the YetAnotherPcInt library developed by Paulo Costa, available on Github https://github.com/paulo-raca/YetAnotherArduinoPcIntLibrary/ and in the Arduino library manager. This library facilitates the use of Pin Change interrupts.

## Instructions

The AsyncSonar object represents an ultrasonic sensor that performs the measurement actions asynchronously. In order to make the library as unobtrusive as possible, no timers are used. Instead, the philosophy of using an `Update()` method, which updates the state of the object, is used.

To perform the actions, AsyncSonar allows defining a callback function, which will be triggered in `Update()` when a valid measurement has been obtained. Additionally, a callback function can be used in case of a timeout.

The sensor is triggered with the `Start()` method to which we can optionally pass a wait value in milliseconds (e.g. for sketch initialization). We can stop the process with the `Stop()` method.

When the sensor is activated, it pings the sensor pin. Once the ping is sent, the state of the pin is changed to receive the Echo. Therefore, only one digital pin is needed to control the sensor.

To wait for the Echo AsyncSonar defines an interrupt on the pin using the YetAnotherPcInt library. When the Echo is received, AsyncSonar stores the time between Ping and Echo, but does not perform any action.

To update the state of AsyncSonar it is necessary to call the `Update()` function, which checks the state of AsyncSonar. If echo has been received, it performs the appropriate callback functions. It is necessary to call the `Update()` function frequently from the main loop.

The callback functions receive as parameter the AsyncSonar object itself so that, within the function, we can use the functions of the AsyncSonar that invoked the method.

The measurement result is obtained with the `GetRawMM()`, `GetRawUS()`, `GetMeasureMM()`, `GetMeasureUS()` functions. The RAW functions provide the value obtained in the measurement, while the Measure functions filter out measurements that get negative measurements and timeouts.

A fast 5-element median filter is also incorporated. The `GetFilteredMM()` and `GetFilteredUS()` functions are used to obtain the median filtered values. The median filter also ignores negative measurements and timeouts.

The `Update()` function accepts as an optional parameter an AsyncSonar object, which can be the same object that invokes it or a different one. After receiving the echo, the AsyncSonar passed as parameter will be activated. This facilitates continuous measurements, or sequential multi-sensor readings.

For the configuration of the AsyncSonar, functions for setting the timeout and triggerInterval are available. To improve the accuracy of the sensor, a function for setting the temperature and correcting the sound velocity is available.

In addition, the `config.h` file, which contains additional library options, is available.

Thus, you can disable the median filter by setting `#define ASYNCSONAR_DISABLE_MEDIAN`, which makes the library faster.
On the other hand, you can have the callback function called inside the ISR of the interrupt, instead of in the next Update(), at the cost of slowing down the ISR. To do this, uncomment the line #define `ASYNCSONAR_USE_SONARISR`.

### Constructor

The AsyncSonar class is instantiated through its constructor.

```c++
AsyncSonar(uint8_t trigger_pin,
	void(*on_ping)(AsyncSonar&) = nullptr,
	void(*on_time_out)(AsyncSonar&) = nullptr)
```

If we have uncommented `#define ASYNCSONAR_USE_SONARISR` in the `config.h` file the constructor will be the following.

```c++
AsyncSonar(uint8_t trigger_pin,
					   void (*on_ping)(AsyncSonar&) = nullptr,
					   void (*on_time_out)(AsyncSonar&) = nullptr,
					   void (*isr)(AsyncSonar&) = nullptr)
```

### Using AsyncSonar

The AsyncSonar class has the following methods:

```c++

// Start and stop
void Start();
void Start(unsigned long);
void Stop();

// Update status
void Update();
void Update(AsyncSonar*);

// Get measurements
unsigned int GetRawMM();
unsigned long GetRawUS();
unsigned int GetMeasureMM();
unsigned long GetMeasureUS();

// Configuration
void SetTemperatureCorrection(int8_t tempCelsius);
void SetTimeOutDistance(unsigned int distanceMM);
void SetTimeOut(unsigned int timeOutMillis);
void SetTriggerInterval(unsigned int timeOutMillis);

```

Additionally, you have the following methods if you have commented `#define ASYNCSONAR_DISABLE_MEDIAN` in the `config.h` file.

```c++
unsigned long GetFilteredUS();
unsigned int GetFilteredMM();
```

## Examples

The AsyncSonar library includes the following examples to illustrate its use.

### SyncSimple: Shows blocking usage

```c++
#include "AsyncSonarLib.h"

AsyncSonar sonarA0(A0);

void setup()
{
	Serial.begin(115200);
	sonarA0.SetTemperatureCorrection(28);  // optional
}

void loop()
{
	sonarA0.Start();  // start now
	delay(50);  //wait ping to complete
	sonarA0.Update();  // update sonar

	// show results
	Serial.print("Ping: ");
	Serial.println(sonarA0.GetMeasureMM());
}

```

### SyncWithCallback: Show blocking usage with callback function

```c++
#include "AsyncSonarLib.h"

// ping complete callback
void PingRecieved(AsyncSonar& sonar)
{
	Serial.print("Ping: ");
	Serial.println(sonar.GetMeasureMM());
}

// timeout callback
void TimeOut(AsyncSonar& sonar)
{
	Serial.println("TimeOut");
}

AsyncSonar sonarA0(A0, PingRecieved, TimeOut);

void setup()
{
	Serial.begin(115200);

	sonarA0.SetTemperatureCorrection(28);  // optional
}

void loop()
{
	sonarA0.Start();  // start now
	delay(50);  //wait ping to complete
	sonarA0.Update();  // update sonar
}
```

### AsyncSingle: Show a simple example of asynchronous usage

```c++
#include "AsyncSonarLib.h"

// ping complete callback
void PingRecieved(AsyncSonar& sonar)
{
	Serial.print("Ping: ");
	Serial.println(sonar.GetMeasureMM());
}

// timeout callback
void TimeOut(AsyncSonar& sonar)
{
	Serial.println("TimeOut");
}

AsyncSonar sonarA0(A0, PingRecieved, TimeOut);

void setup()
{
	Serial.begin(115200);

	sonarA0.SetTemperatureCorrection(28);  // optional
	sonarA0.Start(1500);	// start in 1500ms
}

void loop()
{
	// this is where magic begins
	sonarA0.Update();
}
```

### AsyncContinuous: Displays continuous asynchronous usage with callback reactivation

```c++
#include "AsyncSonarLib.h"

AsyncSonar sonarA0(A0, [](AsyncSonar& sonar) { Serial.println(sonar.GetMeasureMM()); Sonar.Start();});

void setup()
{
	Serial.begin(115200);

	sonarA0.Start(1500);	// start in 1500ms
}

void loop()
{
	// this is where magic begins
	sonarA0.Update();
}

void setup()
{
	Serial.begin(115200);

	sonarA0.Start(1500);	// start in 1500ms
}

void loop()
{
	// this is where magic begins
	sonarA0.Update();
}
```

### AsyncChain: Displays continuous asynchronous usage using AsyncSonar itself as parameter in Update

```c++
#include "AsyncSonarLib.h"

// ping complete callback
void PingRecieved(AsyncSonar& sonar)
{
  Serial.print("Ping: ");
  Serial.println(sonar.GetMeasureMM());
}

// timeout callback
void TimeOut(AsyncSonar& sonar)
{
  Serial.println("TimeOut");
}

AsyncSonar sonarA0(A0, PingRecieved, TimeOut);

// --- ## In this demo, this code simulates other project tasks
unsigned long interval = 1000;
unsigned long previousMillis;

void debug(char* text)
{
  Serial.print(text);
  Serial.println(millis());
}

// show the asynchronous behavior by printing TIME each 1000ms
void OtherTasks()
{
  if (static_cast<unsigned long>(millis() - previousMillis) >= interval)
  {
    debug("  TIME: ");
    previousMillis = millis();
  }
}
// ---- End other tasks

void setup()
{
  Serial.begin(115200);

  sonarA0.SetTemperatureCorrection(28);  // optional
  sonarA0.Start(500); // start in 500ms
}

void loop()
{
  // this is where magic begins
  sonarA0.Update(&sonarA0);

  OtherTasks();
}
```

### AsyncChainMedian: Displays asynchronous usage using the median filter.

```c++
#include "AsyncSonarLib.h"

// ping complete callback
void PingRecieved(AsyncSonar& sonar)
{
	Serial.print("Ping: ");
	Serial.println(sonar.GetFilteredMM());
}

AsyncSonar sonarA0(A0, PingRecieved);

void setup()
{
	Serial.begin(115200);

	sonarA0.SetTemperatureCorrection(28);  // optional
	sonarA0.Start(500); // start in 500ms
}

void loop()
{
	// this is where magic begins
	sonarA0.Update(&sonarA0);
}
```

### AsyncChainMultiple: Displays asynchronous continuous measurement with multiple sensors

```c++
#include "AsyncSonarLib.h"

void PingRecieved(AsyncSonar&);
void TimeOut0(AsyncSonar&);
void TimeOut1(AsyncSonar&);

AsyncSonar sonarA0(A0, PingRecieved, TimeOut0);
AsyncSonar sonarA1(A1, PingRecieved, TimeOut1);

// ping complete callback
// (this example shows how to access sonar from callback)
void PingRecieved(AsyncSonar& sonar)
{
	Serial.print("Ping");
	Serial.print(&sonar == &sonarA1);  // print '0' if sonar A0, '1' if sonar A1
	Serial.print(": ");
	Serial.println(sonar.GetMeasureMM());
}

// timeout callbacks
// (this example shows how to use different callbacks for each sensor)
void TimeOut0(AsyncSonar& sonar)
{
	Serial.println("TimeOut0");
}

void TimeOut1(AsyncSonar& sonar)
{
	Serial.println("TimeOut1");
}

void setup()
{
	Serial.begin(115200);

	sonarA0.SetTemperatureCorrection(28);  // optional
	sonarA1.SetTemperatureCorrection(28);  // optional
	sonarA0.Start(500);  // start in 500ms
}

void loop()
{
	// this is where magic begins
	sonarA0.Update(&sonarA1);
	sonarA1.Update(&sonarA0);
}
```

### AsyncISR: Muestra el uso con ejecución de la callback en la ISR- AsyncISR: Shows the use with callback execution in the ISR

```c++
// Uncomment ASYNCSONAR_USE_SONAR ISR in config.harderr
#include "AsyncSonarLib.h"

// ping complete callback
void PingRecieved(AsyncSonar& sonar)
{
	Serial.print("Ping: ");
	Serial.println(sonar.GetMeasureMM());
}

// timeout callback
void TimeOut(AsyncSonar& sonar)
{
	Serial.println("TimeOut");
}

// ISR callback
void SonarISR(AsyncSonar& sonar)
{
	Serial.print("ISR: ");
	Serial.println(sonar.GetRawMM());
}

AsyncSonar sonarA0(A0, PingRecieved, TimeOut, SonarISR);

void setup()
{
	Serial.begin(115200);

	sonarA0.SetTemperatureCorrection(28);  // optional
	sonarA0.Start(500); // start in 500ms
}

void loop()
{
	// this is where magic begins
	sonarA0.Update();

	delay(100);
}
```
