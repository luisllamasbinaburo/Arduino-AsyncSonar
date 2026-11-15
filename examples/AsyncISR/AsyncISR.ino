/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/

// Minimum latency: the 4th callback runs inside the interrupt, as soon as
// the echo is received, even while loop() is busy (here, with a delay).
// Keep ISR callbacks very short: no Serial, no delay().
// Wiring: HC-SR04 Trig + Echo -> A0 (one pin mode)

#include <AsyncSonarLib.h>

// ping complete callback (called from Update)
void PingReceived(AsyncSonar& sonar)
{
	Serial.print("Ping: ");
	Serial.print(sonar.GetMeasureMM());
	Serial.println(" mm");
}

// timeout callback (called from Update)
void TimeOut(AsyncSonar& /*sonar*/)
{
	Serial.println("TimeOut");
}

// ISR callback: switch on the built-in LED when an object is closer than 20 cm
void SonarISR(AsyncSonar& sonar)
{
	digitalWrite(LED_BUILTIN, sonar.GetRawMM() < 200 ? HIGH : LOW);
}

AsyncSonar sonarA0(A0, PingReceived, TimeOut, SonarISR);

void setup()
{
	Serial.begin(115200);
	pinMode(LED_BUILTIN, OUTPUT);

	sonarA0.SetTemperatureCorrection(28);  // optional
	sonarA0.Start(500);  // start in 500ms
}

void loop()
{
	// this is where magic begins
	sonarA0.Update(&sonarA0);

	delay(100);  // simulates a slow loop: the LED still reacts immediately
}
