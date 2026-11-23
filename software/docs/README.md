# Documentación de Software – Monitor de Corriente ESP32

## Diagramas de Flujo

Se incluye un diagrama de flujo en formato PNG que representa el funcionamiento general del sistema y sus procesos principales.

## Documentación de software

### Funciones principales

**`iniciarWiFi()`**
- **Propósito:** Crear el punto de acceso del ESP32 e intentar conectarse a una red Wi-Fi externa.
- **Parámetros:** Ninguno.
- **Retorna:** `void`.

**`iniciarADC()`**
- **Propósito:** Inicializar los dos conversores ADS1115 y configurar sus parámetros de lectura.
- **Parámetros:** Ninguno.
- **Retorna:** `void`.

**`medirCorriente(ads, canal)`**
- **Propósito:** Obtener muestras de un canal, calcular el valor RMS y convertirlo a corriente.
- **Parámetros:** Conversor ADS1115 y canal diferencial seleccionado.
- **Retorna:** `float`, corriente en amperios.

**`actualizarCorrientes()`**
- **Propósito:** Actualizar las mediciones de los cuatro conectores y aplicar sus factores de calibración.
- **Parámetros:** Ninguno.
- **Retorna:** `void`.

**`iniciarSD()`**
- **Propósito:** Inicializar la tarjeta microSD y preparar los archivos CSV.
- **Parámetros:** Ninguno.
- **Retorna:** `void`.

**`registrarMedicionLocal()`**
- **Propósito:** Guardar las mediciones y sus fechas en el archivo CSV.
- **Parámetros:** Ninguno.
- **Retorna:** `void`.

**`enviarJSON(json)`**
- **Propósito:** Enviar las mediciones a la API mediante una solicitud HTTPS.
- **Parámetros:** Cadena con los datos en formato JSON.
- **Retorna:** `bool`, indica si el envío fue exitoso.

## Instalación

1. Instalar Arduino IDE.
2. Instalar el soporte para placas ESP32.
3. Instalar la biblioteca Adafruit ADS1X15.
4. Abrir `main.ino` y comprobar que `pagina_web.h` esté en la misma carpeta.
5. Seleccionar la placa ESP32 Dev Module y el puerto correspondiente.
6. Conectar el ESP32 al computador y cargar el programa.
7. Conectar el dispositivo a la alimentación.

## Operación

### Encendido del Sistema

1. Alimentar el ESP32.
2. Esperar la inicialización de los conversores, la tarjeta microSD y las conexiones Wi-Fi.
3. Conectarse a la red inalámbrica `Monitor-ESP32`.

### Operación Normal

1. Abrir un navegador e ingresar a `http://192.168.4.1`.
2. Consultar las mediciones de los cuatro conectores.
3. Revisar los gráficos de corriente.
4. Configurar la conexión Wi-Fi externa y la API si corresponde.
5. Consultar o descargar los registros almacenados en la microSD.

### Apagado del Sistema

1. Finalizar la consulta o descarga de datos.
2. Desconectar la alimentación del ESP32.
3. Retirar la tarjeta microSD solamente con el dispositivo apagado.

## Troubleshooting

### Problemas Comunes

**Problema 1: No conecta a Wi-Fi**
- **Causa probable:** SSID o contraseña incorrectos, o falta de señal.
- **Solución:** Revisar las credenciales de la red externa desde la interfaz web y volver a guardarlas.

**Problema 2: ADS1115 no responde**
- **Causa probable:** Conexiones I2C incorrectas o dirección equivocada.
- **Solución:** Revisar SDA, SCL, alimentación y las direcciones `0x48` y `0x49`.

**Problema 3: La microSD no está disponible**
- **Causa probable:** Problemas de conexión SPI o tarjeta inaccesible.
- **Solución:** Revisar las conexiones CS, SCK, MISO y MOSI, y comprobar el formato de la tarjeta.

**Problema 4: No se envían datos a Supabase**
- **Causa probable:** Falta de conexión a Internet o configuración incorrecta de la API.
- **Solución:** Revisar la URL HTTPS, el token y el identificador del dispositivo.

**Problema 5: Mediciones incorrectas**
- **Causa probable:** Conexiones defectuosas o calibración inadecuada.
- **Solución:** Revisar las pinzas, las entradas de los ADS1115 y los factores de calibración.
