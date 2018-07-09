/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
****************************************************/

#ifndef _ASYNCSONARLIB_h
#define _ASYNCSONARLIB_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <YetAnotherPcInt.h>
#include "Config.h"

enum AsyncSonarStatus
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
	SonarCallback OnPing = nullptr;
	SonarCallback OnTimeOut = nullptr;

#ifndef ASYNCSONAR_USE_SONARISR
	AsyncSonar(uint8_t trigger_pin,
		void(*on_ping)(AsyncSonar&) = nullptr,
		void(*on_time_out)(AsyncSonar&) = nullptr)
		: TriggerPin(trigger_pin), OnPing(on_ping), OnTimeOut(on_time_out) {}


#else
	SonarCallback SonarISR = nullptr;

	AsyncSonar(uint8_t trigger_pin,
					   void (*on_ping)(AsyncSonar&) = nullptr,
					   void (*on_time_out)(AsyncSonar&) = nullptr,
					   void (*isr)(AsyncSonar&) = nullptr)
		: TriggerPin(trigger_pin), OnPing(on_ping), OnTimeOut(on_time_out), SonarISR(isr) {}
#endif

	void Start();
	void Start(unsigned long);
	void Stop();

	void Update();
	void Update(AsyncSonar*);

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
	void SetTriggerInterval(unsigned int timeOutMillis);

private:
	void ping();
	static void responseStart(AsyncSonar*);
	static void responseEnd(AsyncSonar*);
	void timeOut();
	void finishMeasure();
	void trigger(AsyncSonar* nextAsyncSonar) const;
	void delayedStart(unsigned long, unsigned long);

	volatile AsyncSonarStatus _status = IDLE;

	unsigned long _startTime;
	unsigned long _startDelay;
	volatile unsigned long _responseStartMicros;
	volatile unsigned long _responseEndMicros;

	volatile unsigned long _lastRaw;
	volatile unsigned long _lastMeasure;
	volatile unsigned long _filtered[5];
	uint8_t _filterIndex = 0;

	unsigned long _soundSpeedFactor = 583;
	unsigned long _timeoutInterval = 25;
	unsigned long _triggerInterval = 35;

#ifndef ASYNCSONAR_DISABLE_MEDIAN
	static unsigned long GetMedian(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
#endif
};


#endif

