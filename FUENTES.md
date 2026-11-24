# Fuentes

## Librerías de Software Utilizadas

| Librería | Propósito en el proyecto |
|-----------|-------------------------|
| `Arduino.h` | Proporciona las funciones básicas del entorno Arduino, como configuración de pines, temporización y comunicación serial. |
| `Wire.h` | Implementa la comunicación I2C utilizada para intercambiar datos entre la ESP32 y el módulo ADS1115. |
| `SPI.h` | Permite la comunicación mediante el protocolo SPI, necesaria para la interacción con ciertos periféricos y el módulo de almacenamiento SD. |
| `SD.h` | Gestiona la lectura y escritura de datos en una tarjeta microSD para almacenamiento local de registros de consumo energético. |
| `WiFi.h` | Permite conectar la ESP32 a una red inalámbrica para transmitir datos de monitoreo hacia la plataforma web. |
| `WiFiClientSecure.h` | Habilita conexiones seguras mediante TLS/SSL para proteger la transmisión de datos a servicios web. |
| `HTTPClient.h` | Permite realizar solicitudes HTTP/HTTPS para enviar o consultar información en la base de datos remota. |
| `WebServer.h` | Implementa un servidor web local en la ESP32 para exponer información o configuraciones a través de una interfaz web. |
| `Preferences.h` | Permite almacenar configuraciones persistentes en la memoria no volátil de la ESP32, como credenciales Wi-Fi o parámetros del sistema. |
| `Adafruit_ADS1X15.h` | Controla el convertidor analógico-digital ADS1115, encargado de digitalizar la señal proveniente del sensor de corriente. |
| `time.h` | Gestiona funciones relacionadas con fecha y hora, utilizadas para marcas temporales de mediciones y registros. |
| `math.h` | Proporciona funciones matemáticas avanzadas utilizadas en cálculos de corriente RMS, potencia, energía y estimación de costos. |

## Modelos 3D de terceros

El archivo `Modelado Encapsulado.zip` incluye modelos de componentes descargados de terceros para:
| Componente | Enlace modelo | Autor |
|---|---|---|
|TF Micro SD Card Memory Modul Arduino  |https://grabcad.com/library/tf-micro-sd-card-memory-modul-arduino-1  |Hans Meyer |
|ADS1115  |https://grabcad.com/library/16bit-12c-adc-pga-ads11115-1  |TheElectronicsArchive |
|ESP32 wroom 32  |https://grabcad.com/library/esp32-wroom-32-5  |Siddhant Gatkine |
|PCB PERFORADA 7*9CM  |https://grabcad.com/library/pcb-perforada-7-9cm-baquelita-1  |Diego Rivero |
|TRRS Jack Breakout  |https://grabcad.com/library/trrs-jack-breakout-sparkfun-1  |eddyosp |
|TF Micro SD Card Memory Modul  |https://grabcad.com/library/tf-micro-sd-card-memory-modul-arduino-1  |Hans Meyer |
|Conector PinSocket   |https://grabcad.com/library/female-pitch-connector-2-54-mm-x8-x15-x19-pins-1  |Andrei Golyakov |
|Resistencia 1K y 10K  |https://grabcad.com/library/resistor-10k-1k-470-330-ohms-pitch-10mm-1  |Daniel Haro |
|Diodo Led Color Rojo  |https://grabcad.com/library/diodo-led-color-rojo-1  |Ad Astra |

## Uso de Inteligencia Artificial

Fue utilizada la inteligencia Artificial Chat GPT al momento del desarrollo del modelo 3D como fuente de información respecto a cómo utilizar la herramienta tanto Fusion 360 como Kikad, además de brindar apoyo al momento de la búsqueda de componentes.
Adicionalmente se le pidieron instrucciones para ir generando el código a base de lo que se necesitaba y orientándolo en todo el proceso.
