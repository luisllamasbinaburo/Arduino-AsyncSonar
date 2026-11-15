/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/

// Non blocking continuous measures, filtered with the 5 samples median filter.
// Open the Serial Plotter to compare raw and filtered values.
// Wiring: HC-SR04 Trig + Echo -> A0 (one pin mode)

#include <AsyncSonarLib.h>

// ping complete callback
void PingReceived(AsyncSonar& sonar)
{
	Serial.print("Measure:");
	Serial.print(sonar.GetMeasureMM());
	Serial.print(",Filtered:");
	Serial.println(sonar.GetFilteredMM());
}

AsyncSonar sonarA0(A0, PingReceived);

void setup()
{
	Serial.begin(115200);

	sonarA0.SetTemperatureCorrection(28);  // optional
	sonarA0.Start(500);  // start in 500ms
}

void loop()
{
	// this is where magic begins
	sonarA0.Update(&sonarA0);
}
