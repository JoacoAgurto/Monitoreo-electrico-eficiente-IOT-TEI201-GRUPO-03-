## Librerías Utilizadas

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
