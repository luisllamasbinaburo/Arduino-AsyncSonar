/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/

// Blocking use: start a measure, wait for it and read the result.
// Wiring: HC-SR04 Trig + Echo -> A0 (one pin mode)

#include <AsyncSonarLib.h>

AsyncSonar sonarA0(A0);

void setup()
{
	Serial.begin(115200);
	sonarA0.SetTemperatureCorrection(28);  // optional, ambient temperature in Celsius
}

void loop()
{
	sonarA0.Start();   // start now
	delay(50);         // wait for the ping to complete
	sonarA0.Update();  // update sonar

	// show results
	Serial.print("Ping: ");
	Serial.print(sonarA0.GetMeasureMM());
	Serial.println(" mm");
}
