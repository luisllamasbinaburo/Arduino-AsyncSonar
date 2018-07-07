/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/
 
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
