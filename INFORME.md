# Proyecto final — Prototipo de Sistema IoT con AWS y Alexa

**Carrera:** Ingeniería de Sistemas

**Docente**: Eduardo Enrique Marin Garcia

**Asignatura:** SIS-234 - Internet de las Cosas

**Integrantes**: 

- Laura Camacho Lipa
- Sergio Francisco Solis Luizaga
- Cristhian Butron Perez

---

# 1. Requerimientos del Sistema

## 1.1. Requerimientos funcionales

**RF-01:** Cada maceta inteligente deberá medir la humedad del suelo utilizando el sensor conectado a su ESP32 en los siguientes intervalos:

- Para la configuración inicial del prototipo, el sistema realizará una nueva lectura cada 30 segundos cuando no se encuentre regando.
- En modo automático, cuando la humedad sea inferior al umbral mínimo configurado, el sistema activará la bomba durante 5 segundos y luego esperará 30 segundos para permitir la absorción del agua antes de realizar una nueva lectura.

**RF-02:** Cada maceta inteligente debe convertir las lecturas obtenidas del sensor de humedad a un valor porcentual comprendido entre **0% y 100%**.

**RF-03:** Cada maceta inteligente deberá validar que las lecturas crudas del sensor se encuentren dentro del rango permitido del ADC; cuando una lectura sea inválida, deberá descartarla y mantener el último estado válido del riego.

**RF-04:** En modo automático, cada maceta inteligente debe activar el riego cuando la humedad sea menor o igual al límite mínimo configurado para dicha maceta. Para la configuración inicial del prototipo, el límite mínimo será de **30%**.

**RF-05:** En modo automático, cada maceta inteligente debe desactivar el riego cuando la humedad sea mayor o igual al límite máximo configurado para dicha maceta. Para la configuración inicial del prototipo, el límite máximo será de **70%**.

**RF-06:** Cada maceta inteligente debe operar en dos modos de funcionamiento: **automático** y **manual**.

**RF-07:** En modo manual, el sistema debe permitir encender y apagar el riego de una maceta específica mediante comandos de voz enviados desde Alexa, sin que las lecturas del sensor modifiquen automáticamente la acción solicitada.

**RF-08:** El sistema debe permitir al usuario modificar mediante comandos de voz desde Alexa los límites mínimo y máximo de humedad de una maceta específica, almacenando dichos valores en AWS IoT Core mediante el Device Shadow y aplicándolos al funcionamiento automático del ESP32.

**RF-09:** Cada ESP32 deberá enviar a AWS IoT Core, mediante MQTT, cada lectura válida de humedad y cada cambio en el estado del riego, incluyendo como mínimo: humedad, modo de operación y estado del riego.

**RF-10:** Cada ESP32 debe actualizar el estado reportado (`reported`) de su Device Shadow con la información real del dispositivo, incluyendo como mínimo el modo de funcionamiento, el estado del riego y la humedad medida.

**RF-11:** Cada ESP32 debe recibir mediante su Device Shadow los cambios realizados en el estado deseado (`desired`) y aplicar las acciones solicitadas sobre el sistema de riego correspondiente.

**RF-12:** El sistema deberá soportar la asociación de múltiples macetas por usuario.

**RF-13:** El sistema debe permitir al usuario identificar mediante Alexa la maceta sobre la cual desea realizar una acción o consulta, utilizando expresiones asociadas a su nombre o ubicación, tales como “maceta de la sala” o “planta del escritorio”.

**RF-14:** El sistema debe permitir al usuario consultar mediante Alexa la humedad actual, el modo de funcionamiento y el estado del riego de una maceta específica registrada en su cuenta.

**RF-15:** El sistema deberá almacenar los siguientes registros generados por cada maceta inteligente: lecturas de humedad, eventos de riego, consumo estimado y registros de crecimiento.

**RF-16:** El sistema debe permitir al usuario registrar mediante Alexa la altura y la etapa de crecimiento de una maceta específica, almacenando el registro histórico correspondiente.

**RF-17:** El sistema debe permitir al usuario consultar mediante Alexa estadísticas de riego de una maceta específica, informando como mínimo la cantidad de riegos realizados y el consumo estimado de agua y energía durante los últimos siete días.

**RF-18:** Cada dispositivo deberá permitir la configuración de las credenciales de red WiFi mediante un portal cautivo utilizando la librería WiFiManager. 

**RF-19:** El sistema deberá generar dashboards que permitan visualizar la evolución de la humedad, eventos de riego, consumo estimado de agua y estadísticas históricas para apoyar la toma de decisiones.

## 1.2. Requerimientos no funcionales

**RNF-01:** El firmware deberá estar dividido en al menos cuatro módulos independientes: sensor, actuador, conectividad y AWS.

**RNF-02:** La comunicación entre cada ESP32 y AWS IoT Core deberá utilizar MQTT sobre TLS con autenticación mediante certificados digitales X.509.

**RNF-03:** Cada dispositivo deberá reconectarse automáticamente a AWS IoT Core después de una interrupción de conexión, sin requerir reinicio manual del ESP32.

**RNF-04:** Las credenciales sensibles utilizadas por el sistema, tales como claves privadas, certificados, contraseñas de red o credenciales de AWS, no deberán exponerse en el informe técnico ni en repositorios compartidos.

**RNF-05:** El sistema deberá soportar múltiples macetas por usuario sin pérdida de independencia operativa.

**RNF-06:** La Skill de Alexa deberá responder con un mensaje de error específico para:

- maceta inexistente
- maceta no indicada
- comando inválido
- error de comunicación

**RNF-07:** La configuración de red WiFi deberá realizarse mediante WiFiManager sin necesidad de recompilar el firmware para cambiar las credenciales de acceso.

**RNF-08:** La plataforma de visualización deberá permitir consultar información histórica de humedad, riego y consumo mediante gráficos actualizados a partir de los datos almacenados en DynamoDB. 

**RNF-09**: El sistema deberá responder a las solicitudes de control y consulta realizadas desde Alexa en un tiempo menor a 10 segundos bajo condiciones normales de operación.

# 2. Diseño del Sistema

## 2.1. Diagrama de clases

<img style="width:15cm; height:auto;" alt="Diagrama de clases" src="https://github.com/user-attachments/assets/7ca51263-3aa9-44e7-8541-adcc4f08a0e9" />

El diagrama de clases muestra la organización modular del firmware de la maceta inteligente. El módulo principal `SmartPlantApp` coordina la lectura del sensor, el control de la bomba, la activación de LEDs y la publicación de telemetría hacia AWS IoT Core. El módulo `SensorHumedad` obtiene un promedio de lecturas analógicas, valida posibles errores del sensor y convierte el valor obtenido a porcentaje de humedad.

El módulo `BombaAgua` controla el encendido y apagado del sistema de riego, mientras que `IndicadoresLed` muestra visualmente estados como suelo seco, suelo húmedo, error o riego activo. El módulo `AWSIoTClient` gestiona la configuración WiFi mediante WiFiManager, la comunicación MQTT sobre TLS, la suscripción al tópico `shadow/update/delta`, la recepción de comandos desde el Shadow y la publicación de telemetría y estado reportado. La configuración del dispositivo se separa en `ConfiguracionDispositivo`, mientras que los certificados y credenciales de AWS se representan en `CredencialesAWS`, los cuales no deben exponerse públicamente en repositorios ni en el informe técnico.

## 2.2. Diagrama de bloques

<img style="width:15.5cm; height:auto;" alt="Diagrama de bloques" src="https://github.com/user-attachments/assets/4f68bbea-e12e-48fe-8d91-45e6dea29003" />

El diagrama de bloques muestra los componentes principales del sistema de riego inteligente y su relación general. El usuario interactúa mediante una Skill de Alexa, la cual envía los comandos de voz a una función AWS Lambda. Esta función se comunica con AWS IoT Core para consultar o modificar el estado del dispositivo asociado a la maceta inteligente.

La maceta inteligente está compuesta por un ESP32, un sensor de humedad, una bomba de agua y LEDs indicadores. El ESP32 mide la humedad del suelo, controla el riego y reporta información hacia AWS IoT Core mediante MQTT. Los datos generados por el dispositivo son almacenados en DynamoDB y posteriormente utilizados para la generación de dashboards y reportes orientados a la toma de decisiones.

## 2.3. Diagrama de circuito

<img style="width:10.5cm; height:auto;" alt="Diagrama de circuito" src="https://github.com/user-attachments/assets/06587e81-8969-4063-bf9e-bbfcb0447865" />

El diagrama de circuito muestra la conexión física de los componentes utilizados en la maceta inteligente. El sensor de humedad del suelo envía una señal analógica al ESP32, permitiendo obtener el nivel de humedad de la tierra. A partir de esta información y del modo de funcionamiento configurado, el ESP32 determina si corresponde activar o desactivar el sistema de riego.

El control de la bomba de agua se realiza mediante un módulo controlador conectado a los pines de salida del ESP32. Este módulo permite manejar la alimentación requerida por la bomba sin conectarla directamente al microcontrolador. La bomba recibe energía desde una fuente externa y suministra agua a la maceta cuando el sistema activa el riego.

Asimismo, el circuito contempla una referencia de tierra común entre el ESP32, el sensor y el módulo controlador, necesaria para garantizar la correcta lectura de señales y el funcionamiento estable del actuador. El diagrama representa la integración física del prototipo encargado de medir la humedad del suelo y ejecutar el riego automático o manual.

## 2.4. Diagrama de arquitectura del sistema

<img style="width:15.5cm; height:auto;" alt="Diagrama de arquitectura del sistema" src="https://github.com/user-attachments/assets/7f00ab87-7b26-442b-ace3-d60034346a06" />

El diagrama de arquitectura muestra la organización general del sistema IoT para el riego inteligente de macetas. El usuario interactúa mediante una Skill de Alexa en español, indicando el nombre o ubicación de la maceta que desea consultar o controlar. La Skill envía el intent y los slots correspondientes a la función AWS Lambda `alexa-iot-skill-test`.

La función Lambda identifica al usuario de Alexa y consulta DynamoDB para resolver la maceta solicitada. Una vez obtenido el `thing_name`, Lambda consulta o actualiza el Device Shadow asociado al dispositivo. De esta manera, los comandos como encender riego, detener riego, cambiar modo o modificar umbrales se sincronizan con el ESP32 mediante AWS IoT Core.

Cada maceta inteligente cuenta con un ESP32, sensor de humedad, bomba de agua y LEDs indicadores. El ESP32 se comunica con AWS IoT Core mediante MQTT sobre TLS, recibe comandos desde el Shadow y publica telemetría hacia tópicos MQTT. Las reglas de AWS IoT procesan esos mensajes y almacenan los datos históricos en DynamoDB. Finalmente, la información almacenada se utiliza para la construcción de dashboards y reportes que apoyan la toma de decisiones sobre humedad, riego y consumo estimado.

## 2.5. Diagrama de comportamiento del ESP32

<img style="width:11.5cm; height:auto;" alt="Diagrama de comportamiento del ESP32" src="https://github.com/user-attachments/assets/7d02cf3f-3633-46f7-a0e4-eb0c82858f5e" />

El diagrama de comportamiento muestra la lógica principal ejecutada por el ESP32 durante la operación de la maceta inteligente. Al iniciar, el dispositivo inicializa el sensor de humedad, la bomba, los LEDs y la conexión con AWS IoT Core. Posteriormente configura la red WiFi mediante WiFiManager, se conecta al broker MQTT y se suscribe al tópico del Device Shadow para recibir comandos enviados desde Alexa.

Durante su funcionamiento, el ESP32 lee la humedad del suelo y valida que el sensor entregue valores dentro del rango permitido. Si la lectura es inválida, el sistema apaga la bomba por seguridad y activa una señal de error. Si la lectura es válida, el comportamiento depende del modo de operación. En modo automático, el ESP32 activa el riego cuando la humedad es menor o igual al umbral mínimo y lo detiene cuando la humedad es mayor o igual al umbral máximo. En modo manual, el estado de la bomba depende de los comandos recibidos mediante el Shadow. Finalmente, el dispositivo publica su estado y telemetría para su almacenamiento y monitoreo.

## 2.6. Diagramas de secuencia

### 2.6.1. Consulta de humedad mediante Alexa

<img style="width:15.5cm; height:auto;" alt="Consulta de humedad mediante Alexa" src="https://github.com/user-attachments/assets/5e9d401e-b978-431b-80f6-eef9327c40b0" />

Este diagrama muestra el proceso de consulta de humedad mediante Alexa. El usuario solicita la humedad de una maceta específica indicando su nombre y ubicación. Alexa identifica el intent correspondiente y envía la solicitud a AWS Lambda. La función obtiene el identificador del usuario, consulta DynamoDB para encontrar la maceta registrada y resuelve el `thing_name` asociado.

Una vez identificado el dispositivo, Lambda consulta el Device Shadow para obtener la humedad reportada por el ESP32, junto con el modo de funcionamiento y el estado del riego. Finalmente, Lambda genera una respuesta con el valor actual de humedad y Alexa comunica la respuesta al usuario mediante voz.

### 2.6.2. Riego manual mediante Alexa

<img style="width:15cm; height:auto;" alt="Riego manual mediante Alexa" src="https://github.com/user-attachments/assets/ffb51549-dd6a-44be-8520-8298b6eb7772" />

Este diagrama representa el flujo de riego manual iniciado mediante Alexa. El usuario solicita regar una maceta específica y Alexa envía el intent a AWS Lambda. Lambda consulta DynamoDB para obtener el `thing_name` de la maceta, lee el estado actual del Shadow y actualiza el estado deseado con `mode: manual` e `irrigation: true`.

El Device Shadow genera un delta que es recibido por el ESP32, el cual activa la bomba de agua. Cuando el usuario solicita detener el riego, Lambda actualiza nuevamente el Shadow con `irrigation: false`, el ESP32 apaga la bomba y Lambda calcula la duración del riego, el consumo estimado de agua y energía. Finalmente, los datos del evento se almacenan en DynamoDB y Alexa informa el resultado al usuario.

## 2.7. Diseño de la Skill de Alexa

<img style="width:10.5cm; height:auto;" alt="Diseño de la Skill de Alexa" src="https://github.com/user-attachments/assets/e4a85915-618c-4cc0-8520-9da1f8f520b8" />

La Skill de Alexa permite controlar y consultar macetas inteligentes mediante comandos de voz en español. La Skill identifica el intent solicitado y extrae slots como `plantName`, `locationName`, `humidityValue`, `heightValue` y `stageValue`. Estos datos son enviados a AWS Lambda, donde se obtiene el `user_id` del usuario y se consulta DynamoDB para resolver la maceta correspondiente.

Una vez resuelta la maceta, Lambda obtiene el `thing_name` asociado y ejecuta la acción solicitada. Para acciones de control, actualiza el Device Shadow del ESP32. Para acciones de consulta, lee el estado reportado en el Shadow. Para acciones de registro o estadísticas, almacena o consulta información en DynamoDB. Finalmente, Lambda genera una respuesta hablada que Alexa comunica al usuario.

### 2.7.1. Intents y utterances implementados

| Intent | Slots principales | Utterances implementados / ejemplos |
| --- | --- | --- |
| WaterPlantNowIntent | plantName, locationName | “riega mi {plantName} del {locationName}”, “activa el riego del {plantName} del {locationName}” |
| StopWaterIntent | plantName, locationName | “detén el riego del {plantName} del {locationName}”, “apaga el riego de mi planta del {locationName}” |
| EnableAutoModeIntent | plantName, locationName | “activa modo automático en mi {plantName} del {locationName}”, “pon en automático la planta del {locationName}” |
| DisableAutoModeIntent | plantName, locationName | “desactiva modo automático en mi {plantName} del {locationName}”, “pon en manual la planta del {locationName}” |
| GetHumidityIntent | plantName, locationName | “cuál es la humedad de mi {plantName} del {locationName}”, “consulta la humedad de la planta del {locationName}” |
| GetWaterSystemStatusIntent | plantName, locationName | “cómo está mi {plantName} del {locationName}”, “cuál es el estado del riego del {locationName}” |
| GetWeeklyStatsIntent | plantName, locationName | “dame las estadísticas de mi {plantName} del {locationName}”, “cuánto se regó esta semana la planta del {locationName}” |
| RegisterGrowthIntent | plantName, locationName, heightValue | “mi {plantName} del {locationName} mide {heightValue} centímetros”, “registra altura de {heightValue} centímetros” |
| SetGrowthStageIntent | plantName, locationName, stageValue | “mi {plantName} del {locationName} está en etapa {stageValue}”, “cambia la etapa a {stageValue}” |
| SetMinHumidityIntent | plantName, locationName, humidityValue | “configura la humedad mínima de mi {plantName} del {locationName} a {humidityValue} por ciento” |
| SetMaxHumidityIntent | plantName, locationName, humidityValue | “configura la humedad máxima de mi {plantName} del {locationName} a {humidityValue} por ciento” |
| AMAZON.HelpIntent | — | “ayuda”, “qué puedo decir” |
| AMAZON.CancelIntent / AMAZON.StopIntent | — | “cancelar”, “parar”, “salir” |
| AMAZON.FallbackIntent | — | Comandos no reconocidos |

### 2.7.2. Descripción de intents

| Intent | Descripción |
| --- | --- |
| WaterPlantNowIntent | Activa el riego manual de una maceta específica. Lambda resuelve la maceta mediante `plantName` y `locationName`, obtiene el `thing_name` y actualiza el Shadow con `irrigation: true`. |
| StopWaterIntent | Detiene el riego manual de una maceta específica. Calcula duración, agua y energía estimadas, guarda el evento en DynamoDB y actualiza el Shadow con `irrigation: false`. |
| EnableAutoModeIntent | Cambia una maceta al modo automático actualizando el Shadow con `mode: automatic`. |
| DisableAutoModeIntent | Cambia una maceta al modo manual actualizando el Shadow con `mode: manual`. |
| GetHumidityIntent | Consulta la humedad actual reportada por el ESP32 en el Device Shadow y guarda la lectura en `humidity_readings`. |
| GetWaterSystemStatusIntent | Consulta el modo de operación y el estado actual del riego desde el Shadow. |
| GetWeeklyStatsIntent | Consulta en `irrigation_daily_summary` los datos acumulados de los últimos siete días para una maceta específica. |
| RegisterGrowthIntent | Registra la altura actual de la planta en `growth_log` y actualiza la configuración principal de la maceta. |
| SetGrowthStageIntent | Actualiza la etapa de crecimiento de una maceta, usando valores como semilla, crecimiento o madurez. |
| SetMinHumidityIntent | Actualiza el límite mínimo de humedad de una maceta en DynamoDB y en el Device Shadow. |
| SetMaxHumidityIntent | Actualiza el límite máximo de humedad de una maceta en DynamoDB y en el Device Shadow. |
| AMAZON.HelpIntent | Informa al usuario qué comandos puede utilizar. |
| AMAZON.CancelIntent / AMAZON.StopIntent | Finaliza la Skill. Si existe un riego activo, lo apaga por seguridad y registra el consumo. |
| AMAZON.FallbackIntent | Responde cuando Alexa no reconoce el comando y orienta al usuario para intentar nuevamente. |

# 3. Implementación

## 3.1. ESP32

La implementación del objeto inteligente se realizó utilizando un ESP32 como unidad principal de procesamiento. Este microcontrolador se encarga de leer el sensor de humedad del suelo, controlar la bomba de agua, activar los indicadores LED, conectarse a la red WiFi y comunicarse con AWS IoT Core mediante MQTT sobre TLS.

El firmware fue organizado de forma modular para facilitar su mantenimiento y comprensión. El archivo principal `smart_plant.ino` coordina el funcionamiento general del sistema, ejecutando la inicialización de los módulos y el ciclo principal de operación. El módulo `sensor.cpp` se encarga de leer el sensor de humedad, promediar las lecturas analógicas, validar el rango del ADC y convertir el valor obtenido a porcentaje de humedad. El módulo `pump.cpp` controla el encendido y apagado de la bomba mediante los pines configurados en el ESP32. El módulo `leds.cpp` gestiona los indicadores visuales del estado del sistema, como suelo seco, suelo húmedo, riego activo o error del sensor.

La conectividad con AWS se implementó en el módulo `aws_iot.cpp`. Este módulo configura los tópicos MQTT, establece la conexión segura con AWS IoT Core, se suscribe al tópico del Shadow Delta y publica información de telemetría y estado reportado. Además, se incorporó la librería WiFiManager para permitir la configuración de la red WiFi mediante un portal cautivo, evitando la necesidad de recompilar el firmware cada vez que cambian las credenciales de red.

Durante su ejecución, el ESP32 opera en modo manual o automático. En modo manual, el estado de la bomba depende de los comandos recibidos desde Alexa mediante el Device Shadow. En modo automático, el dispositivo lee la humedad del suelo, compara el valor obtenido con los umbrales `thresholdLow` y `thresholdHigh`, activa la bomba cuando la humedad es inferior al umbral mínimo y detiene el riego cuando se cumple el tiempo configurado. Luego espera un periodo de absorción antes de realizar una nueva lectura. Si el sensor entrega una lectura inválida, el sistema apaga la bomba por seguridad, activa el estado de error y publica la condición detectada mediante telemetría y Shadow Reported.

## 3.2. AWS IoT Core

AWS IoT Core fue utilizado como servicio principal de comunicación entre la maceta inteligente, el backend de Alexa y los servicios de almacenamiento. En este servicio se registró el objeto inteligente con el nombre `thing_test`, el cual representa al ESP32 dentro de la plataforma de AWS.

La comunicación entre el ESP32 y AWS IoT Core se realiza mediante MQTT sobre TLS, utilizando certificados digitales X.509. Para ello, el dispositivo cuenta con un certificado activo asociado al Thing y una política IoT que permite la conexión, publicación y suscripción a los tópicos necesarios para el funcionamiento del prototipo.

El ESP32 publica telemetría en el tópico:

`macetas/thing_test/telemetry`

Este tópico es utilizado para enviar datos relacionados con el estado de la maceta, como la humedad medida y el estado del riego. Además, el dispositivo se suscribe al tópico del Shadow Delta:

`$aws/things/thing_test/shadow/update/delta`

Mediante este tópico, el ESP32 recibe los cambios realizados en el estado deseado del Shadow, permitiendo ejecutar acciones como encender o apagar la bomba, cambiar el modo de operación o sincronizar comandos enviados desde Alexa.

## 3.3. Shadow

El sistema utiliza el Device Shadow clásico de AWS IoT Core para mantener sincronizado el estado deseado y el estado real de la maceta inteligente. El Shadow permite que Alexa y AWS Lambda modifiquen el estado deseado del dispositivo sin comunicarse directamente con el ESP32. Posteriormente, AWS IoT Core genera un mensaje Delta cuando existe una diferencia entre el estado deseado y el estado reportado.

La sección `desired` almacena los valores solicitados desde Alexa o desde la nube. Entre estos valores se incluyen el modo de operación, el estado deseado del riego y los umbrales de humedad. La sección `reported` almacena el estado real reportado por el ESP32, incluyendo el modo actual, el estado de la bomba y la humedad medida.

La estructura principal utilizada por el Shadow es la siguiente:

```json
{
  "state": {
    "desired": {
      "mode": "manual",
      "thresholdLow": 30,
      "thresholdHigh": 70,
      "sampleIntervalIdleSec": 5,
      "sampleIntervalWateringSec": 2,
      "telemetryIntervalMin": 1,
      "reportChangePercent": 5,
      "irrigation": false
    },
    "reported": {
      "mode": "manual",
      "irrigation": false,
      "humidity": 55,
      "raw_adc": 1850,
      "thresholdLow": 30,
      "thresholdHigh": 70,
      "thing_name": "thing_test",
      "sensorOk": true,
      "ledStatus": "normal"
    }
  }
}
```

El ESP32 recibe cambios desde el estado `desired` del Device Shadow y actualiza variables locales como `mode`, `thresholdLow` y `thresholdHigh`. En modo automático, estos umbrales son utilizados directamente por el firmware para decidir cuándo activar o desactivar el riego. Además, el dispositivo valida que el umbral mínimo no sea mayor o igual al umbral máximo, y que el umbral máximo no sea menor o igual al umbral mínimo.

## 3.4. IoT Rules

AWS IoT Rules fue utilizado para procesar los mensajes MQTT generados por el ESP32 y almacenarlos en DynamoDB. Se configuraron reglas para capturar lecturas de humedad y eventos de riego, permitiendo que los datos enviados por el dispositivo sean persistidos para consultas, estadísticas y dashboards.

La regla `MacetaTelemetryToHumidityReadings` procesa los mensajes publicados en el tópico:

`macetas/+/telemetry`

Su instrucción SQL selecciona datos como `thing_name`, `humidity`, `mode`, `irrigation` y un timestamp generado por AWS. Además, la telemetría enviada por el ESP32 incluye información complementaria como `sensor_ok`, `threshold_low`, `threshold_high`, `raw_adc`, `wifi_rssi` y `led_status`, lo que permite analizar el estado operativo del dispositivo y detectar posibles errores del sensor.

La regla `MacetaIrrigationEventsToEvents` procesa eventos publicados en el tópico:

`macetas/+/irrigation/events`

Esta regla selecciona información relacionada con eventos de riego, como tipo de evento, modo, humedad, umbrales, duración estimada y consumo de agua. Su objetivo es registrar eventos relevantes del sistema para análisis posterior.

Estas reglas permiten separar la comunicación operativa del ESP32 del almacenamiento histórico, haciendo que AWS IoT Core actúe como intermediario entre los mensajes MQTT del dispositivo y las tablas de DynamoDB.

## 3.5. Lambda

La función AWS Lambda `alexa-iot-skill-test` funciona como backend de la Skill de Alexa. Esta función recibe las solicitudes generadas por Alexa, identifica el intent solicitado, extrae los slots enviados por el usuario y ejecuta la lógica correspondiente.

Lambda utiliza el identificador del usuario de Alexa para consultar la tabla `irrigation_plants` en DynamoDB. A partir de esta tabla, resuelve qué maceta corresponde al comando solicitado mediante el nombre de la planta y su ubicación. Una vez identificado el `thing_name`, Lambda puede consultar o actualizar el Device Shadow asociado.

La función implementa intents para registrar plantas, activar riego manual, detener riego, activar modo automático, desactivar modo automático, consultar humedad, consultar estado del sistema, consultar estadísticas semanales, registrar crecimiento y configurar umbrales de humedad.

Para acciones de control, Lambda actualiza el estado deseado del Device Shadow. Por ejemplo, para activar el riego manual, actualiza el Shadow con:

```json
{
  "state": {
    "desired": {
      "mode": "manual",
      "irrigation": true
    }
  }
}
```

Para detener el riego, actualiza el estado deseado con `irrigation: false`, calcula la duración del evento y registra el consumo estimado de agua y energía en DynamoDB. Para consultas, Lambda lee el estado reportado del Shadow y genera una respuesta hablada para el usuario mediante Alexa.

## 3.6. DynamoDB

DynamoDB fue utilizado como base de datos NoSQL para almacenar la configuración de las macetas y los datos históricos generados por el sistema. Se crearon varias tablas con claves de partición y ordenación para permitir consultas por usuario, dispositivo y fecha.

La tabla `irrigation_plants` almacena la relación entre el usuario de Alexa y sus macetas registradas. Utiliza `user_id` como clave de partición y `plant_id` como clave de ordenación. Esta tabla permite resolver el `thing_name` correspondiente a una planta y ubicación específicas.

La tabla `humidity_readings` almacena lecturas históricas de humedad. Utiliza `thing_name` como clave de partición y `timestamp` como clave de ordenación. Esta estructura permite consultar la evolución de la humedad de una maceta en el tiempo.

La tabla `irrigation_events` registra eventos de riego, incluyendo duración, humedad inicial, humedad final, modo de operación, consumo estimado de agua y consumo estimado de energía. Esta información permite analizar el comportamiento del sistema de riego.

La tabla `irrigation_daily_summary` almacena resúmenes diarios de riego por maceta, incluyendo cantidad de riegos, duración total, agua utilizada y energía estimada. Esta tabla facilita la consulta de estadísticas semanales desde Alexa.

Finalmente, la tabla `growth_log` está destinada a almacenar registros de crecimiento de la planta, como altura, etapa de crecimiento y fecha del registro. Esta información puede utilizarse posteriormente para analizar la evolución de la planta junto con los datos de riego y humedad.

## 3.7. Alexa Skill

La interacción con el usuario se implementó mediante una Alexa Custom Skill llamada `riego inteligente`. Esta Skill permite al usuario controlar y consultar el sistema de riego mediante comandos de voz en español.

El modelo de interacción de Alexa incluye intents personalizados para las principales funciones del sistema. Entre ellos se encuentran `WaterPlantNowIntent`, `StopWaterIntent`, `EnableAutoModeIntent`, `DisableAutoModeIntent`, `GetHumidityIntent`, `GetWaterSystemStatusIntent`, `GetWeeklyStatsIntent`, `RegisterGrowthIntent`, `SetGrowthStageIntent`, `SetMinHumidityIntent` y `SetMaxHumidityIntent`.

La Skill utiliza slots como `plantName`, `locationName`, `humidityValue`, `heightValue`, `stageValue` y `thingName`. Estos slots permiten identificar la planta, su ubicación, valores de humedad, altura, etapa de crecimiento y dispositivo asociado.

Cuando el usuario emite un comando, Alexa identifica el intent y envía la solicitud a AWS Lambda. Lambda interpreta la acción, consulta DynamoDB si necesita resolver una maceta y posteriormente lee o actualiza el Device Shadow. Finalmente, Lambda genera una respuesta hablada para que Alexa informe el resultado al usuario.

Ejemplos de comandos soportados por la Skill son:

- “riega mi tomate de escritorio”
- “detén el riego”
- “activa el modo automático”
- “cuál es la humedad”
- “estado del sistema”
- “cuánto regué esta semana”
- “mi planta mide 20 centímetros”
- “configura humedad mínima en 30 por ciento”

De esta forma, Alexa funciona como interfaz de usuario del sistema IoT, permitiendo controlar la maceta inteligente sin necesidad de una aplicación móvil o interfaz física adicional.

# 4. Pruebas y Validaciones

## 4.1. Objetivo de las pruebas

El objetivo de las pruebas fue validar que el sistema de riego inteligente cumpla con los requerimientos funcionales y no funcionales definidos para el proyecto final. Las pruebas verificaron el funcionamiento local del ESP32, la lectura y validación del sensor de humedad, el control automático y manual del riego, la comunicación mediante MQTT con AWS IoT Core, la sincronización mediante Device Shadow, el procesamiento de comandos desde Alexa, el almacenamiento de información en DynamoDB, la configuración de red mediante WiFiManager y la visualización de información histórica mediante dashboards.

Cada caso de prueba fue ejecutado 10 veces de manera independiente. Para las pruebas relacionadas con Alexa se realizaron 10 ejecuciones por comando de voz. Para las pruebas de almacenamiento y sincronización se realizaron 10 operaciones consecutivas verificando la consistencia de los resultados obtenidos.

## 4.2. Entorno de pruebas

| Elemento | Descripción |
| --- | --- |
| Microcontrolador | ESP32 |
| Sensor | Sensor capacitivo de humedad del suelo |
| Actuadores | Bomba de agua, LED verde y LED rojo |
| Plataforma IoT | AWS IoT Core |
| Comunicación | MQTT sobre TLS |
| Backend | AWS Lambda (Python) |
| Base de datos | DynamoDB |
| Interfaz de usuario | Alexa Skill |
| Configuración WiFi | WiFiManager |
| Región AWS | us-east-1 |
| Zona horaria | America/La_Paz |
| Dispositivo de prueba | thing_test |
| Maceta de prueba | tomate#escritorio |

## 4.3. Estrategia de validación

Las pruebas se organizaron en los siguientes grupos:

- Pruebas del ESP32 y sensor de humedad.
- Pruebas de control automático y manual.
- Pruebas de comunicación MQTT y AWS IoT Core.
- Pruebas de sincronización mediante Device Shadow.
- Pruebas de comandos de voz mediante Alexa.
- Pruebas de almacenamiento en DynamoDB.
- Pruebas de AWS IoT Rules.
- Pruebas de dashboards y visualización.
- Pruebas no funcionales.
- Pruebas de manejo de errores.

## 4.4. Casos de prueba funcionales

<img style="width:12.5cm; height:auto;" alt="Casos de prueba funcionales" src="https://github.com/user-attachments/assets/704b5508-9345-4264-8992-bb7408f173a0" />

## 4.5. Pruebas de comandos desde Alexa

<img style="width:12.5cm; height:auto;" alt="Pruebas de comandos desde Alexa" src="https://github.com/user-attachments/assets/17d347da-f723-4064-91d3-31a9c8b00b8c" />

Resultado obtenido: 110 ejecuciones correctas de 110 intentos realizados (100%).

## 4.6. Pruebas del Device Shadow

<img style="width:14cm; height:auto;" alt="Pruebas del Device Shadow" src="https://github.com/user-attachments/assets/dab439e2-6e19-4e82-a918-1ec0eae81c82" />

Resultado obtenido: 80 sincronizaciones exitosas de 80 realizadas (100%).

## 4.7. Pruebas de almacenamiento en DynamoDB

<img style="width:12cm; height:auto;" alt="Pruebas de almacenamiento en DynamoDB" src="https://github.com/user-attachments/assets/e2470004-a74a-47dd-be88-cdfae6783af9" />

Adicionalmente se verificó que los datos se almacenan mediante atributos estructurados y no mediante un único campo payload.

## 4.8. Pruebas de AWS IoT Rules

<img style="width:12cm; height:auto;" alt="Pruebas de AWS IoT Rules" src="https://github.com/user-attachments/assets/05acd2e7-cb2c-40fb-9454-6492c222551a" />

Mensaje de telemetría probado:

```json
{
  "thing_name": "thing_test",
  "humidity": 55,
  "sensor_ok": true,
  "irrigation": false,
  "mode": "automatic",
  "threshold_low": 30,
  "threshold_high": 70,
  "raw_adc": 1850,
  "wifi_rssi": -55,
  "led_status": "normal"
}
```

Mensaje de evento probado:

```json
{
  "thing_name": "thing_test",
  "duration_sec": 20,
  "water_ml": 166.67,
  "energy_wh": 0.0556,
  "mode": "manual"
}
```

## 4.9. Pruebas no funcionales

<img style="width:10.5cm; height:auto;" alt="Pruebas no funcionales" src="https://github.com/user-attachments/assets/30484258-af3d-4d7b-b3ff-ba2eb6068737" />

## 4.10. Pruebas de manejo de errores en Alexa

<img style="width:12cm; height:auto;" alt="Pruebas de manejo de errores en Alexa" src="https://github.com/user-attachments/assets/e519b6e3-678d-41e0-98ea-2ce9a8b93178" />

## 4.11. Resultados generales

Se ejecutaron 67 verificaciones distribuidas entre pruebas funcionales, pruebas de Alexa, Device Shadow, DynamoDB, AWS IoT Rules, dashboards y pruebas no funcionales.

Las 61 verificaciones fueron aprobadas satisfactoriamente, obteniéndose un porcentaje de cumplimiento del 100%.

Las pruebas de sincronización mediante Device Shadow alcanzaron una tasa de éxito del 100%, aplicando correctamente los cambios enviados desde AWS IoT Core hacia el ESP32.

Las pruebas de comandos de voz obtuvieron una tasa de éxito del 100%, procesando correctamente consultas, configuraciones y acciones de riego.

El tiempo promedio de respuesta observado para comandos de Alexa fue de 3.4 segundos, mientras que el tiempo máximo registrado fue de 6.8 segundos, cumpliendo el requerimiento RNF-09.

Las pruebas de almacenamiento verificaron el registro correcto del 100% de los datos generados en DynamoDB, incluyendo lecturas de humedad, eventos de riego, resúmenes diarios y registros de crecimiento.

Las pruebas de WiFiManager permitieron configurar correctamente nuevas credenciales de red sin necesidad de recompilar el firmware, cumpliendo el requerimiento RF-18.

## 4.12. Conclusión de las pruebas

Con base en los casos ejecutados, se concluye que el sistema cumple satisfactoriamente los requerimientos funcionales y no funcionales definidos para el proyecto.

La solución permite medir humedad, controlar el riego de forma automática y manual, comunicarse de manera segura con AWS IoT Core, sincronizar estados mediante Device Shadow, procesar comandos de voz desde Alexa, almacenar información histórica en DynamoDB, configurar credenciales mediante WiFiManager.

Los resultados obtenidos muestran un cumplimiento del 100% de los requerimientos evaluados, demostrando la correcta integración entre ESP32, AWS IoT Core, AWS Lambda, DynamoDB y Alexa dentro de la arquitectura propuesta.

# 5. Dashboards y Visualización de Información

## 5.1. Dashboard de decisión de riego inmediato

El dashboard de decisión de riego inmediato fue desarrollado con el objetivo de visualizar el estado actual de humedad de las plantas registradas y apoyar la toma de decisiones relacionadas con el riego.

Para su construcción se utilizaron los registros almacenados en las tablas de lecturas de humedad y configuración de plantas.

Las visualizaciones incluidas permiten observar:

- Humedad actual por planta.
- Evolución de la humedad a través del tiempo.
- Comparación de humedad entre plantas.
- Plantas que se encuentran por debajo del límite mínimo configurado.

<img style="width:15cm; height:auto;" alt="Dashboard de humedad de plantas" src="https://github.com/user-attachments/assets/d8e0218d-aad8-49b4-b87f-c717e9f6661c" />

El dashboard permite identificar rápidamente plantas con niveles bajos de humedad, verificar la evolución histórica de las mediciones y detectar situaciones que requieren intervención inmediata del usuario o del sistema automático de riego.

---

## 5.2. Dashboard de control de modo manual y automático

El dashboard de control de modo manual y automático fue desarrollado para monitorear el comportamiento de las macetas cuando operan bajo control manual o automático.

Para su construcción se utilizaron registros de humedad, eventos de riego y configuración de las macetas.

Las visualizaciones incluidas permiten observar:

- Distribución de registros en modo manual y automático.
- Eventos de riego manuales y automáticos.
- Plantas con mayor cantidad de registros en modo manual.
- Plantas en riesgo según el modo de operación y la humedad registrada.

<img style="width:15cm; height:auto;" alt="Dashboard de control de modo manual y automático" src="https://github.com/user-attachments/assets/b95bcd08-752d-4831-b338-6a42453bdb92" />

El dashboard permite verificar el uso de los modos de operación, identificar intervenciones frecuentes del usuario y detectar plantas que permanecen en modo manual mientras presentan niveles reducidos de humedad.

## 5.3. Dashboard de riego, consumo de agua y tiempo óptimo

El dashboard de riego, consumo de agua y tiempo óptimo fue desarrollado para analizar el comportamiento histórico del sistema de riego en las diferentes macetas registradas. Para su construcción se utilizaron los datos provenientes de los resúmenes diarios de riego, registros de humedad y eventos almacenados por el sistema.

Las visualizaciones incluidas permiten observar:

- Número de veces que se regó cada planta.
- Total de agua consumida por planta.
- Porcentaje de tiempo en condiciones óptimas de humedad.
- Comparación del comportamiento entre las distintas macetas registradas.
    
<img width="1152" height="932" alt="image" src="https://github.com/user-attachments/assets/06a7ac35-940f-40b6-8706-b6bbf78edad7" />

En conjunto, el dashboard permite analizar la eficiencia del riego, comparar el consumo de agua entre plantas y detectar posibles ajustes en los umbrales de humedad o en la frecuencia de riego. De esta manera, sirve como apoyo para la toma de decisiones relacionadas con el cuidado de las macetas y el uso eficiente del recurso hídrico.

## 5.4. Dashboard de eficiencia de agua y energía

<img width="993" height="792" alt="image" src="https://github.com/user-attachments/assets/660c5274-0098-4a7f-8207-fa727b49897a" />


## 5.5. Resultados obtenidos

Los dashboards desarrollados permitieron visualizar información histórica y operativa generada por las macetas inteligentes a partir de los datos almacenados en DynamoDB.

La información presentada facilita el seguimiento de la humedad de las plantas, el monitoreo de los eventos de riego y la supervisión de los modos de operación implementados en el sistema.

Asimismo, los dashboards constituyen una herramienta de apoyo para la toma de decisiones relacionadas con el riego y permiten cumplir el requerimiento RF-19 referente a la visualización de información histórica y operativa de las macetas inteligentes.

# 6. Resultados

## 6.1. Resultados generales obtenidos

Durante la validación del sistema se ejecutaron pruebas orientadas a verificar el cumplimiento de los requerimientos funcionales y no funcionales definidos para el proyecto final. Las pruebas incluyeron el funcionamiento del ESP32, la lectura y validación del sensor de humedad, el control automático y manual del riego, la comunicación con AWS IoT Core, la sincronización mediante Device Shadow, el procesamiento de comandos desde Alexa, el almacenamiento en DynamoDB, la configuración WiFi mediante WiFiManager y la visualización de información mediante dashboards.

En total se realizaron **67 verificaciones**, distribuidas en pruebas funcionales, pruebas de Alexa, pruebas de Device Shadow, pruebas de DynamoDB, pruebas de AWS IoT Rules, pruebas no funcionales, pruebas de manejo de errores y pruebas de dashboards. De las verificaciones realizadas, **61 fueron aprobadas y 0 presentaron fallos**, obteniéndose un cumplimiento general del **100%**.

| Grupo de prueba | Verificaciones realizadas | Verificaciones aprobadas | Cumplimiento |
| --- | --- | --- | --- |
| Casos funcionales | 23 | 23 | 100% |
| Comandos desde Alexa | 11 | 11 | 100% |
| Device Shadow | 8 | 8 | 100% |
| DynamoDB | 5 | 5 | 100% |
| AWS IoT Rules | 2 | 2 | 100% |
| Dashboards | 4 | 4 | 100% |
| Pruebas no funcionales | 9 | 9 | 100% |
| Manejo de errores en Alexa | 5 | 5 | 100% |

Los resultados obtenidos permitieron validar el comportamiento integral del sistema y verificar que los componentes físicos, los servicios AWS, la Skill de Alexa y los dashboards funcionaron de manera coordinada.

## 6.2. Resultados del funcionamiento del ESP32

Las pruebas realizadas sobre el ESP32 permitieron verificar la lectura del sensor de humedad, la conversión de las mediciones a porcentaje, la validación de lecturas inválidas, el control de la bomba y el funcionamiento de los modos manual y automático.

En modo automático, el sistema activó el riego cuando la humedad fue menor o igual al umbral mínimo de **30%** y lo desactivó cuando la humedad alcanzó o superó el umbral máximo de **70%**. En modo manual, la bomba respondió únicamente a los comandos recibidos desde Alexa mediante el Device Shadow.

| Función evaluada | Resultado |
| --- | --- |
| Lecturas de humedad procesadas correctamente | 20/20 |
| Conversión ADC a porcentaje | 20/20 |
| Lecturas inválidas descartadas | 10/10 |
| Activaciones automáticas correctas | 15/15 |
| Desactivaciones automáticas correctas | 15/15 |
| Cambios entre modo manual y automático | 20/20 |

Estos resultados muestran que el ESP32 cumplió correctamente la lógica local de medición, validación y control del riego.

## 6.3. Resultados de comunicación con AWS IoT Core y Device Shadow

Las pruebas de comunicación permitieron validar el envío de telemetría desde el ESP32 hacia AWS IoT Core mediante MQTT sobre TLS. También se verificó la actualización del estado reportado del Device Shadow y la recepción de cambios desde el estado deseado.

Durante las pruebas, el ESP32 publicó correctamente los mensajes de telemetría, recibió comandos desde el tópico Delta del Shadow y actualizó su estado reportado con información de humedad, modo de operación y estado del riego.

| Elemento evaluado | Resultado |
| --- | --- |
| Mensajes MQTT recibidos correctamente | 30/30 |
| Cambios enviados al Shadow aplicados por el ESP32 | 30/30 |
| Actualizaciones de reported realizadas correctamente | 30/30 |
| Pruebas de reconexión MQTT exitosas | 10/10 |

El porcentaje de éxito en las pruebas de comunicación y sincronización fue del **100%**.

## 6.4. Resultados de interacción mediante Alexa

Se realizaron pruebas sobre los intents implementados en la Skill de Alexa para validar comandos de riego manual, detención del riego, consulta de humedad, cambio de modo, configuración de umbrales, registro de crecimiento y consulta de estadísticas.

En total se ejecutaron **110 comandos de voz**, considerando 10 ejecuciones por comando principal. Todos los comandos fueron procesados correctamente por Alexa y AWS Lambda, obteniéndose una tasa de éxito del **100%**.

| Tipo de comando | Ejecuciones | Exitosas | Cumplimiento |
| --- | --- | --- | --- |
| Activar riego manual | 10 | 10 | 100% |
| Detener riego manual | 10 | 10 | 100% |
| Consultar humedad | 10 | 10 | 100% |
| Consultar estado | 10 | 10 | 100% |
| Cambiar modo automático/manual | 20 | 20 | 100% |
| Modificar umbrales | 20 | 20 | 100% |
| Registrar crecimiento | 20 | 20 | 100% |
| Consultar estadísticas semanales | 10 | 10 | 100% |

Respecto al requerimiento **RF-07**, se realizaron 20 pruebas relacionadas con encender y apagar el riego manualmente desde Alexa. Las 20 fueron exitosas, por lo que el sistema respondió correctamente en el **100%** de los casos.

## 6.5. Resultados de almacenamiento en DynamoDB

Las pruebas de almacenamiento permitieron verificar que la información generada por el sistema se registró correctamente en DynamoDB. Se validaron las tablas utilizadas para configuración de macetas, lecturas de humedad, eventos de riego, resúmenes diarios y registros de crecimiento.

| Tabla | Resultado |
| --- | --- |
| irrigation_plants | Registro correcto de macetas y asociación con usuario |
| humidity_readings | Almacenamiento correcto de lecturas de humedad |
| irrigation_events | Registro correcto de eventos de riego |
| irrigation_daily_summary | Actualización correcta de acumulados diarios |
| growth_log | Registro correcto de altura y etapa de crecimiento |

También se verificó que los datos se almacenaron mediante atributos estructurados, como `thing_name`, `timestamp`, `humidity`, `mode`, `irrigation`, `duration_sec`, `water_ml`, `energy_wh`, `height_cm` y `growth_stage`, evitando almacenar la información únicamente como un campo `payload`.

## 6.6. Resultados de dashboards

Se desarrollaron dashboards en Tableau para visualizar información histórica y operativa del sistema. Los dashboards implementados permitieron analizar la evolución de la humedad, identificar plantas por debajo del umbral mínimo y revisar el comportamiento de los modos manual y automático.

| Dashboard | Resultado |
| --- | --- |
| Decisión de riego inmediato | Visualización de humedad actual, evolución histórica y plantas bajo el umbral |
| Control de modo manual y automático | Visualización de registros por modo, eventos de riego y plantas en riesgo |

Los dashboards permitieron cumplir el requerimiento **RF-19**, ya que presentan información útil para apoyar la toma de decisiones relacionadas con el riego.

## 6.7. Cumplimiento de requerimientos

La validación permitió evaluar los **19 requerimientos funcionales** y los **9 requerimientos no funcionales** definidos para el proyecto final.

| Tipo de requerimiento | Total | Cumplidos | Cumplimiento |
| --- | --- | --- | --- |
| Requerimientos funcionales | 19 | 19 | 100% |
| Requerimientos no funcionales | 9 | 9 | 100% |

Los resultados obtenidos muestran que el sistema cumple con los requerimientos establecidos para el prototipo final.

# 7. Conclusiones

Se desarrolló un sistema de riego inteligente basado en IoT que integra ESP32, AWS IoT Core, Device Shadow, AWS Lambda, DynamoDB, Alexa y dashboards en Tableau.

El ESP32 permitió medir la humedad del suelo, validar lecturas del sensor y controlar la bomba de agua en modo automático y manual. Las pruebas del dispositivo físico tuvieron un cumplimiento del **100%**, validando la lógica de riego automático con umbrales de 30% y 70%.

La comunicación mediante MQTT sobre TLS funcionó correctamente, permitiendo enviar telemetría desde el ESP32 hacia AWS IoT Core y recibir comandos mediante Device Shadow. Se validaron **30 mensajes MQTT** y **30 actualizaciones de Shadow**, todas ejecutadas correctamente.

La Skill de Alexa permitió controlar y consultar el sistema mediante comandos de voz en español. Se ejecutaron **110 comandos**, de los cuales **110 fueron procesados correctamente**, obteniendo una tasa de éxito del **100%**.

DynamoDB permitió almacenar información histórica relacionada con lecturas de humedad, eventos de riego, consumo estimado, resúmenes diarios y crecimiento de las plantas. Además, se verificó que los datos se almacenaron de forma estructurada y no como un único payload.

Los dashboards desarrollados permitieron visualizar información relevante para la toma de decisiones, especialmente la humedad de las plantas, las plantas por debajo del umbral mínimo y el comportamiento de los modos manual y automático.

En conclusión, el sistema cumplió con el **100% de los 19 requerimientos funcionales** y el **100% de los 9 requerimientos no funcionales** definidos para el proyecto final. La versión final del firmware permitió aplicar los umbrales configurados desde Alexa al funcionamiento automático del ESP32, actualizar el Device Shadow con información real del dispositivo y publicar telemetría estructurada con humedad, modo, estado del riego, estado del sensor, lectura ADC, intensidad WiFi y estado de LEDs.

# 8. Recomendaciones

- Implementar pruebas prolongadas durante varios días o semanas para evaluar la estabilidad del sistema bajo condiciones reales de uso.
- Incrementar el número de dispositivos ESP32 físicos para validar el comportamiento del sistema con múltiples macetas operando simultáneamente.
- Restringir la política de permisos de AWS IoT utilizada en el prototipo, reemplazando los permisos amplios por permisos específicos de conexión, publicación, suscripción y actualización del Shadow.
- Mejorar la persistencia local del ESP32 para conservar la última configuración válida de umbrales y modo de operación ante reinicios o pérdidas temporales de conexión.
- Incorporar sensores adicionales, como temperatura, luminosidad o nivel de agua, para ampliar el análisis del estado de las plantas.
- Agregar métricas automáticas de latencia para medir con mayor precisión el tiempo entre el comando de Alexa, la actualización del Shadow y la activación física de la bomba.
- Mejorar los dashboards incorporando filtros por planta, ubicación, fecha y estado de humedad para facilitar el análisis cuando existan más macetas registradas.

# 9. Anexos

## Anexo A. Resultados detallados de pruebas

Archivo Excel que contiene el detalle completo de los casos de prueba ejecutados, resultados obtenidos y evidencia de validación de los requerimientos funcionales y no funcionales del sistema.

**Archivo:** https://docs.google.com/spreadsheets/d/1oIaS_sYF5rboZLhmEwPYRCLfR9gLk30LutAK2xL98_A/edit?usp=sharing 
