/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
****************************************************/

#include "AsyncSonarLib.h"

void AsyncSonar::Start()
{
	delayedStart(millis(), 0);
}

void AsyncSonar::Start(unsigned long startDelayMillis)
{
	delayedStart(millis(), startDelayMillis);
}

void AsyncSonar::Stop()
{
	PcInt::detachInterrupt(EchoPin);
	_status = IDLE;
}

// ISR: echo pulse starts
void AsyncSonar::responseStart(AsyncSonar *item)
{
	item->_responseStartMicros = micros();
	PcInt::detachInterrupt(item->EchoPin);
	PcInt::attachInterrupt(item->EchoPin, responseEnd, item, FALLING);
}

// ISR: echo pulse ends
void AsyncSonar::responseEnd(AsyncSonar *item)
{
	unsigned long endMicros = micros();
	PcInt::detachInterrupt(item->EchoPin);
	item->_lastRaw = endMicros - item->_responseStartMicros;
	item->_status = FINISHED;
	if (item->SonarISR != nullptr) item->SonarISR(*item);
}

void AsyncSonar::Update()
{
	Update(nullptr);
}

void AsyncSonar::Update(AsyncSonar *nextAsyncSonar)
{
	if (_status == IDLE) return;

	if (_status == STARTING && static_cast<unsigned long>(millis() - _startTime) >= _startDelay)
		ping();

	if (_status == WAITING_RESPONSE && static_cast<unsigned long>(millis() - _startTime) >= _timeoutInterval)
	{
		// stop listening first, so the ISR can not finish the measure while we handle the timeout
		PcInt::detachInterrupt(EchoPin);
		if (_status != FINISHED)
		{
			timeOut();
			trigger(nextAsyncSonar);
			return;
		}
	}

	if (_status == FINISHED)
	{
		finishMeasure();
		trigger(nextAsyncSonar);
	}
}

void AsyncSonar::timeOut()
{
	Stop();
	_lastRaw = _timeoutInterval * 1000UL;
	if (OnTimeOut != nullptr) OnTimeOut(*this);
}

void AsyncSonar::finishMeasure()
{
	Stop();
	if (_lastRaw > 0)
	{
		_lastMeasure = _lastRaw;

#ifndef ASYNCSONAR_DISABLE_MEDIAN
		if (!_filterPrimed)
		{
			// fill the window with the first value, so the median is meaningful from the start
			for (uint8_t i = 0; i < 5; i++) _filtered[i] = _lastMeasure;
			_filterPrimed = true;
		}
		_filtered[_filterIndex] = _lastMeasure;
		_filterIndex = (_filterIndex + 1) % 5;
#endif
	}
	if (OnPing != nullptr) OnPing(*this);
}

void AsyncSonar::trigger(AsyncSonar* nextAsyncSonar) const
{
	if (nextAsyncSonar != nullptr)
	{
		nextAsyncSonar->delayedStart(_startTime, nextAsyncSonar->_triggerInterval);
	}
}

void AsyncSonar::delayedStart(unsigned long startTime, unsigned long startDelay)
{
	_status = STARTING;
	_startTime = startTime;
	_startDelay = startDelay;

	if (static_cast<unsigned long>(millis() - _startTime) >= _startDelay) ping();
}

void AsyncSonar::ping()
{
	PcInt::detachInterrupt(EchoPin);

	pinMode(TriggerPin, OUTPUT);
	digitalWrite(TriggerPin, LOW);
	delayMicroseconds(4);
	digitalWrite(TriggerPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(TriggerPin, LOW);
	pinMode(EchoPin, INPUT);  // in one pin mode, this releases the line for the echo
	delayMicroseconds(4);

	_status = WAITING_RESPONSE;
	_startTime = millis();
	PcInt::attachInterrupt(EchoPin, responseStart, this, RISING);
}

unsigned int AsyncSonar::usToMM(unsigned long us) const
{
	return us * 100UL / _soundSpeedFactor;
}

unsigned long AsyncSonar::GetRawUS() const
{
	// 32 bit variable written from the ISR: read it atomically (safe to call from SonarISR too)
	uint8_t oldSREG = SREG;
	cli();
	unsigned long raw = _lastRaw;
	SREG = oldSREG;
	return raw;
}

unsigned int AsyncSonar::GetRawMM() const
{
	return usToMM(GetRawUS());
}

unsigned long AsyncSonar::GetMeasureUS() const
{
	return _lastMeasure;
}

unsigned int AsyncSonar::GetMeasureMM() const
{
	return usToMM(GetMeasureUS());
}

void AsyncSonar::SetTemperatureCorrection(int8_t tempCelsius)
{
	// speed of sound (m/s) = 331 + 0.6 * T
	_soundSpeedFactor = 2000000L / (3310L + 6L * tempCelsius);
}

void AsyncSonar::SetTimeOutDistance(unsigned int distanceMM)
{
	// round-trip time for distanceMM, rounded up, plus 1 ms margin for millis() resolution
	_timeoutInterval = (static_cast<unsigned long>(distanceMM) * _soundSpeedFactor + 99999UL) / 100000UL + 1;
}

void AsyncSonar::SetTimeOut(unsigned int timeOutMillis)
{
	_timeoutInterval = timeOutMillis;
}

void AsyncSonar::SetTriggerInterval(unsigned int intervalMillis)
{
	_triggerInterval = intervalMillis;
}

#ifndef ASYNCSONAR_DISABLE_MEDIAN
unsigned long AsyncSonar::GetFilteredUS() const
{
	if (!_filterPrimed) return GetMeasureUS();
	return GetMedian(_filtered[0], _filtered[1], _filtered[2], _filtered[3], _filtered[4]);
}

unsigned int AsyncSonar::GetFilteredMM() const
{
	return usToMM(GetFilteredUS());
}

unsigned long AsyncSonar::GetMedian(unsigned long a, unsigned long b, unsigned long c, unsigned long d, unsigned long e)
{
	return b < a ? d < c ? b < d ? a < e ? a < d ? e < d ? e : d
		: c < a ? c : a
		: e < d ? a < d ? a : d
		: c < e ? c : e
		: c < e ? b < c ? a < c ? a : c
		: e < b ? e : b
		: b < e ? a < e ? a : e
		: c < b ? c : b
		: b < c ? a < e ? a < c ? e < c ? e : c
		: d < a ? d : a
		: e < c ? a < c ? a : c
		: d < e ? d : e
		: d < e ? b < d ? a < d ? a : d
		: e < b ? e : b
		: b < e ? a < e ? a : e
		: d < b ? d : b
		: d < c ? a < d ? b < e ? b < d ? e < d ? e : d
		: c < b ? c : b
		: e < d ? b < d ? b : d
		: c < e ? c : e
		: c < e ? a < c ? b < c ? b : c
		: e < a ? e : a
		: a < e ? b < e ? b : e
		: c < a ? c : a
		: a < c ? b < e ? b < c ? e < c ? e : c
		: d < b ? d : b
		: e < c ? b < c ? b : c
		: d < e ? d : e
		: d < e ? a < d ? b < d ? b : d
		: e < a ? e : a
		: a < e ? b < e ? b : e
		: d < a ? d : a;
}
#endif
