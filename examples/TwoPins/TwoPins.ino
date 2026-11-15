/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/

// Classic 4 wire connection: Trig and Echo on different pins.
// Wiring: HC-SR04 Trig -> D2, Echo -> D3
// The Echo pin must support Pin Change Interrupts (any pin on Uno/Nano,
// see README for Mega and Leonardo).

#include <AsyncSonarLib.h>

const uint8_t TRIGGER_PIN = 2;
const uint8_t ECHO_PIN = 3;

void PingReceived(AsyncSonar& sonar)
{
	Serial.print("Distance: ");
	Serial.print(sonar.GetMeasureMM() / 10.0, 1);
	Serial.println(" cm");
}

void TimeOut(AsyncSonar& /*sonar*/)
{
	Serial.println("Out of range");
}

AsyncSonar sonar(TRIGGER_PIN, ECHO_PIN, PingReceived, TimeOut);

void setup()
{
	Serial.begin(115200);

	sonar.SetTimeOutDistance(2000);  // ignore objects further than 2 m
	sonar.SetTriggerInterval(60);    // 60 ms between pings (datasheet recommendation)
	sonar.Start(500);
}

void loop()
{
	sonar.Update(&sonar);
}
