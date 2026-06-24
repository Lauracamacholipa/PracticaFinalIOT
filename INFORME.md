# Proyecto final — Prototipo de Sistema IoT con AWS y Alexa

**Carrera:** Ingeniería de Sistemas

**Docente**: Eduardo Enrique Marin Garcia

**Asignatura:** SIS-234 - Internet de las Cosas

**Integrantes**: 

- Laura Camacho Lipa
- Sergio Francisco Solis Luizaga
- Cristhian Butron

---

# 1. Requerimientos del Sistema

## 1.1. Requerimientos funcionales

**RF-01:** Cada maceta inteligente deberá medir la humedad del suelo utilizando el sensor conectado a su ESP32 en los siguientes intervalos:

- Para la configuración inicial del prototipo se utilizarán intervalos de 5 segundos cuando el sistema no se encuentre regando.
- cada 2 segundos cuando el sistema se encuentre regando.

**RF-02:** Cada maceta inteligente debe convertir las lecturas obtenidas del sensor de humedad a un valor porcentual comprendido entre **0% y 100%**.

**RF-03:** Cada maceta inteligente deberá validar que las lecturas crudas del sensor se encuentren dentro del rango permitido del ADC; cuando una lectura sea inválida, deberá descartarla y mantener el último estado válido del riego.

**RF-04:** En modo automático, cada maceta inteligente debe activar el riego cuando la humedad sea menor o igual al límite mínimo configurado para dicha maceta. Para la configuración inicial del prototipo, el límite mínimo será de **30%**.

**RF-05:** En modo automático, cada maceta inteligente debe desactivar el riego cuando la humedad sea mayor o igual al límite máximo configurado para dicha maceta. Para la configuración inicial del prototipo, el límite máximo será de **70%**.

**RF-06:** Cada maceta inteligente debe operar en dos modos de funcionamiento: **automático** y **manual**.

**RF-07:** En modo manual, el sistema debe permitir encender y apagar el riego de una maceta específica mediante comandos de voz enviados desde Alexa, sin que las lecturas del sensor modifiquen automáticamente la acción solicitada.

**RF-08:** El sistema debe permitir al usuario modificar mediante comandos de voz desde Alexa los límites mínimo y máximo de humedad de una maceta específica, almacenando dichos valores en AWS IoT Core para su posterior utilización por el sistema.

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

Durante su ejecución, el ESP32 lee periódicamente la humedad del suelo. Si el sensor entrega una lectura inválida, el sistema activa el modo de error y apaga la bomba por seguridad. Si la lectura es válida, el comportamiento depende del modo de funcionamiento. En modo automático, el ESP32 activa el riego cuando la humedad es baja y lo detiene cuando la humedad supera el límite máximo configurado. En modo manual, el estado de la bomba depende de los comandos recibidos desde AWS IoT Core mediante el Device Shadow.

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
      "humidity": -1,
      "thing_name": "thing_test",
      "sensorOk": true,
      "ledStatus": "irrigating"
    }
  }
}
```

Cuando el usuario solicita una acción desde Alexa, Lambda actualiza la sección `desired` del Shadow. Si el ESP32 aún no ha reportado ese mismo estado, AWS IoT Core genera un Delta. El ESP32 recibe este Delta mediante MQTT, interpreta los campos recibidos y aplica la acción correspondiente sobre la bomba o el modo de operación. Posteriormente, el dispositivo publica un nuevo estado `reported` para reflejar su estado real.

## 3.4. IoT Rules

AWS IoT Rules fue utilizado para procesar los mensajes MQTT generados por el ESP32 y almacenarlos en DynamoDB. Se configuraron reglas para capturar lecturas de humedad y eventos de riego, permitiendo que los datos enviados por el dispositivo sean persistidos para consultas, estadísticas y dashboards.

La regla `MacetaTelemetryToHumidityReadings` procesa los mensajes publicados en el tópico:

`macetas/+/telemetry`

Su instrucción SQL selecciona datos como `thing_name`, `humidity`, `growth_stage`, `mode`, `irrigation` y un timestamp generado por AWS. Esta regla permite almacenar lecturas de humedad válidas en la tabla `humidity_readings`.

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
