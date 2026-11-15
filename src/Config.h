/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
****************************************************/

#pragma once

// Disable the 5 samples median filter.
// Saves 22 bytes of RAM per sensor (GetFilteredMM / GetFilteredUS are removed).
//#define ASYNCSONAR_DISABLE_MEDIAN

// NOTE: ASYNCSONAR_USE_SONARISR is no longer needed (since v1.1.0).
// The ISR callback is always available as the 4th constructor argument.