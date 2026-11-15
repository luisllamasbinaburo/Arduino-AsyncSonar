/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
****************************************************/

#ifndef ASYNCSONARLIB_H
#define ASYNCSONARLIB_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <YetAnotherPcInt.h>
#include "Config.h"

// uint8_t underlying type: reads/writes are atomic on AVR (shared with the ISR)
enum AsyncSonarStatus : uint8_t
{
	STARTING,
	WAITING_RESPONSE,
	FINISHED,
	IDLE,
};

class AsyncSonar
{
public:
	typedef void (*SonarCallback)(AsyncSonar&);

	uint8_t TriggerPin;
	uint8_t EchoPin;
	SonarCallback OnPing = nullptr;
	SonarCallback OnTimeOut = nullptr;
	SonarCallback SonarISR = nullptr;  // called from the interrupt when the echo ends

	// One pin mode: Trig and Echo connected to the same Arduino pin
	AsyncSonar(uint8_t pin,
		SonarCallback on_ping = nullptr,
		SonarCallback on_time_out = nullptr,
		SonarCallback isr = nullptr)
		: TriggerPin(pin), EchoPin(pin), OnPing(on_ping), OnTimeOut(on_time_out), SonarISR(isr) {}

	// Two pins mode: Trig and Echo on different pins (Echo must support Pin Change Interrupts)
	AsyncSonar(uint8_t trigger_pin, uint8_t echo_pin,
		SonarCallback on_ping = nullptr,
		SonarCallback on_time_out = nullptr,
		SonarCallback isr = nullptr)
		: TriggerPin(trigger_pin), EchoPin(echo_pin), OnPing(on_ping), OnTimeOut(on_time_out), SonarISR(isr) {}

	void Start();
	void Start(unsigned long startDelayMillis);
	void Stop();

	void Update();
	void Update(AsyncSonar* nextAsyncSonar);

	AsyncSonarStatus GetStatus() const { return _status; }

	unsigned int GetRawMM() const;
	unsigned long GetRawUS() const;
	unsigned int GetMeasureMM() const;
	unsigned long GetMeasureUS() const;

#ifndef ASYNCSONAR_DISABLE_MEDIAN
	unsigned long GetFilteredUS() const;
	unsigned int GetFilteredMM() const;
#endif

	void SetTemperatureCorrection(int8_t tempCelsius);
	void SetTimeOutDistance(unsigned int distanceMM);
	void SetTimeOut(unsigned int timeOutMillis);
	void SetTriggerInterval(unsigned int intervalMillis);

private:
	void ping();
	static void responseStart(AsyncSonar*);
	static void responseEnd(AsyncSonar*);
	void timeOut();
	void finishMeasure();
	void trigger(AsyncSonar* nextAsyncSonar) const;
	void delayedStart(unsigned long startTime, unsigned long startDelay);
	unsigned int usToMM(unsigned long us) const;

	volatile AsyncSonarStatus _status = IDLE;

	unsigned long _startTime = 0;
	unsigned long _startDelay = 0;
	volatile unsigned long _responseStartMicros = 0;
	volatile unsigned long _lastRaw = 0;
	unsigned long _lastMeasure = 0;

	unsigned long _soundSpeedFactor = 583;  // 100 * round-trip microseconds per mm (20 C)
	unsigned long _timeoutInterval = 25;    // ms (~4.3 m)
	unsigned long _triggerInterval = 35;    // ms

#ifndef ASYNCSONAR_DISABLE_MEDIAN
	unsigned long _filtered[5] = { 0, 0, 0, 0, 0 };
	uint8_t _filterIndex = 0;
	bool _filterPrimed = false;

	static unsigned long GetMedian(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
#endif
};

#endif
