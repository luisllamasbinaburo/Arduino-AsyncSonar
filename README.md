# Librería Arduino AsyncSonar
La librería AsyncSonar permite controlar un sensor de ultrasonidos como el HC-SR04 de forma "asíncrona" (no bloqueante), es decir, permite que el bucle principal realice acciones adicionales mientras está esperando que se reciba el Echo.

Adicionalmente la librería AsyncSonar permite utilizar el sensor empleando un único pin para Trigger y Echo. Las mediciones se pueden obtener tanto en microsegundos como en milímetros. También se puede definir un periodo o distancia de Timeout.

Por otro lado, incorpora funciones para corrección de la velocidad del sonido con la temperatura y un filtro de mediana extremadamente rápido con un tamaño de ventana de 5 elementos.

Los objetos AsyncSonar se pueden encadenar, de forma que varios sensores de ultrasonidos se disparen secuencialmente cuando acabe el ping anterior, sin requerir intervención del usuario.

Más información https://www.luisllamas.es/libreria-arduino-asyncsonar/

## Requisitos
La librería AsyncSonar emplea la librería YetAnotherPcInt desarrollada por Paulo Costa, disponible en Github https://github.com/paulo-raca/YetAnotherArduinoPcIntLibrary/ y en el gestor de librerías de Arduino. Esta librería facilita el uso de las interrupciones Pin Change.

## Instrucciones de uso
El objeto AsyncSonar representa un sensor de ultrasonidos que realiza las acciones de la medición de forma asíncrona. Con objeto de que la librería sea lo menos intrusiva posible no se emplean Timers. En su lugar, se usa la filosofía de emplear un método `Update()`, que actualiza el estado del objeto.

Para realizar las acciones, AsyncSonar permite definir una función de callback, que se lanzará en `Update()` cuando se haya obtenido una medición válida. Adicionalmente, se puede emplear una función de callback en caso de que ocurra un Timeout.

El sensor se activa con el método `Start()` al que, opcionalmente, podemos pasar un valor de espera en milisegundos (por ejemplo, para la inicialización del sketch). Podemos dentener el proceso con el método `Stop()`

Cuando el sensor se activa, realiza el Ping en el pin del sensor. Una vez enviado el ping se cambia el estado del pin para recibir el Echo. Por tanto, únicamente es necesario un pin digital para controlar el sensor.

Para la espera del Echo AsyncSonar define una interrupción en el pin mediante la librería YetAnotherPcInt. Cuando el Echo es recibido, AsyncSonar almacena el tiempo entre Ping y el Echo, pero no realiza ninguna acción.

Para actualizar el estado de AsyncSonar es necesario llamar a la función `Update()`, que comprueba el estado de AsyncSonar. Si se ha recibido el echo, realiza las funciones de callback oportunas. Es necesario llamar a la función `Update()` con frecuencia desde el bucle principal.

Las funciones de callback reciben como parámetro el propio objeto AsyncSonar por lo que, dentro de la función, podemos emplear las funciones del AsyncSonar que ha invocado el método.

El resultado de la medición se obtiene con las funciones `GetRawMM()`, `GetRawUS()`, `GetMeasureMM()`, `GetMeasureUS()`. Las funciones RAW proporcionan el valor obtenido en la medición, mientras que las funciones Measure filtran las mediciones que obtienen mediciones negativas y timeouts.

También se incorpora un filtro de mediana rápido de 5 elementos. Para obtener los valores filtrados con la mediana se emplean las funciones `GetFilteredMM()` y `GetFilteredUS()`. El filtro de mediana también ignora las mediciones negativas y timeouts.

Las función `Update()` admite como parámetro opcional un objeto AsyncSonar, que puede ser el mismo objeto que la invoca u otro distinto. Tras recibir el echo, se activará el AsyncSonar pasado como parámetro. Esto facilita realizar mediciones continuas, o lecturas multisensor de forma secuencial.

Para la configuración del AsyncSonar se disponen funciones para establecer el timeout y el intervalo entre disparos triggerInterval. Para mejorar la precisión del sensor se dispone de la función para establecer la temperatura y corregir la velocidad del sonido.

Adicionalmente se dispone del fichero `config.h`, que contiene opciones adicionales de la librería.

Así, se puede desactivar el filtro de mediana descomentando `#define ASYNCSONAR_DISABLE_MEDIAN`, lo que hace más rápida la librería.
Por otro lado, se puede hacer que la función de callback se llame dentro del ISR de la interrupción, en lugar de en el próximo Update(), a costa de ralentizar la ISR. Para ello, descomentar la línea #define `ASYNCSONAR_USE_SONARISR`. 


### Constructor
La clase AsyncSonar se instancia a través de su constructor.
```c++
AsyncSonar(uint8_t trigger_pin,
	void(*on_ping)(AsyncSonar&) = nullptr,
	void(*on_time_out)(AsyncSonar&) = nullptr)
```

Si tenemos descomentado `#define ASYNCSONAR_USE_SONARISR` en el fichero `config.h` el constructor será el siguiente.
	
```c++
AsyncSonar(uint8_t trigger_pin,
					   void (*on_ping)(AsyncSonar&) = nullptr,
					   void (*on_time_out)(AsyncSonar&) = nullptr,
					   void (*isr)(AsyncSonar&) = nullptr)	
```

### Uso de AsyncSonar
La clase AsyncSonar dispone de los siguientes métodos
```c++

//Iniciar y detener
void Start();
void Start(unsigned long);
void Stop();

//Actualizar el estado
void Update();
void Update(AsyncSonar*);

//Obtener medicion
unsigned int GetRawMM();
unsigned long GetRawUS();
unsigned int GetMeasureMM();
unsigned long GetMeasureUS();

//Configuracion	
void SetTemperatureCorrection(int8_t tempCelsius);
void SetTimeOutDistance(unsigned int distanceMM);
void SetTimeOut(unsigned int timeOutMillis);
void SetTriggerInterval(unsigned int timeOutMillis);
	
```

Adicionalemnte, se tiene los siguientes métodos si se tiene comentado `#define ASYNCSONAR_DISABLE_MEDIAN` en el fichero `config.h`.
```c++
unsigned long GetFilteredUS();
unsigned int GetFilteredMM();
```


## Ejemplos
La librería AsyncSonar incluye los siguientes ejemplos para ilustrar su uso.

* SyncSimple: Muestra el uso bloqueante.
```c++
#include "AsyncSonarLib.h"

AsyncSonar sonarA0(A0);

void setup() 
{
	Serial.begin(115200);
	sonarA0.SetTemperatureCorrection(28);  // optional
}

void loop()
{
	sonarA0.Start();  // start now
	delay(50);  //wait ping to complete
	sonarA0.Update();  // update sonar
	
	// show results
	Serial.print("Ping: ");
	Serial.println(sonarA0.GetMeasureMM());
}

```

* SyncWithCallback: Muestra el uso bloqueante con función de callback
```c++
#include "AsyncSonarLib.h"

// ping complete callback
void PingRecieved(AsyncSonar& sonar)
{
	Serial.print("Ping: ");
	Serial.println(sonar.GetMeasureMM());
}

// timeout callback
void TimeOut(AsyncSonar& sonar)
{
	Serial.println("TimeOut");
}

AsyncSonar sonarA0(A0, PingRecieved, TimeOut);

void setup()
{
	Serial.begin(115200);
	
	sonarA0.SetTemperatureCorrection(28);  // optional
}

void loop()
{
	sonarA0.Start();  // start now
	delay(50);  //wait ping to complete
	sonarA0.Update();  // update sonar
}
```

* AsyncSingle: Muestra un ejemplo sencillo de uso asíncrono
```c++
#include "AsyncSonarLib.h"

// ping complete callback
void PingRecieved(AsyncSonar& sonar)
{
	Serial.print("Ping: ");
	Serial.println(sonar.GetMeasureMM());
}

// timeout callback
void TimeOut(AsyncSonar& sonar)
{
	Serial.println("TimeOut");
}

AsyncSonar sonarA0(A0, PingRecieved, TimeOut);

void setup()
{
	Serial.begin(115200);
	
	sonarA0.SetTemperatureCorrection(28);  // optional
	sonarA0.Start(1500);	// start in 1500ms
}

void loop()
{
	// this is where magic begins
	sonarA0.Update();
}
```

* AsyncContinuous: Muestra el uso asíncrono continuo con reactivacion en el callback
```c++
#include "AsyncSonarLib.h"

AsyncSonar sonarA0(A0, [](AsyncSonar& sonar) { Serial.println(sonar.GetMeasureMM()); Sonar.Start();});

void setup()
{
	Serial.begin(115200);

	sonarA0.Start(1500);	// start in 1500ms
}

void loop()
{
	// this is where magic begins
	sonarA0.Update();
}

void setup()
{
	Serial.begin(115200);

	sonarA0.Start(1500);	// start in 1500ms
}

void loop()
{
	// this is where magic begins
	sonarA0.Update();
}
```

* AsyncChain: Muestra el uso asíncrono continuo usando el propio AsyncSonar como parámetro en Update
```c++
#include "AsyncSonarLib.h"

// ping complete callback
void PingRecieved(AsyncSonar& sonar)
{
  Serial.print("Ping: ");
  Serial.println(sonar.GetMeasureMM());
}

// timeout callback
void TimeOut(AsyncSonar& sonar)
{
  Serial.println("TimeOut");
}

AsyncSonar sonarA0(A0, PingRecieved, TimeOut);

// ---- In this demo, this code simulates other project tasks
unsigned long interval = 1000;
unsigned long previousMillis;

void debug(char* text)
{
  Serial.print(text);
  Serial.println(millis());
}

// show the asynchronous behavior by printing TIME each 1000ms
void OtherTasks()
{
  if (static_cast<unsigned long>(millis() - previousMillis) >= interval)
  {
    debug("  TIME: ");
    previousMillis = millis();
  }
}
// ---- End other tasks

void setup()
{
  Serial.begin(115200);

  sonarA0.SetTemperatureCorrection(28);  // optional
  sonarA0.Start(500); // start in 500ms
}

void loop()
{
  // this is where magic begins
  sonarA0.Update(&sonarA0);

  OtherTasks();
}
```

* AsyncChainMedian: Muestra el uso asíncrono utilizando el filtro de mediana
```c++
#include "AsyncSonarLib.h"

// ping complete callback
void PingRecieved(AsyncSonar& sonar)
{
	Serial.print("Ping: ");
	Serial.println(sonar.GetFilteredMM());
}

AsyncSonar sonarA0(A0, PingRecieved);

void setup()
{
	Serial.begin(115200);

	sonarA0.SetTemperatureCorrection(28);  // optional
	sonarA0.Start(500); // start in 500ms
}

void loop()
{
	// this is where magic begins
	sonarA0.Update(&sonarA0);
}
```

* AsyncChainMultiple: Muestra la medición contínua asíncrona con múltiples sensores
```c++
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
```

* AsyncISR: Muestra el uso con ejecución de la callback en la ISR
```c++
// Uncomment ASYNCSONAR_USE_SONAR ISR in config.harderr
#include "AsyncSonarLib.h"

// ping complete callback
void PingRecieved(AsyncSonar& sonar)
{
	Serial.print("Ping: ");
	Serial.println(sonar.GetMeasureMM());
}

// timeout callback
void TimeOut(AsyncSonar& sonar)
{
	Serial.println("TimeOut");
}

// ISR callback
void SonarISR(AsyncSonar& sonar)
{
	Serial.print("ISR: ");
	Serial.println(sonar.GetRawMM());
}

AsyncSonar sonarA0(A0, PingRecieved, TimeOut, SonarISR);

void setup()
{
	Serial.begin(115200);
	
	sonarA0.SetTemperatureCorrection(28);  // optional
	sonarA0.Start(500); // start in 500ms
}

void loop()
{
	// this is where magic begins
	sonarA0.Update();
	
	delay(100);
}
```
