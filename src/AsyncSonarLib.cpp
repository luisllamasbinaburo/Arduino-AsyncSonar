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

void AsyncSonar::Start(unsigned long startDelay)
{
	delayedStart(millis(), startDelay);
}

void AsyncSonar::Stop()
{
	PcInt::detachInterrupt(TriggerPin);
	_status = IDLE;
}

void AsyncSonar::responseStart(AsyncSonar *item)
{
	PcInt::detachInterrupt(item->TriggerPin);
	item->_responseStartMicros = micros();
	PcInt::attachInterrupt(item->TriggerPin, responseEnd, item, FALLING);
}

void AsyncSonar::responseEnd(AsyncSonar *item)
{
	PcInt::detachInterrupt(item->TriggerPin);
	item->_responseEndMicros = micros();
	item->_status = FINISHED;
#ifdef ASYNCSONAR_USE_SONARISR
	if(item->SonarISR !=nullptr) item->SonarISR(*item);
#endif
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
		timeOut();
		trigger(nextAsyncSonar);
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
	_lastRaw = _timeoutInterval * 1000;
	if (OnTimeOut != nullptr) OnTimeOut(*this);
}

void AsyncSonar::finishMeasure()
{
	Stop();
	_lastRaw = _responseEndMicros - _responseStartMicros;
	if (_lastRaw > 0)
	{
		_lastMeasure = _lastRaw;
		
#ifndef ASYNCSONAR_DISABLE_MEDIAN
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

void AsyncSonar::delayedStart(unsigned long startTime, unsigned long starDelay)
{
	_status = STARTING;
	_startTime = startTime;
	_startDelay = starDelay;

	if (static_cast<unsigned long>(millis() - _startTime) >= _startDelay) ping();
}

void AsyncSonar::ping()
{
	PcInt::detachInterrupt(TriggerPin);
	
	pinMode(TriggerPin, OUTPUT);
	digitalWrite(TriggerPin, LOW);
	delayMicroseconds(4);
	digitalWrite(TriggerPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(TriggerPin, LOW);
	pinMode(TriggerPin, INPUT);
	delayMicroseconds(4);
	_status = WAITING_RESPONSE;
	_startTime = millis();
	PcInt::attachInterrupt(TriggerPin, responseStart, this, RISING);
}

unsigned long AsyncSonar::GetRawUS() const
{
	return _responseEndMicros - _responseStartMicros;;
}

unsigned int AsyncSonar::GetRawMM() const
{
	return GetRawUS() * 100 / _soundSpeedFactor;
}

unsigned long AsyncSonar::GetMeasureUS() const
{
	return _lastMeasure;
}

unsigned int AsyncSonar::GetMeasureMM() const
{
	return GetMeasureUS() * 100 / _soundSpeedFactor;
}


void AsyncSonar::SetTemperatureCorrection(int8_t tempCelsius)
{
	_soundSpeedFactor = 2000000 / (3310 + 6 * tempCelsius);
}

void AsyncSonar::SetTimeOutDistance(unsigned int distanceMM)
{
	_timeoutInterval = distanceMM * 2 * _soundSpeedFactor / 100000;
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
	unsigned long median = GetMedian(_filtered[0], _filtered[1], _filtered[2], _filtered[3], _filtered[4]);
	return median == 0 ? GetMeasureUS() : median;
}

unsigned int AsyncSonar::GetFilteredMM() const
{
	return GetFilteredUS() * 100 / _soundSpeedFactor;
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
